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


#include "allocate_free.hpp"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "log.hpp"

namespace paralution {

// Check -- if there is no CUDA support 
// pinned memory is disabled
#ifndef SUPPORT_CUDA
#undef PARALUTION_CUDA_PINNED_MEMORY
#endif

//#define MEM_ALIGNMENT 64
//#define LONG_PTR size_t
//#define LONG_PTR long


#ifndef PARALUTION_CUDA_PINNED_MEMORY

template <typename DataType>
void allocate_host(const int size, DataType **ptr) {

  LOG_DEBUG(0, "allocate_host()",
            size);

  if (size > 0) {

    assert(*ptr == NULL);
    
    // C++ style
    *ptr = new DataType[size];

    // C style    
    // *ptr =  (DataType *) malloc(size*sizeof(DataType));   

    // C style (zero-set)
    // 
    // *ptr = (DataType *) calloc(size, sizeof(DatatType));
    
    // Aligned allocation     
    // total size = (size*datatype) + (alignment-1) + (void ptr)
    //    void *non_aligned =  malloc(size*sizeof(DataType)+(MEM_ALIGNMENT-1)+sizeof(void*)); 
    //    assert(non_aligned != NULL);
    
    //    void *aligned = (void*)( ((LONG_PTR)(non_aligned)+MEM_ALIGNMENT+sizeof(void*) ) & ~(MEM_ALIGNMENT-1) );
    
    //    *((void**)aligned-1) = non_aligned;
    
    //    *ptr = (DataType*) aligned;
    
    //  LOG_INFO("A " << *ptr << " " <<  aligned << " " << non_aligned << " "<<  sizeof(DataType) << " " << size);

    assert(*ptr != NULL);

    LOG_DEBUG(0, "allocate_host()",
              *ptr);

  }

}

template <typename DataType>
void free_host(DataType **ptr) {

  LOG_DEBUG(0, "free_host()",
            *ptr);

  assert(*ptr != NULL);

  // C++ style
  delete[] *ptr;
  
  // C style    
  // free(*ptr);

  // Aligned allocation     
  //  free(*((void **)*ptr-1));

  *ptr = NULL;

}

#endif

template <typename DataType>
void set_to_zero_host(const int size, DataType *ptr) {

  LOG_DEBUG(0, "set_to_zero_host()",
            "size =" << size <<
            " ptr=" << ptr);

  if (size > 0) {

    assert(ptr != NULL);
    
    memset(ptr, 0, size*sizeof(DataType));
    
    //  for (int i=0; i<size; ++i)
    //    ptr[i] = DataType(0);
  }

}

#ifndef PARALUTION_CUDA_PINNED_MEMORY

template void allocate_host<float       >(const int size, float        **ptr);
template void allocate_host<double      >(const int size, double       **ptr);
template void allocate_host<int         >(const int size, int          **ptr);
template void allocate_host<unsigned int>(const int size, unsigned int **ptr);
template void allocate_host<char        >(const int size, char         **ptr);

template void free_host<float       >(float        **ptr);
template void free_host<double      >(double       **ptr);
template void free_host<int         >(int          **ptr);
template void free_host<unsigned int>(unsigned int **ptr);
template void free_host<char        >(char         **ptr);

#endif

template void set_to_zero_host<float       >(const int size, float        *ptr);
template void set_to_zero_host<double      >(const int size, double       *ptr);
template void set_to_zero_host<int         >(const int size, int          *ptr);
template void set_to_zero_host<unsigned int>(const int size, unsigned int *ptr);
template void set_to_zero_host<char        >(const int size, char         *ptr);

}

