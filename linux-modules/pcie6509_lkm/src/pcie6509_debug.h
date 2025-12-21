/******************************************************************************
 *                                                                            *
 * File:        pcie6509_debug.h                                              *
 *                                                                            *
 * Description: The interface to the PCIE6509 Linux device driver.            *
 *                                                                            *
 * Date:         02/13/2014                                                   *
 * History:                                                                   *
 *                                                                            *
 *  1  02/13/14  D. Dubash                                                    *
 *               Initial release                                              *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 *  Copyright (C) 2010 and beyond Concurrent Computer Corporation             *
 *  All rights reserved                                                       *
 *                                                                            *
 ******************************************************************************/

#ifndef __PCIE6509_DEBUG_H_INCLUDED__
#define __PCIE6509_DEBUG_H_INCLUDED__

/**********************************************************
debug ROUTINES
***********************************************************/
#ifdef    PCIE6509_DEBUG
#define     DbgHdr    "[%-7s %4d]:%2d.%s(): "

#define     D_ENTER         0x00000001  /* enter routine */
#define     D_EXIT          0x00000002  /* exit routine */

#define     D_L1            0x00000004  /* level 1 */
#define     D_L2            0x00000008  /* level 2 */
#define     D_L3            0x00000010  /* level 3 */
#define     D_L4            0x00000020  /* level 4 */

#define     D_ERR           0x00000040  /* level error */
#define     D_WAIT          0x00000080  /* level wait */

#define     D_INT0          0x00000100  /* interrupt level 0 */
#define     D_INT1          0x00000200  /* interrupt level 1 */
#define     D_INT2          0x00000400  /* interrupt level 2 */
#define     D_INT3          0x00000800  /* interrupt level 3 */
#define     D_INTW          0x00001000  /* interrupt wakeup level */
#define     D_INTE          0x00002000  /* interrupt error */

#define     D_RTIME         0x00010000  /* display read times */
#define     D_WTIME         0x00020000  /* display write times */
#define     D_REGS          0x00040000  /* dump registers */
#define     D_IOCTL         0x00080000  /* ioctl call */

#define     D_DATA          0x00100000  /* data level */
#define     D_DMA           0x00200000  /* DMA level */

#define     D_NEVER         0x00000000  /* never print this debug message */
#define     D_ALWAYS        0xffffffff  /* always print this debug message */
#define     D_TEMP          D_ALWAYS    /* Only use for temporary debug code */

#define     D_ALL_LVLS      (D_L1|D_L2|D_L3|D_L4)  /* all levels */
#define     D_ALL_INTS      (D_INT0|D_INT1|D_INT2|D_INT3|D_INTW)
                                             /* all interrupt levels */
#define     D_ALL_ERR       (D_ERR|D_INTE)   /* all error debug */
#define     D_ALL_WAIT      (D_WAIT|D_INTW)  /* all wait debug */

/* debug control */
/* This variable can be used to debug the driver by setting bits in 
 * pcie6509_debug_ctrl and altering the behavior of the driver based on these
 * bits. Contents of this variable can be directly controlled by the user
 * by writing to the /proc/pcie6509 file the argument 
 * echo "pcie6509_debug_ctrl=0x1234" > /proc/pcie6509
 */
static int pcie6509_debug_ctrl = 0;

/* debug mask */
static int pcie6509_debug_mask = D_ENTER |
    D_EXIT |
    D_L1 |
    D_L2 |
    D_L3 |
    D_L4 |
    D_ERR | D_WAIT | D_INT0 | D_INT1 | D_INT2 | D_INT3 | D_INTW | D_INTE |
    /*  D_REGS      | */
    D_IOCTL |
    /*  D_DATA      | */
    D_DMA | 0;

