/*============================================================================*
 * FILE:                        T I M E. C            
 *============================================================================*
 *
 *      COPYRIGHT (C) 1998-2017 BY ABACO SYSTEMS, INC. 
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
 * FUNCTION:   BusTools/1553-API Library:
 *             This file contains routines to handle time and timetag
 *             operations for the API.             
 *
 *     Currently, the structure is very simple -- there is a 64-bit value which
 *     consists of a 32-bit LSW and a 32-bit MSW. There are two time tag 
 *     resolutions.  For current firmware V6 the  time tag resolution is 
 *     1 nanosecond.  The entire 64 time structure is used giving
 *     a total time limit of over 500 years. The time tag on legacy firmware
 *     uses only 45 bits of the 64 bits available and the time tag resolution is
 *     1 millisecond giving a total time limit of only 407 days.
 *     every message in the hardware is stamped with a timetag -- the
 *     software can convert the elapsed time entry to a time string.
 *     
 * USER ENTRY POINTS: 
 *      BusTools_GetTimeTagMode  - Return time tag settings.
 *      BusTools_TimeGetString   - Converts a time value to a printable string
 *      BusTools_TimeGetFmtString- Converts a time value to a selected printable string
 *      BusTools_TimeGetStringLV - LabView interface
 *      BusTools_TimeTagMode     - Setup time tag parameters & display options
 *      BusTools_TimeTagInit     - Resets elapsed time counters and the
 *                                 timetag counter in the hardware
 *      BusTools_TimeTagRead     - Reads the time tag from the hardware
 *      BusTools_TimeTagReset    - Resets Time Tag to zero (0)
 *      BusTools_TimeTagWrite    - Writes the hardware time tag
 *      BusTools_SetTimeIncrement- Setups up to use auto-icrement the Time Tag register
 *      BusTools_IRIG_Calibration- Calibrate the IRIG DAC
 *      BusTools_IRIG_Config     - Sets up the IRIG control register initial DAC register value
 *      BusTools_IRIG_SetTime    - Sets the internal IRIG time
 *      BusTools_IRIG_Valid      - Returns whether there is a valid IRIG signal detected.
 *
 * EXTERNAL NON-USER ENTRY POINTS:
 *      bt_memcpy            - memcpy() replacement which only moves WORDS.
 *      DumpTimeTag          - Debug dump time tag memory helper function.
 *                             conversion (sets time_previous=time_current)
 *      TimeTagConvert       - Converts timetag from h/w value to usecs
 *      TimeTagInterrupt     - Counts the timetag overflow interrupts.
 *      TimeTagZeroModule    - Initializes Time Tag module on API cardnum startup.
 *===========================================================================*/

/* $Revision:  8.22 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  02/06/1999   Converted time keeping functions to support future IRIG and/or
               GPS timing.V3.03.ajh
  04/20/1999   Added BusTools_TimeGetStringLV() function.V3.03.ajh
  09/01/1999   Added BusTools_SetTimeTag() function prototype.V3.20.ajh
  10/01/1999   Changed bt_memcpy() to move aligned 16-bit words only for the
               IP-1553 on the VXI backplane.  Needs more work.  V3.20.ajh
  11/29/1999   Changed to support 45-bit time tag counter in hardware.V3.30.ajh
  01/18/2000   Added support for the ISA-1553, PMC-1553, and IP PROM V5.V4.00.ajh
  01/25/2000   Fixed IP-1553 V1-V4 time tag problem (tt was divided by 20) that
               was introduced in V4.00.  Changed TimeGetString to not return a
               full IRIG to M/D string if day is zero.V4.01.ajh
  02/23/2000   Modified the function BusTools_TimeGetString to get the current
               year from the host.  Changed the time tag interrupt and clear
               functions to fix the time tag problem when BusTools_BM_MessageRead
               is called around the time that the tag overflows.V4.01.ajh
  08/19/2000   Changed BusTools_TimeGetString() function to correctly display
               "long" values when compiled in 16-bit mode.V4.11.ajh
  09/15/2000   Modified to merge with UNIX/Linux version.V4.16.ajh
  11/30/2000   Fixed Init to zero time tag load register and SW shadow.V4.25.ajh
  01/03/2002   Improve platform and O/S portability.v4.46
  03/15/2002   Add IRIG support.v4.48
  06/05/2002   Add IRIG support for IP-D1553 and VME-1553 v4.52
  02/25/2003   Add IRIG support for QPCI-1553,  QPMC-1553, and PCC-1553
  02/19/2004   Change the time tag read function for newer F/W to read successive buffers
  06/25/2004   Change BusTools_IRIG_SetTime to account day offset and allow for either gmtime
               or localtime functions
  01/02/2006   Update for portibility
  11/19/2007   Added code to disable the IRIG during calibration. In BusTools_IRIG_Calibration.
  11/19/2007   Added code in BusTools_IRIG_SetTime to take time format of dddhhmmss.
  06/25/2009   Move get_48BitHostTimeTag to O/s specific files.
  04/06/2011   Add API_TTM_XCLK mode and BusTools_TimeTagReset.
  05/11/2012   Major change to combine V6 F/W and V5 F/W into single API  
  10/10/2012   Add new formated time conversion function
  11/05/2014   Define MAX_BTA 64 for Windows.
  10/03/2017   Modified board/firmware check for API_TTM_XCLK. bch
 */

/*---------------------------------------------------------------------------*
 *                     INCLUDES, DEFINES and GLOBALS
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "busapi.h" 
#include "apiint.h"
#include "btdrv.h"
#include "globals.h"

/*---------------------------------------------------------------------------*
 *                     Local Data Base Variables
 *---------------------------------------------------------------------------*/
static BT_UINT TTDisplay_fmt = {API_TTD_RELM};    // Time Tag display format

static BT1553_TIME time_current [MAX_BTA];    // Time in one microsecond ticks.
static BT1553_TIME TTLR_Shadow[MAX_BTA];      // Time Tag Load Register Shadow.
static BT_UINT     TTInit_type[MAX_BTA];      // Time Tag initialization type
static BT_UINT     TTMode_Def[MAX_BTA];       // Time Tag operational Mode
static BT_U32BIT   TTPeriod_Def[MAX_BTA];     // External TT sync pulse period (us)
                                              //  or overflow period.
#if defined(__WIN32__)
// Pointer to user function BusTools_TimeTagGet() in DLLname.dll:
static NOMANGLE BT_INT (CCONV *pBusTools_TimeTagGet[MAX_BTA])(
   BT_UINT cardnum,               // (i) card number (0 - based)
   BT1553_TIME * timetag);    // (i) pointer to time tag structure
#endif


/*---------------------------------------------------------------------------*
 *              Internal Prototypes
 *---------------------------------------------------------------------------*/


/****************************************************************************
*
*  PROCEDURE - DumpTimeTag
*
*  FUNCTION
*     Debug dump Time Tag memory helper function.
*
****************************************************************************/

void DumpTimeTag(
   BT_UINT cardnum,         // (i) card number
   FILE  * hfMemFile)       // (i) handle of output file
{
#if 0 //Not used
   char *TTD[3] = {"API_TTD_RELM",
                   "API_TTD_IRIG",
                   "API_TTD_DATE"};
#endif

   char *TTI[4] = {"API_TTI_ZERO",
                   "API_TTI_DAY",
                   "API_TTI_IRIG",
                   "API_TTI_EXT"};

   char *TTM[6] = {"API_TTM_FREE",
                   "API_TTM_RESET",
                   "API_TTM_SYNC",
                   "API_TTM_RELOD",
                   "API_TTM_IRIG",
                   "API_TTM_AUTO"};               


   fprintf(hfMemFile, "Time Tag Mode = %s, TT Init Type = %s, Period = %d\n\n",
           TTM[TTMode_Def[cardnum]], TTI[TTInit_type[cardnum]], TTPeriod_Def[cardnum]);

}

/****************************************************************************
*
*  PROCEDURE - TimeTagInterrupt
*
*  FUNCTION
*     Handle the timetag overflow interrupt queue entry.  
*     This function gets called when the discrete input toggles and causes
*     the TT counter to get loaded from the TT load register, or
*     when the discrete input clears the time tag counter.
****************************************************************************/

BT_U32BIT TimeTagInterrupt(
   BT_UINT   cardnum)      // (i) Card number.
{

   /****************************************************************
   * A timetag overflow means that the discrete input has toggled
   *  and caused the time tag counter to either be zero'ed or be
   *  reloaded from the time tag load register.  We need to
   *  compute the new TT load register value and write it to the
   *  HW, or update the software "time_current" value...
   ****************************************************************/
   switch ( TTMode_Def[cardnum] )
   {
   case API_TTM_SYNC:  // TT counter sync'ed to external pulse train
      TTLR_Shadow[cardnum].microseconds += TTPeriod_Def[cardnum];
      if ( TTLR_Shadow[cardnum].microseconds < TTPeriod_Def[cardnum] ) // Carry?
         TTLR_Shadow[cardnum].topuseconds++;                           // Yes...
      BusTools_TimeTagWrite(cardnum, &TTLR_Shadow[cardnum], 0);
      break;
   case API_TTM_RELOD: // Reload previous value into TT counter
         BusTools_TimeTagWrite(cardnum, &TTLR_Shadow[cardnum], 0);
      break;
   }

   return 0;
}

/****************************************************************************
*
*  PROCEDURE - TimeTagZeroModule
*
*  FUNCTION
*         Initializes Time Tag module on API cardnum startup.
*
****************************************************************************/
void TimeTagZeroModule(
   BT_UINT   cardnum)      // (i) Card number.
{
   TTLR_Shadow[cardnum].topuseconds  = 0;         // Time Tag Load Reg Shadow.
   TTLR_Shadow[cardnum].microseconds = 1000000L;  // Time Tag Load Reg Shadow.
   TTInit_type[cardnum]  = API_TTI_ZERO;      // Time Tag initialization type
   TTMode_Def[cardnum]   = API_TTM_FREE;      // Time Tag operational Mode
   TTPeriod_Def[cardnum] = 0L;                // External TT sync pulse period (us)
}


/*---------------------------------------------------------------------------*
 *                      EXTERNAL USER ENTRY POINTS
 *---------------------------------------------------------------------------*/

