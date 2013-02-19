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

void processSolverOutput(const char* aha)
{
    QString str = QString(aha).trimmed();
    Agros2D::log()->printMessage(QObject::tr("Solver"), str.replace("---- ", ""));
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
HermesSolverContainer<Scalar>* HermesSolverContainer<Scalar>::factory(Block* block)
{
    if (block->linearityType() == LinearityType_Linear)
    {
        return new LinearSolverContainer<Scalar>(block);
    }
    else if (block->linearityType() == LinearityType_Newton)
    {
        return new NewtonSolverContainer<Scalar>(block);
    }
    else if (block->linearityType() == LinearityType_Picard)
    {
        return new PicardSolverContainer<Scalar>(block);
    }
}


template <typename Scalar>
LinearSolverContainer<Scalar>::LinearSolverContainer(Block* block) : HermesSolverContainer<Scalar>(block)
{
    m_linearSolver = new LinearSolver<Scalar>();
    m_linearSolver->set_verbose_output(true);
}

template <typename Scalar>
LinearSolverContainer<Scalar>::~LinearSolverContainer()
{
    delete m_linearSolver;
    m_linearSolver = NULL;
}

template <typename Scalar>
void LinearSolverContainer<Scalar>::solve(Scalar* solutionVector)
{
    m_linearSolver->solve();
    this->m_slnVector = m_linearSolver->get_sln_vector();
}

template <typename Scalar>
NewtonSolverContainer<Scalar>::NewtonSolverContainer(Block* block) : HermesSolverContainer<Scalar>(block)
{
    m_newtonSolver = new NewtonSolver<Scalar>();
    m_newtonSolver->set_verbose_output(true);
    m_newtonSolver->set_verbose_callback(processSolverOutput);
    m_newtonSolver->set_newton_tol(block->nonlinearTolerance());
    m_newtonSolver->set_newton_max_iter(block->nonlinearSteps());
    m_newtonSolver->set_max_allowed_residual_norm(1e15);
    if (block->newtonAutomaticDamping())
    {
        m_newtonSolver->set_initial_auto_damping_coeff(block->newtonDampingCoeff());
        m_newtonSolver->set_necessary_successful_steps_to_increase(block->newtonDampingNumberToIncrease());
    }
    else
        m_newtonSolver->set_manual_damping_coeff(true, block->newtonDampingCoeff());
}

template <typename Scalar>
NewtonSolverContainer<Scalar>::~NewtonSolverContainer()
{
    delete m_newtonSolver;
    m_newtonSolver = NULL;
}

template <typename Scalar>
void NewtonSolverContainer<Scalar>::projectPreviousSolution(Scalar* solutionVector,
                                                            Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaces,
                                                            Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutions)
{
    if (solutions.empty())
    {
        int ndof = Space<Scalar>::get_num_dofs(castConst(spaces));
        memset(solutionVector, 0, ndof*sizeof(Scalar));
    }
    else
    {
        OGProjection<double> ogProjection;
        ogProjection.project_global(castConst(spaces),
                                    solutions,
                                    solutionVector, this->m_block->projNormTypeVector());
    }
}

template <typename Scalar>
void NewtonSolverContainer<Scalar>::solve(Scalar* solutionVector)
{
    m_newtonSolver->solve(solutionVector);
    this->m_slnVector = m_newtonSolver->get_sln_vector();
}

template <typename Scalar>
PicardSolverContainer<Scalar>::PicardSolverContainer(Block* block) : HermesSolverContainer<Scalar>(block)
{
    m_picardSolver = new PicardSolver<Scalar>();
    m_picardSolver->set_verbose_output(true);
    m_picardSolver->set_verbose_callback(processSolverOutput);
    m_picardSolver->set_picard_tol(block->nonlinearTolerance());
    m_picardSolver->set_picard_max_iter(block->nonlinearSteps());
    if (block->picardAndersonAcceleration())
    {
        m_picardSolver->use_Anderson_acceleration(true);
        m_picardSolver->set_num_last_vector_used(block->picardAndersonNumberOfLastVectors());
        m_picardSolver->set_anderson_beta(block->picardAndersonBeta());
    }
    else
        m_picardSolver->use_Anderson_acceleration(false);
    //m_picardSolver.data()->set_max_allowed_residual_norm(1e15);
}

template <typename Scalar>
PicardSolverContainer<Scalar>::~PicardSolverContainer()
{
    delete m_picardSolver;
    m_picardSolver = NULL;
}

template <typename Scalar>
void PicardSolverContainer<Scalar>::projectPreviousSolution(Scalar* solutionVector,
                                                            Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaces,
                                                            Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutions)
{
    if (solutions.empty())
    {
        int ndof = Space<Scalar>::get_num_dofs(castConst(spaces));
        memset(solutionVector, 0, ndof*sizeof(Scalar));
    }
    else
    {
        OGProjection<double> ogProjection;
        ogProjection.project_global(castConst(spaces),
                                    solutions,
                                    solutionVector, this->m_block->projNormTypeVector());
    }
}

template <typename Scalar>
void PicardSolverContainer<Scalar>::solve(Scalar* solutionVector)
{
    m_picardSolver->solve(solutionVector);
    this->m_slnVector = m_picardSolver->get_sln_vector();
}

template <typename Scalar>
Solver<Scalar>::~Solver()
{
    clearActualSpaces();

    if (m_hermesSolverContainer)
        delete m_hermesSolverContainer;
    m_hermesSolverContainer = NULL;
}

template <typename Scalar>
void Solver<Scalar>::init(Block* block)
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
void Solver<Scalar>::initSelectors(Hermes::vector<ProjNormType>& projNormType,
                                   Hermes::vector<RefinementSelectors::Selector<Scalar> *>& selectors)
{
    // set adaptivity selector
    RefinementSelectors::Selector<Scalar> *select = NULL;

    // create types of projection and selectors
    for (int i = 0; i < m_block->numSolutions(); i++)
    {
        // add norm
        projNormType.push_back(Agros2D::problem()->configView()->projNormType);

        RefinementSelectors::CandList candList;

        if (m_block->adaptivityType() == AdaptivityType_None)
        {
            if(Agros2D::problem()->configView()->useAniso)
                candList = RefinementSelectors::H2D_HP_ANISO;
            else
                candList = RefinementSelectors::H2D_HP_ISO;
        }
        else
        {
            switch (m_block->adaptivityType())
            {
            case AdaptivityType_H:
                if(Agros2D::problem()->configView()->useAniso)
                    candList = RefinementSelectors::H2D_H_ANISO;
                else
                    candList = RefinementSelectors::H2D_H_ISO;
                break;
            case AdaptivityType_P:
                if(Agros2D::problem()->configView()->useAniso)
                    candList = RefinementSelectors::H2D_P_ANISO;
                else
                    candList = RefinementSelectors::H2D_P_ISO;
                break;
            case AdaptivityType_HP:
                if(Agros2D::problem()->configView()->useAniso)
                    candList = RefinementSelectors::H2D_HP_ANISO;
                else
                    candList = RefinementSelectors::H2D_HP_ISO;
                break;
            }
        }
        select = new RefinementSelectors::H1ProjBasedSelector<Scalar>(candList, Agros2D::problem()->configView()->convExp, H2DRS_DEFAULT_ORDER);

        // add refinement selector
        selectors.push_back(select);
    }
}

template <typename Scalar>
void Solver<Scalar>::deleteSelectors(Hermes::vector<RefinementSelectors::Selector<Scalar> *>& selectors)
{
    foreach(RefinementSelectors::Selector<Scalar> *select, selectors)
        delete select;
    selectors.clear();
}

template <typename Scalar>
Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> Solver<Scalar>::deepMeshAndSpaceCopy(Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaces, bool createReference)
{
    Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> newSpaces;
    int totalComp = 0;
    foreach(Field* field, m_block->fields())
    {
        bool refineMesh = false;
        int orderIncrease = 0;
        if(createReference)
        {
            AdaptivityType adaptivityType = field->fieldInfo()->adaptivityType();
            if (Agros2D::problem()->configView()->finerReference || (adaptivityType != AdaptivityType_P))
                refineMesh = true;

            if (Agros2D::problem()->configView()->finerReference || (adaptivityType != AdaptivityType_H))
                orderIncrease = 1;
        }

        Mesh *mesh = NULL;
        // Deep copy of mesh for each field separately, than use for all field component the same one
        if (refineMesh)
        {
            Mesh::ReferenceMeshCreator meshCreator(spaces.at(totalComp)->get_mesh());
            mesh = meshCreator.create_ref_mesh();
        }
        else
        {
            mesh = new Mesh();
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
void Solver<Scalar>::setActualSpaces(Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaces)
{
    clearActualSpaces();
    m_actualSpaces = spaces;
}

template <typename Scalar>
void Solver<Scalar>::clearActualSpaces()
{
    // used meshes (should be shared between spaces)
    QList<Hermes::Hermes2D::Mesh *> meshes;
    foreach (Hermes::Hermes2D::Space<Scalar> *space, m_actualSpaces)
        if (!meshes.contains(space->get_mesh()))
            meshes.append(space->get_mesh());

    // clear meshes
    foreach (Hermes::Hermes2D::Mesh *mesh, meshes)
        delete mesh;

    // clear spaces
    foreach (Hermes::Hermes2D::Space<Scalar> *space, m_actualSpaces)
        delete space;

    m_actualSpaces.clear();
}

template <typename Scalar>
void Solver<Scalar>::addSolutionToStore(BlockSolutionID solutionID, Scalar* solutionVector)
{
    assert(solutionID.solutionMode == SolutionMode_Normal);
    Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> newSpaces = deepMeshAndSpaceCopy(m_actualSpaces, false);

    Hermes::vector<Hermes::Hermes2D::Mesh *> meshes = spacesMeshes(newSpaces);
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutions = createSolutions<Scalar>(meshes);
    Solution<Scalar>::vector_to_solutions(solutionVector, castConst(newSpaces), solutions);

    SolutionStore::SolutionRunTimeDetails runTime(Agros2D::problem()->actualTimeStepLength(),
                                                  0.0,
                                                  Hermes::Hermes2D::Space<double>::get_num_dofs(castConst(newSpaces)));

    Agros2D::solutionStore()->addSolution(solutionID, MultiArray<Scalar>(newSpaces, solutions), runTime);
}

template <typename Scalar>
Scalar *Solver<Scalar>::solveOneProblem(Scalar* initialSolutionVector,
                                        Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaces,
                                        int adaptivityStep,
                                        Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> previousSolution)
{
    Hermes::HermesCommonApi.set_integral_param_value(Hermes::matrixSolverType, Agros2D::problem()->config()->matrixSolver());

    try
    {
        m_hermesSolverContainer->projectPreviousSolution(initialSolutionVector, spaces, previousSolution);
        m_hermesSolverContainer->setTableSpaces()->set_spaces(castConst(spaces));
        m_hermesSolverContainer->setWeakFormulation(m_block->weakForm());
        m_hermesSolverContainer->setMatrixRhsOutput(m_solverCode, adaptivityStep);

        m_hermesSolverContainer->solve(initialSolutionVector);
    }
    catch (Hermes::Exceptions::Exception& e)
    {
        QString error = QString("%1").arg(e.what());
        // Agros2D::log()->printDebug(m_solverID, QObject::tr("Solver failed: %1").arg(error));
        throw AgrosSolverException(QObject::tr("Solver failed: %1").arg(error));
    }
    return m_hermesSolverContainer->slnVector();
}

template <typename Scalar>
void Solver<Scalar>::solveSimple(int timeStep, int adaptivityStep)
{
    // to be used as starting vector for the Newton solver
    MultiArray<Scalar> previousTSMultiSolutionArray;
    if((m_block->isTransient() && m_block->linearityType() != LinearityType_Linear) && (timeStep > 0))
        previousTSMultiSolutionArray = Agros2D::solutionStore()->multiSolutionPreviousCalculatedTS(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal));

    // cout << "Solving with " << Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(multiSolutionArray.spaces()))) << " dofs" << endl;

    // check for DOFs
    int ndof = Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(m_actualSpaces));
    if (ndof == 0)
    {
        Agros2D::log()->printDebug(m_solverID, QObject::tr("DOF is zero"));
        throw(AgrosSolverException("DOF is zero"));
    }

    // cout << QString("updating with time %1\n").arg(Agros2D::problem()->actualTime()).toStdString() << endl;

    // update timedep values
    foreach (Field* field, m_block->fields())
        Module::updateTimeFunctions(Agros2D::problem()->actualTime());
    updateExactSolutionFunctions();

    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(m_actualSpaces, Agros2D::problem()->actualTime());

    BDF2Table* bdf2Table = NULL;
    if (m_block->isTransient())
    {
        bdf2Table = new BDF2ATable();
        bdf2Table->setOrder(min(timeStep, Agros2D::problem()->config()->timeOrder()));
        bdf2Table->setPreviousSteps(Agros2D::problem()->timeStepLengths());
    }

    m_block->setWeakForm(new WeakFormAgros<double>(m_block));
    m_block->weakForm()->set_current_time(Agros2D::problem()->actualTime());
    m_block->weakForm()->registerForms(bdf2Table);

    // TODO: remove for linear solver
    Scalar *initialSolutionVector;
    try
    {
        initialSolutionVector = new Scalar[ndof];
        Scalar *solutionVector = solveOneProblem(initialSolutionVector, m_actualSpaces, adaptivityStep,
                                                 previousTSMultiSolutionArray.solutions());
        delete [] initialSolutionVector;

        if (bdf2Table)
            delete bdf2Table;

        // output
        BlockSolutionID solutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal);
        addSolutionToStore(solutionID, solutionVector);
    }
    catch (AgrosSolverException e)
    {
        delete [] initialSolutionVector;

        if (bdf2Table)
            delete bdf2Table;

        throw AgrosSolverException(QObject::tr("Solver failed: %1").arg(e.toString()));
    }
}

template <typename Scalar>
NextTimeStep Solver<Scalar>::estimateTimeStepLength(int timeStep, int adaptivityStep)
{
    double timeTotal = Agros2D::problem()->config()->timeTotal().number();

    // TODO: move to some config?
    const double relativeTimeStepLen = Agros2D::problem()->actualTimeStepLength() / timeTotal;
    const double maxTimeStepRatio = relativeTimeStepLen > 0.02 ? 2.0 : 3.0; // small steps may rise faster
    const double maxTimeStepLength = timeTotal / 10;
    const double maxToleranceMultiplyToAccept = 2.5; //3.0;

    TimeStepMethod method = Agros2D::problem()->config()->timeStepMethod();
    if(method == TimeStepMethod_Fixed)
        return NextTimeStep(Agros2D::problem()->config()->constantTimeStepLength());

    MultiArray<Scalar> referenceCalculation =
            Agros2D::solutionStore()->multiArray(Agros2D::solutionStore()->lastTimeAndAdaptiveSolution(m_block, SolutionMode_Normal));

    // todo: ensure this in gui
    assert(Agros2D::problem()->config()->timeOrder() >= 2);

    // todo: in the first step, I am acualy using order 1 and thus I am unable to decrease it!
    // this is not good, since the second step is not calculated (and the error of the first is not being checked)
    if (timeStep == 1)
    {
        m_averageErrorToLenghtRatio = 0.;
        return NextTimeStep(Agros2D::problem()->actualTimeStepLength());
    }

    BDF2Table* bdf2Table = new BDF2ATable();
    int previouslyUsedOrder = min(timeStep, Agros2D::problem()->config()->timeOrder());
    bdf2Table->setOrder(previouslyUsedOrder - 1);

    bdf2Table->setPreviousSteps(Agros2D::problem()->timeStepLengths());
    //cout << "using time order" << min(timeStep, Agros2D::problem()->config()->timeOrder()) << endl;

    m_block->setWeakForm(new WeakFormAgros<double>(m_block));
    m_block->weakForm()->set_current_time(Agros2D::problem()->actualTime());
    m_block->weakForm()->registerForms(bdf2Table);

    // solve, for nonlinear solver use solution obtained by BDFA method as an initial vector
    // TODO: remove for linear solver
    Scalar *initialSolutionVector = new Scalar[Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(m_actualSpaces))];
    Scalar *solutionVector = solveOneProblem(initialSolutionVector, m_actualSpaces, adaptivityStep,
                                             timeStep > 0 ? referenceCalculation.solutions() : Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *>());
    delete [] initialSolutionVector;

    Hermes::vector<Hermes::Hermes2D::Mesh *> meshes = spacesMeshes(m_actualSpaces);
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutions = createSolutions<Scalar>(meshes);
    Solution<Scalar>::vector_to_solutions(solutionVector, castConst(m_actualSpaces), solutions);

    double error = Global<Scalar>::calc_abs_errors(referenceCalculation.solutions(), solutions);

    // delete solutions
    deleteSolutions(solutions);

    //update
    double actualRatio = error / Agros2D::problem()->actualTimeStepLength();
    m_averageErrorToLenghtRatio = ((timeStep - 2) * m_averageErrorToLenghtRatio + actualRatio) / (timeStep - 1);
    //m_averageErrorToLenghtRatio = (m_averageErrorToLenghtRatio + actualRatio) / 2.;

    double TOL;
    if(Agros2D::problem()->config()->timeStepMethod() == TimeStepMethod_BDFTolerance)
    {
        TOL = Agros2D::problem()->config()->timeMethodTolerance().number();
    }
    else if(Agros2D::problem()->config()->timeStepMethod() == TimeStepMethod_BDFNumSteps)
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
    double nextTimeStepLength = pow(TOL / error, 1.0 / (Agros2D::problem()->config()->timeOrder() + 1)) * Agros2D::problem()->actualTimeStepLength();

    nextTimeStepLength = min(nextTimeStepLength, maxTimeStepLength);
    nextTimeStepLength = min(nextTimeStepLength, Agros2D::problem()->actualTimeStepLength() * maxTimeStepRatio);
    nextTimeStepLength = max(nextTimeStepLength, Agros2D::problem()->actualTimeStepLength() / maxTimeStepRatio);

    Agros2D::log()->printDebug(m_solverID, QString("time adaptivity, time %1, rel. error %2, tolerance %3, step size %4 -> %5 (%6 %)").
                               arg(Agros2D::problem()->actualTime()).
                               arg(error).
                               arg(TOL).
                               arg(Agros2D::problem()->actualTimeStepLength()).
                               arg(nextTimeStepLength).
                               arg(nextTimeStepLength / Agros2D::problem()->actualTimeStepLength()*100.));
    if(refuseThisStep)
        Agros2D::log()->printDebug(m_solverID, "time step refused");

    delete bdf2Table;
    return NextTimeStep(nextTimeStepLength, refuseThisStep);
}

