// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#include "solver_linear.h"

#include "util.h"
#include "util/global.h"

#include "field.h"
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

template <typename Scalar>
LinearSolverAgros<Scalar>::LinearSolverAgros()
    : LinearSolver<Scalar>(), SolverAgros()
{
}

template <typename Scalar>
LinearSolverContainer<Scalar>::LinearSolverContainer() : HermesSolverContainer<Scalar>()
{
    m_linearSolver = new LinearSolverAgros<Scalar>();
    m_linearSolver->set_verbose_output(false);

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
