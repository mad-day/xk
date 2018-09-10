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

static int vop_mkX (struct vnode* dir,struct vpath* comp,struct vattr* attr,struct vnode** target) { return -EOPNOTSUPP; }
static int vop_symlink (struct vnode* dir,struct vpath* comp,struct vattr* attr,const char* content,uint contentsiz,struct vnode** target) { return -EOPNOTSUPP; }
static int vop_walk (struct vnode* dir,struct vpath* first_comp, struct vpath** next) { return -EOPNOTSUPP; }
static ssize_t vop_rw (struct vnode* fil,uint64_t off, struct vsbuf* vsb) { return -EOPNOTSUPP; }
static int vop_truncate (struct vnode* fil,uint64_t len) { return -EOPNOTSUPP; }
static int vop_allocate (struct vnode* fil,uint mode,uint64_t off,uint64_t len) { return -EOPNOTSUPP; }
static int vop_getattr (struct vnode* fil,struct vattr* attr) { return -EOPNOTSUPP; }
static int vop_rmX (struct vnode* dir,struct vpath* comp) { return -EOPNOTSUPP; }
static int vop_link (struct vnode* dir,struct vpath* comp,struct vnode* other) { return -EOPNOTSUPP; }
static int vop_rename (struct vnode* src,struct vpath* srcnam,struct vnode* dest,struct vpath* dstnam) { return -EOPNOTSUPP; }
static void vop_put (struct vnode* vn) { }

#define SETDEFAULT(field,def) if(!(ops->field)) ops->field = def

void vn_fillops(struct vnode_ops* ops){
	SETDEFAULT(vop_create,vop_mkX);
	SETDEFAULT(vop_mkdir,vop_mkX);
	SETDEFAULT(vop_mknod,vop_mkX);
	SETDEFAULT(vop_symlink,vop_symlink);
	SETDEFAULT(vop_walk,vop_walk);
	SETDEFAULT(vop_read,vop_rw);
	SETDEFAULT(vop_write,vop_rw);
	SETDEFAULT(vop_truncate,vop_truncate);
	SETDEFAULT(vop_allocate,vop_allocate);
	SETDEFAULT(vop_getattr,vop_getattr);
	SETDEFAULT(vop_rmdir,vop_rmX);
	SETDEFAULT(vop_unlink,vop_rmX);
	SETDEFAULT(vop_link,vop_link);
	SETDEFAULT(vop_rename,vop_rename);
	SETDEFAULT(vop_put,vop_put);
}

