// vim:ts=4 expandtab:
/*****************************************************************************
 *                                                                           *
 * File:         gsc16ao_util.h                                              *
 *                                                                           *
 * Description:  The interface to the GSC16AO Linux device driver.           *
 *                                                                           *
 * Date:         5/30/2003                                                   *
 * History:                                                                  *
 *                                                                           *
 *   1  5/30/03 G. Barton                                                    *
 *              Created                                                      *
 *                                                                           *
 *  Copyrights (c):                                                          *
 *      Concurrent Computer Corporation, 2003                                *
 *****************************************************************************/
#ifndef _GSC16AO_UTIL_H

#define DEVICE_NAME "gsc16ao"

#include <stdarg.h>

/* Filtering frontend for printk/printf. see utils.c */
void prmsg(unsigned char dlevel, unsigned char dclass, char *pFormat, ...);

/* dlevel */
#define GDL_CRITERR       0
#define GDL_ERR           1
#define GDL_WARN          2
#define GDL_NOTICE        3
#define GDL_INFO          4
#define GDL_TRACE1        5
#define GDL_TRACE2        6
#define GDL_TRACE3        7
#define GDL_TRACE4        8
#define GDL_ENTRY_EXIT    9
/* dclass*/
#define GDC_ALWAYS        0xff
#define GDC_INIT          0x01
#define GDC_OPENCLOSE     0x02
#define GDC_RDWR          0x04
#define GDC_IOCTL         0x08
#define GDC_INTR          0x10
#define GDC_MMAP          0x20
#define GDC_REG           0x40
#define GDC_MISC          0x80

/*
 * message output class and level flags
 */
extern unsigned char _debug_class_flags;
extern unsigned char _debug_level;

#endif
