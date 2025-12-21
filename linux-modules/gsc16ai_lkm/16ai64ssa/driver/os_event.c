// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_event.c $
// $Rev: 42879 $
// $Date: 2018-05-29 11:45:41 -0500 (Tue, 29 May 2018) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include "main.h"



//*****************************************************************************
void os_event_create(os_event_t* evnt)
{
	evnt->condition	= 0;
	memset(&evnt->entry, 0, sizeof(evnt->entry));
	WAIT_QUEUE_HEAD_INIT(&evnt->queue);
	WAIT_QUEUE_ENTRY_INIT(&evnt->entry, current);
	SET_CURRENT_STATE(TASK_INTERRUPTIBLE);
	add_wait_queue(&evnt->queue, &evnt->entry);
}



//*****************************************************************************
void os_event_destroy(os_event_t* evnt)
{
	remove_wait_queue(&evnt->queue, &evnt->entry);
	SET_CURRENT_STATE(TASK_RUNNING);
}



//*****************************************************************************
void os_event_resume(os_event_t* evnt)
{
	EVENT_RESUME_IRQ(&evnt->queue, evnt->condition);
}



//*****************************************************************************
void os_event_wait(os_event_t* evnt, os_time_tick_t timeout)
{
	if (timeout)
		EVENT_WAIT_IRQ_TO(&evnt->queue, evnt->condition, timeout);
	else
		EVENT_WAIT_IRQ(&evnt->queue, evnt->condition);
}


