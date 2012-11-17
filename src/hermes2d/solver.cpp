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

#include "field.h"
#include "block.h"
#include "problem.h"
#include "solver.h"
#include "module.h"
#include "scene.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "module_agros.h"
#include "solutionstore.h"
#include "plugin_interface.h"
#include "logview.h"
#include "util.h"
#include "bdf2.h"

using namespace Hermes::Hermes2D;


int DEBUG_COUNTER = 0;

void processSolverOutput(const char* aha)
{
    QString str = QString(aha).trimmed();
    Util::log()->printMessage(QObject::tr("Solver"), str.replace("---- ", ""));
}

template <typename Scalar>
void HermesSolverContainer<Scalar>::setMatrixRhsOutputGen(Hermes::Hermes2D::Mixins::MatrixRhsOutput<Scalar>* solver, QString solverName, int adaptivityStep)
{
    if(Util::config()->saveMatrixRHS)
    {
        solver->output_matrix();
        solver->output_rhs();
        QString name = QString("%1/%2-%3-%4").arg(tempProblemDir()).arg(solverName).arg(Util::problem()->actualTimeStep()).arg(adaptivityStep);
        solver->set_matrix_filename(QString("%1-Matrix").arg(name).toStdString());
        solver->set_rhs_filename(QString("%1-RHS").arg(name).toStdString());
    }
}

template <typename Scalar>
HermesSolverContainer<Scalar>* HermesSolverContainer<Scalar>::factory(Block* block, MultiSpace<Scalar> spaces)
{
    if (block->linearityType() == LinearityType_Linear)
    {
        return new LinearSolverContainer<Scalar>(block, spaces);
    }
    else if (block->linearityType() == LinearityType_Newton)
    {
        return new NewtonSolverContainer<Scalar>(block, spaces);
    }
    else if (block->linearityType() == LinearityType_Picard)
    {
        return new PicardSolverContainer<Scalar>(block, spaces);
    }
}


template <typename Scalar>
LinearSolverContainer<Scalar>::LinearSolverContainer(Block* block, MultiSpace<Scalar> spaces) : HermesSolverContainer<Scalar>(block)
{
    m_linearSolver = QSharedPointer<LinearSolver<Scalar> >(new LinearSolver<Scalar>(block->weakForm().data(), spaces.nakedConst()));
}


template <typename Scalar>
void LinearSolverContainer<Scalar>::solve(Scalar* solutionVector)
{
    int ndof = Space<Scalar>::get_num_dofs(m_linearSolver.data()->get_spaces());
    m_linearSolver.data()->solve();
    memcpy(solutionVector, m_linearSolver.data()->get_sln_vector(), ndof * sizeof(Scalar));
}

template <typename Scalar>
NewtonSolverContainer<Scalar>::NewtonSolverContainer(Block* block, MultiSpace<Scalar> spaces) : HermesSolverContainer<Scalar>(block)
{
    m_newtonSolver = QSharedPointer<NewtonSolver<Scalar> >(new NewtonSolver<Scalar>(block->weakForm().data(), spaces.nakedConst()));
    m_newtonSolver.data()->set_verbose_output(true);
    m_newtonSolver.data()->set_verbose_callback(processSolverOutput);
    m_newtonSolver.data()->set_newton_tol(block->nonlinearTolerance());
    m_newtonSolver.data()->set_newton_max_iter(block->nonlinearSteps());
    m_newtonSolver.data()->set_max_allowed_residual_norm(1e15);
    if (block->newtonAutomaticDamping())
    {
        m_newtonSolver.data()->set_initial_auto_damping_coeff(block->newtonDampingCoeff());
        m_newtonSolver.data()->set_necessary_successful_steps_to_increase(block->newtonDampingNumberToIncrease());
    }
    else
        m_newtonSolver.data()->set_manual_damping_coeff(true, block->newtonDampingCoeff());
}

template <typename Scalar>
void NewtonSolverContainer<Scalar>::projectPreviousSolution(Scalar* solutionVector, MultiSpace<Scalar> spaces,
                                                            MultiSolution<Scalar> solutions)
{
    if(solutions.empty())
    {
        int ndof = Space<Scalar>::get_num_dofs(spaces.nakedConst());
        memset(solutionVector, 0, ndof*sizeof(Scalar));
    }
    else
    {
        OGProjection<double> ogProjection;
        ogProjection.project_global(spaces.nakedConst(),
                                    solutions.naked(),
                                    solutionVector, this->m_block->projNormTypeVector());
    }
}

template <typename Scalar>
void NewtonSolverContainer<Scalar>::solve(Scalar* solutionVector)
{
    int ndof = Space<Scalar>::get_num_dofs(m_newtonSolver.data()->get_spaces());
    m_newtonSolver.data()->solve(solutionVector);
    memcpy(solutionVector, m_newtonSolver.data()->get_sln_vector(), ndof * sizeof(Scalar));
}

