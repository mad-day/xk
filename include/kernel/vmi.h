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

#define VMI_FLAGS_READ   0x01
#define VMI_FLAGS_WRITE  0x02
#define VMI_FLAGS_EXEC   0x04

#define VMI_FLAGS_USER   0x10
#define VMI_FLAGS_KERNEL 0x20

/*
 * Handles a Page Fault/Page Request.
 *
 * Returns:
 *	- 0 if the page fault should result in a kernel panic.
 *	- 1 if the page fault was handled properly.
 */
uint vmi_page_fault(vaddr_t addr, uint flags);

/* Helper function used for finding a region within an aspace using an vaddr. */
vmm_region_t *vmi_find_region(const vmm_aspace_t *aspace, vaddr_t vaddr);


/* Helper function for using the VMM lock. */
mutex_t* vmi_vmm_lock(void);

