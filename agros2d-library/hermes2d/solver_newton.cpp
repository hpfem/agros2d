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

#include "solver_newton.h"

#include "util.h"
#include "util/global.h"
#include "util/conf.h"
#include "logview.h"
#include "problem.h"

#include "block.h"

using namespace Hermes::Hermes2D;


template <typename Scalar>
NewtonSolverAgros<Scalar>::NewtonSolverAgros(Block *block)
    : NewtonSolver<Scalar>(), SolverAgros(block)
{
}

template <typename Scalar>
bool NewtonSolverAgros<Scalar>::on_initialization()
{
    m_residualNorms.clear();
    m_solutionNorms.clear();
    m_relativeChangeOfSolutions.clear();

    return !Agros2D::problem()->isAborted();
}

template <typename Scalar>
bool NewtonSolverAgros<Scalar>::on_initial_step_end()
{
    m_phase = Phase_Init;
    setError();
    return !Agros2D::problem()->isAborted();
}

template <typename Scalar>
bool NewtonSolverAgros<Scalar>::on_step_begin()
{
    return !Agros2D::problem()->isAborted();
}

template <typename Scalar>
bool NewtonSolverAgros<Scalar>::on_step_end()
{
    return !Agros2D::problem()->isAborted();
}

template <typename Scalar>
bool NewtonSolverAgros<Scalar>::on_finish()
{
    m_phase = Phase_Finished;
    setError();
    return !Agros2D::problem()->isAborted();
}

template <typename Scalar>
void NewtonSolverAgros<Scalar>::on_damping_factor_updated()
{
    m_phase = Phase_DFDetermined;
    setError();
}

template <typename Scalar>
void NewtonSolverAgros<Scalar>::on_reused_jacobian_step_end()
{
    m_phase = Phase_JacobianReused;
    setError();
}

template <typename Scalar>
void NewtonSolverAgros<Scalar>::setError()
{
    int iteration;
    if (m_phase == Phase_Init)
        iteration = 0;
    else
        iteration = this->get_current_iteration_number();

    const Hermes::vector<double>& residual_norms = this->get_parameter_value(this->residual_norms());
    const Hermes::vector<double>& solution_norms = this->get_parameter_value(this->solution_norms());
    const Hermes::vector<double>& solution_change_norms = this->get_parameter_value(this->solution_change_norms());
    const Hermes::vector<double>& damping_factors = this->get_parameter_value(this->damping_factors());
    const Hermes::vector<bool>& jacobian_recalculated_log = this->get_parameter_value(this->iterations_with_recalculated_jacobian());

    double current_damping_factor = damping_factors.back();
    double previous_damping_factor = current_damping_factor;
    if (damping_factors.size() > 1)
        previous_damping_factor = damping_factors.at(damping_factors.size() - 2);

    double previous_solution_norm = solution_norms.back();
    if (solution_norms.size() > 2)
        previous_solution_norm = solution_norms.at(solution_norms.size() - 2);

    // add iteration
    m_steps.append(iteration);
    m_residualNorms.append(residual_norms.back());
    m_solutionNorms.append(solution_norms.back());
    m_relativeChangeOfSolutions.append(solution_change_norms.back() / previous_solution_norm * 100);

    assert(m_steps.size() == m_residualNorms.size());
    assert(m_steps.size() == m_solutionNorms.size());
    assert(m_steps.size() == m_relativeChangeOfSolutions.size());

    if (m_phase == Phase_Init)
    {
        assert(iteration == 0);
        Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Initial step, error: %1")
                                     .arg(m_residualNorms.last()));
    }
    else if (m_phase == Phase_DFDetermined)
    {
        Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Iteration: %1 (res. norm: %3, rel. change of sol.: %4 %, Jacobian recalculated, damping : %2)")
                                     .arg(iteration)
                                     .arg(previous_damping_factor)
                                     .arg(m_residualNorms.last())
                                     .arg(QString::number(m_relativeChangeOfSolutions.last(), 'f', 5)));
    }
    else if (m_phase == Phase_JacobianReused)
    {
        Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Iteration: %1 (res. norm: %3, rel. change of sol.: %4 %, Jacobian reused, damping: %2)")
                                     .arg(iteration)
                                     .arg(current_damping_factor)
                                     .arg(m_residualNorms.last())
                                     .arg(QString::number(m_relativeChangeOfSolutions.last(), 'f', 5)));
    }
    else if (m_phase == Phase_Finished)
    {
        QString reuses;
        m_jacobianCalculations = 0;
        for (int i = 0; i < jacobian_recalculated_log.size(); i++)
        {
            if(jacobian_recalculated_log.at(i))
            {
                m_jacobianCalculations++;
                reuses.append("F ");
            }
            else
            {
                reuses.append("T ");
            }
        }

        Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Calculation finished (Jacobian recalculated %1x)")
                                     .arg(m_jacobianCalculations));
    }
    else
        assert(0);

    m_damping.append(current_damping_factor);
    Agros2D::log()->updateNonlinearChartInfo(m_phase, m_steps, m_relativeChangeOfSolutions);
}

