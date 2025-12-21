// vim:ts=4 expandtab:
/*****************************************************************************
 *                                                                           *
 * File:         gsc16ao_ioctl.c                                             *
 *                                                                           *
 * Description:  Linux host driver for PCI/PMC-16AO-12 analog output card    *
 *               Driver IOCTL functions                                      *
 *                                                                           *
 * Date:         10/07/2009                                                  *
 * History:                                                                  *
 *                                                                           *
 *   9 10/07/09 D. Dubash                                                    *
 *              Fixed problem where board initialization was returning to    *
 *              user before board initialization was complete.               *
 *                                                                           *
 *   8 05/20/09 D. Dubash                                                    *
 *              New ioctl IOCTL_GSC16AO_SELECT_DIFFERENTIAL_SYNC_IO          *
 *              New ioctl IOCTL_GSC16AO_DISABLE_EXT_BURST_TRIGGER            *
 *                                                                           *
 *   7 06/25/07 D. Dubash                                                    *
 *              Support for new GSC16AO16 sixteen channel board.             *
 *              Support for RH4.1.7 on 64_bit architecture                   *
 *                                                                           *
 *   6 11/16/06 D. Dubash                                                    *
 *              Support for redHawk 4.1.7.                                   *
 *                                                                           *
 *   5 10/06/05 D. Dubash                                                    *
 *              Support for redHawk 2.3.3.                                   *
 *                                                                           *
 *   4  8/23/05 D. Dubash                                                    *
 *              Support for redHawk 2.2. Added device to card association.   *
 *                                                                           *
 *   3  8/20/03 G. Barton                                                    *
 *              Add WAIT_FOR_INTERRUPT ioctl support                         *
 *                                                                           *
 *   2  5/30/03 G. Barton                                                    *
 *              Adapted for Redhawk Linux                                    *
 *                                                                           *
 *   1  12/2002 E. Hillman (evan@generalstandards.com)                       *
 *              Created                                                      *
 *                                                                           *
 *  Copyrights (c):                                                          *
 *      Concurrent Computer Corporation, 2003                                *
 *      General Standards Corporation (GSC), Dec 2002                        *
 *****************************************************************************/

//#include <linux/config.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/pci.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/ioctl.h>
#include <linux/slab.h>

#include <asm/uaccess.h>
#include <asm/io.h>
//#include <asm/system.h>

#include "gsc16ao_regs.h"
#include "gsc16ao.h"
#include "gsc16ao_ioctl.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#endif

extern unsigned char _debug_level;
extern unsigned char _debug_class_flags;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
long unlocked_device_ioctl(struct file *filp,u_int iocmd,unsigned long ioarg )
{
    int ret;
    struct inode *inode;
	struct gsc_board *device = (struct gsc_board *)filp->private_data;

	prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): unlocked_device_ioctl() entered...\n", device->minor);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
    inode = filp->f_dentry->d_inode;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,12,0)
    inode = filp->f_path.dentry;
#else
    inode = filp->f_path.dentry->d_inode;
#endif

    mutex_lock(&device->ioctl_mtx);
    if(device->ioctl_processing) {
        prmsg(GDL_TRACE1, GDC_IOCTL,"ioctl processing active: busy\n");
        mutex_unlock(&device->ioctl_mtx);
        return (-EBUSY);    /* DO NOT USE IOCTL_RETURN() CALL HERE */
    }
    mutex_unlock(&device->ioctl_mtx);

    ret = device_ioctl(inode, filp, iocmd, ioarg);

    return( (long) ret );
}
#endif

#define IOCTL_RETURN(Code) {     \
    device->ioctl_processing = 0; \
    return (Code);  \
}

/* ioctl file operation: this does the bulk of the work */
int
device_ioctl(struct inode *inode, struct file *fp, unsigned int num, unsigned long arg)
{
	struct gsc_board *device = (struct gsc_board *)fp->private_data;
	struct register_params regs;
	struct chan_select chanSelect;
	struct gsc_debug_flags debug_flags;
	unsigned long regval,ulval;
	int    retval, done;
	
	prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): device_ioctl() entered...\n", device->minor);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,33)
    if(device->ioctl_processing) {
        prmsg(GDL_TRACE1, GDC_IOCTL,"ioctl processing active: busy\n");
        return (-EBUSY);    /* DO NOT USE IOCTL_RETURN() CALL HERE */
    } 
