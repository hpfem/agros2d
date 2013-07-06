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

#include "solver.h"

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

using namespace Hermes::Hermes2D;

int DEBUG_COUNTER = 0;

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
    unsigned int iteration = this->get_parameter_value(this->iteration());
    const Hermes::vector<double>& residual_norms = this->get_parameter_value(this->residual_norms());
    const Hermes::vector<double>& solution_norms = this->get_parameter_value(this->solution_norms());
    double solution_change_norm = this->get_parameter_value(this->solution_change_norm());
    const Hermes::vector<double>& damping_coefficients = this->get_parameter_value(this->damping_coefficients());
    double current_damping_coefficient = 1.0;
    if (damping_coefficients.size() >= 1.0)
        current_damping_coefficient = damping_coefficients.at(damping_coefficients.size() - 1);
    unsigned int successful_steps_damping = this->get_parameter_value(this->successful_steps_damping());
    unsigned int successful_steps_jacobian = this->get_parameter_value(this->successful_steps_jacobian());

    double initial_residual_norm = residual_norms[0];
    double initial_solution_norm = solution_norms[0];

    double current_residual_norm = residual_norms[iteration - 1];
    double current_solution_norm = solution_norms[iteration - 1];
    QString resNorms;
    for(int i = 0; i < iteration; i++)
        resNorms = QObject::tr("%1%2, ").arg(resNorms).arg(residual_norms[i]);

    double previous_residual_norm = current_residual_norm;
    double previous_solution_norm = current_solution_norm;
    double previous_dampinig_coefficient = current_damping_coefficient;
    if (iteration > 1)
    {
        previous_residual_norm = residual_norms[iteration - 2];
        previous_solution_norm = solution_norms[iteration - 2];
    }
    if(damping_coefficients.size() >= 2)
        previous_dampinig_coefficient = damping_coefficients.at(damping_coefficients.size() - 2);


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
        m_errors.append(solution_change_norm);
        break;
    case Hermes::Hermes2D::SolutionDistanceFromPreviousRelative:
        m_errors.append(solution_change_norm / current_solution_norm);
        break;
    default:
        throw AgrosException(QObject::tr("Convergence measurement '%1' doesn't exists.").arg(this->current_convergence_measurement));
    }

    assert(m_steps.size() == m_errors.size());

    if(phase == Phase_Init)
    {
        assert(iteration == 1);
        Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Initial step, error: %1")
                                     .arg(m_errors.last()));
    }
    else if (phase == Phase_DFDetermined)
    {
        Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Iteration: %1, Jacobian recalculated, damping coeff.: %2, error: %3")
                                     .arg(iteration)
                                     .arg(previous_dampinig_coefficient)
                                     .arg(m_errors.last()));
    }
    else if (phase == Phase_JacobianReused)
    {
        Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Iteration: %1, Jacobian reused, damping coeff.: %2, error: %3")
                                     .arg(iteration)
                                     .arg(current_damping_coefficient)
                                     .arg(m_errors.last()));
    }
    else if (phase == Phase_Finished)
    {
        Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Iteration: %1, calculation finished, error: %2")
                                     .arg(iteration)
                                     .arg(m_errors.last()));
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

    m_damping.append(current_damping_coefficient);
    Agros2D::log()->setNonlinearTable(m_steps, m_errors);
}

template <typename Scalar>
void HermesSolverContainer<Scalar>::setMatrixRhsOutputGen(Hermes::Hermes2D::Mixins::MatrixRhsOutput<Scalar>* solver, QString solverName, int adaptivityStep)
{
    if(Agros2D::configComputer()->saveMatrixRHS)
    {
        solver->output_matrix();
        solver->output_rhs();
        QString name = QString("%1/%2_%3_%4").arg(tempProblemDir()).arg(solverName).arg(Agros2D::problem()->actualTimeStep()).arg(adaptivityStep);
        solver->set_matrix_filename(QString("%1_Matrix").arg(name).toStdString());
        solver->set_rhs_filename(QString("%1_RHS").arg(name).toStdString());
        solver->set_matrix_number_format("%g");
        solver->set_rhs_number_format("%g");
    }
}

