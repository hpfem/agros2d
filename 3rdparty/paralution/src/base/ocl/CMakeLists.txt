# *************************************************************************
#
#    PARALUTION   www.paralution.com
#
#    Copyright (C) 2012-2013 Dimitar Lukarski
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# *************************************************************************

include_directories(base/ocl)

set(OCL_SOURCES
  ocl_allocate_free.cpp
  backend_ocl.cpp
  ocl_vector.cpp
  ocl_matrix_csr.cpp
  ocl_matrix_coo.cpp
  ocl_matrix_dense.cpp
  ocl_matrix_mcsr.cpp
  ocl_matrix_bcsr.cpp
  ocl_matrix_ell.cpp
  ocl_matrix_dia.cpp
  ocl_matrix_hyb.cpp
)

set(OCL_PUBLIC_HEADERS
  ocl_allocate_free.hpp
  backend_ocl.hpp
  ocl_vector.hpp
  ocl_matrix_csr.hpp
  ocl_matrix_coo.hpp
  ocl_matrix_dense.hpp
  ocl_matrix_mcsr.hpp
  ocl_matrix_bcsr.hpp
  ocl_matrix_ell.hpp
  ocl_matrix_dia.hpp
  ocl_matrix_hyb.hpp
)
