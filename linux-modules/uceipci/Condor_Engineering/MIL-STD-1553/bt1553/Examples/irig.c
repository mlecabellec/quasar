/*============================================================================*
 * FILE:             E X A M P L E _ I R I G 1 . C
 *============================================================================*
 *
 * COPYRIGHT (C) 2004 - 2007 BY
 *          ABACO SYSTEMS, INC., GOLETA, CALIFORNIA
 *          ALL RIGHTS RESERVED.
 *
 *          THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND
 *          COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND WITH
 *          THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR ANY
 *          OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
 *          AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
 *          SOFTWARE IS HEREBY TRANSFERRED.
 *
 *          THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
 *          NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY ABACO
 *          SYSTEMS.
 *
 *===========================================================================*
 *
 * FUNCTION:    EXAMPLE PROGRAM
 *              This is a basic test program that demonstrates setup of the
 *              IRIG-B output and input.
 *
 *              This program just configures IRIG for INTERNAL source, turns
 *              on the IRIG output signal, sets the IRIG time to 123:12:34:56
 *              and then loops ten times reading and displaying the time every
 *              2 seconds.
 *
 *              NOTE - This REQUIRES a board with the IRIG option.  The part
 *                     number should include a 'W', which indicates IRIG.  For
 *                     example:  QPCI-1553-4MW
 *
 *
 *===========================================================================*/

/* $Revision:  1.02 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
    01/26/04   Initial version.  RSW
    01/27/04   Minor cleanup.  RSW
    09/20/04   Added option to initialize with BusTools_API_LoadChannel.  RSW
*/

#include "busapi.h"
#include <stdio.h>
#include <time.h>

//----------------------- Initializion Method Selection ---------------------------
// There are two methods of initializing the board:
//      1.  BusTools_API_InitExtended() - This is the "old-style" method.  It is
//          a bit more complicated but it applies to all platforms and products.
//
//      2.  BusTools_API_OpenChannel() - This is the "New, improved" method.  This
//          is less complicated (fewer parameters to get right), but does not work
//          with all possible platforms and products.  See the documentation in
//          the Software Reference Manual for this function to determine if this
//          method can be used with your platform and product.  This method is 
//          available with API version 5.10 and later.
//
// If you want to use method 1, define _USE_INIT_EXTENDED_.
// If you want to use method 2, comment out the definition for _USE_INIT_EXTENDED_.

//#define _USE_INIT_EXTENDED_


#ifdef _USE_INIT_EXTENDED_

// Constants for device information needed by BusTools_API_InitExtended.
// MODIFY THESE CONSTANTS TO MATCH YOUR CONFIGURATION, refer to documentation
// on the BusTools_API_InitExtended function for help.
#define MY_CARD_NUM		0
#define MY_BASE_ADDR	0
#define MY_IO_ADDR		0
#define MY_PLATFORM		PLATFORM_PC
#define MY_CARD_TYPE	PCCD1553
#define MY_CARRIER		NATIVE
#define MY_CHANNEL		CHANNEL_1
#define MY_MAPPING		CARRIER_MAP_DEFAULT

#else

// Constants to be used as parameters to BusTools_FindDevice and 
// BusTools_API_OpenChannel.
// MODIFY TO MATCH YOUR CONFIGURATION.
#define MY_CARD_TYPE	RXMC1553
#define MY_INSTANCE     1
#define MY_CHANNEL		CHANNEL_1

#endif
//---------------------------------------------------------------------------------