template <typename Scalar>
QSharedPointer<HermesSolverContainer<Scalar> > HermesSolverContainer<Scalar>::factory(Block* block)
{
    Hermes::HermesCommonApi.set_integral_param_value(Hermes::matrixSolverType, block->matrixSolver());
    Agros2D::log()->printDebug(QObject::tr("Solver"), QObject::tr("Linear solver: %1").arg(matrixSolverTypeString(block->matrixSolver())));

    QSharedPointer<HermesSolverContainer<Scalar> > solver;

    if (block->linearityType() == LinearityType_Linear)
    {
        solver = QSharedPointer<HermesSolverContainer<Scalar> >(new LinearSolverContainer<Scalar>(block));
    }
    else if (block->linearityType() == LinearityType_Newton)
    {
        solver = QSharedPointer<HermesSolverContainer<Scalar> >(new NewtonSolverContainer<Scalar>(block));
    }
    else if (block->linearityType() == LinearityType_Picard)
    {
        solver = QSharedPointer<HermesSolverContainer<Scalar> >(new PicardSolverContainer<Scalar>(block));
    }

    assert(!solver.isNull());

    if (IterSolver<Scalar> *linearSolver = dynamic_cast<IterSolver<Scalar> *>(solver->linearSolver()))
    {
        linearSolver->set_max_iters(block->iterLinearSolverIters());
        linearSolver->set_tolerance(block->iterLinearSolverToleranceAbsolute());
    }
    if (ParalutionLinearMatrixSolver<Scalar> *linearSolver = dynamic_cast<ParalutionLinearMatrixSolver<Scalar> *>(solver.data()->linearSolver()))
    {
        linearSolver->set_solver_type(block->iterLinearSolverMethod());
        linearSolver->set_precond(new Hermes::Preconditioners::ParalutionPrecond<Scalar>(block->iterLinearSolverPreconditioner()));
    }

    return solver;
}

template <typename Scalar>
void HermesSolverContainer<Scalar>::projectPreviousSolution(Scalar* solutionVector,
                                                            Hermes::vector<SpaceSharedPtr<Scalar> > spaces,
                                                            Hermes::vector<MeshFunctionSharedPtr<Scalar> > solutions)
{
    if (solutions.empty())
    {
        int ndof = Space<Scalar>::get_num_dofs(spaces);
        memset(solutionVector, 0, ndof*sizeof(Scalar));
    }
    else
    {
        OGProjection<double> ogProjection;
        ogProjection.project_global(spaces,
                                    solutions,
                                    solutionVector, this->m_block->projNormTypeVector());
    }
}