template <typename Scalar>
PicardSolverContainer<Scalar>::PicardSolverContainer(Block* block, MultiSpace<Scalar> spaces) : HermesSolverContainer<Scalar>(block)
{
    Hermes::vector<Solution<Scalar>* > slns;
    for (int i = 0; i < spaces.size(); i++)
    {
        QSharedPointer<Hermes::Hermes2D::Space<Scalar> > space = spaces.at(i).space;
        Hermes::Hermes2D::Space<Scalar> *spc = space.data();
        slns.push_back(new Hermes::Hermes2D::ConstantSolution<double>(spc->get_mesh(), 200));
    }

    m_picardSolver = QSharedPointer<PicardSolver<Scalar> >(new PicardSolver<Scalar>(block->weakForm().data(), spaces.nakedConst()));
    m_picardSolver.data()->set_verbose_output(true);
    m_picardSolver.data()->set_verbose_callback(processSolverOutput);
    m_picardSolver.data()->set_picard_tol(block->nonlinearTolerance());
    m_picardSolver.data()->set_picard_max_iter(block->nonlinearSteps());
    if (block->picardAndersonAcceleration())
    {
        m_picardSolver.data()->use_Anderson_acceleration(true);
        m_picardSolver.data()->set_num_last_vector_used(block->picardAndersonNumberOfLastVectors());
        m_picardSolver.data()->set_anderson_beta(block->picardAndersonBeta());
    }
    else
        m_picardSolver.data()->use_Anderson_acceleration(false);
    //m_picardSolver.data()->set_max_allowed_residual_norm(1e15);
}

template <typename Scalar>
void PicardSolverContainer<Scalar>::projectPreviousSolution(Scalar* solutionVector, MultiSpace<Scalar> spaces, MultiSolution<Scalar> solutions)
{
    if(solutions.empty())
    {
        int ndof = Space<Scalar>::get_num_dofs(spaces.nakedConst());
        memset(solutionVector, 0, ndof*sizeof(Scalar));
    }
    else
    {
        OGProjection<double> ogProjection;
        ogProjection.project_global(spaces.nakedConst(),
                                    solutions.naked(),
                                    solutionVector, this->m_block->projNormTypeVector());
    }
}

template <typename Scalar>
void PicardSolverContainer<Scalar>::solve(Scalar* solutionVector)
{
    int ndof = Space<Scalar>::get_num_dofs(m_picardSolver.data()->get_spaces());
    m_picardSolver.data()->solve();
    memcpy(solutionVector, m_picardSolver.data()->get_sln_vector(), ndof * sizeof(Scalar));
}

template <typename Scalar>
Solver<Scalar>::~Solver()
{
    if(m_hermesSolverContainer)
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
        m_solverName += iter.next()->fieldInfo()->fieldId();
        if (iter.hasNext())
            m_solverName += ", ";
    }
    m_solverID = QObject::tr("Solver (%1)").arg(m_solverName);

    // TODO: it should be here, but for now moved to solve one problem
    //    assert(! m_hermesSolverContainer);
    //    if(m_block->linearityType() == LinearityType_Linear)
    //        m_hermesSolverContainer = new LinearSolverContainer<Scalar>(m_block->weakForm(), NULL, m_solverName);
}

template <typename Scalar>
QMap<FieldInfo*, QSharedPointer<Mesh> > Solver<Scalar>::readMesh()
{
    // load the mesh file
    QMap<FieldInfo*, QSharedPointer<Mesh> > meshes = readMeshesFromFile(tempProblemFileName() + ".xml");

    // check that all boundary edges have a marker assigned
    QSet<int> boundaries;
    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        QSharedPointer<Mesh> mesh = meshes[fieldInfo];
        for (int i = 0; i < mesh->get_max_node_id(); i++)
        {
            Node *node = mesh.data()->get_node(i);

            if ((node->used == 1 && node->ref < 2 && node->type == 1))
            {
                int marker = atoi(mesh.data()->get_boundary_markers_conversion().get_user_marker(node->marker).marker.c_str());

                assert(marker >= 0 || marker == -999);

                if (marker >= 0 && Util::scene()->edges->at(marker)->marker(fieldInfo) == SceneBoundaryContainer::getNone(fieldInfo))
                    boundaries.insert(marker);
            }
        }

        if (boundaries.count() > 0)
        {
            QString markers;
            foreach (int marker, boundaries)
                markers += QString::number(marker) + ", ";
            markers = markers.left(markers.length() - 2);

            Util::log()->printError(m_solverID, QObject::tr("boundary edges '%1' does not have a boundary marker").arg(markers));

            meshes.clear();
            return meshes;
        }
        boundaries.clear();
    }

    Util::problem()->setMeshesInitial(meshes);

    return meshes;
}

