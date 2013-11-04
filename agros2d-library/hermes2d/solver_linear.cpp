// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "solver_linear.h"

#include "util.h"
#include "util/global.h"

#include "field.h"
#include "block.h"
#include "problem.h"
#include "hermes2d/problem_config.h"
#include "module.h"
#include "scene.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "solutionstore.h"
#include "plugin_interface.h"
#include "logview.h"
#include "bdf2.h"

#include "pythonlab/pythonengine.h"

using namespace Hermes::Hermes2D;

template <typename Scalar>
LinearSolverAgros<Scalar>::LinearSolverAgros(Block *block)
    : LinearSolver<Scalar>(), SolverAgros(block)
{
}

template <typename Scalar>
LinearSolverContainer<Scalar>::LinearSolverContainer(Block* block) : HermesSolverContainer<Scalar>(block)
{
    m_linearSolver = new LinearSolverAgros<Scalar>(block);
    m_linearSolver->set_verbose_output(false);

    // solver cache
    m_linearSolver->set_do_not_use_cache(true);
    if (Agros2D::configComputer()->useSolverCache)
        if (block->isTransient() || block->adaptivityType() != AdaptivityType_None)
            m_linearSolver->set_do_not_use_cache(false);

    this->m_constJacobianPossible = true;
}

template <typename Scalar>
LinearSolverContainer<Scalar>::~LinearSolverContainer()
{
    delete m_linearSolver;
    m_linearSolver = NULL;
}

template <typename Scalar>
void LinearSolverContainer<Scalar>::matrixUnchangedDueToBDF(bool unchanged)
{
    m_linearSolver->set_jacobian_constant(unchanged && this->m_constJacobianPossible);
}

template <typename Scalar>
void LinearSolverContainer<Scalar>::solve(Scalar* previousSolutionVector)
{
    m_linearSolver->solve(previousSolutionVector);
    this->m_slnVector = m_linearSolver->get_sln_vector();
}

//template class VectorStore<double>;
template class LinearSolverContainer<double>;
