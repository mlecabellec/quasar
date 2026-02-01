#ifndef __TSYNC_DRIVER_HELPERS__
#define __TSYNC_DRIVER_HELPERS__ 1

#include <asm/byteorder.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 8)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

#include "ddtpro.h"
#include "tsyncpci.h"



#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
typedef unsigned long       uintptr_t;
#endif

#ifdef NIOS
    #define BIG_END_16(value)         value
    #define BIG_END_16_IID(value)     be16_to_cpu(value)
    #define BIG_END_32(value)         value
    #define BIG_END_64(value)         value
#else
    #define BIG_END_16(value)         be16_to_cpu(value)
    #define BIG_END_16_IID(value)     be16_to_cpu(value)
    #define BIG_END_32(value)         be32_to_cpu(value)
    #define BIG_END_64(value)         be64_to_cpu(value)
#endif

typedef tsyncpci_dev_t TPRO_INSTANCE_T;

#define TSYNC_READ_REG(offset) (ioread16((void*)hw->ioAddr + (offset)))
#define TSYNC_READ_REG32(offset) (ioread32((void*)hw->ioAddr + (offset)))
#define TSYNC_WRITE_REG(value, offset) (iowrite16((value), (void*)hw->ioAddr + (offset)))

#define TSYNC_COPY_FROM_IO(destination, offset, words)                      \
{                                                                           \
    uint32_t iii;                                                           \
    uint16_t ooo = offset;                                                  \
                                                                            \
    for (iii = 0; iii < words; iii++)                                       \
    {                                                                       \
        ((uint16_t*)destination)[iii] = TSYNC_READ_REG(ooo);                \
        ooo += sizeof(uint16_t);                                            \
    }                                                                       \
}


#define TSYNC_COPY_FROM_IO32(destination, offset, lwords)                   \
{                                                                           \
    uint32_t iii;                                                           \
    uint16_t ooo = offset;                                                  \
                                                                            \
    for (iii = 0; iii < lwords; iii++)                                      \
    {                                                                       \
        ((uint32_t*)destination)[iii] = TSYNC_READ_REG32(ooo);              \
        ooo += sizeof(uint32_t);                                            \
    }                                                                       \
}

/*
#define TSYNC_COPY_FROM_IO32(destination, offset, words) memcpy_fromio((destination), (void*)hw->ioAddr + (offset), (words) * sizeof(uint16_t));                     \
*/

#define BUSY_WAIT(microseconds) usleep_range(microseconds, microseconds)

#define SRC_LOCATION_FORMAT_STRING "(%s:%u)\n"
#define SRC_LOCATION_VARIABLES __FILE__, __LINE__
#endif