template <typename Scalar>
void Solver<Scalar>::initSelectors(Hermes::vector<ProjNormType>& projNormType,
                                   Hermes::vector<RefinementSelectors::Selector<Scalar> *>& selector)
{
    // set adaptivity selector
    RefinementSelectors::Selector<Scalar> *select = NULL;

    // create types of projection and selectors
    for (int i = 0; i < m_block->numSolutions(); i++)
    {
        // add norm
        projNormType.push_back(Util::config()->projNormType);

        RefinementSelectors::CandList candList;

        if (m_block->adaptivityType() == AdaptivityType_None)
        {
            if(Util::config()->useAniso)
                candList = RefinementSelectors::H2D_HP_ANISO;
            else
                candList = RefinementSelectors::H2D_HP_ISO;
        }
        else
        {
            switch (m_block->adaptivityType())
            {
            case AdaptivityType_H:
                if(Util::config()->useAniso)
                    candList = RefinementSelectors::H2D_H_ANISO;
                else
                    candList = RefinementSelectors::H2D_H_ISO;
                break;
            case AdaptivityType_P:
                if(Util::config()->useAniso)
                    candList = RefinementSelectors::H2D_P_ANISO;
                else
                    candList = RefinementSelectors::H2D_P_ISO;
                break;
            case AdaptivityType_HP:
                if(Util::config()->useAniso)
                    candList = RefinementSelectors::H2D_HP_ANISO;
                else
                    candList = RefinementSelectors::H2D_HP_ISO;
                break;
            }
        }
        select = new RefinementSelectors::H1ProjBasedSelector<Scalar>(candList, Util::config()->convExp, H2DRS_DEFAULT_ORDER);

        // add refinement selector
        selector.push_back(select);
    }
}

template <typename Scalar>
void Solver<Scalar>::deleteSelectors(Hermes::vector<RefinementSelectors::Selector<Scalar> *>& selector)
{
    foreach(RefinementSelectors::Selector<Scalar> *select, selector)
    {
        // todo: should it be deleted?
        //delete select;
    }
    selector.clear();
}

//template <typename Scalar>
//void Solver<Scalar>::createInitialSolution(Mesh* mesh, MultiSolutionArray<Scalar>& msa)
//{
//    MultiSolutionArray<Scalar> msaInitial = msa.copySpaces();
//    int totalComp = 0;
//    foreach(Field* field, m_block->m_fields)
//    {
//        for (int comp = 0; comp < field->fieldInfo()->module()->number_of_solution(); comp++)
//        {
//            // nonlinear - initial solution
//            // solution.at(i)->set_const(mesh, 0.0);

//            // constant initial solution
//            InitialCondition<double> *initial = new InitialCondition<double>(mesh, field->fieldInfo()->initialCondition.number());
//            msaInitial.setSolution(QSharedPointer<Solution<Scalar> >(initial), totalComp);
//            totalComp++;
//        }
//    }
//    BlockSolutionID solutionID(m_block, 0, 0, SolutionType_Normal);
//    Util::solutionStore()->saveSolution(solutionID, msaInitial);
//}

//template <typename Scalar>
//Hermes::vector<QSharedPointer<Space<Scalar> > > Solver<Scalar>::createCoarseSpace()
//{
//    Hermes::vector<QSharedPointer<Space<Scalar> > > space;

//    foreach(Field* field, m_block->fields())
//    {
//        MultiSolutionArray<Scalar> multiSolution = Util::solutionStore()->multiSolution(Util::solutionStore()->lastTimeAndAdaptiveSolution(field->fieldInfo(), SolutionMode_Normal));
//        for(int comp = 0; comp < field->fieldInfo()->module()->numberOfSolutions(); comp++)
//        {
//            Space<Scalar>* oldSpace = multiSolution.component(comp).space.data();

//            Mesh *mesh = new Mesh(); //TODO probably leak ... where is the mesh released
//            mesh->copy(oldSpace->get_mesh());

//            // TODO: double -> Scalar
//            Space<double>::ReferenceSpaceCreator spaceCreator(oldSpace, mesh, 0);
//            space.push_back(QSharedPointer<Space<Scalar> >(spaceCreator.create_ref_space()));
//        }
//    }

//    return space;
//}

template <typename Scalar>
MultiSpace<Scalar> Solver<Scalar>::deepMeshAndSpaceCopy(MultiSpace<Scalar> spaces)
{
    MultiSpace<Scalar> newSpaces;
    int totalComp = 0;
    foreach(Field* field, m_block->fields())
    {
        // Deep copy of mesh for each field separately, than use for all field component the same one
        QSharedPointer<Mesh> mesh(new Mesh());
        mesh.data()->copy(spaces.at(totalComp).data()->get_mesh());

        for(int comp = 0; comp < field->fieldInfo()->module()->numberOfSolutions(); comp++)
        {
            // TODO: double -> Scalar
            Space<double>::ReferenceSpaceCreator spaceCreator(spaces.at(totalComp).data(), mesh.data(), 0);
            newSpaces.add(QSharedPointer<Space<Scalar> >(spaceCreator.create_ref_space()), mesh);

            totalComp++;
        }
    }
    return newSpaces;
}

