/****************************************************************************** 
 *                                                                            *
 * File:         pcie6509.c                                                   *
 *                                                                            *
 * Description:  Linux Host driver for PCIE6509 card. This driver is          *
 *               an installable module.                                       *
 *                                                                            *
 * Date:         02/13/2014                                                   *
 * History:                                                                   *
 *                                                                            *
 *  1  02/13/14  D. Dubash                                                    *
 *               Initial release                                              *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 *  Copyright (C) 2010 and beyond Concurrent Computer Corporation             *
 *  All rights reserved                                                       *
 *                                                                            *
 ******************************************************************************/

/* MODVERSION is redundant in RedHawk 2.1 as it is being include in the 
 * <module>.ko file. However, the <module>.o file still does not contain 
 * the module version check if MODVERSION has not been configured in the 
 * system. The Redhawk 2.1 is shipped with MODVERSION not configured in 
 * the system.
 */
#ifndef __KERNEL__
#define __KERNEL__
#endif

#ifndef __VERSION__
#define __VERSION__
#endif

#ifndef MODULE
#define MODULE
#endif

/**************************************************************************
 *  If module version support is required, the modversions.h module must  *
 *  be included before any other header files. To enable versioning in    *
 *  the module, if it has been enabled in the kernel, we must first make  *
 *  sure that CONFIG_MODVERSIONS has been defined in <linux/autoconf.h>.  *
 *  If kernel has defined conversion, we will force the driver to include *
 *  the <linux/modversions.h> file. The user could alternatively          *
 *  define the variable MODVERSIONS in the Makefile to also force         *
 *  module versions to be generated.                                      *
 **************************************************************************/
#if (defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS))
#define MODVERSIONS /* force module version on */
#endif

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
#include <generated/autoconf.h>
#ifdef MODVERSIONS
#include <config/modversions.h>
#endif
#else
#include <linux/autoconf.h>
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#endif

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/pci.h>
#include <linux/mm.h>
#include <linux/slab.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
#include <linux/sched/signal.h>
#include <linux/uaccess.h>
#else
#include <linux/sched.h>
#endif
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/ioctl.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/signal.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>

#include <asm/io.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,5,4)
#include <asm/system.h>
#endif
#include <asm/uaccess.h>
#include <asm/signal.h>
#include <asm/errno.h>

#include <linux/seq_file.h> /* procfs support */

#include "pcie6509_user.h"
#include "pcie6509.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
# define ioremap_nocache ioremap
#endif

/*** ALSO DEFINED IN pcie6509_user.h ***/
/*** BOARD SPECIFIC INFORMATION IN .../include/linux/pci_ids.h ***/

/*** THE ORDER OF ENTRIES IN THIS STRUCT MUST BE SAME AS ENTRIES IN
 *** 'enum pcie6509_type_index' in pcie6509_user.h FILE
 ***/
struct pcie6509_board_entry pcie6509_boards_supported[] = {
  /* device   subsystem_device       name           board_type */
   { 0x6509,  PCIE6509_DEVICE_ID_PCIE_6509,  "PCIE-N6509",   N6509_0}, 

   { -1,      -1,                    "null",            -1}  /* end of table */
};

/*** Define Board Regions ***/
#define LOCAL_REGION    REGION_0    /* LOCAL Register */

MODULE_LICENSE("Proprietary");
MODULE_DESCRIPTION("");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23)
static irqreturn_t device_interrupt(int, void *);
#else
static irqreturn_t device_interrupt (int, void *, struct pt_regs *);
#endif

static char *PCIE6509_FileName;

/*** prototype functions ***/
static int PCIE6509_init(void);
static void PCIE6509_exit(void);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static int device_open(struct inode *, struct file *);
static int device_close(struct inode *, struct file *);

int PCIE6509_configure_mem_regions(struct pcie6509_board *pcie6509_device,
                                 struct pci_dev *pdev, int which,
                                 u32 * address, u32 * size, u32 * flags);
void PCIE6509_deconfigure_mem_regions(struct pcie6509_board *pcie6509_device,
                                    int which, u32 address, u32 size,
                                    u32 flags);
int PCIE6509_allocate_physical_memory(struct pcie6509_board *pcie6509_device, int size);
void PCIE6509_free_physical_memory(struct pcie6509_board *pcie6509_device);
void PCIE6509_reset_board(struct pcie6509_board *pcie6509_device);
void PCIE6509_initialize_board(struct pcie6509_board *pcie6509_device);
int  _PCIE6509_Set_DigitalOutputPmask(pcie6509_local_ctrl_data_t *lp,
                                 pcie6509_shadow_regs_t *sp, pcie6509_io_port_t *iop);
u_int __PCIE6509_LineMaskReg(u_int cur, u_char ports_selected, u_char *line_mask, int *num_ports);
int _PCIE6509_Get_DigitalInputPmask(pcie6509_local_ctrl_data_t *lp, pcie6509_io_port_t *iop);
void __PCIE6509_GetLineMask(u_int cur, u_char ports_selected, u_char *line_mask, u_char *num_ports);

/*** local module data ***/
static struct file_operations device_fops = {
  open      :device_open,
  release   :device_close,
  read      :device_read,
  write     :device_write,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
  unlocked_ioctl:unlocked_device_ioctl,
#else
  ioctl     :device_ioctl,
#endif
  mmap      :device_mmap,
};

/*** local variables ***/
static __s8 version[8];
static __s8 built[32];

static int Ncie6509_Max_Region;

static int proc_enabled;
static int num_boards;
static int device_major = 0;    // device major number (dynamically assigned)
static struct pcie6509_board *boards = 0; // linked list of all boards found
static struct pcie6509_board *pcie6509_device_list[PCIE6509_MAX_BOARDS];

/* <<< IF NOT SUPPORTED, DISABLE OPERATION >>> */
#define WRITE_SUPPORTED 1 /* write: supported=1, not_supported=0 */

/* <<< DISABLE DUMMY_READ AND DUMMY_WRITE WHEN ACTUAL CODE ADDED >>> */
#define DUMMY_WRITE     0 /* write: dummy=1, actual=0 */
#define DUMMY_READ      0 /* read: dummy=1, actual=0 */
#if ((DUMMY_READ == 1) || (DUMMY_WRITE == 1))
    static char dummy_read_write[1024];
#endif /* (DUMMY_READ || DUMMY_WRITE) */

#define PCIE6509_CHECK_FOR_NULL_ARG(Desc)                       \
    if(arg == 0) {                                              \
        pcie6509_device->error = PCIE6509_INVALID_PARAMETER;    \
        DEBUGPx(D_ERR,                                          \
            "Parameter must be pointer to %s\n", Desc);         \
        ret_code = -EINVAL;                                     \
        goto Get_Out;                                           \
    }

#define PCIE6509_COPY_FROM_USER(Addr,Size) {                    \
    PCIE6509_UNLOCK();     /** Free the lock **/                \
    if(copy_from_user(Addr, (void *) arg, Size)) {              \
        pcie6509_device->error = PCIE6509_FAULT_ERROR;          \
        DEBUGP(D_ERR,"copy to user failed\n");                  \
        ret_code = -EFAULT;                                     \
        PCIE6509_LOCK();    /* Lock every thing */              \
        goto Get_Out;                                           \
    }                                                           \
    PCIE6509_LOCK();    /* Lock every thing */                  \
}

#define PCIE6509_COPY_TO_USER(Addr,Size) {                      \
    PCIE6509_UNLOCK();     /** Free the lock **/                \
    if(copy_to_user((void *) arg, Addr, Size)) {                \
        pcie6509_device->error = PCIE6509_FAULT_ERROR;          \
        DEBUGP(D_ERR,"copy to user failed\n");                  \
        ret_code = -EFAULT;                                     \
        PCIE6509_LOCK();    /* Lock every thing */              \
        goto Get_Out;                                           \
    }                                                           \
    PCIE6509_LOCK();    /* Lock every thing */                  \
}


/******************************************************************************
 ***                                                                        ***
 *** Device vmops open Handler                                              ***
 ***                                                                        ***
 ******************************************************************************/
void PCIE6509_vmops_open(struct vm_area_struct *vma)
{
    struct pcie6509_board *pcie6509_device =
        (struct pcie6509_board *) vma->vm_file->private_data;

    pcie6509_device = pcie6509_device;  /* get rid of warning */

    DEBUGPx(D_L2, "end= 0x%lx start=0x%lx size=0x%lx\n",
            vma->vm_end, vma->vm_start, (vma->vm_end - vma->vm_start));
}

/******************************************************************************
 ***                                                                        ***
 *** Device vmops close Handler                                             ***
 ***                                                                        ***
 ******************************************************************************/
void PCIE6509_vmops_close(struct vm_area_struct *vma)
{
    unsigned int size;
    struct pcie6509_board *pcie6509_device =
        (struct pcie6509_board *) vma->vm_file->private_data;

    DEBUGPx(D_L2, "end= 0x%lx start=0x%lx size=0x%lx\n",
            vma->vm_end, vma->vm_start, (vma->vm_end - vma->vm_start));

    size = vma->vm_end - vma->vm_start;

    pcie6509_device->phys_mem_size_freed += size;

    /* if all of the mapped memory is freed, we can go and clean up
     * the buffer
     */
    if (pcie6509_device->phys_mem_size_freed == pcie6509_device->phys_mem_size) {
        /* free physical memory */
        DEBUGP(D_L2, "##### FREEING MEMORY ######\n");
        PCIE6509_free_physical_memory(pcie6509_device);
    }
}

struct vm_operations_struct pcie6509_vm_ops = {
  open      :PCIE6509_vmops_open,
  close     :PCIE6509_vmops_close
};

static int  pcie6509_proc_show(struct seq_file *m, void *v);
static ssize_t  pcie6509_proc_write(struct file *file, const char __user *buf,
                                    size_t count, loff_t *data);

