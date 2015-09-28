/*
  Default header file for malloc-2.8.x, written by Doug Lea
  and released to the public domain, as explained at
  http://creativecommons.org/publicdomain/zero/1.0/ 
 
  This header is for ANSI C/C++ only.  You can set any of
  the following #defines before including:

  * If USE_DL_PREFIX is defined, it is assumed that malloc.c 
    was also compiled with this option, so all routines
    have names starting with "dl".

  * If HAVE_USR_INCLUDE_MALLOC_H is defined, it is assumed that this
    file will be #included AFTER <malloc.h>. This is needed only if
    your system defines a struct mallinfo that is incompatible with the
    standard one declared here.  Otherwise, you can include this file
    INSTEAD of your system system <malloc.h>.  At least on ANSI, all
    declarations should be compatible with system versions

  * If MSPACES is defined, declarations for mspace versions are included.
*/

#ifndef MALLOC_280_H
#define MALLOC_280_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>   /* for size_t */
#include <malloc.h>

#define dlcalloc               calloc
#define dlfree                 free
#define dlmalloc               malloc
#define dlmemalign             memalign
#define dlrealloc              realloc
#define dlvalloc               valloc
#define dlpvalloc              pvalloc
#define dlmallinfo             mallinfo
#define dlmallopt              mallopt
#define dlmalloc_trim          malloc_trim
#define dlmalloc_stats         malloc_stats
#define dlmalloc_usable_size   malloc_usable_size
#define dlmalloc_footprint     malloc_footprint
#define dlmalloc_max_footprint malloc_max_footprint
#define dlmalloc_footprint_limit malloc_footprint_limit
#define dlmalloc_set_footprint_limit malloc_set_footprint_limit
#define dlmalloc_inspect_all   malloc_inspect_all
#define dlindependent_calloc   independent_calloc
#define dlindependent_comalloc independent_comalloc
#define dlbulk_free            bulk_free

#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif /* MALLOC_280_H */
