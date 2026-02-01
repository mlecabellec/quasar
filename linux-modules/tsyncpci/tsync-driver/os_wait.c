/*******************************************************************************
**
**  Module  : os_wait.c
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
#include <asm/io.h>
#include <linux/errno.h>
#include "os_wait.h"

#ifdef HAVE_SIGNAL_FUNCTIONS_OWN_HEADER
#include <linux/sched/signal.h>
#else
#include <linux/sched.h>
#endif


/*******************************************************************************
**          Defines
*******************************************************************************/

/*******************************************************************************
  mywait_event_interruptible

  This function is modified from the original (sched.h) to take the
  wait_queue_head as a pointer and not an object.
*******************************************************************************/

#define __mywait_event_interruptible(wq, condition, ret)    \
do {                                                        \
    wait_queue_t __wait;                                    \
    init_waitqueue_entry(&__wait, current);                 \
                                                            \
    __add_wait_queue(wq, &__wait);                          \
    for (;;) {                                              \
        set_current_state(TASK_INTERRUPTIBLE);              \
        if (condition)                                      \
            break;                                          \
        if (!signal_pending(current)) {                     \
            schedule();                                     \
            continue;                                       \
        }                                                   \
        ret = -ERESTARTSYS;                                 \
        break;                                              \
    }                                                       \
    set_current_state(TASK_RUNNING);                        \
    __remove_wait_queue(wq, &__wait);                       \
} while (0)

#define mywait_event_interruptible(wq, condition)           \
({                                                          \
    int __ret = 0;                                          \
    if (!(condition))                                       \
        __mywait_event_interruptible(wq, condition, __ret); \
    __ret;                                                  \
})


/*******************************************************************************
  myinterruptible_sleep_on_timeout

  This function was modified from the original (sched.c) to take and
  check condition before calling schedule_timeout.
*******************************************************************************/
#define SLEEP_ON_VAR        \
    unsigned long flags;    \
    wait_queue_t wait;      \
    init_waitqueue_entry(&wait, current);

#define SLEEP_ON_HEAD                   \
    spin_lock_irqsave(&q->lock,flags); \
    __add_wait_queue(q, &wait);         \
    spin_unlock(&q->lock);

#define	SLEEP_ON_TAIL                       \
    spin_lock_irq(&q->lock);               \
    __remove_wait_queue(q, &wait);          \
    spin_unlock_irqrestore(&q->lock,flags);


long myinterruptible_sleep_on_timeout(wait_queue_head_t *q,
                                      int                timeout,
                                      atomic_t          *cond)
{
    SLEEP_ON_VAR

    set_current_state(TASK_INTERRUPTIBLE);

    SLEEP_ON_HEAD
    if (atomic_read(cond))  return timeout;
    timeout = schedule_timeout(timeout);
    SLEEP_ON_TAIL

    return timeout;
}

/***************************************************************************
**          OS_WAIT GLOBAL FUNCTIONS
***************************************************************************/

/*******************************************************************************
**
** Function:    os_waitPend()
** Description: This function performes waitaphore pend operation.  The timout
**              operation is performed with resolution of a jiffy (1 / HZ).
**
** Parameters:
**     IN: *wait    - Head of the wait queue
**          timeout - Wait timeout
**         *cond    - atomic_t count
**
**     RETURNS: 0 - Success, obtained waitaphore
**              1 - no waitaphore, timed out
**
*******************************************************************************/
int os_waitPend(wait_queue_head_t* wait, int timeout, atomic_t *cond)
{
    int status = OS_WAIT_STATUS_NG;

    switch (timeout) {

    case OS_WAIT_NO_WAIT:
        /*----------------------------------------------------------------------
        NO WAIT

        This checks the waitaphore state and returns immiediatelly
        with waitaphore status.  Since there is no good way of checking
        the wait queue status, the condition is checked instead.
        ----------------------------------------------------------------------*/
        /* check the condition */
        status = atomic_read(cond);

        if (status == 0)
            status = OS_WAIT_STATUS_NG;
        else
            status = OS_WAIT_STATUS_OK;

        break;


    case OS_WAIT_WAIT_FOREVER:
        /*----------------------------------------------------------------------
        WAIT FOREVER

        This would wait until a condition is TRUE or wait signal is
        received.
        ----------------------------------------------------------------------*/
        status = mywait_event_interruptible(wait, (atomic_read(cond)));

        if (status == -ERESTARTSYS)
            status = OS_WAIT_STATUS_NG;
        else
            status = OS_WAIT_STATUS_OK;

        break;


    default: {
        /*----------------------------------------------------------------------
        TIMED

        This function is based on myinterruptible_sleep_on_timeout
        function (defined above).  This would check for the condition to
        be TRUE and then wait for a wait queue signal for a specified time
        in jiffies.
        ----------------------------------------------------------------------*/
        unsigned long flags;
        wait_queue_t  waitQueue;

        init_waitqueue_entry(&waitQueue, current);

        spin_lock_irqsave(&wait->lock,flags);
        __add_wait_queue(wait, &waitQueue);
        spin_unlock_irqrestore(&wait->lock,flags); // POTENTIAL BUG FIX - since we are waiting for an interrupt dont you need to restore them here for uni-cpu systems?

        set_current_state(TASK_INTERRUPTIBLE);

        /* check the condition */
        if (atomic_read(cond)) {
            status = timeout;
            goto remove_queue;
        }

        timeout = schedule_timeout(timeout);

        remove_queue:
        set_current_state(TASK_RUNNING);  // BUG FIX - need to wake the thread manually in case the goto in the if statement above got us here

        spin_lock_irqsave(&wait->lock,flags);
        __remove_wait_queue(wait, &waitQueue);
        spin_unlock_irqrestore(&wait->lock,flags);


        /* check the timeout status (and condition) */
        if (timeout == 0   &&  !atomic_read(cond))
            status = OS_WAIT_STATUS_TIMEOUT;
        else
            status = OS_WAIT_STATUS_OK;

        break;
    }  /* end - default */

    }  /* end - switch */


    /* return status */
    return (status);

} /* End - os_waitPend() */