/****************************************************************************
*
*  PROCEDURE - BusTools_TimeGetString
*
*  FUNCTION
*     This routine is used to convert a BusTools time structure into
*     a string suitable for display to a user, containing ASCII days,
*     hours, minutes, seconds, microseconds.
*     The input time is a 64-bit binary value in microseconds.  
*     A 48-bit value covers a range of about 8.93 years.
*
*     First we divide the 64-bit value by 1,000,000 or 1000,000,000; the remainder 
*     is the number of microseconds/nanoseconds and the quotent (seconds) 
*     fits into 32-bits. Then it is reasonably easy to convert to days, hours,
*     minutes and seconds.
*
*  RETURNS
*     Nothing.
*
*  Note: This routine takes about 9.5 us on a 486DX4/100 (not using sprintf).
****************************************************************************/
NOMANGLE void CCONV BusTools_TimeGetString(
   BT1553_TIME * curtime,   // (i) Timer to be converted to string value.
   char        * string)    // (i/o) Pointer, store resulting time string. If set to "NANO"
                            //       then the lsb is treated as a nanosecond not microsecond.
{
   /***********************************************************************
   *  Local variables
   ***********************************************************************/
   BT_UINT nanotime=0; // nano second time
   long days;          // Number of days in time tag ( 0 to 3257 )
   long hours;         // Number of hours ( 0 to 23 )
   long minutes;       // Number of minutes ( 0 to 59 )
   long seconds;       // Number of seconds ( 0 to 59 )
   unsigned long subseconds; // nano or micro depending on selection ( 0 to 999999 ) v5 F/W  ( 0 to 999999999 ) v6 F/W
   
   /*********************************************************************
   * If your compiler does not support this code, you will have to build
   * your own using whatever tools your environment provides.
   * When I tried the ldiv() ANSI function it seemed to fail if the sign bit
   * of curtime->microseconds was set (at least on Microsoft/Borland),
   * as well as when the sign bit of curtime->topuseconds was set...ajh
   *********************************************************************/

   CEI_UINT64   big_time;   // 64-bit timer value for the arithmetic

   /* Extract Seconds from micro-/nano-seconds */
   big_time = curtime->topuseconds;
   big_time = (big_time << 32) | curtime->microseconds;

   if((strncmp(string,"NANO",4)==0) || (TTDisplay_fmt & API_TT_NANO))
   {
      subseconds  = (unsigned long)(big_time%1000000000);  /* 0 to 999999999  */
      seconds      = (unsigned long)(big_time/1000000000);
      nanotime = API_TT_NANO;
   }
   else
   {
      subseconds = (unsigned long)(big_time%1000000);  /* 0 to 999999  */
      seconds      = (unsigned long)(big_time/1000000);   
      nanotime = API_TT_MICRO;
   }

   /* Extract Minutes from Seconds */
   minutes = seconds/60;      /* 0 to 0x10C6F7A0/60 */
   seconds = seconds%60;      /* 0 to 59            */

   /* Extract Hours from Minutes */
   hours = minutes/60;        /* 0 to 0x1316B       */
   minutes = minutes%60;      /* 0 to 59            */

   /* Extract Days from Hours */
   days = (hours/24);       /* 1 to 3258          */
   hours = hours%24;        /* 0 to 23            */

   /* Dispatch on the display format defined */
   switch ( TTDisplay_fmt | nanotime) 
   {
   case API_TTD_RELM:   // Relative to Midnight Format "(ddd)hh:mm:ss.uuuuuu"
      if ( days )
         sprintf(string,"(%ld)%2.2ld:%2.2ld:%2.2ld.%6.6ld",
                 days, hours, minutes, seconds, subseconds);
      else if ( hours )
         sprintf(string,"%ld:%2.2ld:%2.2ld.%6.6ld",
                 hours, minutes, seconds, subseconds);
      else if ( minutes )
         sprintf(string,"%ld:%2.2ld.%6.6ld", minutes, seconds, subseconds);
      else
         sprintf(string,"%ld.%6.6ld", seconds, subseconds);
      return;
   case API_TTD_RELM_NS:   // Relative to Midnight Format "(ddd)hh:mm:ss.uuuuuuuuu"
      if ( days )
         sprintf(string,"(%ld)%2.2ld:%2.2ld:%2.2ld.%9.9ld",
                 days, hours, minutes, seconds, subseconds);
      else if ( hours )
         sprintf(string,"%ld:%2.2ld:%2.2ld.%9.9ld",
                 hours, minutes, seconds, subseconds);
      else if ( minutes )
         sprintf(string,"%ld:%2.2ld.%9.9ld", minutes, seconds, subseconds);
      else
         sprintf(string,"%ld.%9.9ld", seconds, subseconds);
      return;

   case API_TTD_IRIG:   // Full IRIG Format "(ddd)hh:mm:ss.uuuuuu"
                        // (implies a delta time is being displayed).
      days++;           // increment days to adjust to IRIG julian date 
      if ( days )
         sprintf(string,"(%ld)%2.2ld:%2.2ld:%2.2ld.%6.6ld",
                 days, hours, minutes, seconds, subseconds);
      else if ( hours )
         sprintf(string,"%ld:%2.2ld:%2.2ld.%6.6d",
                 hours, minutes, seconds, (BT_U32BIT)subseconds);
      else if (minutes )
         sprintf(string,"%ld:%2.2ld.%6.6ld", minutes, seconds,subseconds);
      else
         sprintf(string,"%ld.%6.6ld", seconds, subseconds);
      return;
   case API_TTD_IRIG_NS:// Full IRIG Format "(ddd)hh:mm:ss.uuuuuuuuu"
                        // (implies a delta time is being displayed).
      days++;           // increment days to adjust to IRIG julian date 
      if ( days )
         sprintf(string,"(%ld)%2.2ld:%2.2ld:%2.2ld.%9.9ld",
                 days, hours, minutes, seconds, subseconds);
      else if ( hours )
         sprintf(string,"%ld:%2.2ld:%2.2ld.%9.9d",
                 hours, minutes, seconds, (BT_U32BIT)subseconds);
      else if (minutes )
         sprintf(string,"%ld:%2.2ld.%9.9ld", minutes, seconds,subseconds);
      else
         sprintf(string,"%ld.%9.9ld", seconds, subseconds);
      return;

   case API_TTD_DATE:   // Date Format "(MM/dd)hh:mm:ss.uuuuuu"
   case API_TTD_DATE_NS:
      {                 // Convert the IRIG day of the year to month/day.
                        //  IRIG day 1 = Jan 1.  We need the year for
                        //  leap year calculations.
         int m = 0;                 // The first month is January
         static int year = 0;       // We read this from the host.
         int leap;                  // Is this year a leap year?
         int resulting_month = 1;   // Just in case the day of the year should
         int resulting_day = 0;     //  just happen to be zero (which is illegal).
         
         days++;                    // Adjust for the fact there is no day 0.
         // Get the current year for leap year computations:
         if ( year == 0 )
         {
            time_t timer;                     // Time since 1900
            struct tm *tblock;                // Pointer to structure containing year

            timer = time(NULL);               // Get time of day et.al.
            tblock = localtime(&timer);       // Convert date/time to a structure
            year = tblock->tm_year + 1900;    // Get and save the year
         }

         leap = ((year % 4) == 0) ? 1 : 0;    // Is this year a leap year?
                                              // Should work until 3000...
         
	 // convert days to handle the end of the calendar year, takes into account a leap year
         if(days > (365 + leap))
           days %= (365 + leap);

         // Convert day of the year to month:day
         while ( days > 0 )
         {
            resulting_day   = days;
            resulting_month = m;
            switch(m)
            {
               case 1: days -= 31; break;          // Jan
               case 2: days -= (28 + leap); break; // Feb
               case 3: days -= 31; break;          // March
               case 4: days -= 30; break;          // April
               case 5: days -= 31; break;          // May
               case 6: days -= 30; break;          // June
               case 7: days -= 31; break;          // July
               case 8: days -= 31; break;          // August
               case 9: days -= 30; break;          // Sept
               case 10:days -= 31; break;          // Oct
               case 11:days -= 30; break;          // Nov
               case 12:days -= 31; break;          // Dec
            }
            m++;  // Go to next month
         }
         // Display all components, unless the day is zero
         //   (which implies a delta time display) then display a short version.
         if(nanotime == API_TT_NANO)
         {
            if ( resulting_day )
               sprintf(string,"(%d/%d)%2.2ld:%2.2ld:%2.2ld.%9.9ld", resulting_month,
                       resulting_day, hours, minutes, seconds, subseconds);
            else if ( hours )
               sprintf(string,"%ld:%2.2ld:%2.2ld.%9.9ld",
                    hours, minutes, seconds, subseconds);
            else if ( minutes )
               sprintf(string,"%ld:%2.2ld.%9.9ld", minutes, seconds, subseconds);
            else
               sprintf(string,"%ld.%9.9ld", seconds, subseconds);
         }
         else
         {
            if ( resulting_day )
               sprintf(string,"(%d/%d)%2.2ld:%2.2ld:%2.2ld.%6.6ld", resulting_month,
                       resulting_day, hours, minutes, seconds, subseconds);
            else if ( hours )
               sprintf(string,"%ld:%2.2ld:%2.2ld.%6.6ld",
                       hours, minutes, seconds, subseconds);
            else if ( minutes )
               sprintf(string,"%ld:%2.2ld.%6.6ld", minutes, seconds, subseconds);
            else
               sprintf(string,"%ld.%6.6ld", seconds, subseconds);
         }
      }
   }
}

