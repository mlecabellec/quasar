
/*
{+D}
   SYSTEM:          Acromag PCI/PMC424

   MODULE NAME:     apmc424.c

   VERSION:         L

   CREATION DATE:   06/29/06

   CODED BY:        FJM

   ABSTRACT:        Multiple apmc464 devices

   CALLING
	SEQUENCE:

   MODULE TYPE:

   I/O RESOURCES:

   SYSTEM
	RESOURCES:

   MODULES
	CALLED:

   REVISIONS:

 DATE      BY       PURPOSE
-------- ----  ------------------------------------------------
09/10/03 FJM   Red Hat Linux 9 update.
12/22/04 FJM   Fedora core FC3 update.
06/30/06 FJM   Support for multiple apmc424 devices
01/11/07 FJM   Add support x86_64
06/12/07 FJM   Fedora 7 update.
05/23/08 FJM   Fedora 8/9 update.
04/01/09 FJM   Remove 'IRQF_DISABLED' from request_irq().
06/02/10 FJM   Fedora13 update added #include <linux/wait.h> & <linux/sched.h>
05/25/11 FJM   Fedora 15 update.
01/23/13 FJM   Fedora 18 update, removed include of asm/system.h.

{-D}
*/

/* APMC424 device */


#ifndef BUILDING_FOR_KERNEL
#define BUILDING_FOR_KERNEL	/* controls conditional inclusion in file pmcmulticommon.h */
#endif


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <asm/uaccess.h>
////////////////////////////////////////////////////////
//Partie en lien avec l'utilisation des signaux Linux
////////////////////////////////////////////////////////
#include <linux/pid.h>
#include <linux/pid_namespace.h>
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

///////////////////////////////////////////
//Partie en lien avec les sockets NETLINK//
///////////////////////////////////////////
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/string.h>
#include <linux/version.h>
///////////////////////////////////////////

#include "./../../pmccommon/pmcmulticommon.h"
#include "../pmc424.h"

///////////////////////////////////////////
//Partie en lien avec les sockets NETLINK//
///////////////////////////////////////////
#define NETLINK_USER NETLINK_USERSOCK
///////////////////////////////////////////

#define PMC424 (word)0x4243 /* Pmc424 device ID */

#define MAJOR_NUM	46

int open_dev[MAX_PMCS];
unsigned int board_irq[MAX_PMCS];
unsigned long pmc_address[MAX_PMCS];
int ret_val = 0;


////////////////////////////////////////////////////////
//Partie en lien avec l'utilisation des signaux Linux
////////////////////////////////////////////////////////
int signum = SIGUSR1;
struct task_struct *task;
struct pid *pid_s;
struct siginfo info;

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
int *messageAtransmettreVoieTTLascii;
int tailleMessageAtransmettre;
int indiceTableauMessageTTL=0;
bool startEmission=false;
///////////////////////////////////////////
//Partie en lien avec les sockets NETLINK//
///////////////////////////////////////////
struct sock *nl_sk;
struct nlmsghdr * nlhISRSendTTLSignal;
int _pid;
///////////////////////////////////////////
///////////////////////////////////////////

struct file;
struct inode;
struct pt_regs;

