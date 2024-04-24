// SPDX-License-Identifier: LGPL-2.1-or-later or MIT

/*
 * This file is part of libvfn.
 *
 * Copyright (C) 2023 The libvfn Authors. All Rights Reserved.
 *
 * This library (libvfn) is dual licensed under the GNU Lesser General
 * Public License version 2.1 or later or the MIT license. See the
 * COPYING and LICENSE files for more information.
 */

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define log_fmt(fmt) "iommu/vfio: " fmt

#include "ccan/str/str.h"
#include "ccan/compiler/compiler.h"
#include "ccan/minmax/minmax.h"

#include "vfn/driverkit.h"
#include "vfn/support.h"
#include "vfn/iommu.h"
#include "vfn/trace.h"
#include "vfn/pci/util.h"

#include "context.h"

// struct driverkit_container {
// 	struct iommu_ctx ctx;
// 	char *name;
// 	int initialized;
// };

// static struct driverkit_container driverkit_default_container = {
// 	.name = "default",
// 	.initialized = 0,
// };

static int driverkit_dma_map(struct iommu_ctx *ctx, struct iova_mapping *m)
{
	kern_return_t ret;
	IODMACommand **dmaCommand = (IODMACommand **) &m->opaque[0];
	log_debug("driverkit_dma_map (PrepareForDMA) %p %zu, %p, %p", m->vaddr, m->len, m->opaque[0], m->opaque[1]);
	IODMACommandSpecification dmaSpecification = {
		.options = kIODMACommandSpecificationNoOptions,
		.maxAddressBits = 64,
	};
	struct driverkit_pci_device *dev = container_of(ctx, struct driverkit_pci_device, ctx);

	// OSNumber vaddr_key = OSNumber::withNumber(vaddr);
	// IOMemoryDescriptor *mem = driverkit_pci_device->allocations->getObject(vaddr_key);
	// vaddr_key->release();
	// log_fatal_if(!mem, "Failed to find IOMemoryDescriptor for vaddr %p", vaddr);

	ret = IODMACommand::Create(dev->dev, kIODMACommandCreateNoOptions, &dmaSpecification, dmaCommand);
	if (ret != kIOReturnSuccess){
		log_error("IODMACommand::Create failed with error code: %x", ret);
		return -1;
	}

	if (!m->opaque[1]){
		log_error("driverkit_dma_map: opaque[1] is NULL");
		return -1;
	}

	uint64_t dmaFlags = 0;
	uint32_t dmaSegmentCount = 1;
	IOAddressSegment physicalAddressSegment;
	ret = (*dmaCommand)->PrepareForDMA(
		kIODMACommandPrepareForDMANoOptions,
		(IOMemoryDescriptor *) m->opaque[1],
		0,
		m->len,
		&dmaFlags,
		&dmaSegmentCount,
		&physicalAddressSegment
	);
	if (ret != kIOReturnSuccess){
		log_error("failed to PrepareForDMA %x", ret);
		return -1;
	}
	if (dmaSegmentCount != 1){
		log_error("dmaSegmentCount not 1! %u", dmaSegmentCount);
		return -1;
	}
	assert (m->len == physicalAddressSegment.length);
	m->iova = physicalAddressSegment.address;

	log_debug("driverkit_dma_map: iova %llx", m->iova);

	return 0;
}

static int driverkit_dma_unmap(struct iommu_ctx *ctx, struct iova_mapping *m)
{
	log_debug("driverkit_dma_unmap (CompleteDMA) %p %zu", m->vaddr, m->len);
	IODMACommand *dmaCommand = (IODMACommand *) m->opaque[0];
	kern_return_t ret = (int) dmaCommand->CompleteDMA(kIODMACommandCompleteDMANoOptions);
	if (ret != kIOReturnSuccess){
		return -1;
	}
	OSSafeReleaseNULL(dmaCommand);
	m->opaque[0] = NULL;
	return 0;
}

static int driverkit_dma_unmap_all(struct iommu_ctx *ctx)
{
	return 0;
}

static const struct iommu_ctx_ops driverkit_ops = {
	.get_device_fd = NULL,

	.iova_reserve = NULL,
	.iova_put_ephemeral = NULL,

	.dma_map = driverkit_dma_map,
	.dma_unmap = driverkit_dma_unmap,
	.dma_unmap_all = driverkit_dma_unmap_all,
};

void iommu_ctx_init(struct iommu_ctx *ctx)
{
	memcpy(&ctx->ops, &driverkit_ops, sizeof(ctx->ops));
	ctx->lock = IOLockAlloc();
	ctx->map.lock = IOLockAlloc();
	skiplist_init(&ctx->map.list);
}

struct iommu_ctx *driverkit_get_iommu_context(const char *name)
{
	return NULL;
	// if (strcmp(name, "default")){
	// 	// Only 'default' supported
	// 	return NULL;
	// }
	// return driverkit_get_default_iommu_context();
}

struct iommu_ctx *driverkit_get_default_iommu_context(void)
{
	return NULL;
	// if (!driverkit_default_container.initialized){
	// 	iommu_ctx_init(&driverkit_default_container.ctx);
	// }

	// return &driverkit_default_container.ctx;
}

#ifdef __cplusplus
}
#endif
