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

// deal.ii
#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>

#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_in.h>
#include <deal.II/grid/grid_reordering.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/dofs/dof_accessor.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_values.h>
#include <deal.II/base/quadrature_lib.h>

#include <deal.II/base/function.h>
#include <deal.II/numerics/vector_tools.h>
#include <deal.II/numerics/matrix_tools.h>
#include <deal.II/numerics/error_estimator.h>

#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/compressed_sparsity_pattern.h>
#include <deal.II/lac/solver_cg.h>
#include <deal.II/lac/sparse_direct.h>
#include <deal.II/lac/precondition.h>

#include <deal.II/grid/grid_refinement.h>
#include <deal.II/numerics/error_estimator.h>

#include <deal.II/numerics/fe_field_function.h>
#include <deal.II/numerics/data_out.h>

#include <streambuf>
#include <sstream>

#include "solver.h"
// #include "solver_linear.h"
// #include "solver_newton.h"
// #include "solver_picard.h"

#include "util.h"
#include "util/global.h"

#include "field.h"
#include "problem.h"
#include "hermes2d/problem_config.h"
//#include "module.h"
#include "scene.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "solutionstore.h"
#include "plugin_interface.h"
#include "logview.h"
#include "bdf2.h"
#include "plugin_interface.h"
#include "weak_form.h"

#include "pythonlab/pythonengine.h"

SolverDeal::SolverDeal(const FieldInfo *fieldInfo, int initialOrder)
    : m_fieldInfo(fieldInfo), m_solution_previous(NULL)
{    
    // fe
    qDebug() << "SolverDeal::SolverDeal: numberOfSolutions" << fieldInfo->numberOfSolutions();
    // m_fe = new dealii::FESystem<2>(dealii::FE_Q<2>(initialOrder), 1);
    m_fe = new dealii::FESystem<2>(dealii::FE_Q<2>(initialOrder), fieldInfo->numberOfSolutions());

    // copy initial mesh
    m_triangulation = new dealii::Triangulation<2>();
    m_triangulation->copy_triangulation(*m_fieldInfo->initialMesh());

    // info
    // std::vector<dealii::types::boundary_id> bindicators = m_triangulation->get_boundary_indicators();
    // std::cout << "Number of boundary indicators: " << bindicators.size() << std::endl;
    // std::cout << "Number of active cells: " << m_triangulation->n_active_cells() << std::endl;
    // std::cout << "Total number of cells: " << m_triangulation->n_cells() << std::endl;

    // create dof handler
    m_doFHandler = new dealii::DoFHandler<2>(*m_triangulation);

    // create solution vector
    m_solution = new dealii::Vector<double>();
}

SolverDeal::~SolverDeal()
{
    // delete m_triangulation;
    // delete m_fe;
}

void SolverDeal::setup()
{
    QTime time;
    time.start();

    m_doFHandler->distribute_dofs(*m_fe);

    // reinit sln and rhs
    system_rhs.reinit(m_doFHandler->n_dofs());
    m_solution->reinit(m_doFHandler->n_dofs());

    std::cout << "Number of degrees of freedom: " << m_doFHandler->n_dofs() << std::endl;

    hanging_node_constraints.clear ();
    dealii::DoFTools::make_hanging_node_constraints(*m_doFHandler,
                                                    hanging_node_constraints);
    hanging_node_constraints.close();

    // dealii::CompressedSparsityPattern c_sparsity(m_doFHandler->n_dofs());
    sparsity_pattern.reinit(m_doFHandler->n_dofs(),
                            m_doFHandler->n_dofs(),
                            m_doFHandler->max_couplings_between_dofs());

    dealii::DoFTools::make_sparsity_pattern(*m_doFHandler,
                                            sparsity_pattern);

    // sparsity_pattern.copy_from(c_sparsity);

    hanging_node_constraints.condense(sparsity_pattern);
    sparsity_pattern.compress();

    // sparsity_pattern.compress();
    system_matrix.reinit(sparsity_pattern);

    qDebug() << "setup (" << time.elapsed() << "ms )";
}

void SolverDeal::assembleSystem()
{
    assert(0);
}

void SolverDeal::assembleDirichlet()
{
    assert(0);
}

void SolverDeal::solve()
{
    qDebug() << "residual" << system_rhs.l2_norm();

    QTime time;
    time.start();

    solveUMFPACK();
    // solveCG();

    hanging_node_constraints.distribute(*m_solution);

    // copy solution
    if (Agros2D::problem()->isTransient() || Agros2D::problem()->isNonlinear())
        m_solution_previous = new dealii::Vector<double>(*m_solution);

    qDebug() << "solved (" << time.elapsed() << "ms )";
}

