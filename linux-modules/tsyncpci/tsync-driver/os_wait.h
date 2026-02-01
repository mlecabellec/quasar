/*******************************************************************************
**
**  Module  : os_wait.h
**  Date    : 04/05/06
**  Purpose : Provides Linux OS support for waitaphores.
**
**  Copyright(C) 2006 Spectracom Corporation. All Rights Reserved.
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
**
*******************************************************************************/

#ifndef os_wait_INCLUDE
#define os_wait_INCLUDE

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,7,0)
#include <linux/kthread.h>
#endif

#include <linux/wait.h>  /* linux support for waitaphores */
#include <asm/atomic.h>  /* linux support for atomic operations */

#include "kernel_compat.h"


/*******************************************************************************
**          Defines
*******************************************************************************/
#define OS_WAIT_NO_WAIT          (0)  /* no wait */
#define OS_WAIT_WAIT_FOREVER    (-1)  /* wait forever */

#define OS_WAIT_STATUS_NG        (1)  /* status: Not Good */
#define OS_WAIT_STATUS_OK        (0)  /* status: OK */
#define OS_WAIT_STATUS_TIMEOUT  (-1)  /* status: TIMEOUT */


/*******************************************************************************
**          Waitaphore Object Definition
*******************************************************************************/
typedef struct wait_queue_head_t os_WaitObject, *os_WaitHandle;


/*******************************************************************************
**          Waitaphore Support Routines
*******************************************************************************/
#define os_waitInit(wait)   init_waitqueue_head((wait))
#define os_waitPost(wait)   wake_up_interruptible((wait))

int os_waitPend(wait_queue_head_t* wait, int timeout, atomic_t* cond);

#endif  /** os_wait_INCLUDE **/


