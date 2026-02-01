#ifndef _KERNEL_COMPAT_H_
#define _KERNEL_COMPAT_H_

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#define HAVE_SIGNAL_FUNCTIONS_OWN_HEADER
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
#define wait_queue_t wait_queue_entry_t
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
#define TSYNC_CLASS_CREATE(x, y) class_create(y)
#else
#define TSYNC_CLASS_CREATE(x, y) class_create(x, y)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 28)
#define STRTOBOOL(x, y) kstrtobool(x, y)
#else
#define STRTOBOOL(x, y) strtobool(x, y)
#endif

#endif //_KERNEL_COMPAT_H_
