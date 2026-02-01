#include <linux/device.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/bcd.h>
#include "tsync_ptp.h"

#ifndef NIOS
	#define TSYNC_SYSTIME_NS_LO  0x008
	#define TSYNC_SYSTIME_NS_HI  0x00a
	#define TSYNC_SYSTIME_SEC_LO 0x00c
	#define TSYNC_SYSTIME_SEC_HI 0x00e
#else
	#define TSYNC_SYSTIME_NS_LO  0x428
	#define TSYNC_SYSTIME_NS_HI  0x42a
	#define TSYNC_SYSTIME_SEC_LO 0x42c
	#define TSYNC_SYSTIME_SEC_HI 0x42e
#endif

struct tsync_clock {
	void __iomem *regs;
	struct ptp_clock *ptp_clock;
	struct ptp_clock_info caps;
};

DEFINE_SPINLOCK(register_lock);

static struct tsync_clock tsync_clock;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
static int ptp_tsync_gettime(struct ptp_clock_info *ptp, struct timespec64 *ts)
#else
static int ptp_tsync_gettime(struct ptp_clock_info *ptp, struct timespec *ts)
#endif
{
	struct tsync_clock *phc = container_of(ptp, struct tsync_clock, caps);
	u16 ns_lo, ns_hi, sec_lo, sec_hi;
	u32 sec, ns;

	spin_lock(&register_lock);

	sec_lo = ioread16(phc->regs + TSYNC_SYSTIME_SEC_LO);
	sec_hi = ioread16(phc->regs + TSYNC_SYSTIME_SEC_HI);
	ns_lo  = ioread16(phc->regs + TSYNC_SYSTIME_NS_LO);
	ns_hi  = ioread16(phc->regs + TSYNC_SYSTIME_NS_HI);

	spin_unlock(&register_lock);

	sec = (sec_hi << 16) + sec_lo;
	ns = (((ns_hi & 0x0fff) << 16) + ns_lo) * 5;
	ts->tv_sec = sec;
	ts->tv_nsec = ns;

	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)
static int ptp_tsync_adjfine(struct ptp_clock_info *ptp, long ppb)
{
	return -EOPNOTSUPP;
}
#else
static int ptp_tsync_adjfreq(struct ptp_clock_info *ptp, s32 ppb)
{
	return -EOPNOTSUPP;
}
#endif

static int ptp_tsync_adjtime(struct ptp_clock_info *ptp, s64 delta)
{
	return -EOPNOTSUPP;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
static int ptp_tsync_settime(struct ptp_clock_info *ptp,
			     const struct timespec64 *ts)
#else
static int ptp_tsync_settime(struct ptp_clock_info *ptp,
			     const struct timespec *ts)
#endif
{
	return -EOPNOTSUPP;
}

static struct ptp_clock_info ptp_tsync_caps = {
	.owner	    = THIS_MODULE,
	.name	    = "tsync ptp clock",
	.max_adj    = 50000000,
	.n_ext_ts   = 0,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0)
	.n_pins	    = 0,
#endif
	.pps	    = 0,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)
	.adjfine    = ptp_tsync_adjfine,
#else
	.adjfreq    = ptp_tsync_adjfreq,
#endif
	.adjtime    = ptp_tsync_adjtime,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
	.gettime64  = ptp_tsync_gettime,
	.settime64  = ptp_tsync_settime,
#else
	.gettime    = ptp_tsync_gettime,
	.settime    = ptp_tsync_settime,
#endif
	.enable     = 0,
};

int tsync_ptp_register(tsyncpci_dev_t *tsync_priv)
{
	tsync_clock.caps = ptp_tsync_caps;
	tsync_clock.ptp_clock = ptp_clock_register(&tsync_clock.caps, NULL);
	tsync_clock.regs = (void __iomem *)tsync_priv->ioAddr;

	if (IS_ERR(tsync_clock.ptp_clock)) {
		tsync_clock.ptp_clock = NULL;
		pr_debug("ptp_clock_register() failed\n");
		return PTR_ERR(tsync_clock.ptp_clock);
	}

	return 0;
}

void tsync_ptp_unregister(void)
{
	if (tsync_clock.ptp_clock) {
		ptp_clock_unregister(tsync_clock.ptp_clock);
		tsync_clock.ptp_clock = NULL;
		pr_debug("Removed PTP tsync clock successfully\n");
	}
}

