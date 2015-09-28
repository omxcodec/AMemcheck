/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include "debug_stacktrace.h"

#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef USE_NDK_BUILD
#include "include/unwind.h"
#else
#include <unwind.h>
#endif

#include "dlmalloc.h"
#include "debug_ptrace.h"
#include "libc_logging.h"

/* depends how the system includes define this */
#ifdef HAVE_UNWIND_CONTEXT_STRUCT
typedef struct _Unwind_Context __unwind_context;
#else
typedef _Unwind_Context __unwind_context;
#endif

static ptrace_context_t* gPtraceContext = NULL;
static void* gDemangler;
typedef char* (*DemanglerFn)(const char*, char*, size_t*, int*);
static DemanglerFn gDemanglerFn = NULL;

#define MAX_BACKTRACE_DEPTH 16

__LIBC_HIDDEN__ void backtrace_startup() {
  gPtraceContext = load_ptrace_context(getpid());
  gDemangler = dlopen("libgccdemangle.so", RTLD_NOW);
  if (gDemangler != NULL) {
    void* sym = dlsym(gDemangler, "__cxa_demangle");
    gDemanglerFn = reinterpret_cast<DemanglerFn>(sym);
  }
}

__LIBC_HIDDEN__ void backtrace_shutdown() {
  free_ptrace_context(gPtraceContext);
  dlclose(gDemangler);
}

static char* demangle_symbol_name(const char* symbol) {
  if (gDemanglerFn == NULL) {
    return NULL;
  }
  return (*gDemanglerFn)(symbol, NULL, NULL, NULL);
}

struct stack_crawl_state_t {
  uintptr_t* frames;
  size_t frame_count;
  size_t max_depth;
  bool have_skipped_self;

  stack_crawl_state_t(uintptr_t* frames, size_t max_depth)
      : frames(frames), frame_count(0), max_depth(max_depth), have_skipped_self(false) {
  }
};

static _Unwind_Reason_Code trace_function(__unwind_context* context, void* arg) {
  stack_crawl_state_t* state = static_cast<stack_crawl_state_t*>(arg);

  uintptr_t ip = _Unwind_GetIP(context);

  // The first stack frame is get_backtrace itself. Skip it.
  if (ip != 0 && !state->have_skipped_self) {
    state->have_skipped_self = true;
    return _URC_NO_REASON;
  }

#ifdef __arm__
  /*
   * The instruction pointer is pointing at the instruction after the bl(x), and
   * the _Unwind_Backtrace routine already masks the Thumb mode indicator (LSB
   * in PC). So we need to do a quick check here to find out if the previous
   * instruction is a Thumb-mode BLX(2). If so subtract 2 otherwise 4 from PC.
   */
  if (ip != 0) {
    short* ptr = reinterpret_cast<short*>(ip);
    // Thumb BLX(2)
    if ((*(ptr-1) & 0xff80) == 0x4780) {
      ip -= 2;
    } else {
      ip -= 4;
    }
  }
#endif

  state->frames[state->frame_count++] = ip;
  return (state->frame_count >= state->max_depth) ? _URC_END_OF_STACK : _URC_NO_REASON;
}

__LIBC_HIDDEN__ int get_backtrace(uintptr_t* frames, size_t max_depth) {
  stack_crawl_state_t state(frames, max_depth);
  _Unwind_Backtrace(trace_function, &state);
  return state.frame_count;
}

/////////////////////////////////////////////////////////////////////////////////////

/*
 * Describes a single frame of a backtrace.
 */
typedef struct {
    uintptr_t absolute_pc;     /* absolute PC offset */
    uintptr_t stack_top;       /* top of stack for this frame */
    size_t stack_size;         /* size of this stack frame */
} backtrace_frame_t;

/*
 * Describes the symbols associated with a backtrace frame.
 */
typedef struct {
    uintptr_t relative_pc;       /* relative frame PC offset from the start of the library,
                                    or the absolute PC if the library is unknown */
    uintptr_t relative_symbol_addr; /* relative offset of the symbol from the start of the
                                    library or 0 if the library is unknown */
    char* map_name;              /* executable or library name, or NULL if unknown */
    char* symbol_name;           /* symbol name, or NULL if unknown */
    char* demangled_name;        /* demangled symbol name, or NULL if unknown */
} backtrace_symbol_t;

enum {
    // A hint for how big to make the line buffer for format_backtrace_line
    MAX_BACKTRACE_LINE_LENGTH = 800,
};

static void init_backtrace_symbol(backtrace_symbol_t* symbol, uintptr_t pc) {
    symbol->relative_pc = pc;
    symbol->relative_symbol_addr = 0;
    symbol->map_name = NULL;
    symbol->symbol_name = NULL;
    symbol->demangled_name = NULL;
}