static void nlmod_recv_msg (struct sk_buff * skb)
{
	struct nlmsghdr * nlh;
	int indiceTableauMessageTTL = 0;
	char *messageAtransmettreBrut;
	char *messageAtransmettreVoieTTLchar;
	nlh = (struct nlmsghdr *) skb->data;
	printk ("nlmod : received msg\n");
	printk(KERN_INFO "nlmod : msg payload:%s\n", (char *)nlmsg_data(nlh));
	/////////////////////////////////////////////
	/////////////////////////////////////////////

	/*On vérifie le type de demande qui a été envoyé de l'interface utilisateurs
	vers le module à travers les sockets NETLINK*/
	
	if(strncmp(nlmsg_data(nlh), "INIT",4) == 0)
	{
		//On récupère le PID du serveur NETLINK de façon à pouvoir lui remonter des informations
		_pid = nlh->nlmsg_pid;
		nlhISRSendTTLSignal->nlmsg_len = _pid;
		
		//Partie liée à l'utilisation des signaux Linux
		pid_s = find_get_pid (_pid);
		task = pid_task (pid_s, PIDTYPE_PID);
   		memset(&info, 0, sizeof(struct siginfo));
    		info.si_signo = signum;

		printk("Message d'initialisation reçu\n");
		messageAtransmettreVoieTTLchar =  kmalloc (sizeof(char)*strlen(nlmsg_data(nlh))-3,GFP_KERNEL);
		messageAtransmettreVoieTTLascii =  kmalloc (sizeof(int)*strlen(nlmsg_data(nlh))-4,GFP_KERNEL);

		messageAtransmettreBrut =  kmalloc (sizeof(char)*strlen(nlmsg_data(nlh))+1,GFP_KERNEL);

		strcpy(messageAtransmettreBrut,nlmsg_data(nlh));
		
		messageAtransmettreBrut[strlen(nlmsg_data(nlh))]= '\0';
		
		strncpy(messageAtransmettreVoieTTLchar,&messageAtransmettreBrut[4],strlen(nlmsg_data(nlh))-3);
		kfree(messageAtransmettreBrut);

		
		messageAtransmettreVoieTTLchar[strlen(nlmsg_data(nlh))-3] = '\0';

		printk("Message à transmettre:%s\n",messageAtransmettreVoieTTLchar);
		
		for (indiceTableauMessageTTL = 0;indiceTableauMessageTTL<strlen(messageAtransmettreVoieTTLchar); indiceTableauMessageTTL ++)
		{
			if(messageAtransmettreVoieTTLchar[indiceTableauMessageTTL]=='1')
			{
				messageAtransmettreVoieTTLascii[indiceTableauMessageTTL]=16;
			}
			else
			{
				messageAtransmettreVoieTTLascii[indiceTableauMessageTTL]=0;
			}
			printk("Message int:%d\n",messageAtransmettreVoieTTLascii[indiceTableauMessageTTL]);
		}
		tailleMessageAtransmettre = strlen(messageAtransmettreVoieTTLchar);
		kfree(messageAtransmettreVoieTTLchar);
		printk("Taille du message a transmettre : %d\n",tailleMessageAtransmettre);	
		startEmission=true;
	}
}

static int
open( struct inode *inode, struct file *fp )
{
   int minor;

   minor = inode->i_rdev & 0xf;

   if( minor > (MAX_PMCS - 1))
	   return( -ENODEV );
  
   if( open_dev[minor] )
	   return( -EBUSY );

   open_dev[minor] = 1;

   return( 0 );
} 


static int
release( struct inode *inode, struct file *fp )
{
   int minor;

   minor = inode->i_rdev & 0xf;

   if( minor > (MAX_PMCS - 1))
	   return( -ENODEV );

   if( open_dev[minor] )
   {
	   open_dev[minor] = 0;
	   return( 0 );
   }
   return( -ENODEV );
}

static ssize_t
read( struct file *fp, char *buf, size_t length, loff_t *offset )
{ 
	unsigned long adata, ldata ;
	unsigned short sdata;
	unsigned char cdata;

	get_user( adata, (unsigned long *)buf );		/* pickup address */
	switch( length )
	{
		case 1:	/* 8 bit */
		   cdata = readb( (void *) adata );
		   ldata = ( unsigned long )cdata;		/* convert to long */
		break;
		case 2:	/* 16 bit */
		   sdata = readw( (void *) adata );
		   ldata = ( unsigned long )sdata;		/* convert to long */
		break;
		case 4:	/* 32 bit */
		   ldata = readl( (void *) adata );
		break;
		default:
		    cdata = sdata = adata = ldata = 0;
		    return( -EINVAL );
		break;
	}
	put_user( ldata,(unsigned long *)( buf + (sizeof(unsigned long)) ) );	/* update user data */
	return( length );
}



static ssize_t
write( struct file *fp, const char *buf, size_t length, loff_t *offset )
{ 
	unsigned long adata, ldata ;

	get_user( adata, (unsigned long *)buf );				/* pickup address */
	get_user( ldata, (unsigned long *)( buf + (sizeof(unsigned long)) ) );	/* pickup data */
	switch( length )
	{
		case 1:	/* 8 bit */
		   writeb( (int)ldata, (void *)adata );
		break;
		case 2:	/* 16 bit */
		   writew( (int)ldata, (void *)adata );
		break;
		case 4:	/* 32 bit */
		   writel( (int)ldata, (void *)adata );
		break;
		default:
		    return( -EINVAL );
		break;
	}
    return( length );
}

