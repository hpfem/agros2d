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

#ifndef PARALUTION_MIC_VECTOR_KERNEL_HPP_
#define PARALUTION_MIC_VECTOR_KERNEL_HPP_

namespace paralution {

template <typename ValueType>
void dot(const ValueType *vec1, const ValueType *vec2, const int size, ValueType &d) {
  
  ValueType tmp = ValueType(0.0);

#pragma offload target(mic:0)			    \
  in(vec1:length(0) MIC_REUSE MIC_RETAIN)	    \
  in(vec2:length(0) MIC_REUSE MIC_RETAIN)	    
#pragma omp parallel for reduction(+:tmp)
  for (int i=0; i<size; ++i)
    tmp += vec1[i]*vec2[i];

  d = tmp;

}

template <typename ValueType>
void asum(const ValueType *vec, const int size, ValueType &d) {
  
  ValueType tmp = ValueType(0.0);

#pragma offload target(mic:0)			    \
  in(vec:length(0) MIC_REUSE MIC_RETAIN)	    
#pragma omp parallel for reduction(+:tmp)
  for (int i=0; i<size; ++i)
    tmp += fabs(vec[i]);

  d = tmp;

}


template <typename ValueType>
void amax(const ValueType *vec, const int size, ValueType &d) {
  
  ValueType tmp = ValueType(0.0);

#pragma offload target(mic:0)			    \
  in(vec:length(0) MIC_REUSE MIC_RETAIN)	    
#pragma omp parallel for
  for (int i=0; i<size; ++i) {
    ValueType val = fabs(vec[i]);
    if (val > tmp)
#pragma omp critical
      {
	if (val > tmp)
	  tmp = val;
      }
    }

  d = tmp;

}


template <typename ValueType>
void norm(const ValueType *vec, const int size, ValueType &d) {
  
  ValueType tmp = ValueType(0.0);

#pragma offload target(mic:0)			    \
  in(vec:length(0) MIC_REUSE MIC_RETAIN)	    
#pragma omp parallel for reduction(+:tmp)
  for (int i=0; i<size; ++i)
    tmp += vec[i]*vec[i];

  d = sqrt(tmp);

}

template <typename ValueType>
void reduce(const ValueType *vec, const int size, ValueType &d) {
  
  ValueType tmp = ValueType(0.0);

#pragma offload target(mic:0)			    \
  in(vec:length(0) MIC_REUSE MIC_RETAIN)	    
#pragma omp parallel for reduction(+:tmp)
  for (int i=0; i<size; ++i)
    tmp += vec[i];

  d = tmp;

}


template <typename ValueType>
void scaleadd(const ValueType *vec1, const ValueType alpha, const int size, ValueType *vec2) {

#pragma offload target(mic:0)				    \
  in(vec1:length(0) MIC_REUSE MIC_RETAIN)		    \
  in(vec2:length(0) MIC_REUSE MIC_RETAIN) 
#pragma omp parallel for 
  for (int i=0; i<size; ++i)
    vec2[i] = alpha*vec2[i] + vec1[i];

}

template <typename ValueType>
void addscale(const ValueType *vec1, const ValueType alpha, const int size, ValueType *vec2) {

#pragma offload target(mic:0)				    \
  in(vec1:length(0) MIC_REUSE MIC_RETAIN)		    \
  in(vec2:length(0) MIC_REUSE MIC_RETAIN) 
#pragma omp parallel for 
  for (int i=0; i<size; ++i)
    vec2[i] = vec2[i] + alpha*vec1[i];

}


template <typename ValueType>
void scaleaddscale(const ValueType *vec1, const ValueType alpha, const ValueType beta, 
		   const int size, ValueType *vec2) {

#pragma offload target(mic:0)				    \
  in(vec1:length(0) MIC_REUSE MIC_RETAIN)		    \
  in(vec2:length(0) MIC_REUSE MIC_RETAIN) 
#pragma omp parallel for 
  for (int i=0; i<size; ++i)
    vec2[i] = alpha*vec2[i] + beta*vec1[i];

}

template <typename ValueType>
void scaleadd2(const ValueType *vec1, const ValueType *vec2, 
	       const ValueType alpha, const ValueType beta, const ValueType gamma,
	       const int size, ValueType *vec3) {

#pragma offload target(mic:0)				    \
  in(vec1:length(0) MIC_REUSE MIC_RETAIN)		    \
  in(vec2:length(0) MIC_REUSE MIC_RETAIN)		    \
  in(vec3:length(0) MIC_REUSE MIC_RETAIN) 
#pragma omp parallel for 
  for (int i=0; i<size; ++i)
    vec3[i] = alpha*vec3[i] + beta*vec1[i] + gamma*vec2[i];

}

template <typename ValueType>
void scale(const ValueType alpha, const int size, ValueType *vec) {

#pragma offload target(mic:0)				    \
  in(vec:length(0) MIC_REUSE MIC_RETAIN)		    
#pragma omp parallel for 
  for (int i=0; i<size; ++i)
    vec[i] *= alpha;
  
}

template <typename ValueType>
void pointwisemult(const ValueType *vec1, const int size, ValueType *vec2) {

#pragma offload target(mic:0)				    \
  in(vec1:length(0) MIC_REUSE MIC_RETAIN)		    \
  in(vec2:length(0) MIC_REUSE MIC_RETAIN) 
#pragma omp parallel for 
  for (int i=0; i<size; ++i)
    vec2[i] = vec2[i]*vec1[i];

}

template <typename ValueType>
void pointwisemult2(const ValueType *vec1,  const ValueType *vec2, 
		    const int size, ValueType *vec3) {

#pragma offload target(mic:0)				    \
  in(vec1:length(0) MIC_REUSE MIC_RETAIN)		    \
  in(vec2:length(0) MIC_REUSE MIC_RETAIN)		    \
  in(vec3:length(0) MIC_REUSE MIC_RETAIN)		    
#pragma omp parallel for 
  for (int i=0; i<size; ++i)
    vec3[i] = vec2[i]*vec1[i];

}

template <typename ValueType>
void permute(const int *perm, const ValueType *in, 
	     const int size, ValueType *out) {
  
#pragma offload target(mic:0)				    \
  in(perm:length(0) MIC_REUSE MIC_RETAIN)		    \
  in(in:length(0) MIC_REUSE MIC_RETAIN)			    \
  in(out:length(0) MIC_REUSE MIC_RETAIN)		    
#pragma omp parallel for 
  for (int i=0; i<size; ++i)
    out[ perm[i] ] = in[i];
  
}

template <typename ValueType>
void permuteback(const int *perm, const ValueType *in, 
		 const int size, ValueType *out) {
  
#pragma offload target(mic:0)				    \
  in(perm:length(0) MIC_REUSE MIC_RETAIN)		    \
  in(in:length(0) MIC_REUSE MIC_RETAIN)			    \
  in(out:length(0) MIC_REUSE MIC_RETAIN)		    
#pragma omp parallel for 
  for (int i=0; i<size; ++i)
    out[i] = in[ perm[i] ];
  
}

}

#endif // PARALUTION_BASE_VECTOR_KERNEL_HPP_