/****************************************************************************
*
*  PROCEDURE - BusTools_TimeGetFmtString
*
*  FUNCTION
*     This routine is used to convert a BusTools time structure into
*     a string suitable for display to a user, containing ASCII days,
*     hours, minutes, seconds, nanoseconds.
*     The input time is a 64-bit binary value in microseconds. A 64-bit value 
*     covers a range of about 584 years.
*
*     First we divide the 64-bit value by 1,000,000,000; the remainder is the
*     number of nanoseconds and the quotent (seconds) fits into 32-bits.
*     Then it is reasonably easy to convert to days, hours, minutes and seconds.
*
*  RETURNS
*     Nothing.
****************************************************************************/
NOMANGLE void CCONV BusTools_TimeGetFmtString(
   BT_INT        tFormat,   // (i) Format assignment (API_TT_DEFAULT, API_TTD_RELM, API_TTD_IRIG, API_TTD_DATE
                            //                        API_TTD_RELM_NS, API_TTD_IRIG_NS, API_TTD_DATE_NS)
   BT1553_TIME * curtime,   // (i) Timer to be converted to string value.
   char        * string)    // (i/o) Pointer, store resulting time string. .
{
   /***********************************************************************
   *  Local variables
   ***********************************************************************/
   long days;          // Number of days in time tag ( 0 to 3257 )
   long hours;         // Number of hours ( 0 to 23 )
   long minutes;       // Number of minutes ( 0 to 59 )
   long seconds;       // Number of seconds ( 0 to 59 )
   unsigned long subseconds; // nano or micro depending on selection ( 0 to 999999 ) v5 F/W  ( 0 to 999999999 ) v6 F/W
   
   /*********************************************************************
   * If your compiler does not support this code, you will have to build
   * your own using whatever tools your environment provides.
   * When I tried the ldiv() ANSI function it seemed to fail if the sign bit
   * of curtime->microseconds was set (at least on Microsoft/Borland),
   * as well as when the sign bit of curtime->topuseconds was set...ajh
   *********************************************************************/

   CEI_UINT64   big_time;   // 64-bit timer value for the arithmetic

   /* Extract Seconds from micro-/nano-seconds */
   big_time = curtime->topuseconds;
   big_time = (big_time << 32) | curtime->microseconds;


   if((strncmp(string,"NANO",4)==0) || (tFormat & API_TT_NANO))
   {
      subseconds  = (unsigned long)(big_time%1000000000);  /* 0 to 999999999  */
      seconds      = (unsigned long)(big_time/1000000000);
      tFormat |= API_TT_NANO;  // make sure nano is set
   }
   else
   {
      subseconds = (unsigned long)(big_time%1000000);  /* 0 to 999999  */
      seconds      = (unsigned long)(big_time/1000000);   
   }

   /* Extract Minutes from Seconds */
   minutes = seconds/60;      /* 0 to 0x10C6F7A0/60 */
   seconds = seconds%60;      /* 0 to 59            */

   /* Extract Hours from Minutes */
   hours = minutes/60;        /* 0 to 0x1316B       */
   minutes = minutes%60;      /* 0 to 59            */

   /* Extract Days from Hours */
   days = (hours/24);       /* 1 to 3258          */
   hours = hours%24;        /* 0 to 23            */

   /* Dispatch on the display format defined */
   if(tFormat == API_TT_DEFAULT)
      tFormat = TTDisplay_fmt;

   switch (tFormat)
   {
   case API_TTD_RELM:   // Relative to Midnight Format "(ddd)hh:mm:ss.uuuuuu"
      if ( days )
         sprintf(string,"(%ld)%2.2ld:%2.2ld:%2.2ld.%6.6ld",
                 days, hours, minutes, seconds, subseconds);
      else if ( hours )
         sprintf(string,"%ld:%2.2ld:%2.2ld.%6.6ld",
                 hours, minutes, seconds, subseconds);
      else if ( minutes )
         sprintf(string,"%ld:%2.2ld.%6.6ld", minutes, seconds, subseconds);
      else
         sprintf(string,"%ld.%6.6ld", seconds, subseconds);
      return;
   case API_TTD_RELM_NS:   // Relative to Midnight Format "(ddd)hh:mm:ss.uuuuuuuuu"
      if ( days )
         sprintf(string,"(%ld)%2.2ld:%2.2ld:%2.2ld.%9.9ld",
                 days, hours, minutes, seconds, subseconds);
      else if ( hours )
         sprintf(string,"%ld:%2.2ld:%2.2ld.%9.9ld",
                 hours, minutes, seconds, subseconds);
      else if ( minutes )
         sprintf(string,"%ld:%2.2ld.%9.9ld", minutes, seconds, subseconds);
      else
         sprintf(string,"%ld.%9.9ld", seconds, subseconds);
      return;

   case API_TTD_IRIG:   // Full IRIG Format "(ddd)hh:mm:ss.uuuuuu"
                        // (implies a delta time is being displayed).
      days++;           // increment days to adjust to IRIG julian date 
      if ( days )
         sprintf(string,"(%ld)%2.2ld:%2.2ld:%2.2ld.%6.6ld",
                 days, hours, minutes, seconds, subseconds);
      else if ( hours )
         sprintf(string,"%ld:%2.2ld:%2.2ld.%6.6d",
                 hours, minutes, seconds, (BT_U32BIT)subseconds);
      else if (minutes )
         sprintf(string,"%ld:%2.2ld.%6.6ld", minutes, seconds,subseconds);
      else
         sprintf(string,"%ld.%6.6ld", seconds, subseconds);
      return;
   case API_TTD_IRIG_NS:// Full IRIG Format "(ddd)hh:mm:ss.uuuuuuuuu"
                        // (implies a delta time is being displayed).
      days++;           // increment days to adjust to IRIG julian date 
      if ( days )
         sprintf(string,"(%ld)%2.2ld:%2.2ld:%2.2ld.%9.9ld",
                 days, hours, minutes, seconds, subseconds);
      else if ( hours )
         sprintf(string,"%ld:%2.2ld:%2.2ld.%9.9d",
                 hours, minutes, seconds, (BT_U32BIT)subseconds);
      else if (minutes )
         sprintf(string,"%ld:%2.2ld.%9.9ld", minutes, seconds,subseconds);
      else
         sprintf(string,"%ld.%9.9ld", seconds, subseconds);
      return;

   case API_TTD_DATE:   // Date Format "(MM/dd)hh:mm:ss.uuuuuu"
   case API_TTD_DATE_NS:
      {                 // Convert the IRIG day of the year to month/day.
                        //  IRIG day 1 = Jan 1.  We need the year for
                        //  leap year calculations.
         int m = 0;                 // The first month is January
         static int year = 0;       // We read this from the host.
         int leap;                  // Is this year a leap year?
         int resulting_month = 1;   // Just in case the day of the year should
         int resulting_day = 0;     //  just happen to be zero (which is illegal).
         
         days++;                    // Adjust for the fact there is no day 0.
         // Get the current year for leap year computations:
         if ( year == 0 )
         {
            time_t timer;                     // Time since 1900
            struct tm *tblock;                // Pointer to structure containing year

            timer = time(NULL);               // Get time of day et.al.
            tblock = localtime(&timer);       // Convert date/time to a structure
            year = tblock->tm_year + 1900;    // Get and save the year
         }

         leap = ((year % 4) == 0) ? 1 : 0;    // Is this year a leap year?
                                              // Should work until 3000...
         
	 // convert days to handle the end of the calendar year, takes into account a leap year
         if(days > (365 + leap))
           days %= (365 + leap);

         // Convert day of the year to month:day
         while ( days > 0 )
         {
            resulting_day   = days;
            resulting_month = m;
            switch(m)
            {
               case 1: days -= 31; break;          // Jan
               case 2: days -= (28 + leap); break; // Feb
               case 3: days -= 31; break;          // March
               case 4: days -= 30; break;          // April
               case 5: days -= 31; break;          // May
               case 6: days -= 30; break;          // June
               case 7: days -= 31; break;          // July
               case 8: days -= 31; break;          // August
               case 9: days -= 30; break;          // Sept
               case 10:days -= 31; break;          // Oct
               case 11:days -= 30; break;          // Nov
               case 12:days -= 31; break;          // Dec
            }
            m++;  // Go to next month
         }
         // Display all components, unless the day is zero
         //   (which implies a delta time display) then display a short version.
         if(tFormat & API_TT_NANO)
         {
            if ( resulting_day )
               sprintf(string,"(%d/%d)%2.2ld:%2.2ld:%2.2ld.%9.9ld", resulting_month,
                       resulting_day, hours, minutes, seconds, subseconds);
            else if ( hours )
               sprintf(string,"%ld:%2.2ld:%2.2ld.%9.9ld",
                    hours, minutes, seconds, subseconds);
            else if ( minutes )
               sprintf(string,"%ld:%2.2ld.%9.9ld", minutes, seconds, subseconds);
            else
               sprintf(string,"%ld.%9.9ld", seconds, subseconds);
         }
         else
         {
            if ( resulting_day )
               sprintf(string,"(%d/%d)%2.2ld:%2.2ld:%2.2ld.%6.6ld", resulting_month,
                       resulting_day, hours, minutes, seconds, subseconds);
            else if ( hours )
               sprintf(string,"%ld:%2.2ld:%2.2ld.%6.6ld",
                       hours, minutes, seconds, subseconds);
            else if ( minutes )
               sprintf(string,"%ld:%2.2ld.%6.6ld", minutes, seconds, subseconds);
            else
               sprintf(string,"%ld.%6.6ld", seconds, subseconds);
         }
      }
   }
}

/*****************************************************************************
*
*  PROCEDURE - BusTools_GetTimeTagMode
*
*  FUNCTION
*     Returns the time tag Display, Initialization and mode settings.
*
*  RETURNS
*     API_SUCCESS             
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_GetTimeTagMode(
   BT_UINT   cardnum,         // (i) card number (0 - based)
   BT_INT    *TTDisplay,      // (o  Pointer to time tag display format
   BT_INT    *TTInit,         // (o) Pointer to time tag counter initialization mode
   BT_INT    *TTMode,         // (o) Pointer to time tag timer operation mode
   BT_U32BIT *TTPeriod)       // (o) Pointer to period of external TTL sync input
{
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;
   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;
      
   *TTDisplay = TTDisplay_fmt;
   *TTInit    = TTInit_type[cardnum];
   *TTMode    = TTMode_Def[cardnum];
   *TTPeriod  = TTPeriod_Def[cardnum];

   return API_SUCCESS;
}

/*****************************************************************************
*
*  PROCEDURE - BusTools_TimeTagMode
*
*  FUNCTION
*     This routine is called to setup the time tag parameters
*     and conversion options.
*
*   TTDisplay    Display Type:
* -------------- --------------------------------------------------------------
* API_TT_DEFAULT Unchanged from previous call
* API_TTD_RELM   Relative to midnight format (dd-hh:mm:ss.useconds, default)
* API_TTD_IRIG   IRIG Format (ddd-hh:mm:ss.uuuuuu) (All board variants)
* API_TTD_DATE   Date Format (MM/dd-hh:mm:ss.uuuuuu)
*
*    TTRecFmt    Recording Time Format:
* -------------- --------------------------------------------------------------
* API_TT_DEFAULT Unchanged from previous call
* API_TTI_ZERO   Time set to zero when BM Started (All board variants, default)
* API_TTI_DAY    Time of day, relative to midnight when Bus Monitor Started
*                (Host Clock reference)
* API_TTI_IRIG   Time of year (IRIG format) (Host clock reference)
* API_TTI_EXT    External time reference (provided by function BusTools_TimeSet
*                in the user-supplied DLL specified by DLLname)
*
*     TTMode     Time Tag counter operating mode:
* -------------- --------------------------------------------------------------
* API_TT_DEFAULT Unchanged from previous call
* API_TTM_FREE   Free running time tag counter (All board variants, default)
* API_TTM_RESET  Time Tag counter reset to zero on external TTL input
*                discrete active (All board variants).
* API_TTM_SYNC   Sync the Time Tag to the external TTL input.
*                The TTPeriod parameter sets the period of the external TTL
*                input in microseconds.
* API_TTM_RELOD  Time Tag counter is reset to the value previously loaded into
*                the Time Tag Load register (see BusTools_TimeTagWrite).
*
*  RETURNS
*     API_SUCCESS              // no errors.
*     API_HARDWARE_NOSUPPORT   // Function not supported by current hardware
*     API_TIMETAG_BAD_DISPLAY  // Unknown display format
*     API_TIMETAG_BAD_INIT;    // Unknown Time Tag Initialization method
*     API_TIMETAG_BAD_MODE;    // Unknown Time Tag Operating Mode
*     API_TIMETAG_NO_DLL       // DLL containing BusTools_TimeTagGet() could not be loaded
*     API_TIMETAG_NO_FUNCTION  // Error getting address of the BusTools_TimeTagGet() function
*     API_TIMETAG_USER_ERROR   // User function BusTools_TimeTagGet() returned an error
*     API_NO_OS_SUPPORT        // Function not supported by underlying Operating System
*
*****************************************************************************/

