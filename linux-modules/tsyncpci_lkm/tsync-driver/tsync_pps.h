#ifndef TSYNC_PPS_H
#define TSYNC_PPS_H

#include "tsyncpci.h"

int  tsync_pps_register(tsyncpci_dev_t *tsync_priv);
void tsync_pps_unregister(tsyncpci_dev_t *tsync_priv);
void tsync_pps_isr_handler(int idx);

#endif //TSYNC_PPS_H
