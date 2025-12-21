/************************************************************/
/*  DRIVER LINUX POUR LE COUPLEUR EMUABI                    */
/*                                                          */
/* Regroupement des directives "#include" utilisees         */
/* par le driver.                                           */
/*                                                          */
/*                      AIRBUS D&S - TE343 - Yves Guillemot */
/************************************************************/

/*
Quand       Qui   Quoi
----------  ----  -------------------------------------------------------------
13/06/2014  yg    - Version initiale

 */

#ifndef EMUABI_DRIVERINCLUDES_H_
#define EMUABI_DRIVERINCLUDES_H_

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
#include <asm/io.h>
#include <asm/ioctls.h>
#include <asm/uaccess.h>
#include <linux/blkdev.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/stringify.h>

#include "asdc.h"           /* Structures de donnees et codes ioctl */
#include "asdc_statics.h"   /* Variables statiques */
#include "interfaces.h"     /* Macros de portage LynxOS/Linux et prototypes */
#include "asdcalloc.h"      /* Prototypes fonctions d'alloc. en mem. echange */
#include "interface_vfx70.h"
#include "do_acces.h"
#include "asdcExport.h"
#include "version.h"
#include "msg_debug.h"

#endif /* EMUABI_DRIVERINCLUDES_H_ */