NOMANGLE BT_INT CCONV BusTools_TimeTagMode(
   BT_UINT   cardnum,       // (i) card number (0 - based)
   BT_INT   TTDisplay,      // (i) time tag display format
   BT_INT   TTInit,         // (i) time tag counter initialization mode
   BT_INT   TTMode,         // (i) time tag timer operation mode
   char     *DLLname,       // (i) name of the DLL containing time function
   BT_U32BIT TTPeriod,      // (i) period of external TTL sync input
   BT_U32BIT lParm1,        // (i) spare parm1
   BT_U32BIT lParm2)        // (i) spare parm2
{
   /***********************************************************************
   *  Check for legal call
   ***********************************************************************/

   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;
   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if(!board_is_v5_uca[cardnum])
      TTPeriod*=1000;

   /***********************************************************************
   *  Dispatch on the display format definition specified.  This is the
   *   mode used by BusTools_TimeGetString() to convert time to ASCII.
   * This is a global parameter, global to all cards!
   ***********************************************************************/
   switch ( TTDisplay )
   {
      case API_TT_DEFAULT:   // Leave previous display format unchanged.
         break;
      case API_TTD_RELM:     // Display time relative to midnight this morning
         TTDisplay_fmt = API_TTD_RELM;
         break;
      case API_TTD_IRIG:     // Display time in IRIG format
         TTDisplay_fmt = API_TTD_IRIG;
         break;
      case API_TTD_DATE:     // Display time in month/day format
         TTDisplay_fmt = API_TTD_DATE;
         break;
      case API_TTD_RELM_NS:     // Display time relative to midnight this morning
         TTDisplay_fmt = API_TTD_RELM_NS;
         break;
      case API_TTD_IRIG_NS:     // Display time in IRIG format
         TTDisplay_fmt = API_TTD_IRIG_NS;
         break;
      case API_TTD_DATE_NS:     // Display time in month/day format
         TTDisplay_fmt = API_TTD_DATE_NS;
         break;
      default:               // Everything else comes here.
         return API_TIMETAG_BAD_DISPLAY;  // Unknown display format
   }
   if(board_using_shared_memory[cardnum])
      vbtWriteSharedMemory[cardnum](cardnum,&TTDisplay_fmt,
                                    SHRMEM_CHAN_INFO + (CurrentCardSlot[cardnum] * SHRMEM_CHAN_SIZE) + CHAN_TT_DISP,1);

   /***********************************************************************
   *  Dispatch on the Time Tag Initialization Type definition specified.
   *  This is the method used to initialize the time tag counter.
   ***********************************************************************/
   switch ( TTInit )
   {
      case API_TT_DEFAULT:   // Leave previous TT initialization mode unchanged.
         break;
      case API_TTI_ZERO:     // Zero the TT counter when starting the Bus Monitor
         TTInit_type[cardnum] = API_TTI_ZERO;
         break;
      case API_TTI_DAY:      // Load the TT counter with time of day relative to midnight
         TTInit_type[cardnum] = API_TTI_DAY;
         break;
      case API_TTI_IRIG:     // Load the TT counter with IRIG time from the host clock
         TTInit_type[cardnum] = API_TTI_IRIG;
         break;
      case API_TTI_EXT:      // Load the TT counter with time from the user function
                             //  BusTools_TimeTagGet() in the DLL named "DLLname".
#if defined(_USER_DLL_)
      {
         HMODULE hTTdll;        // Handle to DLLname.dll

         hTTdll = LoadLibrary((LPCTSTR)DLLname);  // Search standard path for user's DLL

         if ( hTTdll == NULL )
            return API_TIMETAG_NO_DLL;
         // Now get the address of the user's time tag initialization function.
         pBusTools_TimeTagGet[MAX_BTA] = (void *)GetProcAddress(hTTdll, "BusTools_TimeTagGet");
         if ( pBusTools_TimeTagGet[MAX_BTA] == NULL )
            return API_TIMETAG_NO_FUNCTION;
         // Got the function address so the mode is valid.
         TTInit_type[cardnum] = API_TTI_EXT;
         break;
      }
#else 
         return API_NO_OS_SUPPORT; // Function not supported by underlying Operating System
#endif //_USER_DLL_
      default:
         return API_TIMETAG_BAD_INIT;  // Unknown Time Tag Initialization method
   }
   if(board_using_shared_memory[cardnum])
      vbtWriteSharedMemory[cardnum](cardnum,&TTInit_type[cardnum],
                                    SHRMEM_CHAN_INFO + (CurrentCardSlot[cardnum] * SHRMEM_CHAN_SIZE) + CHAN_TT_INIT,1);

   /***********************************************************************
   *  Dispatch on the Time Tag Counter Operating Mode specification.
   *  This defines how the Time Tag Counter runs.
   ***********************************************************************/
   channel_status[cardnum].irig_on=0;
   switch ( TTMode )
   {
      case API_TT_DEFAULT:   // Leave previous operating mode unchanged.
         break;

      case API_TTM_FREE:     // Time Tag counter free runs
         TTMode_Def[cardnum] = API_TTM_FREE;  // Remember mode.
         // Disable clear of Time Tag counter by discrete input
         // Disable reload of Time Tag Counter from holding reg by discrete input
         if(board_is_v5_uca[cardnum])
            api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, 0x0000);
         else   
            vbtSetTTRegister[cardnum](cardnum, TTREG_CONTROL, 0x0000);
         break;
      case API_TTM_RESET:    // Time Tag counter reset to zero by discrete input
         TTMode_Def[cardnum] = API_TTM_RESET;  // Remember mode.
         // Enable clear of Time Tag counter by discrete input
         // Disable reload of Time Tag Counter from holding reg by discrete input
         if(board_is_v5_uca[cardnum])
            api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, 0x0001);
         else
            vbtSetTTRegister[cardnum](cardnum, TTREG_CONTROL, 0x0001);
         break;
      case API_TTM_SYNC:     // Time Tag counter sync'ed to discrete input
         TTMode_Def[cardnum] = API_TTM_SYNC;  // Remember mode.
         TTPeriod_Def[cardnum] = TTPeriod;    // Remember period of sync input.
         // Disable clear of Time Tag Counter from holding reg by discrete input
         // Enable reload of Time Tag counter by discrete input if supported
         // by the current hardware.  If no HW support, emulate the function
         // in the API software.
         if(board_is_v5_uca[cardnum])
            api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, 0x0001);
         else
            vbtSetTTRegister[cardnum](cardnum, TTREG_CONTROL, 0x0001);
         break;
      case API_TTM_RELOD:    // Time Tag counter reset to TT load register by discrete input
         TTMode_Def[cardnum] = API_TTM_RELOD; // Remember mode.
         TTPeriod_Def[cardnum] = TTPeriod;    // Remember period of sync input.
         // Disable clear of Time Tag counter by discrete input
         // Enable reload of Time Tag Counter from holding reg by discrete input       
         if(board_is_v5_uca[cardnum])
            api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, 0x0001);
         else
            vbtSetTTRegister[cardnum](cardnum, TTREG_CONTROL, 0x0001);
         break;

      case API_TTM_IRIG: // Time Tag reset to external or internal IRIG time value
         if(!board_has_irig[cardnum])
            return API_HARDWARE_NOSUPPORT; // Does not support IRIG.
         channel_status[cardnum].irig_on=1;
         TTMode_Def[cardnum] = API_TTM_IRIG; // Remember mode.
         if(board_is_v5_uca[cardnum])
            api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, TIME_IRIG);
         else
	    vbtSetTTRegister[cardnum](cardnum, TTREG_CONTROL, TIME_IRIG);
	 break;
      case API_TTM_AUTO: //AUTO Increment Time Tag Counter on external pusle
         TTMode_Def[cardnum] = API_TTM_AUTO;  // Remember mode.
         TTPeriod_Def[cardnum] = TTPeriod;    // Remember period of sync input.
         if(board_is_v5_uca[cardnum])
            api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, 0x0000);
         else   
            vbtSetTTRegister[cardnum](cardnum, TTREG_CONTROL, 0x0000);
         break;
      case API_TTM_XCLK:  // use 1Mhz external clock
         if(CurrentCardType[cardnum] != R15XMC2)
           return API_HARDWARE_NOSUPPORT;
         // Disable clear of Time Tag counter by discrete input
         // Disable reload of Time Tag Counter from holding reg by discrete input
         if(board_is_v5_uca[cardnum])
         {
            if((_HW_FPGARev[cardnum] & 0xfff) < 0x500)
               return API_HARDWARE_NOSUPPORT;  // no support for external clock before firmware v5.00
            if(lParm1 == TIME_EXT_EDGE)
               api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, TIME_EXT_CLK | TIME_EXT_EDGE);
            else
               api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, TIME_EXT_CLK);
         }
         else
         {
            CEI_UINT32 ext_clk_period;

            if(TTPeriod == 0)
               return API_TIMETAG_BAD_PERIOD; 

            ext_clk_period =  (V6_BASE_TIME/TTPeriod) << 16;

            if(lParm1 == TIME_EXT_EDGE)
               vbtSetTTRegister[cardnum](cardnum, TTREG_CONTROL, TIME_EXT_CLK | TIME_EXT_EDGE | ext_clk_period);
            else
               vbtSetTTRegister[cardnum](cardnum, TTREG_CONTROL, TIME_EXT_CLK | ext_clk_period);
         }
         TTMode_Def[cardnum] = API_TTM_XCLK;  // Remember mode.
         break;

      default:
         return API_TIMETAG_BAD_MODE;  // Unknown Time Tag Operating Mode
   }
   if(board_using_shared_memory[cardnum])
   {
      vbtWriteSharedMemory[cardnum](cardnum,&TTMode_Def[cardnum],
                                    SHRMEM_CHAN_INFO + (CurrentCardSlot[cardnum] * SHRMEM_CHAN_SIZE) + CHAN_TT_MODE,1);

      vbtWriteSharedMemory[cardnum](cardnum,&TTPeriod_Def[cardnum],
                                    SHRMEM_CHAN_INFO + (CurrentCardSlot[cardnum] * SHRMEM_CHAN_SIZE) + CHAN_TT_PER,1);
   }

   if(lParm2 == 0)
      return BusTools_TimeTagInit(cardnum);
   else
      return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE - BusTools_TimeTagInit