void SolverDeal::solveUMFPACK()
{
    dealii::SparseDirectUMFPACK direct;
    direct.initialize(system_matrix);
    direct.vmult(*m_solution, system_rhs);
}

void SolverDeal::solveCG()
{
    dealii::SolverControl solver_control(10000, 1e-13);
    dealii::SolverCG<> solver(solver_control);
    solver.solve(system_matrix, *m_solution, system_rhs, dealii::PreconditionIdentity());
}


void SolverAgros::clearSteps()
{
    m_steps.clear();
    m_damping.clear();
    m_residualNorms.clear();
    m_solutionNorms.clear();
}

/*
void AgrosExternalSolverExternal::solve(double* initial_guess)
{
    initialGuess = initial_guess;

    fileMatrix = QString("%1/solver_matrix").arg(cacheProblemDir());
    fileRHS = QString("%1/solver_rhs").arg(cacheProblemDir());
    fileInitial = QString("%1/solver_initial").arg(cacheProblemDir());
    fileSln = QString("%1/solver_sln").arg(cacheProblemDir());

    this->set_matrix_export_format(EXPORT_FORMAT_BSON);
    this->set_matrix_filename(fileMatrix.toStdString());
    this->set_matrix_varname("matrix");
    this->set_matrix_number_format((char *) "%g");
    this->set_rhs_export_format(EXPORT_FORMAT_BSON);
    this->set_rhs_filename(fileRHS.toStdString());
    this->set_rhs_varname("rhs");
    this->set_rhs_number_format((char *) "%g");

    // store state
    bool matrixOn = this->output_matrixOn;
    bool rhsOn = this->output_rhsOn;
    this->output_matrixOn = true;
    this->output_rhsOn = true;

    // write matrix and rhs to disk
    // QTime time;
    // time.start();
    this->process_matrix_output(this->m);
    // qDebug() << "process_matrix_output" << time.elapsed();
    // time.start();
    this->process_vector_output(this->rhs);
    // qDebug() << "process_vector_output" << time.elapsed();

    // write initial guess to disk
    if (initialGuess)
    {
        SimpleVector<double> initialVector;
        initialVector.alloc(rhs->get_size());
        initialVector.set_vector(initialGuess);
        initialVector.export_to_file(fileInitial.toStdString().c_str(),
                                     (char *) "initial",
                                     EXPORT_FORMAT_BSON,
                                     (char *) "%lf");
        initialVector.free();
    }

    if (!(Agros2D::problem()->isTransient() || Agros2D::problem()->isNonlinear()))
        this->m->free();
    this->rhs->free();

    // restore state
    this->output_matrixOn = matrixOn;
    this->output_rhsOn = rhsOn;

    // exec octave
    m_process = new QProcess();
    m_process->setStandardOutputFile(tempProblemDir() + "/solver.out");
    m_process->setStandardErrorFile(tempProblemDir() + "/solver.err");
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
    connect(m_process, SIGNAL(finished(int)), this, SLOT(processFinished(int)));

    setSolverCommand();
    m_process->start(command);

    // execute an event loop to process the request (nearly-synchronous)
    QEventLoop eventLoop;
    QObject::connect(m_process, SIGNAL(finished(int)), &eventLoop, SLOT(quit()));
    QObject::connect(m_process, SIGNAL(error(QProcess::ProcessError)), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    SimpleVector<double> slnVector;
    // time.start();
    slnVector.import_from_file((char*) fileSln.toStdString().c_str(), "sln", EXPORT_FORMAT_BSON);
    // qDebug() << "slnVector import_from_file" << time.elapsed();

    delete [] this->sln;
    this->sln = new double[slnVector.get_size()];
    memcpy(this->sln, slnVector.v, slnVector.get_size() * sizeof(double));

    QFile::remove(command);
    if (initialGuess)
        QFile::remove(fileInitial);
    QFile::remove(fileMatrix);
    QFile::remove(fileRHS);
    QFile::remove(fileSln);

    QFile::remove(tempProblemDir() + "/solver.out");
    QFile::remove(tempProblemDir() + "/solver.err");
}
*/

