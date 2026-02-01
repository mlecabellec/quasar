/********************************************************************/
/* Driver Linux pour carte E/S TOR PCIe-6509 de National Instrument */
/*                                                                  */
/* DriverIncludes.h : Regroupement des directives "#include"        */
/* utilisees par le driver.                                         */
/*                                                                  */
/*                             ************ - ************ - ************
 * Anonymized */
/********************************************************************/
/************************************************************/

/*
Quand       Qui   Quoi
----------  ----  -------------------------------------------------------------
 4/12/2013  yg    - Version initiale

*/

#ifndef PCIE6509_DRIVERINCLUDES_H_
#define PCIE6509_DRIVERINCLUDES_H_

#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/param.h>
#include <linux/pci.h>
#include <linux/sysctl.h>
#include <linux/version.h>
#include <linux/wait.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
#include <linux/sched/signal.h>
#include <linux/uaccess.h>
#else
#include <linux/sched.h>
#endif

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kdev_t.h>
#include <linux/types.h>

#include "pcie6509.h"
#include "pcie6509Driver.h"
#include "pcie6509_version.h"

#endif /* PCIE6509_DRIVERINCLUDES_H_ */
