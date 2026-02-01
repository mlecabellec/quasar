/*============================================================================*
 * FILE:                  A P I N A M E S . H
 *============================================================================*
 *
 *      COPYRIGHT (C) 1998-2016 BY ABACO SYSTEMS, INC. 
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
 *
 *
 *===========================================================================*
 *
 * FUNCTION:    Header file for BusTools API trace mechanism.  This file is
 *              used to define the function names for the trace function.
 *
 *===========================================================================*/

/* $Revision:  5.50 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  06/10/1999   Created definitions for trace buffer function.V3.11.ajh
  03/16/2000   Modified to support V4.01 release.V4.01.ajh
 */

#ifndef _BUSNAMES_H_
#define _BUSNAMES_H_

/**********************************************************************
*  Function name definitions for the trace function.
**********************************************************************/

#define NBUSTOOLS_BC_MESSAGEREAD            1    // 0x00000002
#define NBUSTOOLS_BC_START                  2    // 0x00000004
#define NBUSTOOLS_BC_STARTSTOP              3    // 0x00000008
#define NBUSTOOLS_BM_MESSAGEREAD            4    // 0x00000010

#define BM_MSGREADBLOCK                     5    // 0x00000020
#define NBUSTOOLS_BM_STARTSTOP              6    // 0x00000040
#define NBUSTOOLS_RT_MESSAGEGETID           7    // 0x00000080
#define NBUSTOOLS_RT_MESSAGEREAD            8    // 0x00000100

#define NBUSTOOLS_RT_STARTSTOP              9    // 0x00000200
#define NBM_MESSAGECONVERT                  10   // 0x00000400
#define NBM_TRIGGER_OCCUR                   11   // 0x00000800
#define NCALLUUSERTHREAD                    12   // 0x00001000

#define NINTQUEUEENTRY                      13   // 0x00002000
#define NSIGNALUUSERTHREAD                  14   // 0x00004000
#define NTIME_TAG_INTERRUPT                 15   // 0x00008000
#define NTIME_TAG_CLEARFLAG                 16   // 0x00010000

#define NVBTNOTIFY                          17   // 0x00020000
#define NVBTSETUP                           18   // 0x00040000
#define NVBTSHUTDOWN                        19   // 0x00080000

#define NBUS_LOADING_FILTER                 21   // 0x00200000

#if 0
#define DumpTraceMask       NBUSTOOLS_BC_MESSAGEREAD     | \
                            NBUSTOOLS_BC_START           | \
                            NBUSTOOLS_BC_STARTSTOP       | \
                            NBUSTOOLS_BM_MESSAGEREAD     | \
                            BM_MSGREADBLOCK              | \
                            NBUSTOOLS_BM_STARTSTOP       | \
                            NBUSTOOLS_RT_MESSAGEGETID    | \
                            NBUSTOOLS_RT_MESSAGEREAD     | \
                            NBUSTOOLS_RT_STARTSTOP       | \
                            NBM_MESSAGECONVERT           | \
                            NBM_TRIGGER_OCCUR            | \
                            NCALLUUSERTHREAD             | \
                            NINTQUEUEENTRY               | \
                            NSIGNALUUSERTHREAD           | \
                            NTIME_TAG_INTERRUPT          | \
                            NTIME_TAG_CLEARFLAG          | \
                            NVBTNOTIFY                   | \
                            NVBTSETUP                    | \
                            NVBTSHUTDOWN                 
#else
#define DumpTraceMask 0
#endif //0
                      
#endif  // #ifndef _BUSNAMES_H_

