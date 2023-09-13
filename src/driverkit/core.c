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

#define log_fmt(fmt) "driverkit/core: " fmt

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "vfn/support/align.h"
#include "vfn/support/atomic.h"
#include "vfn/support/compiler.h"
#include "vfn/support/log.h"
#include "vfn/support/mem.h"

#include "vfn/driverkit/container.h"
#include "vfn/driverkit/pci.h"

#include "vfn/trace.h"

#include "ccan/compiler/compiler.h"
#include "ccan/str/str.h"

#include <DriverKit/IODMACommand.h>
#include <DriverKit/IOBufferMemoryDescriptor.h>

#include "iommu.h"


// TODO: Move 'vfio_do_map_dma' to iommu.c and call it 'iommu_do_map_dma'. Doesn't belong here.
// static int vfio_do_map_dma(IOPCIDevice* dev, IOBufferMemoryDescriptor *vaddr, size_t len, uint64_t* iova, IODMACommand **dmaCommand)
static int vfio_do_map_dma(IOPCIDevice* dev, void *vaddr, size_t len, uint64_t* iova, IODMACommand **dmaCommand)
{
	log_debug("vfio_do_map_dma (PrepareForDMA) %p %zu", vaddr, len);
	IODMACommandSpecification dmaSpecification;

	bzero(&dmaSpecification, sizeof(dmaSpecification));

	dmaSpecification.options = kIODMACommandSpecificationNoOptions;
	dmaSpecification.maxAddressBits = 64;

	IODMACommand::Create(dev, kIODMACommandCreateNoOptions, &dmaSpecification, dmaCommand);

	uint64_t dmaFlags = 0;
	uint32_t dmaSegmentCount = 1;
	IOAddressSegment physicalAddressSegment;
	kern_return_t ret = (*dmaCommand)->PrepareForDMA(
		kIODMACommandPrepareForDMANoOptions,
		(IOBufferMemoryDescriptor *)vaddr,
		0,
		len,
		&dmaFlags,
		&dmaSegmentCount,
		&physicalAddressSegment
	);
	if (ret != kIOReturnSuccess){
		log_debug("failed to PrepareForDMA %x", ret);
		return -1;
	}
	if (dmaSegmentCount != 1){
		log_error("dmaSegmentCount not 1! %u", dmaSegmentCount);
		return -1;
	}
	assert (len == physicalAddressSegment.length);
	*iova = physicalAddressSegment.address;

	return 0;
}

static int vfio_do_unmap_dma(IODMACommand *dmaCommand)
{
	int ret = (int) dmaCommand->CompleteDMA(kIODMACommandCompleteDMANoOptions);
	OSSafeReleaseNULL(dmaCommand);
	return ret;
}

static int vfio_unmap_all(struct driverkit_pci_device* pci)
{
	return -1;
}

// Why this relation between vfio_.. and iommu?
static int vfio_iommu_uninit(struct driverkit_pci_device* pci)
{
	return -1;
}

// int vfio_map_vaddr(struct driverkit_pci_device* pci, IOBufferMemoryDescriptor *vaddr, size_t len, uint64_t *iova)
int vfio_map_vaddr(struct driverkit_pci_device* pci, void *vaddr, size_t len, uint64_t *iova)
{
	uint64_t _iova;
	IODMACommand *dmaCommand;
	if (iommu_vaddr_to_iova(pci->iommu_mappings, (IOBufferMemoryDescriptor*) vaddr, &_iova)){
		goto out;
	}

	if (vfio_do_map_dma(pci->dev, (IOBufferMemoryDescriptor *)vaddr, len, &_iova, &dmaCommand)) {
		log_debug("failed to map dma\n");
		return -1;
	}

	if (iommu_add_mapping(pci->iommu_mappings, (IOBufferMemoryDescriptor *)vaddr, len, &_iova, dmaCommand)) {
		log_debug("failed to add mapping\n");
		return -1;
	}

out:
	if (iova)
		*iova = _iova;

	return 0;
}

// int vfio_unmap_vaddr(struct driverkit_pci_device* pci, IOBufferMemoryDescriptor *vaddr, size_t *len)
int vfio_unmap_vaddr(struct driverkit_pci_device* pci, void *vaddr, size_t *len)
{
	OSArray* dma_iova = (OSArray*) iommu_find_mapping(pci->iommu_mappings, (IOBufferMemoryDescriptor *)vaddr);
	if (!dma_iova) {
		log_debug("failed to find mapping!");
		return 0;
	}
	IODMACommand* dmaCommand = (IODMACommand*)dma_iova->getObject(0);

	if (len){
		log_error("Getting length is unsupported");
		*len = 0;
	}

	if (vfio_do_unmap_dma(dmaCommand)) {
		log_debug("failed to unmap dma\n");
		return -1;
	}

	iommu_remove_mapping(pci->iommu_mappings, (IOBufferMemoryDescriptor *)vaddr);
	return 0;
}

// int vfio_map_vaddr_ephemeral(struct driverkit_pci_device* pci, IOBufferMemoryDescriptor *vaddr, size_t len, uint64_t *iova)
int vfio_map_vaddr_ephemeral(struct driverkit_pci_device* pci, void *vaddr, size_t len, uint64_t *iova)
{
	return vfio_map_vaddr(pci, vaddr, len, iova);
}

int vfio_unmap_ephemeral_iova(struct driverkit_pci_device* pci, size_t len, uint64_t iova)
{
	// TODO: Invert iommu_find_mapping i.e. iova->buf, not buf->iova
	return 0;
}

#ifdef __cplusplus
}
#endif