#endif

    device->ioctl_processing++;   

	/* main ioctl function dispatch */
	/* 'break' at the end of branches which need to wait for the */
	/* channels ready condition, 'return' from others */
	switch (num) {
	case IOCTL_GSC16AO_NO_COMMAND:
		prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_NO_COMMAND\n", device->minor);
		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_INIT_BOARD:
		prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_INIT_BOARD\n", device->minor);
		
		regval = readlocal(device,BOARD_CTRL_REG);
		regval &= (~(BCR_IRQ_MASK));
		regval |= (BCR_IRQ_INIT | (BCR_INITIALIZE));
		device->ioctl_wait_init_complete = TRUE;
		writelocal(device,regval,BOARD_CTRL_REG);
		
		device->timeout=FALSE;
		device->watchdog_timer.expires=jiffies+device->timeout_seconds*HZ;
		add_timer(&device->watchdog_timer);
		writel(readl(IntCntrlStat(device)) | IRQ_PCI_ENABLE | IRQ_LOCAL_PCI_ENABLE, IntCntrlStat(device));
		wait_event_interruptible(device->ioctlwq,(!device->ioctl_wait_init_complete));
		if (device->timeout)
		{
			prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_INIT_BOARD: jiffies=%ld expires=%ld\n",
			      device->minor, jiffies, device->watchdog_timer.expires);
			prmsg(GDL_ERR, GDC_IOCTL, "(%d): timeout during IOCTL_GSC16AO_INIT_BOARD\n", device->minor);
			device->ioctl_wait_init_complete = FALSE;
			device->error = GSC16AO_ERR_IOCTL_TIMEOUT;
			IOCTL_RETURN (-EIO);
		}
		else
			del_timer_sync(&device->watchdog_timer);
		
		prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_INIT_BOARD: ics=%.8X bcr=%.8X dcs=%.8X\n",device->minor, readl(IntCntrlStat(device)), readlocal(device,BOARD_CTRL_REG),readl(DMACmdStatus(device)) );
		prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_INIT_BOARD done\n", device->minor);

		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_READ_REGISTER:
	case IOCTL_GSC16AO_WRITE_REGISTER:
	  prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_READ_REGISTER\n", device->minor);
	  copy_from_user_ret(&regs, (void *)arg, sizeof(regs), (-EFAULT));

	  switch (regs.regset) {
	  case GSC16AO_GSC_REGISTER:
	    if (regs.regnum > GSC16AO_GSC_ACLK) {
	      device->error = GSC16AO_ERR_INVALID_PARAMETER;
	      IOCTL_RETURN (-EIO);
	    }
	    if (num == IOCTL_GSC16AO_READ_REGISTER)
	      regs.regval = readl(device->local_addr + regs.regnum);
	    else
	      writel(regs.regval, device->local_addr + regs.regnum);
	    break;
	    
	  case GSC16AO_PCI_REGISTER:
	    if (regs.regnum > 63) {
	      device->error = GSC16AO_ERR_INVALID_PARAMETER;
	      IOCTL_RETURN (-EIO);
	    }
	    if (num == IOCTL_GSC16AO_READ_REGISTER)
	      pci_read_config_dword (device->pdev,
				     (unsigned long)(regs.regnum * sizeof(u32)),
				     (u32*)&regs.regval);
	    else
	      pci_write_config_dword (device->pdev,
				      (unsigned long)(regs.regnum * sizeof(u32)),
				      (u32)regs.regval);
	    break;
	    
	  case GSC16AO_PLX_REGISTER:
	    if (regs.regnum > GSC16AO_PLX_LBRD1) {
	      device->error = GSC16AO_ERR_INVALID_PARAMETER;
	      IOCTL_RETURN (-EIO);
	    }
	    if (num == IOCTL_GSC16AO_READ_REGISTER)
	      regs.regval = readl(device->runtime_addr + regs.regnum);
	    else
	      writel(regs.regval, device->runtime_addr + regs.regnum);
	    break;
	    
	  default:
	    device->error = GSC16AO_ERR_INVALID_PARAMETER;
	    IOCTL_RETURN (-EIO);
	  }    

	  if (num == IOCTL_GSC16AO_READ_REGISTER) {
	    copy_to_user_ret((void *)arg, &regs, sizeof(regs), (-EFAULT));
	  }
	  IOCTL_RETURN (0);

	case IOCTL_GSC16AO_SET_DEBUG_FLAGS:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_SET_DEBUG_FLAGS\n", device->minor);
		copy_from_user_ret(&debug_flags, (void *)arg, sizeof(debug_flags), (-EFAULT));
		_debug_class_flags = debug_flags.db_classes;
		_debug_level = debug_flags.db_level;
		IOCTL_RETURN (0);

	case IOCTL_GSC16AO_GET_DEVICE_ERROR:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_GET_DEVICE_ERROR\n", device->minor);
		put_user_ret(device->error, (unsigned long *)arg, (-EFAULT));
		IOCTL_RETURN (0);
		break;
		
	case IOCTL_GSC16AO_SET_WRITE_MODE:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_WRITE_MODE_CONFIG\n", device->minor);
		get_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
		if ((ulval != GSC16AO_SCAN_MODE && ulval != GSC16AO_DMA_MODE)) {
			device->error = GSC16AO_ERR_INVALID_PARAMETER;
			IOCTL_RETURN (-EIO);
		}
		device->ulWriteMode=ulval;
		IOCTL_RETURN (0);	
		
	case IOCTL_GSC16AO_AUTO_CAL:
		prmsg(GDL_INFO, GDC_IOCTL, "(%d): calibrating device\n", device->minor);
		regval = readlocal(device,BOARD_CTRL_REG);
		regval &= (~(BCR_IRQ_MASK));
		regval |= BCR_IRQ_AUTOCAL_COMPLETE;
		regval |= BCR_INIT_CALIBRATION; /* start calibration */

		device->ioctl_wait_autocal_complete = TRUE;
		writelocal(device,regval,BOARD_CTRL_REG);
		
		device->timeout=FALSE;
		device->watchdog_timer.expires=jiffies+device->timeout_seconds*HZ;
		add_timer(&device->watchdog_timer);
		
		writel(readl(IntCntrlStat(device)) | IRQ_PCI_ENABLE | IRQ_LOCAL_PCI_ENABLE, IntCntrlStat(device));
		wait_event_interruptible(device->ioctlwq,(!device->ioctl_wait_autocal_complete));
		
		if (device->timeout) {
			prmsg(GDL_ERR, GDC_IOCTL, "(%d): timeout when calibrating device\n", device->minor);
			device->ioctl_wait_autocal_complete = FALSE;
			device->error = GSC16AO_ERR_IOCTL_TIMEOUT;
			IOCTL_RETURN (-EIO);
		}
		else
			del_timer_sync(&device->watchdog_timer);

		if(device->calibStatus==AUTOCAL_FAILED)
			IOCTL_RETURN (-EIO);
		
		prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): ioctl autocal done\n", device->minor);
		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_WAIT_FOR_INTERRUPT:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_WAIT_FOR_INTERRUPT\n", device->minor);
		get_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
		switch (ulval) {
		case GSC16AO_BCR_IRQ_OUT_BUFFER_EMPTY:
		case GSC16AO_BCR_IRQ_OUT_BUFFER_LOW_QUARTER:
		case GSC16AO_BCR_IRQ_OUT_BUFFER_HIGH_QUARTER:
		case GSC16AO_BCR_IRQ_BURST_TRIGGER_READY:
		case GSC16AO_BCR_IRQ_LOAD_READY:
		  break;

		default:
		  prmsg(GDL_ERR, GDC_IOCTL, "(%d): IOCTL_GSC16AO_WAIT_FOR_INTERRUPT: Bad IRQ event 0x%04x\n",
			device->minor, ulval);
		  device->error = GSC16AO_ERR_INVALID_PARAMETER;
		  IOCTL_RETURN (-EIO);
		}

		GSC16AO_LOCK(device); /* Hold off other threads, interrupts */
		if (device->ioctl_wait_event_type != 0 /* || readlocal(device,BOARD_CTRL_REG) & BCR_IRQ_MASK */) {
		  GSC16AO_UNLOCK(device);
		  prmsg(GDL_ERR, GDC_IOCTL, "(%d): IOCTL_GSC16AO_WAIT_FOR_INTERRUPT: IRQ event already in use\n",
			device->minor);
		  IOCTL_RETURN (-EBUSY);
		}
		
		/*
		 * Evaluate wait condition and wait as needed
		 */
		retval = 0;
		done = FALSE;
		while (!done) {
		    /* Wait condition satisfied ? */
		    unsigned long bcr_reg = readlocal(device, BOARD_CTRL_REG);
		    unsigned long bor_reg = readlocal(device, BUFFER_OPS_REG);

		    switch (ulval) {
		    case GSC16AO_BCR_IRQ_OUT_BUFFER_EMPTY:
		      if (bor_reg & BOR_CIRCULAR_BUFFER) {
			prmsg(GDL_ERR, GDC_IOCTL, "(%d): IOCTL_GSC16AO_WAIT_FOR_INTERRUPT: Buffer closed?\n",
			      device->minor);
			retval = -EIO;  /* Must be in open buffer mode */
			done = TRUE;
		      }
		      else if (bor_reg & BOR_BUFFER_EMPTY) {
			done = TRUE;
		      }
		      break;
		      
		    case GSC16AO_BCR_IRQ_OUT_BUFFER_LOW_QUARTER:
		      if (bor_reg & BOR_CIRCULAR_BUFFER) {
			prmsg(GDL_ERR, GDC_IOCTL, "(%d): IOCTL_GSC16AO_WAIT_FOR_INTERRUPT: Buffer closed?\n",
			      device->minor);
			retval = -EIO;  /* Must be in open buffer mode */
			done = TRUE;
		      }
		      else if (bor_reg & BOR_BUFFER_LOW_QUARTER) {
			done = TRUE;
		      }
		      break;

		    case GSC16AO_BCR_IRQ_OUT_BUFFER_HIGH_QUARTER:
		      if (bor_reg & BOR_CIRCULAR_BUFFER) {
			prmsg(GDL_ERR, GDC_IOCTL, "(%d): IOCTL_GSC16AO_WAIT_FOR_INTERRUPT: Buffer closed?\n",
			      device->minor);
			retval = -EIO;  /* Must be in open buffer mode */
			done = TRUE;
		      }
		      else if (bor_reg & BOR_BUFFER_HIGH_QUARTER) {
			done = TRUE;
		      }
		      break;

		    case GSC16AO_BCR_IRQ_BURST_TRIGGER_READY:
		      if ((bcr_reg & BCR_BURST_ENABLED) == 0) {
			prmsg(GDL_ERR, GDC_IOCTL, "(%d): IOCTL_GSC16AO_WAIT_FOR_INTERRUPT: Bursts not enabled?\n",
			      device->minor);
			retval = -EIO;  /* Must be in burst trigger mode */
			done = TRUE;
		      }
		      else if (bcr_reg & BCR_BURST_READY) {
			done = TRUE;
		      }
		      break;
		      
		    case GSC16AO_BCR_IRQ_LOAD_READY:
		      if ((bor_reg & BOR_CIRCULAR_BUFFER) == 0) {
			prmsg(GDL_ERR, GDC_IOCTL, "(%d): IOCTL_GSC16AO_WAIT_FOR_INTERRUPT: Buffer open?\n",
			      device->minor);
			retval = -EIO;  /* Must be in closed buffer mode */
			done = TRUE;
		      }
		      else {
			/* LOAD REQUEST must be asserted or LOAD READY will never come */
			bor_reg = readlocal(device, BUFFER_OPS_REG);
			if ((bor_reg & BOR_LOAD_REQUEST) == 0) {
			  writelocal(device, bor_reg | BOR_LOAD_REQUEST, BUFFER_OPS_REG);
			  bor_reg = readlocal(device, BUFFER_OPS_REG);
			}
			if (bor_reg & BOR_LOAD_READY) {
			  done = TRUE;
			}
		      }      
		      break;

		    default:
		      prmsg(GDL_ERR, GDC_IOCTL, "(%d): IOCTL_GSC16AO_WAIT_FOR_INTERRUPT: Unknown IRQ event\n",
			    device->minor);
		      retval = -EIO;
		      done = TRUE;
		      break;
		  }; /* Switch */

		  if (!done) {
		    /*
		     * Condition not satisified. Wait for an interrupt
		     */
		    /* Enable specific the interrupt condition */
		    regval = readlocal(device,BOARD_CTRL_REG);
		    regval &= (~(BCR_IRQ_MASK));
		    regval |= (ulval);
		    writelocal(device,regval,BOARD_CTRL_REG);

		    device->ioctl_wait_event_type = ulval; /* Tell intr routine what to look for */
		    device->ioctl_wait_event = TRUE;

		    GSC16AO_UNLOCK(device);
		    wait_event_interruptible(device->ioctlwq,(!device->ioctl_wait_event));
		    GSC16AO_LOCK(device); /* reacquire mutex */

		    if (device->ioctl_wait_event_type == ulval) {
		      /* Disable interrupt condition in case of timeout */
		      regval = readlocal(device,BOARD_CTRL_REG);
		      regval &= (~(BCR_IRQ_MASK));
		      writelocal(device,regval,BOARD_CTRL_REG);
		    }
		    device->ioctl_wait_event_type = 0; /* no longer waiting */

		    if (device->timeout) {
		      prmsg(GDL_ERR, GDC_IOCTL, "(%d): timeout during wait for interrupt\n", device->minor);
		      device->error = GSC16AO_ERR_IOCTL_TIMEOUT;
		      retval = -EIO;
		      done = TRUE;
		    }
		  }
	        }; /* while (!done); */

		GSC16AO_UNLOCK(device);
		IOCTL_RETURN (retval);
		
	case IOCTL_GSC16AO_PROGRAM_RATE_GEN:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_PROGRAM_RATE_GEN\n", device->minor);
		get_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
		if ((ulval < 0) || (ulval > GSC16AO_MAX_RATE_GEN)) {
			device->error = GSC16AO_ERR_INVALID_PARAMETER;
			IOCTL_RETURN (-EIO);
		}
		//device->notifyInt=ulval;
		writelocal(device,ulval,RATE_CTRL_REG);
		break;
		
	case IOCTL_GSC16AO_SELECT_ACTIVE_CHAN:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_SELECT_ACTIVE_CHAN\n", device->minor);
		copy_from_user_ret(&chanSelect, (void *)arg, sizeof(chanSelect), (-EFAULT));
		writelocal(device,chanSelect.ulChannels, CHANNEL_SELECTION_REG);
		break;
		
	case IOCTL_GSC16AO_SET_OUT_BUFFER_SIZE:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_SET_OUT_BUFFER_SIZE\n", device->minor);
		get_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
        if(device->board_info.board_type == GSC_16AO_16) {
		    if (ulval > OUT_BUFFER_SIZE_AO16_262144) {
			    device->error = GSC16AO_ERR_INVALID_PARAMETER;
			    IOCTL_RETURN (-EIO);
		    }
        } else {
		    if (ulval > OUT_BUFFER_SIZE_AO12_131072) {
			    device->error = GSC16AO_ERR_INVALID_PARAMETER;
			    IOCTL_RETURN (-EIO);
		    }
        }
		regval = readlocal(device, BUFFER_OPS_REG);
		regval &= (~BOR_BUFFER_SIZE_MASK);
		regval |= ulval;
		writelocal(device,regval, BUFFER_OPS_REG);
		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_GET_BUF_STATUS:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_GET_BUF_STATUS\n", device->minor);
		put_user_ret(readlocal(device, BUFFER_OPS_REG)&GSC16AO_BUFFER_STATUS_MASK, (unsigned long *)arg, (-EFAULT));
		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_ENABLE_CLK:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_ENABLE_CLK\n", device->minor);
		regval = readlocal(device, BUFFER_OPS_REG);
		regval |= BOR_ENABLE_CLOCK;
		writelocal(device,regval, BUFFER_OPS_REG);
		IOCTL_RETURN (0);
		break;
			
	case IOCTL_GSC16AO_DISABLE_CLK:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_DISABLE_CLK\n", device->minor);
		regval = readlocal(device, BUFFER_OPS_REG);
		regval &= ~(BOR_ENABLE_CLOCK);
		writelocal(device,regval, BUFFER_OPS_REG);
		IOCTL_RETURN (0);
		break;

	case IOCTL_GSC16AO_GET_CALIB_STATUS:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_GET_CALIB_STATUS\n", device->minor);
		put_user_ret(device->calibStatus, (unsigned long *)arg, (-EFAULT));
		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_SELECT_DATA_FORMAT:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_SELECT_DATA_FORMAT\n", device->minor);
		get_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
		if ((ulval != GSC16AO_TWOS_COMP) && (ulval != GSC16AO_OFFSET_BINARY)) {
			device->error = GSC16AO_ERR_INVALID_PARAMETER;
			IOCTL_RETURN (-EIO);
		}
		regval = readlocal(device, BOARD_CTRL_REG);
		if (ulval==GSC16AO_OFFSET_BINARY)
		{
			regval |= BCR_OFFSET_BINARY;
		}
		else
		{
			regval &= ~(BCR_OFFSET_BINARY);
		}
		writelocal(device,regval, BOARD_CTRL_REG);
		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_SELECT_OUTPUT_RANGE:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_SELECT_OUTPUT_RANGE\n", device->minor);
        if(device->board_type != GSC_16AO_16) {
            device->error = GSC16AO_ERR_INVALID_PARAMETER;
            IOCTL_RETURN (-EIO);
        }
		get_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
        if(device->board_info.flv) {    /* if gsc16ao_flv */
            regval = readlocal(device, BOARD_CTRL_REG);
            regval &= ~BCR_OUTPUT_RANGE_FLV;
            if(device->board_info.high_level) { /* only 5V or 10 V */
                if((ulval != GSC16AO16_RANGE_5) && (ulval != GSC16AO16_RANGE_10)) {
			        device->error = GSC16AO_ERR_INVALID_PARAMETER;
			        IOCTL_RETURN (-EIO);
                }
                if(ulval == GSC16AO16_RANGE_5)
                    regval |= BCR_OUTPUT_RANGE_5V_FLV;
                else
                    regval |= BCR_OUTPUT_RANGE_10V_FLV;
            } else {                            /* only 1.5V or 2.5V */
                if((ulval != GSC16AO16_RANGE_1_5) && (ulval != GSC16AO16_RANGE_2_5)) {
			        device->error = GSC16AO_ERR_INVALID_PARAMETER;
			        IOCTL_RETURN (-EIO);
                }
                if(ulval == GSC16AO16_RANGE_1_5)
                    regval |= BCR_OUTPUT_RANGE_1_5V_FLV;
                else
                    regval |= BCR_OUTPUT_RANGE_2_5V_FLV;
            }
        } else {
            regval = readlocal(device, BOARD_CTRL_REG);
            regval &= ~BCR_OUTPUT_RANGE;
            switch(ulval) {
                case GSC16AO16_RANGE_1_25:
                    regval |= BCR_OUTPUT_RANGE_1_25V;
                break;
                case GSC16AO16_RANGE_2_5:
                    regval |= BCR_OUTPUT_RANGE_2_5V;
                break;
                case GSC16AO16_RANGE_5:
                    regval |= BCR_OUTPUT_RANGE_5V;
                break;
                case GSC16AO16_RANGE_10:
                    regval |= BCR_OUTPUT_RANGE_10V;
                break;
                default:
                    device->error = GSC16AO_ERR_INVALID_PARAMETER;
                    IOCTL_RETURN (-EIO);
                break;
            }
        }

		writelocal(device,regval, BOARD_CTRL_REG);

		IOCTL_RETURN (0);

	case IOCTL_GSC16AO_SELECT_OUTPUT_FILTER:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_SELECT_OUTPUT_FILTER\n", device->minor);

        if((device->board_type != GSC_16AO_16) || (device->board_info.flv==0)) {
            device->error = GSC16AO_ERR_INVALID_PARAMETER;
            IOCTL_RETURN (-EIO);
        }

		get_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
        regval = readlocal(device, BOARD_CTRL_REG);
        regval &= ~BCR_OUTPUT_FILTER;

        switch(ulval) {
            case GSC16AO16_FILTER_NONE:
                regval |= BCR_OUTPUT_FILTER_NONE;
            break;
            case GSC16AO16_FILTER_A:
                regval |= BCR_OUTPUT_FILTER_A;
            break;
            case GSC16AO16_FILTER_B:
                regval |= BCR_OUTPUT_FILTER_B;
            break;
            default:
                device->error = GSC16AO_ERR_INVALID_PARAMETER;
                IOCTL_RETURN (-EIO);
            break;
        }

		writelocal(device,regval, BOARD_CTRL_REG);

		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_SELECT_SAMPLING_MODE:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_SELECT_SAMPLING_MODE\n", device->minor);
		get_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
		if ((ulval != GSC16AO_CONT_MODE) && (ulval != GSC16AO_BURST_MODE)) {
			device->error = GSC16AO_ERR_INVALID_PARAMETER;
			IOCTL_RETURN (-EIO);
		}
		regval = readlocal(device, BOARD_CTRL_REG);
		if (ulval==GSC16AO_BURST_MODE)
		{
			regval |= BCR_BURST_ENABLED;
		}
		else
		{
			regval &= ~(BCR_BURST_ENABLED);
		}
		writelocal(device,regval, BOARD_CTRL_REG);
		IOCTL_RETURN (0);
	
	case IOCTL_GSC16AO_SELECT_DIFFERENTIAL_SYNC_IO:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_SELECT_DIFFERENTIAL_SYNC_IO\n", device->minor);
		get_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
		if ((ulval != GSC16AO_TTL) && (ulval != GSC16AO_LVDS)) {
			device->error = GSC16AO_ERR_INVALID_PARAMETER;
			IOCTL_RETURN (-EIO);
		}
		regval = readlocal(device, BOARD_CTRL_REG);
		if (ulval==GSC16AO_LVDS)
		{
			regval |= BCR_DIFFERENTIAL_SYNC_IO;
		}
		else
		{
			regval &= ~(BCR_DIFFERENTIAL_SYNC_IO);
		}
		writelocal(device,regval, BOARD_CTRL_REG);
		IOCTL_RETURN (0);
	
	case IOCTL_GSC16AO_DISABLE_EXT_BURST_TRIGGER:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_DISABLE_EXT_BURST_TRIGGER\n", device->minor);
		get_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
		if ((ulval != GSC16AO_DISABLE_EXT_TRIG) && (ulval != GSC16AO_ENABLE_EXT_TRIG)) {
			device->error = GSC16AO_ERR_INVALID_PARAMETER;
			IOCTL_RETURN (-EIO);
		}
		regval = readlocal(device, BOARD_CTRL_REG);
		if (ulval==GSC16AO_DISABLE_EXT_TRIG)
		{
			regval |= BCR_DISABLE_EXT_BURST_TRIGGER;
		}
		else
		{
			regval &= ~(BCR_DISABLE_EXT_BURST_TRIGGER);
		}
		writelocal(device,regval, BOARD_CTRL_REG);
		IOCTL_RETURN (0);
	
	case IOCTL_GSC16AO_GET_BURSTING_STATUS:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_GET_BURSTING_STATUS\n", device->minor);
		put_user_ret(readlocal(device, BOARD_CTRL_REG)&BCR_BURST_READY, (unsigned long *)arg, (-EFAULT));
		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_BURST_TRIGGER:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_BURST_TRIGGER\n", device->minor);
		regval = readlocal(device, BOARD_CTRL_REG);
		regval |= BCR_BURST_TRIGGER;
		writelocal(device,regval, BOARD_CTRL_REG);
		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_ENABLE_REMOTE_GND_SENSE:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_ENABLE_REMOTE_GND_SENSE\n", device->minor);
		regval = readlocal(device, BOARD_CTRL_REG);
		regval |= BCR_REMOTE_GROUND_SENSE;
		writelocal(device,regval, BOARD_CTRL_REG);
		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_DISABLE_REMOTE_GND_SENSE:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_DISABLE_REMOTE_GND_SENSE\n", device->minor);
		regval = readlocal(device, BOARD_CTRL_REG);
		regval &= ~BCR_REMOTE_GROUND_SENSE;
		writelocal(device,regval, BOARD_CTRL_REG);
		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_SELECT_OUT_CLKING_MODE:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_SELECT_OUT_CLKING_MODE\n", device->minor);
		get_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
		if ((ulval != SEQUENTIAL) && (ulval != SIMULTANEOUS)) {
			device->error = GSC16AO_ERR_INVALID_PARAMETER;
			IOCTL_RETURN (-EIO);
		}
		regval = readlocal(device, BOARD_CTRL_REG);
		if (ulval==SIMULTANEOUS)
		{
			regval |= BCR_SIMULTANEOUS_OUTPUTS;
		}
		else
		{
			regval &= ~(BCR_SIMULTANEOUS_OUTPUTS);
		}
		writelocal(device,regval, BOARD_CTRL_REG);
		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_SELECT_CLK_SOURCE:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_SELECT_CLK_SOURCE\n", device->minor);
		get_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
		if ((ulval != EXTERNAL) && (ulval != INTERNAL)) {
			device->error = GSC16AO_ERR_INVALID_PARAMETER;
			IOCTL_RETURN (-EIO);
		}
		regval = readlocal(device, BUFFER_OPS_REG);
		if (ulval==EXTERNAL)
		{
			regval |= BOR_EXTERNAL_CLOCK;
		}
		else
		{
			regval &= ~(BOR_EXTERNAL_CLOCK);
		}
		writelocal(device,regval, BUFFER_OPS_REG);
		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_GET_CLK_STATUS:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_GET_CLK_STATUS\n", device->minor);
		put_user_ret(readlocal(device, BUFFER_OPS_REG)&BOR_CLOCK_READY, (unsigned long *)arg, (-EFAULT));
		IOCTL_RETURN (0);			

	case IOCTL_GSC16AO_SINGLE_OUTPUT_CLK_EVT:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_SINGLE_OUTPUT_CLK_EVT\n", device->minor);
		regval = readlocal(device, BUFFER_OPS_REG);
		regval |= BOR_SW_CLOCK;
		writelocal(device,regval, BUFFER_OPS_REG);
		IOCTL_RETURN (0);			
		
	case IOCTL_GSC16AO_SELECT_BUF_CONFIG:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_SELECT_BUF_CONFIG\n", device->minor);
		get_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
		if ((ulval != GSC16AO_OPEN_BUF) && (ulval != GSC16AO_CIRCULAR_BUF)) {
			device->error = GSC16AO_ERR_INVALID_PARAMETER;
			IOCTL_RETURN (-EIO);
		}
		regval = readlocal(device, BUFFER_OPS_REG);
		if (ulval==GSC16AO_CIRCULAR_BUF)
		{
			regval |= GSC16AO_CIRCULAR_BUF;
		}
		else
		{
			regval &= ~(GSC16AO_CIRCULAR_BUF);
		}
		writelocal(device,regval, BUFFER_OPS_REG);
		IOCTL_RETURN (0);			
	
	case IOCTL_GSC16AO_LOAD_ACCESS_REQ:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_LOAD_ACCESS_REQ\n", device->minor);
		regval = readlocal(device, BUFFER_OPS_REG);
		if ((regval & BOR_CIRCULAR_BUFFER) == 0) {
			device->error = GSC16AO_ERR_INVALID_PARAMETER;
			IOCTL_RETURN (-EIO);
		}
		regval |= BOR_LOAD_REQUEST | BOR_LOAD_READY;
		// set up to wait for the IRQ.`

		device->ioctl_wait_load_ready = TRUE;
	
		device->timeout=FALSE;
		device->watchdog_timer.expires=jiffies+device->timeout_seconds*HZ;
		add_timer(&device->watchdog_timer);
	
		writel(readl(IntCntrlStat(device)) | IRQ_PCI_ENABLE | IRQ_LOCAL_PCI_ENABLE, IntCntrlStat(device));
		wait_event_interruptible(device->ioctlwq,(!device->ioctl_wait_load_ready));
		writelocal(device,regval, BUFFER_OPS_REG);
		if (device->timeout)
		{
		  device->ioctl_wait_load_ready = FALSE;
		  device->error = GSC16AO_ERR_IOCTL_TIMEOUT;
		  device->timeout=FALSE;
		  IOCTL_RETURN (-EIO);
		}
		else
		  del_timer_sync(&device->watchdog_timer);
		
		if (device->timeout) {
		  prmsg(GDL_ERR, GDC_IOCTL, "(%d): channel setup timeout\n", device->minor);
		  device->ioctl_wait_load_ready = FALSE;
		  device->error = GSC16AO_ERR_IOCTL_TIMEOUT;
		  IOCTL_RETURN (-EIO);
		}
		IOCTL_RETURN (0);			
		
	case IOCTL_GSC16AO_GET_CIR_BUF_STATUS:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_GET_CIR_BUF_STATUS\n", device->minor);
		put_user_ret(readlocal(device, BUFFER_OPS_REG)&BOR_LOAD_READY, (unsigned long *)arg, (-EFAULT));
		IOCTL_RETURN (0);			

	case IOCTL_GSC16AO_CLEAR_BUFFER:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_CLEAR_BUFFER\n", device->minor);
		regval = readlocal(device, BUFFER_OPS_REG);
		regval |= BOR_CLEAR_BUFFER;
		writelocal(device,regval, BUFFER_OPS_REG);
		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_GET_DEVICE_TYPE:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_GET_DEVICE_TYPE\n", device->minor);
		put_user_ret(device->board_type, (unsigned long *)arg, (-EFAULT));
		IOCTL_RETURN (0);
		
	case IOCTL_GSC16AO_SET_TIMEOUT:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_SET_TIMEOUT\n", device->minor);
		get_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
		device->timeout_seconds = ulval;
		IOCTL_RETURN (0);

    case IOCTL_GSC16AO_DISABLE_PCI_INTERRUPTS:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_DISABLE_PCI_INTERRUPTS\n", device->minor);
        {
			regval = readl(IntCntrlStat(device));
			regval &= ~PCI_INT_ENABLE;
			writel(regval, (IntCntrlStat(device)));
        }
        IOCTL_RETURN (0);

    case IOCTL_GSC16AO_ENABLE_PCI_INTERRUPTS:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_ENABLE_PCI_INTERRUPTS\n", device->minor);
        {
			regval = readl(IntCntrlStat(device));
			regval |= PCI_INT_ENABLE;
			writel(regval, (IntCntrlStat(device)));
        }
        IOCTL_RETURN (0);

    case IOCTL_GSC16AO_GET_OFFSET:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_GET_OFFSET\n", device->minor);
		get_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
        switch(ulval) {
            case GSC16AO_GSC_REGS_MMAP_OFFSET:
	            ulval = device->PciBar[2].Physical.u.LowPart & (~PAGE_MASK); 
            break;
            case GSC16AO_PLX_REGS_MMAP_OFFSET:
	            ulval = device->PciBar[0].Physical.u.LowPart & (~PAGE_MASK); 
            break;
	        default:
	                prmsg(GDL_ERR, GDC_IOCTL, "(%d): Unknown Offset\n", device->minor);
		            IOCTL_RETURN (-EINVAL);
            break;
        }
        
		put_user_ret(ulval, (unsigned long *)arg, (-EFAULT));
        IOCTL_RETURN (0);

    case IOCTL_GSC16AO_GET_BOARD_INFO:
	        prmsg(GDL_TRACE1, GDC_IOCTL, "(%d): IOCTL_GSC16AO_GET_BOARD_INFO\n", device->minor);
	    copy_to_user_ret((void *)arg, &device->board_info, sizeof(board_info_t), (-EFAULT));
    break;

	default:
	        prmsg(GDL_ERR, GDC_IOCTL, "(%d): Unknown IOCTL code\n", device->minor);
		IOCTL_RETURN (-EINVAL);
	}
	IOCTL_RETURN (0);
}