template <typename Scalar>
LinearSolverContainer<Scalar>::LinearSolverContainer(Block* block) : HermesSolverContainer<Scalar>(block)
{
    m_linearSolver = new LinearSolver<Scalar>();
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

template <typename Scalar>
NewtonSolverContainer<Scalar>::NewtonSolverContainer(Block* block) : HermesSolverContainer<Scalar>(block)
{
    m_newtonSolver = new NewtonSolverAgros<Scalar>(block);
    m_newtonSolver->set_verbose_output(true);
    m_newtonSolver->set_tolerance(block->nonlinearTolerance());
    m_newtonSolver->set_max_allowed_iterations(block->nonlinearSteps());
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

template <typename Scalar>
PicardSolverContainer<Scalar>::PicardSolverContainer(Block* block) : HermesSolverContainer<Scalar>(block)
{
    m_picardSolver = new PicardSolver<Scalar>();
    m_picardSolver->set_verbose_output(true);
    m_picardSolver->set_tolerance(block->nonlinearTolerance());
    m_picardSolver->set_max_allowed_iterations(block->nonlinearSteps());
    if (block->picardAndersonAcceleration())
    {
        m_picardSolver->use_Anderson_acceleration(true);
        m_picardSolver->set_num_last_vector_used(block->picardAndersonNumberOfLastVectors());
        m_picardSolver->set_anderson_beta(block->picardAndersonBeta());
    }
    else
        m_picardSolver->use_Anderson_acceleration(false);

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
    m_picardSolver->solve(previousSolutionVector);
    this->m_slnVector = m_picardSolver->get_sln_vector();
}

template <typename Scalar>
ProblemSolver<Scalar>::~ProblemSolver()
{
    clearActualSpaces();
}

template <typename Scalar>
void ProblemSolver<Scalar>::init(Block* block)
{
    m_block = block;

    QListIterator<Field*> iter(m_block->fields());
    while (iter.hasNext())
    {
        QString str = iter.next()->fieldInfo()->fieldId();
        m_solverName += str;
        m_solverCode += str;
        if (iter.hasNext())
        {
            m_solverName += ", ";
            m_solverCode += "_";
        }
    }
    m_solverID = QObject::tr("Solver (%1)").arg(m_solverName);
}

template <typename Scalar>
void ProblemSolver<Scalar>::initSelectors(Hermes::vector<NormType>& projNormType,
                                          Hermes::vector<RefinementSelectors::Selector<Scalar> *>& selectors)
{
    // set adaptivity selector
    RefinementSelectors::Selector<Scalar> *select = NULL;

    // create types of projection and selectors
    for (int i = 0; i < m_block->numSolutions(); i++)
    {
        // add norm
        projNormType.push_back(m_block->adaptivityNormType());

        RefinementSelectors::CandList candList;

        if (m_block->adaptivityType() == AdaptivityType_None)
        {
            if (m_block->adaptivityUseAniso())
                candList = RefinementSelectors::H2D_HP_ANISO;
            else
                candList = RefinementSelectors::H2D_HP_ISO;
        }
        else
        {
            switch (m_block->adaptivityType())
            {
            case AdaptivityType_H:
                if (m_block->adaptivityUseAniso())
                    candList = RefinementSelectors::H2D_H_ANISO;
                else
                    candList = RefinementSelectors::H2D_H_ISO;
                break;
            case AdaptivityType_P:
                if (m_block->adaptivityUseAniso())
                    candList = RefinementSelectors::H2D_P_ANISO;
                else
                    candList = RefinementSelectors::H2D_P_ISO;
                break;
            case AdaptivityType_HP:
                if (m_block->adaptivityUseAniso())
                    candList = RefinementSelectors::H2D_HP_ANISO;
                else
                    candList = RefinementSelectors::H2D_HP_ISO;
                break;
            }
        }
        select = new RefinementSelectors::H1ProjBasedSelector<Scalar>(candList);

        // add refinement selector
        selectors.push_back(select);
    }
}

template <typename Scalar>
void ProblemSolver<Scalar>::deleteSelectors(Hermes::vector<RefinementSelectors::Selector<Scalar> *>& selectors)
{
    foreach(RefinementSelectors::Selector<Scalar> *select, selectors)
        delete select;
    selectors.clear();
}

template <typename Scalar>
Hermes::vector<SpaceSharedPtr<Scalar> > ProblemSolver<Scalar>::deepMeshAndSpaceCopy(Hermes::vector<SpaceSharedPtr<Scalar> > spaces, bool createReference)
{
    Hermes::vector<SpaceSharedPtr<Scalar> > newSpaces;
    int totalComp = 0;
    foreach(Field* field, m_block->fields())
    {
        bool refineMesh = false;
        int orderIncrease = 0;
        if(createReference)
        {
            AdaptivityType adaptivityType = field->fieldInfo()->adaptivityType();
            if (m_block->adaptivityFinerReference() || (adaptivityType != AdaptivityType_P))
                refineMesh = true;

            if (m_block->adaptivityFinerReference() || (adaptivityType != AdaptivityType_H))
                orderIncrease = 1;
        }

        MeshSharedPtr mesh;
        // Deep copy of mesh for each field separately, than use for all field component the same one
        if (refineMesh)
        {
            Mesh::ReferenceMeshCreator meshCreator(spaces.at(totalComp)->get_mesh());
            mesh = meshCreator.create_ref_mesh();
        }
        else
        {
            mesh = MeshSharedPtr(new Mesh());
            mesh->copy(spaces.at(totalComp)->get_mesh());
        }

        for (int comp = 0; comp < field->fieldInfo()->numberOfSolutions(); comp++)
        {
            // TODO: double -> Scalar
            Space<double>::ReferenceSpaceCreator spaceCreator(spaces.at(totalComp),
                                                              mesh,
                                                              orderIncrease);
            newSpaces.push_back(spaceCreator.create_ref_space());

            totalComp++;
        }
    }

    return newSpaces;
}

template <typename Scalar>
void ProblemSolver<Scalar>::setActualSpaces(Hermes::vector<SpaceSharedPtr<Scalar> > spaces)
{
    clearActualSpaces();
    m_actualSpaces = spaces;
}

template <typename Scalar>
void ProblemSolver<Scalar>::clearActualSpaces()
{
    m_actualSpaces.clear();
}

template <typename Scalar>
Scalar *ProblemSolver<Scalar>::solveOneProblem(Hermes::vector<SpaceSharedPtr<Scalar> > spaces,
                                               int adaptivityStep,
                                               Hermes::vector<MeshFunctionSharedPtr<Scalar> > previousSolution)
{
    LinearMatrixSolver<Scalar> *linearSolver = m_hermesSolverContainer->linearSolver();

    if (m_block->isTransient())
        linearSolver->set_reuse_scheme(HERMES_REUSE_MATRIX_REORDERING);

    Scalar* initialSolutionVector = new Scalar[Hermes::Hermes2D::Space<Scalar>::get_num_dofs(spaces)];

    m_hermesSolverContainer->projectPreviousSolution(initialSolutionVector, spaces, previousSolution);
    m_hermesSolverContainer->setMatrixRhsOutput(m_solverCode, adaptivityStep);
    m_hermesSolverContainer->solve(initialSolutionVector);

    if (initialSolutionVector)
        delete [] initialSolutionVector;

    // linear solver statistics
    if (IterSolver<Scalar> *iterLinearSolver = dynamic_cast<IterSolver<Scalar> *>(linearSolver))
    {
        Agros2D::log()->printDebug(QObject::tr("Solver"),
                                   QObject::tr("Iterative solver statistics: %1 iterations")
                                   .arg(iterLinearSolver->get_num_iters()));
    }

    return m_hermesSolverContainer->slnVector();
}

template <typename Scalar>
void ProblemSolver<Scalar>::solveSimple(int timeStep, int adaptivityStep)
{
    // to be used as starting vector for the Newton solver
    MultiArray<Scalar> previousTSMultiSolutionArray;
    // if ((m_block->isTransient() && m_block->linearityType() != LinearityType_Linear) && (timeStep > 0))
    if ((m_block->isTransient()) && (timeStep > 0))
        previousTSMultiSolutionArray = Agros2D::solutionStore()->multiSolutionPreviousCalculatedTS(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal));

    // check for DOFs
    int ndof = Hermes::Hermes2D::Space<Scalar>::get_num_dofs(actualSpaces());
    if (ndof == 0)
    {
        Agros2D::log()->printDebug(m_solverID, QObject::tr("DOF is zero"));
        throw(AgrosSolverException("DOF is zero"));
    }
    Agros2D::log()->printDebug(QObject::tr("Solver"), QObject::tr("Number of DOFs: %1").arg(ndof));

    // cout << QString("updating with time %1\n").arg(Agros2D::problem()->actualTime()).toStdString() << endl;
    m_block->updateExactSolutionFunctions();

    Hermes::vector<SpaceSharedPtr<Scalar> > spaces = actualSpaces();
    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(spaces, Agros2D::problem()->actualTime());

    if (m_block->isTransient())
    {
        int order = min(timeStep, Agros2D::problem()->config()->value(ProblemConfig::TimeOrder).toInt());
        bool matrixUnchanged = m_block->weakForm()->bdf2Table()->setOrderAndPreviousSteps(order, Agros2D::problem()->timeStepLengths());
        m_hermesSolverContainer->matrixUnchangedDueToBDF(matrixUnchanged);

        // update timedep values
        Module::updateTimeFunctions(Agros2D::problem()->actualTime());
    }

    m_block->weakForm()->set_current_time(Agros2D::problem()->actualTime());
    m_block->weakForm()->updateExtField();

    try
    {
        Scalar *solutionVector = solveOneProblem(actualSpaces(), adaptivityStep,
                                                 previousTSMultiSolutionArray.solutions());

        // output
        Hermes::vector<MeshFunctionSharedPtr<Scalar> > solutions = createSolutions<Scalar>(spacesMeshes(actualSpaces()));
        Solution<Scalar>::vector_to_solutions(solutionVector, actualSpaces(), solutions);

        BlockSolutionID solutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal);
        SolutionStore::SolutionRunTimeDetails runTime(Agros2D::problem()->actualTimeStepLength(),
                                                      0.0,
                                                      Hermes::Hermes2D::Space<double>::get_num_dofs(actualSpaces()));

        if (dynamic_cast<NewtonSolverContainer<Scalar> *>(m_hermesSolverContainer.data()))
        {
            NewtonSolverAgros<Scalar> *solver = dynamic_cast<NewtonSolverContainer<Scalar> *>(m_hermesSolverContainer.data())->solver();

            runTime.setNewtonResidual(solver->errors());
            runTime.setNewtonDamping(solver->damping());
        }

        Agros2D::solutionStore()->addSolution(solutionID, MultiArray<Scalar>(actualSpaces(), solutions), runTime);
    }
    catch (AgrosSolverException e)
    {
        throw AgrosSolverException(QObject::tr("Solver failed: %1").arg(e.toString()));
    }
}

