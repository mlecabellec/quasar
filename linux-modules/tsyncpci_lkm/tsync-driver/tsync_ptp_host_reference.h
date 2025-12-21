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
#ifndef __TSYNC_PTP_HOST_REFERENCE_H__
#define __TSYNC_PTP_HOST_REFERENCE_H__

#include "tsyncpci.h"

int tsync_ptp_host_reference_register(tsyncpci_dev_t *tsync_priv,
	struct device *dev);
void tsync_ptp_host_reference_unregister(struct device *dev);

#endif
