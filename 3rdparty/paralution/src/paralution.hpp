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


#ifndef PARALUTION_PARALUTION_HPP_
#define PARALUTION_PARALUTION_HPP_

#include "base/version.hpp"
#include "base/backend_manager.hpp"

#include "base/operator.hpp"
#include "base/vector.hpp"

#include "base/matrix_formats.hpp"
#include "base/global_matrix.hpp"
#include "base/local_matrix.hpp"

#include "base/host/host_vector.hpp"
#include "base/host/host_matrix_csr.hpp"
#include "base/host/host_matrix_coo.hpp"

#include "base/global_stencil.hpp"
#include "base/local_stencil.hpp"

#include "base/global_vector.hpp"
#include "base/local_vector.hpp"

#include "solvers/solver.hpp"
#include "solvers/iter_ctrl.hpp"
#include "solvers/chebyshev.hpp"
#include "solvers/mixed_precision.hpp"
#include "solvers/krylov/cg.hpp"
#include "solvers/krylov/cr.hpp"
#include "solvers/krylov/bicgstab.hpp"
#include "solvers/krylov/gmres.hpp"
#include "solvers/krylov/fgmres.hpp"
#include "solvers/krylov/minres.hpp"
#include "solvers/krylov/idr.hpp"
#include "solvers/multigrid/multigrid.hpp"
#include "solvers/multigrid/multigrid_amg.hpp"
#include "solvers/deflation/dpcg.hpp"
#include "solvers/direct/inversion.hpp"
#include "solvers/direct/lu.hpp"
#include "solvers/direct/qr.hpp"

#include "solvers/preconditioners/preconditioner.hpp"
#include "solvers/preconditioners/preconditioner_ai.hpp"
#include "solvers/preconditioners/preconditioner_as.hpp"
#include "solvers/preconditioners/preconditioner_multicolored.hpp"
#include "solvers/preconditioners/preconditioner_multicolored_gs.hpp"
#include "solvers/preconditioners/preconditioner_multicolored_ilu.hpp"
#include "solvers/preconditioners/preconditioner_multielimination.hpp"
#include "solvers/preconditioners/preconditioner_saddlepoint.hpp"
#include "solvers/preconditioners/preconditioner_blockprecond.hpp"

#include "utils/allocate_free.hpp"
#include "utils/math_functions.hpp"
#include "utils/time_functions.hpp"


#endif // PARALUTION_PARALUTION_HPP_
