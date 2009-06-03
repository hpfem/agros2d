// This file is part of Hermes2D.
//
// Copyright 2005-2008 Jakub Cerveny <jakub.cerveny@gmail.com>
// Copyright 2005-2008 Lenka Dubcova <dubcova@gmail.com>
// Copyright 2005-2008 Pavel Solin <solin@unr.edu>
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

// $Id: common.h 1103 2008-11-04 10:20:01Z lenka $

#ifndef __HERMES2D_COMMON_H
#define __HERMES2D_COMMON_H

// common headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdarg>
#include <assert.h>
#include <pthread.h>
#include <math.h>

// STL stuff
#include <vector>
#include <string>
#include <set>

#include <Judy.h>


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


// min/max macros for Win32
#ifdef _MSC_VER
#include <minmax.h>
#endif


// how many bits the order number takes
const int order_bits = 5;
const int order_mask = (1 << order_bits) - 1;


// macros for combining quad horizontal and vertical orders
#define make_quad_order(h_order, v_order) (((v_order) << order_bits) + (h_order))
#define get_h_order(order) ((order) & order_mask)
#define get_v_order(order) ((order) >> order_bits)


#ifdef COMPLEX
  #include <complex>
  typedef std::complex<double> complex;
  typedef complex scalar;
  typedef complex complex2[2];
#else
  typedef double scalar;
#endif


typedef int int2[2];
typedef int int3[3];
typedef int int4[4];
typedef int int5[5];

typedef unsigned long long uint64;

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
inline double sqr(complex x) { return std::norm(x); }
#endif

inline double magn(double x) { return fabs(x); }
#ifdef COMPLEX
inline double magn(complex x) { return std::abs(x); }
#endif

#define is_int(x) ((int) (x) == (x))


extern bool verbose_mode;
extern bool warn_integration;
void __error_fn(const char* fname, const char* msg, ...);
void __warn_fn(const char* fname, const char* msg, ...);
void __info_fn(const char* msg, ...);
void __verbose_fn(const char* msg, ...);


#ifdef _WIN32
  #ifdef __MINGW32__
    // MinGW compiler
    #ifndef __ASSERT_FUNCTION
      #define __ASSERT_FUNCTION __func__
    #endif
  #else
    // other Win32 compiler
    #ifndef __ASSERT_FUNCTION
      #define __ASSERT_FUNCTION "<unknown function>"
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


void __hermes2d_fwrite(const void* ptr, size_t size, size_t nitems, FILE* stream, const char* fname);
void __hermes2d_fread(void* ptr, size_t size, size_t nitems, FILE* stream, const char* fname);

#define hermes2d_fwrite(ptr, size, nitems, stream) \
      __hermes2d_fwrite((ptr), (size), (nitems), (stream), __ASSERT_FUNCTION)

#define hermes2d_fread(ptr, size, nitems, stream) \
      __hermes2d_fread((ptr), (size), (nitems), (stream), __ASSERT_FUNCTION)


void begin_time();
double cur_time();
double end_time();


#endif
