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


#ifndef PARALUTION_MATRIX_FORMATS_HPP_
#define PARALUTION_MATRIX_FORMATS_HPP_

#include <string>

namespace paralution {

/// Matrix Names
const std::string _matrix_format_names [8] = {"DENSE", 
                                              "CSR", 
                                              "MCSR",
                                              "BCSR",
                                              "COO", 
                                              "DIA", 
                                              "ELL", 
                                              "HYB"
};

/// Matrix Enumeration
enum _matrix_format {DENSE = 0, 
                     CSR   = 1, 
                     MCSR  = 2,
                     BCSR  = 3,
                     COO   = 4, 
                     DIA   = 5, 
                     ELL   = 6, 
                     HYB   = 7};
  

/// Sparse Matrix -
/// Sparse Compressed Row Format
template <typename ValueType, typename IndexType>
struct MatrixCSR {
  /// Row offsets (row ptr)
  IndexType *row_offset;

  /// Column index
  IndexType *col;

  /// Values
  ValueType *val;
};

/// Sparse Matrix -
/// Modified Sparse Compressed Row Format
template <typename ValueType, typename IndexType>
struct MatrixMCSR {
  /// Row offsets (row ptr)
  IndexType *row_offset;

  /// Column index
  IndexType *col;

  /// Values
  ValueType *val;

  /// Diagonal elements
  ValueType *diag;
};

template <typename ValueType, typename IndexType>
struct MatrixBCSR {
};

/// Sparse Matrix -
/// Coordinate Format
template <typename ValueType, typename IndexType>
struct MatrixCOO {
  /// Row index
  IndexType *row;

  /// Column index
  IndexType *col;

  // Values
  ValueType *val;
};

/// Sparse Matrix -
/// Diagonal Format (see DIA_IND for indexing)
template <typename ValueType, typename IndexType, typename Index = IndexType>
struct MatrixDIA {
  /// Number of diagonal
  Index num_diag;

  /// Offset with respect to the main diagonal
  IndexType *offset;

  /// Values
  ValueType *val;
};

/// Sparse Matrix -
/// ELL Format (see ELL_IND for indexing)
template <typename ValueType, typename IndexType, typename Index = IndexType>
struct MatrixELL {
  /// Maximal elements per row
  Index max_row;

  /// Column index
  IndexType *col;

  /// Values
  ValueType *val;
};

/// Sparse Matrix -
/// Contains ELL and COO Matrices
template <typename ValueType, typename IndexType, typename Index = IndexType>
struct MatrixHYB {
  MatrixELL<ValueType, IndexType, Index> ELL;
  MatrixCOO<ValueType, IndexType> COO;
};

/// Dense Matrix (see DENSE_IND for indexing)
template <typename ValueType>
struct MatrixDENSE {
  /// Values
  ValueType *val;
};




}

#endif // PARALUTION_MATRIX_FORMATS_HPP_

