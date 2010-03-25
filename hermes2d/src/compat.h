/*
   This header file contains platform compatibility layer.

   It is included from common.h, so it is automatically included in all hermes
   sources. The implementation of the functions in this file is in the
   src/compat directory.
*/

#ifndef __HERMES2D_COMPAT_H
#define __HERMES2D_COMPAT_H


#ifndef HAVE_FMEMOPEN
/// Implementation of GNU fmemopen. Intended to be used if the current platform does not support it.
FILE *fmemopen (void *buf, size_t size, const char *opentype);
#endif

# define HERMES2D_API
# define HERMES2D_API_USED_TEMPLATE(__implementation)
# define HERMES2D_API_USED_STL_VECTOR(__type)

//C99 functions
#include "compat/c99_functions.h"

#endif
