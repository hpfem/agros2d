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

#include "solver_picard.h"

#include "util.h"
#include "util/global.h"
#include "util/conf.h"
#include "logview.h"
#include "problem.h"

#include "block.h"

using namespace Hermes::Hermes2D;


template <typename Scalar>
PicardSolverAgros<Scalar>::PicardSolverAgros(Block *block)
    : PicardSolver<Scalar>(), m_block(block)
{
}

template <typename Scalar>
bool PicardSolverAgros<Scalar>::on_initialization()
{
    m_errors.clear();

    return !Agros2D::problem()->isAborted();
}

template <typename Scalar>
bool PicardSolverAgros<Scalar>::on_initial_step_end()
{
    setError(Phase_Init);
    return !Agros2D::problem()->isAborted();
}

template <typename Scalar>
bool PicardSolverAgros<Scalar>::on_step_begin()
{
    return !Agros2D::problem()->isAborted();
}

template <typename Scalar>
bool PicardSolverAgros<Scalar>::on_step_end()
{
    setError(Phase_DFDetermined);
    return !Agros2D::problem()->isAborted();
}

template <typename Scalar>
bool PicardSolverAgros<Scalar>::on_finish()
{
    setError(Phase_Finished);
    return !Agros2D::problem()->isAborted();
}

template <typename Scalar>
void PicardSolverAgros<Scalar>::clearSteps()
{
    m_steps.clear();
    m_errors.clear();
}

template <typename Scalar>
void PicardSolverAgros<Scalar>::setError(Phase phase)
{
    if (phase == Phase_Init)
    {
        Agros2D::log()->printMessage(QObject::tr("Solver (Picard)"), QObject::tr("Initial step"));
        return;
    }

    int iteration = this->get_parameter_value(this->iteration()) - 1;
    const Hermes::vector<double>& residual_norms = this->get_parameter_value(this->residual_norms());
    const Hermes::vector<double>& solution_norms = this->get_parameter_value(this->solution_norms());
    const Hermes::vector<double>& solution_change_norms = this->get_parameter_value(this->solution_change_norms());

    // add iteration
    m_steps.append(iteration);
    m_errors.append(solution_change_norms.back());

    assert(m_steps.size() == m_errors.size());

    if (phase == Phase_DFDetermined)
    {
        Agros2D::log()->printMessage(QObject::tr("Solver (Picard)"), QObject::tr("Iteration: %1, error: %2")
                                     .arg(iteration)
                                     .arg(m_errors.last()));
    }
    else if (phase == Phase_Finished)
    {
        Agros2D::log()->printMessage(QObject::tr("Solver (Picard)"), QObject::tr("Calculation finished, error: %1")
                                     .arg(m_errors.last()));
    }
    else
        assert(0);

    Agros2D::log()->setNonlinearTable(m_steps, m_errors);
}

template <typename Scalar>
PicardSolverContainer<Scalar>::PicardSolverContainer(Block* block) : HermesSolverContainer<Scalar>(block)
{
    m_picardSolver = new PicardSolverAgros<Scalar>(block);
    m_picardSolver->set_verbose_output(false);
    m_picardSolver->clear_tolerances();
    m_picardSolver->set_tolerance(block->nonlinearTolerance(), block->nonlinearConvergenceMeasurement());
    m_picardSolver->set_max_allowed_iterations(1e5);
    if (block->picardAndersonAcceleration())
    {
        m_picardSolver->use_Anderson_acceleration(true);
        m_picardSolver->set_num_last_vector_used(block->picardAndersonNumberOfLastVectors());
        m_picardSolver->set_anderson_beta(block->picardAndersonBeta());
    }
    else
    {
        m_picardSolver->use_Anderson_acceleration(false);
    }

    // solver cache
    m_picardSolver->set_do_not_use_cache(!Agros2D::configComputer()->useSolverCache);
}

template <typename Scalar>
PicardSolverContainer<Scalar>::~PicardSolverContainer()
{
    delete m_picardSolver;
    m_picardSolver = NULL;
}

template <typename Scalar>
void PicardSolverContainer<Scalar>::solve(Scalar* previousSolutionVector)
{
    m_picardSolver->clearSteps();
    m_picardSolver->solve(previousSolutionVector);
    this->m_slnVector = m_picardSolver->get_sln_vector();
}

template class PicardSolverContainer<double>;
