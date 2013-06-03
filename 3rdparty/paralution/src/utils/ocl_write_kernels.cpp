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

#include <iostream>
#include <fstream>
#include <string>

#include "paths.h"

const int oclKernels = 10;

static const std::string ocl_kernel_files[oclKernels] = {
                "ocl_kernels_general",
                "ocl_kernels_vector",
                "ocl_kernels_csr",
                "ocl_kernels_bcsr",
                "ocl_kernels_mcsr",
                "ocl_kernels_dense",
                "ocl_kernels_ell",
                "ocl_kernels_dia",
                "ocl_kernels_coo",
                "ocl_kernels_hyb"
};

static const std::string ocl_header_string[oclKernels] = {
                "OCL_KERNELS_GENERAL",
                "OCL_KERNELS_VECTOR",
                "OCL_KERNELS_CSR",
                "OCL_KERNELS_BCSR",
                "OCL_KERNELS_MCSR",
                "OCL_KERNELS_DENSE",
                "OCL_KERNELS_ELL",
                "OCL_KERNELS_DIA",
                "OCL_KERNELS_COO",
                "OCL_KERNELS_HYB"
};


int main() {

  for (int i = 0; i < oclKernels; ++i) {

    std::ifstream input_kernel( (kernelPath + ocl_kernel_files[i] + ".cl").c_str() );
    std::ofstream output_kernel( (kernelPath + ocl_kernel_files[i] + ".hpp").c_str() );

    if ( input_kernel.is_open() && output_kernel.is_open() ) {

      std::string line;

      output_kernel << "#ifndef PARALUTION_" << ocl_header_string[i] << "_HPP_\n";
      output_kernel << "#define PARALUTION_" << ocl_header_string[i] << "_HPP_\n";
      output_kernel << "\nnamespace paralution {\n";
      output_kernel << "\nconst char *" << ocl_kernel_files[i] << " =";

      while ( !input_kernel.eof() ) {

        std::getline( input_kernel, line );
        output_kernel << "\n\t\"" << line << "\\n\"";

      }

      output_kernel << ";\n}\n";
      output_kernel << "\n#endif // PARALUTION_" << ocl_header_string[i] << "_HPP_\n";

    } else {

      std::cout <<"Could not open OpenCL kernel files.\n";
      return 1;

    }

  }

  std::cout <<"-- Writing OpenCL kernel headers...\n";

  return 0;

}

