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

#ifndef __H2D_COMMON_H
#define __H2D_COMMON_H

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
#include <time.h>

#include <float.h>

// STL stuff
#include <vector>
#include <string>
#include <map>
#include <set>
#include <queue>
#include <sstream>
#include <fstream>

// platform compatibility stuff
#include "compat.h"

// others
#include <Judy.h>
#include "auto_local_array.h"
#include "common_time_period.h"

// Enabling second derivatives in weak forms. Turned off by default. Second
// derivatives are employed, among others, by stabilization methods for
// transport equations. For usage see the example linear-convection-diffusion.
#define H2D_SECOND_DERIVATIVES_ENABLED

enum // node types
{
  H2D_TYPE_VERTEX = 0,
  H2D_TYPE_EDGE = 1
};

enum // element modes
{
  H2D_MODE_TRIANGLE = 0,
  H2D_MODE_QUAD = 1
};


const int H2D_ANY = -1234;

// how many bits the order number takes
const int H2D_ORDER_BITS = 5;
const int H2D_ORDER_MASK = (1 << H2D_ORDER_BITS) - 1;


// macros for combining quad horizontal and vertical orders
#define H2D_MAKE_QUAD_ORDER(h_order, v_order) (((v_order) << H2D_ORDER_BITS) + (h_order))
#define H2D_GET_H_ORDER(order) ((order) & H2D_ORDER_MASK)
#define H2D_GET_V_ORDER(order) ((order) >> H2D_ORDER_BITS)
extern H2D_API const std::string get_quad_order_str(const int quad_order); ///< Returns string representation of the quad order: used for debugging purposses.

#ifdef H2D_COMPLEX
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
#ifdef H2D_COMPLEX
inline double sqr(cplx x) { return std::norm(x); }
#endif

inline double magn(double x) { return fabs(x); }
#ifdef H2D_COMPLEX
inline double magn(cplx x) { return std::abs(x); }
#endif

inline double conj(double a) {  return a; }
#ifdef H2D_COMPLEX
inline cplx conj(cplx a) { return std::conj(a); }
#endif

#define H2D_IS_INT(x) ((int) (x) == (x))

/* basic logging functions */
struct H2D_API __h2d_log_info { ///< Info of a log record. Used for output log function.
  const char code;
  const char* log_file;
  const char* src_function;
  const char* src_file;
  const int src_line;
  __h2d_log_info(const char code, const char* log_file, const char* src_function, const char* src_file, const int src_line)
    : code(code), log_file(log_file), src_function(src_function), src_file(src_file), src_line(src_line) {};
};
#define __LOG_INFO(__event) __h2d_log_info(__event, H2D_LOG_FILE, __CURRENT_FUNCTION, __FILE__, __LINE__)
extern H2D_API void __h2d_exit_if(bool cond, int code = -1); ///< Exits with the given code if condition if met.
extern H2D_API bool __h2d_log_message_if(bool cond, const __h2d_log_info& info, const char* msg, ...); ///< Logs a message if condition is met.

/* log file */
#ifdef H2D_REPORT_NO_FILE
#  define H2D_LOG_FILE NULL
#else
# ifdef H2D_REPORT_FILE
#  define H2D_LOG_FILE H2D_REPORT_FILE
# else
#  ifndef H2D_TEST
#    define H2D_LOG_FILE "hermes2d.log" // default filename for a library
#  else
#    define H2D_LOG_FILE "test.log" // default filename for a library test
#  endif
# endif
#endif

/* function name */
#ifdef _WIN32 //Win32
# ifdef __MINGW32__ //MinGW
#   define __CURRENT_FUNCTION __func__
# else //MSVC and other compilers
#   define __CURRENT_FUNCTION __FUNCTION__
# endif
#else //Linux and Mac
# define __CURRENT_FUNCTION __PRETTY_FUNCTION__
#endif

/* event codes */
#define H2D_EC_ERROR 'E' /* errors */
#define H2D_EC_ASSERT 'X' /* asserts */
#define H2D_EC_WARNING 'W' /* warnings */
#define H2D_EC_INFO 'I' /* info about results */
#define H2D_EC_VERBOSE 'V' /* more details for info */
#define H2D_EC_TRACE 'R' /* execution tracing */
#define H2D_EC_TIME 'T' /* time measurements */
#define H2D_EC_DEBUG 'D' /* general debugging messages */

/* error and assert macros */
#define error(...) __h2d_exit_if(__h2d_log_message_if(true, __LOG_INFO(H2D_EC_ERROR), __VA_ARGS__))
#define error_if(__cond, ...) __h2d_exit_if(__h2d_log_message_if(__cond, __LOG_INFO(H2D_EC_ERROR), __VA_ARGS__))
#ifndef NDEBUG
# define assert_msg(__cond, ...) assert(!__h2d_log_message_if(!(__cond), __LOG_INFO(H2D_EC_ASSERT), __VA_ARGS__))
#else
# define assert_msg(__cond, ...)
#endif

