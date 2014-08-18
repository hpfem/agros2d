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


#ifndef PARALUTION_MIC_MIC_UTILS_HPP_
#define PARALUTION_MIC_MIC_UTILS_HPP_

#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "backend_mic.hpp"
#include "mic_allocate_free.hpp"

#include <stdlib.h>

#define MIC_ALLOC   alloc_if(1)
#define MIC_FREE    free_if(1)
#define MIC_RETAIN  free_if(0)
#define MIC_REUSE   alloc_if(0)

namespace paralution {

template <typename ValueType>
void copy_to_mic(const int mic_dev, const ValueType *src, ValueType *dst, const int size) {

#pragma offload target(mic:mic_dev)	    \
  in(dst:length(0) MIC_REUSE MIC_RETAIN)    \
  in(src:length(size))
#pragma omp parallel for 
	for (int i=0; i<size; ++i) 
        dst[i] = src[i];

}

template <typename ValueType>
void copy_to_host(const int mic_dev, const ValueType *src, ValueType *dst, const int size) {


#pragma offload target(mic:mic_dev)	    \
  in(src:length(0) MIC_REUSE MIC_RETAIN)    \
  out(dst:length(size))
#pragma omp parallel for 
	for (int i=0; i<size; ++i) 
        dst[i] = src[i];

}

template <typename ValueType>
void copy_mic_mic(const int mic_dev, const ValueType *src, ValueType *dst, const int size) {

#pragma offload target(mic:mic_dev)			    \
  in(src:length(0) MIC_REUSE MIC_RETAIN)		    \
  in(dst:length(0) MIC_REUSE MIC_RETAIN) 
#pragma omp parallel for 
  for (int i=0; i<size; ++i)
    dst[i] = src[i];


}


};


#endif // PARALUTION_MIC_MIC_UTILS_HPP_