#define TDEBUGP(x,Str,Args...) do {                                     \
     if (pcie6509_debug_mask & (x)) {                                   \
         if(((x) == D_ERR) || ((x) == D_INTE)) {                        \
            if(pcie6509_device)                                         \
                printk( KERN_INFO DbgHdr "ERROR! " Str,                 \
                    PCIE6509_FileName,__LINE__,pcie6509_device->minor,  \
                                __FUNCTION__,Args);                     \
            else                                                        \
                printk( KERN_INFO DbgHdr "ERROR! " Str,                 \
                    PCIE6509_FileName,__LINE__,-1,                      \
                                __FUNCTION__,Args);                     \
         } else {                                                       \
            if(pcie6509_device)                                         \
                printk( KERN_INFO DbgHdr Str,                           \
                    PCIE6509_FileName,__LINE__,pcie6509_device->minor,  \
                                __FUNCTION__,Args);                     \
            else                                                        \
                printk( KERN_INFO DbgHdr Str,                           \
                    PCIE6509_FileName,__LINE__,-1,                      \
                                __FUNCTION__,Args);                     \
         }                                                              \
     }                                                                  \
} while (0)                         
#define DEBUGPx(x,Str,Args...) do { TDEBUGP(x,Str,Args); } while (0)
#define DEBUGP(x,Str) do { TDEBUGP(x,Str "%s",""); } while (0)
#define DEBUGP_ENTER do { DEBUGP(D_ENTER,"====== ENTER ======\n"); } while (0)
#define DEBUGP_EXIT do { DEBUGP(D_EXIT,"****** EXIT  ******\n"); } while (0)

#define TIME_STAMP(What,TStamp)    {                                    \
    if(pcie6509_debug_mask & What)                                      \
        do_gettimeofday(&pcie6509_device->TStamp);                      \
}

#define PRINT_TIME(What,Str,T_Start,T_End,Bytes)                            \
    do {                                                                    \
        if(pcie6509_debug_mask & What) {                                    \
            unsigned long s, u, t;                                          \
            s = pcie6509_device->T_End.tv_sec - pcie6509_device->T_Start.tv_sec;  \
            u = pcie6509_device->T_End.tv_usec - pcie6509_device->T_Start.tv_usec;\
            t = s*1000000 + u;                                              \
            if(Bytes > 0)                                                   \
                DEBUGPx(What,Str "Time = %05lu.%03lu ms, %2lu.%04lu MB/s\n",\
                t/1000,t%1000,Bytes/t,((Bytes%t)*10000)/t);                 \
            else                                                            \
                DEBUGPx(What,Str "Time = %05lu.%03lu ms\n",                 \
                t/1000,t%1000);                                             \
        }                                                                   \
    } while (0)                                                             \

module_param(pcie6509_debug_mask, uint, 0);
MODULE_PARM_DESC(pcie6509_debug_mask, "PCIE6509 Debug Mask");

module_param(pcie6509_debug_ctrl, uint, 0);
MODULE_PARM_DESC(pcie6509_debug_mask, "PCIE6509 Debug Ctrl");

#else                           /* else PCIE6509_DEBUG */

#define DEBUGPx(x,Str,Args...)
#define DEBUGP(x,Str)
#define DEBUGP_ENTER
#define DEBUGP_EXIT
#define TIME_STAMP(What,TStamp)
#define PRINT_TIME(What,Str,T_Start,T_End,Bytes)

#endif                          /* end PCIE6509_DEBUG */

/**********************************************************
error ROUTINES
***********************************************************/
#define ErrHdr    "[%-7s %4d]:%2d.%s(): ERROR!!!: "

#define TERRP(Str,Args...)                                          \
     do {                                                           \
        if(pcie6509_device)                                         \
            printk( KERN_INFO ErrHdr Str,                           \
                PCIE6509_FileName,__LINE__,pcie6509_device->minor,  \
                __FUNCTION__,Args);                                 \
        else                                                        \
            printk( KERN_INFO ErrHdr Str,                           \
                PCIE6509_FileName,__LINE__,-1,                      \
                __FUNCTION__,Args);                                 \
} while (0)                         

#define ERRPx(Str,Args...) do { TERRP(Str,Args); } while (0)
#define ERRP(Str) do { TERRP(Str "%s",""); } while (0)

#endif    /* __PCIE6509_DEBUG_H_INCLUDED__ */

