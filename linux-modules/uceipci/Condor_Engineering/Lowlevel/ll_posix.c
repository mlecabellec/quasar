/*============================================================================*
 * FILE:                       L L _ P O S I X . C
 *============================================================================*
 *
 *      COPYRIGHT (C) 2006 - 2017 BY ABACO SYSTEMS, INC.
 *      ALL RIGHTS RESERVED.
 *
 *      THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND
 *      COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND WITH
 *      THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR ANY
 *      OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
 *      AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
 *      SOFTWARE IS HEREBY TRANSFERRED.
 *
 *      THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
 *      NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY ABACO SYSTEMS.
 *
 *===========================================================================*
 *
 * FUNCTION:   Low-level POSIX pthread interface functions
 *
 * EXTERNAL ENTRY POINTS:
 *
 *  Mutex functions:
 *   CEI_MUTEX_CREATE      -  create a mutex
 *   CEI_MUTEX_CREATE_SHARED - create a shared mutex
 *   CEI_MUTEX_LOCK        -  lock a mutex
 *   CEI_MUTEX_TRYLOCK     -  try locking a mutex
 *   CEI_MUTEX_TRYLOCK_TIMEOUT - try locking a mutex with a timeout
 *   CEI_MUTEX_UNLOCK      -  unlock a mutex
 *   CEI_MUTEX_DESTROY     -  destroy a mutex
 *
 *  Event functions:
 *   CEI_EVENT_CREATE      -  creates an event
 *   CEI_EVENT_SHARED_CREATE - create a shared event 
 *   CEI_EVENT_DESTROY     -  destroy an event
 *   CEI_WAIT_FOR_EVENT    -  wait (infinite or timed) for event
 *   CEI_EVENT_SIGNAL      -  signal an event
 *
 *  Thread functions:
 *   CEI_THREAD_CREATE     -  create a thread
 *   CEI_THREAD_DESTROY    -  destroys a thread
 *   CEI_TIMER             -  create and destroy timer (ms)
 *   CEI_SIGNAL            -  create and destroy a real-time signal
 *   CEI_THREAD_EXIT       -  exit thread
 *   CEI_HANDLE_DESTROY    -  close a handle
 *
 *  Shared memory functions:
 *   CEI_SHM_CREATE        - create/open a shared memory object
 *   CEI_SHM_OPEN          - open an existing shared memory object
 *   CEI_SHM_CLOSE         - close a shared memory object
 *   CEI_SHM_DESTROY       - destroy a shared memory object
 *
 *  Semaphore functions:
 *   CEI_SEM_CREATE        - create/open a semaphore
 *   CEI_SEM_OPEN          - open an existing semaphore
 *   CEI_SEM_CLOSE         - close a semaphore
 *   CEI_SEM_LOCK          - lock a semaphore
 *   CEI_SEM_UNLOCK        - unlock a semaphore
 *   CEI_SEM_GETVALUE      - get the value of a semaphore 
 *   CEI_SHM_DESTROY       - destroy a semaphore
 *
 *  Message Queue functions (not supported on kernels before 2.6.6):
 *   CEI_MSG_QUEUE_CREATE  - create a message queue
 *   CEI_MSG_QUEUE_OPEN    - open a message queue
 *   CEI_MSG_QUEUE_CLOSE   - close a message queue
 *   CEI_MSG_QUEUE_SEND    - send a message to a message queue
 *   CEI_MSG_QUEUE_RECEIVE - receive a message from a message queue
 *   CEI_MSG_QUEUE_RECEIVE_TIMEOUT - timeout waiting to receive a message from
 *                                   a message queue
 *   CEI_MSG_QUEUE_EXIT    - destroy a message queue

 *===========================================================================*/

/* $Revision:  1.13 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  02/09/2006    Initial. bch
  05/15/2006    modified time delay for CEI_WAIT_FOR_EVENT, modified CEI_SIGNAL
                 argument list, modified CEI_SIGNAL signal configuration. bch
  09/14/2006    replaced BTD_OK with POSIX_SUCCESS. bch
  03/13/2007    added CEI_MUTEX_LOCK, CEI_MUTEX_TRYLOCK, and CEI_MUTEX_UNLOCK
                 functions. moved debugging macros from posix.h. bch
  05/18/2007    changed iDelay in CEI_WAIT_FOR_EVENT from CEI_UINT to CEI_INT. bch
  06/15/2007    modified CEI_WAIT_FOR_EVENT and CEI_MUTEX_CREATE. bch
  11/18/2008    modified CEI_MUTEX_CREATE, CEI_EVENT_SIGNAL, and 
                 CEI_THREAD_DESTROY. bch
  02/27/2009    added "lowlevel.h". bch
  03/26/2009    replaced "lowlevel.h" with "cei_types.h". replaced "CEI_U32"
                 with "CEI_UINT". bch
  07/19/2012    added CEI_THREAD_EXIT and CEI_HANDLE_DESTROY. bch
  08/28/2014    added CEI_MUTEX_CREATE_SHARED, CEI_SHM_CREATE, CEI_SHM_OPEN,
                 CEI_SHM_CLOSE, CEI_SHM_DESTROY, CEI_SEM_CREATE, CEI_SEM_OPEN,
                 CEI_SEM_CLOSE, CEI_SEM_LOCK, CEI_SEM_UNLOCK, CEI_SEM_GETVALUE,
                 CEI_SEM_DESTROY, CEI_MSG_QUEUE_CREATE, CEI_MSG_QUEUE_OPEN,
                 CEI_MSG_QUEUE_CLOSE, CEI_MSG_QUEUE_EXIT, CEI_MSG_QUEUE_SEND,
                 CEI_MSG_QUEUE_RECEIVE, CEI_MSG_QUEUE_RECEIVE_TIMEOUT,
                 CEI_EVENT_SHARED_CREATE. bch
  09/17/2014    added CEI_MUTEX_TRYLOCK_TIMEOUT. bch
  04/10/2015    modified CEI_MUTEX_CREATE and CEI_MUTEX_SHARED_CREATE. bch
  09/27/2017    added NO_MSGQ_SUPPORT to disable message queue functions. bch
*/
#include <stddef.h>
#include "cei_types.h"
#include "ll_posix.h"