/*
    QSharedPointer<HermesSolverContainer<Scalar> > HermesSolverContainer<Scalar>::factory(Block* block)

    QString solverName;
    QListIterator<FieldBlock*> iter(block->fields());
    while (iter.hasNext())
    {
        solverName += iter.next()->fieldInfo()->fieldId();
        if (iter.hasNext())
            solverName += ", ";
    }

    HermesCommonApi.set_integral_param_value(matrixSolverType, block->matrixSolver());
    Agros2D::log()->printDebug(QObject::tr("Solver (%1)").arg(solverName), QObject::tr("Linear solver: %1").arg(matrixSolverTypeString(block->matrixSolver())));

    if (block->matrixSolver() == SOLVER_EXTERNAL)
    {
        // register external solver
        ExternalSolver<double>::create_external_solver = getExternalSolver;
    }

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

    solver->linearSolver()->set_verbose_output(false);

    assert(!solver.isNull());

    if (LoopSolver<Scalar> *linearSolver = dynamic_cast<LoopSolver<Scalar> *>(solver->linearSolver()))
    {
        linearSolver->set_max_iters(block->iterLinearSolverIters());
        linearSolver->set_tolerance(block->iterLinearSolverToleranceAbsolute());
    }
    if (IterativeParalutionLinearMatrixSolver<Scalar> *linearSolver = dynamic_cast<IterativeParalutionLinearMatrixSolver<Scalar> *>(solver.data()->linearSolver()))
    {
        linearSolver->set_solver_type(block->iterLinearSolverType());
        linearSolver->set_precond(new Preconditioners::ParalutionPrecond<Scalar>(block->iterPreconditionerType()));
    }
    if (AMGParalutionLinearMatrixSolver<Scalar> *linearSolver = dynamic_cast<AMGParalutionLinearMatrixSolver<Scalar> *>(solver.data()->linearSolver()))
    {
        linearSolver->set_smoother(block->iterLinearSolverType(), block->iterPreconditionerType());
    }

    return solver;
*/

/*
template <typename Scalar>
void HermesSolverContainer<Scalar>::projectPreviousSolution(Scalar* solutionVector,
                                                            std::vector<SpaceSharedPtr<Scalar> > spaces,
                                                            std::vector<MeshFunctionSharedPtr<Scalar> > solutions)
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
*/

template <typename Scalar>
ProblemSolver<Scalar>::~ProblemSolver()
{

}

/*
template <typename Scalar>
void ProblemSolver<Scalar>::init()
{
    m_block = block;

    QListIterator<FieldBlock*> iter(m_block->fields());
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
*/
/*
template <typename Scalar>
Scalar *ProblemSolver<Scalar>::solveOneProblem(std::vector<SpaceSharedPtr<Scalar> > spaces,
                                               int adaptivityStep,
                                               std::vector<MeshFunctionSharedPtr<Scalar> > previousSolution)
{
    LinearMatrixSolver<Scalar> *linearSolver = m_hermesSolverContainer->linearSolver();

    // if (m_block->isTransient())
    //     linearSolver->set_reuse_scheme(HERMES_REUSE_MATRIX_REORDERING);

    m_hermesSolverContainer->setMatrixRhsOutput(m_solverCode, adaptivityStep);

    if (LoopSolver<Scalar> *iterLinearSolver = dynamic_cast<LoopSolver<Scalar> *>(linearSolver))
    {
        // iterative solver
        Scalar *initialSolutionVector = new Scalar[Space<Scalar>::get_num_dofs(spaces)];
        m_hermesSolverContainer->projectPreviousSolution(initialSolutionVector, spaces, previousSolution);
        m_hermesSolverContainer->solve(initialSolutionVector);

        delete [] initialSolutionVector;

        Agros2D::log()->printDebug(QObject::tr("Solver"),
                                   QObject::tr("Iterative solver statistics: %1 iterations, residual %2")
                                   .arg(iterLinearSolver->get_num_iters())
                                   .arg(iterLinearSolver->get_residual_norm()));
    }
    else
    {
        // direct solver
        m_hermesSolverContainer->solve(nullptr);
    }

    return m_hermesSolverContainer->slnVector();
}
*/