*
*  FUNCTION
*     This routine resets the elapsed time counters and sets the
*     timetag counter in the hardware to zero, or to the value specified
*     by TTInit_type[cardnum], as set by BusTools_TimeTagMode().
*     Some modes obtain the time from the user-function "BusTools_TimeTagGet"
*     in the DLL specified by the user.
*
* TTInit_type[]             Time Tag Initializatio Performed
* -------------             --------------------------------
* API_TTI_ZERO              TT counter set to zero,
*                           sofware base time counters set to zero.
* API_TTI_DAY               TT counter loaded with host time since midnight,
*                           software base time counters set to zero.
* API_TTI_IRIG              TT counter loaded with host time since Jan 1,
*                           software base time counters set to zero.
* API_TTI_EXT               TT counter set by user function "BusTools_TimeTagGet",
*                           software base time counters set to zero.
*
*  RETURNS
*     API_SUCCESS             -> success
*     API_BUSTOOLS_BADCARDNUM -> cardnum >= MAX_BTA
*     API_BUSTOOLS_NOTINITED  -> bustools not inited for this card
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_TimeTagInit(
   BT_UINT  cardnum)              // (i) Card number.
{
   /*******************************************************************
   *  Local Variables
   *******************************************************************/
   BT1553_TIME timetag;           // Used to zero the time tag counters.
   int status=API_SUCCESS;
#ifndef NO_HOST_TIME              //
   BT1553_TIME host_time;
#endif //#ifdef NO_HOST_TIME
   /*******************************************************************
   *  Check initial/error conditions
   *******************************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   /*******************************************************************
   *  Initialize the hardware and software time tag components based
   *    on the current initialization mode.
   *******************************************************************/
   switch ( TTInit_type[cardnum] )
   {
   // Initial time set to zero:
   case API_TTI_ZERO:

      /****************************************************************
      *  Reset SW and HW timetag load registers to zero.V4.25.ajh
      ****************************************************************/

      timetag.microseconds = 0;
      timetag.topuseconds = 0;
      BusTools_TimeTagWrite(cardnum, &timetag, 1);

      /****************************************************************
      *  Reset hardware Bus Monitor timetag counter to zero.
      ****************************************************************/

      status = API_SUCCESS;
      break;

   // Initial time set to host time since midnight:
   case API_TTI_DAY:
#ifdef NO_HOST_TIME
      status = API_NO_OS_SUPPORT;
#else
      /****************************************************************
      *  Fetch host time since midnight.
      ****************************************************************/
      if(board_is_v5_uca[cardnum])  
         get_64BitHostTimeTag(API_TTI_DAY, &host_time);
      else
         get_64BitHostTimeTag(API_TTI_DAY64, &host_time);
      /****************************************************************
      *  Initialize the hardware and software time tag components.
      ****************************************************************/
      status = BusTools_TimeTagWrite(cardnum, &host_time, 1);
#endif //NO_HOST_TIME
      break;
      
   // Initial time set to host time since Jan 1:
   case API_TTI_IRIG:
      /****************************************************************
      *  Fetch host time since Jan 1.
      ****************************************************************/
#ifndef NO_HOST_TIME
	   if(TTMode_Def[cardnum] != API_TTM_IRIG)
      {

         if(board_is_v5_uca[cardnum])  
            get_64BitHostTimeTag(API_TTI_IRIG, &host_time);
         else
            get_64BitHostTimeTag(API_TTI_IRIG64, &host_time);
         status = BusTools_TimeTagWrite(cardnum, &host_time, 1);
      }
#else
      status = API_NO_OS_SUPPORT;
#endif //NO_HOST_TIME
      break;
#ifdef _USER_DLL_
   // Initial time set by user-supplied function:
   case API_TTI_EXT:
      /****************************************************************
      *  Fetch user supplied time value.
      ****************************************************************/
      if ( pBusTools_TimeTagGet[cardnum](cardnum, (BT1553_TIME *)&host_time) )
         return API_TIMETAG_USER_ERROR;
      /****************************************************************
      *  Initialize the hardware and software time tag components.
      ****************************************************************/
      status = BusTools_TimeTagWrite(cardnum, (BT1553_TIME *) &host_time, 1);
	  break;
#endif //_USER_DLL_
   default:
      return API_NO_OS_SUPPORT;
   }

   if(TTMode_Def[cardnum] == API_TTM_RESET)
   {
      BT1553_TIME ztime;
      ztime.topuseconds = 0x0;
      ztime.microseconds = 0x0;
      vbtWriteTimeTag[cardnum](cardnum,&ztime);
      vbtWriteTimeTagIncr[cardnum](cardnum,0x0);                    // set the increment value to 0
      if(board_is_v5_uca[cardnum])
         api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, TIME_HWL);
      else
         vbtSetTTRegister[cardnum](cardnum, TTREG_CONTROL, TIME_HWL);  // Set auto increment option
   }
   else if(TTMode_Def[cardnum] == API_TTM_AUTO)
   { 
      vbtWriteTimeTagIncr[cardnum](cardnum,TTPeriod_Def[cardnum]);      // set the increment value
      if(board_is_v5_uca[cardnum])
         api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, TIME_AUTO);
      else
         vbtSetTTRegister[cardnum](cardnum, TTREG_CONTROL, TIME_AUTO);  // Set auto increment option
   }
   else
      vbtWriteTimeTagIncr[cardnum](cardnum,0);

   return status;
}

/****************************************************************************
*
*  PROCEDURE - BusTools_TimeTagRead
*
*  FUNCTION
*     This routine reads the current timetag from the hardware.
*
*  RETURNS
*     API_SUCCESS
*     API_BUSTOOLS_BADCARDNUM -> cardnum >= MAX_BTA
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_HARDWARE_NOSUPPORT  -> Does not support reading Time Tag register.
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_TimeTagRead(
   BT_UINT cardnum,               // (i) board number
   BT1553_TIME * timetag)     // (o) resulting 48-bit time value
{
   /************************************************
   *  Check initial and error conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   // Get the actual time tag value (45 bits) from the board,
   //  and return it to the caller.
   vbtReadTimeTag[cardnum](cardnum, (BT_U32BIT *)timetag);
 
   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE - BusTools_TimeTagWrite
*
*  FUNCTION
*     This routine writes the specified value to the timetag load register,
*     and causes the value to be loaded into the time tag counter.
*
*  RETURNS
*     API_SUCCESS
*     API_BUSTOOLS_BADCARDNUM -> cardnum >= MAX_BTA
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_HARDWARE_NOSUPPORT  -> Does not support writing the Time Tag register.
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_TimeTagWrite(
   BT_UINT cardnum,           // (i) card number (0 - based)
   BT1553_TIME * timetag,     // (i) pointer to time tag structure
   BT_INT flag)               // (i) flag->0 just load the TT Register,
                              //     flag->1 load the TT Register into the counter
{
   /************************************************
   *  Check initial and error conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   // Save the new value of the Time Tag Load Register since we can't read it.
   TTLR_Shadow[cardnum] = *timetag;

   // Write the value out depending on the hardware support provided:

   if (TTMode_Def[cardnum] == API_TTM_IRIG)
      return API_TIMETAG_WRITE_ERROR;
   // Write the specified value to the time tag register.

   vbtWriteTimeTag[cardnum](cardnum, timetag);

   if(board_using_shared_memory[cardnum])
   {
      vbtWriteSharedMemory[cardnum](cardnum,&timetag->microseconds,
                                    SHRMEM_CHAN_INFO + (CurrentCardSlot[cardnum] * SHRMEM_CHAN_SIZE) + CHAN_TT_LSB,1);

      vbtWriteSharedMemory[cardnum](cardnum,&timetag->topuseconds,
                                    SHRMEM_CHAN_INFO + (CurrentCardSlot[cardnum] * SHRMEM_CHAN_SIZE) + CHAN_TT_MSB,1);
   }

   // If specified, load the TT register into the time tag counter.
   // Be sure to maintain the setting of the HW load TT counter bit!
   if ( flag )
   {
      if(board_is_v5_uca[cardnum])
      {
         if ( (TTMode_Def[cardnum] == API_TTM_SYNC) ||
              (TTMode_Def[cardnum] == API_TTM_RELOD)   )
            vbtSetHWRegister(cardnum, HWREG_CONTROL_T_TAG, 0x0003); // SW load, HW load enabled
	     else if (TTMode_Def[cardnum] == API_TTM_RELOD)
            vbtSetHWRegister(cardnum, HWREG_CONTROL_T_TAG, 0); // SW load, HW load enabled		   
         else
            vbtSetHWRegister(cardnum, HWREG_CONTROL_T_TAG, 0x0002); // SW load only.
      }
      else
      {
         if ( (TTMode_Def[cardnum] == API_TTM_SYNC) ||
              (TTMode_Def[cardnum] == API_TTM_RELOD)   )
            vbtSetTTRegister[cardnum](cardnum, TTREG_CONTROL, 0x0003); // SW load, HW load enabled
	     else if (TTMode_Def[cardnum] == API_TTM_RELOD)
            vbtSetTTRegister[cardnum](cardnum, TTREG_CONTROL, 0); // SW load, HW load enabled		   
         else
            vbtSetTTRegister[cardnum](cardnum, TTREG_CONTROL, 0x0002); // SW load only.
      }
   }

   

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE - BusTools_TimeTagReset
*
*  FUNCTION
*     This routine enables an external pulse to reset the time tag counter to zero
*
*  RETURNS
*     API_SUCCESS
*     API_BUSTOOLS_BADCARDNUM -> cardnum >= MAX_BTA
*     API_BUSTOOLS_NOTINITED  -> BusTools API not initialized
*     API_HARDWARE_NOSUPPORT  -> Does not support writing the Time Tag register.
*
****************************************************************************/
NOMANGLE BT_INT CCONV BusTools_TimeTagReset(BT_UINT cardnum, //
                                            BT_UINT tflag)   // EXT_RESET_ENABLE (1) EXT_RESET_DISABLE (0) 
{
   BT_U32BIT rdata;
   BT_U16BIT rdata16;

   /************************************************
   *  Check initial and error conditions
   ************************************************/
   if (cardnum >= MAX_BTA)
      return API_BUSTOOLS_BADCARDNUM;

   if (bt_inited[cardnum] == 0)
      return API_BUSTOOLS_NOTINITED;

   if((_HW_FPGARev[cardnum] & 0x0fff) < 0x500)
      return API_HARDWARE_NOSUPPORT;

   if(tflag >= EXT_RESET_ENABLE)
   {
      if(board_is_v5_uca[cardnum])
      {
         rdata16 = vbtGetHWRegister(cardnum, HWREG_CONTROL_T_TAG);  //Read the time tag control register value
         rdata16 |= TIME_RESET;  //Set the TTC Reset  Enable  bit
         vbtSetHWRegister(cardnum,HWREG_CONTROL_T_TAG,rdata16);     //Write out new value with reset asserted
      } 
      else
      {
         rdata = vbtGetTTRegister[cardnum](cardnum, TTREG_CONTROL);  //Read the time tag control register value
         rdata |= TIME_RESET;  //Set the TTC Reset Enable bit
         vbtSetTTRegister[cardnum](cardnum,TTREG_CONTROL,rdata);     //Write out new value with reset asserted
      }
   }
   else
   {
      if(board_is_v5_uca[cardnum])
      {
         rdata16 = vbtGetHWRegister(cardnum, HWREG_CONTROL_T_TAG);  //Read the time tag control register value
         rdata16 &= ~TIME_RESET;  //Set the TTC Reset  Enable  bit
         vbtSetHWRegister(cardnum,HWREG_CONTROL_T_TAG,rdata16);     //Write out new value with reset asserted
      } 
      else
      {
         rdata = vbtGetTTRegister[cardnum](cardnum, TTREG_CONTROL);  //Read the time tag control register value
         rdata &= ~TIME_RESET;  //Set the TTC Reset Enable bit
         vbtSetTTRegister[cardnum](cardnum,TTREG_CONTROL,rdata);     //Write out new value with reset asserted
      }
   } 

   return API_SUCCESS;  
}

/**************************************************************************
*
*  PROCEDURE NAME - BusTools_IRIG_Calibration
*
*  FUNCTION
*     This routine calibrates the IRIG DAC
*
**************************************************************************/

