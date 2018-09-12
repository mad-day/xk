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
#include <sys/syscall.h>
#include <arch/arm64.h>

void arm64_syscall(struct arm64_iframe_long *frame, bool is_64bit)
{
    frame->r[0] = /* X0 */
    generic_syscall_handler(
     is_64bit?frame->r[8]:frame->r[7], /* no */ /* X8 on 64 bit, r7 on 32 bit. */
     frame->r[0], /* a1 */
     frame->r[1], /* a2 */
     frame->r[2], /* a3 */
     frame->r[3], /* a4 */
     frame->r[4], /* a5 */
     frame->r[5]  /* a6 */
    );
}