/*
 * Gets the symbols for each frame of a backtrace.
 * The symbols array must be big enough to hold one symbol record per frame.
 * The symbols must later be freed using free_backtrace_symbols.
 */
static void get_backtrace_symbols(const ptrace_context_t* context,
        const backtrace_frame_t* backtrace, size_t frames,
        backtrace_symbol_t* backtrace_symbols) {
    for (size_t i = 0; i < frames; i++) {
        const backtrace_frame_t* frame = &backtrace[i];
        backtrace_symbol_t* symbol = &backtrace_symbols[i];
        init_backtrace_symbol(symbol, frame->absolute_pc);

        const map_info_t* mi;
        const symbol_t* s;
        find_symbol_ptrace(context, frame->absolute_pc, &mi, &s);
        if (mi) {
            symbol->relative_pc = frame->absolute_pc - mi->start;
            if (mi->name[0]) {
                symbol->map_name = strdup(mi->name);
            }
        }
        if (s) {
            symbol->relative_symbol_addr = s->start;
            symbol->symbol_name = strdup(s->name);
            symbol->demangled_name = demangle_symbol_name(symbol->symbol_name);
        }
    }
}

/*
 * Frees the storage associated with backtrace symbols.
 */
static void free_backtrace_symbols(backtrace_symbol_t* backtrace_symbols, size_t frames) {
    for (size_t i = 0; i < frames; i++) {
        backtrace_symbol_t* symbol = &backtrace_symbols[i];
        dlfree(symbol->map_name);
        dlfree(symbol->symbol_name);
        free(symbol->demangled_name); //pairing with demangle(malloc)
        init_backtrace_symbol(symbol, 0);
    }
}

/**
 * Formats a line from a backtrace as a zero-terminated string into the specified buffer.
 */
static void format_backtrace_line(unsigned frameNumber, const backtrace_frame_t* frame __attribute__((unused)),
    const backtrace_symbol_t* symbol, char* buffer, size_t bufferSize) {
  const char* mapName = symbol->map_name ? symbol->map_name : "<unknown>";
  const char* symbolName = symbol->demangled_name ? symbol->demangled_name : symbol->symbol_name;
  if (symbolName) {
    uint32_t pc_offset = symbol->relative_pc - symbol->relative_symbol_addr;
    if (pc_offset) {
      snprintf(buffer, bufferSize, "			#%02d  pc %08x	%s (%s+0x%x)",
	          frameNumber, symbol->relative_pc, mapName, symbolName, pc_offset);
    } else {
      snprintf(buffer, bufferSize, "			#%02d  pc %08x	%s (%s)",
              frameNumber, symbol->relative_pc, mapName, symbolName);
    }
  } else {
    snprintf(buffer, bufferSize, "			#%02d  pc %08x	%s",
            frameNumber, symbol->relative_pc, mapName);
  }
}

__LIBC_HIDDEN__ void log_backtrace(uintptr_t* frames, size_t frame_count) {
  uintptr_t self_bt[MAX_BACKTRACE_DEPTH];
  if (frames == NULL) {
    frame_count = get_backtrace(self_bt, MAX_BACKTRACE_DEPTH);
    frames = self_bt;
  }

  backtrace_frame_t *backtrace = (backtrace_frame_t *)dlmalloc(sizeof(backtrace_frame_t) * frame_count);
  if (!backtrace) {
    return;
  }
  memset(backtrace, 0, sizeof(backtrace_frame_t) * frame_count);
  for (size_t i = 0; i < frame_count; i++) {
    backtrace[i].absolute_pc = frames[i]; //stack_top and stack_size don't been used;
  }

  backtrace_symbol_t *backtrace_symbols = (backtrace_symbol_t *)dlmalloc(sizeof(backtrace_symbol_t) * frame_count);
  if (!backtrace_symbols) {
    dlfree(backtrace);
    return;
  }
  memset(backtrace_symbols, 0, sizeof(backtrace_symbol_t) * frame_count);

  __libc_format_log(TANGMAI_LOG_ERROR, "libc",
                    "*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***\n");
  get_backtrace_symbols(gPtraceContext, backtrace, frame_count, backtrace_symbols);
  for (size_t i = 0; i < frame_count; i++) {
	  char line[MAX_BACKTRACE_LINE_LENGTH];
	  format_backtrace_line(i, &backtrace[i], &backtrace_symbols[i],
			  line, MAX_BACKTRACE_LINE_LENGTH);
	  __libc_format_log(TANGMAI_LOG_ERROR, "libc",  "    %s\n", line);
  }
  free_backtrace_symbols(backtrace_symbols, frame_count);

  dlfree(backtrace_symbols);
  dlfree(backtrace);
}