NOMANGLE BT_INT CCONV BusTools_IRIG_Calibration(BT_UINT cardnum, 
                                                BT_INT flag)        // (i) card number
{

   BT_INT    status;

   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if ( bt_inited[cardnum] == 0 )
      return API_BUSTOOLS_NOTINITED;

   if(!board_has_irig[cardnum])
      return API_HARDWARE_NOSUPPORT;                                      // Does not support IRIG.

   if(board_is_v5_uca[cardnum])
   {
      api_writehwreg(cardnum, HWREG_CONTROL_T_TAG,0);                     //disable IRIG
      status = vbtIRIGCal(cardnum,flag);
      api_writehwreg(cardnum, HWREG_CONTROL_T_TAG, TIME_IRIG);            // enable IRIG
   }
   else
   { 
      vbtSetTTRegister[cardnum](cardnum, TTREG_CONTROL,0);                //disable IRIG
      status = vbtIRIGCalV6(cardnum,flag);
      vbtSetTTRegister[cardnum](cardnum, TTREG_CONTROL, TIME_IRIG);       // enable IRIG
   }

   return status;
}

/**************************************************************************
*
*  PROCEDURE NAME - BusTools_IRIG_Init
*
*  FUNCTION
*     This routine calibrates the IRIG DAC
*
**************************************************************************/

NOMANGLE BT_INT CCONV BusTools_IRIG_Config(BT_UINT cardnum,   // (i) card number
                                           BT_UINT intFlag,   // Internal IRIG or External
                                           BT_UINT outFlag)   // Output Internal IRIG    
{
   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if ( bt_inited[cardnum] == 0 )
      return API_BUSTOOLS_NOTINITED;

   if(!board_has_irig[cardnum])
      return API_HARDWARE_NOSUPPORT; // Does not support IRIG.

   if(board_is_v5_uca[cardnum])
   {
      vbtIRIGConfig(cardnum, (BT_U16BIT)(intFlag | outFlag)); 
      vbtIRIGWriteDAC(cardnum,(BT_U16BIT)IRIG_DEFAULT_DAC);  // Set the IRIG DAC to the default value 
   }
   else
   {
      vbtSetCSCRegister[cardnum](cardnum,IRIG_V6_CTL,(BT_U32BIT)(intFlag | outFlag));
      vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,(IRIG_DEFAULT_DAC + V6_IRIG_DAC_SEL));
   }

   return API_SUCCESS;
}

/**************************************************************************
*
*  PROCEDURE NAME - BusTools_IRIG_Valid
*
*  FUNCTION
*     This routine checks for a valid IRIG signal.
*
*  RETURNS 
*     Error code or Success (valid)
*
**************************************************************************/

NOMANGLE BT_INT CCONV BusTools_IRIG_Valid(BT_UINT cardnum)    
{

   BT_U32BIT valid;
   BT_U16BIT valid16;

   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if ( bt_inited[cardnum] == 0 )
      return API_BUSTOOLS_NOTINITED;

   if(!board_has_irig[cardnum])
      return API_HARDWARE_NOSUPPORT; // Does not support IRIG.

   if(board_is_v5_uca[cardnum])
   {
      vbtIRIGValid(cardnum, &valid16);
      valid = valid16;
   }
   else
      valid = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
   
   if(valid & 0x8)
	   return API_SUCCESS;
   else
	   return API_IRIG_NO_SIGNAL;
}


/**************************************************************************
*
*  PROCEDURE NAME - BusTools_IRIG_Settime
*
*  FUNCTION
*     This routine sets the IRIG Time.  Used for Internal IRIG-B mode
*
**************************************************************************/

NOMANGLE BT_INT CCONV BusTools_IRIG_SetTime(BT_UINT cardnum,    // (i) card number
                                            BT_U64BIT timedate, // (i) timedate to set -1 SYSTEM_TIME
                                            BT_U32BIT flag)     // (i) 0 = GMTIME 1 = LOCALTIME
                                                                //     0x10 = USER_TIME in DDD:HH:MM:SS format
{
   BT_U32BIT IRIGTime;
   union
   {
      struct toy
	  {
#ifdef NON_INTEL_BIT_FIELDS
          BT_U32BIT reserved:2;
	      BT_U32BIT hundred_day:2;
	      BT_U32BIT tens_of_day:4;
          BT_U32BIT unit_day   :4;
          BT_U32BIT tens_of_hr :2;
          BT_U32BIT unit_hr    :4;
          BT_U32BIT tens_of_min:3;
          BT_U32BIT unit_min   :4;
          BT_U32BIT tens_of_sec:3;
          BT_U32BIT unit_sec   :4;
#else  //INTEL-Compatible bit field ordering 
	      BT_U32BIT unit_sec   :4;
	      BT_U32BIT tens_of_sec:3;
	      BT_U32BIT unit_min   :4;
	      BT_U32BIT tens_of_min:3;
	      BT_U32BIT unit_hr    :4;
	      BT_U32BIT tens_of_hr :2;
	      BT_U32BIT unit_day   :4;
	      BT_U32BIT tens_of_day:4;
	      BT_U32BIT hundred_day:2;
              BT_U32BIT reserved:2;
#endif  //NON_INTEL_BIT_FIELDS
	  } TOY;
	  BT_U32BIT timeval;
   }timeconvert;

   struct tm *ltime;
   time_t tt;
   BT_UINT yday,ddd,hh,mm,ss;
   CEI_UINT64 ss_mask  = 0xff;
   CEI_UINT64 mm_mask  = 0xff<<8;  
   CEI_UINT64 hh_mask  = 0xff<<16;
   CEI_UINT64 ddd_mask = (CEI_UINT64)0xfff<<24;

   if ( cardnum >= MAX_BTA )
      return API_BUSTOOLS_BADCARDNUM;

   if ( bt_inited[cardnum] == 0 )
      return API_BUSTOOLS_NOTINITED;

   if(!board_has_irig[cardnum])
      return API_HARDWARE_NOSUPPORT; // Does not support IRIG.

   if(flag & 0x10)
   {
      ss =  (BT_UINT)( timedate  & ss_mask);
      mm =  (BT_UINT)((timedate  & mm_mask)  >> 8);
      hh =  (BT_UINT)((timedate  & hh_mask)  >> 16);
      ddd = (BT_UINT)((timedate  & ddd_mask) >> 24); //

      timeconvert.TOY.unit_sec = ss % 10;  //tm_sec [0 - 61] allows for a leap second or two
      timeconvert.TOY.tens_of_sec = ss/10; 
      timeconvert.TOY.unit_min = mm % 10;  //tm_min [0 - 59]
      timeconvert.TOY.tens_of_min = mm/10;
      timeconvert.TOY.unit_hr = hh % 10;  //tm_hour [0 - 23]
      timeconvert.TOY.tens_of_hr = hh/10;
      timeconvert.TOY.unit_day = (((ddd)%100)%10); //tm_yday [0 - 365] allows for leap year
      timeconvert.TOY.tens_of_day = (((ddd)%100)-timeconvert.TOY.unit_day)/10;
      timeconvert.TOY.hundred_day = (ddd)/100;
      timeconvert.TOY.reserved = 0;

   }
   else
   {
      //get the time from the system clock
      if(timedate == (BT_U64BIT)-1)
      {
         timedate = (BT_U64BIT)time(&tt);
      }

      // convert the the timedate to the irig stuff
      if(flag)
         ltime = localtime((const time_t*)&timedate);
      else
         ltime = gmtime((const time_t *)&timedate);

      timeconvert.TOY.unit_sec = ltime->tm_sec % 10;  //tm_sec [0 - 61] allows for a leap second or two
      timeconvert.TOY.tens_of_sec = ltime->tm_sec/10; 
      timeconvert.TOY.unit_min = ltime->tm_min % 10;  //tm_min [0 - 59]
      timeconvert.TOY.tens_of_min = ltime->tm_min/10;
      timeconvert.TOY.unit_hr = ltime->tm_hour % 10;  //tm_hour [0 - 23]
      timeconvert.TOY.tens_of_hr = ltime->tm_hour/10;
      yday = ltime->tm_yday+1;                        //ltime yday is 0-based; IRIG is 1-based

      timeconvert.TOY.unit_day = (((yday)%100)%10); //tm_yday [0 - 365] allows for leap year
      timeconvert.TOY.tens_of_day = (((yday)%100)-timeconvert.TOY.unit_day)/10;
      timeconvert.TOY.hundred_day = (yday)/100;
      timeconvert.TOY.reserved = 0;
   }

   IRIGTime = timeconvert.timeval;  
 
   if(board_is_v5_uca[cardnum])
   {
      BT_U16BIT itime_l, itime_m;
      itime_l = (BT_U16BIT)(IRIGTime & 0x0000ffff);
      itime_m = (BT_U16BIT)((IRIGTime & 0xffff0000) >> 16); 
      vbtIRIGSetTime(cardnum,itime_l,itime_m);
   }
   else
      vbtSetCSCRegister[cardnum](cardnum,IRIG_V6_TOY,IRIGTime);

   return API_SUCCESS;
}

/****************************************************************************
*
*  PROCEDURE - TimeTagConvert
*
*  FUNCTION
*     This routine converts the current 45-bit hardware timetag 
*     to 48-bit microseconds as follows:
*
*            time_actual = time_tag + time_current.
*
*     where time_current is the initial time setup by BusTools_SetTimeTag().
*
****************************************************************************/
void TimeTagConvert(
   BT_UINT          cardnum,      // (i) Card number.
   BT1553_V5TIME *time_tag,       // (i) 48-bit Hardware value to be updated.
   BT1553_TIME *time_actual)      // (o) Pointer to resulting 64-bit time.
{
   /***********************************************************************
   *  Local variables
   ***********************************************************************/
   BT_U32BIT    microseconds;     // Lower time-tag dword temp.
   BT_U16BIT    topuseconds;      // Upper time-tag word temp.
   BT_U32BIT    micro_save;
   /***********************************************************************
   *  Add supplied hardware timetag to initial time.
   ***********************************************************************/
   micro_save = flips(time_tag->microseconds);
   time_tag->microseconds = micro_save;

   microseconds = time_current[cardnum].microseconds + time_tag->microseconds; 
   topuseconds  = (BT_U16BIT)
                 (time_current[cardnum].topuseconds  + time_tag->topuseconds);

   // Add in the carry if there is one.
   if ( microseconds < time_tag->microseconds)    // Is there a carry?
      topuseconds++;                               // Yes, add it in...

   /*******************************************************************
   *  Return the total time (time_tag + time_current[cardnum]).
   *******************************************************************/

   time_actual->topuseconds = topuseconds;
   time_actual->microseconds = microseconds;
   return;
}