static int pcie6509_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, pcie6509_proc_show, NULL);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
static struct proc_ops pcie6509_proc_operations = {
        .proc_open           = pcie6509_proc_open,
        .proc_read           = seq_read,
        .proc_write          = pcie6509_proc_write,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
static struct file_operations pcie6509_proc_operations = {
        .owner          = THIS_MODULE,
        .open           = pcie6509_proc_open,
        .read           = seq_read,
        .write          = pcie6509_proc_write,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

/******************************************************************************
 ***                                                                        ***
 *** Device Timeout Handler                                                 ***
 ***                                                                        ***
 ******************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
void PCIE6509_timeout_handler(unsigned long ptr)
#else
void PCIE6509_timeout_handler(struct timer_list* ptr)
#endif
{
    struct pcie6509_board *pcie6509_device;

    pcie6509_device = (struct pcie6509_board *) ptr;

    pcie6509_device->timeout = TRUE;

    if(pcie6509_device->wakeup_pending == TRUE) {
       pcie6509_device->wakeup_pending = FALSE;
       pcie6509_device->wakeup = TRUE;
       wake_up(&pcie6509_device->intwq);
    }

    DEBUGP_ENTER;
    DEBUGP_EXIT;
}

/******************************************************************************
 *                                                                            *
 *    Function:    pcie6509_proc_show                                         *
 *                                                                            *
 *    Purpose:                                                                *
 *                                                                            *
 *        Implement the read service for the module's /proc file system       *
 *        entry. Read the documentation on this service for details.          *
 *                                                                            *
 *    Arguments:                                                              *
 *        m        pointer to the seq_file struct                             *
 *        v        void *                                                     *
 *                                                                            *
 *    Returned:                                                               *
 *        int      0                                                          *
 *                                                                            *
 ******************************************************************************/
/* procfs support */
static int pcie6509_proc_show(struct seq_file *m, void *v)
{
    int  bcnt;
    struct pcie6509_board *pcie6509_device;

    seq_printf(m,
                "version: %s\n"
                "built: %s\n"
                "boards: %d\n", PCIE6509_VERSION, built, num_boards);

#ifdef PCIE6509_DEBUG
    seq_printf(m, "%s: 0x%08x\n",D_PCIE6509_DEBUG_MASK, pcie6509_debug_mask);
    seq_printf(m, "%s: 0x%08x\n",D_PCIE6509_DEBUG_CTRL, pcie6509_debug_ctrl);
#endif /* PCIE6509_DEBUG */

    for (bcnt = 0; bcnt < num_boards; bcnt++) {
        pcie6509_device = pcie6509_device_list[bcnt];
        seq_printf(m, "  card=%d: [%02x:%02x.%1d] bus=%d, slot=%d, "
                      "func=%d, irq=%d, ID=%04x, Master/Slave Signature=0x%x/0x%x\n",
                bcnt, pcie6509_device->info.bus, pcie6509_device->info.slot,
                pcie6509_device->info.func, pcie6509_device->info.bus,
                pcie6509_device->info.slot, pcie6509_device->info.func,
                pcie6509_device->irqlevel, pcie6509_device->info.device,
                pcie6509_device->info.MasterSignature,
                pcie6509_device->info.SlaveSignature);
    }

    seq_puts(m,"\n");

    return 0;
}

/******************************************************************************
 *                                                                            *
 *    Function:    pcie6509_proc_write                                        *
 *                                                                            *
 *    Purpose:                                                                *
 *                                                                            *
 *        Implement the write service for the module's /proc file system      *
 *        entry. Read the documentation on this service for details, as       *
 *        we ignore some arguments according to our needs and the             *
 *        documentation.                                                      *
 *                                                                            *
 *    Arguments:                                                              *
 *        file     Point to file structure.                                   *
 *        buf      Records pointer to where the data in "page" begins.        *
 *        count    The limit to the amount of data we can write.              *
 *        data     A private data item that we may use, but don't.            *
 *                                                                            *
 *    Returned:                                                               *
 *        int      The number of characters written.                          *
 *                                                                            *
 ******************************************************************************/
static ssize_t  pcie6509_proc_write(struct file *file, const char __user *buf,
                               size_t count, loff_t *data)
{
    struct pcie6509_board *pcie6509_device = pcie6509_device_list[0];
    ssize_t ret = -ENOMEM;
    char *page, *p;
    int  tkn;

    if(count > PAGE_SIZE)
        return -EOVERFLOW;

    page = (char *)__get_free_page(GFP_KERNEL);
    if (page) {
        ret = -EFAULT;
        if (copy_from_user(page, buf, count))
            goto out;
        page[count-1]=0;
        DEBUGPx(D_L1, "input string:[%s] size:[%ld]\n",
                                                page, (unsigned long)count);

        tkn=0;
        p=page;
        while(*p == ' ')p++;    /* skip leading spaces */

        /* scan the token list for a match */
        while(Tokens[tkn].token != NULL) {
            if(strncmp(p,Tokens[tkn].token,strlen(Tokens[tkn].token)) == 0) {
                p+=strlen(Tokens[tkn].token);
                while(*p == ' ')p++;    /* skip leading spaces */
                if((*p == '=') || (*p == '\t')) p++;       /* skip '=' */
                while(*p == ' ')p++;    /* skip trailing spaces */

                switch(Tokens[tkn].which) {
#ifdef PCIE6509_DEBUG
                    case T_PCIE6509_DEBUG_MASK:
                    case T_PCIE6509_DEBUG_CTRL:
                        {
                            char *endp;
                            int  value;
                            value = simple_strtol(p,&endp,0);
                            if(*endp && (*endp != ' ')) {
                                ERRPx("Invalid argument [%s] for token '%s'\n",
                                                       p, Tokens[tkn].token);
                                goto out_inval;
                            }
                            if(Tokens[tkn].which == T_PCIE6509_DEBUG_MASK) {
                                pcie6509_debug_mask=value;
                                printk(KERN_INFO PCIE6509_DRIVER_NAME 
                                         ": %s=0x%08x (%d)\n",
                                      D_PCIE6509_DEBUG_MASK,
                                      pcie6509_debug_mask, pcie6509_debug_mask);
                            }
                            if(Tokens[tkn].which == T_PCIE6509_DEBUG_CTRL) {
                                pcie6509_debug_ctrl=value;
                                printk(KERN_INFO PCIE6509_DRIVER_NAME 
                                         ": %s=0x%08x (%d)\n",
                                      D_PCIE6509_DEBUG_CTRL,
                                      pcie6509_debug_ctrl, pcie6509_debug_ctrl);
                            }
                        }
                        ret = count;
                        goto out;
                    break;
#endif /* PCIE6509_DEBUG */

                    /* << PLACE ADDITIONAL TOKENS HERE >> */

                    default:
                        ERRPx
                           (": Internal Error!!! Token '%s' not implemented\n",
                               Tokens[tkn].token);
                        goto out_inval;
                    break;
                }
            }
            tkn++;
        }

        ERRPx("Invalid argument: [%s]\n",page);
out_inval:
        ret = -EINVAL;
    }
out:
    free_page((unsigned long)page);
    return ret;
}

/*******************************
 *     Board found routine     *
 *******************************/
static int
board_found(struct pci_dev *pdev, int *board_type)
{
    int i;
    struct pcie6509_board *pcie6509_device;
    unsigned short reg;

    pcie6509_device=0;  /* get rid of warnings if debug disabled */
    reg=0;              /* get rid of warnings if debug disabled */

    /* determine if this is one of the boards supported. */
    i = 0;
    while (pcie6509_boards_supported[i].subsystem_device != -1) {
        if (pcie6509_boards_supported[i].subsystem_device ==
                                                pdev->device) {
#ifdef PCIE6509_DEBUG
            DEBUGPx(D_L2, "found board %s type %d\n",
                     pcie6509_boards_supported[i].name, i);
            pci_read_config_word(pdev, 0x2E, &reg);
            DEBUGPx(D_L2, "pci_read_config_word reg=0x%x\n", reg);
#endif  /* PCIE6509_DEBUG */

            if(board_type)
                *board_type = pcie6509_boards_supported[i].board_type;

            return(TRUE);   /* board found */
        }
        i++;
    }

    return(FALSE);  /* no board found */
}

/******************************************************************************
 ***                                                                        ***
 *** Initialization Handler                                                 ***
 ***                                                                        ***
 ******************************************************************************/
static int PCIE6509_init(void)
{
    struct pcie6509_board *pcie6509_device = 0, *device_next = 0;
    struct pci_dev *pdev = NULL;
    int index = 0;
    int i, j;
    int board_type;
    int region_error;
    int slash = '/';
    int ResourceCount;

    PCIE6509_FileName = strrchr(__FILE__, slash);
    if (PCIE6509_FileName[0] == '/')  /* bump past slash */
        PCIE6509_FileName++;

    DEBUGP_ENTER;
    DEBUGPx(D_L1, "PCIE6509_FileName=%s\n", PCIE6509_FileName);

    DEBUGP(D_L1, "Copyright (C) 2006 Concurrent Computer Corporation\n");
    strcpy(version, PCIE6509_VERSION);
    sprintf(built, "%s, %s", __DATE__, __TIME__);
    DEBUGPx(D_L1, "driver version %s - built %s\n", version, built);

    /* We need to make sure that the PCIE6509_MAX_REGION size defined
     * in pcie6509_user.h is at least equal to the kernel enum variable
     * DEVICE_COUNT_RESOURCE. If not, fail the load until the driver is fixed.
     */
    if(PCIE6509_MAX_REGION < DEVICE_COUNT_RESOURCE) {
        ERRPx("Driver Internal Error!!! \nChange PCIE6509_MAX_REGION size (%d) "
            "to at least %d in pcie6509_user.h and recompile",
            PCIE6509_MAX_REGION, DEVICE_COUNT_RESOURCE);
        return (-ENOMEM);
     }

    /* Set to device count resource */ 
    Ncie6509_Max_Region = DEVICE_COUNT_RESOURCE;

    while ((pdev = pci_get_device(PCIE6509_VENDOR_ID, PCI_ANY_ID, pdev))) {

        if (pci_enable_device(pdev))
            continue;

        pcie6509_device = 0;
        if (board_found(pdev, &board_type) == TRUE) {

            pcie6509_device =
                (struct pcie6509_board *) kmalloc(sizeof(struct pcie6509_board),
                                            GFP_KERNEL);
            if (!pcie6509_device) {
                ERRPx("(%d): Unable to allocate memory\n", index);
                continue;
            }
            memset(pcie6509_device, 0, sizeof(struct pcie6509_board)); 
                                                            /* zero struct */
            pcie6509_device->minor = index; /* set early for DEBUG messages */
            DEBUGPx(D_L1,
                    "attaching board #%d device pointer 0x%p\n", index + 1,
                    pcie6509_device);
            pcie6509_device->info.board_type = board_type;
            strcpy(pcie6509_device->info.board_desc, 
                pcie6509_boards_supported[pcie6509_device->info.board_type].name);

            /* allocate space for shadow memory */
            pcie6509_device->shadow_reg = (pcie6509_shadow_regs_t *) __get_free_pages(GFP_KERNEL,
                                                     get_order(sizeof(pcie6509_shadow_regs_t)));
            if(pcie6509_device->shadow_reg == NULL) {
                ERRPx("(%d): Unable to allocate memory\n", index);
                kfree(pcie6509_device);
                pcie6509_device=NULL;
                continue;
            }

            pcie6509_device->phys_shadow_reg =
                    (u32 *)(unsigned long) virt_to_phys((void *) pcie6509_device->shadow_reg);
            /*
             * The bars may be either in I/O space or memory space.  Figure 
             * out which this board is and Deal with either.
             */
            ResourceCount = 0;

            region_error = 0;   /* set success flag */
            /*** mark all the memory regions in use ***/
            for (i = 0; i < Ncie6509_Max_Region; i++) {
                if (PCIE6509_configure_mem_regions(pcie6509_device, pdev, i,
                            &pcie6509_device->mem_region[i].physical_address, 
                            &pcie6509_device->mem_region[i].size,
                            &pcie6509_device->mem_region[i].flags)) {
                    DEBUGP(D_ERR,
                            "#### pcie6509_configure_mem_regions FAILED\n");
                    /*** If configuration failed, we need to remove the
                     *** memory requested
                     ***/
                    /* remove requested memory regions */
                    for (j = 0; j < Ncie6509_Max_Region; j++) {
                        PCIE6509_deconfigure_mem_regions(pcie6509_device, j,
                                                       
                            pcie6509_device->mem_region[j].physical_address,
                            pcie6509_device->mem_region[j].size,
                            pcie6509_device->mem_region[j].flags);
                    }

                    free_pages((unsigned long)pcie6509_device->shadow_reg,
                            get_order(sizeof(pcie6509_shadow_regs_t)));
                    pcie6509_device->shadow_reg=NULL;
                    pcie6509_device->phys_shadow_reg=NULL;

                    kfree(pcie6509_device);
                    pcie6509_device=NULL;

                    ERRPx("PCI Bus# %d, Device# %d.%d: Failed!!!\n",
                          pdev->bus->number,
                          PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn));
                    region_error++; /* mark error */
                    break;
                }
            }

            if (region_error)   /* if region failed, skip card */
                continue;

#ifdef      LOCAL_REGION
            {
                pcie6509_local_ctrl_data_t *local_reg_ptr;
                pcie6509_device->local_region = 
                                &pcie6509_device->mem_region[LOCAL_REGION];
    
                /* Now, check for presence of board */
                local_reg_ptr = (pcie6509_local_ctrl_data_t *)
                       pcie6509_device->local_region->virtual_address;
                DEBUGPx(D_L1,"local_reg_ptr=%p, board_id=0x%x\n",
                       local_reg_ptr, local_reg_ptr->CHInCh.Identification);

#ifdef PCIE6509_DEBUG
{
    u_int   subsys = local_reg_ptr->CHInCh.PCISubsystemID;

    printk("Board ID              0x%04x\n",local_reg_ptr->CHInCh.Identification);
    printk("Subsystem Product ID  0x%02x\n",(subsys >> PCIE6509_SUBPID_SHIFT) & PCIE6509_SUBPID_MASK);
    printk("Subsystem Vendor ID   0x%02x\n",(subsys >> PCIE6509_SUBVID_SHIFT) & PCIE6509_SUBVID_MASK);
    printk("Master Signature      0x%04x\n",local_reg_ptr->Master.CSSignature);
    printk("Slave Signature       0x%04x\n",local_reg_ptr->Slave.CSSignature);
}
#endif
                /* if Board Identification value is bad, error out */
                if ((local_reg_ptr == 0) || 
                       (local_reg_ptr->CHInCh.Identification != PCIE6509_IDENTIFICATION)) {
                    /* remove requested memory regions */
                    for (i = 0; i < Ncie6509_Max_Region; i++) {
                        PCIE6509_deconfigure_mem_regions(pcie6509_device, i,
                        pcie6509_device->mem_region[i].physical_address,
                        pcie6509_device->mem_region[i].size,
                        pcie6509_device->mem_region[i].flags);
                    }
                    free_pages((unsigned long)pcie6509_device->shadow_reg,
                            get_order(sizeof(pcie6509_shadow_regs_t)));
                    pcie6509_device->shadow_reg=NULL;
                    pcie6509_device->phys_shadow_reg=NULL;

                    kfree(pcie6509_device);
                    pcie6509_device=NULL;
    
                    ERRPx("PCI Bus# %d, Device# %d.%d: Failed!!!\n",
                        pdev->bus->number,
                        PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn));
                    continue;
                }
    
                /* <<< ADD CODE FOR  BOARD SPECIFIC INFORMATION HERE >>> */
                pcie6509_device->info.MasterSignature  = local_reg_ptr->Master.CSSignature;
                pcie6509_device->info.SlaveSignature   = local_reg_ptr->Slave.CSSignature;
                pcie6509_device->info.board_id  = local_reg_ptr->CHInCh.Identification;

                DEBUGPx(D_L2,"local_region: phys_addr=0x%x, virt_addr=%p\n",
                    pcie6509_device->local_region->physical_address,
                    pcie6509_device->local_region->virtual_address);

                /*** Resetting local registers ***/
                /* <<< ADD CODE TO RESET ANY LOCAL REGISTERS HERE >>> */
            }
#else       /* else LOCAL_REGION */
            pcie6509_device->local_region = NULL;
#endif      /* end LOCAL_REGION */

            init_waitqueue_head(&pcie6509_device->ioctlwq);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
            mutex_init(&pcie6509_device->ioctl_mtx);
#endif
            pcie6509_device->pdev = pdev;
            pci_dev_get(pcie6509_device->pdev);    /* increment resource count */

            pcie6509_device->irqlevel = pdev->irq;
            pcie6509_device->run_flags = 0;
            pcie6509_device->minor = index;
            pcie6509_device->next = boards;

            for (i = 0; i < Ncie6509_Max_Region; i++) {
                DEBUGPx(D_L1, "base_address[%d]=0x%lx\n", i,
                    (unsigned long)pcie6509_device->pdev->resource[i].start);
            }

            PCIE6509_LOCK_INIT(); /* Init the Spin Lock */

            boards = pcie6509_device;
            index++;

            pcie6509_device_list[num_boards]= pcie6509_device;

            pcie6509_device->info.bus       = pdev->bus->number; /* pci number */
            pcie6509_device->info.slot      = PCI_SLOT(pdev->devfn); /* slot number */
            pcie6509_device->info.func      = PCI_FUNC(pdev->devfn); /* function number */
            pcie6509_device->info.device_id = pdev->device;
            pcie6509_device->info.vendor_id = PCIE6509_VENDOR_ID;
            pcie6509_device->wakeup         = TRUE;
            pcie6509_device->info.device    = 
                pcie6509_boards_supported[pcie6509_device->info.board_type].device;

            strcpy(pcie6509_device->info.version, PCIE6509_VERSION);
            strcpy(pcie6509_device->info.built, __DATE__ ", " __TIME__);
            strcpy(pcie6509_device->info.module_name, PCIE6509_DRIVER_NAME);
            memcpy((char *)&pcie6509_device->info.mem_region,
                   (char *)&pcie6509_device->mem_region,
                   (sizeof(pcie6509_dev_region_t) * Ncie6509_Max_Region));

            /* reset the board */
            PCIE6509_reset_board(pcie6509_device);

            init_waitqueue_head(&pcie6509_device->intwq);

            num_boards++;

            printk(KERN_INFO PCIE6509_DRIVER_NAME ": /dev/%s%d: PCI Bus# %d, "
                   "Device# %d.%d\n",
                   PCIE6509_DRIVER_NAME, pcie6509_device->minor, pcie6509_device->info.bus,
                   pcie6509_device->info.slot, pcie6509_device->info.func);
        }
    }