/*  Used in versions FC14 and earlier */
/*
static int
ioctl( struct inode *inode, struct file *fp, unsigned int cmd, unsigned long arg)
*/

static long
device_ioctl( struct file *fp, unsigned int cmd, unsigned long arg)
{
    unsigned long ldata;
    int i;
	switch( cmd )
	{
		case 0:	/* reserved for future use */
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:/* return PMC address */
                   for(i = 0; i < MAX_PMCS; i++)                        /* get all boards */
                   {
                       ldata = ( unsigned long )pmc_address[i];         /* convert to long */
                       put_user( ldata, (unsigned long *)(arg+(i*(sizeof(unsigned long)))) );	/* update user data */
                   }    
		break;
		case 6:/* return IRQ number */
                   for(i = 0; i < MAX_PMCS; i++)                        /* get all boards */
                   {
                       ldata = ( unsigned long )board_irq[i];           /* convert IRQ to long */
                       put_user( ldata, (unsigned long *)(arg+(i*(sizeof(unsigned long)))) );	/* update user data */
                   }
		break;
	}
/*  Used in versions FC14 and earlier */
/*	return( cmd ); */
	return((long)cmd );
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
#define NULLMEMBER
#define NULLLASTMEMBER
#else
#define NULLMEMBER NULL,
#define NULLLASTMEMBER NULL
#endif


static struct file_operations pmc424_ops = {
  .owner = THIS_MODULE, /* owner of the world */
  NULLMEMBER                 /* seek */
  .read = read,         /* read */
  .write = write,       /* write */
  NULLMEMBER                 /* readdir */
  NULLMEMBER                 /* poll */
/*  Used in versions FC14 and earlier */
/*  .ioctl = ioctl,       / * ioctl */
  .unlocked_ioctl = device_ioctl, /* unlocked_ioctl */
  NULLMEMBER                 /* mmap */
  .open = open,         /* open */
  NULLMEMBER                 /* flush */
  .release = release,   /* release */
  NULLMEMBER                 /* fsync */
  NULLMEMBER                 /* fasync */
  NULLMEMBER                 /* lock */ 
  NULLMEMBER                 /* readv */
  NULLMEMBER                 /* writev */
  NULLMEMBER                 /* sendpage */
  NULLLASTMEMBER             /* get_unmapped_area */
};


static irqreturn_t
apmc424_handler( int irq, void *did, struct pt_regs *cpu_regs )
{ 
  volatile unsigned short nValue;
  volatile PMC_BOARD_MEMORY_MAP* pPMCCard;
  int int_status;
  int i;

   int_status = 0;                      /* indicate interrupt not handled */

   for(i = 0; i < MAX_PMCS; i++)        /* check all boards */
   {
     if( open_dev[i] )          	/* if board is open */
     {
	pPMCCard = (PMC_BOARD_MEMORY_MAP*)pmc_address[i];
	nValue = readw((unsigned short*)&pPMCCard->InterruptRegister);
	
	if(nValue & APMC_INT_PENDING)          /* Call interrupt handler for any pending interrupts */
	{
          isr_pmc424( (void *)pmc_address[i] );
  	  int_status = 1;
	}
     }
   }

   if( int_status )
       return( IRQ_HANDLED);
   else
       return( IRQ_NONE);
}



int
init_module( void ) 
{ 
	struct pci_dev *p424;
	int i,j;
	char devnamebuf[32];
	char devnumbuf[8];
	//////////////////////////////////////////
	/*Partie en lien avec les socket NETLINK*/
	//////////////////////////////////////////
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
	struct netlink_kernel_cfg cfg;
#endif
	//////////////////////////////////////////
	//////////////////////////////////////////
	memset( &open_dev[0], 0, sizeof(open_dev));
	memset( &board_irq[0], 0, sizeof(board_irq));
	memset( &pmc_address[0], 0, sizeof(pmc_address));

	nlhISRSendTTLSignal = (struct nlmsghdr *) kmalloc (NLMSG_SPACE(10),GFP_KERNEL);
	memset(nlhISRSendTTLSignal,0,NLMSG_SPACE(10));
	nlhISRSendTTLSignal->nlmsg_len = NLMSG_SPACE(10);
	nlhISRSendTTLSignal->nlmsg_flags = 0;
	p424 = NULL;

	//On crée la socket de type NETLINK nous permettant de recevoir des informations envoyées 		depuis l'interface utilisateur
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
	cfg.input = nlmod_recv_msg;
	cfg.groups = 0;
	nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
#else
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, 0, nlmod_recv_msg, NULL, THIS_MODULE);
#endif

	if (!nl_sk)
	{	
		printk (KERN_ALERT "modnl : Error creating socket.\n");
	}
	/////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

		
	for( i = 0, j = 0; i < MAX_PMCS; i++ )
	{
		/*  Use pci_find_device() for earlier versions FC3/4/5/6 */
		/*  p424 = ( struct pci_dev *)pci_find_device( VENDOR_ID, PMC424, p424 ); / * Pmc424 */
		p424 = ( struct pci_dev *)pci_get_device( VENDOR_ID, PMC424, p424 ); /* Pmc424 */
		if( p424 )
		{
			pmc_address[i] = (unsigned long)p424->resource[0].start;
			/*  Used in earlier versions FC3/4/5/6/7/8 */
			/*    pmc_address[i] = (unsigned long)__ioremap( pmc_address[i], 4096, 				_PAGE_PCD ); / * no cache!  PPC use _PAGE_NO_CACHE */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
			pmc_address[i] = (unsigned long)ioremap( pmc_address[i], 4096); /* no cache! */
#else
			pmc_address[i] = (unsigned long)ioremap_nocache( pmc_address[i], 4096); /* no cache! */
#endif

			if( pmc_address[i] )
			{
				memset( &devnamebuf[0], 0, sizeof(devnamebuf));
				memset( &devnumbuf[0], 0, sizeof(devnumbuf));
				strcpy(devnamebuf, DEVICE_NAME);
				sprintf(&devnumbuf[0],"%d",i);
				strcat(devnamebuf, devnumbuf);
				ret_val = pci_enable_device(p424);
				board_irq[i] = p424->irq;
				/*  Used for earlier versions FC3/4/5/6 */
				/*		ret_val = request_irq ( board_irq[i], 					apmc424_handler, SA_INTERRUPT | SA_SHIRQ, devnamebuf, ( void 					*)pmc_address[i] ); */
				/*  Used for FC7 */
				/*		ret_val = request_irq ( board_irq[i], 					(irq_handler_t)apmc424_handler, SA_INTERRUPT | SA_SHIRQ, 					devnamebuf, ( void *)pmc_address[i] );*/
				/*  Used for FC8/9/10 */
				/*		ret_val = request_irq ( board_irq[i], 						(irq_handler_t)apmc424_handler, IRQF_DISABLED | 				IRQF_SHARED, devnamebuf, ( void *)pmc_address[i] );*/
				ret_val = request_irq ( board_irq[i], (irq_handler_t)apmc424_handler, IRQF_SHARED, devnamebuf, ( void *)pmc_address[i] );
				printk("%s mapped   I/O=%08lX IRQ=%02X Rv=%X\n",devnamebuf,(unsigned long)pmc_address[i], board_irq[i],ret_val);
				j++;
			}
		}
		else
			break;
	}
		if( j )	/* found at least one device */
		{
			ret_val = register_chrdev ( MAJOR_NUM, DEVICE_NAME, &pmc424_ops );

			if( ret_val < 0)
			{
				printk(DEVICE_NAME);
				printk(" Failed to register error = %d\n", ret_val);
			}
			else
				return( 0 );
		}

  return( -ENODEV );
}

