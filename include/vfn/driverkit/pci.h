/* SPDX-License-Identifier: LGPL-2.1-or-later or MIT */

#ifndef LIBVFN_VFIO_PCI_H
#define LIBVFN_VFIO_PCI_H

#include <DriverKit/OSCollection.h>
#include <PCIDriverKit/PCIDriverKit.h>

#define off_t int

// NOTE: Dummy PROT to satisfy Linux code
enum {
    PROT_READ=0,
    PROT_WRITE=1,
};

struct driverkit_bar_info {
    uint8_t type;
    uint8_t memory_index;
    uint64_t size;
};

struct driverkit_pci_device {
	struct IOPCIDevice* dev;
	OSDictionary* iommu_mappings;
	const char *bdf;

	// struct vfio_region_info config_region_info;
	struct driverkit_bar_info bar_region_info[6];
};

/**
 * vfio_pci_open - open and initialize pci device
 * @pci: &struct driverkit_pci_device to initialize
 * @bdf: pci device identifier ("bus:device:function") to open
 *
 * Open the pci device identified by @bdf and initialize @pci.
 *
 * Return: On success, returns ``0``. On error, returns ``-1`` and sets
 * ``errno``.
 */
int vfio_pci_open(struct driverkit_pci_device *pci, const char *bdf);

/**
 * vfio_pci_map_bar - map a vfio device region into virtual memory
 * @pci: &struct driverkit_pci_device
 * @idx: the vfio region index to map
 * @len: number of bytes to map
 * @offset: offset at which to start mapping
 * @prot: what accesses to permit to the mapped area (see ``man mmap``).
 *
 * Map the vfio device memory region identified by @idx into virtual memory.
 *
 * Return: On success, returns the virtual memory address mapped. On error,
 * returns ``NULL`` and sets ``errno``.
 */
void *vfio_pci_map_bar(struct driverkit_pci_device *pci, int idx, size_t len, uint64_t offset,
		       int prot);

/**
 * vfio_pci_unmap_bar - unmap a vfio device region in virtual memory
 * @pci: &struct driverkit_pci_device
 * @idx: the vfio region index to unmap
 * @mem: virtual memory address to unmap
 * @len: number of bytes to unmap
 * @offset: offset at which to start unmapping
 *
 * Unmap the virtual memory address, previously mapped to the vfio device memory
 * region identified by @idx.
 */
void vfio_pci_unmap_bar(struct driverkit_pci_device *pci, int idx, void *mem, size_t len,
			uint64_t offset);

/**
 * vfio_pci_read_config - Read from the PCI configuration space
 * @pci: &struct driverkit_pci_device
 * @buf: buffer to store the bytes read
 * @len: number of bytes to read
 * @offset: offset at which to read
 *
 * Read a number of bytes at a specified @offset in the PCI configuration space.
 *
 * Return: On success, returns the number of bytes read. On error, return ``-1``
 * and set ``errno``.
 */
static inline ssize_t vfio_pci_read_config(struct driverkit_pci_device *pci, void *buf, size_t len,
					   off_t offset)
{
	abort();
	// return pread(pci->dev.fd, buf, len, pci->config_region_info.offset + offset);
}

/**
 * vfio_pci_write_config - Write into the PCI configuration space
 * @pci: &struct driverkit_pci_device
 * @buf: buffer to write
 * @len: number of bytes to write
 * @offset: offset at which to write
 *
 * Write a number of bytes at a specified @offset in the PCI configuration space.
 *
 * Return: On success, returns the number of bytes written. On error, return
 * ``-1`` and set ``errno``.
 */
static inline ssize_t vfio_pci_write_config(struct driverkit_pci_device *pci, void *buf, size_t len,
					    off_t offset)
{
	abort();
	// return pwrite(pci->dev.fd, buf, len, pci->config_region_info.offset + offset);
}


#endif /* LIBVFN_VFIO_PCI_H */