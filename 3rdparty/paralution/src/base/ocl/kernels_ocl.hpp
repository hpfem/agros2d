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


#ifndef PARALUTION_KERNELS_OCL_HPP_
#define PARALUTION_KERNELS_OCL_HPP_

#define KERNELCOUNT 58

static const std::string kernels_ocl[KERNELCOUNT] = {
                "kernel_set_to",
                "kernel_reverse_index",
                "kernel_buffer_addscalar",
                "kernel_scaleadd",
                "kernel_scaleaddscale",
                "kernel_scaleadd2",
                "kernel_pointwisemult",
                "kernel_pointwisemult2",
                "kernel_copy_offset_from",
                "kernel_permute",
                "kernel_permute_backward",
                "kernel_dot",
                "kernel_axpy",
                "kernel_csr_spmv_scalar",
                "kernel_csr_add_spmv_scalar",
                "kernel_csr_scale_diagonal",
                "kernel_csr_scale_offdiagonal",
                "kernel_csr_add_diagonal",
                "kernel_csr_add_offdiagonal",
                "kernel_csr_extract_diag",
                "kernel_csr_extract_inv_diag",
                "kernel_csr_extract_submatrix_row_nnz",
                "kernel_csr_extract_submatrix_copy",
                "kernel_csr_diagmatmult",
                "kernel_csr_add_csr_same_struct",
                "kernel_scale",
                "kernel_mcsr_spmv_scalar",
                "kernel_mcsr_add_spmv_scalar",
                "kernel_ell_spmv",
                "kernel_ell_add_spmv",
                "kernel_dia_spmv",
                "kernel_dia_add_spmv",
                "kernel_coo_permute",
                "kernel_coo_spmv_flat",
                "kernel_coo_spmv_reduce_update",
                "kernel_coo_spmv_serial",
                "kernel_red_recurse",
                "kernel_red_partial_sum",
                "kernel_red_extrapolate",
                "kernel_csr_permute_rows",
                "kernel_csr_permute_cols",
                "kernel_csr_calc_row_nnz",
                "kernel_csr_permute_row_nnz",
                "kernel_reduce",
                "kernel_ell_max_row",
                "kernel_ell_csr_to_ell",
                "kernel_asum",
                "kernel_amax",
                "kernel_dense_spmv",
                "kernel_csr_extract_l_triangular",
                "kernel_csr_slower_nnz_per_row",
                "kernel_csr_supper_nnz_per_row",
                "kernel_csr_lower_nnz_per_row",
                "kernel_csr_upper_nnz_per_row",
                "kernel_csr_compress_count_nrow",
                "kernel_csr_compress_copy",
                "kernel_scaleaddscale_offset",
                "kernel_csr_extract_u_triangular"
};


#endif // PARALUTION_KERNELS_OCL_HPP_

