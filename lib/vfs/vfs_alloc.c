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
#include <vfs/vnode.h>
#include <vfs/vpath.h>
#include <vfs/vmount.h>
#include <errno.h>
#include <malloc.h>

struct vnode* vn_construct(void){
	struct vnode* t = calloc(1,sizeof(struct vnode));
	if(!t) return 0;
	mutex_init(&(t->v_lock));
	t->v_usecount = 1;
	return t;
}

struct vpath* vp_construct(void){
	struct vpath* t = calloc(1,sizeof(struct vpath));
	if(!t) return 0;
	return t;
}

struct vmount* vfs_mnt_construct(void){
	struct vmount* t = calloc(1,sizeof(struct vmount));
	if(!t) return 0;
	mutex_init(&(t->m_lock));
	t->m_usecount = 1;
	return t;
}

