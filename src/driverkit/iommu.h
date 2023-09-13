/* SPDX-License-Identifier: LGPL-2.1-or-later or MIT */

/*
 * This file is part of libvfn.
 *
 * Copyright (C) 2022 The libvfn Authors. All Rights Reserved.
 *
 * This library (libvfn) is dual licensed under the GNU Lesser General
 * Public License version 2.1 or later or the MIT license. See the
 * COPYING and LICENSE files for more information.
 */

#include <assert.h>
#include <vfn/support/log.h>
#include <DriverKit/OSDictionary.h>
#include <DriverKit/OSArray.h>
#include <DriverKit/OSNumber.h>

static inline OSArray *iommu_find_mapping(OSDictionary* iommu_mappings, IOBufferMemoryDescriptor *vaddr)
{
	log_debug("iommu_find_mapping: %llx", (uint64_t)vaddr);
	return (OSArray *) iommu_mappings->getObject(vaddr);
}

static inline int iommu_add_mapping(OSDictionary* iommu_mappings, IOBufferMemoryDescriptor *vaddr, size_t len, uint64_t* iova, IODMACommand* dmaCommand)
{
	OSArray* dma_iova = OSArray::withCapacity(2);
	OSNumber* _iova = OSNumber::withNumber(*iova, 64);
	log_debug("iommu_add_mapping: %llx, %llx, %llx, %llx", (uint64_t)vaddr, (uint64_t)iova, (uint64_t)dmaCommand, (uint64_t) _iova);
	dma_iova->setObject(0, dmaCommand);
	dma_iova->setObject(1, _iova);
	return !iommu_mappings->setObject(vaddr, dma_iova);
}

static inline void iommu_remove_mapping(OSDictionary* iommu_mappings, IOBufferMemoryDescriptor *vaddr)
{
	OSArray* dma_iova = (OSArray *) iommu_mappings->getObject(vaddr);
	if (!dma_iova){
		log_error("iommu_remove_mapping: Unable to find %llx", (uint64_t) vaddr);
		return;
	}

	OSNumber* _iova= (OSNumber*) dma_iova->getObject(1);
	log_debug("iommu_remove_mapping: %llx, %llx", (uint64_t)vaddr, (uint64_t) _iova);
	_iova->release();

	iommu_mappings->removeObject(vaddr);
	dma_iova->release();
}

static inline bool iommu_vaddr_to_iova(OSDictionary* iommu_mappings, IOBufferMemoryDescriptor *vaddr, uint64_t *iova)
{
	OSArray* dma_iova = iommu_find_mapping(iommu_mappings, vaddr);
	if (!dma_iova){
		return false;
	}

	OSNumber* _iova= (OSNumber*) dma_iova->getObject(1);
	*iova = _iova->unsigned64BitValue();
	return true;
}
