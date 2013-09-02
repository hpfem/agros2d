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
    : NewtonSolver<Scalar>(), m_block(block)
{
}

template <typename Scalar>
bool NewtonSolverAgros<Scalar>::on_initialization()
{
    m_errors.clear();

    return !Agros2D::problem()->isAborted();
}

template <typename Scalar>
bool NewtonSolverAgros<Scalar>::on_initial_step_end()
{
    setError(Phase_Init);
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
    setError(Phase_Finished);
    return !Agros2D::problem()->isAborted();
}

template <typename Scalar>
void NewtonSolverAgros<Scalar>::on_damping_factor_updated()
{
    setError(Phase_DFDetermined);
}

template <typename Scalar>
void NewtonSolverAgros<Scalar>::on_reused_jacobian_step_end()
{
    setError(Phase_JacobianReused);
}

template <typename Scalar>
void NewtonSolverAgros<Scalar>::clearSteps()
{
    m_steps.clear();
    m_errors.clear();
}

template <typename Scalar>
void NewtonSolverAgros<Scalar>::setError(Phase phase)
{
    int iteration;
    if(phase == Phase_Init)
        iteration = 0;
    else
        iteration = this->get_current_iteration_number() - 1;

    const Hermes::vector<double>& residual_norms = this->get_parameter_value(this->residual_norms());
    const Hermes::vector<double>& solution_norms = this->get_parameter_value(this->solution_norms());
    const Hermes::vector<double>& solution_change_norms = this->get_parameter_value(this->solution_change_norms());
    const Hermes::vector<double>& damping_factors = this->get_parameter_value(this->damping_factors());
    const Hermes::vector<bool>& jacobian_recalculated_log = this->get_parameter_value(this->iterations_with_recalculated_jacobian());
    double current_damping_factor = 1.0;
    if (damping_factors.size() >= 1.0)
        current_damping_factor = damping_factors.at(damping_factors.size() - 1);
    unsigned int successful_steps_damping = this->get_parameter_value(this->successful_steps_damping());
    unsigned int successful_steps_jacobian = this->get_parameter_value(this->successful_steps_jacobian());

    double initial_residual_norm = residual_norms[0];
    double initial_solution_norm = solution_norms[0];

    double current_residual_norm = residual_norms[iteration];
    double current_solution_norm = solution_norms[iteration];
    double current_solution_change_norm = solution_change_norms[iteration];
    QString resNorms;
    for(int i = 0; i < residual_norms.size(); i++)
        resNorms = QObject::tr("%1%2, ").arg(resNorms).arg(residual_norms[i]);

    double previous_residual_norm = current_residual_norm;
    double previous_solution_norm = current_solution_norm;
    double previous_damping_factors = current_damping_factor;
    if (iteration > 1)
    {
        previous_residual_norm = residual_norms[iteration - 1];
        previous_solution_norm = solution_norms[iteration - 1];
    }
    if(damping_factors.size() >= 2)
        previous_damping_factors = damping_factors.at(damping_factors.size() - 2);


    // add iteration
    m_steps.append(iteration);

    switch(this->current_convergence_measurement)
    {
    case Hermes::Hermes2D::ResidualNormRelativeToInitial:
        m_errors.append((initial_residual_norm - current_residual_norm) / initial_residual_norm);
        break;
    case Hermes::Hermes2D::ResidualNormRelativeToPrevious:
        m_errors.append((previous_residual_norm - current_residual_norm) / previous_residual_norm);
        break;
    case Hermes::Hermes2D::ResidualNormRatioToInitial:
        m_errors.append(current_residual_norm / initial_residual_norm);
        break;
    case Hermes::Hermes2D::ResidualNormRatioToPrevious:
        m_errors.append(current_residual_norm / previous_residual_norm);
        break;
    case Hermes::Hermes2D::ResidualNormAbsolute:
        m_errors.append(current_residual_norm);
        break;
    case Hermes::Hermes2D::SolutionDistanceFromPreviousAbsolute:
        m_errors.append(current_solution_change_norm);
        break;
    case Hermes::Hermes2D::SolutionDistanceFromPreviousRelative:
        m_errors.append(current_solution_change_norm / current_solution_norm);
        break;
    default:
        throw AgrosException(QObject::tr("Convergence measurement '%1' doesn't exists.").arg(this->current_convergence_measurement));
    }

    assert(m_steps.size() == m_errors.size());
    if(phase != Phase_Finished)
        assert(m_steps.size() == iteration + 1);

    if(phase == Phase_Init)
    {
        assert(iteration == 0);
        Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Initial step, error: %1")
                                     .arg(m_errors.last()));
        //Agros2D::log()->printDebug(QObject::tr("Solver (Newton)"), QObject::tr("Norms history %1").arg(resNorms));
    }
    else if (phase == Phase_DFDetermined)
    {
        Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Iteration: %1, Jacobian recalculated, damping coeff.: %2, error: %3")
                                     .arg(iteration)
                                     .arg(previous_damping_factors)
                                     .arg(m_errors.last()));
        //Agros2D::log()->printDebug(QObject::tr("Solver (Newton)"), QObject::tr("Norms history %1").arg(resNorms));
    }
    else if (phase == Phase_JacobianReused)
    {
        Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Iteration: %1, Jacobian reused, damping coeff.: %2, error: %3")
                                     .arg(iteration)
                                     .arg(current_damping_factor)
                                     .arg(m_errors.last()));
    }
    else if (phase == Phase_Finished)
    {
        QString reuses;
        m_jacobianCalculations = 0;
        for(int i = 0; i < jacobian_recalculated_log.size(); i++)
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

        Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Calculation finished, error: %2, Jacobian recalculated %3x")
                                     .arg(m_errors.last())
                                     .arg(m_jacobianCalculations));
        //Agros2D::log()->printDebug(QObject::tr("Solver (Newton)"), QObject::tr("Jacobian reuse history %1").arg(reuses));
    }
    else
        assert(0);

    //    if (iteration == 1)
    //    {
    //        Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Initial step, error: %1")
    //                                     .arg(m_errors.last()));
    //    }
    //    else
    //    {
    //        if (successful_steps_jacobian == 0 && iteration > 2 && m_block->newtonMaxStepsWithReusedJacobian() > 0)
    //            Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Results not improved, step restarted with new Jacobian"));
    //        if (m_block->newtonDampingType() == DampingType_Automatic && successful_steps_damping == 0)
    //            Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Results not improved, step restarted with new damping coefficient"));

    //        Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Iteration: %1, damping coeff.: %2, error: %3")
    //                                     .arg(iteration)
    //                                     .arg(current_damping_coefficient)
    //                                     .arg(m_errors.last()));
    //    }

    m_damping.append(current_damping_factor);
    Agros2D::log()->setNonlinearTable(m_steps, m_errors);
}

