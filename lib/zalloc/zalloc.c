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
#include <sys/zalloc.h>
#include <list.h>
#include <kernel/mutex.h>
#include <malloc.h>

/*
 * Shamelessly ripped from list.h and changed.
 */
#define list_for_every_entry_rev(list, entry, type, member) \
  for((entry) = containerof((list)->prev, type, member);\
    &(entry)->member != (list);\
    (entry) = containerof((entry)->member.prev, type, member))


typedef struct zone_object {
	uintptr_t refc;
} zone_object_t;

typedef struct zone_block {
	struct list_node  node;
	uintptr_t         begin;
	off_t             size,count,used;
} zone_block_t;

struct zone{
	mutex_t           mutex;
	off_t             allocsz;
	zone_event_t      alloc, free;
	void             *userdata;
	struct list_node  blocks;
};

static inline off_t alignment(off_t allocsz){
	allocsz += 15;
	allocsz -= allocsz&16;
	return allocsz;
}

zone_t zinit(off_t allocsz,zone_event_t alloc,zone_event_t free,void* userdata){
	zone_t t = calloc(1,sizeof(struct zone));
	if(!t) return 0;
	mutex_init(&t->mutex);
	list_initialize(&t->blocks);
	t->allocsz = alignment(sizeof(zone_object_t)+allocsz);
	t->alloc = alloc;
	t->free = free;
	t->userdata = userdata;
	return t;
}

void *zone_userdata(zone_t zone){
	return zone->userdata;
}

static vaddr_t izalloc(zone_t zone){
	zone_object_t* obj;
	zone_block_t* block;
	vaddr_t chunk = (vaddr_t)(zone->allocsz),newmem;
	off_t i,n;
	
	mutex_acquire(&zone->mutex);
	list_for_every_entry(&zone->blocks,block,zone_block_t,node) {
		newmem = block->begin;
		n = block->count;
		for(i=0;i<n;++i,newmem+=chunk) {
			obj = (zone_object_t*)newmem;
			if(obj->refc) continue;
			block->used++;
			obj->refc++;
			obj++;
			return (vaddr_t)obj;
		}
	}
	mutex_release(&zone->mutex);
	
	return 0;
}
static bool izfree(zone_t zone,zone_object_t* obj) {
	zone_block_t* block;
	vaddr_t chunk = (vaddr_t)(zone->allocsz),newmem;
	off_t n;
	
	obj--;
	vaddr_t buf = (vaddr_t)obj;
	
	mutex_acquire(&zone->mutex);
	list_for_every_entry(&zone->blocks,block,zone_block_t,node) {
		newmem = block->begin;
		n = block->count*chunk;
		n += newmem-1;
		if(buf<newmem||n<buf) continue;
		
		obj->refc--;
		if(obj->refc) return false;
		
		block->used--;
		if(block->used) return false;
		
		list_delete(&block->node);
		list_add_tail(&zone->blocks,&block->node);
		return true;
	}
	mutex_release(&zone->mutex);
	return false;
}


/*
 * Allocate memory.
 */
vaddr_t zalloc(zone_t zone){
	vaddr_t vt = izalloc(zone);
	if(!vt)
	{
		zone->alloc(zone);
	}
	return vt;
}
vaddr_t zget(zone_t zone) { return izalloc(zone); }

/*
 * Free memory.
 */
void zfree(zone_t zone,vaddr_t buf){
	if(izfree(zone,(zone_object_t*)buf))
	{
		zone->free(zone);
	}
}
void zput(zone_t zone,vaddr_t buf){
	izfree(zone,(zone_object_t*)buf);
}

void zcram(zone_t zone,vaddr_t newmem,off_t size){
	off_t prefix = alignment(sizeof(zone_block_t));
	off_t i;
	vaddr_t chunk = (vaddr_t)(zone->allocsz);
	
	if(!newmem) return;
	
	if(prefix>size) return; /* Ignore it. XXX leak! */
	zone_block_t *blk = (zone_block_t*)newmem;
	blk->size = size;
	size -= prefix;
	size /= chunk;
	blk->count = size;
	blk->used = 0;
	blk->begin = newmem + prefix;
	
	for(i=0;i<size;++i,newmem+=chunk)
		((zone_object_t*)newmem)->refc=0;
	
	mutex_acquire(&zone->mutex);
	list_add_tail(&zone->blocks,&blk->node);
	mutex_release(&zone->mutex);
}

int zuncram(zone_t zone,vaddr_t *oldmem,off_t *size){
	zone_block_t* block;
	
	mutex_acquire(&zone->mutex);
	list_for_every_entry_rev(&zone->blocks,block,zone_block_t,node) {
		if(!block->used) {
			list_delete(&block->node);
			*oldmem = (vaddr_t)block;
			*size = block->size;
		}
	}
	mutex_release(&zone->mutex);
	return 0;
}

