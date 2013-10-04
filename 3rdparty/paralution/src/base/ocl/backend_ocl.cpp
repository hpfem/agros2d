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

#include "../backend_manager.hpp"
#include "backend_ocl.hpp" 
#include "../../utils/log.hpp"
#include "ocl_utils.hpp"
#include "../../utils/HardwareParameters.hpp"
#include "../base_vector.hpp"
#include "../base_matrix.hpp"

#include "ocl_vector.hpp"
#include "ocl_matrix_csr.hpp"
#include "ocl_matrix_coo.hpp"
#include "ocl_matrix_mcsr.hpp"
#include "ocl_matrix_bcsr.hpp"
#include "ocl_matrix_hyb.hpp"
#include "ocl_matrix_dia.hpp"
#include "ocl_matrix_ell.hpp"
#include "ocl_matrix_dense.hpp"

#include "ocl_kernels_general.hpp"
#include "ocl_kernels_vector.hpp"
#include "ocl_kernels_csr.hpp"
#include "ocl_kernels_bcsr.hpp"
#include "ocl_kernels_mcsr.hpp"
#include "ocl_kernels_dense.hpp"
#include "ocl_kernels_ell.hpp"
#include "ocl_kernels_dia.hpp"
#include "ocl_kernels_coo.hpp"
#include "ocl_kernels_hyb.hpp"

#include <assert.h>
#include <sstream>

