/*
 * Copyright (C) 2009 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef AMEMCHECK_H
#define AMEMCHECK_H

#include <sys/cdefs.h>
#include <stddef.h>

__BEGIN_DECLS

// =============================================================================
// init and fini functions
// =============================================================================

void amemcheck_init();
void amemcheck_fini();

// =============================================================================
// misc functions
// =============================================================================

void* amemcheck_malloc(size_t bytes);
void  amemcheck_free(void* mem);
void* amemcheck_calloc(size_t n_elements, size_t elem_size);
void* amemcheck_realloc(void* oldMem, size_t bytes);
void* amemcheck_memalign(size_t alignment, size_t bytes);

struct mallinfo amemcheck_mallinfo();
size_t amemcheck_malloc_usable_size(void* mem);
void*  amemcheck_valloc(size_t bytes);
void*  amemcheck_pvalloc(size_t bytes);

__END_DECLS

#endif  // AMEMCHECK_H