template <typename Scalar>
void Solver<Scalar>::updateExactSolutionFunctions()
{
    foreach(ExactSolutionScalarAgros<double>* function, m_exactSolutionFunctions.keys())
    {
        SceneBoundary* boundary = m_exactSolutionFunctions[function];
        function->setMarkerSource(boundary);
    }
}

template <typename Scalar>
void Solver<Scalar>::createInitialSpace()
{
    // read mesh from file
    if (!Agros2D::problem()->isMeshed())
        throw AgrosSolverException(QObject::tr("Meshes are empty"));

    m_exactSolutionFunctions.clear();
    clearActualSpaces();

    foreach(Field* field, m_block->fields())
    {
        FieldInfo* fieldInfo = field->fieldInfo();

        // create copy of initial mesh
        // Hermes::Hermes2D::Mesh *initialMesh = new Hermes::Hermes2D::Mesh();
        // initialMesh->copy(fieldInfo->initialMesh());

        ProblemID problemId;

        problemId.sourceFieldId = fieldInfo->fieldId();
        problemId.analysisTypeSource = fieldInfo->analysisType();
        problemId.coordinateType = Agros2D::problem()->config()->coordinateType();
        problemId.linearityType = fieldInfo->linearityType();

        int index = 0;
        foreach(SceneEdge* edge, Agros2D::scene()->edges->items())
        {
            SceneBoundary *boundary = edge->marker(fieldInfo);

            if (boundary && (!boundary->isNone()))
            {
                Module::BoundaryType boundaryType = fieldInfo->boundaryType(boundary->type());

                foreach (FormInfo form, boundaryType.essential())
                {
                   // exact solution - Dirichlet BC
                    ExactSolutionScalarAgros<double> *function = fieldInfo->plugin()->exactSolution(problemId, &form, fieldInfo->initialMesh());
                    function->setMarkerSource(boundary);

                    // save function - boundary pairs, so thay can be easily updated in each time step;
                    m_exactSolutionFunctions[function] = boundary;

                    EssentialBoundaryCondition<Scalar> *custom_form = new DefaultEssentialBCNonConst<double>(QString::number(index).toStdString(), function);

                    m_block->bcs().at(form.i - 1 + m_block->offset(field))->add_boundary_condition(custom_form);
                    //  cout << "adding BC i: " << form->i - 1 + m_block->offset(field) << " ( form i " << form->i << ", " << m_block->offset(field) << "), expression: " << form->expression << endl;
                }
            }
            index++;
        }

        // create space
        for (int i = 0; i < fieldInfo->numberOfSolutions(); i++)
        {
            // create copy of initial mesh
            Hermes::Hermes2D::Mesh *oneInitialMesh = new Hermes::Hermes2D::Mesh();
            oneInitialMesh->copy(fieldInfo->initialMesh());

            Space<Scalar> *oneSpace = NULL;
            switch (fieldInfo->spaces()[i+1].type())
            {
            case HERMES_L2_SPACE:
                oneSpace = new L2Space<Scalar>(oneInitialMesh, fieldInfo->polynomialOrder() + fieldInfo->spaces()[i+1].orderAdjust());
                break;
            case HERMES_H1_SPACE:
                oneSpace = new H1Space<Scalar>(oneInitialMesh, m_block->bcs().at(i + m_block->offset(field)), fieldInfo->polynomialOrder() + fieldInfo->spaces()[i+1].orderAdjust());
                break;
            case HERMES_HCURL_SPACE:
                oneSpace = new HcurlSpace<Scalar>(oneInitialMesh, m_block->bcs().at(i + m_block->offset(field)), fieldInfo->polynomialOrder() + fieldInfo->spaces()[i+1].orderAdjust());
                break;
            case HERMES_HDIV_SPACE:
                oneSpace = new HdivSpace<Scalar>(oneInitialMesh, m_block->bcs().at(i + m_block->offset(field)), fieldInfo->polynomialOrder() + fieldInfo->spaces()[i+1].orderAdjust());
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
                        (fieldInfo->labelPolynomialOrder(label) != fieldInfo->polynomialOrder()))
                {
                    m_actualSpaces.at(i)->set_uniform_order(fieldInfo->labelPolynomialOrder(label),
                                                            QString::number(Agros2D::scene()->labels->items().indexOf(label)).toStdString());
                }
            }
        }

        // delete temp initial mesh
        // delete initialMesh;
    }

    assert(!m_hermesSolverContainer);
    m_hermesSolverContainer = HermesSolverContainer<Scalar>::factory(m_block);
}

