#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/mm.h>
#include <asm/pgtable.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/stat.h>
#include <linux/delay.h>
#include <asm/pgtable.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/segment.h>
#include <asm/bitops.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0)
#include <linux/scatterlist.h>
#else
#include <asm/scatterlist.h>
#endif
#include "pcigSkel.h"
#include "pcigDrvStructs.h"
#include "pcigWrappers.h"
#include "pcigInternals.h"

#define TYPE(dev)				(MINOR(dev) >> 4)
#define NUM(dev)				(MINOR(dev) & 0xf)

#ifndef VM_RESERVED
# define VM_RESERVED (VM_DONTEXPAND | VM_DONTDUMP)
#endif

#define PCIGSKEL_DMA_MAPPING		0x0
#define PCIGSKEL_CONFIG_MAPPING		0x1
#define PCIGSKEL_XILINX_MAPPING		0x2
#define PCIGSKEL_PLX_MAPPING		0x3

static int		DebugEnable = 0;
static int		MaxInterruptDescriptors = 16;
static int		MaxDmaBufferDescriptors = 2048;
static int		MaxDmaBuffers = 512;
static int		WaitAfterResetLocalBus = 0;

module_param(DebugEnable, int, S_IRUGO | S_IWUSR);
module_param(MaxInterruptDescriptors, int, S_IRUGO | S_IWUSR);
module_param(MaxDmaBufferDescriptors, int, S_IRUGO | S_IWUSR);
module_param(MaxDmaBuffers, int, S_IRUGO | S_IWUSR);
module_param(WaitAfterResetLocalBus, int, S_IRUGO | S_IWUSR);

MODULE_AUTHOR("Fr�d�ric Deville <frederic.deville@adas.fr>");
MODULE_DESCRIPTION("Driver g�n�rique PCIG");
MODULE_LICENSE("GPL");

MODULE_PARM_DESC(DebugEnable, "Validation du mode Debug");
MODULE_PARM_DESC(MaxInterruptDescriptors, "Nombre max de descripteur d'interruption");
MODULE_PARM_DESC (MaxDmaBufferDescriptors, "Nombre max de descripteur DMA");
MODULE_PARM_DESC(MaxDmaBuffers, "Nombre Max de buffers DMA");
MODULE_PARM_DESC(WaitAfterResetLocalBus, "Temps d'attente apres le reset du FPGA avant de l'acceder");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
// Dummy way to protect concurrent driver access through unlocked_ioctl
DEFINE_SPINLOCK(PcigIoctlLock);
#define IOCTL_LOCK(flags) spin_lock_irqsave(&PcigIoctlLock, flags)
#define IOCTL_UNLOCK(flags) spin_unlock_irqrestore(&PcigIoctlLock, flags)

#else

#define IOCTL_LOCK(flags)
#define IOCTL_UNLOCK(flags)

#endif

struct cdev * device;
static struct class * pcigClass = NULL;

int SkelGetDebugEnableParam()
{
	return(DebugEnable);
}

int SkelGetMaxInterruptDescriptorsParam()
{
	return(MaxInterruptDescriptors);
}

int SkelGetMaxDmaBufferDescriptorsParam()
{
	return(MaxDmaBufferDescriptors);
}

int SkelGetMaxDmaBuffersParam()
{
	return(MaxDmaBuffers);
}

int SkelGetWaitAfterResetLocalBus()
{
	return(WaitAfterResetLocalBus);
}

void* SkelGetPcigClass()
{
	return(pcigClass);
}

static int PcigOpen(struct inode*, struct file*);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
static int PcigRelease( struct inode*, struct file*);
#else
static void PcigRelease( struct inode*, struct file*);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
static long PcigUnlockedIoctl( struct file*, unsigned int, unsigned long);
#else
static int PcigIoctl( struct inode*, struct file*, unsigned int, unsigned long);
#endif
static int PcigMmap(struct file * file, struct vm_area_struct * vma);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,20)
static void PcigIntHandler( int, void *, struct pt_regs *);
#else
irqreturn_t PcigIntHandler( int irq, void *dev_id, struct pt_regs *regs);
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
static int __devinit PcigInitOne(struct pci_dev *pdev, const struct pci_device_id *ent);
static void __devexit PcigRemoveOne(struct pci_dev *dev);
#else
static int PcigInitOne(struct pci_dev *pdev, const struct pci_device_id *ent);
static void PcigRemoveOne(struct pci_dev *dev);
#endif

int SkelRequestIrq(int level, char* lpszName, DEVICE_EXTENSION* pDe)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
	return(request_irq(level, PcigIntHandler, SA_INTERRUPT | SA_SHIRQ, lpszName, pDe));
#else
	return(request_irq(level, (irq_handler_t)PcigIntHandler, IRQF_SHARED, lpszName, pDe));
#endif
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
static struct pci_device_id PcigTbl[] __devinitdata = { 
#else
static struct pci_device_id PcigTbl[] = { 
#endif
	        {PCIGVENDID_80, PCIGDEVID_80, PCIGSUBVENDID_80, PCIGSUBDEVID_80, 0, 0, 0},
	        {PCIGVENDID_54, PCIGDEVID_54, PCIGSUBVENDID_54, PCIGSUBDEVID_54, 0, 0, 0},
	        {PCIGVENDID_56, PCIGDEVID_56, PCIGSUBVENDID_56, PCIGSUBDEVID_56, 0, 0, 0},
	        {PCIEGVENDID_8311, PCIEGDEVID_8311, PCIEGSUBVENDID_8311, PCIEGSUBDEVID_8311, 0, 0, 0},
		        {0,},                        /* terminate list */
};

MODULE_DEVICE_TABLE(pci, PcigTbl);                       /* Ajout au kernel de la liste des periph */

static struct pci_driver PcigDriver =
{                 /* Construction de la structure pci driver */
	name:PCIG_LINUX_DEVICE_NAME,
	id_table:PcigTbl,
	probe:PcigInitOne,
	remove:PcigRemoveOne,
};
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
static int __devinit PcigInitOne(struct pci_dev *pdev, const struct pci_device_id *ent)
#else
static int PcigInitOne(struct pci_dev *pdev, const struct pci_device_id *ent)
#endif
{
	return( OnPcigInitOne(pdev));
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
static void __devexit PcigRemoveOne(struct pci_dev *dev)
#else
static void PcigRemoveOne(struct pci_dev *dev)
#endif
{
	OnPcigRemoveOne(dev);
}

/**********************************/
/* PCIG driver operations table   */
/**********************************/
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,20)
static struct file_operations Fops = {
	NULL,				// module
	NULL,				// seek
	NULL,				// read
	NULL,				// write
	NULL,				// readdir
	NULL,				// select
	PcigIoctl,			// ioctl
	PcigMmap,			// mmap
	PcigOpen,			// open
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
	NULL,				// flush
#endif
	PcigRelease			// release
};
#else
static struct file_operations Fops = {
	.owner = THIS_MODULE,		// module
	.llseek = NULL,				// seek
	.read = NULL,				// read
	.write = NULL,				// write
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	.readdir = NULL,				// readdir
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
	.unlocked_ioctl =  PcigUnlockedIoctl,
#else
	.ioctl = PcigIoctl,			// ioctl
#endif
	.mmap = PcigMmap,			// mmap
	.open = PcigOpen,			// open
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
	.flush = NULL,				// flush
#endif
	.release = PcigRelease			// release
};
#endif

/**********************************/
/* Memory handler functions       */
/**********************************/
void mmap_drv_vopen(struct vm_area_struct * vma);
void mmap_drv_vclose(struct vm_area_struct * vma);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,20)
struct page * mmap_drv_vmmap(struct vm_area_struct *, unsigned long, int);
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,1,0)
vm_fault_t
#else
int 
#endif
    mmap_drv_vmmap_vmf(
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,11,0)
            struct vm_area_struct *vma,
