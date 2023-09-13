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

// TODO: Move this to header files?

#ifndef __APPLE__
#include <errno.h>
#ifdef __GLIBC__
#include <execinfo.h>
#endif
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#else
#include <DriverKit/IOLib.h>
#include <DriverKit/IOBufferMemoryDescriptor.h>
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <vfn/support/align.h>
#include <vfn/support/atomic.h>
#include <vfn/support/compiler.h>
#include <vfn/support/log.h>
#include <vfn/support/mem.h>

size_t __VFN_PAGESIZE;
int __VFN_PAGESHIFT;

static void __attribute__((constructor)) init_page_size(void)
{
	__VFN_PAGESIZE = IOVMPageSize;
	__VFN_PAGESHIFT = 63u - __builtin_clzl(__VFN_PAGESIZE);

	log_debug("pagesize is %zu (shift %d)\n", __VFN_PAGESIZE, __VFN_PAGESHIFT);
}

void backtrace_abort(void)
{
	abort();
}

ssize_t pgmap(void **mem, size_t sz)
{
	ssize_t len = ALIGN_UP(sz, __VFN_PAGESIZE);

	// IOBufferMemoryDescriptor* dmaBuffer;// = *mem;

	IOBufferMemoryDescriptor::Create(
		kIOMemoryDirectionInOut,
		len,
		__VFN_PAGESIZE,
		(IOBufferMemoryDescriptor **)mem
	);
	(*(IOBufferMemoryDescriptor **)mem)->SetLength(len);
	IOAddressSegment virtualAddressSegment;
	(*(IOBufferMemoryDescriptor **)mem)->GetAddressRange(&virtualAddressSegment);
	bzero((void*) virtualAddressSegment.address, virtualAddressSegment.length);

	return len;
}

ssize_t pgmapn(void **mem, unsigned int n, size_t sz)
{
	return pgmap(mem, n * sz);
}

#ifdef __cplusplus
}
#endif