template <typename Scalar>
void Solver<Scalar>::saveSolution(BlockSolutionID solutionID, Scalar* solutionVector)
{
    MultiSpace<Scalar> spaces = ((solutionID.solutionMode == SolutionMode_Normal) ? m_actualSpaces : m_actualSpacesRef);
    MultiSpace<Scalar> newSpaces = deepMeshAndSpaceCopy(spaces);
    MultiSolution<Scalar> solutions;
    solutions.createSolutions(newSpaces.meshes());
    Solution<Scalar>::vector_to_solutions(solutionVector, newSpaces.nakedConst(), solutions.naked());

    Util::solutionStore()->addSolution(solutionID, MultiSolutionArray<Scalar>(newSpaces, solutions, Util::problem()->actualTime()));
}

template <typename Scalar>
void Solver<Scalar>::solveOneProblem(Scalar* solutionVector, SolutionMode solutionMode, int adaptivityStep, MultiSolution<Scalar> previousSolution)
{
    MultiSpace<Scalar> spaces = (solutionMode == SolutionMode_Normal) ? m_actualSpaces : m_actualSpacesRef;
    Hermes::HermesCommonApi.set_param_value(Hermes::matrixSolverType, Util::problem()->config()->matrixSolver());

    m_hermesSolverContainer = HermesSolverContainer<Scalar>::factory(m_block, spaces);

    try
    {
        m_hermesSolverContainer->projectPreviousSolution(solutionVector, spaces, previousSolution);
        m_hermesSolverContainer->settableSpaces()->set_spaces(spaces.nakedConst());
        m_hermesSolverContainer->setMatrixRhsOutput(m_solverName, adaptivityStep);
        m_hermesSolverContainer->solve(solutionVector);

        // TODO: temporarily disabled
        /*
        Util::log()->printDebug(m_solverID, QObject::tr("Newton's solver - assemble/solve/total: %1/%2/%3 s").
                                arg(milisecondsToTime(newton.get_assemble_time() * 1000.0).toString("mm:ss.zzz")).
                                arg(milisecondsToTime(newton.get_solve_time() * 1000.0).toString("mm:ss.zzz")).
                                arg(milisecondsToTime((newton.get_assemble_time() + newton.get_solve_time()) * 1000.0).toString("mm:ss.zzz")));
        msa.setAssemblyTime(newton.get_assemble_time() * 1000.0);
        msa.setSolveTime(newton.get_solve_time() * 1000.0);
        */
    }
    catch (Hermes::Exceptions::Exception e)
    {
        QString error = QString("%1").arg(e.what());
        Util::log()->printDebug(m_solverID, QObject::tr("Solver failed: %1").arg(error));
        throw;
    }
}

template <typename Scalar>
void Solver<Scalar>::solveSimple(int timeStep, int adaptivityStep)
{
    // to be used as starting vector for the Newton solver
    MultiSolutionArray<Scalar> previousTSMultiSolutionArray;
    if((m_block->isTransient() && m_block->linearityType() != LinearityType_Linear) && (timeStep > 0))
        previousTSMultiSolutionArray = Util::solutionStore()->multiSolutionPreviousCalculatedTS(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal));

    //cout << "Solving with " << Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(multiSolutionArray.spaces()))) << " dofs" << endl;

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(m_actualSpaces.nakedConst()) == 0)
    {
        Util::log()->printDebug(m_solverID, QObject::tr("DOF is zero"));
        throw(AgrosSolverException("DOF is zero"));
    }

    cout << QString("updating with time %1\n").arg(Util::problem()->actualTime()).toStdString() << endl;

    // update timedep values
    foreach (Field* field, m_block->fields())
        field->fieldInfo()->module()->updateTimeFunctions(Util::problem()->actualTime());

    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(m_actualSpaces.naked(), Util::problem()->actualTime());

    BDF2Table* bdf2Table = NULL;
    if(m_block->isTransient())
    {
        bdf2Table = new BDF2ATable();
        bdf2Table->setOrder(min(timeStep, Util::problem()->config()->timeOrder()));
        bdf2Table->setPreviousSteps(Util::problem()->timeStepLengths());
    }

    // TODO: wf should be created only at the beginning
    m_block->setWeakForm(QSharedPointer<WeakFormAgros<double> >(new WeakFormAgros<double>(m_block)));
    m_block->weakForm().data()->set_current_time(Util::problem()->actualTime());
    m_block->weakForm().data()->registerForms(bdf2Table);

    Scalar solutionVector[Space<Scalar>::get_num_dofs(m_actualSpaces.nakedConst())];

    solveOneProblem(solutionVector, SolutionMode_Normal, adaptivityStep, previousTSMultiSolutionArray.solutions());

    // output
    BlockSolutionID solutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal);
    saveSolution(solutionID, solutionVector);

    // TODO: remove
    m_block->weakForm().clear();

    delete bdf2Table;
}

