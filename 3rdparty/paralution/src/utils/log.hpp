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


#ifndef PARALUTION_UTILS_LOG_HPP_
#define PARALUTION_UTILS_LOG_HPP_

#include <iostream>
#include <stdlib.h>

// Uncomment to define verbose level
#define VERBOSE_LEVEL 2

// Uncomment for debug mode
// #define DEBUG_MODE



// Do not edit
#ifdef DEBUG_MODE

#undef VERBOSE_LEVEL
#define VERBOSE_LEVEL 10

#endif


// LOG INFO
#define LOG_INFO(stream) {            \
    std::cout << stream << std::endl; \
  }


// LOG ERROR
#define FATAL_ERROR(file, line) {                                \
  LOG_INFO("Fatal error - the program will be terminated ");     \
  LOG_INFO("File: " << file << "; line: " << line);              \
  exit(1);                                                       \
}


// LOG VERBOSE
#ifdef VERBOSE_LEVEL

#define LOG_VERBOSE_INFO(level, stream) {                       \
    if (level <= VERBOSE_LEVEL)                                 \
      std::cout << stream << std::endl;                         \
  }

#else

#define LOG_VERBOSE_INFO(level, stream) ;

#endif


// LOG DEBUG
#ifdef DEBUG_MODE

#define LOG_DEBUG(obj, fct, stream) {                            \
    std::cout << "# Obj addr: " << obj                           \
              << "; fct: " << fct                                \
              << " " << stream << std::endl;             \
  }

#else

#define LOG_DEBUG(obj, fct, stream) ;

#endif



#endif // PARALUTION_UTILS_LOG_HPP_