template <typename Scalar>
void ProblemSolver<Scalar>::solveSimple(int timeStep, int adaptivityStep)
{
    /*
    // to be used as starting vector for the Newton solver
    MultiArray<Scalar> previousTSMultiSolutionArray;
    // if ((m_block->isTransient() && m_block->linearityType() != LinearityType_Linear) && (timeStep > 0))
    if ((m_block->isTransient()) && (timeStep > 0))
        previousTSMultiSolutionArray = Agros2D::solutionStore()->multiSolutionPreviousCalculatedTS(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal));

    // check for DOFs
    int ndof = Space<Scalar>::get_num_dofs(actualSpaces());
    if (ndof == 0)
    {
        Agros2D::log()->printError(m_solverID, QObject::tr("DOF is zero"));
        throw (AgrosSolverException("DOF is zero"));
    }
    Agros2D::log()->printDebug(m_solverID, QObject::tr("Number of DOFs: %1").arg(ndof));

    // cout << QString("updating with time %1\n").arg(Agros2D::problem()->actualTime()).toStdString() << endl;
    m_block->updateExactSolutionFunctions();

    std::vector<SpaceSharedPtr<Scalar> > spaces = actualSpaces();
    Space<Scalar>::update_essential_bc_values(spaces, Agros2D::problem()->actualTime());

    if (m_block->isTransient())
    {
        int order = min(timeStep, Agros2D::problem()->config()->value(ProblemConfig::TimeOrder).toInt());
        bool matrixUnchanged = m_block->weakFormInternal()->bdf2Table()->setOrderAndPreviousSteps(order, Agros2D::problem()->timeStepLengths());
        m_hermesSolverContainer->matrixUnchangedDueToBDF(matrixUnchanged);

        // update timedep values
        Module::updateTimeFunctions(Agros2D::problem()->actualTime());
    }

    m_block->weakFormInternal()->set_current_time(Agros2D::problem()->actualTime());
    m_block->weakFormInternal()->updateExtField();

    try
    {
        Scalar *solutionVector = solveOneProblem(actualSpaces(), adaptivityStep,
                                                 previousTSMultiSolutionArray.solutions());

        // output
        std::vector<MeshFunctionSharedPtr<Scalar> > solutions = createSolutions<Scalar>(spacesMeshes(actualSpaces()));
        Solution<Scalar>::vector_to_solutions(solutionVector, actualSpaces(), solutions);

        BlockSolutionID solutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal);
        SolutionStore::SolutionRunTimeDetails runTime(Agros2D::problem()->actualTimeStepLength(),
                                                      0.0,
                                                      Space<double>::get_num_dofs(actualSpaces()));

        SolverAgros *solver = m_hermesSolverContainer.data()->solver();
        runTime.setNewtonResidual(solver->residualNorms());
        runTime.setNonlinearDamping(solver->damping());
        runTime.setJacobianCalculations(solver->jacobianCalculations());
        runTime.setRelativeChangeOfSolutions(solver->relativeChangeOfSolutions());

        Agros2D::solutionStore()->addSolution(solutionID, MultiArray<Scalar>(actualSpaces(), solutions), runTime);

        // dealii
        FieldBlock *fieldBlock = m_block->fields().at(0);
    }
    catch (AgrosSolverException e)
    {
        throw AgrosSolverException(QObject::tr("Solver failed: %1").arg(e.toString()));
    }
    */
}

template <typename Scalar>
TimeStepInfo ProblemSolver<Scalar>::estimateTimeStepLength(int timeStep, int adaptivityStep)
{
    /*
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
    bool matrixUnchanged = m_block->weakFormInternal()->bdf2Table()->setOrderAndPreviousSteps(previouslyUsedOrder - 1, Agros2D::problem()->timeStepLengths());
    // using different order
    assert(matrixUnchanged == false);
    m_hermesSolverContainer->matrixUnchangedDueToBDF(matrixUnchanged);
    m_block->weakFormInternal()->set_current_time(Agros2D::problem()->actualTime());
    m_block->weakFormInternal()->updateExtField();

    // solutions obtained by time method of higher order in the original calculation
    std::vector<MeshFunctionSharedPtr<Scalar> > timeReferenceSolution;
    if(timeStep > 0)
        timeReferenceSolution = referenceCalculation.solutions();
    Scalar *solutionVector = solveOneProblem(actualSpaces(), adaptivityStep, timeReferenceSolution);

    std::vector<MeshSharedPtr> meshes = spacesMeshes(actualSpaces());
    std::vector<MeshFunctionSharedPtr<Scalar> > solutions = createSolutions<Scalar>(meshes);
    Solution<Scalar>::vector_to_solutions(solutionVector, actualSpaces(), solutions);

    // error calculation
    DefaultErrorCalculator<double, HERMES_H1_NORM> errorCalculator(RelativeErrorToGlobalNorm, solutions.size());
    // calculate error the total error estimate.
    errorCalculator.calculate_errors(referenceCalculation.solutions(), solutions, false);
    double error = errorCalculator.get_total_error_squared();

    // update
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
    double nextTimeStepLength;
    if(error == 0)
        nextTimeStepLength = maxTimeStepLength;
    else
        nextTimeStepLength = pow(TOL / error, 1.0 / (timeOrder + 1)) * Agros2D::problem()->actualTimeStepLength();

    nextTimeStepLength = min(nextTimeStepLength, maxTimeStepLength);
    nextTimeStepLength = min(nextTimeStepLength, Agros2D::problem()->actualTimeStepLength() * maxTimeStepRatio);
    nextTimeStepLength = max(nextTimeStepLength, Agros2D::problem()->actualTimeStepLength() / maxTimeStepRatio);

    Agros2D::log()->printDebug(m_solverID, QString("Time adaptivity, time %1 s, rel. error %2, tolerance %3, step size %4 -> %5 (%6 %), average err/len %7").
                               arg(Agros2D::problem()->actualTime()).
                               arg(error).
                               arg(TOL).
                               arg(Agros2D::problem()->actualTimeStepLength()).
                               arg(nextTimeStepLength).
                               arg(nextTimeStepLength / Agros2D::problem()->actualTimeStepLength()*100.).
                               arg(m_averageErrorToLenghtRatio));
    if(refuseThisStep)
        Agros2D::log()->printMessage(m_solverID, "Transient step refused");

    return TimeStepInfo(nextTimeStepLength, refuseThisStep);
    */
}

