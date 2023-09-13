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

#ifndef LIBVFN_NVME_H
#define LIBVFN_NVME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#ifndef __APPLE__
#include <byteswap.h>
#include <errno.h>
#include <unistd.h>

#include <sys/mman.h>

#include <linux/vfio.h>
#else
#include <vfn/support/platform/macos/byteswap.h>
#include <vfn/support/platform/macos/errno.h>
#include <DriverKit/IOBufferMemoryDescriptor.h>
#endif

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <ccan/likely/likely.h> // FIX: Circular dependency
#include <vfn/support/compiler.h> // FIX: Circular dependency
#include <vfn/support/align.h> // FIX: Circular dependency
#include <vfn/support/atomic.h> // FIX: Circular dependency
#include <vfn/support/log.h> // FIX: Circular dependency
#include <vfn/support/mem.h> // FIX: Circular dependency
#include <vfn/vfio.h> // TODO: Circular dependency
#include <vfn/support.h> // TODO: Circular dependency
#include <vfn/trace.h>
#ifndef __APPLE__
#include <vfn/trace/events.h>
#endif
#include <vfn/nvme/types.h>
#include <vfn/nvme/queue.h>
#include <vfn/nvme/ctrl.h>
#include <vfn/nvme/util.h>
#include <vfn/nvme/rq.h>

#ifdef __cplusplus
}
#endif

#endif /* LIBVFN_NVME_H */