template <typename Scalar>
void Solver<Scalar>::solveReferenceAndProject(int timeStep, int adaptivityStep)
{
    //    SolutionMode solutionMode = solutionExists ? SolutionMode_Normal : SolutionMode_NonExisting;
    //    MultiSolutionArray<Scalar> msa = Agros2D::solutionStore()->multiSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, solutionMode));
    //    MultiSolutionArray<Scalar> msaRef;

    if (Agros2D::problem()->isTransient())
    {
        if(adaptivityStep == 0)
        {
            if (timeStep % m_block->adaptivityRedoneEach() == 0)
            {
                assert(timeStep != 0);
                BlockSolutionID solID(m_block, 1, 0, SolutionMode_Normal);
                MultiArray<Scalar> msaPrevTS = Agros2D::solutionStore()->multiArray(solID);

                setActualSpaces(deepMeshAndSpaceCopy(msaPrevTS.spaces(), false));
            }
            else
            {
                if(timeStep > 1)
                {
                    int lastTimeStepNumAdaptations = Agros2D::solutionStore()->lastAdaptiveStep(m_block, SolutionMode_Normal, timeStep - 1);
                    BlockSolutionID solID(m_block, timeStep-1, max(lastTimeStepNumAdaptations - m_block->adaptivityBackSteps(), 0), SolutionMode_Normal);
                    MultiArray<Scalar> msaPrevTS = Agros2D::solutionStore()->multiArray(solID);

                    setActualSpaces(deepMeshAndSpaceCopy(msaPrevTS.spaces(), false));
                }
            }
        }

        // todo: to be used as starting vector for the Newton solver
        //        MultiSolutionArray<Scalar> previousTSMultiSolutionArray;
        //        if((m_block->isTransient() && m_block->linearityType() != LinearityType_Linear) && (timeStep > 0))
        //            previousTSMultiSolutionArray = Agros2D::solutionStore()->multiSolutionPreviousCalculatedTS(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal));
    }

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(m_actualSpaces) == 0)
    {
        Agros2D::log()->printDebug(m_solverID, QObject::tr("DOF is zero"));
        throw(AgrosSolverException("DOF is zero"));
    }

    // update timedep values
    foreach (Field* field, m_block->fields())
        Module::updateTimeFunctions(Agros2D::problem()->actualTime());

    // todo: delete? delam to pro referencni... (zkusit)
    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(m_actualSpaces, Agros2D::problem()->actualTime());

    BDF2Table* bdf2Table = NULL;
    if(m_block->isTransient())
    {
        bdf2Table = new BDF2ATable();
        bdf2Table->setOrder(min(timeStep, Agros2D::problem()->config()->timeOrder()));
        bdf2Table->setPreviousSteps(Agros2D::problem()->timeStepLengths());
    }

    // TODO: wf should be created only at the beginning
    // TODO: at least for the adaptivity, it should be ok to keep the form. For transiet step would be more complicated...
    m_block->setWeakForm(new WeakFormAgros<double>(m_block));
    m_block->weakForm()->set_current_time(Agros2D::problem()->actualTime());
    m_block->weakForm()->registerForms(bdf2Table);

    // create reference spaces
    Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spacesRef = deepMeshAndSpaceCopy(m_actualSpaces, true);
    assert(m_actualSpaces.size() == spacesRef.size());

    // todo: delete? je to vubec potreba?
    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(spacesRef, Agros2D::problem()->actualTime());

    // solve reference problem
    // TODO: posledni parametr: predchozi reseni pro projekci!!
    // TODO: remove for linear solver
    Scalar *initialSolutionVector = new Scalar[Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(m_actualSpaces))];
    Scalar *solutionVector = solveOneProblem(initialSolutionVector, spacesRef, adaptivityStep,
                                             Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *>());
    delete [] initialSolutionVector;

    // output reference solution
    Hermes::vector<Hermes::Hermes2D::Mesh *> meshesRef = spacesMeshes(spacesRef);
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutionsRef = createSolutions<Scalar>(meshesRef);
    Solution<Scalar>::vector_to_solutions(solutionVector, castConst(spacesRef), solutionsRef);

    BlockSolutionID referenceSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Reference);
    SolutionStore::SolutionRunTimeDetails runTimeRef(Agros2D::problem()->actualTimeStepLength(),
                                                     0.0,
                                                     Hermes::Hermes2D::Space<double>::get_num_dofs(castConst(spacesRef)));
    Agros2D::solutionStore()->addSolution(referenceSolutionID, MultiArray<Scalar>(spacesRef, solutionsRef), runTimeRef);

    // copy spaces and create empty solutions
    Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spacesCopy = deepMeshAndSpaceCopy(m_actualSpaces, false);
    Hermes::vector<Hermes::Hermes2D::Mesh *> meshes = spacesMeshes(m_actualSpaces);
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutions = createSolutions<Scalar>(meshes);

    // project the fine mesh solution onto the coarse mesh.
    Hermes::Hermes2D::OGProjection<Scalar> ogProjection;
    ogProjection.project_global(castConst(spacesCopy), solutionsRef, solutions);

    // save the solution
    BlockSolutionID solutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal);
    SolutionStore::SolutionRunTimeDetails runTime(Agros2D::problem()->actualTimeStepLength(),
                                                  0.0,
                                                  Hermes::Hermes2D::Space<double>::get_num_dofs(castConst(spacesCopy)));
    MultiArray<Scalar> msa(spacesCopy, solutions);
    Agros2D::solutionStore()->addSolution(solutionID, msa, runTime);

    if (bdf2Table)
        delete bdf2Table;
}