#endif
            struct vm_fault *vmf);
int mmap_drv_access_vmf(struct vm_area_struct *vma, unsigned long addr, void *buf, int len, int write);
#else
struct page * mmap_drv_vmmap(struct vm_area_struct *, unsigned long, int*);
#endif
#endif

/***********************************/
/* Memory handler operations table */
/***********************************/
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,20)
static struct vm_operations_struct mmap_drv_vm_ops = {
	mmap_drv_vopen,		// open
	mmap_drv_vclose,	// close
	mmap_drv_vmmap		// no-page fault handler
};
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
static struct vm_operations_struct mmap_drv_vm_ops = {
	.open = mmap_drv_vopen,		// open
	.close = mmap_drv_vclose,	// close	
	.fault = mmap_drv_vmmap_vmf,        // no-page fault handler
	.access = mmap_drv_access_vmf,        // no-page fault handler
};
#else
static struct vm_operations_struct mmap_drv_vm_ops = {
	.open = mmap_drv_vopen,		// open
	.close = mmap_drv_vclose,	// close
	.nopage = mmap_drv_vmmap,	// no-page fault handler
};
#endif
#endif

/**********************************/
/* Driver initialization function */
/**********************************/
int init_module()
{
	dev_t   dev;
	int     ret;
	char	lpszTemp[256];

	// Invocation de la fonction d'initialisation interne
	OnPcigInitModule();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
	pdebug(("============= Install Driver PCIG for kernel 2.6 ============"));
	ret=alloc_chrdev_region( &dev, 0, MAX_PCIG, PCIG_LINUX_DEVICE_NAME);
	sprintf( lpszTemp, "============= dev  %d ============", MAJOR(dev));
	pdebug((lpszTemp));
	if(ret!=0)
	{
		printk(KERN_ERR "PCIG : %s failed \n","Sorry, registering the character device ");
		return(ret);
	}
	pcigClass = class_create(THIS_MODULE, PCIG_LINUX_DEVICE_NAME);
	OnPcigSetMajor(MAJOR(dev));
	device=cdev_alloc();
	device->owner=THIS_MODULE;
	device->ops=&Fops;
	kobject_set_name(&device->kobj, PCIG_LINUX_DEVICE_NAME);
	pdebug(("============= cdev_add ============"));
	ret=cdev_add( device, MKDEV(MAJOR(dev),0), MAX_PCIG);
        if(ret!=0)
	{
		pdebug(("PCIG : Add of driver failed"));
		return(ret);
	}
	pdebug(("============= pci_register_driver ============"));
	if ((ret = pci_register_driver(&PcigDriver)) < 0)
	{
		printk(KERN_ERR "ADAS: can't register PCI driver (%d)\n",ret);
		unregister_chrdev(dev, PCIG_LINUX_DEVICE_NAME);
		return -EIO;
	}
	printk(KERN_INFO "PCIG :  %s The major device number is %d.\n", "Registeration is a success", MAJOR(dev));
	device->dev = dev;
#endif

	return(0);
}

/**********************************/
/* Driver cleanup function        */
/**********************************/
void cleanup_module()
{
	int		cr = 0;
	
	PcigCleanupModule();
	if(OnPcigGetPcigCount() != 0)
	{
		unregister_chrdev(OnPcigGetMajor(), PCIG_LINUX_DEVICE_NAME);
		if (cr < 0)
		{
			pdebug(("Error unregister\n"));
		}
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
       pdebug(("============= Uninstall Driver PCIG for kernel 2.6 ============\n"));	
       cdev_del(device);
       unregister_chrdev_region( MKDEV(OnPcigGetMajor(), 0), MAX_PCIG);
 	   /* unregister PCI driver */
       pci_unregister_driver(&PcigDriver);
	   if(pcigClass)
	   {
			   class_destroy(pcigClass);
	   }

#else
	int               ii, num;
	unsigned long     flags;

	pdebug(("============= Uninstall Driver PCIG for kernel 2.2 or 2.4 ============\n"));	

	/* Unregister the device */
	if(OnPcigGetPcigCount() != 0)
	{
		// RS Mandriva 64
		unregister_chrdev(OnPcigGetMajor(), PCIG_LINUX_DEVICE_NAME);
	}
#endif
}
/********************************/
/* Open driver function         */
/********************************/
int PcigOpen(struct inode *inode, struct file *file)
{
	int		type;
	int		num;

	type = TYPE(inode->i_rdev);
	num = NUM(inode->i_rdev);

	file->private_data = OnPcigOpen(num);
	if(file->private_data == NULL)
	{
		return(-ENODEV);
	}
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,20)
	MOD_INC_USE_COUNT;
#endif
	return(0);
}