/****************************************************************
*
*  PROCEDURE - CEI_MUTEX_CREATE
*
*  FUNCTION
*     Creates a pthread mutex.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_MUTEX_CREATE(CEI_MUTEX *pMutex) {
   CEI_INT status=0;
   pthread_mutexattr_t pt_mattr;

   DBG_VAL1(pMutex);

//   memset(pMutex, 0, sizeof(CEI_MUTEX));

   if((status = pthread_mutexattr_init(&pt_mattr)) != 0) {
     DBG_CHK_STATUS("pthread_mutexattr_init", status);
   }
   else {
//   status = pthread_mutexattr_setprotocol(&pt_mattr, PTHREAD_PRIO_NONE);
//   DBG_CHK_STATUS("pthread_mutexattr_setprotocol", status);
     
   #ifdef _GNU_SOURCE
    #ifdef LL_DEBUG
     if((status = pthread_mutexattr_settype(&pt_mattr, PTHREAD_MUTEX_ERRORCHECK)) != 0) {
    #else
     if((status = pthread_mutexattr_settype(&pt_mattr, PTHREAD_MUTEX_NORMAL)) != 0) {
    #endif
       DBG_CHK_STATUS("pthread_mutexattr_settype", status);
     }
   #endif
     if(status == 0) {
        if((status = pthread_mutex_init(pMutex, &pt_mattr)) != 0) {
         DBG_CHK_STATUS("pthread_mutex_init", status);
       }
     }
     if((status = pthread_mutexattr_destroy(&pt_mattr)) != 0) {
       DBG_CHK_STATUS("pthread_mutexattr_destroy", status);
     }
   }

   if(status != 0)
     return POSIX_ERROR;

   return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_MUTEX_SHARED_CREATE
*
*  FUNCTION
*     Creates a shared pthread mutex to be access by multiple
*     processes
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_MUTEX_SHARED_CREATE(CEI_MUTEX *pMutex) {
  CEI_INT status=0;
  pthread_mutexattr_t pt_mattr;

  DBG_VAL1(pMutex);

  memset(pMutex, 0, sizeof(CEI_MUTEX));

  if((status = pthread_mutexattr_init(&pt_mattr)) != 0) {
    DBG_CHK_STATUS("pthread_mutexattr_init", status);
  }
  else {
//   status = pthread_mutexattr_setprotocol(&pt_mattr, PTHREAD_PRIO_NONE);
//   DBG_CHK_STATUS("pthread_mutexattr_setprotocol", status);

    if((status = pthread_mutexattr_setpshared(&pt_mattr, PTHREAD_PROCESS_SHARED)) != 0) {
      DBG_CHK_STATUS("pthread_mutexattr_setpshared", status);
    }
    else {
    #ifdef _GNU_SOURCE
     #ifdef LL_DEBUG
      if((status = pthread_mutexattr_settype(&pt_mattr, PTHREAD_MUTEX_ERRORCHECK)) != 0) {
     #else
      if((status = pthread_mutexattr_settype(&pt_mattr, PTHREAD_MUTEX_NORMAL)) != 0) {  //PTHREAD_MUTEX_DEFAULT
     #endif
        DBG_CHK_STATUS("pthread_mutexattr_settype", status);
      }
    #endif
     if(status == 0) {
       if((status = pthread_mutex_init(pMutex, &pt_mattr)) != 0) {
         DBG_CHK_STATUS("pthread_mutex_init", status);
       }
      }
    }
    if((status = pthread_mutexattr_destroy(&pt_mattr)) != 0) {
      DBG_CHK_STATUS("pthread_mutexattr_destroy", status);
    }
  }

  if(status != 0)
    return POSIX_ERROR;

  return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_MUTEX_LOCK
*
*  FUNCTION
*     Lock a pthread mutex.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_MUTEX_LOCK(CEI_MUTEX *pMutex) {
   CEI_INT status=0;

   DBG_VAL1(pMutex);

   status = pthread_mutex_lock(pMutex);
   DBG_CHK_STATUS("pthread_mutex_lock", status);

   if(status == EINVAL)
     return POSIX_ERROR;

   return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_MUTEX_TRYLOCK
*
*  FUNCTION
*     Try locking a pthread mutex, if locked returns POSIX_EBUSY
*
*  RETURNS
*     POSIX_EBUSY
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_MUTEX_TRYLOCK(CEI_MUTEX *pMutex) {
  CEI_INT status=0;

  DBG_VAL1(pMutex);

  status = pthread_mutex_trylock(pMutex);
  DBG_CHK_STATUS("pthread_mutex_trylock", status);

  if(status == EINVAL)
    return POSIX_ERROR;
  if(status == EBUSY)
    return POSIX_EBUSY;

  return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_MUTEX_TRYLOCK_TIMEOUT
*
*  FUNCTION
*     Try locking a pthread mutex within the specified
*     microsecond timeout.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_TIMEOUT
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_MUTEX_TRYLOCK_TIMEOUT(CEI_MUTEX *pMutex, CEI_INT timeout) {
  CEI_INT status=0;
  clock_t to_us=0;
  static CEI_INT size_clock_t=sizeof((clock_t *)0);

  DBG_VAL1(pMutex);

  // calculate the timeout
  if((to_us = clock()) == -1)
    return POSIX_ERROR;
  // handle a rollover
  if((to_us + timeout) > size_clock_t)
    to_us = timeout - (size_clock_t - to_us);
  else
    to_us += timeout;

  while(1) {
    status = pthread_mutex_trylock(pMutex);
    DBG_CHK_STATUS("pthread_mutex_trylock", status);
    if(status == 0)
      break;
    else if(status == EINVAL)
      return POSIX_ERROR;
    else if(status == EBUSY) {
      if(clock() >= to_us)
        return POSIX_TIMEOUT;
      continue;
    }
  }

  return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_MUTEX_UNLOCK
*
*  FUNCTION
*     Unlock a pthread mutex.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_MUTEX_UNLOCK(CEI_MUTEX *pMutex) {
  CEI_INT status=0;

  DBG_VAL1(pMutex);

  status = pthread_mutex_unlock(pMutex);
  DBG_CHK_STATUS("pthread_mutex_unlock", status);

  if(status == EINVAL)
    return POSIX_ERROR;

  return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_MUTEX_DESTROY
*
*  FUNCTION
*     Destroys a pthread mutex.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_MUTEX_DESTROY(CEI_MUTEX *pMutex) {
  CEI_INT status=0;

  DBG_VAL1(pMutex);

  status = pthread_mutex_destroy(pMutex);
  DBG_CHK_STATUS("pthread_mutex_destroy", status);

  if(status != 0)
    return POSIX_ERROR;

  memset(pMutex, 0, sizeof(CEI_MUTEX));

  return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_EVENT_CREATE
*
*  FUNCTION
*     Create a pthread event.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_EVENT_CREATE(CEI_EVENT *pEvent) {
  CEI_INT status=0;

  DBG_VAL1(pEvent);

  memset(pEvent, 0, sizeof(CEI_EVENT));

  status = pthread_cond_init(pEvent, NULL);
  DBG_CHK_STATUS("pthread_cond_init", status);
  if(status != 0)
    return POSIX_ERROR;

  return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_EVENT_SHARED_CREATE
*
*  FUNCTION
*     Creates a shared pthread conditional to be access by multiple
*     processes
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_EVENT_SHARED_CREATE(CEI_EVENT *pEvent) {
  CEI_INT status=0;
  pthread_condattr_t pt_cattr;

  DBG_VAL1(pEvent);

  if((status = pthread_condattr_init(&pt_cattr)) != 0) {
    DBG_CHK_STATUS("pthread_condattr_setpshared", status);
  }
  else {
    if((status =  pthread_condattr_setpshared(&pt_cattr, PTHREAD_PROCESS_SHARED)) != 0) {
      DBG_CHK_STATUS("pthread_condattr_setpshared", status);
    }
    else {
      if((status = pthread_cond_init(pEvent, &pt_cattr)) != 0) {
        DBG_CHK_STATUS("pthread_cond_init", status);
      }
    }
    pthread_condattr_destroy(&pt_cattr);
  }

  if(status != 0)
    return POSIX_ERROR;

  return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_EVENT_DESTROY
*
*  FUNCTION
*     Destroys a pthread event.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_EVENT_DESTROY(CEI_EVENT *pEvent) {
  CEI_INT status=0;

  DBG_VAL1(pEvent);

  status = pthread_cond_destroy(pEvent);
  DBG_CHK_STATUS("pthread_cond_destroy", status);

  if(status != 0)
    return POSIX_ERROR;

  memset(pEvent, 0, sizeof(CEI_EVENT));

  return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_WAIT_FOR_EVENT
*
*  FUNCTION
*     Waits for specified event.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
*     PTHREAD_COND_TIMEOUT
****************************************************************/
CEI_INT CEI_WAIT_FOR_EVENT(CEI_EVENT *pEvent, CEI_INT iDelay, CEI_MUTEX *pMutex) {
  CEI_INT ret=POSIX_SUCCESS,status=POSIX_SUCCESS;
  CEI_ULONG val_ns=0;
  struct timespec ts;

  DBG_VAL1(pEvent);
  
  ret = CEI_MUTEX_LOCK(pMutex);
  DBG_CHK_STATUS("CEI_MUTEX_LOCK", ret);

  if(iDelay == INFINITE) {
    // block the calling thread until signaled. Sequence:  lock mutex,
    // mutex is released while the thread waits, mutex locked again once
    // thread is signaled, and finally mutex is unlocked.
    ret = pthread_cond_wait(pEvent, pMutex);
    DBG_CHK_STATUS("pthread_cond_wait", ret);
    if(ret != 0)
      status = POSIX_ERROR;
  }
  else {
    if(clock_gettime(CLOCK_REALTIME, &ts) == 0) {
      val_ns = (iDelay*1000000) + ts.tv_nsec;
      ts.tv_sec += (val_ns/1000000000);
      ts.tv_nsec = (val_ns%1000000000);
      ret = pthread_cond_timedwait(pEvent, pMutex, &ts);
      if(ret == ETIMEDOUT) {
       #ifdef LL_DEBUG
        printf(" <LL_DEBUG> timeout in pthread_cond_timedwait\n"); 
       #endif
        status = POSIX_TIMEOUT;
      }
      else {
        DBG_CHK_STATUS("pthread_cond_timedwait", ret);
      }
    }
    else {
      printf("Error in clock_gettime - errno: %d\n",  errno);
      status = POSIX_ERROR;
    }
  }
  ret = CEI_MUTEX_UNLOCK(pMutex);
  DBG_CHK_STATUS("pthread_mutex_unlock", ret);

  return status;
}

