// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_common.h $
// $Rev: 50965 $
// $Date: 2022-04-25 08:35:42 -0500 (Mon, 25 Apr 2022) $

// Device Driver: Linux: header file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#ifndef	__OS_COMMON_H__
#define	__OS_COMMON_H__



// macros *********************************************************************

#define	OS_IOCTL(i)							_IO  (GSC_IOCTL,(i))
#define	OS_IOCTL_R(i,s,t)					_IOR (GSC_IOCTL,(i),t)
#define	OS_IOCTL_RW(i,s,t)					_IOWR(GSC_IOCTL,(i),t)
#define	OS_IOCTL_W(i,s,t)					_IOW (GSC_IOCTL,(i),t)

#define	OS_IOCTL_DIR_DECODE(c)				_IOC_DIR((c))
#define	OS_IOCTL_DIR_READ					_IOC_READ
#define	OS_IOCTL_DIR_WRITE					_IOC_WRITE

#define	OS_IOCTL_TYPE_DECODE(c)				_IOC_TYPE((c))
#define	OS_IOCTL_INDEX_DECODE(c)			_IOC_NR((c))
#define	OS_IOCTL_SIZE_DECODE(c)				_IOC_SIZE((c))

#define	GSC_EXPORT
#define	GSC_IOCTL							's'

#ifdef __CYGWIN__
// The following is only to allow application code to compile under Cygwin.
// This does nothing to allow linking or building of libraries.

#undef	OS_IOCTL_DIR_READ
#define	OS_IOCTL_DIR_READ			GSC_FIELD_ENCODE(1,26,25)	// app's view
#undef	OS_IOCTL_DIR_WRITE
#define	OS_IOCTL_DIR_WRITE			GSC_FIELD_ENCODE(2,26,25)	// app's view
#undef	OS_IOCTL_DIR_DECODE
#define	OS_IOCTL_DIR_DECODE(c)		((c) & (OS_IOCTL_DIR_READ | OS_IOCTL_DIR_WRITE))

#undef	OS_IOCTL_TYPE_ENCODE
#define	OS_IOCTL_TYPE_ENCODE(v)		GSC_FIELD_ENCODE((v), 7, 0)
#undef	OS_IOCTL_TYPE_DECODE
#define	OS_IOCTL_TYPE_DECODE(c)		GSC_FIELD_DECODE((c), 7, 0)

#undef	OS_IOCTL_INDEX_ENCODE
#define	OS_IOCTL_INDEX_ENCODE(v)	GSC_FIELD_ENCODE((v),15, 8)
#undef	OS_IOCTL_INDEX_DECODE
#define	OS_IOCTL_INDEX_DECODE(c)	GSC_FIELD_DECODE((c),15, 8)

#undef	OS_IOCTL_SIZE_ENCODE
#define	OS_IOCTL_SIZE_ENCODE(v)		GSC_FIELD_ENCODE((v),24,16)
#undef	OS_IOCTL_SIZE_DECODE
#define	OS_IOCTL_SIZE_DECODE(c)		GSC_FIELD_DECODE((c),24,16)

#undef	OS_IOCTL
#define	OS_IOCTL(i)					( OS_IOCTL_TYPE_ENCODE(GSC_IOCTL)	\
									| OS_IOCTL_INDEX_ENCODE((i)))

#undef	OS_IOCTL_R
#define	OS_IOCTL_R(i,s,t)			( OS_IOCTL_DIR_READ					\
									| OS_IOCTL_TYPE_ENCODE(GSC_IOCTL)	\
									| OS_IOCTL_INDEX_ENCODE((i))		\
									| OS_IOCTL_SIZE_ENCODE(s))

#undef	OS_IOCTL_RW
#define	OS_IOCTL_RW(i,s,t)			( OS_IOCTL_DIR_READ					\
									| OS_IOCTL_DIR_WRITE				\
									| OS_IOCTL_TYPE_ENCODE(GSC_IOCTL)	\
									| OS_IOCTL_INDEX_ENCODE((i))		\
									| OS_IOCTL_SIZE_ENCODE(s))

#undef	OS_IOCTL_W
#define	OS_IOCTL_W(i,s,t)			( OS_IOCTL_DIR_WRITE				\
									| OS_IOCTL_TYPE_ENCODE(GSC_IOCTL)	\
									| OS_IOCTL_INDEX_ENCODE((i))		\
									| OS_IOCTL_SIZE_ENCODE(s))

#endif



// data types *****************************************************************

#ifdef __KERNEL__

#include <asm/types.h>
#include <linux/ioctl.h>
#include <linux/types.h>

#else

#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/ioctl.h>
#include <sys/timeb.h>

typedef __s8	s8;
typedef __u8	u8;

typedef __s16	s16;
typedef __u16	u16;

typedef __s32	s32;
typedef __u32	u32;

typedef __s64	s64;
typedef __u64	u64;

#endif



#endif