    if (index == 0) {
        ERRP("No device found\n");
        return (-ENODEV);
    }

    device_major = register_chrdev(0, PCIE6509_DRIVER_NAME, &device_fops);
    if (device_major < 0) {
        /* OK, could not register -- undo the work we have done above */
        ERRP("could not register device number\n");
        for (pcie6509_device = boards; pcie6509_device;
             pcie6509_device = device_next) {
            device_next = pcie6509_device->next;

            /* remove requested memory regions */
            for (i = 0; i < Ncie6509_Max_Region; i++) {
                PCIE6509_deconfigure_mem_regions(pcie6509_device, i,
                            pcie6509_device->mem_region[i].physical_address,
                            pcie6509_device->mem_region[i].size,
                            pcie6509_device->mem_region[i].flags);
            }

            pci_dev_put(pcie6509_device->pdev);    /* decrement resource count */

            kfree(pcie6509_device);
            pcie6509_device=NULL;
        }

        boards = NULL;
        return (-ENODEV);
    }

    DEBUGPx(D_L1, "Number of devices %d\n", num_boards);
    /*
     *  Add /proc file system support.
     */

    if (num_boards) {
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,23)
    {
        struct proc_dir_entry*  entry;
        remove_proc_entry(PCIE6509_DRIVER_NAME, NULL);
        proc_enabled = 0;
        entry   = create_proc_entry(PCIE6509_DRIVER_NAME, S_IRUGO, NULL);
    
        if (entry) {
            proc_enabled = 1;
            //entry->read_proc  = _lcaio_proc_read;
            //entry->write_proc = _lcaio_proc_write;
            entry->proc_fops = &pcie6509_proc_operations;
        }
        else {
            DEBUGP(D_ERR,"Could not create /proc entry\n");
            DEBUGP_EXIT;
            return 0;
        }
    }
#else
        proc_enabled = 0;
        if(! proc_create(PCIE6509_DRIVER_NAME, S_IRUGO, NULL, &pcie6509_proc_operations)) {
            ERRP("Could not create /proc entry\n");
            PCIE6509_exit();
            return (-ENOMEM);
        }
        proc_enabled = 1;
#endif
    }

    printk(KERN_INFO PCIE6509_DRIVER_NAME
           ": driver version %s successfully inserted.\n", PCIE6509_VERSION);
    DEBUGP_EXIT;
    return 0;
}

/******************************************************************************
 ***                                                                        ***
 *** Exit Handler                                                           ***
 ***                                                                        ***
 ******************************************************************************/
static void PCIE6509_exit(void)
{
    struct pcie6509_board *pcie6509_device = 0, *device_next;
    int i;

    DEBUGP_ENTER;
    unregister_chrdev(device_major, PCIE6509_DRIVER_NAME);
    for (pcie6509_device = boards; pcie6509_device;
         pcie6509_device = device_next) {
        device_next = pcie6509_device->next;

        /* free physical memory if allocated */
        PCIE6509_free_physical_memory(pcie6509_device);

        /* remove requested memory regions */
        for (i = 0; i < Ncie6509_Max_Region; i++) {
            PCIE6509_deconfigure_mem_regions(pcie6509_device, i,
                        pcie6509_device->mem_region[i].physical_address,
                        pcie6509_device->mem_region[i].size,
                        pcie6509_device->mem_region[i].flags);
        }

        pci_dev_put(pcie6509_device->pdev);    /* decrement resource count */
        if(pcie6509_device->shadow_reg) {
            free_pages((unsigned long)pcie6509_device->shadow_reg,
                    get_order(sizeof(pcie6509_shadow_regs_t)));
            pcie6509_device->shadow_reg=NULL;
            pcie6509_device->phys_shadow_reg=NULL;
        }

        kfree(pcie6509_device);
        pcie6509_device=NULL;
    }
    boards = NULL;
    if (proc_enabled) {
        remove_proc_entry(PCIE6509_DRIVER_NAME, NULL);
        proc_enabled = 0;
    }

    printk(KERN_INFO PCIE6509_DRIVER_NAME
           ": driver version %s successfully removed.\n", PCIE6509_VERSION);
    DEBUGP_EXIT;
}

/******************************************************************************
 ***                                                                        ***
 *** Device Open Handler                                                    ***
 ***                                                                        ***
 ******************************************************************************/
static int device_open(struct inode *inode, struct file *fp)
{
    struct pcie6509_board *pcie6509_device = 0;
    int    request_irq_flag;

    DEBUGP_ENTER;
    for (pcie6509_device = boards; pcie6509_device;
         pcie6509_device = pcie6509_device->next) {
        DEBUGPx(D_L2, "pcie6509_device->minor=%d MINOR(inode->i_rdev)=%d\n",
                pcie6509_device->minor, MINOR(inode->i_rdev));
        if (MINOR(inode->i_rdev) == pcie6509_device->minor) {
            if (((fp->f_flags & O_APPEND)==0) && RFLAG(RF_OPEN_BUSY)) {
                pcie6509_device->error = PCIE6509_BUSY;
                DEBUGP(D_ERR, "board already opened\n");
                DEBUGP_EXIT;
                return (-EBUSY);
            }

            /* if already opened, return with device pointer */
            if(pcie6509_device->open_cnt) {
                pcie6509_device->open_cnt++;
                fp->private_data = pcie6509_device;
                return 0;
            }

            pcie6509_device->open_cnt = 1;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
            request_irq_flag = (/*IRQF_DISABLED |*/ IRQF_SHARED);
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23)
            request_irq_flag = (IRQF_DISABLED | IRQF_SHARED);
#else
            request_irq_flag = (SA_INTERRUPT | SA_SHIRQ);
#endif
#endif
#if 0 /* FOR NOW */
            if(pcie6509_device->irqlevel >= 0) {
                if (request_irq
                    (pcie6509_device->irqlevel, device_interrupt,
                     request_irq_flag, 
                    PCIE6509_DRIVER_NAME, pcie6509_device) < 0) {
                    DEBUGPx(D_ERR, "can not get interrupt %d\n",
                            pcie6509_device->irqlevel);
                    RFLAG_CLEAR(RF_OPEN_BUSY);  /* clear busy flag */
                    pcie6509_device->error = PCIE6509_BUSY;
                    DEBUGP_EXIT;
                    return (-EBUSY);
                }
                pcie6509_device->irq_added = TRUE;
            }
#endif

            /* Initialize variables in structure */
            RFLAG_SET(RF_OPEN_BUSY);    /* set busy flag */
            pcie6509_device->error = PCIE6509_SUCCESS;
            RFLAG_CLEAR(RF_INIT_PENDING);
            pcie6509_device->wakeup_pending = FALSE;   //default
            pcie6509_device->timeout = FALSE;   //default
            pcie6509_device->timeout_seconds = DEFAULT_TIMEOUT;   //default

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
            init_timer(&pcie6509_device->watchdog_timer);
            pcie6509_device->watchdog_timer.function =
                PCIE6509_timeout_handler;
            pcie6509_device->watchdog_timer.data =
                (unsigned long) pcie6509_device;
#else
            timer_setup(&pcie6509_device->watchdog_timer, PCIE6509_timeout_handler, 0);
#endif
            pcie6509_device->mmap_reg_select = PCIE6509_SELECT_LOCAL_MMAP;

            /* initialize queue heads */
            init_waitqueue_head(&pcie6509_device->ioctlwq);

            /* save structure address in fp */
            fp->private_data = pcie6509_device;

            /* Disable or Enable any interrupts here */

            if (!try_module_get(THIS_MODULE)) {
                if(pcie6509_device->irq_added == TRUE) {
                    free_irq(pcie6509_device->irqlevel, pcie6509_device);
                    pcie6509_device->irq_added = FALSE;
                }

                pcie6509_device->error = PCIE6509_BUSY;
                ERRP("Unable to increment the module count\n");
                DEBUGP_EXIT;
                return -EBUSY;
            }

            DEBUGP(D_L1, "Device open successful\n");
            DEBUGP_EXIT;
            return (0);
        }
    }
    ERRPx("(%d): can not find board\n", MINOR(inode->i_rdev));
    DEBUGP_EXIT;
    return (-ENODEV);
}