template <typename Scalar>
NewtonSolverContainer<Scalar>::NewtonSolverContainer(Block* block) : HermesSolverContainer<Scalar>(block)
{
    m_newtonSolver = new NewtonSolverAgros<Scalar>(block);
    m_newtonSolver->set_verbose_output(false);
    m_newtonSolver->clear_tolerances();
    m_newtonSolver->set_tolerance(block->nonlinearResidualNorm(), ResidualNormAbsolute);
    m_newtonSolver->set_tolerance(block->nonlinearRelativeChangeOfSolutions() / 100.0, SolutionChangeRelative);
    m_newtonSolver->set_max_allowed_iterations(500);
    m_newtonSolver->set_max_allowed_residual_norm(1e15);
    m_newtonSolver->set_sufficient_improvement_factor_jacobian(block->newtonSufficientImprovementFactorForJacobianReuse());
    m_newtonSolver->set_sufficient_improvement_factor(block->nonlinearDampingFactorDecreaseRatio());

    if(block->newtonReuseJacobian())
        m_newtonSolver->set_max_steps_with_reused_jacobian(block->newtonMaxStepsWithReusedJacobian());
    else
        m_newtonSolver->set_max_steps_with_reused_jacobian(0);

    if (block->nonlinearDampingType() == DampingType_Off)
    {
        m_newtonSolver->set_manual_damping_coeff(true, 1.0);
    }
    else if (block->nonlinearDampingType() == DampingType_Fixed)
    {
        m_newtonSolver->set_manual_damping_coeff(true, block->nonlinearDampingCoeff());
    }
    else if (block->nonlinearDampingType() == DampingType_Automatic)
    {
        m_newtonSolver->set_manual_damping_coeff(false, 1.0);
        m_newtonSolver->set_initial_auto_damping_coeff(block->nonlinearDampingCoeff());
        m_newtonSolver->set_necessary_successful_steps_to_increase(block->nonlinearStepsToIncreaseDampingFactor());
    }
    else
    {
        assert(0);
    }

    // solver cache
    m_newtonSolver->set_do_not_use_cache(!Agros2D::configComputer()->useSolverCache);
}

template <typename Scalar>
NewtonSolverContainer<Scalar>::~NewtonSolverContainer()
{
    delete m_newtonSolver;
    m_newtonSolver = NULL;
}

template <typename Scalar>
void NewtonSolverContainer<Scalar>::solve(Scalar* previousSolutionVector)
{
    m_newtonSolver->clearSteps();
    m_newtonSolver->solve(previousSolutionVector);
    this->m_slnVector = m_newtonSolver->get_sln_vector();
}

template class NewtonSolverContainer<double>;