template <typename Scalar>
NextTimeStep Solver<Scalar>::estimateTimeStepLenght(int timeStep, int adaptivityStep)
{
    // todo: move to some config?
    const double MAX_TIME_STEPS_RATIO = 2.0; // todo: 3.0;
    const double MAX_TIME_STEP_LENGTH = Util::problem()->config()->timeTotal().value() / 10;
    const double MAX_TOLERANCE_MULTIPLY_TO_ACCEPT = 2.5;  // todo: 3.0

    TimeStepMethod method = Util::problem()->config()->timeStepMethod();
    if(method == TimeStepMethod_Fixed)
        return NextTimeStep(Util::problem()->config()->constantTimeStepLength());

    // At the present moment we use only estimation using BDF2A with different orders
    assert(method == TimeStepMethod_BDF2AOrder);

    MultiSolutionArray<Scalar> referenceCalculation =
            Util::solutionStore()->multiSolution(Util::solutionStore()->lastTimeAndAdaptiveSolution(m_block, SolutionMode_Normal));

    // todo: ensure this in gui
    assert(Util::problem()->config()->timeOrder() >= 2);

    // todo: in the first step, I am acualy using order 1 and thus I am unable to decrease it!
    // this is not good, since the second step is not calculated (and the error of the first is not being checked)
    if(timeStep == 1)
        return NextTimeStep(Util::problem()->actualTimeStepLength());

    BDF2Table* bdf2Table = new BDF2ATable();
    int previouslyUsedOrder = min(timeStep, Util::problem()->config()->timeOrder());
    bdf2Table->setOrder(previouslyUsedOrder - 1);

    bdf2Table->setPreviousSteps(Util::problem()->timeStepLengths());
    //cout << "using time order" << min(timeStep, Util::problem()->config()->timeOrder()) << endl;

    m_block->setWeakForm(QSharedPointer<WeakFormAgros<double> >(new WeakFormAgros<double>(m_block)));
    m_block->weakForm().data()->set_current_time(Util::problem()->actualTime());
    m_block->weakForm().data()->registerForms(bdf2Table);


    //    MultiSolutionArray<Scalar> multiSolutionArray2 = multiSolutionArray.copySpaces();
    //    multiSolutionArray2.createNewSolutions();

    Scalar solutionVector[Space<Scalar>::get_num_dofs(m_actualSpaces.nakedConst())];

    // solve, for nonlinear solver use solution obtained by BDFA method as an initial vector
    solveOneProblem(solutionVector, SolutionMode_Normal, adaptivityStep, timeStep > 0 ? referenceCalculation.solutions() : MultiSolution<Scalar>());

    MultiSolution<Scalar> solutions;
    solutions.createSolutions(m_actualSpaces.meshes());
    Solution<Scalar>::vector_to_solutions(solutionVector, m_actualSpaces.nakedConst(), solutions.naked());

    double error = Global<Scalar>::calc_abs_errors(referenceCalculation.solutionsNaked(), solutions.naked());

    bool refuseThisStep = error > MAX_TOLERANCE_MULTIPLY_TO_ACCEPT * Util::problem()->config()->timeMethodTolerance().number();

    // this guess is based on assymptotic considerations (diploma thesis of Pavel Kus)
    double nextTimeStepLength = pow(Util::problem()->config()->timeMethodTolerance().number() / error,
                                    1.0 / (Util::problem()->config()->timeOrder() + 1)) * Util::problem()->actualTimeStepLength();

    nextTimeStepLength = min(nextTimeStepLength, MAX_TIME_STEP_LENGTH);
    nextTimeStepLength = min(nextTimeStepLength, Util::problem()->actualTimeStepLength() * MAX_TIME_STEPS_RATIO);
    nextTimeStepLength = max(nextTimeStepLength, Util::problem()->actualTimeStepLength() / MAX_TIME_STEPS_RATIO);

    Util::log()->printDebug(m_solverID, QString("time adaptivity, time %1, rel. error %2, step size %3 -> %4 (%5 %)").
                            arg(Util::problem()->actualTime()).
                            arg(error).
                            arg(Util::problem()->actualTimeStepLength()).
                            arg(nextTimeStepLength).
                            arg(nextTimeStepLength / Util::problem()->actualTimeStepLength()*100.));
    if(refuseThisStep)
        Util::log()->printDebug(m_solverID, "time step refused");

    m_block->weakForm().clear();

    delete bdf2Table;
    return NextTimeStep(nextTimeStepLength, refuseThisStep);
}

