
#ifndef LIBVFN_SUPPORT_ERRNO_H
#define LIBVFN_SUPPORT_ERRNO_H

extern "C" {
extern int errno;

#define ENOENT 2
#define EIO 5
#define EBUSY 16
#define EEXIST 17
#define EINVAL 22
#define EAGAIN 35
#define ETIMEDOUT 60
}

#endif