void
cleanup_module( void )
{
  char devnamebuf[32];
  char devnumbuf[8];
  int i;

  printk("CLEANUP\n");
  if( ret_val >= 0 )
  {
    unregister_chrdev( MAJOR_NUM, DEVICE_NAME );
    for( i = 0; i < MAX_PMCS; i++ )
    {
      if( pmc_address[i] )
      {
        memset( &devnamebuf[0], 0, sizeof(devnamebuf));
        memset( &devnumbuf[0], 0, sizeof(devnumbuf));
        strcpy(devnamebuf, DEVICE_NAME);
        sprintf(&devnumbuf[0],"%d",i);
        strcat(devnamebuf, devnumbuf);

        free_irq( board_irq[i], (void *)pmc_address[i] );
        iounmap( (void *)pmc_address[i] );
        printk("%s unmapped I/O=%08lX IRQ=%02X\n",devnamebuf,(unsigned long)pmc_address[i], board_irq[i]);
        open_dev[i] = 0;
      }
    }
  }
	printk(KERN_INFO "modnl : exit\n");
	kfree(messageAtransmettreVoieTTLascii);
	kfree(nlhISRSendTTLSignal);
	//On ferme la socket de type NETLINK précédemment créee
	netlink_kernel_release(nl_sk);

	
}