/********************************/
/* Release driver function      */
/********************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
static int PcigRelease( struct inode *inode, struct file *file)
#else
static void PcigRelease( struct inode *inode, struct file *file)
#endif
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,20)
	MOD_DEC_USE_COUNT;
#endif
	// MOD_IN_USE;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
	return(0);
#endif
}

/*************************************/
/* device memory map driver function */
/*************************************/
static int PcigMmap(struct file * file, struct vm_area_struct * vma)
{
	DEVICE_EXTENSION*	lpDev;
	unsigned long	ulOffset;
	unsigned long	ulDmaChannel;
	unsigned long	ulMappingType = 0;
//	unsigned long	ulAddressToMap = 0;
	int		res;

	res = 0;
	ulOffset = vma->vm_pgoff << PAGE_SHIFT;
	ulDmaChannel = ulOffset & 0x80000000;
	ulMappingType = (ulOffset & 0x00FF0000) >> 16;
	ulOffset &= 0x0000FFFF;
	if(ulMappingType == PCIGSKEL_PLX_MAPPING)
	{
		lpDev = (DEVICE_EXTENSION*)file->private_data;
		if(OnPcigRemapPlxRegisters(vma, lpDev->ulCardNo))
		{
			pdebug((" Erreur OnPcigRemapPlxRegisters \n"));
			return(-EAGAIN);
		}
	}
	else
	{
		if(ulMappingType == PCIGSKEL_XILINX_MAPPING)
		{
			lpDev = (DEVICE_EXTENSION*)file->private_data;
			if(OnPcigRemapXilinxRegisters(vma, lpDev->ulCardNo))
			{
				pdebug((" Erreur OnPcigRemapXilinxRegisters \n"));
				return(-EAGAIN);
			}
		}
		else
		{
			if(ulMappingType == PCIGSKEL_CONFIG_MAPPING)
			{
				lpDev = (DEVICE_EXTENSION*)file->private_data;
				if(OnPcigRemapConfigRegisters(vma, lpDev->ulCardNo))
				{
					pdebug((" Erreur OnPcigRemapConfigRegisters \n"));
					return(-EAGAIN);
				}
			}
			else
			{
				vma->vm_pgoff = ulOffset >> PAGE_SHIFT;
				if(ulOffset >= __pa(high_memory) || (file->f_flags & O_SYNC))
				{
					vma->vm_flags |= VM_IO;
				}
				lpDev = (DEVICE_EXTENSION*)file->private_data;
				vma->vm_private_data = (void*)(lpDev->ulCardNo | ulDmaChannel);
				vma->vm_flags |= VM_LOCKED | VM_RESERVED /*| VM_SHM*/;
				if ( ulDmaChannel != 0)
				{
					ulDmaChannel = 1;
				}
				if (OnPcigRemapPfnRange(vma, lpDev->ulCardNo, ulDmaChannel)) 
				{
					pdebug((" Erreur remap_pfn_range \n"));
					return(-EAGAIN);
				}

				vma->vm_ops = &mmap_drv_vm_ops;
				mmap_drv_vopen(vma);
			}
		}
	}
	pdebug(("PcigMmap : OK\n"));
	return(res);
}

