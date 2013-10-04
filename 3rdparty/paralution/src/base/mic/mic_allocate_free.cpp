// *************************************************************************
//
//    PARALUTION   www.paralution.com
//
//    Copyright (C) 2012-2013 Dimitar Lukarski
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

#include "mic_allocate_free.hpp"
#include <assert.h>
#include "mic_utils.hpp"

namespace paralution {

template <typename DataType>
void allocate_mic(const int size, DataType **ptr) {

  if (size > 0) {
    assert(*ptr == NULL);

    *ptr = new DataType[size];

    DataType *p = *ptr;

#pragma offload_transfer target(mic:0)              \
  nocopy(p:length(size) MIC_ALLOC MIC_RETAIN)       

#pragma offload_transfer target(mic:0)              \
  in(p:length(size) MIC_REUSE MIC_RETAIN) 

    assert(*ptr != NULL);
  }

}

template <typename DataType>
void free_mic(DataType **ptr) {

  assert(*ptr != NULL);

    DataType *p = *ptr;

#pragma offload_transfer target(mic:0)              \
  out(p:length(0) MIC_REUSE MIC_FREE) 
  
  delete[] *ptr;
  
  *ptr = NULL;

}

template <typename DataType>
void set_to_zero_mic(const int size, DataType *ptr) {

  if (size > 0) {

#pragma offload target(mic:0)                   \
  in(ptr:length(0) MIC_REUSE MIC_RETAIN) 
#pragma omp parallel for 
    for (int i=0; i<size; ++i)
      ptr[i] = DataType(0);

  }

}


template <typename DataType>
void set_to_one_mic(const int size, DataType *ptr) {

  if (size > 0) {

#pragma offload target(mic:0)                   \
  in(ptr:length(0) MIC_REUSE MIC_RETAIN) 
#pragma omp parallel for 
    for (int i=0; i<size; ++i)
      ptr[i] = DataType(1);
    
  }
}



template void allocate_mic<float       >(const int size, float        **ptr);
template void allocate_mic<double      >(const int size, double       **ptr);
template void allocate_mic<int         >(const int size, int          **ptr);
template void allocate_mic<unsigned int>(const int size, unsigned int **ptr);
template void allocate_mic<char        >(const int size, char         **ptr);

template void free_mic<float       >(float        **ptr);
template void free_mic<double      >(double       **ptr);
template void free_mic<int         >(int          **ptr);
template void free_mic<unsigned int>(unsigned int **ptr);
template void free_mic<char        >(char         **ptr);

template void set_to_zero_mic<float       >(const int size, float        *ptr);
template void set_to_zero_mic<double      >(const int size, double       *ptr);
template void set_to_zero_mic<int         >(const int size, int          *ptr);
template void set_to_zero_mic<unsigned int>(const int size, unsigned int *ptr);
template void set_to_zero_mic<char        >(const int size, char         *ptr);


template void set_to_one_mic<float       >(const int size, float        *ptr);
template void set_to_one_mic<double      >(const int size, double       *ptr);
template void set_to_one_mic<int         >(const int size, int          *ptr);
template void set_to_one_mic<unsigned int>(const int size, unsigned int *ptr);
template void set_to_one_mic<char        >(const int size, char         *ptr);


};