/****************************************************************
*
*  PROCEDURE - CEI_EVENT_SIGNAL
*
*  FUNCTION
*     Sets the specified event for all waiting threads.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_EVENT_SIGNAL(CEI_EVENT *pEvent, CEI_MUTEX *pMutex) {
   CEI_INT status=0;

   DBG_VAL1(pEvent);

   status = pthread_cond_signal(pEvent);
   DBG_CHK_STATUS("pthread_cond_signal", status);

   if(status != 0)
     return POSIX_ERROR;

   return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_THREAD_CREATE
*
*  FUNCTION
*     Create and config a POSIX Thread.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_THREAD_CREATE(CEI_THREAD *pThread, CEI_INT iPriority, CEI_VOID *pFunc, CEI_VOID *pFuncArg) {
  CEI_INT ret=0, status=0;
  pthread_attr_t pt_attr;
 #ifndef KERNEL_24
  CEI_INT policy=0, prio_min=0, prio_max=0;
  struct sched_param pt_param;
 #endif

  DBG_VAL1(pThread);

  memset(pThread, 0, sizeof(CEI_THREAD));

  // configure pthread attribute
  status = pthread_attr_init(&pt_attr);
  DBG_CHK_STATUS("pthread_attr_init", status);
  if(status != 0)
    return POSIX_ERROR;

  status = pthread_attr_setinheritsched(&pt_attr, PTHREAD_EXPLICIT_SCHED);  // PTHREAD_INHERIT_SCHED
  DBG_CHK_STATUS("pthread_attr_setinheritsched", status);

  status = pthread_attr_setdetachstate(&pt_attr, PTHREAD_CREATE_DETACHED);  // PTHREAD_CREATE_JOINABLE
  DBG_CHK_STATUS("pthread_attr_setdetachstate", status);

  // create thread, if caller does not have permissions then try with default attributes
  ret = pthread_create(pThread, &pt_attr, pFunc, pFuncArg);
  DBG_CHK_STATUS("pthread_create", ret);
  if(ret == EPERM) {
   #ifdef LL_DEBUG
    printf(" <LL_DEBUG> using default attributes for pthread_create\n");
   #endif
    ret = pthread_create(pThread, NULL, pFunc, pFuncArg);
    DBG_CHK_STATUS("pthread_create (default)", ret);
  }

 #ifndef KERNEL_24
  status = pthread_getschedparam(*pThread, &policy, &pt_param);
  DBG_CHK_STATUS("pthread_getschedparam", status);
  if(status == 0) {
    prio_min = sched_get_priority_min(policy);
    DBG_CHK_STATUS("sched_get_priority_min", prio_min);
    if(prio_min != -1) {
      if(iPriority < prio_min)
        iPriority = prio_min;
    }
    prio_max = sched_get_priority_max(policy);
    DBG_CHK_STATUS("sched_get_priority_max", prio_max);
    if(prio_max != -1) {
      if(iPriority > prio_max)
        iPriority = prio_max;
    }
    status = pthread_setschedprio(*pThread, iPriority);
    DBG_CHK_STATUS("pthread_setschedprio", status);
  }
 #endif

 #ifdef LL_DEBUG
  if(ret == 0) {
    ret = pthread_getschedparam(*pThread, &policy, &pt_param);
    DBG_CHK_STATUS("pthread_getschedparam", ret);
    if(ret == 0) {
      // policy:  refer to `sched_policies.h`
      printf(" <LL_DEBUG> CEI_THREAD schedule attributes: policy (0x%lx), priority (%d)\n", (CEI_ULONG)policy, pt_param.sched_priority);
    }
  }
 #endif

   status = pthread_attr_destroy(&pt_attr);
   DBG_CHK_STATUS("pthread_attr_destroy", status);

   if(ret != 0)
     return POSIX_ERROR;

   return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_THREAD_DESTROY
*
*  FUNCTION
*     Destroys a thread.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_THREAD_DESTROY(CEI_THREAD *pThread) {
   CEI_INT status=0;

   DBG_VAL1(pThread);

   // if thread exists, then cancel it  
   if(pthread_kill(*pThread, 0) == 0) { 
     status = pthread_cancel(*pThread);
     DBG_CHK_STATUS("pthread_cancel", status);
   }

   if(status != 0)
     return POSIX_ERROR;

   memset(pThread, 0, sizeof(CEI_THREAD));

   return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_THREAD_EXIT
*
*  FUNCTION
*     Terminates the thread returning the value provided. This 
*     will not close the thread's handle.
*
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_THREAD_EXIT(CEI_VOID *pStatus, CEI_VOID *pVal) {
  DBG_VAL1(pStatus);

  if(pStatus == NULL)
    return POSIX_ERROR;

  pthread_exit(pStatus);

  return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_TIMER
*
*  FUNCTION
*     Creates or destroys a POSIX timer which calls a user
*     supplied handler at the specified timing interval.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_TIMER(CEI_UINT cflag, CEI_UINT interval, CEI_VOID *pFunc) {
   static timer_t timer_id;
   struct sigaction act;
   static struct sigaction old_act;
   struct itimerspec value;
   struct sigevent evp;
   static CEI_UINT cnfg=0;
  #ifdef LL_DEBUG
   struct timespec res;
  #endif

   DBG_VAL1((cnfg << 8) | cflag);

   switch(cflag) {
     case CEI_CREATE:
      if(cnfg != 0)
        break;
      // set function to call for signal CEI_SIG_TIMER
      sigemptyset(&act.sa_mask);
      act.sa_sigaction = pFunc;
      act.sa_flags = SA_RESTART | SA_SIGINFO;
//      sigemptyset(&act.sa_mask);
      if(sigaction(CEI_SIG_TIMER, &act, &old_act) == -1) {
       #ifdef LL_DEBUG
        printf(" <LL_DEBUG> CEI_TIMER - failed sigaction for signal %d, errno: %d\n", CEI_SIG_TIMER, errno);  // EINVAL, EFAULT, EINTR
       #endif
        return POSIX_ERROR;
      }
      cnfg = 0x1;

     #ifdef LL_DEBUG
      if(clock_getres(CLOCK_REALTIME, &res) == -1)
        printf("Failed clock_getres, errno: %d\n", errno);  // EPERM, EINVAL, EFAULT
      else
        printf(" <LL_DEBUG> CEI_TIMER - realtime clock resolution: %ld ns\n", res.tv_nsec);
     #endif

      // create a timer for polling the board by using the CLOCK_REALTIME clock id
      evp.sigev_signo = CEI_SIG_TIMER;
      evp.sigev_value.sival_int = getpid();
      evp.sigev_notify = SIGEV_SIGNAL;
      if(timer_create(CLOCK_REALTIME, &evp, &timer_id) == -1) {
       #ifdef LL_DEBUG
        printf(" <LL_DEBUG> CEI_TIMER - failed timer_create, errno: %d\n", errno);  // EAGAIN, EINVAL, ENOTSUP
       #endif
        CEI_TIMER(CEI_DESTROY, 0, NULL);
        return POSIX_ERROR;
      }
      cnfg |= 0x2;

      // set the timer time
      value.it_value.tv_sec = value.it_interval.tv_sec = 0;
      value.it_value.tv_nsec = value.it_interval.tv_nsec  = interval * 1000000;  // convert milli- to nano-seconds
      if(timer_settime(timer_id, 0, &value, NULL) == -1) {
       #ifdef LL_DEBUG     
        printf(" <LL_DEBUG> CEI_TIMER - failed timer_settime, errno: %d\n", errno);  // EINVAL
       #endif
        CEI_TIMER(CEI_DESTROY, 0, NULL);
        return POSIX_ERROR;
      }
      break;
     case CEI_DESTROY:
      if(cnfg & 0x2) {
        if(timer_delete(timer_id) == -1) {
         #ifdef LL_DEBUG
          printf(" <LL_DEBUG> CEI_TIMER - failed timer_delete, errno: %d\n", errno);  // EINVAL
         #endif
          return POSIX_ERROR;
        }
        cnfg &= ~0x2;
      }
      if(cnfg & 0x1) {
        if((sigaction(CEI_SIG_TIMER, &old_act, NULL)) == -1) {
         #ifdef LL_DEBUG
          printf(" <LL_DEBUG> CEI_TIMER - failed to restore previous sigaction for signal %d, errno: %d\n", CEI_SIG_TIMER, errno);  // EINVAL, EFAULT, EINTR
         #endif
        }
        cnfg &= ~0x1;
      }
      break;
     default:
      printf("CEI_TIMER - invalid cflag.\n");
      return POSIX_ERROR;
   };

   return 0;
}

/****************************************************************
*
*  PROCEDURE - CEI_SIGNAL
*
*  FUNCTION
*     Sets or unsets a user supplied handler to a specified
*     signal.
*
*  RETURNS
*     POSIX_SUCCESS
*     POSIX_ERROR
****************************************************************/
CEI_INT CEI_SIGNAL(CEI_UINT cflag, CEI_INT signal, CEI_VOID *pFunc) {
  struct sigaction act;
  static struct sigaction old_act;
  static CEI_UINT cnfg_sig=0;
  CEI_UINT cur_sig=0;

  DBG_VAL2(cflag, cnfg_sig);

  if(signal < ((CEI_UINT)CEI_SIG_HINT) || signal > ((CEI_UINT)CEI_SIG_HINT_MAX))
    return POSIX_ERROR;

  cur_sig = (0x1 << (signal - ((CEI_UINT)CEI_SIG_HINT)));
  switch(cflag) {
    case CEI_CREATE:
     if((cnfg_sig & cur_sig) == 1)
       return 0;
     // signal handling function for hardware interrupts (signal from the device driver)
     act.sa_sigaction = pFunc; // user's handler
     act.sa_flags = SA_RESTART | SA_SIGINFO;
     sigemptyset(&act.sa_mask);
//     sigaddset(&act.sa_mask, signal);  // block signal temporarily
     if(sigaction(signal, &act, &old_act) == -1) {
      #ifdef LL_DEBUG   
       printf(" <LL_DEBUG> CEI_SIGNAL - failed sigaction for signal %d, errno: %d\n", signal, errno);  // EINVAL, EFAULT, EINTR
      #endif
       return POSIX_ERROR;
     }
     cnfg_sig |= cur_sig;
     break;
    case CEI_DESTROY:
     if((cnfg_sig & cur_sig) == 0)
       return 0;
     if(sigaction(signal, &old_act, NULL) == -1) {
      #ifdef LL_DEBUG   
       printf(" <LL_DEBUG> CEI_SIGNAL - failed to restore previous sigaction for signal %d, errno: %d\n", signal, errno);  // EINVAL, EFAULT, EINTR
      #endif
     }
     cnfg_sig &= ~cur_sig;
     break;
    default:
     printf("CEI_SIGNAL - invalid cflag.\n");
     return POSIX_ERROR;
  };

  return 0;
}