/*===========================================================================*
 * API ENTRY POINT:      v b t I R I G C a l
 *===========================================================================*
 *
 * FUNCTION:    Calibrates the IRIG DAC for an external input IRIG signal
 *
 * DESCRIPTION: This routine sets the DAC to Vmin + .825(Vmax -Vmin).  Vmin is
 *              the lower peak value and Vmax is the upper peak value.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtIRIGCal(BT_UINT cardnum,BT_INT flag)
{
   BT_U16BIT *irig_dac_reg;
   BT_U16BIT *irig_cal_reg;

   BT_U16BIT dac_min=0,dac_max=0xff;

   BT_U16BIT vmin, vmax, cdata;
   BT_U16BIT dac_start_value=0, dac_start_increment=0;
   BT_U16BIT margin=0;
   int dac_value, dac_increment;
   int bad_signal=0;

   int off;
   int found;

#define dprint(a,b) if(flag){printf(a,b);}

   irig_dac_reg = (BT_U16BIT *)(bt_PageAddr[cardnum][3] + IRIG_CTL_BASE + IRIG_DAC_REG);
   irig_cal_reg = (BT_U16BIT *)(bt_PageAddr[cardnum][3] + IRIG_CTL_BASE + IRIG_CNTL_REG);

   // set the DAC to 0x80 and increase until the upper peak is found
   // the signal range.

   found = FALSE;
   dac_value = 0x80;

   dprint("Initial DAC Setting = %x\n",dac_value);
   *irig_dac_reg = flipws(dac_value);  // Set the DAC value
   MSDELAY(5);                   // Wait 5 Msec for signal
   cdata = irig_cal_reg[0];    // Read the cal register to clear it
   MSDELAY(30);                  // Wait 30 Msec for signal
   cdata = irig_cal_reg[0];    // read the cal register again
   flipw(&cdata);
   if((cdata & 0x1) == 0x1)
   {
      //make sure we are not at a ragged edge
      *irig_dac_reg = flipws((dac_value+3));  // Set the DAC value
      MSDELAY(5);                   // Wait 5 Msec for signal
      cdata = irig_cal_reg[0];    // Read the cal register to clear it
      MSDELAY(30);                  // Wait 30 Msec for signal
      cdata = irig_cal_reg[0];    // read the cal register again
      flipw(&cdata);
      if((cdata & 0x1)==0x0)
        bad_signal+=1;
         
      *irig_dac_reg = flipws((dac_value-3));  // Set the DAC value
      MSDELAY(5);                   // Wait 5 Msec for signal
      cdata = irig_cal_reg[0];    // Read the cal register to clear it
      MSDELAY(30);                  // Wait 30 Msec for signal
      cdata = irig_cal_reg[0];    // read the cal register again
      flipw(&cdata);
      if((cdata & 0x1)==0x0)
         bad_signal+=1;
   }
   else
      bad_signal+=1;

   if(!bad_signal)    // if the cal bit is set we inside the IRIG-B signal 
   {
	  dac_start_value = 0x80;
      dac_start_increment = 0x40;
      dac_min = 0;
	  dac_max =0xff;
      dprint("IRIG-B Signal found at %x initial setting\n", dac_value);
      dprint("dac_start_value     = %x\n",dac_start_value);
      dprint("dac_start_increment = %x\n",dac_start_increment);
      dprint("dac_min             = %x\n",dac_min);
      dprint("dac_max             = %x\n",dac_max);
   }
   else  // search for a point within the IRIG-B singal to start the search.
   {
	  for (dac_value = 0x88;dac_value<0x100;dac_value+=0x8)
	  {
	     *irig_dac_reg = flipws(dac_value);  // Set the DAC value
         dprint("Finding the signal above the mid point -- DAC Setting = %x\n",dac_value);
         MSDELAY(5);                   // Wait 5 Msec for signal
         cdata = irig_cal_reg[0];    // Read the cal register to clear it
         MSDELAY(30);                  // Wait 30 Msec for signal
         cdata = irig_cal_reg[0];    // read the cal register again
         flipw(&cdata);
         if((cdata & 0x1) == 0x1)    // if the cal register is set make sure its not just some noise
		 {
            dac_min = 0x7f;
			dac_max = 0xff;
			dac_start_value = dac_value+3;//(dac_value+dac_max)/2;
			dac_start_increment = 0x20;
			found = TRUE;
            dprint("IRIG-B Signal found at %x above the mid point\n", dac_value);
            dprint("dac_start_value     = %x\n",dac_start_value);
            dprint("dac_start_increment = %x\n",dac_start_increment);
            dprint("dac_min             = %x\n",dac_min);
            dprint("dac_max             = %x\n",dac_max);
			break;
         }
      }
      if(found==FALSE)
	  {
         for (dac_value = 0x78;dac_value>0x0;dac_value-=0x8)
         {
	        *irig_dac_reg = flipws(dac_value);  // Set the DAC value
	        dprint("Finding the signal below the mid point -- DAC Setting = %x\n",dac_value);
            MSDELAY(5);                   // Wait 5 Msec for signal
            cdata = irig_cal_reg[0];    // Read the cal register to clear it
            MSDELAY(30);                  // Wait 30 Msec for signal
            cdata = irig_cal_reg[0];    // read the cal register again
            flipw(&cdata);
            if((cdata & 0x1) == 0x1)    // 
			{
			    dac_min = 0;
			    dac_max = 0x81;
			    dac_start_increment = 0x20;
				dac_start_value = dac_value-3;//(dac_value-dac_min)/2;
                dprint("IRIG-B Signal found at %x below the mid point\n", dac_value);
                dprint("dac_start_value     = %x\n",dac_start_value);
                dprint("dac_start_increment = %x\n",dac_start_increment);
                dprint("dac_min             = %x\n",dac_min);
                dprint("dac_max             = %x\n",dac_max);
			    found = TRUE;
				break;
            }
         }
		 if(found==FALSE)
            return BTD_IRIG_NO_SIGNAL;
      }
   }

   dprint("Signal found at %x.  Now calibrating\n",dac_value);
   found = FALSE;
   dac_increment = dac_start_increment;
   dac_value = dac_start_value;
   while(1)
   {  
      *irig_dac_reg = flipws(dac_value);  // Set the DAC value
	  dprint("Finding the lower transition -- DAC Setting = %x\n",dac_value);
      MSDELAY(5);                   // Wait 5 Msec for signal
      cdata = irig_cal_reg[0];    // Read the cal register to clear it
	  MSDELAY(30);                  // Wait 30 Msec for signal
	  cdata = irig_cal_reg[0];    // read the cal register again
      flipw(&cdata);	
      if((cdata & 0x1) == 0x1)    // 
      {
         dac_value = dac_value - dac_increment;
		 if(dac_increment <= 4)
		 {
            off = -1;
            //fine_tune
            dac_increment = 1;
            do
			{
			   *irig_dac_reg = flipws(dac_value);
	           dprint("Fine tuning lower transition -- DAC Setting = %x\n",dac_value);
			   cdata = irig_cal_reg[0];
			   MSDELAY(20);
			   cdata = irig_cal_reg[0];
               flipw(&cdata);
			   if((cdata & 0x1) == 0x1)
			   {
                  if(off == 1)
				  {
					  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
			         off = 0; //false
                     dac_value -= dac_increment;
				  }
			   }
			   else
			   {
                  if(off == 0)
				  {
					  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
                     off = 1; //true
                     dac_value += dac_increment;
				  }
			   }
			}while(found==FALSE);
			break;
		 }
	  }
      else
	  {
		  dac_value+=dac_increment;
		  if (dac_value >= dac_max)
		    return BTD_IRIG_NO_LOW_PEAK;
	  }
	  dac_increment=dac_increment/2;
	  if(dac_increment == 0)
		  dac_increment = 2;
   }
   vmin = dac_value+margin;
   dprint("Lower transition value = %x\n",vmin);   

   found = FALSE;
 
   dac_value = (vmin+dac_max)/2; // start at the mid point between vmin and the dac_max
   dprint("Start the upper transition search at %x\n",dac_start_value);
   dac_increment = (0xff - vmin)/2;
   while(1)
   {  
      *irig_dac_reg = flipws(dac_value);
	  dprint("Finding upper transition -- DAC Setting = %x\n",dac_value);
      MSDELAY(30);                  // Wait 20 Msec for signal
      cdata = irig_cal_reg[0];    // Read the cal register to clear it
	  MSDELAY(30);                  // Wait 20 Msec for signal
	  cdata = irig_cal_reg[0];    // read the cal register again
      flipw(&cdata);
	  if((cdata & 0x1) == 0x1)    // if the cal register is set let make sure it not just some noise
      {
         dac_value += dac_increment;
         if(dac_increment <= 4)
		 {
			off = -1;
            //fine_tune
            dac_increment = 1;
            do
			{
			   *irig_dac_reg = flipws(dac_value);
	           dprint("Fine tuning upper transition -- DAC Setting = %x\n",dac_value);
			   cdata = irig_cal_reg[0];
			   MSDELAY(20);
			   cdata = irig_cal_reg[0];
               flipw(&cdata);
			   if((cdata & 0x1) == 0x1 )
			   {
                  if(off == 1)
				  {
            		  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
			         off = 0; //false
                     dac_value += dac_increment;
				  }
			   }
			   else
			   {
                  if(off == 0)
				  {
					  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
                     off = 1; //true
                     if(dac_value>vmin)
                        dac_value -= dac_increment;
                     else
                        dac_value += dac_increment;
				  }
			   }
			}while(found==FALSE);
			break;
         }
	  }
      else
	  {
          if(dac_value > vmin)
          {
             dac_value-=dac_increment;
             if(dac_value < 0)
               dac_value = 0;
          }
          else
          {
             dac_value+=dac_increment;
             if(dac_value > 0xff)
                dac_value = 0xff;
          }
	  }
	  dac_increment = dac_increment/2;
      if(dac_increment == 0)
         dac_increment = 2;
   }
   vmax = dac_value-margin;
   dprint("Upper transition value = %x\n",vmax);

   dac_value = vmin + (BT_U16BIT)((825*(vmax-vmin))/1000);
   *irig_dac_reg = flipws(dac_value);
   dprint("vmin = %d\n",vmin);
   dprint("vmax = %d\n",vmax);
   dprint("vpp  = %d\n",vmax-vmin);
   dprint("final DAC setting = %d\n",dac_value);
   *irig_dac_reg = flipws(dac_value);

   if((vmax-vmin)<MIN_DAC_LEVEL)
	   return BTD_IRIG_LEVEL_ERR;

   return API_SUCCESS;
}


/*===========================================================================*
 * API ENTRY POINT:      v b t I R I G C a l V6
 *===========================================================================*
 *
 * FUNCTION:    Calibrates the IRIG DAC for an external input IRIG signal
 *
 * DESCRIPTION: This routine sets the DAC to Vmin + .825(Vmax -Vmin).  Vmin is
 *              the lower peak value and Vmax is the upper peak value.
 *
 *      It will return:
 *              0 if successful, non-zero upon error.
 *===========================================================================*/
