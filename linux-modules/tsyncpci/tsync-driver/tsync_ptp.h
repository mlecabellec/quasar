#ifndef __TSYNC_PTP_H__
#define __TSYNC_PTP_H__

#include "tsyncpci.h"

int tsync_ptp_register(tsyncpci_dev_t *tsync_priv);
void tsync_ptp_unregister(void);

#endif //__TSYNC_PTP_H__