template <typename Scalar>
bool Solver<Scalar>::createAdaptedSpace(int timeStep, int adaptivityStep, bool forceAdaptation)
{
    MultiArray<Scalar> msa = Agros2D::solutionStore()->multiArray(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionMode_Normal));
    MultiArray<Scalar> msaRef = Agros2D::solutionStore()->multiArray(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionMode_Reference));

    Hermes::vector<Hermes::Hermes2D::ProjNormType> projNormType;
    Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *> selector;
    initSelectors(projNormType, selector);

    // calculate element errors and total error estimate.
    Adapt<Scalar> adaptivity(m_actualSpaces, projNormType);
    adaptivity.set_verbose_output(false);

    // calculate error estimate for each solution component and the total error estimate.
    double error = adaptivity.calc_err_est(msa.solutions(), msaRef.solutions()) * 100;
    // update error in solution store
    foreach (Field *field, m_block->fields())
    {
        FieldSolutionID solutionID(field->fieldInfo(), timeStep, adaptivityStep - 1, SolutionMode_Normal);

        // get run time
        SolutionStore::SolutionRunTimeDetails runTime = Agros2D::solutionStore()->multiSolutionRunTimeDetail(solutionID);
        // set error
        runTime.adaptivity_error = error;
        // replace runtime
        Agros2D::solutionStore()->multiSolutionRunTimeDetailReplace(solutionID, runTime);
    }

    // todo: otazku zda uz neni moc dofu jsem mel vyresit nekde drive
    bool adapt = error >= m_block->adaptivityTolerance() && Hermes::Hermes2D::Space<Scalar>::get_num_dofs(m_actualSpaces) < Agros2D::problem()->configView()->maxDofs;

    // allways adapt when forcing adaptation, to be used in solveAdaptiveStep
    adapt = adapt || forceAdaptation;

    if(adapt)
    {
        bool noRefinementPerformed;
        try
        {
            noRefinementPerformed = adaptivity.adapt(selector, Agros2D::problem()->configView()->threshold, Agros2D::problem()->configView()->strategy, Agros2D::problem()->configView()->meshRegularity);
        }
        catch (Hermes::Exceptions::Exception e)
        {
            QString error = QString(e.what());
            Agros2D::log()->printDebug(m_solverID, QObject::tr("Adaptive process failed: %1").arg(error));
            throw;
        }

        Agros2D::log()->printMessage(m_solverID, QObject::tr("adaptivity step (error = %1, DOFs = %2/%3)").
                                     arg(error).
                                     arg(Space<Scalar>::get_num_dofs(msa.spacesConst())).
                                     arg(Space<Scalar>::get_num_dofs(msaRef.spacesConst())));
        adapt = adapt && (!noRefinementPerformed);
    }

    deleteSelectors(selector);

    return adapt;
}

