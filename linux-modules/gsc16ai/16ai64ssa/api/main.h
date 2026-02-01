// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/api/main.h $
// $Rev: 51327 $
// $Date: 2022-07-11 17:21:59 -0500 (Mon, 11 Jul 2022) $

// 16AI64SSA: API Library: header file

#ifndef	__MAIN_H__
#define	__MAIN_H__

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

#include "16ai64ssa_api.h"
#include "gsc_main.h"



// macros *********************************************************************

#define	GSC_DEVS_PER_BOARD					1	// from ../driver/main.h

#define	STREAM_ID_RX						(0 << GSC_IO_SIZE_FLASG_SHIFT)
#define	STREAM_ID_TX						(0 << GSC_IO_SIZE_FLASG_SHIFT)



#endif
