/*============================================================================*
 * FILE:                     L L _ P O S I X . H
 *============================================================================*
 *
 *      COPYRIGHT (C) 2005 - 2017 BY ABACO SYSTEMS, INC.
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
 * FUNCTION:    Header file for low-level POSIX pthread interface.
 *
 *              Warning: this file is shared by many Abaco Systems avionics
 *              products, so please exercise caution when making modifications.
 *
 *===========================================================================*/

/* $Revision:  1.09 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  02/09/2006    initial. bch
  05/25/2006    changed CEI_SIG_TIMER and CEI_SIG_HINT values, added
                 CEI_SIG_HINT_MAX, modified CEI_SIGNAL parameter list. bch
  09/14/2006    changed values for POSIX_ERROR and PTHREAD_COND_TIMEOUT, and 
                 added POSIX_SUCCESS. bch
  03/13/2007    changed CEI_MUTEX_LOCK, CEI_MUTEX_UNLOCK, and
                 CEI_MUTEX_TRYLOCK macros to functions. moved debugging macros
                 to posix.c. bch
  05/18/2007    changed iDelay in CEI_WAIT_FOR_EVENT from CEI_UINT to CEI_INT,
                 added define for CEI_UINT. bch
  02/27/2009    removed "lowlevel.h". bch
  03/26/2009    removed define CEI_UINT. bch
  07/19/2012    added CEI_THREAD_EXIT and CEI_HANDLE_DESTROY. bch
  08/28/2014    added CEI_MUTEX_CREATE_SHARED, CEI_SHM_CREATE, CEI_SHM_OPEN,
                 CEI_SHM_CLOSE, CEI_SHM_DESTROY, CEI_SEM_CREATE, CEI_SEM_OPEN,
                 CEI_SEM_CLOSE, CEI_SEM_LOCK, CEI_SEM_UNLOCK, CEI_SEM_GETVALUE,
                 CEI_SEM_DESTROY, CEI_MSG_QUEUE_CREATE, CEI_MSG_QUEUE_OPEN,
                 CEI_MSG_QUEUE_CLOSE, CEI_MSG_QUEUE_EXIT, CEI_MSG_QUEUE_SEND,
                 CEI_MSG_QUEUE_RECEIVE, CEI_MSG_QUEUE_RECEIVE_TIMEOUT,
                 CEI_EVENT_SHARED_CREATE. Renamed PTHREAD_COND_TIMEOUT to
                 POSIX_EBUSY. Added POSIX_TIMEOUT and POSIX_NOENT.  bch
  09/27/2017    added build option NO_MSGQ_SUPPORT. bch
*/

#ifndef _LL_POSIX_H_
#define _LL_POSIX_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef NO_MSGQ_SUPPORT
#include <mqueue.h>
#endif
#include "cei_types.h"


// used by sigaction 
#ifndef SA_RESTART
 #define SA_RESTART 0
#endif
// real-time signals
#define CEI_SIG_TIMER     SIGRTMIN      // signal for timer
#define CEI_SIG_HINT      SIGRTMIN + 1  // signal for hardware interrupt
#define CEI_SIG_HINT_MAX  SIGRTMAX
// threads
#define CEI_CREATE   0
#define CEI_DESTROY  1
// timer
#define INFINITE  0
// message queue
#define CEI_MQ_READ     1
#define CEI_MQ_WRITE    2
#define CEI_MQ_NONBLOCK 4
// semaphore
#define CEI_SEM_OPEN_MODE  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
// shared memory
#define CEI_SHM_OPEN_MODE  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
// errors
#define POSIX_ERROR          -1
#define POSIX_SUCCESS        0
#define POSIX_EBUSY          1
#define POSIX_TIMEOUT        2
#define POSIX_NOENT          3

#define CEI_MALLOC(a) malloc(a)
#define CEI_FREE(a) free(a)
#define CEI_MUTEX pthread_mutex_t
#define CEI_EVENT pthread_cond_t
#define CEI_THREAD pthread_t
#define CEI_HANDLE int
#define CEI_SEM sem_t

#ifndef NO_MSGQ_SUPPORT
#define CEI_MSGQ mqd_t
typedef struct cei_msg_queue_attr {
  CEI_INT flag;
  mode_t mode;
  CEI_INT max_msg_size;
  CEI_INT max_msg_num;
}CEI_MSG_QUEUE_ATTR;
#endif

