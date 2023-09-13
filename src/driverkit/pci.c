// SPDX-License-Identifier: LGPL-2.1-or-later or MIT

/*
 * This file is part of libvfn.
 *
 * Copyright (C) 2022 The libvfn Authors. All Rights Reserved.
 *
 * This library (libvfn) is dual licensed under the GNU Lesser General
 * Public License version 2.1 or later or the MIT license. See the
 * COPYING and LICENSE files for more information.
 */


#ifdef __cplusplus
extern "C" {
#endif

#define log_fmt(fmt) "driverkit/pci: " fmt

#include <vfn/support/platform/macos/byteswap.h>
#include "vfn/support/atomic.h"
#include "vfn/support/compiler.h"
#include "vfn/support/log.h"
#include "vfn/support/mem.h"
#include "vfn/support/endian.h"
#include "vfn/driverkit/pci.h"
#include "vfn/support/mmio.h"

static int pci_set_bus_master(struct driverkit_pci_device *pci)
{
	return -1;
}

static int vfio_pci_init_bar(struct driverkit_pci_device *pci, int idx)
{
	return -1;
}

static int vfio_pci_init_irq(struct driverkit_pci_device *pci)
{
	return -1;
}

void *vfio_pci_map_bar(struct driverkit_pci_device *pci, int idx, size_t len, uint64_t offset, int prot)
{
    struct macvfn_pci_map_bar* mapping = (struct macvfn_pci_map_bar*) zmallocn(1, sizeof(struct macvfn_pci_map_bar));
    // mapping->parent_ctrl = ctrl;
    mapping->pci = pci;
    mapping->idx = idx;
    mapping->len = len;
    mapping->offset = offset;
    // mapping->prot = prot;

    return mapping;
}

void vfio_pci_unmap_bar(struct driverkit_pci_device *pci, int idx, void *mem, size_t len,
			uint64_t offset)
{
    return;
}

int vfio_pci_open(struct driverkit_pci_device *pci, const char *bdf)
{
	return 0;
}

#ifdef __cplusplus
}
#endif