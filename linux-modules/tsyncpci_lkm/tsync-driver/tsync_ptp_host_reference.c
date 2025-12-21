/*
 * Spectracom Host Reference clock support
 *
 * Copyright (c) 2018 Orolia Group and/or its affiliates
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "tsync_ptp.h"
#include "kernel_compat.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)

#include <linux/ptp_clock_kernel.h>
#include <linux/err.h>
#include <linux/io.h>
#include <asm/div64.h>


#ifdef NIOS
#define HOST_REFERENCE_REGISTER_OFFSET 0x1600
#else
#define HOST_REFERENCE_REGISTER_OFFSET 0x100
#endif
#define HOST_REFERENCE_ID 0x00
	// 15:8 - 0x2D
	// 7:0 - PPS Mux ID
#define HOST_REFERENCE_SECONDS_LSB 0x02
#define HOST_REFERENCE_SECONDS_MIDDLE 0x04
#define HOST_REFERENCE_SECONDS_MSB 0x06
#define HOST_REFERENCE_NANOSECONDS_LSB 0x08
#define HOST_REFERENCE_NANOSECONDS_MSB 0x0A
#define HOST_REFERENCE_FRACTIONAL_PPM 0x0C
	// 15:8 - fractional PPM
	// 7:0 - ignored
#define HOST_REFERENCE_WHOLE_PPM 0x0E
#define HOST_REFERENCE_STATUS 0x10
	// bit 0 - validity

#define CLOCK_NAME "tsync host ref"

struct host_reference {
	struct ptp_clock *ptp_clock;
	struct ptp_clock_info capabilities;
	void __iomem *registers;
};

static const u32 nano_per_second = 1000000000LU;
static struct host_reference host_reference;

DEFINE_SPINLOCK(host_reference_spinlock);

void host_reference_lock(void)
{
	spin_lock(&host_reference_spinlock);
}

void host_reference_unlock(void)
{
	spin_unlock(&host_reference_spinlock);
}

static u16 host_reference_read_register(u16 offset)
{
	const u16 value = ioread16(host_reference.registers + offset);
	pr_debug("%04x is %04x\n", (unsigned)offset, (unsigned)value);
	return value;
}

static void host_reference_write_register(u16 value, u16 offset)
{
	pr_debug("%04x set to %04x\n", (unsigned)offset, (unsigned)value);
	iowrite16(value, host_reference.registers + offset);
}

static ssize_t host_reference_valid_show(
	struct device *dev, struct device_attribute *attribute, char *buf)
{
	unsigned valid;
	ssize_t bytes_written;

	host_reference_lock();

	valid = host_reference_read_register(HOST_REFERENCE_STATUS);

	bytes_written = snprintf(buf, PAGE_SIZE, "%u\n", valid);

	host_reference_unlock();

	return bytes_written;
}

static ssize_t host_reference_valid_store(struct device *dev,
	struct device_attribute *attribute, const char *buf, size_t count)
{
	bool valid;
	if (STRTOBOOL(buf, &valid) < 0)
		return -EINVAL;

	host_reference_lock();

	host_reference_write_register(valid, HOST_REFERENCE_STATUS);

	host_reference_unlock();

	return count;
}

static DEVICE_ATTR(host_reference_valid, 0660,
	host_reference_valid_show,
	host_reference_valid_store);

static struct attribute *host_reference_attrs[] = {
	&dev_attr_host_reference_valid.attr,
	NULL,
};

static const struct attribute_group host_reference_attr_group = {
	.attrs = host_reference_attrs,
};

static const struct attribute_group *host_reference_attr_groups[] = {
	&host_reference_attr_group,
	NULL,
};

static void host_reference_delta_to_time(s64 *seconds, s32 *nanoseconds,
	s64 delta)
{
	u64 s;
	u32 ns;
	const int negative = (delta < 0);

	if (negative)
		s = (u64)-delta;
	else
		s = (u64)delta;

	ns = do_div(s, nano_per_second);
	if (negative) {
		*seconds = -(s64)s;
		*nanoseconds = -(s32)ns;
	} else {
		*seconds = (s64)s;
		*nanoseconds = (s32)ns;
	}
}

static void host_reference_subtract_delta_from_time(struct timespec64 *ts,
	s64 delta)
{
	s64 delta_seconds;
	s32 delta_nanoseconds;
	s64 new_nanoseconds;
	s64 new_seconds;

	host_reference_delta_to_time(&delta_seconds, &delta_nanoseconds,
		delta);

	new_nanoseconds = ts->tv_nsec + delta_nanoseconds;
	new_seconds = ts->tv_sec + delta_seconds;

	while (new_nanoseconds > nano_per_second) {
		new_seconds = new_seconds + 1;
		new_nanoseconds = new_nanoseconds - nano_per_second;
	}
	while (new_nanoseconds < 0) {
		new_seconds = new_seconds - 1;
		new_nanoseconds = new_nanoseconds + nano_per_second;
	}

	ts->tv_sec = (s64)new_seconds;
	ts->tv_nsec = (s32)new_nanoseconds;
}

static void host_reference_do_gettime(struct ptp_clock_info *info,
	struct timespec64 *ts)
{
	u16 seconds_lsb;
	u16 seconds_middle;
	u16 seconds_msb;
	u16 nanoseconds_lsb;
	u16 nanoseconds_msb;

	seconds_lsb = host_reference_read_register(HOST_REFERENCE_SECONDS_LSB);
	seconds_middle = host_reference_read_register(HOST_REFERENCE_SECONDS_MIDDLE);
	seconds_msb = host_reference_read_register(HOST_REFERENCE_SECONDS_MSB);
	nanoseconds_lsb = host_reference_read_register(HOST_REFERENCE_NANOSECONDS_LSB);
	nanoseconds_msb = host_reference_read_register(HOST_REFERENCE_NANOSECONDS_MSB);

	ts->tv_sec = (s64)seconds_lsb +
		(((s64)seconds_middle) << 16) +
		(((s64)seconds_msb) << 32);
	ts->tv_nsec = nanoseconds_lsb +
		(((s32)nanoseconds_msb) << 16);

	pr_debug("host_reference_do_gettime: %lld.%09u\n", ts->tv_sec, (unsigned)ts->tv_nsec);
}

static void host_reference_do_settime(struct ptp_clock_info *info,
	const struct timespec64 *ts)
{
	const u16 seconds_lsb = (u16)(ts->tv_sec & 0xFFFF);
	const u16 seconds_middle = (u16)((ts->tv_sec >> 16) & 0xFFFF);
	const u16 seconds_msb = (u16)((ts->tv_sec >> 32) & 0xFFFF);
	const u16 nanoseconds_lsb = (u16)(ts->tv_nsec & 0xFFFF);
	const u16 nanoseconds_msb = (u16)((ts->tv_nsec >> 16) & 0xFFFF);

	pr_debug("host_reference_do_settime: %lld.%09u\n", ts->tv_sec, (unsigned)ts->tv_nsec);

	host_reference_write_register(seconds_lsb, HOST_REFERENCE_SECONDS_LSB);
	host_reference_write_register(seconds_middle, HOST_REFERENCE_SECONDS_MIDDLE);
	host_reference_write_register(seconds_msb, HOST_REFERENCE_SECONDS_MSB);
	host_reference_write_register(nanoseconds_lsb, HOST_REFERENCE_NANOSECONDS_LSB);
	host_reference_write_register(nanoseconds_msb, HOST_REFERENCE_NANOSECONDS_MSB);
}

static int host_reference_adjtime(struct ptp_clock_info *info, s64 delta)
{
	struct timespec64 ts;

	pr_debug("host_reference_adjtime: delta=%lld\n", delta);

	host_reference_lock();

	host_reference_do_gettime(info, &ts);
	host_reference_subtract_delta_from_time(&ts, delta);
	host_reference_do_settime(info, &ts);

	host_reference_unlock();

	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10, 0)
static int host_reference_adjfreq(struct ptp_clock_info *info, long scaled_ppm)
#else
static int host_reference_adjfreq(struct ptp_clock_info *info, s32 scaled_ppm)
#endif
{
	s16 whole_ppm;
	s16 fractional_ppm;

	pr_debug("host_reference_adjfreq: scaled_ppm=%ld\n", scaled_ppm);

	scaled_ppm = -scaled_ppm;

	whole_ppm = (s16)((scaled_ppm >> 16) & 0xFFFF);
	fractional_ppm = (s16)(scaled_ppm & 0xFFFF);

	host_reference_lock();

	host_reference_write_register(fractional_ppm, HOST_REFERENCE_FRACTIONAL_PPM);
	host_reference_write_register(whole_ppm, HOST_REFERENCE_WHOLE_PPM);

	host_reference_unlock();

	return 0;
}

static int host_reference_settime(struct ptp_clock_info *info,
	const struct timespec64 *ts)
{
	host_reference_lock();
	host_reference_do_settime(info, ts);
	host_reference_unlock();

	return 0;
}

static int host_reference_gettime(struct ptp_clock_info *info,
	struct timespec64 *ts)
{
	host_reference_lock();
	host_reference_do_gettime(info, ts);
	host_reference_unlock();

	return 0;
}

static int host_reference_add_groups(struct device *dev)
{
	return sysfs_create_groups(&dev->kobj, host_reference_attr_groups);
}

static int host_reference_remove_groups(struct device *dev)
{
	sysfs_remove_groups(&dev->kobj, host_reference_attr_groups);
	return 0;
}

static const struct ptp_clock_info host_reference_capabilities = {
	.owner		= THIS_MODULE,
	.name		= CLOCK_NAME,
	.max_adj	= 1000000000,
	.n_ext_ts	= 0,
	.n_pins		= 0,
	.pps		= 0,
	.adjtime	= host_reference_adjtime,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10, 0)
	.adjfine	= host_reference_adjfreq,
#else
	.adjfreq	= host_reference_adjfreq,
#endif
	.gettime64	= host_reference_gettime,
	.settime64	= host_reference_settime,
};

int tsync_ptp_host_reference_register(tsyncpci_dev_t *tsync_priv,
	struct device *dev)
{
	int err;

	host_reference.capabilities = host_reference_capabilities;
	host_reference.ptp_clock =
		ptp_clock_register(&host_reference.capabilities, NULL);
	host_reference.registers =
		(void __iomem *)tsync_priv->ioAddr + HOST_REFERENCE_REGISTER_OFFSET;

	if (IS_ERR(host_reference.ptp_clock)) {
		host_reference.ptp_clock = NULL;
		pr_err("ptp_clock_register() failed\n");
		return PTR_ERR(host_reference.ptp_clock);
	}

	err = host_reference_add_groups(dev);
	if (err) {
		ptp_clock_unregister(host_reference.ptp_clock);
		host_reference.ptp_clock = NULL;
		pr_err("adding sysfs attributes groups failed\n");
		return err;
	}

	return 0;
}

void tsync_ptp_host_reference_unregister(struct device *dev)
{
	(void)host_reference_remove_groups(dev);

	if (host_reference.ptp_clock) {
		ptp_clock_unregister(host_reference.ptp_clock);
		host_reference.ptp_clock = NULL;
		pr_debug("Removed Host Reference PTP clock successfully\n");
	}
}

#else // KERNEL < 4.1.0

int tsync_ptp_host_reference_register(tsyncpci_dev_t *t, struct device *d) { return 0; }
void tsync_ptp_host_reference_unregister(struct device *d) {}

#endif // KERNEL CONDITION