template <typename Scalar>
void ProblemSolver<Scalar>::createInitialSpace()
{
    return; // dealii
    /*
    // read mesh from file
    if (!Agros2D::problem()->isMeshed())
        throw AgrosSolverException(QObject::tr("Problem is not meshed"));

    clearActualSpaces();

    m_block->createBoundaryConditions();

    foreach(FieldBlock* field, m_block->fields())
    {
        FieldInfo* fieldInfo = field->fieldInfo();


        // create copy of initial mesh, for all components only one mesh
        //        MeshSharedPtr oneInitialMesh(new Mesh());
        //        oneInitialMesh->copy(fieldInfo->initialMesh());

        QMap<int, Module::Space> fieldSpaces = fieldInfo->spaces();

        // create space
        for (int i = 0; i < fieldInfo->numberOfSolutions(); i++)
        {
            // spaces in module are numbered from 1!
            int spaceI = i + 1;
            assert(fieldSpaces.contains(spaceI));
            SpaceSharedPtr<Scalar> oneSpace;
            SpaceType spaceType = fieldSpaces[spaceI].type();

            switch (spaceType)
            {
            case HERMES_L2_SPACE:
                oneSpace = SpaceSharedPtr<Scalar>(new L2Space<Scalar>(fieldInfo->initialMesh(), fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt() + fieldInfo->spaces()[i+1].orderAdjust()));
                break;
            case HERMES_L2_MARKERWISE_CONST_SPACE:
                oneSpace = SpaceSharedPtr<Scalar>(new L2MarkerWiseConstSpace<Scalar>(fieldInfo->initialMesh()));
                break;
            case HERMES_H1_SPACE:
                oneSpace = SpaceSharedPtr<Scalar>(new H1Space<Scalar>(fieldInfo->initialMesh(), m_block->bcs().at(i + m_block->offset(field)), fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt() + fieldInfo->spaces()[i+1].orderAdjust()));
                break;
            case HERMES_HCURL_SPACE:
                oneSpace = SpaceSharedPtr<Scalar>(new HcurlSpace<Scalar>(fieldInfo->initialMesh(), m_block->bcs().at(i + m_block->offset(field)), fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt() + fieldInfo->spaces()[i+1].orderAdjust()));
                break;
            case HERMES_HDIV_SPACE:
                oneSpace = SpaceSharedPtr<Scalar>(new HdivSpace<Scalar>(fieldInfo->initialMesh(), m_block->bcs().at(i + m_block->offset(field)), fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt() + fieldInfo->spaces()[i+1].orderAdjust()));
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
                    if(spaceType != HERMES_L2_MARKERWISE_CONST_SPACE)
                    {
                        oneSpace->set_uniform_order(fieldInfo->labelPolynomialOrder(label),
                                                    QString::number(Agros2D::scene()->labels->items().indexOf(label)).toStdString());
                    }
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
    */
}