/******************************************************************************
 ***                                                                        ***
 *** Device Close Handler                                                   ***
 ***                                                                        ***
 ******************************************************************************/
static int device_close(struct inode *inode, struct file *fp)
{
    int dummy_read;
    struct pcie6509_board *pcie6509_device = (struct pcie6509_board *) fp->private_data;
    pcie6509_local_ctrl_data_t *local_reg_ptr;
    local_reg_ptr = (pcie6509_local_ctrl_data_t *)
                       pcie6509_device->local_region->virtual_address;

    DEBUGP_ENTER;

    /* <<< ABORT ANY DMA's HERE >>> */

    if(pcie6509_device->open_cnt > 1) {
        pcie6509_device->open_cnt--;
        return 0;
    }

    /* <<< DISABLE ANY INTERRUPTS HERE >>> */
#if 0 /* FOR NOW */
    local_reg_ptr->watchdog_sw_timeout_enable   = 0x00;
    local_reg_ptr->watchdog_timer_tout_interval = 0x00000000;
#endif

    local_reg_ptr->Master.CSGlobalInterruptEnable   = (PCIE6509_GIER_WTI_DISABLE |
                                                       PCIE6509_GIER_DII_DISABLE);
    local_reg_ptr->Slave.CSGlobalInterruptEnable    = (PCIE6509_GIER_WTI_DISABLE |
                                                       PCIE6509_GIER_DII_DISABLE);
    local_reg_ptr->Master.CSChangeDetectionIrq      = (PCIE6509_CDIR_CHANGE_DETECTION_ERROR_IRQ_DISABLE |
                                                       PCIE6509_CDIR_CHANGE_DETECTION_IRQ_DISABLE);
    local_reg_ptr->Slave.CSChangeDetectionIrq       = (PCIE6509_CDIR_CHANGE_DETECTION_ERROR_IRQ_DISABLE |
                                                       PCIE6509_CDIR_CHANGE_DETECTION_IRQ_DISABLE);
    local_reg_ptr->Master.CSWatchdogTimerInterrupt2 = (PCIE6509_WTI2R_WDT_TRIGGER_IRQ_DISABLE);
    local_reg_ptr->CHInCh.InterruptMask             = (PCIE6509_IMR_CLEAR_CPU_INT |
                                                       PCIE6509_IMR_CLEAR_STC3_INT);
    dummy_read                                      = local_reg_ptr->CHInCh.VolatileInterruptStatus;

    pcie6509_device->open_cnt = 0;

    /* free IRQ resource */
    if(pcie6509_device->irq_added == TRUE) {
        free_irq(pcie6509_device->irqlevel, pcie6509_device);
        pcie6509_device->irq_added = FALSE;
    }
        
    /* free physical memory if allocated */
    PCIE6509_free_physical_memory(pcie6509_device);

    RFLAG_CLEAR(RF_OPEN_BUSY);  /* clear busy flag */
    module_put(THIS_MODULE);    /* decrement count */

    DEBUGP_EXIT;
    return (0);
}

/******************************************************************************
 ***                                                                        ***
 *** Device WRITE Handler                                                   ***
 ***                                                                        ***
 ******************************************************************************/
static ssize_t device_write(struct file *fp, const char *buf, size_t size,
                            loff_t * lt)
{
    struct pcie6509_board *pcie6509_device =
                    (struct pcie6509_board *) fp->private_data;

    pcie6509_io_port_t          output;
    int                         size_returned = 0;
    pcie6509_local_ctrl_data_t  *local_reg_ptr;

    local_reg_ptr = (pcie6509_local_ctrl_data_t *)
                       pcie6509_device->local_region->virtual_address;

    DEBUGP_ENTER;

    TIME_STAMP(D_WTIME, timestamp_write_start);

    /* if no write size, return no bytes write */
    if(size == 0) {
        DEBUGP_EXIT;
        return (0);
    }

    if(size != sizeof(pcie6509_io_port_t)) {
        pcie6509_device->error = PCIE6509_INVALID_PARAMETER;
        DEBUGPx(D_ERR, "Invalid size %ld, expected %ld\n",size,
                                sizeof(pcie6509_io_port_t));
        TIME_STAMP(D_WTIME,timestamp_write_end);
        PRINT_TIME(D_WTIME,"",timestamp_write_start,timestamp_write_end,0);
        DEBUGP_EXIT;
        return (-EINVAL);
    }

    /* if user buffer cannot be read, error out */
    if(access_ok(
#ifdef VERIFY_WRITE
                VERIFY_READ,
#endif
                    (void *) buf, size) == 0 ) {
        pcie6509_device->error = PCIE6509_FAULT_ERROR;
        DEBUGP(D_ERR, "cannot read user buffer\n");
        TIME_STAMP(D_WTIME,timestamp_write_end);
        PRINT_TIME(D_WTIME,"",timestamp_write_start,timestamp_write_end,0);
        DEBUGP_EXIT;
        return (-EFAULT);
    }

    if(RFLAG(RF_WRITE_BUSY)) {    /* test write busy */
        pcie6509_device->error = PCIE6509_BUSY;
        DEBUGP(D_ERR,"write already in progress\n");
        TIME_STAMP(D_WTIME,timestamp_write_end);
        PRINT_TIME(D_WTIME,"",timestamp_write_start,timestamp_write_end,0);
        DEBUGP_EXIT;
        return (-EBUSY);
    }

    PCIE6509_LOCK();             /** Set the lock **/
    RFLAG_SET(RF_WRITE_BUSY);    /* set write busy */

    memset(&output, 0, sizeof(output)); /* zero out structure */

    /* first get user input */
    if(__copy_from_user(&output, buf, sizeof(output))) {
        DEBUGP(D_ERR,"copy to user failed\n");
        TIME_STAMP(D_WTIME,timestamp_write_end);
        PRINT_TIME(D_WTIME," ",timestamp_write_start,timestamp_write_end,0);
        DEBUGP_EXIT;
        PCIE6509_UNLOCK(); /** Unlock **/
        return (-EFAULT);
    }

    size_returned = _PCIE6509_Set_DigitalOutputPmask(local_reg_ptr,
                                                     pcie6509_device->shadow_reg,
                                                     &output);

    DEBUGPx(D_L3,"Bytes written=%d\n",size_returned);
    RFLAG_CLEAR(RF_WRITE_BUSY);  /* clear write busy */
    TIME_STAMP(D_WTIME,timestamp_write_end);
    PRINT_TIME(D_WTIME,"",timestamp_write_start,timestamp_write_end,0);
    DEBUGP_EXIT;
    PCIE6509_UNLOCK(); /** Unlock **/
    return (size_returned);
}

int
_PCIE6509_Set_DigitalOutputPmask(pcie6509_local_ctrl_data_t *lp,
                                 pcie6509_shadow_regs_t *sp, pcie6509_io_port_t *iop)
{
    u_int                       cur;
    u_char                      ports_selected;
    int                         num_ports=0;

    /* There is smarts built into this routine to speed it up:
     * 1) only perform a single register write for all ports within the same register
     */

    /* Ports 0 ... 3 */
    ports_selected = ((iop->port_mask>>0) & 0xF);  /* select ports 0 ... 3 */
    if(ports_selected) {  /* Skip if none of the ports 0 ... 3 are selected */
        cur = sp->Master.DioPortsStaticDigitalOutput;    /* read last shadow state */
        lp->Master.DioPortsStaticDigitalOutput = 
        sp->Master.DioPortsStaticDigitalOutput = 
                __PCIE6509_LineMaskReg(cur,ports_selected,&iop->line_mask[0],&num_ports);
    }

    /* Ports 4 ... 5 */
    ports_selected = ((iop->port_mask>>4) & 0x3);  /* select ports 4 ... 5 */
    if(ports_selected) {  /* Skip if none of the ports 4 ... 5 are selected */
        cur = sp->Master.PFIStaticDigitalOutput; /* read last shadow state */
        lp->Master.PFIStaticDigitalOutput = 
        sp->Master.PFIStaticDigitalOutput = 
                __PCIE6509_LineMaskReg(cur,ports_selected,&iop->line_mask[4],&num_ports);
    }

    /* Ports 6 ... 7 */
    ports_selected = ((iop->port_mask>>6) & 0x3);  /* select ports 6 ... 7 */
    if(ports_selected) {  /* Skip if none of the ports 6 ... 7 are selected */
        cur = sp->Slave.PFIStaticDigitalOutput; /* read last shadow state */
        lp->Slave.PFIStaticDigitalOutput = 
        sp->Slave.PFIStaticDigitalOutput = 
                __PCIE6509_LineMaskReg(cur,ports_selected,&iop->line_mask[6],&num_ports);
    }

    /* Ports 8 ... 11 */
    ports_selected = ((iop->port_mask>>8) & 0xF);  /* select ports 8 ... 11 */
    if(ports_selected) {  /* Skip if none of the ports 8 ... 11 are selected */
        cur = sp->Slave.DioPortsStaticDigitalOutput; /* read last shadow state */
        lp->Slave.DioPortsStaticDigitalOutput = 
        sp->Slave.DioPortsStaticDigitalOutput = 
                __PCIE6509_LineMaskReg(cur,ports_selected,&iop->line_mask[8],&num_ports);
    }

    return(num_ports);
}

u_int
__PCIE6509_LineMaskReg(u_int cur, u_char ports_selected, u_char *line_mask, int *num_ports) 
{
    int     i;
    u_int   line_mask_reg=cur;

    for(i=0;i<4;i++) {
        if(ports_selected & 1) {
            line_mask_reg &= ~(0xFF << (i*8));
            line_mask_reg |= (line_mask[i] << (i*8));
            (*num_ports)++;
        }
        ports_selected >>= 1;   /* shift out port */
    }

    return(line_mask_reg);
}

/******************************************************************************
 ***                                                                        ***
 *** Device READ Handler                                                    ***
 ***                                                                        ***
 ******************************************************************************/