/* reporting macros */
#ifdef H2D_REPORT_ALL
# define H2D_REPORT_WARNING
# undef HERMED2D_REPORT_NO_INTR_WARNING
# define H2D_REPORT_INFO
# define H2D_REPORT_VERBOSE
# define H2D_REPORT_TRACE
# define H2D_REPORT_TIME
#endif
#ifdef H2D_REPORT_RUNTIME_CONTROL
# define H2D_RCTR(__var) __var /* reports will be controled also by runtime report control variables */
extern H2D_API bool __h2d_report_warn;
extern H2D_API bool __h2d_report_warn_intr;
extern H2D_API bool __h2d_report_info;
extern H2D_API bool __h2d_report_verbose;
extern H2D_API bool __h2d_report_trace;
extern H2D_API bool __h2d_report_time;
extern H2D_API bool __h2d_report_debug;
#else
# define H2D_RCTR(__var) true /* reports will be controled strictly by preprocessor directives */
#endif

#if defined(H2D_REPORT_WARNING) || defined(H2D_REPORT_RUNTIME_CONTROL)
# define warn(...) __h2d_log_message_if(true && H2D_RCTR(__h2d_report_warn), __LOG_INFO(H2D_EC_WARNING), __VA_ARGS__)
# define warn_if(__cond, ...) __h2d_log_message_if((__cond) && H2D_RCTR(__h2d_report_warn), __LOG_INFO(H2D_EC_WARNING), __VA_ARGS__)
#else
# define warn(...)
# define warn_if(__cond, ...)
#endif
#if defined(H2D_REPORT_INTR_WARNING) || defined(H2D_REPORT_RUNTIME_CONTROL)
# define warn_intr(...) __h2d_log_message_if(H2D_RCTR(__h2d_report_warn_intr), __LOG_INFO(H2D_EC_WARNING), __VA_ARGS__)
#else
# define warn_intr(...)
#endif
#if defined(H2D_REPORT_INFO) || defined(H2D_REPORT_RUNTIME_CONTROL)
# define info(...) __h2d_log_message_if(true  && H2D_RCTR(__h2d_report_info), __LOG_INFO(H2D_EC_INFO), __VA_ARGS__)
# define info_if(__cond, ...) __h2d_log_message_if((__cond) && H2D_RCTR(__h2d_report_warn), __LOG_INFO(H2D_EC_INFO), __VA_ARGS__)
#else
# define info(...)
# define info_if(__cond, ...)
#endif
#if defined(H2D_REPORT_VERBOSE) || defined(H2D_REPORT_RUNTIME_CONTROL)
# define verbose(...) __h2d_log_message_if(true && H2D_RCTR(__h2d_report_verbose), __LOG_INFO(H2D_EC_VERBOSE), __VA_ARGS__)
#else
# define verbose(...)
#endif
#if defined(H2D_REPORT_TRACE) || defined(H2D_REPORT_RUNTIME_CONTROL)
# define trace(...) __h2d_log_message_if(true && H2D_RCTR(__h2d_report_trace), __LOG_INFO(H2D_EC_TRACE), __VA_ARGS__)
#else
# define trace(...)
#endif
#if defined(H2D_REPORT_TIME) || defined(H2D_REPORT_RUNTIME_CONTROL)
# define report_time(...) __h2d_log_message_if(true && H2D_RCTR(__h2d_report_time), __LOG_INFO(H2D_EC_TIME), __VA_ARGS__)
#else
# define report_time(...)
#endif
#if defined(_DEBUG) || !defined(NDEBUG) || defined(H2D_REPORT_RUNTIME_CONTROL)
# define debug_log(...) __h2d_log_message_if(true && H2D_RCTR(__h2d_report_debug), __LOG_INFO(H2D_EC_DEBUG), __VA_ARGS__)
#else
# define debug_log(...)
#endif

/* file operations */
void __hermes2d_fwrite(const void* ptr, size_t size, size_t nitems, FILE* stream, const __h2d_log_info& err_info);
void __hermes2d_fread(void* ptr, size_t size, size_t nitems, FILE* stream, const __h2d_log_info& err_info);

#define hermes2d_fwrite(ptr, size, nitems, stream) \
      __hermes2d_fwrite((ptr), (size), (nitems), (stream), __LOG_INFO(H2D_EC_ERROR))

#define hermes2d_fread(ptr, size, nitems, stream) \
      __hermes2d_fread((ptr), (size), (nitems), (stream), __LOG_INFO(H2D_EC_ERROR))

/* python support */
extern H2D_API void throw_exception(char *text);

#endif