template <typename Scalar>
void ProblemSolver<Scalar>::solveReferenceAndProject(int timeStep, int adaptivityStep)
{
    /*
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
    if (Space<Scalar>::get_num_dofs(actualSpaces()) == 0)
    {
        Agros2D::log()->printDebug(m_solverID, QObject::tr("DOF is zero"));
        throw(AgrosSolverException(QObject::tr("DOF is zero")));
    }

    // update timedep values
    foreach (FieldBlock* field, m_block->fields())
        Module::updateTimeFunctions(Agros2D::problem()->actualTime());
    m_block->updateExactSolutionFunctions();

    // todo: delete? delam to pro referencni... (zkusit)
    std::vector<SpaceSharedPtr<Scalar> > spaces = actualSpaces();
    Space<Scalar>::update_essential_bc_values(spaces, Agros2D::problem()->actualTime());

    if (m_block->isTransient())
    {
        int order = min(timeStep, Agros2D::problem()->config()->value(ProblemConfig::TimeOrder).toInt());
        bool matrixUnchanged = m_block->weakFormInternal()->bdf2Table()->setOrderAndPreviousSteps(order, Agros2D::problem()->timeStepLengths());
        m_hermesSolverContainer->matrixUnchangedDueToBDF(matrixUnchanged);
    }

    m_block->weakFormInternal()->set_current_time(Agros2D::problem()->actualTime());
    m_block->weakFormInternal()->updateExtField();

    // create reference spaces
    std::vector<SpaceSharedPtr<Scalar> > spacesRef = deepMeshAndSpaceCopy(actualSpaces(), true);
    assert(actualSpaces().size() == spacesRef.size());

    // todo: delete? je to vubec potreba?
    Space<Scalar>::update_essential_bc_values(spacesRef, Agros2D::problem()->actualTime());

    // solve reference problem

    // in adaptivity, in each step we use different spaces. This should be done some other way
    m_hermesSolverContainer->setTableSpaces()->set_spaces(spacesRef);
    Scalar *solutionVector = solveOneProblem(spacesRef, adaptivityStep,
                                             std::vector<MeshFunctionSharedPtr<Scalar> >());

    // output reference solution
    std::vector<MeshSharedPtr> meshesRef = spacesMeshes(spacesRef);
    std::vector<MeshFunctionSharedPtr<Scalar> > solutionsRef = createSolutions<Scalar>(meshesRef);
    Solution<Scalar>::vector_to_solutions(solutionVector, spacesRef, solutionsRef);

    BlockSolutionID referenceSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Reference);
    SolutionStore::SolutionRunTimeDetails runTimeRef(Agros2D::problem()->actualTimeStepLength(),
                                                     0.0,
                                                     Space<double>::get_num_dofs(spacesRef));
    Agros2D::solutionStore()->addSolution(referenceSolutionID, MultiArray<Scalar>(spacesRef, solutionsRef), runTimeRef);

    // copy spaces and create empty solutions
    //std::vector<SpaceSharedPtr<Scalar> > spacesCopy = deepMeshAndSpaceCopy(actualSpaces(), false);
    std::vector<MeshSharedPtr> meshes = spacesMeshes(actualSpaces());
    std::vector<MeshFunctionSharedPtr<Scalar> > solutions = createSolutions<Scalar>(meshes);

    // project the fine mesh solution onto the coarse mesh.
    OGProjection<Scalar> ogProjection;
    ogProjection.project_global(actualSpaces(), solutionsRef, solutions);

    // save the solution
    BlockSolutionID solutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal);
    SolutionStore::SolutionRunTimeDetails runTime(Agros2D::problem()->actualTimeStepLength(),
                                                  0.0,
                                                  Space<double>::get_num_dofs(actualSpaces()));

    SolverAgros *solver = m_hermesSolverContainer.data()->solver();
    runTime.setNewtonResidual(solver->residualNorms());
    runTime.setNonlinearDamping(solver->damping());
    runTime.setJacobianCalculations(solver->jacobianCalculations());
    runTime.setRelativeChangeOfSolutions(solver->relativeChangeOfSolutions());

    MultiArray<Scalar> msa(actualSpaces(), solutions);
    Agros2D::solutionStore()->addSolution(solutionID, msa, runTime);
    */
}