/********************************/
/* I/O Control driver function  */
/********************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
static long PcigUnlockedIoctl( struct file *file, unsigned int cmd, unsigned long arg)
#else
static int PcigIoctl( struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
	DEVICE_EXTENSION*	lpDev;
   	int					err = 0;
	unsigned int		ulValue = 0;
	unsigned int*		lpuiValue = 0;
	REG_DES				RegDes;
	REGS_DES			RegsDes;
	REG_DES_EX			RegDesEx;
	REGS_DES_EX			RegsDesEx;
	REGS_DES_WC			RegsDesWc;
	WAIT_DESC			WaitDesc;
	RESOURCES_INFO		ResInfo;
	CONFIG_FPGA			ConfigFpga;
	DMA_CHANNEL			DmaChannel;
	INTERRUPTS			Interrupts;
	VERSION_STRING		VersionString;
	INT_DESC*			lpIntDesc;
	char*				lpszConfig;
	unsigned int *		lpulRegsValue;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
	unsigned long		flags;
#endif

	pdebug(("PcigIoctl : Enter\n"));
	lpDev = (DEVICE_EXTENSION*)file->private_data;

	if(_IOC_TYPE(cmd) != PCIGIOC_MAGIC)
	{
		return -EINVAL;
	}
	if(_IOC_NR(cmd) > PCIGIOC_MAXNR)
	{
		return -EINVAL;
	}
	if(_IOC_DIR(cmd) & _IOC_READ)
	{
		//err = WrapVerifyAreaRead((void *)arg, size);
	}
	else
	{
		if (_IOC_DIR(cmd) & _IOC_WRITE)
		{
			//err = WrapVerifyAreaWrite((void *)arg, size);
		}
	}
	if (err)
	{
		return(err);
	}
    IOCTL_LOCK(flags);
	switch(cmd)
	{
		case PCIGIOC_ALLOCATE_DESIGN_BUFFER:
			pdebug(("PcigIoctl : PCIGIOC_ALLOCATE_DESIGN_BUFFER\n"));
			err = copy_from_user((int*)&ulValue, (int*)arg, sizeof(unsigned int));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_ALLOCATE_DESIGN_BUFFER copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigAllocateDesignBuffer(lpDev, ulValue);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_ALLOCATE_DESIGN_BUFFER OnPcigAllocateDesignBuffer error\n"));
				IOCTL_UNLOCK(flags);
				return(-EFAULT);
			}
			pdebug(("PcigIoctl : PCIGIOC_ALLOCATE_DESIGN_BUFFER Successfull\n"));
			break;


		case PCIGIOC_GET_ALLOCATED_RESOURCES:
			pdebug(("PcigIoctl : PCIGIOC_GET_ALLOCATED_RESOURCES\n"));
			err = OnPcigGetAllocatedResources(lpDev, &ResInfo);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_ALLOCATED_RESOURCES OnPcigGetAllocatedResources error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_to_user((int*)arg, &ResInfo, sizeof(RESOURCES_INFO));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_ALLOCATED_RESOURCES copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_GET_ALLOCATED_RESOURCES Successfull\n"));
			break;

		case PCIGIOC_GET_PCI_CFG_REGS:
			pdebug(("PcigIoctl : PCIGIOC_GET_PCI_CFG_REGS\n"));
			pdebug(("PcigIoctl : PCIGIOC_GET_PCI_CFG_REGS Successfull\n"));
			break;

		case PCIGIOC_GET_LOCAL_CFG_REGS:
			pdebug(("PcigIoctl : PCIGIOC_GET_LOCAL_CFG_REGS\n"));
			err = copy_to_user((int*)arg, (int*)lpDev->pPlxMem, sizeof(PLXLCFGREGS));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_LOCAL_CFG_REGS copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_GET_LOCAL_CFG_REGS Successfull\n"));
			break;
    
		case PCIGIOC_SET_LOCAL_CFG_REGS:
			pdebug(("PcigIoctl : PCIGIOC_SET_LOCAL_CFG_REGS\n"));
			err = copy_from_user((int*)lpDev->pPlxMem, (int*)arg, sizeof(PLXLCFGREGS));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_SET_LOCAL_CFG_REGS copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_SET_LOCAL_CFG_REGS Successfull\n"));
			break;
    
		case PCIGIOC_GET_LOCAL_CFG_REG:
			pdebug(("PcigIoctl : PCIGIOC_GET_LOCAL_CFG_REG\n"));
			err = copy_from_user((int*)&RegDes, (int*)arg, sizeof(REG_DES));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_LOCAL_CFG_REG copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigGetLocalCfgReg(lpDev, RegDes.ulRegName, &(RegDes.ulRegValue));
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_LOCAL_CFG_REG OnPcigGetLocalCfgReg error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_to_user((int*)arg, (int*)&RegDes, sizeof(REG_DES));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_LOCAL_CFG_REG copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_GET_LOCAL_CFG_REG Successfull\n"));
			break;
     
		case PCIGIOC_SET_LOCAL_CFG_REG:
			pdebug(("PcigIoctl : PCIGIOC_SET_LOCAL_CFG_REG\n"));
			err = copy_from_user((int*)&RegDes, (int*)arg, sizeof(REG_DES));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_SET_LOCAL_CFG_REG copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigSetLocalCfgReg(lpDev, RegDes.ulRegName, RegDes.ulRegValue);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_SET_LOCAL_CFG_REG OnPcigSetLocalCfgReg error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_SET_LOCAL_CFG_REG Successfull\n"));
			break;
     
		case PCIGIOC_INITIALIZE_DMA_CHANNEL:
			pdebug(("PcigIoctl : PCIGIOC_INITIALIZE_DMA_CHANNEL\n"));
			err = copy_from_user((int*)&DmaChannel, (int*)arg, sizeof(DMA_CHANNEL));
			if(err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_INITIALIZE_DMA_CHANNEL copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			lpuiValue = (unsigned int *)DmaChannel.lpBufferDescriptor;
			DmaChannel.lpBufferDescriptor = WrapVmalloc(DmaChannel.ulNbBuffers * sizeof(DMA_BUFFER_DESCRIPTOR));
			if(DmaChannel.lpBufferDescriptor == NULL)
			{
			    pdebug(("PcigIoctl : PCIGIOC_INITIALIZE_DMA_CHANNEL WrapVmalloc error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_from_user((int*)DmaChannel.lpBufferDescriptor, (int*)lpuiValue, DmaChannel.ulNbBuffers * sizeof(DMA_BUFFER_DESCRIPTOR));
			if(err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_INITIALIZE_DMA_CHANNEL copy_from_user error\n"));
				WrapVfree(DmaChannel.lpBufferDescriptor);
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigInitializeDmaChannel(lpDev, &DmaChannel);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_INITIALIZE_DMA_CHANNEL OnPcigInitializeDmaChannel error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			WrapVfree(DmaChannel.lpBufferDescriptor);
			pdebug(("PcigIoctl : PCIGIOC_INITIALIZE_DMA_CHANNEL Successfull\n"));
			break;

		case PCIGIOC_START_DMA_CHANNEL:
			pdebug(("PcigIoctl : PCIGIOC_START_DMA_CHANNEL\n"));
			err = copy_from_user((int*)&ulValue, (int*)arg, sizeof(unsigned int));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_START_DMA_CHANNEL copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigStartDmaChannel(lpDev, ulValue);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_START_DMA_CHANNEL OnPcigStartDmaChannel error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_START_DMA_CHANNEL Successfull\n"));
			break;

		case PCIGIOC_STOP_DMA_CHANNEL:
			pdebug(("PcigIoctl : PCIGIOC_STOP_DMA_CHANNEL\n"));
			err = copy_from_user((int*)&ulValue, (int*)arg, sizeof(unsigned int));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_STOP_DMA_CHANNEL copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigStopDmaChannel(lpDev, ulValue);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_STOP_DMA_CHANNEL OnPcigStopDmaChannel error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_STOP_DMA_CHANNEL Successfull\n"));
			break;

		case PCIGIOC_RESET_DMA_CHANNEL:
			pdebug(("PcigIoctl : PCIGIOC_RESET_DMA_CHANNEL\n"));
			err = copy_from_user((int*)&ulValue, (int*)arg, sizeof(unsigned int));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_RESET_DMA_CHANNEL copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigResetDmaChannel(lpDev, ulValue);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_RESET_DMA_CHANNEL OnPcigResetDmaChannel error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_RESET_DMA_CHANNEL Successfull\n"));
			break;

		case PCIGIOC_FREE_DMA_CHANNEL:
			pdebug(("PcigIoctl : PCIGIOC_FREE_DMA_CHANNEL\n"));
			err = copy_from_user((int*)&ulValue, (int*)arg, sizeof(unsigned int));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_FREE_DMA_CHANNEL copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigReleaseDmaChannel(lpDev, ulValue);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_FREE_DMA_CHANNEL OnPcigReleaseDmaChannel error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_FREE_DMA_CHANNEL Successfull\n"));
			break;

		case PCIGIOC_GET_XILINX_REGS:
			pdebug(("PcigIoctl : PCIGIOC_GET_XILINX_REGS\n"));
			err = copy_from_user((int*)&RegsDes, (int*)arg, sizeof(REGS_DES));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_XILINX_REGS copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			lpulRegsValue = WrapVmalloc(RegsDes.ulNbRegs * OnPcigGetElementSize(RegsDes.ulAddressInc));
			if(lpulRegsValue == 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_XILINX_REGS WrapVmalloc error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigGetXilinxRegs(lpDev, RegsDes.ulStartRegName, RegsDes.ulNbRegs, lpulRegsValue, RegsDes.ulAddressInc);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_XILINX_REGS OnPcigGetXilinxRegs error\n"));
				WrapVfree(lpulRegsValue);
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_to_user((int*)RegsDes.lpulRegsValue, (int*)lpulRegsValue, RegsDes.ulNbRegs * OnPcigGetElementSize(RegsDes.ulAddressInc));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_XILINX_REGS copy_to_user error\n"));
				WrapVfree(lpulRegsValue);
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			WrapVfree(lpulRegsValue);
			pdebug(("PcigIoctl : PCIGIOC_GET_XILINX_REGS Successfull\n"));
			break;

		case PCIGIOC_SET_XILINX_REGS:
			pdebug(("PcigIoctl : PCIGIOC_SET_XILINX_REGS\n"));
			err = copy_from_user((int*)&RegsDes, (int*)arg, sizeof(REGS_DES));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_SET_XILINX_REGS copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			lpulRegsValue = WrapVmalloc(RegsDes.ulNbRegs * OnPcigGetElementSize(RegsDes.ulAddressInc));
			if(lpulRegsValue == 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_SET_XILINX_REGS WrapVmalloc error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_from_user((int*)lpulRegsValue, (int*)RegsDes.lpulRegsValue, RegsDes.ulNbRegs * OnPcigGetElementSize(RegsDes.ulAddressInc));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_SET_XILINX_REGS copy_from_user error\n"));
				WrapVfree(lpulRegsValue);
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigSetXilinxRegs(lpDev, RegsDes.ulStartRegName, RegsDes.ulNbRegs, lpulRegsValue, RegsDes.ulAddressInc);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_SET_XILINX_REGS OnPcigSetXilinxRegs error\n"));
				WrapVfree(lpulRegsValue);
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			WrapVfree(lpulRegsValue);
			pdebug(("PcigIoctl : PCIGIOC_SET_XILINX_REGS Successfull\n"));
			break;

		case PCIGIOC_GET_REG:
			pdebug(("PcigIoctl : PCIGIOC_GET_REG\n"));
			err = copy_from_user((int*)&RegDesEx, (int*)arg, sizeof(REG_DES_EX));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_REG copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigGetReg(lpDev, RegDesEx.ulRegName, RegDesEx.ulRegQual, &(RegDesEx.ulRegValue));
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_REG OnPcigGetReg error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_to_user((int*)arg, (int*)&RegDesEx, sizeof(REG_DES_EX));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_REG copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_GET_REG Successfull\n"));
			break;

		case PCIGIOC_SET_REG:
			pdebug(("PcigIoctl : PCIGIOC_SET_REG\n"));
			err = copy_from_user((int*)&RegDesEx, (int*)arg, sizeof(REG_DES_EX));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_SET_REG copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigSetReg(lpDev, RegDesEx.ulRegName, RegDesEx.ulRegQual, RegDesEx.ulRegValue);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_SET_REG OnPcigSetReg error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_SET_REG Successfull\n"));
			break;

		case PCIGIOC_GET_REGS:
			pdebug(("PcigIoctl : PCIGIOC_GET_REGS\n"));
			err = copy_from_user((int*)&RegsDesEx, (int*)arg, sizeof(REGS_DES_EX));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_REGS copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			lpulRegsValue = WrapVmalloc(RegsDesEx.ulRegCount * OnPcigGetElementSize(RegsDesEx.ulAddressInc));
			if(lpulRegsValue == 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_REGS WrapVmalloc error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigGetRegs(lpDev, lpulRegsValue, RegsDesEx.ulRegCount, RegsDesEx.ulRegName, RegsDesEx.ulRegQual, RegsDesEx.ulAddressInc);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_REGS OnPcigGetRegs error\n"));
				WrapVfree(lpulRegsValue);
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_to_user((int*)RegsDesEx.lpulRegValue, (int*)lpulRegsValue, RegsDesEx.ulRegCount * OnPcigGetElementSize(RegsDesEx.ulAddressInc));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_REGS copy_to_user error\n"));
				WrapVfree(lpulRegsValue);
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			WrapVfree(lpulRegsValue);
			pdebug(("PcigIoctl : PCIGIOC_GET_XILINX_REGS Successfull\n"));
			break;

		case PCIGIOC_SET_REGS:
			pdebug(("PcigIoctl : PCIGIOC_SET_REGS\n"));
			err = copy_from_user((int*)&RegsDesEx, (int*)arg, sizeof(REGS_DES_EX));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_SET_REGS copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			lpulRegsValue = WrapVmalloc(RegsDesEx.ulRegCount * OnPcigGetElementSize(RegsDesEx.ulAddressInc));
			if(lpulRegsValue == 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_SET_REGS WrapVmalloc error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_from_user((int*)lpulRegsValue, (int*)RegsDesEx.lpulRegValue, RegsDesEx.ulRegCount * OnPcigGetElementSize(RegsDesEx.ulAddressInc));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_SET_REGS copy_from_user error\n"));
				WrapVfree(lpulRegsValue);
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigSetRegs(lpDev, lpulRegsValue, RegsDesEx.ulRegCount, RegsDesEx.ulRegName, RegsDesEx.ulRegQual, RegsDesEx.ulAddressInc);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_SET_REGS OnPcigSetRegs error\n"));
				WrapVfree(lpulRegsValue);
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			WrapVfree(lpulRegsValue);
			pdebug(("PcigIoctl : PCIGIOC_SET_XILINX_REGS Successfull\n"));
			break;

		case PCIGIOC_WRITE_AND_CHECK_XILINX_REGS:
			pdebug(("PcigIoctl : PCIGIOC_WRITE_AND_CHECK_XILINX_REGS\n"));
			err = copy_from_user((int*)&RegsDesWc, (int*)arg, sizeof(REGS_DES_WC));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WRITE_AND_CHECK_XILINX_REGS copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			lpulRegsValue = WrapVmalloc(RegsDesWc.ulNbRegs * sizeof(unsigned long));
			if(lpulRegsValue == 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WRITE_AND_CHECK_XILINX_REGS WrapVmalloc error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_from_user((int*)lpulRegsValue, (int*)RegsDesWc.lpulInRegsValue, RegsDesWc.ulNbRegs * sizeof(unsigned long));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WRITE_AND_CHECK_XILINX_REGS copy_from_user error\n"));
				WrapVfree(lpulRegsValue);
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigWriteAndCheckXilinxRegs(lpDev,  RegsDesWc.ulInStartRegName, RegsDesWc.ulInStartRegQual, RegsDesWc.ulOutStartRegName, RegsDesWc.ulOutStartRegQual, RegsDesWc.ulNbRegs, lpulRegsValue, RegsDesWc.ulInAddressInc, RegsDesWc.ulOutAddressInc);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WRITE_AND_CHECK_XILINX_REGS OnPcigGetXilinxRegs error\n"));
				WrapVfree(lpulRegsValue);
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_to_user((int*)RegsDesWc.lpulOutRegsValue, (int*)lpulRegsValue, RegsDesWc.ulNbRegs * sizeof(unsigned long));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WRITE_AND_CHECK_XILINX_REGS copy_to_user error\n"));
				WrapVfree(lpulRegsValue);
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			WrapVfree(lpulRegsValue);
			pdebug(("PcigIoctl : PCIGIOC_WRITE_AND_CHECK_XILINX_REGS Successfull\n"));
			break;

		case PCIGIOC_GET_XILINX_REG:
			pdebug(("PcigIoctl : PCIGIOC_GET_XILINX_REG\n"));
			err = copy_from_user((int*)&RegDes, (int*)arg, sizeof(REG_DES));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_XILINX_REG copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigGetXilinxReg(lpDev, RegDes.ulRegName, &(RegDes.ulRegValue));
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_XILINX_REG OnPcigGetXilinxReg error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_to_user((int*)arg, (int*)&RegDes, sizeof(REG_DES));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_XILINX_REG copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_GET_XILINX_REG Successfull\n"));
			break;

		case PCIGIOC_SET_XILINX_REG:
			pdebug(("PcigIoctl : PCIGIOC_SET_XILINX_REG\n"));
			err = copy_from_user((int*)&RegDes, (int*)arg, sizeof(REG_DES));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_SET_XILINX_REG copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigSetXilinxReg(lpDev, RegDes.ulRegName, RegDes.ulRegValue);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_SET_XILINX_REG OnPcigSetXilinxReg error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_SET_XILINX_REG Successfull\n"));
			break;

		case PCIGIOC_RESET_LOCAL_BUS:
			pdebug(("PcigIoctl : PCIGIOC_RESET_LOCAL_BUS\n"));
			err = OnPcigResetLocalBus(lpDev);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_RESET_LOCAL_BUS OnPcigResetLocalBus error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_RESET_LOCAL_BUS Successfull\n"));
			break;

		case PCIGIOC_LOAD_LOGICAL_DESIGN:
			pdebug(("PcigIoctl : PCIGIOC_LOAD_LOGICAL_DESIGN\n"));
			err = copy_from_user((int*)&ConfigFpga, (int*)arg, sizeof(CONFIG_FPGA));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_LOAD_LOGICAL_DESIGN copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			lpszConfig = (char*)WrapVmalloc(ConfigFpga.ulConfigLen);
			err = copy_from_user((int*)lpszConfig, (int*)ConfigFpga.lpszConfig, ConfigFpga.ulConfigLen);
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_LOAD_LOGICAL_DESIGN copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigLoadLogicalDesign(lpDev, lpszConfig, ConfigFpga.ulConfigLen);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_LOAD_LOGICAL_DESIGN OnPcigLoadLogicalDesign error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			WrapVfree(lpszConfig);
			pdebug(("PcigIoctl : PCIGIOC_LOAD_LOGICAL_DESIGN Successfull\n"));
			break;

		case PCIGIOC_ENABLE_INTERRUPTS:
			pdebug(("PcigIoctl : PCIGIOC_ENABLE_INTERRUPTS\n"));
			err = copy_from_user((int*)&ulValue, (int*)arg, sizeof(unsigned int));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_ENABLE_INTERRUPTS copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigEnableInterrupts(lpDev, ulValue);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_ENABLE_INTERRUPTS OnPcigEnableInterrupts error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_ENABLE_INTERRUPTS Successfull\n"));
			break;
     
		case PCIGIOC_READ_ONE_EEPROM:
			pdebug(("PcigIoctl : PCIGIOC_READ_ONE_EEPROM\n"));
			err = copy_from_user((int*)&RegDes, (int*)arg, sizeof(REG_DES));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_READ_ONE_EEPROM copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigReadEERegister(lpDev, RegDes.ulRegName, &(RegDes.ulRegValue));
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_READ_ONE_EEPROM OnPcigReadEERegister error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_to_user((int*)arg, (int*)&RegDes, sizeof(REG_DES));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_READ_ONE_EEPROM copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_READ_ONE_EEPROM Successfull\n"));
			break;
       
		case PCIGIOC_WRITE_ONE_EEPROM:
			pdebug(("PcigIoctl : PCIGIOC_WRITE_ONE_EEPROM\n"));
			err = copy_from_user((int*)&RegDes, (int*)arg, sizeof(REG_DES));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WRITE_ONE_EEPROM copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigWriteEERegister(lpDev, RegDes.ulRegName, RegDes.ulRegValue);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WRITE_ONE_EEPROM OnPcigWriteEERegister error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_WRITE_ONE_EEPROM Successfull\n"));
			break;
      
		case PCIGIOC_REGISTER_USER_INT:
			pdebug(("PcigIoctl : PCIGIOC_REGISTER_USER_INT\n"));
			err = copy_from_user((int*)&Interrupts, (int*)arg, sizeof(INTERRUPTS));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_REGISTER_USER_INT copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			lpIntDesc = (INT_DESC *)Interrupts.lpInterruptDesc;
			Interrupts.lpInterruptDesc = (INT_DESC*)WrapVmalloc(Interrupts.ulInterruptCount * sizeof(INT_DESC));
			if(Interrupts.lpInterruptDesc == NULL)
			{
			    pdebug(("PcigIoctl : PCIGIOC_REGISTER_USER_INT WrapVmalloc error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_from_user(Interrupts.lpInterruptDesc, lpIntDesc, Interrupts.ulInterruptCount * sizeof(INT_DESC));
			if(err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_REGISTER_USER_INT copy_from_user error\n"));
				WrapVfree(Interrupts.lpInterruptDesc);
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = OnPcigRegisterUserInt(lpDev, Interrupts.ulInterruptCount, Interrupts.lpInterruptDesc);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_REGISTER_USER_INT OnPcigRegisterUserInt error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			WrapVfree(Interrupts.lpInterruptDesc);
			pdebug(("PcigIoctl : PCIGIOC_REGISTER_USER_INT Successfull\n"));
			break;
     
		case PCIGIOC_RELEASE_USER_INT:
			pdebug(("PcigIoctl : PCIGIOC_RELEASE_USER_INT\n"));
			err = OnPcigReleaseUserInt(lpDev);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_RELEASE_USER_INT OnPcigReleaseUserInt error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_RELEASE_USER_INT Successfull\n"));
			break;
      
		case PCIGIOC_GET_NB_DEVICE:
			pdebug(("PcigIoctl : PCIGIOC_GET_NB_DEVICE\n"));
			ulValue = OnPcigGetPcigCount();
			put_user(ulValue, (unsigned int*)arg);
			pdebug(("PcigIoctl : PCIGIOC_GET_NB_DEVICE Successfull\n"));
			break;
         
		case PCIGIOC_GET_VERSION:
			pdebug(("PcigIoctl : PCIGIOC_GET_VERSION\n"));
            strncpy(VersionString.lpszVersionInfo, PCIG_VERSION_INFO, strlen(PCIG_VERSION_INFO) + 1);
			err = copy_to_user( (int *)arg, &VersionString, sizeof(VERSION_STRING));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_VERSION copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_GET_VERSION Successfull\n"));
			break;

		case PCIGIOC_WAIT_FOR_DMA_0:
			pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_0\n"));
			err = copy_from_user((int*)&WaitDesc, (int*)arg, sizeof(WAIT_DESC));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_0 copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			IOCTL_UNLOCK(flags);
			err = OnPcigWaitForDma(lpDev, 0, &WaitDesc, 0);
			IOCTL_LOCK(flags);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_0 OnPcigWaitForDma error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_to_user( (int *)arg, &WaitDesc, sizeof(WAIT_DESC));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_0 copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_0 Successfull\n"));
			break;

		case PCIGIOC_WAIT_FOR_DMA_1:
			pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_1\n"));
			err = copy_from_user((int*)&WaitDesc, (int*)arg, sizeof(WAIT_DESC));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_1 copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			IOCTL_UNLOCK(flags);
			err = OnPcigWaitForDma(lpDev, 1, &WaitDesc, 0);
			IOCTL_LOCK(flags);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_1 OnPcigWaitForDma error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_to_user( (int *)arg, &WaitDesc, sizeof(WAIT_DESC));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_1 copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_1 Successfull\n"));
			break;

		case PCIGIOC_WAIT_FOR_USER_INT:
			pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_USER_INT\n"));
			err = copy_from_user((int*)&WaitDesc, (int*)arg, sizeof(WAIT_DESC));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_USER_INT copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			IOCTL_UNLOCK(flags);
			err = OnPcigWaitForUserInt(lpDev, &WaitDesc, 0);
			IOCTL_LOCK(flags);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_USER_INT OnPcigWaitForUserInt error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_to_user( (int *)arg, &WaitDesc, sizeof(WAIT_DESC));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_USER_INT copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_USER_INT Successfull\n"));
			break;

		case PCIGIOC_WAIT_FOR_DMA_0_EX:
			pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_0_EX\n"));
			err = copy_from_user((int*)&WaitDesc, (int*)arg, sizeof(WAIT_DESC));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_0_EX copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			IOCTL_UNLOCK(flags);
			err = OnPcigWaitForDma(lpDev, 0, &WaitDesc, 1);
			IOCTL_LOCK(flags);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_0_EX OnPcigWaitForDma error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_to_user( (int *)arg, &WaitDesc, sizeof(WAIT_DESC));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_0_EX copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_0_EX Successfull\n"));
			break;

		case PCIGIOC_WAIT_FOR_DMA_1_EX:
			pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_1_EX\n"));
			err = copy_from_user((int*)&WaitDesc, (int*)arg, sizeof(WAIT_DESC));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_1_EX copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			IOCTL_UNLOCK(flags);
			err = OnPcigWaitForDma(lpDev, 1, &WaitDesc, 1);
			IOCTL_LOCK(flags);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_1_EX OnPcigWaitForDma error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_to_user( (int *)arg, &WaitDesc, sizeof(WAIT_DESC));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_1_EX copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_DMA_1_EX Successfull\n"));
			break;

		case PCIGIOC_WAIT_FOR_USER_INT_EX:
			pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_USER_INT_EX\n"));
			err = copy_from_user((int*)&WaitDesc, (int*)arg, sizeof(WAIT_DESC));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_USER_INT_EX copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			IOCTL_UNLOCK(flags);
			err = OnPcigWaitForUserInt(lpDev, &WaitDesc, 1);
			IOCTL_LOCK(flags);
			if(err != PCIG_SUCCESS)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_USER_INT_EX OnPcigWaitForUserInt error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			err = copy_to_user( (int *)arg, &WaitDesc, sizeof(WAIT_DESC));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_USER_INT_EX copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_WAIT_FOR_USER_INT_EX Successfull\n"));
			break;

		case PCIGIOC_GET_DMA0_INT_COUNT:
			pdebug(("PcigIoctl : PCIGIOC_GET_DMA0_INT_COUNT\n"));
			ulValue = OnPcigGetDmaInterrruptCount(lpDev, 0);
			err = copy_to_user( (int *)arg, &ulValue, sizeof(unsigned int));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_DMA0_INT_COUNT copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_GET_DMA0_INT_COUNT Successfull\n"));
			break;

		case PCIGIOC_GET_DMA1_INT_COUNT:
			pdebug(("PcigIoctl : PCIGIOC_GET_DMA1_INT_COUNT\n"));
			ulValue = OnPcigGetDmaInterrruptCount(lpDev, 1); 
			err = copy_to_user( (int *)arg, &ulValue, sizeof(unsigned int));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_DMA1_INT_COUNT copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_GET_DMA1_INT_COUNT Successfull\n"));
			break;

		case PCIGIOC_GET_USER_INT_COUNT:
			pdebug(("PcigIoctl : PCIGIOC_GET_USER_INT_COUNT\n"));
			err = copy_from_user((int*)&ulValue, (int*)arg, sizeof(unsigned int));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_USER_INT_COUNT copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			ulValue = OnPcigGetUserInterruptCount(lpDev, ulValue); 
			err = copy_to_user( (int *)arg, &ulValue, sizeof(unsigned int));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_USER_INT_COUNT copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_GET_USER_INT_COUNT Successfull\n"));
			break;

		case PCIGIOC_READ_ONE_PCIE_EEPROM:
            pdebug(("PcigIoctl : PCIGIOC_READ_ONE_PCIE_EEPROM\n"));
            err = copy_from_user((int*)&RegDes, (int*)arg, sizeof(REG_DES));
            if (err != 0)
            {
                pdebug(("PcigIoctl : PCIGIOC_READ_ONE_PCIE_EEPROM copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
                return -EFAULT;
            }
            err = OnPcigReadPcieEERegister(lpDev, RegDes.ulRegName, &(RegDes.ulRegValue));
            if(err != PCIG_SUCCESS)
            {
                pdebug(("PcigIoctl : PCIGIOC_READ_ONE_PCIE_EEPROM OnPcigReadEERegister error\n"));
				IOCTL_UNLOCK(flags);
                return -EFAULT;
            }
            err = copy_to_user((int*)arg, (int*)&RegDes, sizeof(REG_DES));
            if (err != 0)
            {
                pdebug(("PcigIoctl : PCIGIOC_READ_ONE_PCIE_EEPROM copy_to_user error\n"));
				IOCTL_UNLOCK(flags);
                return -EFAULT;
            }
            pdebug(("PcigIoctl : PCIGIOC_READ_ONE_PCIE_EEPROM Successfull\n"));
			break;

		case PCIGIOC_WRITE_ONE_PCIE_EEPROM:
            pdebug(("PcigIoctl : PCIGIOC_WRITE_ONE_PCIE_EEPROM\n"));
            err = copy_from_user((int*)&RegDes, (int*)arg, sizeof(REG_DES));
            if (err != 0)
            {
                pdebug(("PcigIoctl : PCIGIOC_WRITE_ONE_PCIE_EEPROM copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
                return -EFAULT;
            }
            err = OnPcigWritePcieEERegister(lpDev, RegDes.ulRegName, RegDes.ulRegValue);
            if(err != PCIG_SUCCESS)
            {
                pdebug(("PcigIoctl : PCIGIOC_WRITE_ONE_PCIE_EEPROM OnPcigWriteEERegister error\n"));
				IOCTL_UNLOCK(flags);
                return -EFAULT;
            }
            pdebug(("PcigIoctl : PCIGIOC_WRITE_ONE_PCIE_EEPROM Successfull\n"));
			break;

        case PCIGIOC_PROGRAM_LOGICAL_DESIGN:
            pdebug(("PcigIoctl : PCIGIOC_PROGRAM_LOGICAL_DESIGN\n"));
            err = copy_from_user((int*)&ConfigFpga, (int*)arg, sizeof(CONFIG_FPGA));
            if (err != 0)
            {
                pdebug(("PcigIoctl : PCIGIOC_PROGRAM_LOGICAL_DESIGN copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
                return -EFAULT;
            }
            lpszConfig = (char*)WrapVmalloc(ConfigFpga.ulConfigLen);
            err = copy_from_user((int*)lpszConfig, (int*)ConfigFpga.lpszConfig, ConfigFpga.ulConfigLen);
            if (err != 0)
            {
                pdebug(("PcigIoctl : PCIGIOC_PROGRAM_LOGICAL_DESIGN copy_from_user error\n"));
				IOCTL_UNLOCK(flags);
                return -EFAULT;
            }
            err = OnPcigProgramLogicalDesign(lpDev, lpszConfig, ConfigFpga.ulConfigLen);
            if(err != PCIG_SUCCESS)
            {
                pdebug(("PcigIoctl : PCIGIOC_PROGRAM_LOGICAL_DESIGN OnPcigLoadLogicalDesign error\n"));
				IOCTL_UNLOCK(flags);
                return -EFAULT;
            }
            WrapVfree(lpszConfig);
            pdebug(("PcigIoctl : PCIGIOC_PROGRAM_LOGICAL_DESIGN Successfull\n"));
            break;

		case PCIGIOC_GET_DEVICE_ID :
			pdebug(("PcigIoctl : PCIGIOC_GET_DEVICE_ID\n"));
			ulValue = OnPcigGetDeviceID(lpDev);
			err = copy_to_user( (int *)arg, &ulValue, sizeof(unsigned int));
			if (err != 0)
			{
			    pdebug(("PcigIoctl : PCIGIOC_GET_DEVICE_ID copy_to_user error"));
				IOCTL_UNLOCK(flags);
				return -EFAULT;
			}
			pdebug(("PcigIoctl : PCIGIOC_GET_DEVICE_ID Successfull\n"));
			break;

		default :
			pdebug(("PcigIoctl : Unknow ioctl\n"));
			IOCTL_UNLOCK(flags);
			return -ENOTTY;

	}
	IOCTL_UNLOCK(flags);
	return(0);
}

/********************************/
/* open handler for vm area     */
/********************************/
void mmap_drv_vopen(struct vm_area_struct * vma)
{
	pdebug(("mmap_drv_vopen\n"));
	OnPcigMmapOpen(vma);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,20)
	MOD_INC_USE_COUNT;
