// This file is part of Hermes2D.
//
// Hermes2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Hermes2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hermes2D.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __HERMES2D_COMMON_H
#define __HERMES2D_COMMON_H

#include "config.h"

// common headers
#include <stdexcept>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h> //allows to use offsetof
#include <string.h>
#include <cstdarg>
#include <assert.h>
#include <pthread.h>
#include <math.h>

#include <float.h>

// STL stuff
#include <vector>
#include <string>
#include <set>
#include <queue>
#include <sstream>
#include <fstream>

// platform compatibility stuff
#include "compat.h"

// others
#include <Judy.h>
#include "auto_local_array.h"

enum // node types
{
  TYPE_VERTEX = 0,
  TYPE_EDGE = 1
};

enum // element modes
{
  MODE_TRIANGLE = 0,
  MODE_QUAD = 1
};


const int ANY = -1234;

// how many bits the order number takes
const int order_bits = 5;
const int order_mask = (1 << order_bits) - 1;


// macros for combining quad horizontal and vertical orders
#define make_quad_order(h_order, v_order) (((v_order) << order_bits) + (h_order))
#define get_h_order(order) ((order) & order_mask)
#define get_v_order(order) ((order) >> order_bits)
extern HERMES2D_API const std::string get_quad_order_str(const int quad_order); ///< Returns string representation of the quad order: used for debugging purposses.

#ifdef COMPLEX
  #include <complex>
  typedef std::complex<double> cplx;
  typedef cplx scalar;
  typedef cplx complex2[2];
#else
  typedef double scalar;
#endif


typedef int int2[2];
typedef int int3[3];
typedef int int4[4];
typedef int int5[5];

typedef double double2[2];
typedef double double3[3];
typedef double double4[4];
typedef double double2x2[2][2];
typedef double double3x2[3][2];

typedef scalar scalar2[2];
typedef scalar scalar3[3];


inline int sqr(int x) { return x*x; }
inline double sqr(double x) { return x*x; }
#ifdef COMPLEX
inline double sqr(cplx x) { return std::norm(x); }
#endif

inline double magn(double x) { return fabs(x); }
#ifdef COMPLEX
inline double magn(cplx x) { return std::abs(x); }
#endif

inline double conj(double a) {  return a; }
#ifdef COMPLEX
inline cplx conj(cplx a) { return std::conj(a); }
#endif

#define is_int(x) ((int) (x) == (x))


extern HERMES2D_API bool verbose_mode;
extern HERMES2D_API bool info_mode;
extern HERMES2D_API bool warn_integration;
extern HERMES2D_API void __error_fn(const char* fname, const char* msg, ...);
extern HERMES2D_API void __warn_fn(const char* fname, const char* msg, ...);
extern HERMES2D_API void __info_fn(const char* msg, ...);
extern HERMES2D_API void __verbose_fn(const char* msg, ...);


#ifdef _WIN32
  #ifdef __MINGW32__
    // MinGW compiler
    #ifndef __ASSERT_FUNCTION
      #define __ASSERT_FUNCTION __func__
    #endif
  #else
    // other Win32 compiler
    #ifndef __ASSERT_FUNCTION
      #define __ASSERT_FUNCTION __FUNCTION__
    #endif
  #endif
#else
  // Linux
  #ifdef NDEBUG
    #define __ASSERT_FUNCTION __PRETTY_FUNCTION__
  #else
    #ifndef __ASSERT_FUNCTION
      #define __ASSERT_FUNCTION "<unknown function>"
    #endif
  #endif
#endif


#define error(...) __error_fn(__ASSERT_FUNCTION, __VA_ARGS__)
#define warn(...) __warn_fn(__ASSERT_FUNCTION, __VA_ARGS__)
#define info(...) __info_fn(__VA_ARGS__)
#define verbose(...) __verbose_fn(__VA_ARGS__)

/* logging macros */
#ifdef _DEBUG
extern HERMES2D_API void debug_log(const char* msg, ...); ///< Logs output to an external file. Ignored if not debug.
extern HERMES2D_API bool __debug_assert(const bool cond, const char* file, const int line, const char* msg, ...); ///< Checks the condition. If failed, it logs output to an external file. Returns true if abort is required. Ignored if not debug.
# define debug_assert(__cond, ...) if (__debug_assert(__cond, __FILE__, __LINE__, __VA_ARGS__)) abort()
#else
# define debug_log(...)
# define debug_assert(...)
#endif
extern HERMES2D_API void log_msg(const char* msg, ...); ///< Logs output to an external file.
extern HERMES2D_API bool __assert_msg(const bool cond, const char* file, const int line, const char* msg, ...); ///< Checks the condition. If failed, it logs output to an external file. Returns true if abort is required.
#define assert_msg(__cond, ...) if (__assert_msg(__cond, __FILE__, __LINE__, __VA_ARGS__)) abort()

void __hermes2d_fwrite(const void* ptr, size_t size, size_t nitems, FILE* stream, const char* fname);
void __hermes2d_fread(void* ptr, size_t size, size_t nitems, FILE* stream, const char* fname);

#define hermes2d_fwrite(ptr, size, nitems, stream) \
      __hermes2d_fwrite((ptr), (size), (nitems), (stream), __ASSERT_FUNCTION)

#define hermes2d_fread(ptr, size, nitems, stream) \
      __hermes2d_fread((ptr), (size), (nitems), (stream), __ASSERT_FUNCTION)


extern HERMES2D_API void begin_time();
extern HERMES2D_API double cur_time();
extern HERMES2D_API double end_time();


void throw_exception(char *text);

#endif