namespace paralution {

// TODO we need some smarter way to handle kernels
// Sets all OpenCL kernels for float, double and integer
void paralution_set_ocl_kernels(void) {

  cl_int err;

  // Initialize kernel array
  for (int i = 0; i < 100; ++i)
    (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[i] = NULL;

  // Single Precision ocl_kernels_general_sp.cl
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[1]  = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_set_to", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[3]  = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_reverse_index", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[5]  = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_buffer_addscalar", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[74] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_red_recurse", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[76] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_red_partial_sum", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[78] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_red_extrapolate", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Single Precision ocl_kernels_vector_sp.cl
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[8]  = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_scaleadd", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[10] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_scaleaddscale", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[12] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_scaleadd2", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[14] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_pointwisemult", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[16] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_pointwisemult2", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[18] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_copy_offset_from", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[20] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_permute", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[22] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_permute_backward", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[24] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_dot", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[26] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_axpy", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[52] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_scale", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[88] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_reduce", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[100] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_asum", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[102] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_amax", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Single Precision ocl_kernels_csr_sp.cl
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[28] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_spmv_scalar", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[30] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_add_spmv_scalar", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[32] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_scale_diagonal", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[34] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_scale_offdiagonal", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[36] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_add_diagonal", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[38] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_add_offdiagonal", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[40] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_extract_diag", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[42] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_extract_inv_diag", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[44] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_extract_submatrix_row_nnz", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[46] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_extract_submatrix_copy", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[48] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_diagmatmult", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[50] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_add_csr_same_struct", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[80] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_permute_rows", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[82] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_permute_cols", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[84] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_calc_row_nnz", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[86] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_csr_permute_row_nnz", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Single Precision ocl_kernels_mcsr.cl
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[54] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_mcsr_spmv_scalar", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[56] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_mcsr_add_spmv_scalar", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Single Precision ocl_kernels_ell.cl
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[58] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_ell_spmv", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[60] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_ell_add_spmv", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[92] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_ell_max_row", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[94] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_ell_csr_to_ell", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Single Precision ocl_kernels_dia.cl
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[62] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_dia_spmv", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[64] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_dia_add_spmv", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Single Precision ocl_kernels_coo.cl
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[66] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_coo_permute", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[68] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_coo_spmv_flat", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[70] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_coo_spmv_reduce_update", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[72] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp, "kernel_coo_spmv_serial", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Double Precision ocl_kernels_general_dp.cl
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[2]  = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_set_to", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[4]  = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_reverse_index", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[6]  = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_buffer_addscalar", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[75] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_red_recurse", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[77] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_red_partial_sum", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[79] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_red_extrapolate", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Double Precision ocl_kernels_vector_dp.cl
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[9]  = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_scaleadd", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[11] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_scaleaddscale", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[13] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_scaleadd2", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[15] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_pointwisemult", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[17] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_pointwisemult2", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[19] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_copy_offset_from", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[21] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_permute", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[23] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_permute_backward", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[25] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_dot", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[27] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_axpy", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[53] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_scale", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[89] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_reduce", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[101] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_asum", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[103] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_amax", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Double Precision ocl_kernels_csr_dp.cl
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[29] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_spmv_scalar", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[31] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_add_spmv_scalar", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[33] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_scale_diagonal", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[35] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_scale_offdiagonal", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[37] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_add_diagonal", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[39] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_add_offdiagonal", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[41] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_extract_diag", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[43] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_extract_inv_diag", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[45] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_extract_submatrix_row_nnz", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[47] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_extract_submatrix_copy", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[49] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_diagmatmult", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[51] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_add_csr_same_struct", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[81] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_permute_rows", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[83] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_permute_cols", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[85] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_calc_row_nnz", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[87] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_csr_permute_row_nnz", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Double Precision ocl_kernels_mcsr.cl
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[55] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_mcsr_spmv_scalar", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[57] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_mcsr_add_spmv_scalar", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Double Precision ocl_kernels_ell.cl
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[59] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_ell_spmv", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[61] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_ell_add_spmv", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[93] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_ell_max_row", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[95] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_ell_csr_to_ell", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Double Precision ocl_kernels_dia.cl
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[63] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_dia_spmv", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[65] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_dia_add_spmv", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  // Single Precision ocl_kernels_coo.cl
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[67] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_coo_permute", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[69] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_coo_spmv_flat", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[71] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_coo_spmv_reduce_update", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[73] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp, "kernel_coo_spmv_serial", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  // Integer ocl_kernels_general_int.cl
  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[ 0] = clCreateKernel(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_int, "kernel_set_to", &err);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

}


// Returns the kernel for double precision
template <>
cl_kernel paralution_get_ocl_kernel<double>(int kernel) {

  // return double kernel
  return (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[kernel + 1];

}


// Returns the kernel for single precision
template <>
cl_kernel paralution_get_ocl_kernel<float>(int kernel) {

  // return float kernel
  return (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[kernel];

}


// Returns the int kernel
template <>
cl_kernel paralution_get_ocl_kernel<int>(int kernel) {

  // return int kernel
  // TODO fix - hardcoded (for now because theres only 1 int kernel)
  return (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[0];

}


// Initalizes the OpenCL backend
bool paralution_init_ocl(void) {

  assert(_Backend_Descriptor.OCL_handle == NULL);
  //  assert(_Backend_Descriptor.OCL_dev == -1);

  // OpenCL specific variables
  _Backend_Descriptor.OCL_handle = new oclHandle_t;

  cl_int            err;
  cl_uint           ocl_numPlatforms;
  cl_uint           ocl_numDevices;
  cl_uint           ocl_minAlign;
  std::string       sourceStr_sp;
  std::string       sourceStr_dp;
  std::string       sourceStr_int;
  char              *source_sp;
  char              *source_dp;
  char              *source_int;

  // Query for the number of OpenCL platforms in the system
  err = clGetPlatformIDs(0, NULL, &ocl_numPlatforms);
  CHECK_OCL_ERROR(err, __FILE__, __LINE__);

  if (ocl_numPlatforms < 1) {

    LOG_INFO("No OpenCL platform available");
    LOG_INFO("OpenCL has NOT been initialized!");
    return false;

  } else {

    if (_Backend_Descriptor.OCL_plat == -1)
      _Backend_Descriptor.OCL_plat = OPENCL_PLATFORM;

    // Check for valid OpenCL platform
    if (_Backend_Descriptor.OCL_plat > int(ocl_numPlatforms)-1) {
      LOG_INFO("Invalid OpenCL platform");
      FATAL_ERROR(__FILE__, __LINE__);
    }

    // Allocate host memory for the platforms
    OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_platforms = new cl_platform_id[ocl_numPlatforms];

    // Get the OpenCL platforms
    err = clGetPlatformIDs(ocl_numPlatforms, OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_platforms, NULL);
    CHECK_OCL_ERROR(err, __FILE__, __LINE__);

    // Query for the number of OpenCL devices (any type) for the current OpenCL platform
    err = clGetDeviceIDs((OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_platforms)[_Backend_Descriptor.OCL_plat],
                         CL_DEVICE_TYPE_ALL, 0, NULL, &ocl_numDevices);
    CHECK_OCL_ERROR(err, __FILE__, __LINE__);

    if (ocl_numDevices < 1) {

      if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_platforms != NULL)
        delete[] OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_platforms;

      LOG_INFO("No OpenCL device available");
      return false;

    }

    if (_Backend_Descriptor.OCL_dev == -1)
      _Backend_Descriptor.OCL_dev = OPENCL_DEVICE;

    // Check for valid OpenCL device
    if (_Backend_Descriptor.OCL_dev > int(ocl_numDevices)-1) {
      LOG_INFO("Invalid OpenCL device");
      FATAL_ERROR(__FILE__, __LINE__);
    }

    // Allocate host memory for the devices
    OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices = new cl_device_id[ocl_numDevices];

    // Get the OpenCL devices
    err = clGetDeviceIDs((OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_platforms)[_Backend_Descriptor.OCL_plat],
                         CL_DEVICE_TYPE_ALL, ocl_numDevices,
                         OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices, NULL);
    CHECK_OCL_ERROR(err, __FILE__, __LINE__);

    // Create OpenCL context for computing device
    OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context = clCreateContext(0, 1,
                                                &(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices)[_Backend_Descriptor.OCL_dev],
                                                NULL, NULL, &err);
    CHECK_OCL_ERROR(err, __FILE__, __LINE__);

    // Create OpenCL command queue
    OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_cmdQueue = clCreateCommandQueue(
                                                   OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context,
                                                   (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices)[_Backend_Descriptor.OCL_dev],
                                                   0, &err);
    CHECK_OCL_ERROR(err, __FILE__, __LINE__);

    // Create OpenCL program from source for single precision
    std::stringstream ss;
    ss << "#define BLOCK_SIZE " << OPENCL_BLOCKSIZE << "\n";
    ss << "#define WARP_SIZE "  << OPENCL_WARPSIZE  << "\n";
    sourceStr_sp  = ("#pragma OPENCL EXTENSION cl_khr_fp64 : disable\n");
    sourceStr_sp.append("#define ValueType float\n");
    sourceStr_sp.append(ss.str());
    sourceStr_sp.append(ocl_kernels_general);
    sourceStr_sp.append(ocl_kernels_vector);
    sourceStr_sp.append(ocl_kernels_csr);
    sourceStr_sp.append(ocl_kernels_bcsr);
    sourceStr_sp.append(ocl_kernels_mcsr);
    sourceStr_sp.append(ocl_kernels_dense);
    sourceStr_sp.append(ocl_kernels_ell);
    sourceStr_sp.append(ocl_kernels_dia);
    sourceStr_sp.append(ocl_kernels_coo);
    sourceStr_sp.append(ocl_kernels_hyb);
    // Create OpenCL program from source for double precision
    sourceStr_dp  = ("#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n");
    sourceStr_dp.append("#define ValueType double\n");
    sourceStr_dp.append(ss.str());
    sourceStr_dp.append(ocl_kernels_general);
    sourceStr_dp.append(ocl_kernels_vector);
    sourceStr_dp.append(ocl_kernels_csr);
    sourceStr_dp.append(ocl_kernels_bcsr);
    sourceStr_dp.append(ocl_kernels_mcsr);
    sourceStr_dp.append(ocl_kernels_dense);
    sourceStr_dp.append(ocl_kernels_ell);
    sourceStr_dp.append(ocl_kernels_dia);
    sourceStr_dp.append(ocl_kernels_coo);
    sourceStr_dp.append(ocl_kernels_hyb);
    sourceStr_int = ("#define ValueType int\n");
    sourceStr_int.append(ss.str());
    sourceStr_int.append(ocl_kernels_general);

    source_sp  = new char[sourceStr_sp.size()  + 1];
    std::copy(sourceStr_sp.begin(),  sourceStr_sp.end(),  source_sp);
    source_sp[sourceStr_sp.size()]   = '\0';
    source_dp  = new char[sourceStr_dp.size()  + 1];
    std::copy(sourceStr_dp.begin(),  sourceStr_dp.end(),  source_dp);
    source_dp[sourceStr_dp.size()]   = '\0';
    source_int = new char[sourceStr_int.size() + 1];
    std::copy(sourceStr_int.begin(), sourceStr_int.end(), source_int);
    source_int[sourceStr_int.size()] = '\0';

    if (source_sp == NULL || source_dp == NULL || source_int == NULL) {
      LOG_INFO("Failed to load OpenCL kernel source files");
      paralution_stop_ocl();
      FATAL_ERROR(__FILE__, __LINE__);
    }

    // Create OpenCL program for single precision kernels
    OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp  = clCreateProgramWithSource(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context,
                                                                                            1,
                                                                                            (const char **) &source_sp,
                                                                                            NULL,
                                                                                            &err);
    CHECK_OCL_ERROR(err, __FILE__, __LINE__);
    // Create OpenCL program for double precision kernels
    OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp  = clCreateProgramWithSource(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context,
                                                                                            1,
                                                                                            (const char **) &source_dp,
                                                                                            NULL,
                                                                                            &err);
    CHECK_OCL_ERROR(err, __FILE__, __LINE__);
    // Create OpenCL program for int kernels
    OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_int = clCreateProgramWithSource(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context,
                                                                                            1,
                                                                                            (const char **) &source_int,
                                                                                            NULL,
                                                                                            &err);
    CHECK_OCL_ERROR(err, __FILE__, __LINE__);

    // Compile OpenCL programs
    err  = clBuildProgram(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp,  0,
                          NULL, "-w -cl-mad-enable -cl-no-signed-zeros", NULL, NULL);
    err |= clBuildProgram(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp,  0,
                          NULL, "-w -cl-mad-enable -cl-no-signed-zeros", NULL, NULL);
    err |= clBuildProgram(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_int, 0,
                          NULL, "-w -cl-mad-enable -cl-no-signed-zeros", NULL, NULL);
    if (err == CL_BUILD_PROGRAM_FAILURE) {

      // If compilation failed, get and print build log
      LOG_INFO("CL_BUILD_PROGRAM_FAILURE");
      char ocl_buildLog[1024];
      err = clGetProgramBuildInfo(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp,
                                  (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices)[_Backend_Descriptor.OCL_dev],
                                  CL_PROGRAM_BUILD_LOG,
                                  sizeof(ocl_buildLog),
                                  &ocl_buildLog,
                                  NULL);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);

      LOG_INFO("OCL BuildLog:\n" <<ocl_buildLog);
      FATAL_ERROR(__FILE__, __LINE__);

    }
    CHECK_OCL_ERROR(err, __FILE__, __LINE__);

    // Write kernels to backend kernel array
    paralution_set_ocl_kernels();

    // Free kernel sources
    delete[] source_sp;
    delete[] source_dp;
    delete[] source_int;

    _Backend_Descriptor.OCL_max_work_group_size = OPENCL_BLOCKSIZE;
    _Backend_Descriptor.OCL_computeUnits = OPENCL_CU;

    err = clGetDeviceInfo((OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices)[_Backend_Descriptor.OCL_dev],
                          CL_DEVICE_MEM_BASE_ADDR_ALIGN,
                          sizeof(ocl_minAlign),
                          &ocl_minAlign,
                          NULL);
    CHECK_OCL_ERROR(err, __FILE__, __LINE__);

	  // minAlign in Bytes TODO use this for array creation
    ocl_minAlign /= 8;

  }

  return true;

}


// Stop OpenCL backend
void paralution_stop_ocl(void) {

  if (_Backend_Descriptor.accelerator) {

    cl_int err;

    // If OpenCL command queue was created, release it
	  if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_cmdQueue) {
      err = clReleaseCommandQueue(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_cmdQueue);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_cmdQueue = NULL;
    }

    // If OpenCL kernel was created, release it
    for (int i=0; i<104; ++i) {
      if ((OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[i] != NULL) {
        err = clReleaseKernel((OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_kernel)[i]);
        CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      }
    }

    // If OpenCL program was created, release it
	  if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp) {
      err = clReleaseProgram(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_sp = NULL;
    }
    // If OpenCL program was created, release it
	  if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp) {
      err = clReleaseProgram(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_dp = NULL;
    }
    // If OpenCL program was created, release it
	  if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_int) {
      err = clReleaseProgram(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_int);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_program_int = NULL;
    }

    // If OpenCL platforms were queried, release them
    if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_platforms != NULL)
      delete[] OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_platforms;
    // If OpenCL devices were queried, release them
    if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices != NULL)
      delete[] OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices;

    // If OpenCL context was created, release it
	  if (OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context) {
      err = clReleaseContext(OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_context = NULL;
    }

  }

  delete static_cast<oclHandle_t*> (_Backend_Descriptor.OCL_handle);
  _Backend_Descriptor.OCL_handle = NULL;

  _Backend_Descriptor.OCL_plat = -1;
  _Backend_Descriptor.OCL_dev  = -1;

}


// Print OpenCL computation device information to screen
void paralution_info_ocl(const struct Paralution_Backend_Descriptor backend_descriptor) {

  cl_int          err;
  cl_ulong        ocl_global_mem_size;
  cl_ulong        ocl_local_mem_size;
  size_t          ocl_work_group_size;
  cl_device_type  ocl_typeDevice;

  char            ocl_namePlatform[1024];
  char            ocl_nameDevice[1024];


  // No platform selected
  if (backend_descriptor.OCL_plat == -1) {

    LOG_INFO("No OpenCL platforms available");

  } else {

    LOG_INFO("----------------------------------------------------");

    // Get and print OpenCL platform name
    err = clGetPlatformInfo((OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_platforms)[_Backend_Descriptor.OCL_plat],
                            CL_PLATFORM_NAME,
                            sizeof(ocl_namePlatform),
                            &ocl_namePlatform,
                            NULL);
    CHECK_OCL_ERROR(err, __FILE__, __LINE__);
    LOG_INFO("OpenCL Platform: " <<ocl_namePlatform);

    // No device selected
    if (backend_descriptor.OCL_dev == -1) {

      LOG_INFO("No OpenCL devices available on this platform");

    } else {

      // Get and print OpenCL device name
      err = clGetDeviceInfo((OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices)[_Backend_Descriptor.OCL_dev],
                            CL_DEVICE_NAME,
                            sizeof(ocl_nameDevice),
                            &ocl_nameDevice,
                            NULL);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      LOG_INFO("CL_DEVICE_NAME: " <<ocl_nameDevice);

      // Get and print OpenCL device vendor
      err = clGetDeviceInfo((OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices)[_Backend_Descriptor.OCL_dev],
                            CL_DEVICE_VENDOR,
                            sizeof(ocl_nameDevice),
                            &ocl_nameDevice,
                            NULL);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      LOG_INFO("CL_DEVICE_VENDOR: " <<ocl_nameDevice);

      // Get and print OpenCL device type
      err = clGetDeviceInfo((OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices)[_Backend_Descriptor.OCL_dev],
                            CL_DEVICE_TYPE,
                            sizeof(ocl_typeDevice),
                            &ocl_typeDevice,
                            NULL);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      if (ocl_typeDevice & CL_DEVICE_TYPE_CPU) LOG_INFO("CL_DEVICE_TYPE: CPU");
      if (ocl_typeDevice & CL_DEVICE_TYPE_GPU) LOG_INFO("CL_DEVICE_TYPE: GPU");
      if (ocl_typeDevice & CL_DEVICE_TYPE_ACCELERATOR) LOG_INFO("CL_DEVICE_TYPE: ACCELERATOR");
      if (ocl_typeDevice & CL_DEVICE_TYPE_DEFAULT) LOG_INFO("CL_DEVICE_TYPE: DEFAULT");

      // Print OpenCL device 'max compute units'
      LOG_INFO("CL_DEVICE_MAX_COMPUTE_UNITS: " <<_Backend_Descriptor.OCL_computeUnits);
      // Get and print OpenCL device 'max work group size'
      err = clGetDeviceInfo((OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices)[_Backend_Descriptor.OCL_dev],
                            CL_DEVICE_MAX_WORK_GROUP_SIZE,
                            sizeof(ocl_work_group_size),
                            &ocl_work_group_size,
                            NULL);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      LOG_INFO("CL_DEVICE_MAX_WORK_GROUP_SIZE: " <<ocl_work_group_size);

      // Get and print OpenCL device global memory
      err = clGetDeviceInfo((OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices)[_Backend_Descriptor.OCL_dev],
                            CL_DEVICE_GLOBAL_MEM_SIZE,
                            sizeof(ocl_global_mem_size),
                            &ocl_global_mem_size,
                            NULL);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      LOG_INFO("CL_DEVICE_GLOBAL_MEM_SIZE: " << (ocl_global_mem_size >> 20) <<" MByte");

      // Get and print OpenCL device local memory
      err = clGetDeviceInfo((OCL_HANDLE(_Backend_Descriptor.OCL_handle)->OCL_devices)[_Backend_Descriptor.OCL_dev],
                            CL_DEVICE_LOCAL_MEM_SIZE,
                            sizeof(ocl_local_mem_size),
                            &ocl_local_mem_size,
                            NULL);
      CHECK_OCL_ERROR(err, __FILE__, __LINE__);
      LOG_INFO("CL_DEVICE_LOCAL_MEM_SIZE: " << (ocl_local_mem_size >> 10) <<" KByte");
    }

  }

  LOG_INFO("----------------------------------------------------");

}


template <typename ValueType>
AcceleratorMatrix<ValueType>* _paralution_init_base_ocl_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                               const unsigned int matrix_format) {
  assert(backend_descriptor.backend == OCL);

  switch (matrix_format) {

  case CSR:
    return new OCLAcceleratorMatrixCSR<ValueType>(backend_descriptor);
    
  case COO:
    return new OCLAcceleratorMatrixCOO<ValueType>(backend_descriptor);

  case MCSR:
    return new OCLAcceleratorMatrixMCSR<ValueType>(backend_descriptor);

  case DIA:
    return new OCLAcceleratorMatrixDIA<ValueType>(backend_descriptor);
    
  case ELL:
    return new OCLAcceleratorMatrixELL<ValueType>(backend_descriptor);

  case DENSE:
    return new OCLAcceleratorMatrixDENSE<ValueType>(backend_descriptor);

  case HYB:
    return new OCLAcceleratorMatrixHYB<ValueType>(backend_descriptor);

  case BCSR:
    return new OCLAcceleratorMatrixBCSR<ValueType>(backend_descriptor);


      
  default:
    LOG_INFO("This backend is not supported for Matrix types");
    FATAL_ERROR(__FILE__, __LINE__);   
    return NULL;
  }

}


template <typename ValueType>
AcceleratorVector<ValueType>* _paralution_init_base_ocl_vector(const struct Paralution_Backend_Descriptor backend_descriptor) {

  assert(backend_descriptor.backend == OCL);

  return new OCLAcceleratorVector<ValueType>(backend_descriptor);

}


template AcceleratorVector<float>* _paralution_init_base_ocl_vector(const struct Paralution_Backend_Descriptor backend_descriptor);
template AcceleratorVector<double>* _paralution_init_base_ocl_vector(const struct Paralution_Backend_Descriptor backend_descriptor);
template AcceleratorVector<int>* _paralution_init_base_ocl_vector(const struct Paralution_Backend_Descriptor backend_descriptor);

template AcceleratorMatrix<float>* _paralution_init_base_ocl_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                                     const unsigned int matrix_format);
template AcceleratorMatrix<double>* _paralution_init_base_ocl_matrix(const struct Paralution_Backend_Descriptor backend_descriptor,
                                                                     const unsigned int matrix_format);

template cl_kernel paralution_get_ocl_kernel<float>(int kernel);
template cl_kernel paralution_get_ocl_kernel<double>(int kernel);
template cl_kernel paralution_get_ocl_kernel<int>(int kernel);

}