template <typename Scalar>
TimeStepInfo ProblemSolver<Scalar>::estimateTimeStepLength(int timeStep, int adaptivityStep)
{
    double timeTotal = Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble();

    // TODO: move to some config?
    const double relativeTimeStepLen = Agros2D::problem()->actualTimeStepLength() / timeTotal;
    const double maxTimeStepRatio = relativeTimeStepLen > 0.02 ? 2.0 : 3.0; // small steps may rise faster
    const double maxTimeStepLength = timeTotal / 10;
    const double maxToleranceMultiplyToAccept = 2.5; //3.0;

    TimeStepMethod timeStepMethod = (TimeStepMethod) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt();
    if(timeStepMethod == TimeStepMethod_Fixed)
        return TimeStepInfo(Agros2D::problem()->config()->constantTimeStepLength());

    MultiArray<Scalar> referenceCalculation =
            Agros2D::solutionStore()->multiArray(Agros2D::solutionStore()->lastTimeAndAdaptiveSolution(m_block, SolutionMode_Normal));

    // todo: ensure this in gui
    assert(Agros2D::problem()->config()->value(ProblemConfig::TimeOrder).toInt() >= 2);

    // todo: in the first step, I am acualy using order 1 and thus I am unable to decrease it!
    // this is not good, since the second step is not calculated (and the error of the first is not being checked)
    if (timeStep == 1)
    {
        m_averageErrorToLenghtRatio = 0.;
        return TimeStepInfo(Agros2D::problem()->actualTimeStepLength());
    }

    int previouslyUsedOrder = min(timeStep, Agros2D::problem()->config()->value(ProblemConfig::TimeOrder).toInt());
    bool matrixUnchanged = m_block->weakForm()->bdf2Table()->setOrderAndPreviousSteps(previouslyUsedOrder - 1, Agros2D::problem()->timeStepLengths());
    // using different order
    assert(matrixUnchanged == false);
    m_hermesSolverContainer->matrixUnchangedDueToBDF(matrixUnchanged);
    m_block->weakForm()->set_current_time(Agros2D::problem()->actualTime());
    m_block->weakForm()->updateExtField();

    // solutions obtained by time method of higher order in the original calculation
    Hermes::vector<MeshFunctionSharedPtr<Scalar> > timeReferenceSolution;
    if(timeStep > 0)
        timeReferenceSolution = referenceCalculation.solutions();
    Scalar *solutionVector = solveOneProblem(actualSpaces(), adaptivityStep, timeReferenceSolution);

    Hermes::vector<MeshSharedPtr> meshes = spacesMeshes(actualSpaces());
    Hermes::vector<MeshFunctionSharedPtr<Scalar> > solutions = createSolutions<Scalar>(meshes);
    Solution<Scalar>::vector_to_solutions(solutionVector, actualSpaces(), solutions);

    // error calculation
    DefaultErrorCalculator<double, HERMES_H1_NORM> errorCalculator(RelativeErrorToGlobalNorm, solutions.size());
    // calculate error the total error estimate.
    errorCalculator.calculate_errors(referenceCalculation.solutions(), solutions, false);
    double error = errorCalculator.get_total_error_squared();

    //update
    double actualRatio = error / Agros2D::problem()->actualTimeStepLength();
    m_averageErrorToLenghtRatio = ((timeStep - 2) * m_averageErrorToLenghtRatio + actualRatio) / (timeStep - 1);
    //m_averageErrorToLenghtRatio = (m_averageErrorToLenghtRatio + actualRatio) / 2.;

    double TOL;
    if (timeStepMethod == TimeStepMethod_BDFTolerance)
    {
        TOL = Agros2D::problem()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble();
    }
    else if (timeStepMethod == TimeStepMethod_BDFNumSteps)
    {
        int desiredNumSteps = (timeTotal / Agros2D::problem()->config()->constantTimeStepLength());
        int remainingSteps = max(2, desiredNumSteps - timeStep);
        double desiredStepSize = (timeTotal - Agros2D::problem()->actualTime()) / remainingSteps;
        TOL = desiredStepSize * m_averageErrorToLenghtRatio;

        // to avoid problems at the end of the time interval
        if(fabs(timeTotal - Agros2D::problem()->actualTime()) < 1e-5 * timeTotal)
            TOL = Agros2D::problem()->actualTimeStepLength() * m_averageErrorToLenghtRatio;
    }
    else
        assert(0);


    bool refuseThisStep = error > maxToleranceMultiplyToAccept  * TOL;

    // this guess is based on assymptotic considerations
    int timeOrder = Agros2D::problem()->config()->value(ProblemConfig::TimeOrder).toInt();
    double nextTimeStepLength = pow(TOL / error, 1.0 / (timeOrder + 1)) * Agros2D::problem()->actualTimeStepLength();

    nextTimeStepLength = min(nextTimeStepLength, maxTimeStepLength);
    nextTimeStepLength = min(nextTimeStepLength, Agros2D::problem()->actualTimeStepLength() * maxTimeStepRatio);
    nextTimeStepLength = max(nextTimeStepLength, Agros2D::problem()->actualTimeStepLength() / maxTimeStepRatio);

    Agros2D::log()->printDebug(m_solverID, QString("Time adaptivity, time %1, rel. error %2, tolerance %3, step size %4 -> %5 (%6 %)").
                               arg(Agros2D::problem()->actualTime()).
                               arg(error).
                               arg(TOL).
                               arg(Agros2D::problem()->actualTimeStepLength()).
                               arg(nextTimeStepLength).
                               arg(nextTimeStepLength / Agros2D::problem()->actualTimeStepLength()*100.));
    if(refuseThisStep)
        Agros2D::log()->printMessage(m_solverID, "Transient step refused");

    return TimeStepInfo(nextTimeStepLength, refuseThisStep);
}

