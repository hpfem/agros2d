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


#ifndef PARALUTION_MIC_MATRIX_DIA_KERNEL_HPP_
#define PARALUTION_MIC_MATRIX_DIA_KERNEL_HPP_

#include "../matrix_formats_ind.hpp"

namespace paralution {

template <typename ValueType>
void spmv_dia(const int mic_dev, 
	      const int *offset,  const ValueType *val,
	      const int nrow,
	      const int ndiag,
	      const ValueType *in, ValueType *out) {
  

#pragma offload target(mic:mic_dev)			    \
  in(offset:length(0) MIC_REUSE MIC_RETAIN)	    \
  in(val:length(0) MIC_REUSE MIC_RETAIN)	    \
  in(in:length(0) MIC_REUSE MIC_RETAIN)		    \
  in(out:length(0) MIC_REUSE MIC_RETAIN)	    
#pragma omp parallel for
    for (int i=0; i<nrow; ++i) {
      
      out[i] = ValueType(0.0);
      
      for (int j=0; j<ndiag; ++j) {
        
        int start  = 0;
        int end = nrow;
        int v_offset = 0; 
        
        if ( offset[j] < 0) {
          start -= offset[j];
          v_offset = -start;
        } else {
          end -= offset[j];
          v_offset = offset[j];
        }
        
        if ( (i >= start) && (i < end)) {
          out[i] += val[DIA_IND(i, j, nrow, ndiag)] * in[i+v_offset];
        } else {
          if (i >= end)
            break;
	}
      }
    }

}

template <typename ValueType>
void spmv_add_dia(const int mic_dev, 
		  const int *offset,  const ValueType *val,
		  const int nrow,
		  const int ndiag,
		  const ValueType scalar,
		  const ValueType *in, ValueType *out) {
  

#pragma offload target(mic:mic_dev)			    \
  in(offset:length(0) MIC_REUSE MIC_RETAIN)	    \
  in(val:length(0) MIC_REUSE MIC_RETAIN)	    \
  in(in:length(0) MIC_REUSE MIC_RETAIN)		    \
  in(out:length(0) MIC_REUSE MIC_RETAIN)	    
#pragma omp parallel for
    for (int i=0; i<nrow; ++i) {
      
      for (int j=0; j<ndiag; ++j) {
        
        int start  = 0;
        int end = nrow;
        int v_offset = 0; 
        
        if ( offset[j] < 0) {
          start -= offset[j];
          v_offset = -start;
        } else {
          end -= offset[j];
          v_offset = offset[j];
        }
        
        if ( (i >= start) && (i < end)) {
          out[i] += scalar*val[DIA_IND(i, j, nrow, ndiag)] * in[i+v_offset];
        } else {
          if (i >= end)
            break;
	}
      }
    }

}

}

#endif // PARALUTION_BASE_MATRIX_DIA_KERNEL_HPP_
