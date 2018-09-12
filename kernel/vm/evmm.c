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
//#include <trace.h>
#include <assert.h>
#include <kernel/vmi.h>
#include <kernel/evm.h>

static inline bool evmm_should_wait(evm_page_t *page,evm_prot_t prot) {
	if(
		page->busy ||
		page->absent
	) return true;
	if(page->page_lock&prot) return true;
	return false;
}

static inline bool evmm_can_map(evm_page_t *page,evm_prot_t prot) {
	if(
		page->fictitious ||
		page->error
	) return false;
	return true;
}

static inline uint evmm_prot_to_mmuflags(evm_prot_t prot, bool kernel_aspace) {
	uint ret = 0;
	
	/*
	 * The HAL has weird flags, partially based upon an "opt-out" tagging.
	 *
	 * We assume EVM_PROT_READ bit is set. We can't opt it out.
	 */
	if(!(prot&EVM_PROT_WRITE)) ret |= ARCH_MMU_FLAG_PERM_RO;
	if(!(prot&EVM_PROT_EXEC)) ret |= ARCH_MMU_FLAG_PERM_NO_EXECUTE;
	if(!kernel_aspace) ret |= ARCH_MMU_FLAG_PERM_USER;
	
	return ret;
}

uint vmi_page_fault(vaddr_t addr, uint flags)
{
	bool           kernelrange = false;
	bool           outbound = false;
	vmm_aspace_t  *aspace;
	vmm_region_t  *region;
	evmm_object_t *object = 0;
	vaddr_t        offset;
	evm_page_t    *page, *p;
	evm_prot_t     evm_prot = (flags & EVM_PROT_MASK) | EVM_PROT_READ;
	bool           request_fulfilled = false;
	
	/*
	 * The following steps are done while holding the VMM lock.
	 */
	mutex_acquire(vmi_vmm_lock());
	{
		aspace = vaddr_to_aspace((void*)addr);
		if(aspace) {
			/*
			 * We are in the boundaries of the address spaces.
			 */
			outbound = false;
			
			/*
			 * Is the target address in the higher half?
			 */
			kernelrange = (aspace==vmm_get_kernel_aspace());
			
			/*
			 * retrieve the region.
			 */
			region = vmi_find_region(aspace,addr);
			if(region) {
				if(region->flags&VMM_REGION_FLAG_EXTENDED) {
					object = region->e_object;
					object->refcount++; /* XXX undo this! */
				} else {
					object = 0;
				}
				offset = addr-region->base;
			}
		} else {
			kernelrange = false;
			outbound = true;
			region = 0;
		}
	}
	mutex_release(vmi_vmm_lock());
	/* Not holding the VMM lock anymore. */
	
	/*
	 * First, we handle the common Error cases upfront:
	 *
	 *    - Memory access attempt to a outside of any address space.
	 *      (also known as 'non-canonical address').
	 *
	 *    - Memory access attempt to the kernel memory from userspace.
	 *
	 *    - Memory access attempt outside of any region. (SEGFAULT)
	 */
	if(outbound) goto done;
	if((flags&VMI_FLAGS_USER) && kernelrange) goto done;
	if(!region) goto done;
	
	/*
	 * Other error cases:
	 *
	 *    - Touching an reserved, non-mapped area.
	 *
	 *    - The object is not pageable.
	 */
	if(region->flags&VMM_REGION_FLAG_RESERVED) goto done;
	if(!object) goto done;
	
	/*
	 * Lemma: At this point, we are capable, to use the EVMM object.
	 */
	
	
	mutex_acquire(&object->lock);
	{
		page = 0;
		/*
		 * Step 1, find the Page if it exists.
		 */
		list_for_every_entry(&object->memq, p, evm_page_t, listq)
		{
			if(offset<p->offset) continue;
			if(offset>p->offset) break;
			page = p;
		}
		
		/*
		 * Step 2, if not found, obtain the page.
		 */
		if(!page){
			mutex_release(&object->lock);
			object->pagerops->evm_page_req(object,offset,evm_prot,&page);
			mutex_acquire(&object->lock);
		}
		
		/*
		 * Step 2a, if not obtained, terminate the algorithm.
		 */
		if(!page) goto vmodone;
		
		/*
		 * Step 3, check upfront, if we have a conflict with the
		 * desired use of this page, and the use of this page, prohibited by
		 * the data manager.
		 */
		if(page->not_allowed&evm_prot) goto vmodone;
		
		/*
		 * Step 4, Wait for the page to become ready.
		 */
		while(evmm_should_wait(page,evm_prot)) {
			/*
			 * Prevent the page-out daemon from paging out this page.
			 */
			page->wanted = 1;
			
			mutex_release(&object->lock);
			event_wait(&object->waitq);
			mutex_acquire(&object->lock);
		}
		/*
		 * Step 5, Check, if we can map this page.
		 */
		if(evmm_can_map(page,evm_prot)) goto vmodone;
		
		/*
		 * At this point we MUST have a physical page.
		 */
		DEBUG_ASSERT(page->phys_page);
		
		/*
		 * Retain the rights, already granted to others.
		 * Memoize the rights, granted to ourself.
		 */
		evm_prot_t evmprot2 = evm_prot | ( (EVM_PROT_MASK^page->page_lock) & page->prev_uses );
		page->prev_uses = evmprot2;
		
		/*
		 * Release the object-lock.
		 */
		mutex_release(&object->lock);
		
		/*
		 * Obtain the physical address.
		 */
		paddr_t paddr = vm_page_to_paddr(page->phys_page);
		
		bool freeme;
		
		/*
		 * Final step. Acquire the VMM lock. and map the page.
		 */
		mutex_acquire(vmi_vmm_lock());
		{
			request_fulfilled = true;
			
			arch_mmu_map(&aspace->arch_aspace,ROUNDDOWN(addr, PAGE_SIZE),paddr,1,evmm_prot_to_mmuflags(evmprot2,kernelrange));
			
			/*
			 * Efficiency: decrement the refcount, as we are already holding the mandatory lock.
			 */
			object->refcount--;
			freeme = !object->refcount;
		}
		mutex_release(vmi_vmm_lock());
		
		/*
		 * Free the object if the refcount dropped to ZERO. (UNLIKELY)
		 */
		if(unlikely(freeme)) evmm_destroy(object);
		
		/*
		 * Prevent double-release.
		 */
		object = 0;
		
		/*
		 * At this point we already released the object-lock, so skip it.
		 */
		goto done;
	}
vmodone:
	mutex_release(&object->lock);
	
done:
	if(object) {
		evmm_release(object);
		object = 0;
	}
	
	/*
	 * If we fullfilled the request successfully, we return gracefully.
	 */
	if(request_fulfilled) return 1;
	
	if(flags&VMI_FLAGS_USER) {
		/*
		 * We have a segmentation fault here.
		 */
		return 0; /* TODO: don't crash the kernel! */
	}
	
	return 0;
}

void evmm_release(evmm_object_t* obj)
{
	bool freeme;
	mutex_acquire(vmi_vmm_lock());
	{
		obj->refcount--;
		freeme = !obj->refcount;
	}
	mutex_release(vmi_vmm_lock());
	if(freeme) evmm_destroy(obj);
}

void evmm_destroy(evmm_object_t* obj)
{
	/* TODO: implement! */
}

