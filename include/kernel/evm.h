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

#include <kernel/vm.h>
#include <kernel/mutex.h>
#include <list.h>
/*
 * This code is inspired by the VM code of the Mach kernel, made by the CMU
 * and its contributors.
 */

typedef uint evm_prot_t;

/*
 * These are identical to VMI_FLAGS_READ,VMI_FLAGS_WRITE and VMI_FLAGS_EXEC!
 */
enum {
	EVM_PROT_READ  = 0x01,
	EVM_PROT_WRITE = 0x02,
	EVM_PROT_EXEC  = 0x04,
	/* Mask to extract them from VMI flags. */
	EVM_PROT_MASK  = 0x07,
};

// Forward declarations.
struct evmm_object;
struct evmm_object_ops;

/*
 *     Management of resident (logical) pages.
 *
 *     A small datastructure is kept for each resident
 *     page, indexed by page number. Each structure
 *     is an element of several lists, for example:
 *
 *     - A hashtable bucket for quick object/offset lookup.
 *       (not yet implemented!)
 *
 *     - A list of all pages for a given object.
 *
 *     - An ordered list of pages due for pageout.
 *
 *
 *     Fields in this structure are locked eighter by the lock on the
 *     object that the page belongs to (O) or by the lock on the page
 *     queues (P). Some fields require that both locks be held to change
 *     that field; holding either lock is sufficient to read.
 */
typedef struct evm_page {
	struct list_node    pageq;       /* queue for FIFO queue or free list (P) */
	struct list_node    listq;       /* all pages in same object (O) */
	struct evm_page    *next;        /* Hash table bucket link. (O) */
	
	struct evmm_object *object;      /* which object am I in (O,P) */
	vaddr_t             offset;      /* offset into that object (O,P) */
	
	uint                wirecnt:16,  /* how many wired down maps use me? (O,P) */
	                    inactive:1,  /* page is in inactive list (P) */
	                    active:1,    /* page is in active list (P) */
			    laundry:1,   /* page is being cleaned now (P) */
			    free:1,      /* page is on free list (P) */
			    reference:1, /* page has been used (P) */
			    :0;          /* Padding! */
	//uint pad1;
	
	/* The following fields are locked by (O). */
	uint                busy:1,      /* page is in transit (O) */
	                    wanted:1,    /* someone is waiting for page (O) */
			    tabled:1,    /* page is in Hash table (O) */
			    fictitious:1,/* physical page doesn't exist (O) */
			    absent:1,    /* Data has been requested, but is not yet available (O). */
			    error:1,     /* Data manager was unable to privide data due to error (O). */
			    dirty:1,     /* Page must be cleaned. */
			    precious:1,  /* Page is precious; data must not be discarded, even if clean (O). */
			    overwrit:1,  /* Request to unlock has been made without having data (O). */
			    :0;          /* Padding! */
	//uint pad2;
	
	vm_page_t*          phys_page;   /* Physical page object. */
	
	evm_prot_t          page_lock;   /* Uses temporarily prohibited by data manager (O). Lock-Style. */
	evm_prot_t          not_allowed; /* Uses permanently prohibited by data manager (O). */
	evm_prot_t          unlock_req;  /* Outstanding unlock request (O). */
	evm_prot_t          prev_uses;   /* Outstanding unlock request (O). */
} evm_page_t;

/*
 *     Fields in this structure are locked by the object itself,
 *     or by the VMM lock (V).
 */
typedef struct evmm_object {
	struct list_node        e_list;   /* Mapping-list. (->e_node) */
	mutex_t                 lock;     /* The lock, protecting this object. */
	uint                    refcount; /* The (general) reference count. (V) */
	
	struct list_node        memq;     /* resident memory. */
	
	struct evmm_object_ops* pagerops; /* Operations for Page requests. */
	void*                   pager;    /* The internal pager object. */
} evmm_object_t;

struct evmm_object_ops {
	void (*evm_page_req)(evmm_object_t* object,vaddr_t offset, evm_prot_t prot, evm_page_t** pagep);
};

void evmm_release(evmm_object_t* obj);
void evmm_destroy(evmm_object_t* obj);