template <typename Scalar>
NewtonSolverContainer<Scalar>::NewtonSolverContainer(Block* block) : HermesSolverContainer<Scalar>(block)
{
    m_newtonSolver = new NewtonSolverAgros<Scalar>(block);
    m_newtonSolver->set_verbose_output(false);
    m_newtonSolver->set_tolerance(block->nonlinearTolerance(), block->nonlinearConvergenceMeasurement());
    m_newtonSolver->set_max_allowed_iterations(1e5);
    m_newtonSolver->set_max_allowed_residual_norm(1e15);
    m_newtonSolver->set_convergence_measurement(block->nonlinearConvergenceMeasurement());
    m_newtonSolver->set_sufficient_improvement_factor_jacobian(block->newtonSufficientImprovementFactorForJacobianReuse());
    m_newtonSolver->set_sufficient_improvement_factor(block->newtonSufficientImprovementFactor());

    if(block->newtonReuseJacobian())
        m_newtonSolver->set_max_steps_with_reused_jacobian(block->newtonMaxStepsWithReusedJacobian());
    else
        m_newtonSolver->set_max_steps_with_reused_jacobian(0);

    if (block->newtonDampingType() == DampingType_Off)
    {
        m_newtonSolver->set_manual_damping_coeff(true, 1.);
    }
    else if (block->newtonDampingType() == DampingType_Fixed)
    {
        m_newtonSolver->set_manual_damping_coeff(true, block->newtonDampingCoeff());
    }
    else if (block->newtonDampingType() == DampingType_Automatic)
    {
        m_newtonSolver->set_initial_auto_damping_coeff(block->newtonDampingCoeff());
        m_newtonSolver->set_necessary_successful_steps_to_increase(block->newtonStepsToIncreaseDF());
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
