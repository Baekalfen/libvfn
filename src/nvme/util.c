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

#include <assert.h>
#include <byteswap.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/uio.h>

#include <linux/vfio.h>

#include <vfn/support.h>
#include <vfn/iommu.h>
#include <vfn/vfio.h>
#include <vfn/trace.h>
#include <vfn/nvme.h>

#include "types.h"

#include "crc64table.h"

uint64_t nvme_crc64(uint64_t crc, const unsigned char *buffer, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++)
		crc = (crc >> 8) ^ crc64_nvme_table[(crc & 0xff) ^ buffer[i]];

	return crc ^ (uint64_t)~0;
}

int nvme_set_errno_from_cqe(struct nvme_cqe *cqe)
{
	errno = le16_to_cpu(cqe->sfp) >> 1 ? EIO : 0;

	return errno ? -1 : 0;
}

int nvme_aer(struct nvme_ctrl *ctrl, void *opaque)
{
	struct nvme_rq *rq;
	union nvme_cmd cmd = { .opcode = NVME_ADMIN_ASYNC_EVENT };

	rq = nvme_rq_acquire_atomic(ctrl->adminq.sq);
	if (!rq) {
		errno = EBUSY;
		return -1;
	}

	cmd.cid = rq->cid | NVME_CID_AER;
	rq->opaque = opaque;

	/* rq_exec overwrites the command identifier, so use sq_exec */
	nvme_sq_exec(ctrl->adminq.sq, &cmd);

	return 0;
}

int nvme_sync(struct nvme_ctrl *ctrl, struct nvme_sq *sq, void *sqe, void *buf, size_t len, void *cqe_copy)
{
	struct nvme_cqe cqe;
	struct nvme_rq *rq;
	uint64_t iova;
	int ret = 0;

	if (buf && iommu_map_vaddr(__iommu_ctx(ctrl), buf, len, &iova, IOMMU_MAP_EPHEMERAL)) {
		log_debug("failed to map vaddr\n");
		return -1;
	}

	rq = nvme_rq_acquire_atomic(sq);
	if (!rq)
		return -1;

	if (buf) {
		ret = nvme_rq_map_prp(ctrl, rq, (union nvme_cmd *)sqe, iova, len);
		if (ret) {
			goto release_rq;
		}
	}

	nvme_rq_exec(rq, (union nvme_cmd *)sqe);

	while (nvme_rq_spin(rq, &cqe) < 0) {
		if (errno == EAGAIN) {
			log_error("SPURIOUS CQE (cq %" PRIu16 " cid %" PRIu16 ")\n",
				  (uint16_t)rq->sq->cq->id, cqe.cid);

			continue;
		}

		ret = -1;

		break;
	}

	if (cqe_copy)
		memcpy(cqe_copy, &cqe, 1 << NVME_CQES);

release_rq:
	nvme_rq_release_atomic(rq);

	if (buf)
		log_fatal_if(iommu_unmap_vaddr(__iommu_ctx(ctrl), buf, NULL),
			     "iommu_unmap_vaddr\n");

	return ret;
}

int nvme_admin(struct nvme_ctrl *ctrl, void *sqe, void *buf, size_t len, void *cqe_copy)
{
	return nvme_sync(ctrl, ctrl->adminq.sq, sqe, buf, len, cqe_copy);
}
