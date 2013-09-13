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
#include "solver_linear.h"
#include "solver_newton.h"
#include "solver_picard.h"

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
#include "plugin_interface.h"

#include "pythonlab/pythonengine.h"

using namespace Hermes::Hermes2D;

Hermes::Solvers::ExternalSolver<double>* getExternalSolver(CSCMatrix<double> *m, SimpleVector<double> *rhs)
{
    return new AgrosExternalSolverOctave(m, rhs);
}

AgrosExternalSolverOctave::AgrosExternalSolverOctave(CSCMatrix<double> *m, SimpleVector<double> *rhs)
    : ExternalSolver<double>(m, rhs)
{
}

void AgrosExternalSolverOctave::solve()
{
    solve(NULL);
}

void AgrosExternalSolverOctave::solve(double* initial_guess)
{
    QString file_command = QString("%1/solver_command").arg(cacheProblemDir());
    QString file_matrix = QString("%1/solver_matrix").arg(cacheProblemDir());
    QString file_rhs = QString("%1/solver_rhs").arg(cacheProblemDir());
    QString file_sln = QString("%1/solver_sln").arg(cacheProblemDir());

    this->set_matrix_dump_format(EXPORT_FORMAT_MATLAB_MATIO);
    this->set_rhs_E_matrix_dump_format(EXPORT_FORMAT_MATLAB_MATIO);
    this->set_matrix_filename(file_matrix.toStdString());
    this->set_rhs_filename(file_rhs.toStdString());
    this->set_matrix_number_format((char *) "%g");
    this->set_rhs_number_format((char *) "%g");

    // store state
    bool matrixOn = this->output_matrixOn;
    bool rhsOn = this->output_rhsOn;
    this->output_matrixOn = true;
    this->output_rhsOn = true;

    // write matrix and rhs to disk
    this->process_matrix_output(this->m);
    this->process_vector_output(this->rhs);

    // restore state
    this->output_matrixOn = matrixOn;
    this->output_rhsOn = rhsOn;

    // exec triangle
    m_process = new QProcess();
    m_process->setStandardOutputFile(tempProblemFileName() + ".solver.out");
    m_process->setStandardErrorFile(tempProblemFileName() + ".solver.err");
    // connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(solverError(QProcess::ProcessError)));
    // connect(m_process, SIGNAL(finished(int)), this, SLOT(solverSolutionCreated(int)));

    QString str = "#! /usr/bin/octave -qf\n";
    str += QString("load \"%1\";\n").arg(file_matrix);
    str += QString("load \"%2\";\n").arg(file_rhs);
    str += QString("sln = matrix \\ rhs;\n");
    str += QString("save -ascii \"%4\" sln;").arg(file_sln); // -mat

    writeStringContent(file_command, str);

    QFile script(file_command);
    script.setPermissions(QFile::ReadUser | QFile::ExeUser);

    m_process->start(file_command);

    // execute an event loop to process the request (nearly-synchronous)
    QEventLoop eventLoop;
    QObject::connect(m_process, SIGNAL(finished(int)), &eventLoop, SLOT(quit()));
    QObject::connect(m_process, SIGNAL(error(QProcess::ProcessError)), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    SimpleVector<double> slnVector;
    slnVector.import_from_file((char*) file_sln.toStdString().c_str());

    delete [] this->sln;
    this->sln = new double[slnVector.get_size()];
    memcpy(this->sln, slnVector.v, slnVector.get_size() * sizeof(double));

    QFile::remove(file_command);
    QFile::remove(file_matrix);
    QFile::remove(file_rhs);
    QFile::remove(file_sln);
}

//void AgrosExternalSolver::solverError(QProcess::ProcessError error)
//{
//    m_process->kill();
//}

//void AgrosExternalSolver::solverSolutionCreated(int exitCode)
//{
//    if (exitCode == 0)
//    {

//    }
//}

template <typename Scalar>
void HermesSolverContainer<Scalar>::setMatrixRhsOutputGen(Hermes::Mixins::MatrixRhsOutput<Scalar>* solver, QString solverName, int adaptivityStep)
{
    if(Agros2D::configComputer()->saveMatrixRHS)
    {
        solver->output_matrix(true);
        solver->output_rhs(true);
        QString name = QString("%1/%2_%3_%4").arg(cacheProblemDir()).arg(solverName).arg(Agros2D::problem()->actualTimeStep()).arg(adaptivityStep);
        solver->set_matrix_dump_format(Agros2D::configComputer()->dumpFormat);
        solver->set_rhs_E_matrix_dump_format(Agros2D::configComputer()->dumpFormat);
        solver->set_matrix_filename(QString("%1_Matrix").arg(name).toStdString());
        solver->set_rhs_filename(QString("%1_RHS").arg(name).toStdString());
        solver->set_matrix_number_format((char *) "%g");
        solver->set_rhs_number_format((char *) "%g");
    }
}

template <typename Scalar>
QSharedPointer<HermesSolverContainer<Scalar> > HermesSolverContainer<Scalar>::factory(Block* block)
{
    Hermes::HermesCommonApi.set_integral_param_value(Hermes::matrixSolverType, block->matrixSolver());
    Agros2D::log()->printDebug(QObject::tr("Solver"), QObject::tr("Linear solver: %1").arg(matrixSolverTypeString(block->matrixSolver())));

    if (block->matrixSolver() == Hermes::SOLVER_EXTERNAL)
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
        linearSolver->set_solver_type(block->iterParalutionLinearSolverMethod());
        linearSolver->set_precond(new Hermes::Preconditioners::ParalutionPrecond<Scalar>(block->iterParalutionLinearSolverPreconditioner()));
    }
    if (AMGParalutionLinearMatrixSolver<Scalar> *linearSolver = dynamic_cast<AMGParalutionLinearMatrixSolver<Scalar> *>(solver.data()->linearSolver()))
    {
        linearSolver->set_smoother(block->iterParalutionLinearSolverMethod(), block->iterParalutionLinearSolverPreconditioner());
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
void ProblemSolver<Scalar>::initSelectors(Hermes::vector<QSharedPointer<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> > > &selectors)
{
    // create types of projection and selectors
    for (int i = 0; i < m_block->numSolutions(); i++)
    {
        // set adaptivity selector
        QSharedPointer<RefinementSelectors::Selector<Scalar> > select;

        RefinementSelectors::CandList candList;

        if (m_block->adaptivityType() == AdaptivityType_None)
        {
            if (m_block->adaptivityUseAniso())
                candList = RefinementSelectors::H2D_HP_ANISO;
            else
                candList = RefinementSelectors::H2D_HP_ISO;

            select = QSharedPointer<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> >(new RefinementSelectors::H1ProjBasedSelector<Scalar>(candList));
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

                select = QSharedPointer<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> >(new RefinementSelectors::HOnlySelector<Scalar>());
                break;
            case AdaptivityType_P:
                if (m_block->adaptivityUseAniso())
                    candList = RefinementSelectors::H2D_P_ANISO;
                else
                    candList = RefinementSelectors::H2D_P_ISO;

                select = QSharedPointer<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> >(new RefinementSelectors::POnlySelector<Scalar>(H2DRS_DEFAULT_ORDER, 1, 1));
                break;
            case AdaptivityType_HP:
                if (m_block->adaptivityUseAniso())
                    candList = RefinementSelectors::H2D_HP_ANISO;
                else
                    candList = RefinementSelectors::H2D_HP_ISO;

                select = QSharedPointer<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> >(new RefinementSelectors::H1ProjBasedSelector<Scalar>(candList));
                break;
            }
        }

        // add refinement selector
        selectors.push_back(select);
    }
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

        if (createReference)
        {
            AdaptivityType adaptivityType = field->fieldInfo()->adaptivityType();
            if (m_block->adaptivityFinerReference() || (adaptivityType != AdaptivityType_P))
                refineMesh = field->fieldInfo()->value(FieldInfo::AdaptivitySpaceRefinement).toBool();

            if (m_block->adaptivityFinerReference() || (adaptivityType != AdaptivityType_H))
                orderIncrease = field->fieldInfo()->value(FieldInfo::AdaptivityOrderIncrease).toInt();
        }

        for (int comp = 0; comp < field->fieldInfo()->numberOfSolutions(); comp++)
        {
            MeshSharedPtr mesh;
            // deep copy of mesh for each field component separately
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
    if (LoopSolver<Scalar> *iterLinearSolver = dynamic_cast<LoopSolver<Scalar> *>(linearSolver))
        Agros2D::log()->printDebug(QObject::tr("Solver"),
                                   QObject::tr("Iterative solver statistics: %1 iterations")
                                   .arg(iterLinearSolver->get_num_iters()));

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
            runTime.setJacobianCalculations(solver->jacobianCalculations());
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
    double nextTimeStepLength = pow(TOL / error, 1.0 / (timeOrder + 1)) * Agros2D::problem()->actualTimeStepLength();

    nextTimeStepLength = min(nextTimeStepLength, maxTimeStepLength);
    nextTimeStepLength = min(nextTimeStepLength, Agros2D::problem()->actualTimeStepLength() * maxTimeStepRatio);
    nextTimeStepLength = max(nextTimeStepLength, Agros2D::problem()->actualTimeStepLength() / maxTimeStepRatio);

    Agros2D::log()->printDebug(m_solverID, QString("Time adaptivity, time %1 s, rel. error %2, tolerance %3, step size %4 -> %5 (%6 %)").
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
            case HERMES_L2_MARKERWISE_CONST_SPACE:
                oneSpace = new L2MarkerWiseConstSpace<Scalar>(fieldInfo->initialMesh());
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
        runTime.setJacobianCalculations(solver->jacobianCalculations());
    }

    MultiArray<Scalar> msa(actualSpaces(), solutions);
    Agros2D::solutionStore()->addSolution(solutionID, msa, runTime);
}

template <typename Scalar>
bool ProblemSolver<Scalar>::createAdaptedSpace(int timeStep, int adaptivityStep, bool forceAdaptation)
{
    MultiArray<Scalar> msa = Agros2D::solutionStore()->multiArray(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionMode_Normal));
    MultiArray<Scalar> msaRef = Agros2D::solutionStore()->multiArray(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionMode_Reference));

    Hermes::vector<QSharedPointer<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> > > selector;
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
    foreach (Field *field, m_block->fields())
    {
        // error calculation & adaptivity.
        QSharedPointer<ErrorCalculator<double> > errorCalculator = QSharedPointer<ErrorCalculator<double> >(
                    field->fieldInfo()->plugin()->errorCalculator(field->fieldInfo(),
                                                                  field->fieldInfo()->value(FieldInfo::AdaptivityErrorCalculator).toString(),
                                                                  Hermes::Hermes2D::RelativeErrorToGlobalNorm));

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
                Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *> vect;
                foreach (QSharedPointer<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> > sel, selector)
                    vect.push_back(sel.data());

                noRefinementPerformed = adaptivity.adapt(vect);
            }
            catch (Hermes::Exceptions::Exception e)
            {
                QString error = QString(e.what());
                Agros2D::log()->printDebug(m_solverID, QObject::tr("Adaptive process failed: %1").arg(error));
                throw;
            }

            adapt = adapt && (!noRefinementPerformed);
        }
    }

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
template class PicardSolverContainer<double>;
template class ProblemSolver<double>;
