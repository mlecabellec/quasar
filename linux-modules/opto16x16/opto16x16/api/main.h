// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/api/main.h $
// $Rev: 51396 $
// $Date: 2022-07-14 13:08:36 -0500 (Thu, 14 Jul 2022) $

// OPTO16X16: API Library: header file

#ifndef	__MAIN_H__
#define	__MAIN_H__

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

#include "opto16x16_api.h"
#include "gsc_main.h"



// macros *********************************************************************

#define	GSC_DEVS_PER_BOARD					1	// from ../driver/main.h

#define	STREAM_ID_RX						(0 << GSC_IO_SIZE_FLASG_SHIFT)



#endif
