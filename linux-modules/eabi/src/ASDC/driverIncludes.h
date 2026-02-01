/************************************************************/
/*  DRIVER LINUX POUR LE COUPLEUR EMUABI                    */
/*                                                          */
/* Regroupement des directives "#include" utilisees         */
/* par le driver.                                           */
/*                                                          */
/*                      AIRBUS D&S - TE343 - ************ Anonymized */
/************************************************************/

/*
Quand       Qui   Quoi
----------  ----  -------------------------------------------------------------
13/06/2014  yg    - Version initiale

 */

#ifndef EMUABI_DRIVERINCLUDES_H_
#define EMUABI_DRIVERINCLUDES_H_

#include <asm/io.h>
#include <asm/ioctls.h>
#include <asm/uaccess.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/stringify.h>
#include <linux/types.h>
#include <linux/wait.h>

#include "asdc.h" /* Structures de donnees et codes ioctl */
#include "asdcExport.h"
#include "asdc_statics.h" /* Variables statiques */
#include "asdcalloc.h"    /* Prototypes fonctions d'alloc. en mem. echange */
#include "do_acces.h"
#include "interface_vfx70.h"
#include "interfaces.h" /* Macros de portage LynxOS/Linux et prototypes */
#include "msg_debug.h"
#include "version.h"

#endif /* EMUABI_DRIVERINCLUDES_H_ */