template <typename Scalar>
void Solver<Scalar>::createInitialSpace()
{
    // read mesh from file
    QMap<FieldInfo*, QSharedPointer<Mesh> > meshes = readMesh();
    if (meshes.isEmpty())
        throw(AgrosSolverException("Meshes are empty"));

    // essential boundary conditions
    Hermes::vector<EssentialBCs<double> *> bcs;
    for (int i = 0; i < m_block->numSolutions(); i++)
        bcs.push_back(new EssentialBCs<double>());

    foreach(Field* field, m_block->fields())
    {
        FieldInfo* fieldInfo = field->fieldInfo();

        ProblemID problemId;

        problemId.sourceFieldId = fieldInfo->fieldId();
        problemId.analysisTypeSource = fieldInfo->module()->analysisType();
        problemId.coordinateType = fieldInfo->module()->coordinateType();
        problemId.linearityType = fieldInfo->linearityType();

        int index = 0;
        foreach(SceneEdge* edge, Util::scene()->edges->items())
        {
            SceneBoundary *boundary = edge->marker(fieldInfo);

            if (boundary && (!boundary->isNone()))
            {
                Module::BoundaryType *boundary_type = fieldInfo->module()->boundaryType(boundary->type());

                foreach (FormInfo *form, boundary_type->essential())
                {
                    // plugion interface
                    PluginInterface *plugin = Util::plugins()[fieldInfo->fieldId()];
                    assert(plugin);

                    // exact solution - Dirichlet BC
                    ExactSolutionScalarAgros<double> *function = plugin->exactSolution(problemId, form, meshes[fieldInfo].data());
                    function->setMarkerSource(boundary);

                    EssentialBoundaryCondition<Scalar> *custom_form = new DefaultEssentialBCNonConst<double>(QString::number(index).toStdString(), function);

                    bcs[form->i - 1 + m_block->offset(field)]->add_boundary_condition(custom_form);
                    //  cout << "adding BC i: " << form->i - 1 + m_block->offset(field) << " ( form i " << form->i << ", " << m_block->offset(field) << "), expression: " << form->expression << endl;
                }
            }
            index++;
        }

        // create space
        for (int i = 0; i < fieldInfo->module()->numberOfSolutions(); i++)
        {
            Space<Scalar>* oneSpace;
            switch (fieldInfo->module()->spaces()[i+1].type())
            {
            case HERMES_L2_SPACE:
                oneSpace = new L2Space<Scalar>(meshes[fieldInfo].data(), fieldInfo->polynomialOrder() + fieldInfo->module()->spaces()[i+1].orderAdjust());
                break;
            case HERMES_H1_SPACE:
                oneSpace = new H1Space<Scalar>(meshes[fieldInfo].data(), bcs[i + m_block->offset(field)], fieldInfo->polynomialOrder() + fieldInfo->module()->spaces()[i+1].orderAdjust());
                break;
            default:
                assert(0);
                break;
            }

            //cout << "Space " << i << "dofs: " << actualSpace->get_num_dofs() << endl;
            m_actualSpaces.add(QSharedPointer<Space<Scalar> >(oneSpace), meshes[fieldInfo]);

            // set order by element
            foreach(SceneLabel* label, Util::scene()->labels->items())
            {
                if (!label->marker(fieldInfo)->isNone() &&
                        (fieldInfo->labelPolynomialOrder(label) != fieldInfo->polynomialOrder()))
                {
                    m_actualSpaces.at(i).data()->set_uniform_order(fieldInfo->labelPolynomialOrder(label),
                                                                   QString::number(Util::scene()->labels->items().indexOf(label)).toStdString());
                }
            }
        }
    }
}

