/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Useful ptrace() utility functions. */

#ifndef DEBUG_PTRACE_H
#define DEBUG_PTRACE_H

#include "debug_map_info.h"
#include "debug_symbol_table.h"

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Stores information about a process that is used for several different
 * ptrace() based operations. */
typedef struct {
    map_info_t* map_info_list;
} ptrace_context_t;

/*
 * Loads information needed for examining a remote process using ptrace().
 * The caller must already have successfully attached to the process
 * using ptrace().
 *
 * The context can be used for any threads belonging to that process
 * assuming ptrace() is attached to them before performing the actual
 * unwinding.  The context can continue to be used to decode backtraces
 * even after ptrace() has been detached from the process.
 */
ptrace_context_t* load_ptrace_context(pid_t pid);

/*
 * Frees a ptrace context.
 */
void free_ptrace_context(ptrace_context_t* context);

/*
 * Finds a symbol using ptrace.
 * Returns the containing map and information about the symbol, or
 * NULL if one or the other is not available.
 */
void find_symbol_ptrace(const ptrace_context_t* context,
        uintptr_t addr, const map_info_t** out_map_info, const symbol_t** out_symbol);

#ifdef __cplusplus
}
#endif

#endif // DEBUG_PTRACE_H