template <typename Scalar>
bool ProblemSolver<Scalar>::createAdaptedSpace(int timeStep, int adaptivityStep)
{
    /*
    MultiArray<Scalar> msa = Agros2D::solutionStore()->multiArray(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionMode_Normal));
    MultiArray<Scalar> msaRef = Agros2D::solutionStore()->multiArray(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionMode_Reference));

    std::vector<QSharedPointer<RefinementSelectors::Selector<Scalar> > > selector;
    initSelectors(selector);

    assert(msa.spaces() == actualSpaces());

    // this should be the only place, where we use deep mesh and space copy
    // not counting creating of reference space
    // we have to use it here, since we are going to change the space through adaptivity
    setActualSpaces(deepMeshAndSpaceCopy(actualSpaces(), false));

    // stopping criterion for an adaptivity step.
    QSharedPointer<AdaptivityStoppingCriterion<double> > stopingCriterion;
    switch (m_block->adaptivityStoppingCriterionType())
    {
    case AdaptivityStoppingCriterionType_Cumulative:
        stopingCriterion = QSharedPointer<AdaptivityStoppingCriterion<double> >(new AdaptStoppingCriterionCumulative<double>(m_block->adaptivityThreshold()));
        break;
    case AdaptivityStoppingCriterionType_SingleElement:
        stopingCriterion = QSharedPointer<AdaptivityStoppingCriterion<double> >(new AdaptStoppingCriterionSingleElement<double>(m_block->adaptivityThreshold()));
        break;
    case AdaptivityStoppingCriterionType_Levels:
        stopingCriterion = QSharedPointer<AdaptivityStoppingCriterion<double> >(new AdaptStoppingCriterionLevels<double>(m_block->adaptivityThreshold()));
        break;
    default:
        assert(0);
        break;
    }

    // TODO: hard coupling
    bool adapt = false;

    // update error in solution store
    foreach (FieldBlock *field, m_block->fields())
    {
        // error calculation & adaptivity.
        QSharedPointer<ErrorCalculator<double> > errorCalculator = QSharedPointer<ErrorCalculator<double> >(
                    field->fieldInfo()->plugin()->errorCalculator(field->fieldInfo(),
                                                                  field->fieldInfo()->value(FieldInfo::AdaptivityErrorCalculator).toString(),
                                                                  RelativeErrorToGlobalNorm));

        // adaptivity
        Adapt<Scalar> adaptivity(errorCalculator.data(), stopingCriterion.data());
        adaptivity.set_spaces(m_actualSpaces);
        adaptivity.set_verbose_output(false);

        // calculate error the total error estimate.
        errorCalculator.data()->calculate_errors(msa.solutions(), msaRef.solutions(), true);
        double error = errorCalculator.data()->get_total_error_squared() * 100;

        FieldSolutionID solutionID(field->fieldInfo(), timeStep, adaptivityStep - 1, SolutionMode_Normal);

        // get run time
        SolutionStore::SolutionRunTimeDetails runTime = Agros2D::solutionStore()->multiSolutionRunTimeDetail(solutionID);
        // set error
        runTime.setAdaptivityError(error);
        // replace runtime
        Agros2D::solutionStore()->multiSolutionRunTimeDetailReplace(solutionID, runTime);

        // adaptive tolerance
        adapt = error >= m_block->adaptivityTolerance();

        Agros2D::log()->printMessage(m_solverID, QObject::tr("Adaptivity step: %1 (error: %2, DOFs: %3/%4)").
                                     arg(adaptivityStep).
                                     arg(error).
                                     arg(Space<Scalar>::get_num_dofs(msa.spaces())).
                                     arg(Space<Scalar>::get_num_dofs(msaRef.spaces())));

        Agros2D::log()->updateAdaptivityChartInfo(field->fieldInfo(), timeStep, adaptivityStep);

        if (adapt)
        {
            bool noRefinementPerformed;
            try
            {
                std::vector<RefinementSelectors::Selector<Scalar> *> vect;
                foreach (QSharedPointer<RefinementSelectors::Selector<Scalar> > sel, selector)
                    vect.push_back(sel.data());

                noRefinementPerformed = adaptivity.adapt(vect);
            }
            catch (AgrosException e)
            {
                QString error = QString(e.what());
                Agros2D::log()->printDebug(m_solverID, QObject::tr("Adaptive process failed: %1").arg(error));
                throw;
            }

            adapt = adapt && (!noRefinementPerformed);
        }
    }

    return adapt;
    */
}

template <typename Scalar>
void ProblemSolver<Scalar>::solveInitialTimeStep()
{
    /*
    Agros2D::log()->printDebug(m_solverID, QObject::tr("Initial time step"));

    //std::vector<SpaceSharedPtr<Scalar> > spaces = deepMeshAndSpaceCopy(actualSpaces(), false);
    std::vector<MeshFunctionSharedPtr<Scalar> > solutions;

    int totalComp = 0;
    foreach(FieldBlock* field, m_block->fields())
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
                                                  Space<double>::get_num_dofs(actualSpaces()));

    Agros2D::solutionStore()->addSolution(solutionID,
                                          MultiArray<Scalar>(actualSpaces(), solutions),
                                          runTime);
                                          */
}

template <typename Scalar>
void ProblemSolver<Scalar>::resumeAdaptivityProcess(int adaptivityStep)
{
    /*
    BlockSolutionID solID(m_block, 0, adaptivityStep, SolutionMode_Normal);
    MultiArray<Scalar> msa = Agros2D::solutionStore()->multiArray(solID);

    setActualSpaces(msa.spaces());

    assert(!m_hermesSolverContainer);
    m_hermesSolverContainer = HermesSolverContainer<Scalar>::factory(m_block);
    */
}

ProblemSolverDeal::ProblemSolverDeal() : m_solverDeal(NULL)
{

}


void ProblemSolverDeal::init()
{
    if (m_solverDeal)
        delete m_solverDeal;

    // TODO: more fields
    foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        m_solverDeal = fieldInfo->plugin()->solverDeal(fieldInfo,
                                                       fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt());
    }
}

void ProblemSolverDeal::solve(int timeStep)
{
    // TODO: more fields
    foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        if (fieldInfo->adaptivityType() == AdaptivityMethod_None)
        {
            if (fieldInfo->linearityType() == LinearityType_Linear)
                solveLinear(fieldInfo, timeStep);
            else
                solveNonlinear(fieldInfo, timeStep);
        }
        else
        {
            solveAdaptive(fieldInfo, timeStep);
        }
    }
}

