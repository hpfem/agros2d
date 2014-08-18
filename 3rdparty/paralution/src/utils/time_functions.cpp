// *************************************************************************
//
//    PARALUTION   www.paralution.com
//
//    Copyright (C) 2012-2014 Dimitar Lukarski
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************



// PARALUTION version 0.7.0 


#include <stdlib.h>

#include "time_functions.hpp"
#include "../base/backend_manager.hpp"


// the default OS is Linux

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN64) && !defined(__CYGWIN__)
// Windows
#include <windows.h>

#else
// Linux
#include <sys/time.h>

#endif





namespace paralution {

double paralution_time(void) {

  double the_time_now = 0.0;

  _paralution_sync();
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN64) && !defined(__CYGWIN__)
  // Windows
  
  LARGE_INTEGER now;
  LARGE_INTEGER freq;
  
  QueryPerformanceCounter(&now);
  QueryPerformanceFrequency(&freq);
  
  the_time_now = (now.QuadPart*1000000.0) / static_cast<float>(freq.QuadPart);

#else
// Linux

  struct timeval now;

  gettimeofday(&now, NULL);
  the_time_now = now.tv_sec*1000000.0+(now.tv_usec);

#endif

  return the_time_now;

}

}