#endif
}

/********************************/
/* close handler for vm area    */
/********************************/
void mmap_drv_vclose(struct vm_area_struct * vma)
{
	pdebug(("mmap_drv_vclose\n"));
	OnPcigMmapClose(vma);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,20)
	MOD_DEC_USE_COUNT;
#endif
}

/********************************/
/* page fault handler 1         */
/********************************/
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,20)
struct page * mmap_drv_vmmap(struct vm_area_struct * vma, unsigned long address, int write_access)
#else
struct page * mmap_drv_vmmap(struct vm_area_struct * vma, unsigned long address, int* write_access)
#endif
{
	pdebug(("mmap_drv_vmmap\n"));
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,20)
		return(OnPcigMmapFaultHandler(vma, address, NULL));
#else
		return((struct page *)OnPcigMmapFaultHandler(vma, address, write_access));
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,1,0)
vm_fault_t
#else
int 
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,11,0)
    mmap_drv_vmmap_vmf( struct vm_area_struct *vma, struct vm_fault *vmf)
{
	return(OnPcigMmapFaultHandlerVmf(vma, vmf));
}
#else
    mmap_drv_vmmap_vmf(struct vm_fault *vmf)
{
	return(OnPcigMmapFaultHandlerVmf(vmf->vma, vmf));
}
#endif

int mmap_drv_access_vmf(struct vm_area_struct *vma, unsigned long addr, void *buf, int len, int write)
{
	return(OnPcigMmapAccessVmf(vma, addr, buf, len, write));
}
#endif

/********************************/
/* driver interrupt handler     */
/********************************/
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,20)
static void PcigIntHandler( int irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t PcigIntHandler( int irq, void *dev_id, struct pt_regs *regs)
#endif
{
    DEVICE_EXTENSION *  lpDev;
	unsigned long		itHandled = 0;

    lpDev = (DEVICE_EXTENSION*)dev_id;

	itHandled = PcigIntHandlerModule(lpDev);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,20)
	if (itHandled == 0)
		return;

	return;
#else
	if (itHandled == 0)
		return(IRQ_NONE);

	return(IRQ_HANDLED);
#endif
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,20)
void (*SkelGetInterruptHandler(void))( int, void *, struct pt_regs *)
#else
irqreturn_t (*SkelGetInterruptHandler(void))( int, void *, struct pt_regs *)
#endif
{
	return(PcigIntHandler);
}