template <typename Scalar>
void Solver<Scalar>::solveReferenceAndProject(int timeStep, int adaptivityStep, bool solutionExists)
{
    assert(0);
    //    SolutionMode solutionMode = solutionExists ? SolutionMode_Normal : SolutionMode_NonExisting;
    //    MultiSolutionArray<Scalar> msa = Util::solutionStore()->multiSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, solutionMode));
    //    MultiSolutionArray<Scalar> msaRef;

    //    // to be used as starting vector for the Newton solver
    //    MultiSolutionArray<Scalar> previousTSMultiSolutionArray;
    //    if((m_block->isTransient() && m_block->linearityType() != LinearityType_Linear) && (timeStep > 0))
    //        previousTSMultiSolutionArray = Util::solutionStore()->multiSolutionPreviousCalculatedTS(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal));

    //    // check for DOFs
    //    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(msa.spacesNakedConst()) == 0)
    //    {
    //        Util::log()->printDebug(m_solverID, QObject::tr("DOF is zero"));
    //        throw(AgrosSolverException("DOF is zero"));
    //    }

    //    // update timedep values
    //    foreach (Field* field, m_block->fields())
    //        field->fieldInfo()->module()->updateTimeFunctions(Util::problem()->actualTime());

    //    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(msa.spacesNaked(), Util::problem()->actualTime());

    //    BDF2ATable bdf2ATable;
    //    if(m_block->isTransient())
    //    {
    //        bdf2ATable.setOrder(min(timeStep, Util::problem()->config()->timeOrder()));
    //        bdf2ATable.setPreviousSteps(Util::problem()->timeStepLengths());
    //    }
    //    m_block->setWeakForm(QSharedPointer<WeakFormAgros<double> >(new WeakFormAgros<double>(m_block)));
    //    m_block->weakForm().data()->set_current_time(Util::problem()->actualTime());
    //    m_block->weakForm().data()->registerForms(&bdf2ATable);
    //    // msaRef.setSpaces(smartize(*Space<Scalar>::construct_refined_spaces(desmartize(msa.spaces()))));

    //    // create refined spaces
    //    Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaces = msa.spacesNaked();
    //    Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spacesRef;

    //    int spaceIdx = 0;
    //    foreach(Field* field, m_block->fields())
    //    {
    //        AdaptivityType adaptivityType = field->fieldInfo()->adaptivityType();
    //        for(int comp = 0; comp < field->fieldInfo()->module()->numberOfSolutions(); comp++)
    //        {
    //            Space<Scalar> *space = spaces.at(spaceIdx);
    //            Mesh *mesh;
    //            if((adaptivityType == AdaptivityType_P) && (! Util::config()->finerReference))
    //            {
    //                // use reference solution with increased poly order, but no mesh refinement
    //                mesh = new Mesh(); //TODO probably leak ... where is the mesh released
    //                mesh->copy(space->get_mesh());
    //            }
    //            else
    //            {
    //                // refine the mesh otherwise
    //                Mesh::ReferenceMeshCreator meshCreator(space->get_mesh());
    //                mesh = meshCreator.create_ref_mesh();
    //            }

    //            int orderIncrease = 1;
    //            if((adaptivityType == AdaptivityType_H) && (! Util::config()->finerReference))
    //            {
    //                // for h adaptivity, the standard way is not to increase the polynomial order for the reference solution
    //                orderIncrease = 0;
    //            }
    //            Space<double>::ReferenceSpaceCreator spaceCreator(space, mesh, orderIncrease);
    //            spacesRef.push_back(spaceCreator.create_ref_space());

    //            spaceIdx++;
    //        }
    //    }
    //    assert(spaces.size() == spacesRef.size());

    ////    for (unsigned int i = 0; i < spaces.size(); i++)
    ////    {
    ////        Hermes::Hermes2D::Space<Scalar> *space = spaces.at(i);

    ////        Mesh::ReferenceMeshCreator meshCreator(space->get_mesh());
    ////        Mesh *mesh = meshCreator.create_ref_mesh();

    ////        Space<double>::ReferenceSpaceCreator spaceCreator(space, mesh);
    ////        spacesRef.push_back(spaceCreator.create_ref_space());
    ////    }
    //    msaRef.setSpaces(smartize(spacesRef));

    //    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(msaRef.spacesNaked(), Util::problem()->actualTime());

    //    // create solutions
    //    msaRef.createNewSolutions();

    //    // solve reference problem
    //    solveOneProblem(msaRef, adaptivityStep, previousTSMultiSolutionArray.solutions());

    //    // project the fine mesh solution onto the coarse mesh.
    //    Hermes::Hermes2D::OGProjection<Scalar> ogProjection;
    //    ogProjection.project_global(castConst(msa.spacesNaked()),
    //                                msaRef.solutionsNaked(),
    //                                msa.solutionsNaked());

    //    msa.setTime(Util::problem()->actualTime());
    //    msaRef.setTime(Util::problem()->actualTime());

    //    Util::solutionStore()->removeSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, solutionMode));
    //    Util::solutionStore()->addSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal), msa);
    //    Util::solutionStore()->addSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Reference), msaRef);
}