template <typename Scalar>
void Solver<Scalar>::solveInitialTimeStep()
{
    Agros2D::log()->printDebug(m_solverID, QObject::tr("initial time step"));

    Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaces = deepMeshAndSpaceCopy(m_actualSpaces, false);
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutions;

    int totalComp = 0;
    foreach(Field* field, m_block->fields())
    {
        for (int comp = 0; comp < field->fieldInfo()->numberOfSolutions(); comp++)
        {
            // constant initial solution
            Mesh *mesh = spaces.at(totalComp)->get_mesh();
            ConstantSolution<double> *initial = new ConstantSolution<double>(mesh,
                                                                             field->fieldInfo()->initialCondition().number());
            solutions.push_back(initial);
            totalComp++;
        }
    }

    BlockSolutionID solutionID(m_block, 0, 0, SolutionMode_Normal);
    SolutionStore::SolutionRunTimeDetails runTime(Agros2D::problem()->actualTimeStepLength(),
                                                  0.0,
                                                  Hermes::Hermes2D::Space<double>::get_num_dofs(castConst(m_actualSpaces)));

    Agros2D::solutionStore()->addSolution(solutionID,
                                          MultiArray<Scalar>(spaces, solutions),
                                          runTime);
}

template <typename Scalar>
void Solver<Scalar>::resumeAdaptivityProcess(int adaptivityStep)
{
    BlockSolutionID solID(m_block, 0, adaptivityStep, SolutionMode_Normal);
    MultiArray<Scalar> msa = Agros2D::solutionStore()->multiArray(solID);

    setActualSpaces(deepMeshAndSpaceCopy(msa.spaces(), false));

    assert(!m_hermesSolverContainer);
    m_hermesSolverContainer = HermesSolverContainer<Scalar>::factory(m_block);

}


//template class VectorStore<double>;
template class LinearSolverContainer<double>;
template class NewtonSolverContainer<double>;
template class PicardSolverContainer<double>;
template class Solver<double>;