// Main program
void main() {
	int					status, i, ch_id;

	BT_UINT				wRev1, wRev2;

	char				c;
	BT1553_TIME			timeval;
	CEI_INT64				time_45bit_us, time_45bit_sec;
	unsigned int		usecs_left;

	// The following union gives us 32-bit IRIG format time-of-year.
	// We will use this to set the IRIG time.
	union {
		struct {
			unsigned int	sec1s  : 4;	// Units of seconds.
			unsigned int	sec10s : 3; // Tens of seconds.
			unsigned int	min1s  : 4; // Units of minutes.
			unsigned int	min10s : 3; // Tens of minutes.
			unsigned int	hour1s : 4; // Units of hours.
			unsigned int	hour10s: 2; // Tens of hours.
			unsigned int	day1s  : 4; // Units of days.
			unsigned int	day10s : 4; // Tens of days.
			unsigned int	day100s: 2; // Hundreds of days.
			unsigned int    unused : 2; // Unused bits - clear to zero.
		}			fields;

		BT_U32BIT	all;
	} my_irig_time;

//-------------------------- Initialize API and board -----------------------------
#ifdef _USE_INIT_EXTENDED_

	unsigned int	flag = 1;	// API init flag (1=SW ints, 2=HW ints).

	printf("Initializing API with BusTools_API_InitExtended . . . ");
	status = BusTools_API_InitExtended(MY_CARD_NUM, MY_BASE_ADDR, MY_IO_ADDR, &flag, 
		MY_PLATFORM, MY_CARD_TYPE, MY_CARRIER, MY_CHANNEL, MY_MAPPING);

    ch_id = MY_CARD_NUM;


#else

    int dev_num, mode;

	printf("Initializing API with BusTools_API_OpenChannel . . . ");

    // First find the device based on type and instance.
    dev_num = BusTools_FindDevice(MY_CARD_TYPE, MY_INSTANCE);
    if (dev_num < 0) printf("ERROR IN BUSTOOLS_FINDDEVICE.\n");

    // Open the device and get the channel id.
    mode = API_B_MODE | API_SW_INTERRUPT;  // 1553B protocol, use SW interrupts.
    status = BusTools_API_OpenChannel( &ch_id, mode, dev_num, MY_CHANNEL);

#endif
//---------------------------------------------------------------------------------

	if (status == API_SUCCESS) {
		printf("Success.\n\n");

		status = BusTools_GetRevision(ch_id, &wRev1, &wRev2);
		if (status != API_SUCCESS) printf("Error %d on BusTools_GetRevision.\n",status);
		if (wRev1 > 800)
			printf("Microcode Revision = %x\n",wRev1);
		else
			printf("Microcode Revision = %d\n",wRev1);
		printf("API Revision       = %d\n",wRev2);

		// The microcode revision 1000-digit (decimal) is the single-function flag.
		wRev1 = wRev1 / 1000;
		if (wRev1 & 0x0001)
			printf("This is a Single-Function device.\n\n");
		else
			printf("This is a Multiple-Function device.\n\n");

		// Initialize and reset memory.  Minimum BM setup.
		status = BusTools_BM_Init(ch_id, 1, 1);
		if (status != API_SUCCESS) printf("Error %d on BusTools_BM_Init\n",status);

		// Configure the IRIG generator.  We will use the internal source and enable the output signal.
		// NOTE:  You could set IRIG_EXTERNAL, and externally connect the IRIG OUTPUT to the IRIG INPUT.
		//        You can also use an oscilloscope to look at the IRIG OUTPUT signal.
		status = BusTools_IRIG_Config(ch_id, IRIG_INTERNAL, IRIG_OUT_DISABLE);
		if (status != API_SUCCESS) printf("Error %d on BusTools_IRIG_Config\n", status);

		// Set time tag mode for IRIG.
		status = BusTools_TimeTagMode(ch_id, API_TTD_IRIG, API_TTI_IRIG, API_TTM_IRIG, 0, 0, 0, 0);
		if (status != API_SUCCESS) printf("Error %d on BusTools_TimeTagMode\n", status);

		// Set an initial IRIG time value.
		// We will set:  day 123, time 12:34:56
		my_irig_time.fields.unused		= 0;
		my_irig_time.fields.day100s		= 1;
		my_irig_time.fields.day10s		= 2;
		my_irig_time.fields.day1s		= 3;
		my_irig_time.fields.hour10s		= 1;
		my_irig_time.fields.hour1s		= 2;
		my_irig_time.fields.min10s		= 3;
		my_irig_time.fields.min1s		= 4;
		my_irig_time.fields.sec10s		= 5;
		my_irig_time.fields.sec1s		= 6;
		status = BusTools_IRIG_SetTime(ch_id, 0, my_irig_time.all);
		if (status != API_SUCCESS) printf("Error %d on BusTools_IRIG_SetTime\n", status);

		printf("IRIG generator running, IRIG input set to internal, \ntime set to 123:12:34:56.000000.\n\n");

		// Read time for several iterations.
		printf("Reading time, 10 iterations, about 2 seconds between reads: \n");
		for (i=0; i<10; i++) {
			// Delay for about two seconds, so we can see time change.
			usleep(2000000);

			// Check for valid IRIG input.
			// Note that since we are using INTERNAL IRIG, this will always return success.
			// If you were using EXTERNAL IRIG, and if the signal was lost, it would return
			// API_IRIG_NO_SIGNAL.
			status = BusTools_IRIG_Valid(ch_id);
			if (status == API_SUCCESS) printf("IRIG signal valid.   ");
			else printf("IRIG SIGNAL LOST!!!  ");

			// Read time from the board.
			status = BusTools_TimeTagRead(ch_id, &timeval);
			if (status != API_SUCCESS) printf("Error %d on BusTools_TimeTagRead\n", status);

			// Convert time structure to microseconds.
			time_45bit_us = timeval.topuseconds;
			time_45bit_us = time_45bit_us << 32;
			time_45bit_us |= timeval.microseconds;

			// Convert microseconds to IRIG format (seconds), retain remaining microseconds.
			time_45bit_sec = time_45bit_us / (CEI_INT64) 1000000;
			usecs_left = (unsigned int) (time_45bit_us - (time_45bit_sec * 1000000));

			my_irig_time.fields.unused = 0;

			my_irig_time.fields.day100s = (unsigned int) (time_45bit_sec / (CEI_INT64)   8640000);
			time_45bit_sec = time_45bit_sec - (CEI_INT64) (my_irig_time.fields.day100s * 8640000);

			my_irig_time.fields.day10s = (unsigned int) (time_45bit_sec / (CEI_INT64)   864000);
			time_45bit_sec = time_45bit_sec - (CEI_INT64) (my_irig_time.fields.day10s * 864000);

			my_irig_time.fields.day1s = (unsigned int) (time_45bit_sec / (CEI_INT64)   86400);
			time_45bit_sec = time_45bit_sec - (CEI_INT64) (my_irig_time.fields.day1s * 86400);

			my_irig_time.fields.hour10s = (unsigned int) (time_45bit_sec / (CEI_INT64)   36000);
			time_45bit_sec = time_45bit_sec - (CEI_INT64) (my_irig_time.fields.hour10s * 36000);

			my_irig_time.fields.hour1s = (unsigned int) (time_45bit_sec / (CEI_INT64)   3600);
			time_45bit_sec = time_45bit_sec - (CEI_INT64) (my_irig_time.fields.hour1s * 3600);

			my_irig_time.fields.min10s = (unsigned int) (time_45bit_sec / (CEI_INT64)   600);
			time_45bit_sec = time_45bit_sec - (CEI_INT64) (my_irig_time.fields.min10s * 600);

			my_irig_time.fields.min1s = (unsigned int) (time_45bit_sec / (CEI_INT64)   60);
			time_45bit_sec = time_45bit_sec - (CEI_INT64) (my_irig_time.fields.min1s * 60);

			my_irig_time.fields.sec10s = (unsigned int) (time_45bit_sec / (CEI_INT64)   10);
			time_45bit_sec = time_45bit_sec - (CEI_INT64) (my_irig_time.fields.sec10s * 10);

			my_irig_time.fields.sec1s = (unsigned int) time_45bit_sec;

			printf("Time = %d%d%d:", my_irig_time.fields.day100s, my_irig_time.fields.day10s, my_irig_time.fields.day1s);
			printf("%d%d:",my_irig_time.fields.hour10s, my_irig_time.fields.hour1s);
			printf("%d%d:",my_irig_time.fields.min10s, my_irig_time.fields.min1s);
			printf("%d%d.",my_irig_time.fields.sec10s, my_irig_time.fields.sec1s);
			printf("%06d\n", usecs_left);
		}

		printf("Input to stop and exit.\n");
		scanf("%c",&c);

		// Shut down the IRIG generator.
		status = BusTools_IRIG_Config(ch_id, IRIG_INTERNAL, IRIG_OUT_DISABLE);
		if (status != API_SUCCESS) printf("Error %d on BusTools_IRIG_Config\n", status);
		printf("IRIG output stopped.\n");

		// We're done.  Close API and board
		printf("\nClosing API . . . ");
		status = BusTools_API_Close(ch_id);
		if (status == API_SUCCESS)
			printf("Success.\n");
		else 
			printf("FAILURE, error = %d\n", status);
	} // End of if (initialization successful)
	else printf("FAILURE, error = %d\n", status);

	printf("FINISHED.\n");
} // End of main
