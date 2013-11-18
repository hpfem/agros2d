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
    : PicardSolver<Scalar>(), SolverAgros(block)
{
}

template <typename Scalar>
bool PicardSolverAgros<Scalar>::on_initialization()
{
    m_relativeChangeOfSolutions.clear();

    return !Agros2D::problem()->isAborted();
}

template <typename Scalar>
bool PicardSolverAgros<Scalar>::on_initial_step_end()
{
    m_phase = Phase_Init;
    setError();
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
    m_phase = Phase_DFDetermined;
    setError();
    return !Agros2D::problem()->isAborted();
}

template <typename Scalar>
bool PicardSolverAgros<Scalar>::on_finish()
{
    m_phase = Phase_Finished;
    setError();
    return !Agros2D::problem()->isAborted();
}

template <typename Scalar>
void PicardSolverAgros<Scalar>::setError()
{
    if (m_phase == Phase_Init)
    {
        m_jacobianCalculations = 0;
        Agros2D::log()->printMessage(QObject::tr("Solver (Picard)"), QObject::tr("Initial step"));
        return;
    }

    int iteration = this->get_parameter_value(this->iteration()) - 1;
    const Hermes::vector<double>& residual_norms = this->get_parameter_value(this->residual_norms());
    const Hermes::vector<double>& solution_norms = this->get_parameter_value(this->solution_norms());
    const Hermes::vector<double>& solution_change_norms = this->get_parameter_value(this->solution_change_norms());
    const Hermes::vector<double>& damping_factors = this->get_parameter_value(this->damping_factors());

    double current_damping_factor = damping_factors.back();
    double previous_damping_factor = current_damping_factor;
    if (damping_factors.size() > 1)
        previous_damping_factor = damping_factors.at(damping_factors.size() - 2);

    double previous_solution_norm = solution_norms.back();
    if (solution_norms.size() > 1)
        previous_solution_norm = solution_norms.at(solution_norms.size() - 2);

    // add iteration
    m_steps.append(iteration);
    m_solutionNorms.append(solution_norms.back());
    m_relativeChangeOfSolutions.append(solution_change_norms.back() / previous_solution_norm * 100);

    assert(m_steps.size() == m_solutionNorms.size());
    assert(m_steps.size() == m_relativeChangeOfSolutions.size());

    if (m_phase == Phase_DFDetermined)
    {
        Agros2D::log()->printMessage(QObject::tr("Solver (Picard)"), QObject::tr("Iteration: %1 (rel. change of sol.: %3 %, damping: %2)")
                                     .arg(iteration)
                                     .arg(previous_damping_factor)
                                     .arg(QString::number(m_relativeChangeOfSolutions.last(), 'f', 5)));
    }
    else if (m_phase == Phase_Finished)
    {
        Agros2D::log()->printMessage(QObject::tr("Solver (Picard)"), QObject::tr("Calculation finished"));
    }
    else
        assert(0);

    m_damping.append(current_damping_factor);
    Agros2D::log()->updateNonlinearChartInfo(m_phase, m_steps, m_relativeChangeOfSolutions);
}

template <typename Scalar>
PicardSolverContainer<Scalar>::PicardSolverContainer(Block* block) : HermesSolverContainer<Scalar>(block)
{
    m_picardSolver = new PicardSolverAgros<Scalar>(block);
    m_picardSolver->set_verbose_output(true);
    m_picardSolver->clear_tolerances();
    m_picardSolver->set_tolerance(block->nonlinearRelativeChangeOfSolutions() / 100.0, SolutionChangeRelative);
    m_picardSolver->set_max_allowed_iterations(50);

    if (block->nonlinearDampingType() == DampingType_Off)
    {
        m_picardSolver->set_manual_damping_coeff(true, 1.0);
    }
    else if (block->nonlinearDampingType() == DampingType_Fixed)
    {
        m_picardSolver->set_manual_damping_coeff(true, block->nonlinearDampingCoeff());
    }
    else if (block->nonlinearDampingType() == DampingType_Automatic)
    {
        m_picardSolver->set_manual_damping_coeff(false, 1.0);
        m_picardSolver->set_initial_auto_damping_coeff(block->nonlinearDampingCoeff());
        m_picardSolver->set_necessary_successful_steps_to_increase(block->nonlinearStepsToIncreaseDampingFactor());
    }
    else
    {
        assert(0);
    }

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
