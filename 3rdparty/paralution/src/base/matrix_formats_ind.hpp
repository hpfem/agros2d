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

#ifndef PARALUTION_MATRIX_FORMATS_IND_HPP_
#define PARALUTION_MATRIX_FORMATS_IND_HPP_

// Matrix indexing
#define DENSE_IND(ai,aj,nrow,ncol) ai + aj*nrow
//#define DENSE_IND(ai,aj,nrow,ncol) aj + ai*nol

#define ELL_IND(row, el , nrow, max_row)  ELL_IND_ROW(row, el , nrow, max_row)
#define ELL_IND_ROW(row, el , nrow, max_row) el*nrow + row
#define ELL_IND_EL(row, el , nrow, max_row) el + max_row*row

#define DIA_IND(row, el, nrow, ndiag) DIA_IND_ROW(row, el, nrow, ndiag)
#define DIA_IND_ROW(row, el, nrow, ndiag) el*nrow + row
#define DIA_IND_EL(row, el, nrow, ndiag) el + ndiag*row


#endif // PARALUTION_MATRIX_FORMATS_IND_HPP_