// prototypes
CEI_INT CEI_MUTEX_CREATE(CEI_MUTEX* pMutex);
CEI_INT CEI_MUTEX_SHARED_CREATE(CEI_MUTEX* pMutex);
CEI_INT CEI_MUTEX_LOCK(CEI_MUTEX* pMutex);
CEI_INT CEI_MUTEX_UNLOCK(CEI_MUTEX* pMutex);
CEI_INT CEI_MUTEX_TRYLOCK(CEI_MUTEX* pMutex);
CEI_INT CEI_MUTEX_TRYLOCK_TIMEOUT(CEI_MUTEX* pMutex, CEI_INT timeout);
CEI_INT CEI_MUTEX_DESTROY(CEI_MUTEX* pMutex);
CEI_INT CEI_EVENT_CREATE(CEI_EVENT* pEvent);
CEI_INT CEI_EVENT_SHARED_CREATE(CEI_EVENT* pEvent);
CEI_INT CEI_EVENT_DESTROY(CEI_EVENT* pEvent);
CEI_INT CEI_THREAD_CREATE(CEI_THREAD* pThread, CEI_INT iPriority, CEI_VOID* pFunc, CEI_VOID* pFuncArg);
CEI_INT CEI_THREAD_DESTROY(CEI_THREAD* pThread);
CEI_INT CEI_WAIT_FOR_EVENT(CEI_EVENT* pEvent, CEI_INT iDelay, CEI_MUTEX* pMutex);
CEI_INT CEI_EVENT_SIGNAL(CEI_EVENT* pEvent, CEI_MUTEX* pMutex);
CEI_INT CEI_TIMER(CEI_UINT cflag, CEI_UINT interval, CEI_VOID* pFunc);
CEI_INT CEI_SIGNAL(CEI_UINT cflag, CEI_INT signal, CEI_VOID* pFunc);
CEI_INT CEI_HANDLE_DESTROY(CEI_VOID *pHandle);
CEI_INT CEI_THREAD_EXIT(CEI_VOID* pStatus, CEI_VOID* pVal);

#ifndef NO_MSGQ_SUPPORT
CEI_INT CEI_MSG_QUEUE_CREATE(CEI_CHAR *name, CEI_MSG_QUEUE_ATTR *attr, CEI_MSGQ *mqd);
CEI_INT CEI_MSG_QUEUE_OPEN(CEI_CHAR *name, CEI_INT flag, CEI_MSGQ *mqd);
CEI_INT CEI_MSG_QUEUE_CLOSE(CEI_MSGQ mqd);
CEI_INT CEI_MSG_QUEUE_EXIT(CEI_CHAR *name);
CEI_INT CEI_MSG_QUEUE_SEND(CEI_MSGQ mqd, CEI_CHAR *msg, CEI_INT size, CEI_UINT prio);
CEI_INT CEI_MSG_QUEUE_RECEIVE(CEI_MSGQ mqd, CEI_CHAR *msg, CEI_INT size, CEI_UINT *prio);
CEI_INT CEI_MSG_QUEUE_RECEIVE_TIMEOUT(CEI_MSGQ mqd, CEI_CHAR *msg, CEI_INT size, CEI_UINT *prio, CEI_ULONG timeout_us);
#endif

CEI_INT CEI_SHM_CREATE(CEI_CHAR *name, CEI_INT length, CEI_INT *fd, CEI_CHAR **addr);
CEI_INT CEI_SHM_OPEN(CEI_CHAR *name, CEI_INT length, CEI_INT *fd, CEI_CHAR **addr);
CEI_INT CEI_SHM_CLOSE(CEI_INT fd, CEI_INT length, CEI_CHAR *addr);
CEI_INT CEI_SHM_DESTROY(CEI_CHAR *name);

CEI_INT CEI_SEM_CREATE(CEI_CHAR *name, CEI_SEM **sem);
CEI_INT CEI_SEM_OPEN(CEI_CHAR *name, CEI_SEM **sem);
CEI_INT CEI_SEM_CLOSE(CEI_SEM *sem);
CEI_INT CEI_SEM_LOCK(CEI_SEM *sem);
CEI_INT CEI_SEM_UNLOCK(CEI_SEM *sem);
CEI_INT CEI_SEM_GETVALUE(CEI_SEM* sem, CEI_INT *val);
CEI_INT CEI_SEM_DESTROY(CEI_CHAR *name);


// debugging
#ifdef LL_DEBUG
 #define DBG_CHK_STATUS(_1_, _2_)  if(_2_ != 0) {printf(" <LL_DBG> error in %s - status: %d (%s)\n", _1_, _2_, strerror(_2_));}
 #define DBG_CHK_ERRNO(_1_, _2_)  if(_2_ == -1) {printf(" <LL_DBG> error in %s - errno: %d (%s)\n", _1_, errno, strerror(errno));}
 #define DBG_VAL1(_1_)  printf(" <LL_DBG> %s: 0x%lx\n", __func__, (CEI_ULONG)_1_)
 #define DBG_VAL2(_1_, _2_)  printf(" <LL_DBG> %s: 0x%lx, 0x%lx\n", __func__, (CEI_ULONG)_1_, (CEI_ULONG)_2_)
 #define DBG_STR1(_1_)  printf(" <LL_DBG> %s: %s\n", __func__, _1_)
#else
 #define DBG_CHK_STATUS(_1_, _2_)
 #define DBG_CHK_ERRNO(_1_, _2_)
 #define DBG_VAL1(_1_)
 #define DBG_VAL2(_1_, _2_) 
 #define DBG_STR1(_1_) 
#endif // LL_DEBUG


#endif  // _LL_POSIX_H_