void ProblemSolverDeal::solveLinear(FieldInfo *fieldInfo, int timeStep, int adaptiveStep)
{
    m_solverDeal->setup();
    QTime time;
    time.start();
    m_solverDeal->assembleSystem();
    m_solverDeal->assembleDirichlet();
    qDebug() << "assemble (" << time.elapsed() << "ms )";
    m_solverDeal->solve();

    qDebug() << "solve linear (" << time.elapsed() << "ms )";

    FieldSolutionID solutionID(fieldInfo, timeStep, adaptiveStep, SolutionMode_Normal);
    SolutionStore::SolutionRunTimeDetails runTime(Agros2D::problem()->actualTimeStepLength(), 0.0, 0);

    Agros2D::solutionStore()->addSolution(solutionID, MultiArray(m_solverDeal->doFHandler(), m_solverDeal->solution()), runTime);
}

void ProblemSolverDeal::solveNonlinear(FieldInfo *fieldInfo, int timeStep, int adaptiveStep)
{
    QTime time;
    time.start();

    QVector<double> steps;
    QVector<double> relativeChangeOfSolutions;

    m_solverDeal->setup();
    for (int iteration = 0; iteration < 6; iteration++)
    {
        qDebug() << "step: " << iteration;
        m_solverDeal->assembleSystem();
        m_solverDeal->assembleDirichlet();
        m_solverDeal->solve();

        // update
        steps.append(iteration);
        relativeChangeOfSolutions.append(1.0);

        // , damping: %2
        Agros2D::log()->printMessage(QObject::tr("Solver (Picard)"), QObject::tr("Iteration: %1 (rel. change of sol.: %2 %)")
                                             .arg(iteration)
                                             .arg(QString::number(relativeChangeOfSolutions.last(), 'f', 5)));

        Agros2D::log()->updateNonlinearChartInfo(SolverAgros::Phase_Finished, steps, relativeChangeOfSolutions);
    }
    qDebug() << "solve nonlinear (" << time.elapsed() << "ms )";

    FieldSolutionID solutionID(fieldInfo, timeStep, adaptiveStep, SolutionMode_Normal);
    SolutionStore::SolutionRunTimeDetails runTime(Agros2D::problem()->actualTimeStepLength(), 0.0, 0);

    Agros2D::solutionStore()->addSolution(solutionID, MultiArray(m_solverDeal->doFHandler(), m_solverDeal->solution()), runTime);
}

void ProblemSolverDeal::solveAdaptive(FieldInfo *fieldInfo, int timeStep)
{
    qDebug() << "adaptive solution";
    for (int i = 0; i < fieldInfo->value(FieldInfo::AdaptivitySteps).toInt(); i++)
    {
        double error = 0.0;

        if (i > 0)
        {
            dealii::Vector<float> estimated_error_per_cell(m_solverDeal->triangulation()->n_active_cells());
            dealii::KellyErrorEstimator<2>::estimate(*m_solverDeal->doFHandler(),
                                                     dealii::QGauss<2-1>(2),
                                                     typename dealii::FunctionMap<2>::type(),
                                                     *m_solverDeal->solution(),
                                                     estimated_error_per_cell);

            dealii::GridRefinement::refine_and_coarsen_fixed_number(*m_solverDeal->triangulation(),
                                                                    estimated_error_per_cell,
                                                                    0.3,
                                                                    0.03);

            m_solverDeal->triangulation()->execute_coarsening_and_refinement();

            // print info
            /*
                    // TODO: store previous solution
                    dealii::Vector<float> difference_per_cell (m_solverDeal->triangulation()->n_active_cells());
                    dealii::VectorTools::integrate_difference(*m_solverDeal->doFHandler(),
                                                               *m_solverDeal->solution(),
                                                               dealii::Solution<2>(),
                                                               difference_per_cell,
                                                               dealii::QGauss<dim>(3),
                                                               dealii::VectorTools::H1_norm);
                    error = difference_per_cell.l2_norm();
                    */

            Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("Adaptivity step: %1 (error: %2, DOFs: %3)").
                                         arg(i).
                                         arg(error).
                                         arg(m_solverDeal->doFHandler()->n_dofs()));

            Agros2D::log()->updateAdaptivityChartInfo(fieldInfo, 0, i);
        }

        // TODO: can be
        if (Agros2D::problem()->isNonlinear())
            solveLinear(fieldInfo, timeStep, i);
        else
            solveNonlinear(fieldInfo, timeStep, i);

    }
}

template class ProblemSolver<double>;
