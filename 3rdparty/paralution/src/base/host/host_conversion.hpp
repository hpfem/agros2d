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


#ifndef PARALUTION_HOST_CONVERSION_HPP_
#define PARALUTION_HOST_CONVERSION_HPP_

#include "../matrix_formats.hpp"

namespace paralution {

template <typename ValueType, typename IndexType>
void csr_to_coo(const int omp_threads,
                const IndexType nnz, const IndexType nrow, const IndexType ncol,
                const MatrixCSR<ValueType, IndexType> &src,
                MatrixCOO<ValueType, IndexType> *dst);

template <typename ValueType, typename IndexType>
void csr_to_mcsr(const int omp_threads,
                 const IndexType nnz, const IndexType nrow, const IndexType ncol,
                 const MatrixCSR<ValueType, IndexType> &src,
                 MatrixMCSR<ValueType, IndexType> *dst);

template <typename ValueType, typename IndexType>
void csr_to_dia(const int omp_threads,
                const IndexType nnz, const IndexType nrow, const IndexType ncol,
                const MatrixCSR<ValueType, IndexType> &src,
                MatrixDIA<ValueType, IndexType> *dst, IndexType *nnz_dia);

template <typename ValueType, typename IndexType>
void csr_to_dense(const int omp_threads,
                  const IndexType nnz, const IndexType nrow, const IndexType ncol,
                  const MatrixCSR<ValueType, IndexType> &src,
                  MatrixDENSE<ValueType> *dst);

template <typename ValueType, typename IndexType>
void csr_to_ell(const int omp_threads,
                const IndexType nnz, const IndexType nrow, const IndexType ncol,
                const MatrixCSR<ValueType, IndexType> &src,
                MatrixELL<ValueType, IndexType> *dst, IndexType *nnz_ell);

template <typename ValueType, typename IndexType>
void csr_to_hyb(const int omp_threads,
                const IndexType nnz, const IndexType nrow, const IndexType ncol,
                const MatrixCSR<ValueType, IndexType> &src,
                MatrixHYB<ValueType, IndexType> *dst,
                IndexType *nnz_hyb, IndexType *nnz_ell, IndexType *nnz_coo);

template <typename ValueType, typename IndexType>
void dense_to_csr(const int omp_threads,
                  const IndexType nrow, const IndexType ncol,
                  const MatrixDENSE<ValueType> &src,
                  MatrixCSR<ValueType, IndexType> *dst,
                  IndexType *nnz);

template <typename ValueType, typename IndexType>
void dia_to_csr(const int omp_threads,
                const IndexType nnz, const IndexType nrow, const IndexType ncol,
                const MatrixDIA<ValueType, IndexType> &src,
                MatrixCSR<ValueType, IndexType> *dst, IndexType *nnz_csr);

template <typename ValueType, typename IndexType>
void ell_to_csr(const int omp_threads,
                const IndexType nnz, const IndexType nrow, const IndexType ncol,
                const MatrixELL<ValueType, IndexType> &src,
                MatrixCSR<ValueType, IndexType> *dst, IndexType *nnz_csr);

template <typename ValueType, typename IndexType>
void coo_to_csr(const int omp_threads,
                const IndexType nnz, const IndexType nrow, const IndexType ncol,
                const MatrixCOO<ValueType, IndexType> &src,
                MatrixCSR<ValueType, IndexType> *dst);

template <typename ValueType, typename IndexType>
void mcsr_to_csr(const int omp_threads,
                 const IndexType nnz, const IndexType nrow, const IndexType ncol,
                 const MatrixMCSR<ValueType, IndexType> &src,
                 MatrixCSR<ValueType, IndexType> *dst);

template <typename ValueType, typename IndexType>
void hyb_to_csr(const int omp_threads,
                const IndexType nnz, const IndexType nrow, const IndexType ncol,
                const IndexType nnz_ell, const IndexType nnz_coo,
                const MatrixHYB<ValueType, IndexType> &src,
                MatrixCSR<ValueType, IndexType> *dst, IndexType *nnz_csr);


}

#endif // PARALUTION_HOST_CONVERSION_HPP_