template <typename Scalar>
void ProblemSolver<Scalar>::createInitialSpace()
{
    // read mesh from file
    if (!Agros2D::problem()->isMeshed())
        throw AgrosSolverException(QObject::tr("Problem is not meshed"));

    clearActualSpaces();

    m_block->createBoundaryConditions();

    foreach(Field* field, m_block->fields())
    {
        FieldInfo* fieldInfo = field->fieldInfo();


        // create copy of initial mesh, for all components only one mesh
        //        MeshSharedPtr oneInitialMesh(new Hermes::Hermes2D::Mesh());
        //        oneInitialMesh->copy(fieldInfo->initialMesh());

        QMap<int, Module::Space> fieldSpaces = fieldInfo->spaces();

        // create space
        for (int i = 0; i < fieldInfo->numberOfSolutions(); i++)
        {
            // spaces in module are numbered from 1!
            int spaceI = i + 1;
            assert(fieldSpaces.contains(spaceI));

            Space<Scalar> *oneSpace = NULL;
            switch (fieldSpaces[spaceI].type())
            {
            case HERMES_L2_SPACE:
                oneSpace = new L2Space<Scalar>(fieldInfo->initialMesh(), fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt() + fieldInfo->spaces()[i+1].orderAdjust());
                break;
            case HERMES_H1_SPACE:
                oneSpace = new H1Space<Scalar>(fieldInfo->initialMesh(), m_block->bcs().at(i + m_block->offset(field)), fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt() + fieldInfo->spaces()[i+1].orderAdjust());
                break;
            case HERMES_HCURL_SPACE:
                oneSpace = new HcurlSpace<Scalar>(fieldInfo->initialMesh(), m_block->bcs().at(i + m_block->offset(field)), fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt() + fieldInfo->spaces()[i+1].orderAdjust());
                break;
            case HERMES_HDIV_SPACE:
                oneSpace = new HdivSpace<Scalar>(fieldInfo->initialMesh(), m_block->bcs().at(i + m_block->offset(field)), fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt() + fieldInfo->spaces()[i+1].orderAdjust());
                break;
            default:
                assert(0);
                break;
            }

            // cout << "Space " << i << "dofs: " << actualSpace->get_num_dofs() << endl;
            m_actualSpaces.push_back(oneSpace);

            // set order by element
            foreach(SceneLabel* label, Agros2D::scene()->labels->items())
            {
                if (!label->marker(fieldInfo)->isNone() &&
                        (fieldInfo->labelPolynomialOrder(label) != fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt()))
                {
                    oneSpace->set_uniform_order(fieldInfo->labelPolynomialOrder(label),
                                                QString::number(Agros2D::scene()->labels->items().indexOf(label)).toStdString());
                }
            }

            oneSpace->assign_dofs();
        }

        // delete temp initial mesh
        // delete initialMesh;
    }

    assert(!m_hermesSolverContainer);
    m_hermesSolverContainer = HermesSolverContainer<Scalar>::factory(m_block);

    m_hermesSolverContainer->setWeakFormulation(m_block->weakForm());
    m_hermesSolverContainer->setTableSpaces()->set_spaces(m_actualSpaces);
}