/****************************************************************
*
*  PROCEDURE - CEI_HANDLE_DESTROY
*
*  FUNCTION
*
*  RETURNS
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_HANDLE_DESTROY(CEI_VOID *pHandle) {
  DBG_VAL1(pHandle);
  return POSIX_SUCCESS;
}


#ifndef NO_MSGQ_SUPPORT
/****************************************************************
*
*  PROCEDURE - CEI_MSG_QUEUE_CREATE
*
*  FUNCTION
*     Create a message queue with attributes, if none provide then uses the default
*
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_MSG_QUEUE_CREATE(CEI_CHAR *name, CEI_MSG_QUEUE_ATTR *attr, CEI_MSGQ *mqd) {
  struct mq_attr _attr, *_pattr=NULL;
  CEI_INT oflag=0;

  DBG_VAL1(mqd);

  if((name == NULL) || (attr == NULL))
    return POSIX_ERROR;

  if((attr->max_msg_num > 0) && (attr->max_msg_size > 0)) {
    _pattr = &_attr;
    memset(_pattr, 0, sizeof(_attr));
    _pattr->mq_maxmsg = attr->max_msg_num;
    _pattr->mq_msgsize = attr->max_msg_size;
  }

  oflag = O_CREAT;// | O_EXCL;
  if(attr->flag & CEI_MQ_NONBLOCK)
    oflag |= O_NONBLOCK;
  if(attr->flag & (CEI_MQ_READ | CEI_MQ_WRITE))
    oflag |= O_RDWR;
  else if(attr->flag & CEI_MQ_READ)
    oflag |= O_RDONLY;
  else if(attr->flag & CEI_MQ_WRITE)
    oflag |= O_WRONLY;

  // first check if the message queue exists, if not then create
  *mqd = mq_open(name, oflag , attr->mode, _pattr);
  // report error, but not if the message queue already exists
  if((*mqd == (CEI_MSGQ)-1) && (errno != EEXIST)) {
    DBG_CHK_ERRNO("mq_open", *mqd);
    return POSIX_ERROR;
  }

  return POSIX_SUCCESS;
}

/****************************************************************
*
*  PROCEDURE - CEI_MSG_QUEUE_OPEN
*
*  FUNCTION
*     Open a message queue
*
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_MSG_QUEUE_OPEN(CEI_CHAR *name, CEI_INT flag, CEI_MSGQ *mqd) {
  CEI_INT oflag=0;

  DBG_STR1(name);

  if((name == NULL) || (flag == 0))
    return POSIX_ERROR;

  if(flag & (CEI_MQ_READ | CEI_MQ_WRITE))
    oflag = O_RDWR;
  else if(flag & CEI_MQ_READ)
    oflag = O_RDONLY;
  else if(flag & CEI_MQ_WRITE)
    oflag = O_WRONLY;
  if(flag & CEI_MQ_NONBLOCK)
    oflag |= O_NONBLOCK;
  *mqd = mq_open(name, oflag);
  DBG_CHK_ERRNO("mq_open", *mqd);
  
  if(*mqd == (CEI_MSGQ)-1)
    return POSIX_ERROR;

  return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_MSG_QUEUE_CLOSE
*
*  FUNCTION
*
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_MSG_QUEUE_CLOSE(CEI_MSGQ mqd) {
  CEI_INT status=0;

  DBG_VAL1(mqd);

  if((status = mq_close(mqd)) == -1) {
    DBG_CHK_ERRNO("mq_close", status);
    return POSIX_ERROR;
  }

  return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_MSG_QUEUE_EXIT
*
*  FUNCTION
*
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_MSG_QUEUE_EXIT(CEI_CHAR *name) {
  CEI_INT status=0;

  DBG_STR1(name);

  status = mq_unlink(name);
  DBG_CHK_ERRNO("mq_unlink", status);
  
  if(status == -1)
    return POSIX_ERROR;

  return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_MSG_QUEUE_SEND
*
*  FUNCTION
*  size needs to be less than "CEI_MSG_QUEUE_ATTR" "max_msg_size". "prio" has a range from "0" to "MQ_PRIO_MAX"
* 
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_MSG_QUEUE_SEND(CEI_MSGQ mqd, CEI_CHAR *msg, CEI_INT size, CEI_UINT prio) {
  CEI_INT status=0;

  DBG_VAL1(mqd);

  status = mq_send(mqd, msg, (size_t) size, prio);
  DBG_CHK_ERRNO("mq_send", status);
  
  if(status == -1)
    return POSIX_ERROR;

  return POSIX_SUCCESS;
}



/****************************************************************
*
*  PROCEDURE - CEI_MSG_QUEUE_RECEIVE
*
*  FUNCTION
* 
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_MSG_QUEUE_RECEIVE(CEI_MSGQ mqd, CEI_CHAR *msg, CEI_INT size, CEI_UINT *prio) {
  CEI_INT status=0;

  DBG_VAL1(mqd);

  status = mq_receive(mqd, msg, (size_t) size, prio);
  DBG_CHK_ERRNO("mq_receive", status);
  
  if(status == -1)
    return POSIX_ERROR;

  return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_MSG_QUEUE_RECEIVE_TIMEOUT
*
*  FUNCTION
* 
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_MSG_QUEUE_RECEIVE_TIMEOUT(CEI_MSGQ mqd, CEI_CHAR *msg, CEI_INT size, CEI_UINT *prio, CEI_ULONG timeout_us) {
  CEI_INT status=0;
  CEI_ULONG time_ns=0;
  struct timespec tm;

  DBG_VAL1(mqd);

  if(clock_gettime(CLOCK_REALTIME, &tm) != 0) {
    printf("Error in clock_gettime - errno: %d\n",  errno);
    return POSIX_ERROR;
  }
  tm.tv_sec += timeout_us/1000000;
  if(timeout_us >= 1000000)
     timeout_us = timeout_us%1000000;
  time_ns = timeout_us * 1000;
  if((tm.tv_nsec + time_ns)/1000000000) {
    tm.tv_sec++;
    tm.tv_nsec = (tm.tv_nsec + time_ns)%1000000000;
  }
  else
    tm.tv_nsec += time_ns;

  status = mq_timedreceive(mqd, msg, (size_t) size, prio, &tm);
  DBG_CHK_ERRNO("mq_timedreceive", status);
  
  if(status == -1) {
    if(errno == ETIMEDOUT)
      return POSIX_TIMEOUT;
    return POSIX_ERROR;
  }

  return POSIX_SUCCESS;
}
#endif


/****************************************************************
*
*  PROCEDURE - CEI_SHM_CREATE
*
*  FUNCTION
*     creates shared memory
* 
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_SHM_CREATE(CEI_CHAR *name, CEI_INT length, CEI_INT *fd, CEI_CHAR **addr) {
  CEI_INT ret=POSIX_SUCCESS, status=0;
  CEI_CHAR *_addr=NULL;
  CEI_CHAR *_name=NULL;

  DBG_STR1(name);

  // add the "/" to the shared memory name
  _name = calloc(strlen(name)+2, 1);
  snprintf(_name, strlen(_name)-1, "/%s", name);
  *fd = shm_open(_name, O_RDWR | O_CREAT, CEI_SHM_OPEN_MODE);
  free(_name);
  if(*fd == -1) {
    DBG_CHK_ERRNO("shm_open", *fd);
    return POSIX_ERROR;
  }

  if(length != 0) {
    // set the size of the memory region
    if((status = ftruncate(*fd, length)) == -1) {
      DBG_CHK_ERRNO("ftruncate", status);
      ret = POSIX_ERROR; 
      status = close(*fd);
      DBG_CHK_STATUS("close", status);
    }
    else {
      if((_addr = (CEI_CHAR*)mmap(NULL, (size_t)length, PROT_READ|PROT_WRITE, MAP_SHARED, *fd, 0)) == MAP_FAILED) {
        DBG_CHK_ERRNO("mmap", (CEI_ULONG)_addr);
        return POSIX_ERROR;
      }
      if(_addr == NULL) {
       #ifdef LL_DEBUG
        printf(" <LL_DEBUG> error in %s - invalid address\n", __func__);
       #endif
        return POSIX_ERROR;
      }
      *addr = _addr;

      DBG_VAL1(fd);
    }
  }

  return ret;
}


/****************************************************************
*
*  PROCEDURE - CEI_SHM_OPEN
*
*  FUNCTION
*     open the shared memory for read/write
* 
*  RETURNS
*     POSIX_ERROR
*     POSIX_NOENT
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_SHM_OPEN(CEI_CHAR *name, CEI_INT length, CEI_INT *fd, CEI_CHAR **addr) {
  CEI_CHAR *_addr=NULL;
  CEI_CHAR *_name=NULL;

  DBG_STR1(name);

  // add the "/" to the shared memory name
  _name = calloc(strlen(name)+2, 1);
  snprintf(_name, strlen(_name)-1, "/%s", name);
  *fd = shm_open(_name, O_RDWR, CEI_SHM_OPEN_MODE);
  free(_name);
  if(*fd == -1) {
    DBG_CHK_ERRNO("shm_open", *fd);
    if(errno == ENOENT)
      return POSIX_NOENT;
    return POSIX_ERROR;
  }

  if(length != 0) {
    if((_addr = (CEI_CHAR*)mmap(NULL, (size_t)length, PROT_READ|PROT_WRITE, MAP_SHARED, *fd, 0)) == MAP_FAILED) {
      DBG_CHK_ERRNO("mmap", (CEI_ULONG)_addr);
      return POSIX_ERROR;
    }
    if(_addr == NULL) {
     #ifdef LL_DEBUG
      printf(" <LL_DEBUG> error in %s - invalid address\n", __func__);
     #endif
      return POSIX_ERROR;
    }
    *addr = _addr;

    DBG_VAL1(fd);
  }

  return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_SHM_CLOSE
*
*  FUNCTION
*     unmap the shared memory region and close the reference
* 
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_SHM_CLOSE(CEI_INT fd, CEI_INT length, CEI_CHAR *addr) {
  CEI_INT status=0;

  DBG_VAL1(fd);

  if((length > 0) && (addr != NULL)) {
    if((status = munmap(addr, length)) == -1) {
      DBG_CHK_ERRNO("munmap", status);
      return POSIX_ERROR;
    }
  }

  if((status = close(fd)) == -1) {
    DBG_CHK_ERRNO("close", status);
    return POSIX_ERROR;
  }

  return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_SHM_DESTROY
*
*  FUNCTION
*     unlink the shared memory
* 
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_SHM_DESTROY(CEI_CHAR *name) {
  CEI_INT status=0;
  CEI_CHAR *_name=NULL;

  DBG_STR1(name);

  // add the "/" to the shared memory name
  _name = calloc(strlen(name)+2, 1);
  snprintf(_name, strlen(_name)-1, "/%s", name);
  status = shm_unlink(_name);
  free(_name);
  if(status == -1) {
    DBG_CHK_ERRNO("shm_unlink", status);
    return POSIX_ERROR;
  }

  return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_SEM_CREATE
*
*  FUNCTION
*     create a semaphore and set to "locked" 
* 
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_SEM_CREATE(CEI_CHAR *name, CEI_SEM **sem) {
  CEI_CHAR *_name=NULL;

  DBG_STR1(name);

  // add the "/" to the shared memory name
  _name = calloc(strlen(name)+2, 1);
  snprintf(_name, strlen(_name)-1, "/%s", name);
  *sem = sem_open(_name, O_CREAT, CEI_SEM_OPEN_MODE, 0);
  free(_name);
  if(*sem == SEM_FAILED) {
    DBG_CHK_ERRNO("sem_open", (CEI_ULONG)*sem);
    return POSIX_ERROR;
  }

  DBG_VAL1(*sem);

  return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_SEM_OPEN
*
*  FUNCTION
*     open the semaphore
* 
*  RETURNS
*     POSIX_ERROR
*     POSIX_NOENT
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_SEM_OPEN(CEI_CHAR *name, CEI_SEM **sem) {
  CEI_CHAR *_name=NULL;

  DBG_STR1(name);

  // add the "/" to the shared memory name
  _name = calloc(strlen(name)+2, 1);
  snprintf(_name, strlen(_name)-1, "/%s", name);
  *sem = sem_open(_name, 0);
  free(_name);
  if(*sem == SEM_FAILED) {
    DBG_CHK_ERRNO("sem_open", (CEI_ULONG)*sem);
    if(errno == ENOENT)
      return POSIX_NOENT;
    return POSIX_ERROR;
  }

  DBG_VAL1(*sem);

  return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_SEM_CLOSE
*
*  FUNCTION
*     close a semaphore
* 
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_SEM_CLOSE(CEI_SEM *sem) {
  CEI_INT status=0;

  DBG_VAL1(sem);

  if((status = sem_close(sem)) == -1) {
    DBG_CHK_ERRNO("sem_close", status);
    return POSIX_ERROR;
  }

  return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_SEM_WAIT
*
*  FUNCTION
*     Decrement the semaphore if greater than "0" otherwise block
*     until value is greater than "0".
* 
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_SEM_LOCK(CEI_SEM *sem) {
  CEI_INT status=0;

  DBG_VAL1(sem);

  if((status = sem_wait(sem)) == -1) {
    DBG_CHK_ERRNO("sem_wait", status);
    return POSIX_ERROR;
  }

  return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_SEM_POST
*
*  FUNCTION
*     Increment the semaphore, thereby unlocking it and any
*     process/thread blocking with sem_wait will unblock and then
*     lock the semaphore.
* 
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_SEM_UNLOCK(CEI_SEM *sem) {
  CEI_INT status=0;

  DBG_VAL1(sem);

  if((status = sem_post(sem)) == -1) {
    DBG_CHK_ERRNO("sem_post", status);
    return POSIX_ERROR;
  }

  return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_SEM_GETVALUE
*
*  FUNCTION
*     Get the value of a semaphore. If there are processes/threads
*     blocking then either the value is set to "0" or a negative
*     number representing the count of processes/threads blocking
*     with sem_wait.
* 
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_SEM_GETVALUE(CEI_SEM* sem, CEI_INT *val) {
  CEI_INT status=0;

  DBG_VAL1(sem);

  if((status = sem_getvalue(sem, val)) == -1) {
    DBG_CHK_ERRNO("sem_getvalue", status);
    return POSIX_ERROR;
  }

  return POSIX_SUCCESS;
}


/****************************************************************
*
*  PROCEDURE - CEI_SEM_DESTROY
*
*  FUNCTION
*     unlink the semaphore
* 
*  RETURNS
*     POSIX_ERROR
*     POSIX_SUCCESS
****************************************************************/
CEI_INT CEI_SEM_DESTROY(CEI_CHAR *name) {
  CEI_INT status=0;
  CEI_CHAR *_name=NULL;

  DBG_STR1(name);

  // add the "/" to the shared memory name
  _name = calloc(strlen(name)+2, 1);
  snprintf(_name, strlen(_name)-1, "/%s", name);
  status = sem_unlink(_name);
  free(_name);
  if(status == -1) {
    DBG_CHK_ERRNO("sem_unlink", status);
    return POSIX_ERROR;
  }

  return POSIX_SUCCESS;
}