static ssize_t device_read(struct file *fp, char *buf, size_t size,
                           loff_t * lt)
{
    struct pcie6509_board *pcie6509_device =
                        (struct pcie6509_board *) fp->private_data;

    pcie6509_io_port_t          input;
    int                         size_returned = 0;
    pcie6509_local_ctrl_data_t  *local_reg_ptr;

    local_reg_ptr = (pcie6509_local_ctrl_data_t *)
                       pcie6509_device->local_region->virtual_address;

    DEBUGP_ENTER;

    TIME_STAMP(D_RTIME, timestamp_read_start);

    /* if no read size, return no bytes read */
    if(size == 0) {
        DEBUGP_EXIT;
        return (0);
    }

    if(size != sizeof(pcie6509_io_port_t)) {
        pcie6509_device->error = PCIE6509_INVALID_PARAMETER;
        DEBUGPx(D_ERR, "Invalid size %ld, expected %ld\n",size,
                                sizeof(pcie6509_io_port_t));
        TIME_STAMP(D_RTIME,timestamp_read_end);
        PRINT_TIME(D_RTIME,"",timestamp_read_start,timestamp_read_end,0);
        DEBUGP_EXIT;
        return (-EINVAL);
    }

    /* if user buffer cannot be read, error out */
    if(access_ok(
#ifdef VERIFY_WRITE
                VERIFY_READ,
#endif
                (void *) buf, size) == 0 ) {
        pcie6509_device->error = PCIE6509_FAULT_ERROR;
        DEBUGP(D_ERR, "cannot read user buffer\n");
        TIME_STAMP(D_RTIME,timestamp_read_end);
        PRINT_TIME(D_RTIME," ",timestamp_read_start,timestamp_read_end,0);
        DEBUGP_EXIT;
        return (-EFAULT);
    }

    /* if buffer cannot be written to, error out */
    if(access_ok(
#ifdef VERIFY_WRITE
                    VERIFY_WRITE, 
#endif
                    (void *) buf, size) == 0 ) {
        pcie6509_device->error = PCIE6509_FAULT_ERROR;
        DEBUGP(D_ERR, "cannot write to user buffer\n");
        TIME_STAMP(D_RTIME,timestamp_read_end);
        PRINT_TIME(D_RTIME," ",timestamp_read_start,timestamp_read_end,0);
        DEBUGP_EXIT;
        return (-EFAULT);
    }

    if(RFLAG(RF_READ_BUSY)) {    /* test read busy */
        pcie6509_device->error = PCIE6509_BUSY;
        DEBUGP(D_ERR,"read already in progress\n");
        TIME_STAMP(D_RTIME,timestamp_read_end);
        PRINT_TIME(D_RTIME," ",timestamp_read_start,timestamp_read_end,0);
        DEBUGP_EXIT;
        return (-EBUSY);
    }

    if (pcie6509_device->ioctl_processing) {
        DEBUGP(D_ERR,"ioctl processing active: busy\n");
        DEBUGP_EXIT;
        return (-EBUSY);
    }

    PCIE6509_LOCK();            /** Set the lock **/
    RFLAG_SET(RF_READ_BUSY);    /* set read busy */

    memset(&input, 0, sizeof(input)); /* zero out structure */

    /* first get user input */
    if(__copy_from_user(&input, buf, sizeof(input))) {
        DEBUGP(D_ERR,"copy to user failed\n");
        TIME_STAMP(D_RTIME,timestamp_read_end);
        PRINT_TIME(D_RTIME," ",timestamp_read_start,timestamp_read_end,0);
        DEBUGP_EXIT;
        PCIE6509_UNLOCK(); /** Unlock **/
        return (-EFAULT);
    }

    if(input.port_mask & PCIE6509_ALL_PORTS_MASK) {  /* skip if no mask is selected */
        size_returned = _PCIE6509_Get_DigitalInputPmask(local_reg_ptr, &input);

        if(__copy_to_user(buf, &input, sizeof(input))) {
            pcie6509_device->error = PCIE6509_FAULT_ERROR;
            DEBUGP(D_ERR,"copy to user failed\n");
            TIME_STAMP(D_RTIME,timestamp_read_end);
            PRINT_TIME(D_RTIME," ",timestamp_read_start,timestamp_read_end,0);
            DEBUGP_EXIT;
            PCIE6509_UNLOCK(); /** Unlock **/
            return (-EFAULT);
        }
    }

    DEBUGPx(D_L3,"Bytes read=%d\n",size_returned);
    RFLAG_CLEAR(RF_READ_BUSY);  /* clear read busy */
    TIME_STAMP(D_RTIME,timestamp_read_end);
    PRINT_TIME(D_RTIME," ",timestamp_read_start,timestamp_read_end,0);
    DEBUGP_EXIT;
    PCIE6509_UNLOCK(); /** Unlock **/
    return (size_returned);
}   

int
_PCIE6509_Get_DigitalInputPmask(pcie6509_local_ctrl_data_t *lp, pcie6509_io_port_t *iop)
{
    u_char                      ports_selected;
    u_int                       cur;
    u_char                      num_ports=0;

    /* Ports 0 ... 3 */
    ports_selected = ((iop->port_mask>>0) & 0xF);  /* select ports 0 ... 3 */
    if(ports_selected) {  /* Skip if none of the ports 0 ... 3 are selected */
        cur = lp->Master.DioPortsStaticDigitalInput;    /* read current input state */
        __PCIE6509_GetLineMask(cur,ports_selected, &iop->line_mask[0], &num_ports);
    }

    /* Ports 4 ... 5 */
    ports_selected = ((iop->port_mask>>4) & 0x3);  /* select ports 4 ... 5 */
    if(ports_selected) {  /* Skip if none of the ports 4 ... 5 are selected */
        cur = lp->Master.PFIStaticDigitalInput; /* read current input state */
        __PCIE6509_GetLineMask(cur,ports_selected, &iop->line_mask[4], &num_ports);
    }

    /* Ports 6 ... 7 */
    ports_selected = ((iop->port_mask>>6) & 0x3);  /* select ports 6 ... 7 */
    if(ports_selected) {  /* Skip if none of the ports 6 ... 7 are selected */
        cur = lp->Slave.PFIStaticDigitalInput; /* read current input state */
        __PCIE6509_GetLineMask(cur,ports_selected, &iop->line_mask[6], &num_ports);
    }

    /* Ports 8 ... 11 */
    ports_selected = ((iop->port_mask>>8) & 0xF);  /* select ports 8 ... 11 */
    if(ports_selected) {  /* Skip if none of the ports 8 ... 11 are selected */
        cur = lp->Slave.DioPortsStaticDigitalInput; /* read current input state */
        __PCIE6509_GetLineMask(cur,ports_selected, &iop->line_mask[8], &num_ports);
    }

    return(num_ports);
}

void
__PCIE6509_GetLineMask(u_int cur, u_char ports_selected, u_char *line_mask, u_char *num_ports)
{
    int i;

    for(i=0;i<4;i++) {
        if(ports_selected & 1) {
            line_mask[i] = (cur & 0xFF);
            (*num_ports)++;
        }
        ports_selected >>= 1;   /* shift out port */
        cur >>= 8;              /* shift out next line mask */
    }
}


/******************************************************************************
 ***                                                                        ***
 *** Device Interrupt Handler                                               ***
 ***                                                                        ***
 ******************************************************************************/
static irqreturn_t
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23)
device_interrupt(int irq, void *dev_id)
#else
device_interrupt (int irq, void * dev_id, struct pt_regs *regs)
#endif
{
#if 0 /* FOR NOW */
    pcie6509_local_ctrl_data_t *local_reg_ptr;
    u_int                    mask;
    u_int                    status;
    u_int                    pending;
    u_int                    clear;
    u_int                    dummy_read;

    struct pcie6509_board *pcie6509_device = (struct pcie6509_board *) dev_id;
    local_reg_ptr = (pcie6509_local_ctrl_data_t *)
                       pcie6509_device->local_region->virtual_address;

    PCIE6509_LOCK(); /** Set the lock **/

    DEBUGP_ENTER;

    mask    = local_reg_ptr->master_interrupt_control;
    status  = local_reg_ptr->change_status;
    pending = mask & status & PCIE6509_ALL_MASTER_INT;

    /* if it is not our interrupt, return */ 
    if(pending == 0) {
        PCIE6509_UNLOCK(); /** Unlock **/
        return IRQ_NONE;
    }

//printk("mask=0x%04x status=0x%04x pending=0x%04x\n",mask,status,pending);
    clear = 0;
    if(status & mask & PCIE6509_WDT_EXP_INT)
        clear |= PCIE6509_CLEAR_INTERRUPT_WDT;

    if(status & mask & (PCIE6509_FALLING_EDGE_INT|PCIE6509_RISING_EDGE_INT|PCIE6509_EDGE_INT))
        clear |= PCIE6509_CLEAR_EDGE;

    if(status & mask & PCIE6509_OVERFLOW_INT)
        clear |= (PCIE6509_CLEAR_OVERFLOW | PCIE6509_CLEAR_EDGE);

//printk("clear=0x%x\n",clear);

    /* clear interrupt states */
    if(clear) {
        local_reg_ptr->clear    =   clear;
        /* issue dummy status read to get rid of following spurious interrupt */
        dummy_read = local_reg_ptr->change_status;
    }

    pcie6509_device->info.interrupt.count++;    /* increment interrupt count */
    pcie6509_device->info.interrupt.mask       = mask;
    pcie6509_device->info.interrupt.status     = status;
    pcie6509_device->info.interrupt.pending    = pending;

    if(pcie6509_device->wakeup_pending == TRUE) {
       pcie6509_device->wakeup_pending = FALSE;
       pcie6509_device->wakeup = TRUE;
       wake_up(&pcie6509_device->intwq);
    }

    DEBUGP_EXIT;

    PCIE6509_UNLOCK(); /** Unlock **/
#endif
    return IRQ_HANDLED;
}

#define IOCTL_RETURN(Code) {         \
    pcie6509_device->ioctl_processing = 0; \
    DEBUGP_EXIT;                \
    PCIE6509_UNLOCK();                     \
    return (Code);              \
}   

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
long unlocked_device_ioctl(struct file *filp,u_int iocmd,unsigned long ioarg )
{
    int ret;
    struct inode *inode;
    struct pcie6509_board *pcie6509_device = (struct pcie6509_board *) 
                    filp->private_data;

    DEBUGP_ENTER;
    DEBUGPx(D_L2, "device pointer: 0x%p\n", pcie6509_device);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
    inode = filp->f_dentry->d_inode;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
    inode = filp->f_path.dentry;
#else
    inode = filp->f_path.dentry->d_inode;
#endif

    mutex_lock(&pcie6509_device->ioctl_mtx);
    if(pcie6509_device->ioctl_processing) {
        DEBUGP(D_ERR,"ioctl processing active: busy\n");
        DEBUGP_EXIT;
        mutex_unlock(&pcie6509_device->ioctl_mtx);
        return (-EBUSY);    /* DO NOT USE IOCTL_RETURN() CALL HERE */
    }
    mutex_unlock(&pcie6509_device->ioctl_mtx);

    ret = device_ioctl(inode, filp, iocmd, ioarg);

    return( (long) ret );
}
#endif

/******************************************************************************
 ***                                                                        ***
 *** Device Ioctl Handler                                                   ***
 ***                                                                        ***
 ******************************************************************************/