template <typename Scalar>
void ProblemSolver<Scalar>::solveReferenceAndProject(int timeStep, int adaptivityStep)
{
    //    SolutionMode solutionMode = solutionExists ? SolutionMode_Normal : SolutionMode_NonExisting;
    //    MultiSolutionArray<Scalar> msa = Agros2D::solutionStore()->multiSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, solutionMode));
    //    MultiSolutionArray<Scalar> msaRef;

    if (Agros2D::problem()->isTransient())
    {
        if((adaptivityStep == 0) && (timeStep > 1))
        {
            // when timeStep == 1 and then each adaptivityRedonenEach time steps start adaptivity from the initial mesh
            if ((timeStep - 1) % m_block->adaptivityRedoneEach() == 0)
            {
                assert(timeStep != 0);
                BlockSolutionID solID(m_block, 1, 0, SolutionMode_Normal);
                MultiArray<Scalar> msaPrevTS = Agros2D::solutionStore()->multiArray(solID);

                setActualSpaces(msaPrevTS.spaces());
                //setActualSpaces(deepMeshAndSpaceCopy(msaPrevTS.spaces(), false));
            }
            // otherwise do not start over, but use space from the previous time level
            // do not use the last adaptation, substract adaptivityBackSteps from it
            else
            {
                int lastTimeStepNumAdaptations = Agros2D::solutionStore()->lastAdaptiveStep(m_block, SolutionMode_Normal, timeStep - 1);
                BlockSolutionID solID(m_block, timeStep-1, max(lastTimeStepNumAdaptations - m_block->adaptivityBackSteps(), 0), SolutionMode_Normal);
                MultiArray<Scalar> msaPrevTS = Agros2D::solutionStore()->multiArray(solID);

                setActualSpaces(msaPrevTS.spaces());
                //setActualSpaces(deepMeshAndSpaceCopy(msaPrevTS.spaces(), false));
            }
        }

        // todo: to be used as starting vector for the Newton solver
        //        MultiSolutionArray<Scalar> previousTSMultiSolutionArray;
        //        if((m_block->isTransient() && m_block->linearityType() != LinearityType_Linear) && (timeStep > 0))
        //            previousTSMultiSolutionArray = Agros2D::solutionStore()->multiSolutionPreviousCalculatedTS(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal));
    }

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(actualSpaces()) == 0)
    {
        Agros2D::log()->printDebug(m_solverID, QObject::tr("DOF is zero"));
        throw(AgrosSolverException(QObject::tr("DOF is zero")));
    }

    // update timedep values
    foreach (Field* field, m_block->fields())
        Module::updateTimeFunctions(Agros2D::problem()->actualTime());
    m_block->updateExactSolutionFunctions();

    // todo: delete? delam to pro referencni... (zkusit)
    Hermes::vector<SpaceSharedPtr<Scalar> > spaces = actualSpaces();
    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(spaces, Agros2D::problem()->actualTime());

    if(m_block->isTransient())
    {
        int order = min(timeStep, Agros2D::problem()->config()->value(ProblemConfig::TimeOrder).toInt());
        bool matrixUnchanged = m_block->weakForm()->bdf2Table()->setOrderAndPreviousSteps(order, Agros2D::problem()->timeStepLengths());
        m_hermesSolverContainer->matrixUnchangedDueToBDF(matrixUnchanged);
    }

    m_block->weakForm()->set_current_time(Agros2D::problem()->actualTime());
    m_block->weakForm()->updateExtField();

    // create reference spaces
    Hermes::vector<SpaceSharedPtr<Scalar> > spacesRef = deepMeshAndSpaceCopy(actualSpaces(), true);
    assert(actualSpaces().size() == spacesRef.size());

    // todo: delete? je to vubec potreba?
    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(spacesRef, Agros2D::problem()->actualTime());

    // solve reference problem

    // in adaptivity, in each step we use different spaces. This should be done some other way
    m_hermesSolverContainer->setTableSpaces()->set_spaces(spacesRef);
    Scalar *solutionVector = solveOneProblem(spacesRef, adaptivityStep,
                                             Hermes::vector<MeshFunctionSharedPtr<Scalar> >());

    // output reference solution
    Hermes::vector<MeshSharedPtr> meshesRef = spacesMeshes(spacesRef);
    Hermes::vector<MeshFunctionSharedPtr<Scalar> > solutionsRef = createSolutions<Scalar>(meshesRef);
    Solution<Scalar>::vector_to_solutions(solutionVector, spacesRef, solutionsRef);

    BlockSolutionID referenceSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Reference);
    SolutionStore::SolutionRunTimeDetails runTimeRef(Agros2D::problem()->actualTimeStepLength(),
                                                     0.0,
                                                     Hermes::Hermes2D::Space<double>::get_num_dofs(spacesRef));
    Agros2D::solutionStore()->addSolution(referenceSolutionID, MultiArray<Scalar>(spacesRef, solutionsRef), runTimeRef);

    // copy spaces and create empty solutions
    //Hermes::vector<SpaceSharedPtr<Scalar> > spacesCopy = deepMeshAndSpaceCopy(actualSpaces(), false);
    Hermes::vector<MeshSharedPtr> meshes = spacesMeshes(actualSpaces());
    Hermes::vector<MeshFunctionSharedPtr<Scalar> > solutions = createSolutions<Scalar>(meshes);

    // project the fine mesh solution onto the coarse mesh.
    Hermes::Hermes2D::OGProjection<Scalar> ogProjection;
    ogProjection.project_global(actualSpaces(), solutionsRef, solutions);

    // save the solution
    BlockSolutionID solutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal);
    SolutionStore::SolutionRunTimeDetails runTime(Agros2D::problem()->actualTimeStepLength(),
                                                  0.0,
                                                  Hermes::Hermes2D::Space<double>::get_num_dofs(actualSpaces()));
    if (dynamic_cast<NewtonSolverContainer<Scalar> *>(m_hermesSolverContainer.data()))
    {
        NewtonSolverAgros<Scalar> *solver = dynamic_cast<NewtonSolverContainer<Scalar> *>(m_hermesSolverContainer.data())->solver();

        runTime.setNewtonResidual(solver->errors());
        runTime.setNewtonDamping(solver->damping());
    }

    MultiArray<Scalar> msa(actualSpaces(), solutions);
    Agros2D::solutionStore()->addSolution(solutionID, msa, runTime);
}

