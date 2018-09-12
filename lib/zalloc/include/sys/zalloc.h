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
#include <sys/types.h>
/* Inspired by the Zone allocator implemented in the CMU Mach kernel. */

struct zone;
typedef struct zone *zone_t;

typedef void (*zone_event_t) (zone_t zone);

zone_t zinit(off_t allocsz,zone_event_t alloc,zone_event_t free,void* userdata);

void *zone_userdata(zone_t zone);

/*
 * Allocate memory.
 */
vaddr_t zalloc(zone_t zone);
vaddr_t zget(zone_t zone);

/*
 * Free memory.
 */
void zfree(zone_t zone,vaddr_t buf);
void zput(zone_t zone,vaddr_t buf);

void zcram(zone_t zone,vaddr_t newmem,off_t size);

int zuncram(zone_t zone,vaddr_t *oldmem,off_t *size);