BT_INT vbtIRIGCalV6(BT_UINT cardnum,BT_INT flag)
{
   BT_U16BIT dac_min=0,dac_max=0xff;

   BT_U32BIT vmin, vmax, cdata;
   BT_U16BIT dac_start_value=0, dac_start_increment=0;
   BT_U16BIT margin=0;
   BT_UINT dac_value, dac_increment;
   int bad_signal=0;

   int off;
   int found;

#define dprint(a,b) if(flag){printf(a,b);}

   // set the DAC to 0x80 and increase until the upper peak is found
   // the signal range.

   found = FALSE;
   dac_value = 0x80;

   dprint("Initial DAC Setting = %x\n",dac_value);

   vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,dac_value + V6_IRIG_DAC_SEL);
   MSDELAY(5);                   // Wait 5 Msec for signal

   cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
   MSDELAY(30);                  // Wait 30 Msec for signal

   cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);

   if((cdata & 0x1) == 0x1)
   {
      //make sure we are not at a ragged edge
      vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,(dac_value + 3 + V6_IRIG_DAC_SEL));
      MSDELAY(5);                   // Wait 5 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
      MSDELAY(30);                  // Wait 30 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
      if((cdata & 0x1)==0x0)
        bad_signal+=1;
         
      vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,((dac_value - 3) + V6_IRIG_DAC_SEL));
      MSDELAY(5);                   // Wait 5 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
      MSDELAY(30);                  // Wait 30 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
      if((cdata & 0x1)==0x0)
         bad_signal+=1;
   }
   else
      bad_signal+=1;

   if(!bad_signal)    // if the cal bit is set we inside the IRIG-B signal 
   {
	  dac_start_value = 0x80;
      dac_start_increment = 0x40;
      dac_min = 0;
	  dac_max =0xff;
      dprint("IRIG-B Signal found at %x initial setting\n", dac_value);
      dprint("dac_start_value     = %x\n",dac_start_value);
      dprint("dac_start_increment = %x\n",dac_start_increment);
      dprint("dac_min             = %x\n",dac_min);
      dprint("dac_max             = %x\n",dac_max);
   }
   else  // search for a point within the IRIG-B singal to start the search.
   {
	  for (dac_value = 0x88;dac_value<0x100;dac_value+=0x8)
	  {
         vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,(dac_value + V6_IRIG_DAC_SEL));
         dprint("Finding the signal above the mid point -- DAC Setting = %x\n",dac_value);
         MSDELAY(5);                   // Wait 5 Msec for signal
         cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
         MSDELAY(30);                  // Wait 30 Msec for signal
         cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);

         if((cdata & 0x1) == 0x1)    // if the cal register is set make sure its not just some noise
		 {
            dac_min = 0x7f;
			dac_max = 0xff;
			dac_start_value = dac_value+3;//(dac_value+dac_max)/2;
			dac_start_increment = 0x20;
			found = TRUE;
            dprint("IRIG-B Signal found at %x above the mid point\n", dac_value);
            dprint("dac_start_value     = %x\n",dac_start_value);
            dprint("dac_start_increment = %x\n",dac_start_increment);
            dprint("dac_min             = %x\n",dac_min);
            dprint("dac_max             = %x\n",dac_max);
			break;
         }
      }
      if(found==FALSE)
	  {
         for (dac_value = 0x78;dac_value>0x0;dac_value-=0x8)
         {
            vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,(dac_value + V6_IRIG_DAC_SEL));
	        dprint("Finding the signal below the mid point -- DAC Setting = %x\n",dac_value);
            MSDELAY(5);                   // Wait 5 Msec for signal
            cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
            MSDELAY(30);                  // Wait 30 Msec for signal
            cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);

            if((cdata & 0x1) == 0x1)    // 
			{
			    dac_min = 0;
			    dac_max = 0x81;
			    dac_start_increment = 0x20;
				dac_start_value = dac_value-3;//(dac_value-dac_min)/2;
                dprint("IRIG-B Signal found at %x below the mid point\n", dac_value);
                dprint("dac_start_value     = %x\n",dac_start_value);
                dprint("dac_start_increment = %x\n",dac_start_increment);
                dprint("dac_min             = %x\n",dac_min);
                dprint("dac_max             = %x\n",dac_max);
			    found = TRUE;
				break;
            }
         }
		 if(found==FALSE)
            return BTD_IRIG_NO_SIGNAL;
      }
   }

   dprint("Signal found at %x.  Now calibrating\n",dac_value);
   found = FALSE;
   dac_increment = dac_start_increment;
   dac_value = dac_start_value;
   while(1)
   {  
      vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,(dac_value + V6_IRIG_DAC_SEL));
	  dprint("Finding the lower transition -- DAC Setting = %x\n",dac_value);
      MSDELAY(5);                   // Wait 5 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
	  MSDELAY(30);                  // Wait 30 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);

      if((cdata & 0x1) == 0x1)    // 
      {
         dac_value = dac_value - dac_increment;
		 if(dac_increment <= 4)
		 {
            off = -1;
            //fine_tune
            dac_increment = 1;
            do
			{
               vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,(dac_value + V6_IRIG_DAC_SEL));
	           dprint("Fine tuning lower transition -- DAC Setting = %x\n",dac_value);
               cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
			   MSDELAY(20);
               cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);

			   if((cdata & 0x1) == 0x1)
			   {
                  if(off == 1)
				  {
					  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
			         off = 0; //false
                     dac_value -= dac_increment;
				  }
			   }
			   else
			   {
                  if(off == 0)
				  {
					  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
                     off = 1; //true
                     dac_value += dac_increment;
				  }
			   }
			}while(found==FALSE);
			break;
		 }
	  }
      else
	  {
		  dac_value+=dac_increment;
		  if (dac_value >= dac_max)
		    return BTD_IRIG_NO_LOW_PEAK;
	  }
	  dac_increment=dac_increment/2;
	  if(dac_increment == 0)
		  dac_increment = 2;
   }
   vmin = dac_value+margin;
   dprint("Lower transition value = %x\n",vmin);   

   found = FALSE;
 
   dac_value = (vmin+dac_max)/2; // start at the mid point between vmin and the dac_max
   dprint("Start the upper transition search at %x\n",dac_start_value);
   dac_increment = (0xff - vmin)/2;
   while(1)
   {  
      vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,(dac_value + V6_IRIG_DAC_SEL));
	  dprint("Finding upper transition -- DAC Setting = %x\n",dac_value);
      MSDELAY(30);                  // Wait 20 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
	  MSDELAY(30);                  // Wait 20 Msec for signal
      cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);

	  if((cdata & 0x1) == 0x1)    // if the cal register is set let make sure it not just some noise
      {
         dac_value += dac_increment;
         if(dac_increment <= 4)
		 {
			off = -1;
            //fine_tune
            dac_increment = 1;
            do
			{
               vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,(dac_value + V6_IRIG_DAC_SEL));
	           dprint("Fine tuning upper transition -- DAC Setting = %x\n",dac_value);
               cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);
			   MSDELAY(20);
               cdata = vbtGetCSCRegister[cardnum](cardnum,IRIG_V6_CTL);

			   if((cdata & 0x1) == 0x1 )
			   {
                  if(off == 1)
				  {
            		  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
			         off = 0; //false
                     dac_value += dac_increment;
				  }
			   }
			   else
			   {
                  if(off == 0)
				  {
					  found = TRUE;
					  margin = 0;
				  }
				  else
				  {
                     off = 1; //true
                     if(dac_value>vmin)
                        dac_value -= dac_increment;
                     else
                        dac_value += dac_increment;
				  }
			   }
			}while(found==FALSE);
			break;
         }
	  }
      else
	  {
          if(dac_value > vmin)
          {
             dac_value-=dac_increment;
             if((BT_INT)dac_value < 0) 
                dac_value = 0;
          }
          else
          {
             dac_value+=dac_increment;
             if(dac_value > 0xff)
                dac_value = 0xff;
          }
	  }
	  dac_increment = dac_increment/2;
      if(dac_increment == 0)
         dac_increment = 2;
   }
   vmax = dac_value-margin;
   dprint("Upper transition value = %x\n",vmax);

   dac_value = vmin + (BT_U16BIT)((825*(vmax-vmin))/1000);
   vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,(dac_value + V6_IRIG_DAC_SEL));
   dprint("vmin = %d\n",vmin);
   dprint("vmax = %d\n",vmax);
   dprint("vpp  = %d\n",vmax-vmin);
   dprint("final DAC setting = %d\n",dac_value);
   vbtSetCSCRegister[cardnum](cardnum,V6_DAC_CTRL,(dac_value + V6_IRIG_DAC_SEL));

   if((vmax-vmin)<MIN_DAC_LEVEL)
	   return BTD_IRIG_LEVEL_ERR;

   return API_SUCCESS;
}
/*===========================================================================*
 * API ENTRY POINT:      v b t I R I G C o n f i g
 *===========================================================================*
 *
 * FUNCTION:    configure the IRIG contrl register.
 *
 * DESCRIPTION: The function write the value to the IRIG control register
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/
void vbtIRIGConfig(BT_UINT cardnum, BT_U16BIT value)
{
   BT_U16BIT * irig_cntl_reg;

   irig_cntl_reg = (BT_U16BIT *)(bt_PageAddr[cardnum][3] + IRIG_CTL_BASE + IRIG_CNTL_REG);
   *irig_cntl_reg = flipws(value);
}

/*===========================================================================*
 * API ENTRY POINT:      v b t I R I G V a l i d
 *===========================================================================*
 *
 * FUNCTION:    configure the IRIG contrl register.
 *
 * DESCRIPTION: The function returns the valid bit setting in the config register
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/
void vbtIRIGValid(BT_UINT cardnum, BT_U16BIT * valid)
{
   BT_U16BIT * irig_cntl_reg;

   irig_cntl_reg = (BT_U16BIT *)(bt_PageAddr[cardnum][3] + IRIG_CTL_BASE + IRIG_CNTL_REG);
   *valid = (BT_U16BIT)(flipws(*irig_cntl_reg) & 0x8);
}

/*===========================================================================*
 * API ENTRY POINT:      v b t I R I G W r i t e D A C
 *===========================================================================*
 *
 * FUNCTION:    Set IRIG DAC register.
 *
 * DESCRIPTION: The function write the value to the IRIG DAC register
 *
 *      It will return:
 *              nothing.
 *===========================================================================*/
void vbtIRIGWriteDAC(BT_UINT cardnum, BT_U16BIT value)
{
   BT_U16BIT * irig_dac_reg;

   irig_dac_reg = (BT_U16BIT *)(bt_PageAddr[cardnum][3] + IRIG_CTL_BASE + IRIG_DAC_REG);
   *irig_dac_reg = flipws(value); 
}

/*===========================================================================*
 * API ENTRY POINT:      v b t I R I G S E T T I M E
 *===========================================================================*
 *
 * FUNCTION:    Set the IRIG time value.
 *
 * DESCRIPTION: The function write the value to the IRIG TOY register
 *
 *      It will return:
 *              nothing
 *===========================================================================*/
void vbtIRIGSetTime(BT_UINT cardnum, BT_U16BIT time_lsb, BT_U16BIT time_msb)
{
   BT_U16BIT *irig_toy_reg;

   irig_toy_reg = (BT_U16BIT *)(bt_PageAddr[cardnum][3] + IRIG_CTL_BASE + IRIG_TOY_REG_LSB);

   irig_toy_reg[0] = flipws(time_lsb);
   irig_toy_reg[1] = flipws(time_msb);
}