template <typename Scalar>
bool Solver<Scalar>::createAdaptedSpace(int timeStep, int adaptivityStep)
{
    assert(0);
    //    MultiSolutionArray<Scalar> msa = Util::solutionStore()->multiSolution(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionMode_Normal));
    //    MultiSolutionArray<Scalar> msaRef = Util::solutionStore()->multiSolution(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionMode_Reference));

    //    Hermes::vector<Hermes::Hermes2D::ProjNormType> projNormType;
    //    Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *> selector;
    //    initSelectors(projNormType, selector);

    //    MultiSolutionArray<Scalar> msaNew = msa.copySpaces();
    //    msaNew.createNewSolutions();

    //    // calculate element errors and total error estimate.
    //    //cout << "adaptivity called with space " << msa.spacesNaked().at(0) << endl;
    //    Adapt<Scalar> adaptivity(msaNew.spacesNaked(), projNormType);
    //    adaptivity.set_verbose_output(true);

    //    // calculate error estimate for each solution component and the total error estimate.
    //    double error = adaptivity.calc_err_est(msa.solutionsNaked(), msaRef.solutionsNaked()) * 100;
    //    // cout << "ERROR " << error << endl;
    //    // set adaptive error
    //    // msa.setAdaptiveError(error);

    //    bool adapt = error >= m_block->adaptivityTolerance() && Hermes::Hermes2D::Space<Scalar>::get_num_dofs(msaNew.spacesNaked()) < Util::config()->maxDofs;
    //    // cout << "adapt " << adapt << ", error " << error << ", adpat tol " << m_block->adaptivityTolerance() << ", num dofs " <<  Hermes::Hermes2D::Space<Scalar>::get_num_dofs(msaNew.spacesNaked()) << ", max dofs " << Util::config()->maxDofs << endl;

    //    double initialDOFs = Hermes::Hermes2D::Space<Scalar>::get_num_dofs(msaNew.spacesNaked());

    //    // condition removed, adapt allways to allow to perform single adaptivity step in the future.
    //    // should be refacted.
    //    //    if (adapt)
    //    //    {
    //    cout << "*** starting adaptivity. dofs before adapt " << Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(msaNew.spacesNaked())) << "tr " << Util::config()->threshold <<
    //            ", st " << Util::config()->strategy << ", reg " << Util::config()->meshRegularity << endl;
    //    try
    //    {
    //        bool noref = adaptivity.adapt(selector,
    //                                      Util::config()->threshold,
    //                                      Util::config()->strategy,
    //                                      Util::config()->meshRegularity);
    //    }
    //    catch (Hermes::Exceptions::Exception e)
    //    {
    //        QString error = QString(e.what());
    //        Util::log()->printDebug(m_solverID, QObject::tr("Adaptive process failed: %1").arg(error));
    //        throw;
    //    }

    //    // cout << "last refined " << adaptivity.get_last_refinements().size() << endl;
    //    // cout << "adapted space dofs: " << Space<Scalar>::get_num_dofs(castConst(msaNew.spacesNaked())) << ", noref " << noref << endl;

    //    // store solution
    //    // msaNew.setTime(Util::problem()->actualTime());
    //    Util::solutionStore()->addSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_NonExisting), msaNew);

    //    Util::log()->printMessage(m_solverID, QObject::tr("adaptivity step (error = %1, DOFs = %2/%3)").
    //                              arg(error).
    //                              arg(initialDOFs).
    //                              arg(Space<Scalar>::get_num_dofs(castConst(msaNew.spacesNaked()))));

    //    deleteSelectors(selector);
    //    return adapt;
}

//template <typename Scalar>
//void Solver<Scalar>::saveSolution(BlockSolutionID id, Scalar* solutionVector)
//{
//    MultiSpace<Scalar> space;
//    if(id.solutionMode == SolutionMode_Normal)
//        space = m_actualSpace;
//    else if (id.solutionMode == SolutionMode_Reference)
//        space = m_actualReferenceSpace;
//    else
//        assert(0);

//    MultiSpace<Scalar> spaceCopy = space.deepCopy();

//    MultiSolution<scalar> solution;
//    Solution<Scalar>::vector_to_solutions(solutionVector, solutions, msa.solutionsNaked());


//    MultiSolutionArray<Scalar> msa(spaceCopy, solution, Util::problem()->actualTime());
//    Util::solutionStore()->addSolution(id, msa);
//}

template <typename Scalar>
void Solver<Scalar>::solveInitialTimeStep()
{
    Util::log()->printDebug(m_solverID, QObject::tr("initial time step"));

    MultiSolution<Scalar> solutions;
    int totalComp = 0;
    foreach(Field* field, m_block->fields())
    {
        for (int comp = 0; comp < field->fieldInfo()->module()->numberOfSolutions(); comp++)
        {
            // constant initial solution
            QSharedPointer<Mesh> mesh = m_actualSpaces.meshes().at(totalComp);
            ConstantSolution<double> *initial = new ConstantSolution<double>(mesh.data(), field->fieldInfo()->initialCondition().number());
            solutions.add(QSharedPointer<Solution<Scalar> >(initial), mesh);
            totalComp++;
        }
    }

    BlockSolutionID solutionID(m_block, 0, 0, SolutionMode_Normal);
    Util::solutionStore()->addSolution(solutionID, MultiSolutionArray<Scalar>(m_actualSpaces, solutions, Util::problem()->actualTime()));
}


//template class VectorStore<double>;
template class LinearSolverContainer<double>;
template class NewtonSolverContainer<double>;
template class PicardSolverContainer<double>;
template class Solver<double>;
