/*
 * Copyright (c) 2018 Simon Schmidt
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once
#include <stdbool.h>
#include <sys/types.h>
#include <compiler.h>
#include <vstream/vsbuf.h>
#include <kernel/mutex.h>
/*
 * This code is inspired by 4.4BSD-Lite by UC Berkeley.
 *
 * Acknowledgements:
 *      The Regents of the University of California.
 *        representing:
 *          The CSRG (+) of UC Berkeley
 */


__BEGIN_CDECLS;

struct vnode;
struct vmount;
struct vnode_ops;
struct vpath;

/*
 * VNode types.
 */
enum {
	VNON, /* no type. */
	VREG,
	VDIR,
	VBLK,
	VCHR,
	VLNK,
	VSOCK,
	VFIFO,
	VBAD, /* bad type. */
};

/*
 * VNode attribute.
 */
struct vattr {
	uint     va_type;    /* VNode type (create) */
	ushort   va_mode;    /* Unix file Mode. */
	ushort   va_nlink;   /* Number of links. */
	uint32_t va_uid;     /* Unix User id. */
	uint32_t va_gid;     /* Unix Group id. */
	uint64_t va_fileid;  /* Inode. */
	uint64_t va_size;    /* file size. */
	off_t    va_blocks;  /* Number of blocks. (user) */
	off_t    va_blksize; /* Block size. (user) */
	uint64_t va_rdev;    /* Device ID for this device file. (VBLK,VCHR) (not used yet) */
};

struct vnode {
	struct vmount    *v_mount;     /* The VMount, from which this VNode is part of. */
	struct vnode_ops *v_op;        /* The operations for this VNode*/
	void             *v_data;
	uint              v_type;      /* vnode type. */
	mutex_t           v_lock;      /* protects the reference count(s). */
	uint              v_usecount;  /* reference count. */
};

struct vnode_ops {
	int (*vop_create) (struct vnode* dir,struct vpath* comp,struct vattr* attr,struct vnode** target);
	int (*vop_mknod) (struct vnode* dir,struct vpath* comp,struct vattr* attr,struct vnode** target);
	int (*vop_mkdir) (struct vnode* dir,struct vpath* comp,struct vattr* attr,struct vnode** target);
	int (*vop_symlink) (struct vnode* dir,struct vpath* comp,struct vattr* attr,const char* content,uint contentsiz,struct vnode** target);
	int (*vop_walk) (struct vnode* dir,struct vpath* first_comp, struct vpath** next);
	ssize_t (*vop_read) (struct vnode* fil,uint64_t off, struct vsbuf* vsb);
	ssize_t (*vop_write) (struct vnode* fil,uint64_t off, struct vsbuf* vsb);
	int (*vop_truncate) (struct vnode* fil,uint64_t len);
	int (*vop_allocate) (struct vnode* fil,uint mode,uint64_t off,uint64_t len);
	int (*vop_getattr) (struct vnode* fil,struct vattr* attr);
	int (*vop_rmdir) (struct vnode* dir,struct vpath* comp);
	int (*vop_unlink) (struct vnode* dir,struct vpath* comp);
	int (*vop_link) (struct vnode* dir,struct vpath* comp,struct vnode* other);
	int (*vop_rename) (struct vnode* src,struct vpath* srcnam,struct vnode* dest,struct vpath* dstnam);
	void (*vop_put) (struct vnode* vn);
};

void vn_fillops(struct vnode_ops* ops);

int vn_walk(struct vnode* base,struct vpath* vphead);

ssize_t vn_read(struct vnode* fil,uint64_t off, struct vsbuf* vsb);
ssize_t vn_write(struct vnode* fil,uint64_t off, struct vsbuf* vsb);
void vn_put(struct vnode* vn);
struct vnode* vn_retain(struct vnode* vn);

/*
 * Allocates a VNode with reference count 1.
 *
 * ->v_mount and ->v_op is NULL.
 */
struct vnode* vn_construct(void);


__END_CDECLS

