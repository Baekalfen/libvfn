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

#ifndef LIBVFN_SUPPORT_MMIO_H
#define LIBVFN_SUPPORT_MMIO_H

struct macvfn_pci_map_bar {
	// struct nvme_ctrl* parent_ctrl;
    struct driverkit_pci_device *pci;
	unsigned int idx;
	size_t len;
	uint64_t offset;
	// int prot;
};

/**
 * mmio_read32 - read 4 bytes in memory-mapped register
 * @addr: memory-mapped register
 *
 * Return: read value (native endian)
 */
static inline leint32_t mmio_read32(void *base_addr, uint64_t offset)
{
	/* memory-mapped register */
	struct macvfn_pci_map_bar* mapping = (struct macvfn_pci_map_bar *) base_addr;

    uint32_t val;
    // log_debug("mmio_read32: bar: %x, %llx, %llx", mapping->idx, mapping->offset, offset);
    mapping->pci->dev->MemoryRead32(mapping->pci->bar_region_info[mapping->idx].memory_index, mapping->offset + offset, &val);

    return (leint32_t __force)(val);
}

/**
 * mmio_lh_read64 - read 8 bytes in memory-mapped register
 * @addr: memory-mapped register
 *
 * Read the low 4 bytes first, then the high 4 bytes.
 *
 * Return: read value (native endian)
 */
static inline leint64_t mmio_lh_read64(void *base_addr, uint64_t offset)
{
    struct macvfn_pci_map_bar* mapping = (struct macvfn_pci_map_bar *) base_addr;

    uint64_t val;
    // log_debug("mmio_lh_read64: bar: %x, %llx, %llx", mapping->idx, mapping->offset, offset);
    mapping->pci->dev->MemoryRead64(mapping->pci->bar_region_info[mapping->idx].memory_index, mapping->offset + offset, &val);

    return (leint64_t __force)(val);
}

#define mmio_read64(base_addr, offset) mmio_lh_read64(base_addr, offset)

/**
 * mmio_write32 - write 4 bytes to memory-mapped register
 * @addr: memory-mapped register
 * @v: value to write (native endian)
 */
static inline void mmio_write32(void *base_addr, uint64_t offset, leint32_t v)
{
	/* memory-mapped register */
	struct macvfn_pci_map_bar* mapping = (struct macvfn_pci_map_bar *) base_addr;
    // log_debug("mmio_write32: bar: %x, %llx, %llx, %x", mapping->idx, mapping->offset,  offset, v);
    mapping->pci->dev->MemoryWrite32(mapping->pci->bar_region_info[mapping->idx].memory_index, mapping->offset + offset, v);
}

/**
 * mmio_lh_write64 - write 8 bytes to memory-mapped register
 * @addr: memory-mapped register
 * @v: value to write (native endian)
 *
 * Write 8 bytes to memory-mapped register as two 4 byte writes (low bytes
 * first, then high).
 */
static inline void mmio_lh_write64(void *base_addr, uint64_t offset, leint64_t v)
{
	mmio_write32(base_addr, offset, (leint32_t __force)v);
	mmio_write32((uint8_t *)base_addr, offset + 4, (leint32_t __force)(v >> 32));
}

/**
 * mmio_hl_write64 - write 8 bytes to memory-mapped register
 * @addr: memory-mapped register
 * @v: value to write (native endian)
 *
 * Write 8 bytes to memory-mapped register as two 4 byte writes (high bytes
 * first, then low).
 */
static inline void mmio_hl_write64(void *base_addr, uint64_t offset, leint64_t v)
{
	mmio_write32((uint8_t *)base_addr, offset + 4, (leint32_t __force)((uint64_t __force)v >> 32));
	mmio_write32(base_addr, offset, (leint32_t __force)v);
}

#endif /* LIBVFN_SUPPORT_MMIO_H */
