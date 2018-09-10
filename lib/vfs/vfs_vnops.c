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
#include <errno.h>
#include <malloc.h>

/*#define VNODE(base) containerof(base,struct vnode,vp_list)*/
#define VPATH(base) containerof(base,struct vpath,vp_list)

static int vn_resolve(struct vnode* base,struct vnode** target)
{
	*target = base;
	/* TODO: resolve mountpoints, once invented! */
	return 0;
}

int vn_walk(struct vnode* base,struct vpath* vphead)
{
	struct vpath *cur, *next;
	struct vnode* pos = base;
	
	cur = VPATH(vphead->vp_list.next);
	for(;;){
		next = cur;
		int r = pos->v_op->vop_walk(pos,cur,&next);
		if(!r) return r;
		if(!next) return -EIO;
		if(vp_ishead(next)) return 0;
		vn_resolve(VPATH(next->vp_list.prev)->vp_node,&pos);
	}
	
	return -EOPNOTSUPP;
}

ssize_t vn_read(struct vnode* fil,uint64_t off, struct vsbuf* vsb)
{
	return fil->v_op->vop_read(fil,off,vsb);
}

ssize_t vn_write(struct vnode* fil,uint64_t off, struct vsbuf* vsb)
{
	return fil->v_op->vop_write(fil,off,vsb);
}

void vn_put(struct vnode* vn)
{
	mutex_acquire(&(vn->v_lock));
	vn->v_usecount--;
	bool done = !(vn->v_usecount);
	mutex_release(&(vn->v_lock));
	if(done) {
		vn->v_op->vop_put(vn);
		mutex_destroy(&(vn->v_lock));
		free(vn);
	}
}

struct vnode* vn_retain(struct vnode* vn)
{
	mutex_acquire(&(vn->v_lock));
	vn->v_usecount++;
	mutex_release(&(vn->v_lock));
	return vn;
}

