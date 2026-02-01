#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/pps_kernel.h>

#include "tsync_pps.h"
#include "tsync_hw_func.h"

struct tsync_pps_device_data {
	struct pps_device *pps;
	struct pps_source_info info;
};

static struct tsync_pps_device_data tsync_pps[TSYNC_PCI_MAX_NUM];

int tsync_pps_register(tsyncpci_dev_t *tsync_priv)
{
	u32 idx = tsync_priv->slotPosition;
	int pps_default_params;

	snprintf(tsync_pps[idx].info.name, PPS_MAX_NAME_LEN - 1,
		"tsyncpci%d", idx);
	snprintf(tsync_pps[idx].info.path, PPS_MAX_NAME_LEN - 1,
		"/dev/pps%d", idx);

	tsync_pps[idx].info.mode  = PPS_CAPTUREASSERT  | PPS_OFFSETASSERT |
				    PPS_ECHOASSERT     | PPS_CANWAIT      |
				    PPS_TSFMT_TSPEC;

	tsync_pps[idx].info.owner = THIS_MODULE;

	pps_default_params = PPS_CAPTUREASSERT | PPS_OFFSETASSERT;
	tsync_pps[idx].pps = pps_register_source(&tsync_pps[idx].info,
						  pps_default_params);

	if (tsync_pps[idx].pps == NULL) {
		pr_err("failed register PPS source\n");
		return -ENOMEM;
	}

	dev_info(tsync_pps[idx].pps->dev, "tsync PPS source registered\n");

	return set_hw_int_mask_by_index(tsync_priv, 0, 0);
}

void tsync_pps_unregister(tsyncpci_dev_t *tsync_priv)
{
	u32 idx = tsync_priv->slotPosition;

	dev_info(tsync_pps[idx].pps->dev, "tsync pps source unregistered\n");
	pps_unregister_source(tsync_pps[idx].pps);
}

void tsync_pps_isr_handler(int idx)
{
	struct pps_event_time ts;

	pps_get_ts(&ts);
	pps_event(tsync_pps[idx].pps, &ts, PPS_CAPTUREASSERT, NULL);
}