int device_ioctl(struct inode *inode, struct file *fp, unsigned int num,
                 unsigned long arg)
{
    struct pcie6509_board *pcie6509_device = 
                            (struct pcie6509_board *) fp->private_data;
    unsigned long           bindaddr, bindoffset=0;
    pcie6509_phys_mem_t        pcie6509_phys_mem;
    pcie6509_mmap_select_t     pcie6509_mmap_select;
    pcie6509_user_error_t      pcie6509_user_error;
    pcie6509_driver_int_t      driver_int;

    int                     ret_code, index;
    u_int64_t               user_val;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
    if(pcie6509_device->ioctl_processing) {
        DEBUGP(D_ERR,"ioctl processing active: busy\n");
        DEBUGP_EXIT;
        return (-EBUSY);    /* DO NOT USE IOCTL_RETURN() CALL HERE */
    }
#endif

    PCIE6509_LOCK();    /* Lock every thing */
    pcie6509_device->ioctl_processing++;

    DEBUGP_ENTER;
    DEBUGPx(D_L2, "device pointer: 0x%p\n", pcie6509_device);
    ret_code = 0;       /* assume good return */

    switch (num) {

    case IOCTL_PCIE6509_ADD_IRQ:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_ADD_IRQ\n");
        if (pcie6509_device->irq_added == FALSE) {
            if(pcie6509_device->irqlevel >= 0) {
                /*** add irq ***/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
                if (request_irq
                    (pcie6509_device->irqlevel, device_interrupt, 
                       IRQF_SHARED, PCIE6509_DRIVER_NAME, pcie6509_device) < 0) {
#else
                if (request_irq
                    (pcie6509_device->irqlevel, device_interrupt, IRQF_DISABLED
                       | IRQF_SHARED, PCIE6509_DRIVER_NAME, pcie6509_device) < 0) {
#endif
                    DEBUGPx(D_ERR, "can not get interrupt %d\n",
                            pcie6509_device->irqlevel);
                    DEBUGP_EXIT;
                    ret_code = -EBUSY;
                    goto Get_Out;
                }
                pcie6509_device->irq_added = TRUE;
            }

            /*** enable interrupts ***/
            /* <<< ADD CODE TO ENABLE INTERRUPTS HERE >>> */
        }
        break;

    case IOCTL_PCIE6509_DISABLE_PCI_INTERRUPTS:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_DISABLE_PCI_INTERRUPTS\n");

        /* <<< ADD CODE TO DISABLE PCI INTERRUPTS >>> */

        DEBUGP_EXIT;
        break;

    case IOCTL_PCIE6509_ENABLE_PCI_INTERRUPTS:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_ENABLE_PCI_INTERRUPTS\n");

        /* <<< ADD CODE TO ENABLE PCI INTERRUPTS >>> */

        DEBUGP_EXIT;
        break;

    case IOCTL_PCIE6509_WAIT_FOR_INTERRUPT:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_WAIT_FOR_INTERRUPT\n");
        pcie6509_device->wakeup = FALSE;
        pcie6509_device->timeout= FALSE;
        pcie6509_device->watchdog_timer.expires =
            jiffies + pcie6509_device->timeout_seconds * HZ;

        if(pcie6509_device->timeout_seconds)
            add_timer(&pcie6509_device->watchdog_timer);

        pcie6509_device->wakeup_pending= TRUE;
        pcie6509_device->ioctl_processing=0;
        PCIE6509_UNLOCK();    /* UnLock */
        wait_event_interruptible(pcie6509_device->intwq,
                                            pcie6509_device->wakeup);
        PCIE6509_LOCK();    /* Lock */
        pcie6509_device->ioctl_processing=1;

        if(pcie6509_device->timeout) {
            pcie6509_device->error = PCIE6509_TIMEOUT;
            ret_code = -ETIME;
            goto Get_Out;
        } else
            del_timer_sync(&pcie6509_device->watchdog_timer);

        if(arg) {
            driver_int.count =
                pcie6509_device->info.interrupt.count;
            driver_int.mask =
                pcie6509_device->info.interrupt.mask;
            driver_int.status =
                pcie6509_device->info.interrupt.status;
            driver_int.pending =
                pcie6509_device->info.interrupt.pending;

            if(copy_to_user((void *) arg, &driver_int,
                                   sizeof(pcie6509_driver_int_t))) {
                    pcie6509_device->error = PCIE6509_FAULT_ERROR;
                    DEBUGP(D_ERR,"copy to user failed\n");
                    ret_code = -EFAULT;
                    goto Get_Out;
            }
        }

        DEBUGP_EXIT;
        break;

    case IOCTL_PCIE6509_WAKE_INTERRUPT:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_WAKE_INTERRUPT\n");

        if(pcie6509_device->wakeup_pending) {
            pcie6509_device->wakeup = TRUE;
            wake_up(&pcie6509_device->intwq);
        }

        DEBUGP_EXIT;
        break;

    case IOCTL_PCIE6509_GET_DRIVER_ERROR:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_GET_DRIVER_ERROR\n");

        if(arg) {
            index=0;
            while(pcie6509_device_error[index].name) {
                if(pcie6509_device_error[index].error == pcie6509_device->error)
                    break;
                index++;
            }

            /* if error found, return to user */
            if(pcie6509_device_error[index].name) {
                pcie6509_user_error.error = pcie6509_device->error;
                strncpy(pcie6509_user_error.name,
                    pcie6509_device_error[index].name,PCIE6509_ERROR_NAME_SIZE);
                strncpy(pcie6509_user_error.desc,
                    pcie6509_device_error[index].desc,PCIE6509_ERROR_DESC_SIZE);
                /* Copy the structure to the user */
                if(copy_to_user((void *) arg, &pcie6509_user_error, 
                                       sizeof(pcie6509_user_error_t))) {
                    pcie6509_device->error = PCIE6509_FAULT_ERROR;
                    DEBUGP(D_ERR,"copy to user failed\n");
                    ret_code = -EFAULT;
                    goto Get_Out;
                }
            } else {    /* else, unknown error */
                pcie6509_user_error.error = pcie6509_device->error;
                sprintf(pcie6509_user_error.name,"0x%x",pcie6509_device->error);
                strncpy(pcie6509_user_error.desc, "Unknown Driver Error!!!",
                                        PCIE6509_ERROR_DESC_SIZE);
            }
            
        } else {
            pcie6509_device->error = PCIE6509_SUCCESS;
        }
        break;

    case IOCTL_PCIE6509_GET_DRIVER_INFO:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_GET_DRIVER_INFO\n");
            
        PCIE6509_CHECK_FOR_NULL_ARG("pcie6509_driver_info_t");

        /* Copy the structure to the user */
        PCIE6509_COPY_TO_USER(&pcie6509_device->info,
                                    sizeof(pcie6509_device->info));
        break;

    case IOCTL_PCIE6509_GET_PHYSICAL_MEMORY:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_GET_PHYSICAL_MEMORY\n");

        PCIE6509_CHECK_FOR_NULL_ARG("pcie6509_phys_mem_t");

        /* If pointer to structure is supplied, return physical DMA memory */
        PCIE6509_COPY_FROM_USER(&pcie6509_phys_mem,
                                    sizeof(pcie6509_phys_mem_t));

        /* if no physical memory allocated */
        if (pcie6509_device->phys_mem == 0) {
            pcie6509_device->error = PCIE6509_RESOURCE_ALLOCATION_ERROR;
            DEBUGP(D_ERR,
                    "Physical memory not allocated. Do mmap()\n");
            ret_code = -ENOMEM;
            goto Get_Out;
        }

        pcie6509_phys_mem.phys_mem = pcie6509_device->phys_mem;
        pcie6509_phys_mem.phys_mem_size = pcie6509_device->phys_mem_size;

        PCIE6509_COPY_TO_USER(&pcie6509_phys_mem,
                                sizeof(pcie6509_phys_mem_t));
        break;

    case IOCTL_PCIE6509_INIT_BOARD:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_INIT_BOARD\n");

        PCIE6509_initialize_board(pcie6509_device);  /* initialize the board */
        PCIE6509_reset_board(pcie6509_device);  /* reset the board */
        break;

    case IOCTL_PCIE6509_GET_INTERRUPT_COUNTER:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_GET_INTERRUPT_COUNTER\n");
        PCIE6509_CHECK_FOR_NULL_ARG("u_int64_t");
        PCIE6509_COPY_TO_USER(&pcie6509_device->info.interrupt.count,
                                sizeof(u_int64_t));
        break;

    case IOCTL_PCIE6509_SET_INTERRUPT_COUNTER:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_SET_INTERRUPT_COUNTER\n");
        PCIE6509_CHECK_FOR_NULL_ARG("u_int64_t");
        PCIE6509_COPY_FROM_USER(&user_val, sizeof(u_int64_t));

        pcie6509_device->info.interrupt.count = user_val;
        break;

    case IOCTL_PCIE6509_GET_TIMEOUT:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_GET_TIMEOUT\n");
        PCIE6509_CHECK_FOR_NULL_ARG("int");
        PCIE6509_COPY_TO_USER(&pcie6509_device->timeout_seconds, sizeof(int));
        break;

    case IOCTL_PCIE6509_SET_TIMEOUT:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_SET_TIMEOUT\n");
        PCIE6509_CHECK_FOR_NULL_ARG("int");
        PCIE6509_COPY_FROM_USER(&user_val, sizeof(int));

        pcie6509_device->timeout_seconds = user_val;
        break;

    case IOCTL_PCIE6509_MMAP_SELECT: 
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_MMAP_SELECT ...entry\n");

        PCIE6509_CHECK_FOR_NULL_ARG("pcie6509_mmap_select_t");

        PCIE6509_COPY_FROM_USER(&pcie6509_mmap_select, 
                                sizeof(pcie6509_mmap_select_t));

        switch(pcie6509_mmap_select.select) {
            case PCIE6509_SELECT_LOCAL_MMAP:
                if(pcie6509_device->local_region == NULL) {
                    DEBUGP(D_ERR,"Local Region Not Present\n");
                    ret_code = -ENOMEM;
                    goto Get_Out;
                }
                pcie6509_mmap_select.size = PCIE6509_LOCAL_MMAP_SIZE;   
                        /* return size to user */
                bindaddr = (pcie6509_device->local_region->physical_address
                                                            / PAGE_SIZE)
                    * PAGE_SIZE;
                bindoffset = pcie6509_device->local_region->physical_address 
                                                            - bindaddr;
            break;

            case PCIE6509_SELECT_PHYS_MEM_MMAP:
                /* just return physical memory allocated size */
                pcie6509_mmap_select.size = pcie6509_device->phys_mem_size;;   
                bindoffset = 0;
            break;

            case PCIE6509_SELECT_SHADOW_REG_MMAP:
                /* just return physical shadow memory allocated size */
                pcie6509_mmap_select.size = sizeof(pcie6509_shadow_regs_t);   
                bindoffset = 0;
            break;

            default:
                /* Invalid argument */
                DEBUGPx(D_ERR,"Invalid Selection: %d\n",
                                            pcie6509_mmap_select.select);
                ret_code = -EINVAL;
            break;
        }

        if(ret_code != 0) {
            pcie6509_device->error = PCIE6509_INVALID_PARAMETER;
            goto Get_Out;
        }

        /* tell driver what user has selected */
        pcie6509_device->mmap_reg_select = pcie6509_mmap_select.select;

        /* return offset to user */
        pcie6509_mmap_select.offset = bindoffset;

        PCIE6509_COPY_TO_USER(&pcie6509_mmap_select, sizeof(pcie6509_mmap_select_t));

        break;

    case IOCTL_PCIE6509_NO_COMMAND:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_NO_COMMAND\n");
        break;

    case IOCTL_PCIE6509_REMOVE_IRQ:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_REMOVE_IRQ\n");
        if (pcie6509_device->irq_added == TRUE) {

            /*** remove interrupts ***/
            /* <<< ADD CODE TO REMOVE INTERRUPTS HERE >>> */

            /*** free irq ***/
            free_irq(pcie6509_device->irqlevel, pcie6509_device);
            pcie6509_device->irq_added = FALSE;
        }
        break;

    case IOCTL_PCIE6509_RESET_BOARD:
        DEBUGP(D_IOCTL, "IOCTL_PCIE6509_RESET_BOARD\n");

        PCIE6509_reset_board(pcie6509_device);  /* reset the board */
        break;

    default:
        pcie6509_device->error = PCIE6509_INVALID_PARAMETER;
        DEBUGPx(D_ERR, "unexpected IOCTL 0x%x\n", num);
        ret_code = -EINVAL;
        break;
    }

Get_Out:
    IOCTL_RETURN(ret_code);
}

/****************************
 *     Initialize Board     *
 ****************************/
void
PCIE6509_initialize_board(struct pcie6509_board *pcie6509_device)
{
    RFLAG_SET(RF_INIT_PENDING);
        
    /* <<< ADD CODE TO INITIALIZE HARDWARE HERE >>> */

    RFLAG_CLEAR(RF_INIT_PENDING);
}

/***********************
 *     Reset Board     *
 ***********************/
void
PCIE6509_reset_board(struct pcie6509_board *pcie6509_device)
{
    int lines;
    int dummy_read;
    pcie6509_local_ctrl_data_t *local_reg_ptr;

    RFLAG_SET(RF_RESET_PENDING);

    local_reg_ptr = (pcie6509_local_ctrl_data_t *)
                       pcie6509_device->local_region->virtual_address;

    pcie6509_device->error = PCIE6509_SUCCESS;
    pcie6509_device->info.interrupt.count  = 0;
    pcie6509_device->info.interrupt.mask   = 0;
    pcie6509_device->info.interrupt.status = 0;
    pcie6509_device->info.interrupt.pending= 0;

#if ((DUMMY_READ == 1) || (DUMMY_WRITE == 1))
    sprintf(dummy_read_write,"This is a dummy read operation "
                             "performed by the driver.\n"
                             "The quick brown fox jumps over the lazy dog.\n");
#endif /* DUMMY_READ || DUMMY WRITE */

    /* reset board */
    local_reg_ptr->Master.DioPortsStaticDigitalOutput   = 0x00000000; /* set port 0..3 as input */
    local_reg_ptr->Slave.DioPortsStaticDigitalOutput    = 0x00000000; /* set port 8..11 as input */
    local_reg_ptr->Master.PFIStaticDigitalOutput        = 0x00000000; /* set port 4..5 as input */
    local_reg_ptr->Slave.PFIStaticDigitalOutput         = 0x00000000; /* set port 6..7 as input */

    local_reg_ptr->Master.DioPortsDIODirection          = 0x00000000; /* set port 0..3 as input */
    local_reg_ptr->Slave.DioPortsDIODirection           = 0x00000000; /* set port 8..11 as input */
    local_reg_ptr->Master.PFIDirection                  = 0x00000000; /* set port 4..5 as input */
    local_reg_ptr->Slave.PFIDirection                   = 0x00000000; /* set port 6..7 as input */

    for(lines=0; lines < PCIE6509_LINES_PER_PORT; lines++) {
        local_reg_ptr->Master.PFIOutputSelectPort0[lines] = 0x00;     /* set port 4 as input */
        local_reg_ptr->Master.PFIOutputSelectPort1[lines] = 0x00;     /* set port 5 as input */
        local_reg_ptr->Slave.PFIOutputSelectPort0[lines]  = 0x00;     /* set port 6 as input */
        local_reg_ptr->Slave.PFIOutputSelectPort1[lines]  = 0x00;     /* set port 7 as input */
    }

    local_reg_ptr->Master.DioPortsDIFilterP01           = 0x00000000;
    local_reg_ptr->Master.DioPortsDIFilterP23           = 0x00000000;
    local_reg_ptr->Slave.DioPortsDIFilterP01            = 0x00000000;
    local_reg_ptr->Slave.DioPortsDIFilterP23            = 0x00000000;
    local_reg_ptr->Master.PFIFilterPort0Low             = 0x0000;
    local_reg_ptr->Master.PFIFilterPort0High            = 0x0000;
    local_reg_ptr->Master.PFIFilterPort1Low             = 0x0000;
    local_reg_ptr->Master.PFIFilterPort1High            = 0x0000;
    local_reg_ptr->Slave.PFIFilterPort0Low              = 0x0000;
    local_reg_ptr->Slave.PFIFilterPort0High             = 0x0000;
    local_reg_ptr->Slave.PFIFilterPort1Low              = 0x0000;
    local_reg_ptr->Slave.PFIFilterPort1High             = 0x0000;

    local_reg_ptr->Master.DioPortsDOWDTSafeState        = 0x00000000;   /* port 0..3 */
    local_reg_ptr->Slave.DioPortsDOWDTSafeState         = 0x00000000;   /* port 8..11 */
    local_reg_ptr->Master.PFIWDTSafeState               = 0x00000000;   /* port 4..5 */
    local_reg_ptr->Slave.PFIWDTSafeState                = 0x00000000;   /* port 6..7 */

    local_reg_ptr->Master.DioPortsDOWDTModeSelectP01    = 0x00000000;   /* port 0..1 */
    local_reg_ptr->Master.DioPortsDOWDTModeSelectP23    = 0x00000000;   /* port 2..3 */
    local_reg_ptr->Slave.DioPortsDOWDTModeSelectP01     = 0x00000000;   /* port 8..9 */
    local_reg_ptr->Slave.DioPortsDOWDTModeSelectP23     = 0x00000000;   /* port 10..11 */
    local_reg_ptr->Master.PFIWDTModeSelect              = 0x00000000;   /* port 4..5 */
    local_reg_ptr->Slave.PFIWDTModeSelect               = 0x00000000;   /* port 6..7 */

    local_reg_ptr->Master.CSWatchdogTimeout             = 0x00000000;   /* port 0..5 */
    local_reg_ptr->Slave.CSWatchdogTimeout              = 0x00000000;   /* port 6..11 */

    local_reg_ptr->Master.CSWatchdogConfiguration       = 0x0000;       /* port 0..5 */
    local_reg_ptr->Slave.CSWatchdogConfiguration        = 0x0000;       /* port 6..11 */
    
    local_reg_ptr->Master.CSWatchdogControl             = 0x0000;       /* port 0..5 */
    local_reg_ptr->Slave.CSWatchdogControl              = 0x0000;       /* port 6..11 */
    
    local_reg_ptr->Master.DioPortsDIChangeIrqRE         = 0x00000000;   /* port 0..3 */
    local_reg_ptr->Slave.DioPortsDIChangeIrqRE          = 0x00000000;   /* port 8..11 */

    local_reg_ptr->Master.DioPortsDIChangeIrqFE         = 0x00000000;   /* port 0..3 */
    local_reg_ptr->Slave.DioPortsDIChangeIrqFE          = 0x00000000;   /* port 8..11 */

    local_reg_ptr->Master.PFIChangeIrq                  = 0x00000000;   /* port 4..5 */
    local_reg_ptr->Slave.PFIChangeIrq                   = 0x00000000;   /* port 6..7 */

    local_reg_ptr->Master.CSGlobalInterruptEnable       = (PCIE6509_GIER_WTI_DISABLE |  /* port 0..5 */
                                                           PCIE6509_GIER_DII_DISABLE);
    local_reg_ptr->Slave.CSGlobalInterruptEnable        = (PCIE6509_GIER_WTI_DISABLE |  /* port 6..11 */
                                                           PCIE6509_GIER_DII_DISABLE);

    local_reg_ptr->Master.CSChangeDetectionIrq          = (PCIE6509_CDIR_CHANGE_DETECTION_ERROR_IRQ_DISABLE | /* port 0..5 */
                                                           PCIE6509_CDIR_CHANGE_DETECTION_IRQ_DISABLE);
    local_reg_ptr->Slave.CSChangeDetectionIrq           = (PCIE6509_CDIR_CHANGE_DETECTION_ERROR_IRQ_DISABLE | /* port 6..11 */
                                                           PCIE6509_CDIR_CHANGE_DETECTION_IRQ_DISABLE);

    local_reg_ptr->Master.CSWatchdogTimerInterrupt1     = (0);          /* port 0..5 */
    local_reg_ptr->Slave.CSWatchdogTimerInterrupt1      = (0);          /* port 6..11 */

    local_reg_ptr->Master.CSWatchdogTimerInterrupt2     = (PCIE6509_WTI2R_WDT_TRIGGER_IRQ_DISABLE); /* port 0..5 */
    local_reg_ptr->Slave.CSWatchdogTimerInterrupt2      = (PCIE6509_WTI2R_WDT_TRIGGER_IRQ_DISABLE); /* port 6..11 */

    local_reg_ptr->CHInCh.InterruptMask                 = (PCIE6509_IMR_CLEAR_CPU_INT |
                                                           PCIE6509_IMR_CLEAR_STC3_INT);
    dummy_read                                          = local_reg_ptr->CHInCh.VolatileInterruptStatus;

    local_reg_ptr->Master.CSIntForwardingControlStatus  = (PCIE6509_ICSR_INT_FORWARDING_RESET);
    local_reg_ptr->Slave.CSIntForwardingControlStatus   = (PCIE6509_ICSR_INT_FORWARDING_RESET);

    local_reg_ptr->Master.CSIntForwardingDestination    = (0);  /* as per documentation */
    local_reg_ptr->Slave.CSIntForwardingDestination     = (24); /* as per documentatiaon */

    local_reg_ptr->Master.CSRTSITrigDirection           = 0x0000;
    local_reg_ptr->Slave.CSRTSITrigDirection            = 0x0000;
    for(lines=0; lines < PCIE6509_LINES_PER_PORT; lines++) {
        local_reg_ptr->Master.CSRTSIOutputSelect[lines] = 0x00;
        local_reg_ptr->Slave.CSRTSIOutputSelect[lines]  = 0x00;
    }

    local_reg_ptr->CHInCh.Scrap                         = 0x00000000;

    local_reg_ptr->Master.CSScratchPad                  = 0x00000000;
    local_reg_ptr->Slave.CSScratchPad                   = 0x00000000;

    local_reg_ptr->Master.CSJointReset                  = (PCIE6509_JRR_SOFTWARE_RESET);
    local_reg_ptr->Slave.CSJointReset                   = (PCIE6509_JRR_SOFTWARE_RESET);

    /* clear shadow registers */
    memset(pcie6509_device->shadow_reg,0,sizeof(pcie6509_shadow_regs_t));

    RFLAG_CLEAR(RF_RESET_PENDING);
}


/************************************
 *     Configure memory regions     *
 ************************************/
int
PCIE6509_configure_mem_regions(struct pcie6509_board *pcie6509_device,
                             struct pci_dev *pdev, int which,
                             u32 * address, u32 * size, u32 * flags)
{
    DEBUGP_ENTER;
    /*** First get the requested memory region ***/
    *address = 0;
    *address = pci_resource_start(pdev, which);
    //*size = pci_resource_end(pdev, which) - *address;
    *size = pci_resource_len(pdev, which);
    DEBUGPx(D_L2, "PCI_BASE_ADDRESS_%d: address = 0x%x, size = 0x%x\n",
            which, *address, *size);

    /* if address and or size zero, region not present */
    if((*address == 0) || (*size == 0)) {
        DEBUGPx(D_L2,
            "Region %d Not Present: address = 0x%x, size = 0x%x\n",
            which, *address, *size);
            
        *address = *size = 0;
        DEBUGP_EXIT;
        return (0);
    }
    
    pcie6509_device->mem_region[which].virtual_address = 0;

    /*** Next, request the memory regions so that they are marked busy ***/

    *flags = pdev->resource[which].flags;

    /* I/O Region */
    if (pdev->resource[which].flags & IORESOURCE_IO) {
        DEBUGPx(D_L2, "Device %x Vendor %x:  region %d is I/O mapped\n",
                pdev->device, pdev->vendor, which);

        if (!request_region(*address, *size, PCIE6509_DRIVER_NAME)) {
            DEBUGPx(D_L2, "Device %x Vendor %x:  I/O region %d is busy\n",
                    pdev->device, pdev->vendor, which);
            *address = 0;   /* do not release it */
            DEBUGP(D_ERR, "request_region(IO REGION) failed\n");
            DEBUGP_EXIT;
            return (1);
        }
    } else {    /* Memory Region */
        DEBUGPx(D_L2, "Device %x Vendor %x:  region %d is memory mapped\n",
                pdev->device, pdev->vendor, which);

        if (!request_mem_region(*address, *size, PCIE6509_DRIVER_NAME)) {
            DEBUGPx(D_L2,
                    "Device %x Vendor %x:  memory region %d is busy\n",
                    pdev->device, pdev->vendor, which);
            *address = 0;   /* do not release it */
            DEBUGP(D_ERR, "request_region(MEMORY REGION) failed\n");
            DEBUGP_EXIT;
            return (1);
        }

        /* now remap physical to virtual address */
        pcie6509_device->mem_region[which].virtual_address = 
                    (unsigned int *) ioremap_nocache(
                            pcie6509_device->mem_region[which].physical_address,
                            pcie6509_device->mem_region[which].size);
        DEBUGPx(D_L2,"virtual_address[%d]=%p\n",which,
                        pcie6509_device->mem_region[which].virtual_address);
    }
    DEBUGP_EXIT;
    return (0);
}


/***************************************
 *     De-configure memory regions     *
 ***************************************/
void
PCIE6509_deconfigure_mem_regions(struct pcie6509_board *pcie6509_device, int which,
                               u32 address, u32 size, u32 flags)
{
    DEBUGP_ENTER;
    DEBUGPx(D_L2, "pcie6509_deconfigure_mem_regions = 0x%x "
            "size=0x%x flags=0x%x\n", address, size, flags);

    /* if no address to deconfigure, simply return */
    if ((address == 0) || (size == 0)) {
        DEBUGP_EXIT;
        return;
    }

    /* I/O Region */
    if (flags & IORESOURCE_IO) {
        DEBUGPx(D_L2, "Address %x Size %x:  Deconfigure I/O region %d\n",
                address, size, which);

        release_region(address, size);

    } else {    /* Memory Region */
        /* unmap virtual address */
        if(pcie6509_device->mem_region[which].virtual_address != 0)
                    iounmap(pcie6509_device->mem_region[which].virtual_address);

        DEBUGPx(D_L2,
                "Address %x Size %x:  Deconfigure memory region %d\n",
                address, size, which);

        release_mem_region(address, size);
    }
    DEBUGP_EXIT;
}

/******************************************************************************
 ***                                                                        ***
 *** Device mmap Handler                                                    ***
 ***                                                                        ***
 ******************************************************************************/
static int device_mmap(struct file *fp, struct vm_area_struct *vma)
{
    struct pcie6509_board *pcie6509_device = 0;
    u32 size, round_size;
    unsigned long page;
    int ret = 0;
    unsigned int offset = vma->vm_pgoff << PAGE_SHIFT;

    /* Get the device structure from the private_data field */
    pcie6509_device = (struct pcie6509_board *) fp->private_data;
    DEBUGP_ENTER;
    size = vma->vm_end - vma->vm_start;

    if(pcie6509_device->local_region != NULL) {
        DEBUGPx(D_L2, "mmap(): physical local register: "
            "address=0x%x size=0x%x\n",
            pcie6509_device->local_region->physical_address,
            pcie6509_device->local_region->size);
    }

    DEBUGPx(D_L2, "mmap(): start=0x%x end=0x%x offset=0x%x\n",
            (int) vma->vm_start, (int) vma->vm_end, offset);

    /* offset must be zero */
    if (offset) {
        pcie6509_device->error = PCIE6509_INVALID_PARAMETER;
        DEBUGPx(D_L2, "mmap(): Invalid offset 0x%x. Offset must be zero\n",
                offset);
        DEBUGP_EXIT;
        ret = -EINVAL;
    }

    /* Use mmap_reg_select to distinguish between Control/DataGSC  register 
     *  and PLX register */
    switch ((int) pcie6509_device->mmap_reg_select) {
    case PCIE6509_SELECT_LOCAL_MMAP: 
                                /* Control/Data - PCIE6509 (GSC) Registers */
        if(pcie6509_device->local_region == NULL) {
            DEBUGP(D_ERR,"Local Region Not Present\n");
            pcie6509_device->error = PCIE6509_INVALID_PARAMETER;
            DEBUGP_EXIT;
            ret = -EINVAL;
            break;
        }

        round_size = ((pcie6509_device->local_region->size +
                       PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

        DEBUGPx(D_L2, "Input Size = 0x%x Local Page "
                "Rounded Size = 0x%x\n", (u32) size, (u32) round_size);

        if (size > round_size) {
            DEBUGPx(D_ERR,"Invalid Size: Size=%d, round_size=%d\n",size,round_size);
            pcie6509_device->error = PCIE6509_INVALID_PARAMETER;
            DEBUGP_EXIT;
            ret = -EINVAL;
            break;
        }

        page = pcie6509_device->local_region->physical_address;
        DEBUGPx(D_L2, "mmap(): GSC vm_start=0x%x page=0x%x\n",
                (int) vma->vm_start, (int) page);

        vma->vm_flags |= VM_IO; /* NEEDED IF USER DOES AN mlockall() */

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,9)
        if (remap_page_range(vma, vma->vm_start, page, 
                                            size, vma->vm_page_prot)) {
#else
        if (remap_pfn_range(vma, vma->vm_start, (page >> PAGE_SHIFT), 
                                            size, vma->vm_page_prot)) {
#endif
            pcie6509_device->error = PCIE6509_RESOURCE_ALLOCATION_ERROR;
            DEBUGPx(D_L2,
                    "mmap(): remap_page_range() failed - vm_start=0x%x "
                    "page=0x%x, size=0x%x\n", (int) vma->vm_start,
                    (int) page, (int) size);
            DEBUGP_EXIT;
            ret = -ENOMEM;
        }
        break;

    case PCIE6509_SELECT_PHYS_MEM_MMAP:
        vma->vm_ops = &pcie6509_vm_ops;

        /* if allocation of physical memory failed, error return */
        if ((ret = PCIE6509_allocate_physical_memory(pcie6509_device, size))) {
            DEBUGP_EXIT;
            return ret;
        }

        page = (unsigned long)pcie6509_device->phys_mem;

        DEBUGPx(D_L2, "mmap(): GSC vm_start=0x%x page=0x%x\n",
                (int) vma->vm_start, (int) page);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,9)
        if(remap_page_range(vma, vma->vm_start, page, 
                                    size, vma->vm_page_prot)) {
#else
        if(remap_pfn_range(vma, vma->vm_start, (page >> PAGE_SHIFT), 
                                            size, vma->vm_page_prot)) {
#endif
            pcie6509_device->error = PCIE6509_RESOURCE_ALLOCATION_ERROR;
            DEBUGPx(D_L2,
                    "mmap(): remap_page_range() failed - vm_start=0x%x "
                    "page=0x%x, size=0x%x\n", (int) vma->vm_start,
                    (int) page, (int) size);
            PCIE6509_free_physical_memory(pcie6509_device);
            DEBUGP_EXIT;
            ret = -ENOMEM;
        }

        DEBUGP_EXIT;
        return ret;
        break;

    case PCIE6509_SELECT_SHADOW_REG_MMAP:
        page = (unsigned long)pcie6509_device->phys_shadow_reg;

        DEBUGPx(D_L2, "mmap(): GSC vm_start=0x%x page=0x%x\n",
                (int) vma->vm_start, (int) page);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,9)
        if(remap_page_range(vma, vma->vm_start, page, 
                                    size, vma->vm_page_prot)) {
#else
        if(remap_pfn_range(vma, vma->vm_start, (page >> PAGE_SHIFT), 
                                            size, vma->vm_page_prot)) {
#endif
            pcie6509_device->error = PCIE6509_RESOURCE_ALLOCATION_ERROR;
            DEBUGPx(D_L2,
                    "mmap(): remap_page_range() failed - vm_start=0x%x "
                    "page=0x%x, size=0x%x\n", (int) vma->vm_start,
                    (int) page, (int) size);
            DEBUGP_EXIT;
            ret = -ENOMEM;
        }

        DEBUGP_EXIT;
        return ret;
        break;

    default:
        pcie6509_device->error = PCIE6509_INVALID_PARAMETER;
        DEBUGPx(D_ERR, "Invalid parameter %ld\n",
                pcie6509_device->mmap_reg_select);
        DEBUGP_EXIT;
        ret = -EINVAL;
        break;
    }

    DEBUGP_EXIT;
    return ret;
}

/********************************************
 *     Allocate physical memory for DMA     *
 ********************************************/
int PCIE6509_allocate_physical_memory(struct pcie6509_board *pcie6509_device, int size)
{
    unsigned long virt_mem;
    int i, npages;

    /* if memory already allocated, return */
    if (pcie6509_device->phys_mem) {
        pcie6509_device->error = PCIE6509_ADDRESS_IN_USE;
        DEBUGPx(D_L2,
                "Physical memory at 0x%p, size=%d already allocated\n",
                pcie6509_device->phys_mem, pcie6509_device->phys_mem_size);
        return (-EADDRINUSE); /* good return */
    }

    if (size == 0) {
        DEBUGP(D_ERR, "Physical Memory size of 0 is invalid\n");
        pcie6509_device->error = PCIE6509_INVALID_PARAMETER;
        return -EINVAL; /* error return */
    }

    /*** Now allocate physical memory ***/
    /* round to page size */
    pcie6509_device->phys_mem_size = (size + PAGE_SIZE - 1) / PAGE_SIZE *
        PAGE_SIZE;
    pcie6509_device->phys_mem_size_freed = 0;
    pcie6509_device->virt_mem = (u32 *) __get_free_pages(GFP_KERNEL,
                                                     get_order
                                                     (pcie6509_device->
                                                      phys_mem_size));

    DEBUGPx(D_L1, "virt_mem=%p size=0x%x\n",
            pcie6509_device->virt_mem, pcie6509_device->phys_mem_size);

    if (pcie6509_device->virt_mem == 0) {
        DEBUGPx(D_ERR, "can not allocate DMA memory of size %d bytes\n",
                pcie6509_device->phys_mem_size);
        pcie6509_device->error = PCIE6509_RESOURCE_ALLOCATION_ERROR;
        pcie6509_device->phys_mem_size = 0;
        pcie6509_device->phys_mem_size_freed = 0;
        DEBUGP_EXIT;
        return (-ENOMEM);
    }

    /* reserve pages so that DMA will work */
    virt_mem = (unsigned long) pcie6509_device->virt_mem;
    npages = pcie6509_device->phys_mem_size / PAGE_SIZE;
    for (i = 0; i < npages; i++) {
        SetPageReserved(virt_to_page(virt_mem));
        virt_mem += PAGE_SIZE;
    }
    pcie6509_device->phys_mem =
        (u32 *)(unsigned long) virt_to_phys((void *) pcie6509_device->virt_mem);

    DEBUGPx(D_L1, "phys_mem=%p\n", pcie6509_device->phys_mem);
    if (pcie6509_device->phys_mem == 0) {
        PCIE6509_free_physical_memory(pcie6509_device);
        DEBUGP(D_ERR, "virt_to_phys returned zero physical memory\n");
        pcie6509_device->error = PCIE6509_RESOURCE_ALLOCATION_ERROR;
        DEBUGP_EXIT;
        return (-ENOMEM);
    }

    return (0);
}

/********************************
 *     Free physical memory     *
 ********************************/
void PCIE6509_free_physical_memory(struct pcie6509_board *pcie6509_device)
{
    unsigned long virt_mem;
    int i, npages;

    if (pcie6509_device->virt_mem) {
        DEBUGPx(D_L1, "Freeing Physical Memory at virt=%p phys=%p size=%d\n",
            pcie6509_device->virt_mem, pcie6509_device->phys_mem,
            pcie6509_device->phys_mem_size);
        virt_mem = (unsigned long) pcie6509_device->virt_mem;
        npages = pcie6509_device->phys_mem_size / PAGE_SIZE;
        for (i = 0; i < npages; i++) {
            ClearPageReserved(virt_to_page(virt_mem));
            virt_mem += PAGE_SIZE;
        }

        /* free requested buffer */
        free_pages((unsigned long) pcie6509_device->virt_mem,
                   get_order(pcie6509_device->phys_mem_size));
        pcie6509_device->virt_mem = (u32 *) NULL;
        pcie6509_device->phys_mem = (u32 *) NULL;
        pcie6509_device->phys_mem_size = 0;
        pcie6509_device->phys_mem_size_freed = 0;
    }
}

/***************************************
 *     define init and exit modules    *
 ***************************************/
module_init(PCIE6509_init);
module_exit(PCIE6509_exit);