template <typename Scalar>
bool ProblemSolver<Scalar>::createAdaptedSpace(int timeStep, int adaptivityStep, bool forceAdaptation)
{
    MultiArray<Scalar> msa = Agros2D::solutionStore()->multiArray(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionMode_Normal));
    MultiArray<Scalar> msaRef = Agros2D::solutionStore()->multiArray(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionMode_Reference));

    Hermes::vector<Hermes::Hermes2D::NormType> projNormType;
    Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *> selector;
    initSelectors(projNormType, selector);

    assert(msa.spaces() == actualSpaces());

    // this should be the only place, where we use deep mesh and space copy
    // not counting creating of reference space
    // we have to use it here, since we are going to change the space through adaptivity
    setActualSpaces(deepMeshAndSpaceCopy(actualSpaces(), false));

    // error calculation & adaptivity.
    DefaultErrorCalculator<double, HERMES_H1_NORM> errorCalculator(RelativeErrorToGlobalNorm, m_actualSpaces.size());
    // stopping criterion for an adaptivity step.
    AdaptivityStoppingCriterion<Scalar> *stopingCriterion = NULL;
    switch (m_block->adaptivityStoppingCriterionType())
    {
    case AdaptivityStoppingCriterionType_Cumulative:
        stopingCriterion = new AdaptStoppingCriterionCumulative<double>(m_block->adaptivityThreshold());
        break;
    case AdaptivityStoppingCriterionType_SingleElement:
        stopingCriterion = new AdaptStoppingCriterionSingleElement<double>(m_block->adaptivityThreshold());
        break;
    case AdaptivityStoppingCriterionType_Levels:
        stopingCriterion = new AdaptStoppingCriterionLevels<double>(m_block->adaptivityThreshold());
        break;
    default:
        assert(0);
        break;
    }

    // adaptivity
    Adapt<Scalar> adaptivity(&errorCalculator, stopingCriterion);
    adaptivity.set_spaces(m_actualSpaces);
    adaptivity.set_verbose_output(false);

    // calculate error the total error estimate.
    errorCalculator.calculate_errors(msa.solutions(), msaRef.solutions(), true);
    double error = errorCalculator.get_total_error_squared() * 100;
    // update error in solution store
    foreach (Field *field, m_block->fields())
    {
        FieldSolutionID solutionID(field->fieldInfo(), timeStep, adaptivityStep - 1, SolutionMode_Normal);

        // get run time
        SolutionStore::SolutionRunTimeDetails runTime = Agros2D::solutionStore()->multiSolutionRunTimeDetail(solutionID);
        // set error
        runTime.setAdaptivityError(error);
        // replace runtime
        Agros2D::solutionStore()->multiSolutionRunTimeDetailReplace(solutionID, runTime);
    }

    // todo: otazku zda uz neni moc dofu jsem mel vyresit nekde drive
    bool adapt = error >= m_block->adaptivityTolerance() && Hermes::Hermes2D::Space<Scalar>::get_num_dofs(actualSpaces())
            < Agros2D::problem()->setting()->value(ProblemSetting::Adaptivity_MaxDofs).toInt();

    // allways adapt when forcing adaptation, to be used in solveAdaptiveStep
    adapt = adapt || forceAdaptation;

    if (adapt)
    {
        Agros2D::log()->printMessage(m_solverID, QObject::tr("Adaptivity step (error = %1, DOFs = %2/%3)").
                                     arg(error).
                                     arg(Space<Scalar>::get_num_dofs(msa.spaces())).
                                     arg(Space<Scalar>::get_num_dofs(msaRef.spaces())));

        bool noRefinementPerformed;
        try
        {
            noRefinementPerformed = adaptivity.adapt(selector);
        }
        catch (Hermes::Exceptions::Exception e)
        {
            deleteSelectors(selector);
            delete stopingCriterion;

            QString error = QString(e.what());
            Agros2D::log()->printDebug(m_solverID, QObject::tr("Adaptive process failed: %1").arg(error));
            throw;
        }

        adapt = adapt && (!noRefinementPerformed);
    }

    deleteSelectors(selector);
    delete stopingCriterion;

    return adapt;
}

