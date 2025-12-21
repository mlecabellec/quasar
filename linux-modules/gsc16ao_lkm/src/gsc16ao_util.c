// vim:ts=4 expandtab:
/*****************************************************************************
 *                                                                           *
 * File:         gsc16ao_util.c                                              *
 *                                                                           *
 * Description:  gsc16ao driver utility functions                            *
 *                                                                           *
 *                                                                           *
 * Date:         8/28/2012                                                   *
 * History:                                                                  *
 *                                                                           *
 *   2  8/28/12 D. Dubash                                                    *
 *              Get rid of warning on RH6.0 32-bit.                          *
 *                                                                           *
 *   1  5/30/03 G. Barton                                                    *
 *              Created                                                      *
 *                                                                           *
 *  Copyrights (c):                                                          *
 *      Concurrent Computer Corporation, 2003                                *
 *****************************************************************************/

//#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>

#include "gsc16ao_util.h"
//#include <string.h>

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#undef KERN_CRIT
#define KERN_CRIT "CRITICAL ERROR:"
#undef KERN_ERR
#define KERN_ERR "ERROR:"
#undef KERN_WARN
#define KERN_WARN "WARNING:"
#undef KERN_NOTICE
#define KERN_NOTICE "NOTICE:"
#undef KERN_INFO
#define KERN_INFO "INFO:"
#undef KERN_DEBUG
#define KERN_DEBUG "DEBUG:"
#endif

extern unsigned char _debug_level;
extern unsigned char _debug_class_flags;
static int need_header = 1;

void
prmsg(unsigned char dlevel, unsigned char dclass, char *pFormat, ...)
{
  char msgbuf[1020];  /* Same size as printk buffer */
  va_list args;
  int len = 0;
  
  if ((dclass == GDC_ALWAYS) ||
      ((dclass & _debug_class_flags) && (_debug_level >= dlevel))) {
    /*
     * Message is enabled...
     */
    if (need_header) {
      switch (dlevel) {
      case GDL_CRITERR:
	strcpy (msgbuf, KERN_CRIT);
	break;
	
      case GDL_ERR:
	strcpy (msgbuf, KERN_ERR);
	break;
	
      case GDL_WARN:
	strcpy(msgbuf, KERN_WARNING);
	break;
	
      case GDL_NOTICE:
	strcpy(msgbuf, KERN_NOTICE);
	break;
	
      case GDL_INFO:
	strcpy(msgbuf, KERN_INFO);
	break;
	
      case GDL_TRACE1:
      case GDL_TRACE2:
      case GDL_TRACE3:
      case GDL_TRACE4:
      case GDL_ENTRY_EXIT:
	strcpy(msgbuf, KERN_DEBUG);
	break;
      }

      /* Add device name */
      len = strlen(msgbuf);
      strcpy(&msgbuf[len], DEVICE_NAME);
      len = strlen(msgbuf);
    }
    
    /* Format remainder of message */
    va_start(args, pFormat);
    vsnprintf(&msgbuf[len], sizeof(msgbuf) - len - 1, pFormat, args);
    va_end(args);

    msgbuf[sizeof(msgbuf)-1] = 0; /* Paranoia: make sure string is NULL terminated */
    
#ifdef __KERNEL__
    printk("%s", msgbuf);
#else
    printf("%s", msgbuf);
#endif
    
    if (msgbuf[strlen(msgbuf)-1] != '\n')
      need_header = 0;
    else
      need_header = 1;
  }
}