void isr_pmc424(void* pAddr)

{
	volatile struct map424 *map_ptr;	/* pointer to board memory map */
	volatile word istat;
	/////////////////////////////////////////////
	/* Partie en lien avec les sockets NETLINK */
	/////////////////////////////////////////////
	struct sk_buff * skb_out;
	
	/*
		Entry point of routine:
		Initialize pointers
	*/

	map_ptr = (struct map424 *)pAddr;
	
	/* Counter/Timer ISR */
	istat = readw( &map_ptr->CInterruptStatusReg );/* interrupt status */

	if( istat )                       /* any interrupt(s) pending? */
	{
		//printk("Entree dans la routine\n");
		
		writew( istat, &map_ptr->CInterruptStatusReg ); /* write interrupt pending to clear*/
		//printk("taille:%d\n",tailleMessageAtransmettre);
		//printk("taille1:%d\n",indiceTableauMessageTTL);
		if (startEmission == true)
		{
			if(indiceTableauMessageTTL!=(tailleMessageAtransmettre-1))
			{

				writew( messageAtransmettreVoieTTLascii[indiceTableauMessageTTL], &map_ptr->IOPort[2]);
				//printk("Valeur transmise:%d\n",messageAtransmettreVoieTTLascii[indiceTableauMessageTTL]);
				indiceTableauMessageTTL++;
			}
			else
			{
				indiceTableauMessageTTL = 0;
				startEmission = false;
				printk("ACK vers interface utilisateur\n");
				writew(0, &map_ptr->IOPort[2]);
				//send_sig_info (signum, &info, task);
				//printk("\nCounter ISR HwdAdr=%lX Istat=%X\n",(unsigned long)map_ptr, istat);

				//Partie qui permet de gérer l'envoi d'une requête d'information de la routine
				//d'interruption (fonction isr_pmc424(void * pAddr)) vers l'interface utilisateurs.
				// Dans notre cas, cela permet à l'interface utilisateur de savoir à quel moment 
				//envoyer le prochain bit à transmettre sur la voie TTL
				//printk ("nlmod : received cmd for send\n");

				//On alloue dynamiquement un buffer ayant pour taille la commande que l'on 
				//souhaite passer (dans notre cas 1 caractère car nous envoyons la commande '1'),
				//afin d' indiquer à l'interface utilisateurs que le prochain caractère est attendu
				skb_out = nlmsg_new(1, 0);

				//On test si l'allocation dynamique a pu être correctement effectué
				if (!skb_out) 
				{	
					printk(KERN_ERR "nlmod : Failed to allocate new skb\n");
					return;
				}
		
				nlhISRSendTTLSignal = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, 1, 0);
	
				//Nous n'utilisons pas le multicast dans notre cas, par conséquent nous
				//initialisons cette variable à 0
				NETLINK_CB(skb_out).dst_group = 0; 

				//Nous copions notre requête d'information (caractère '1') dans la partie
				// données utile de la socket NETLINK à travers l'entête de la socket NETLINK
				strncpy(nlmsg_data(nlhISRSendTTLSignal),"1",1);

				//On envoi la requête d'information depuis la routine d'interruption 
				//(fonction isr_pmc424(void * pAddr) vers l'interface utilisateurs
				// à travers la socket précedemment crée et à l'aide du PID précédemment récupéré
				//(fonction nlmod_recv_msg (struct sk_buff * skb), nous permettant ainsi, de contacter 
				//le serveur NETLINK présent au niveau de l'interface utilisateurs
				nlmsg_unicast(nl_sk, skb_out, _pid);
				printk("Fin interruption");
			}
		}
	}
}

MODULE_LICENSE("GPL and additional rights");