template <typename Scalar>
void ProblemSolver<Scalar>::solveInitialTimeStep()
{
    Agros2D::log()->printDebug(m_solverID, QObject::tr("Initial time step"));

    //Hermes::vector<SpaceSharedPtr<Scalar> > spaces = deepMeshAndSpaceCopy(actualSpaces(), false);
    Hermes::vector<MeshFunctionSharedPtr<Scalar> > solutions;

    int totalComp = 0;
    foreach(Field* field, m_block->fields())
    {
        for (int comp = 0; comp < field->fieldInfo()->numberOfSolutions(); comp++)
        {
            // constant initial solution
            MeshSharedPtr mesh = actualSpaces().at(totalComp)->get_mesh();
            ConstantSolution<double> *initial = new ConstantSolution<double>(mesh, field->fieldInfo()->value(FieldInfo::TransientInitialCondition).toDouble());
            solutions.push_back(initial);
            totalComp++;
        }
    }

    QList<SolutionStore::SolutionRunTimeDetails::FileName> fileNames;

    BlockSolutionID solutionID(m_block, 0, 0, SolutionMode_Normal);
    SolutionStore::SolutionRunTimeDetails runTime(Agros2D::problem()->actualTimeStepLength(),
                                                  0.0,
                                                  Hermes::Hermes2D::Space<double>::get_num_dofs(actualSpaces()));

    Agros2D::solutionStore()->addSolution(solutionID,
                                          MultiArray<Scalar>(actualSpaces(), solutions),
                                          runTime);
}

template <typename Scalar>
void ProblemSolver<Scalar>::resumeAdaptivityProcess(int adaptivityStep)
{
    BlockSolutionID solID(m_block, 0, adaptivityStep, SolutionMode_Normal);
    MultiArray<Scalar> msa = Agros2D::solutionStore()->multiArray(solID);

    setActualSpaces(msa.spaces());

    assert(!m_hermesSolverContainer);
    m_hermesSolverContainer = HermesSolverContainer<Scalar>::factory(m_block);

}


//template class VectorStore<double>;
template class LinearSolverContainer<double>;
template class NewtonSolverContainer<double>;
template class PicardSolverContainer<double>;
template class ProblemSolver<double>;
