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
#include <kernel/mutex.h>

__BEGIN_CDECLS;

struct vnode;
struct vmount;
struct vmount_ops;

struct vmount {
	struct vmount_ops *m_op;
	void              *m_data;
	mutex_t            m_lock;      /* protects the reference count(s). */
	uint               m_usecount;  /* reference count. */
};

struct vmount_ops {
};

void vfs_mnt_fillops(struct vmount_ops* ops);

/*
 * Allocates a VNode with reference count 1.
 *
 * ->m_op is NULL.
 */
struct vmount* vfs_mnt_construct(void);


__END_CDECLS

