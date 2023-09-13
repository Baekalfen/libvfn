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

struct driverkit_container {
	struct iommu_ctx ctx;
	char *name;
	int initialized;
};

static struct driverkit_container driverkit_default_container = {
	.name = "default",
	.initialized = 0,
};

static int driverkit_dma_map(struct iommu_ctx *ctx, void *vaddr, size_t len,
				       uint64_t *iova, unsigned long flags, void **opaque)
{
	IODMACommand **dmaCommand = (IODMACommand **) opaque;
	log_debug("driverkit_dma_map (PrepareForDMA) %p %zu", vaddr, len);
	IODMACommandSpecification dmaSpecification;
	struct driverkit_pci_device *dev = container_of(ctx, struct driverkit_pci_device, ctx);

	bzero(&dmaSpecification, sizeof(dmaSpecification));

	dmaSpecification.options = kIODMACommandSpecificationNoOptions;
	dmaSpecification.maxAddressBits = 64;

	IODMACommand::Create(dev->dev, kIODMACommandCreateNoOptions, &dmaSpecification, dmaCommand);

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

static int driverkit_dma_unmap(struct iommu_ctx *ctx, struct iova_mapping *m)
{
	uint64_t iova = m->iova;
	size_t len = m->len;
	log_debug("driverkit_dma_unmap (CompleteDMA) %llu %zu", iova, len);
	IODMACommand *dmaCommand = (IODMACommand *) m->opaque;
	int ret = (int) dmaCommand->CompleteDMA(kIODMACommandCompleteDMANoOptions);
	OSSafeReleaseNULL(dmaCommand);
	return ret;
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
	if (strcmp(name, "default")){
		// Only 'default' supported
		return NULL;
	}
	return driverkit_get_default_iommu_context();
}

struct iommu_ctx *driverkit_get_default_iommu_context(void)
{
	if (!driverkit_default_container.initialized){
		iommu_ctx_init(&driverkit_default_container.ctx);
	}

	return &driverkit_default_container.ctx;
}

#ifdef __cplusplus
}
#endif
