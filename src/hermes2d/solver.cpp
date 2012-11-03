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
VectorStore<Scalar>::~VectorStore()
{
    if(m_vector)
        delete[] m_vector;
}

template <typename Scalar>
VectorStore<Scalar>::VectorStore()  : m_vector(NULL), m_length(0)
{

}

template <typename Scalar>
Scalar* VectorStore<Scalar>::createNew(int length)
{
    if(m_vector)
        delete[] m_vector;
    m_vector = new Scalar[length];
    m_length = length;
    return m_vector;
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
HermesSolverContainer<Scalar>* HermesSolverContainer<Scalar>::factory(Block* block, Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > spaces)
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
LinearSolverContainer<Scalar>::LinearSolverContainer(Block* block, Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > spaces) : HermesSolverContainer<Scalar>(block)
{
    m_linearSolver = QSharedPointer<LinearSolver<Scalar> >(new LinearSolver<Scalar>(block->weakForm().data(), castConst(desmartize(spaces))));
}


template <typename Scalar>
void LinearSolverContainer<Scalar>::solve(Scalar* solutionVector)
{
    int ndof = Space<Scalar>::get_num_dofs(m_linearSolver.data()->get_spaces());
    m_linearSolver.data()->solve();
    memcpy(solutionVector, m_linearSolver.data()->get_sln_vector(), ndof * sizeof(Scalar));
}

template <typename Scalar>
NewtonSolverContainer<Scalar>::NewtonSolverContainer(Block* block, Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > spaces) : HermesSolverContainer<Scalar>(block)
{
    m_newtonSolver = QSharedPointer<NewtonSolver<Scalar> >(new NewtonSolver<Scalar>(block->weakForm().data(), castConst(desmartize(spaces))));
    m_newtonSolver.data()->set_verbose_output(true);
    m_newtonSolver.data()->set_verbose_callback(processSolverOutput);
    m_newtonSolver.data()->set_newton_tol(block->nonlinearTolerance());
    m_newtonSolver.data()->set_newton_max_iter(block->nonlinearSteps());
    m_newtonSolver.data()->set_max_allowed_residual_norm(1e15);
}

template <typename Scalar>
void NewtonSolverContainer<Scalar>::projectPreviousSolution(Scalar* solutionVector, Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > spaces,
                                                            Hermes::vector<QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > > solutions)
{
    if(solutions.empty())
    {
        int ndof = Space<Scalar>::get_num_dofs(castConst(desmartize(spaces)));
        memset(solutionVector, 0, ndof*sizeof(Scalar));
    }
    else
    {
        OGProjection<double> ogProjection;
        ogProjection.project_global(castConst(desmartize(spaces)),
                                    desmartize(solutions),
                                    solutionVector, this->m_block->projNormTypeVector());
    }
}

template <typename Scalar>
void NewtonSolverContainer<Scalar>::solve(Scalar* solutionVector)
{
    int ndof = Space<Scalar>::get_num_dofs(m_newtonSolver.data()->get_spaces());
    m_newtonSolver.data()->solve();
    memcpy(solutionVector, m_newtonSolver.data()->get_sln_vector(), ndof * sizeof(Scalar));
}

template <typename Scalar>
PicardSolverContainer<Scalar>::PicardSolverContainer(Block* block, Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > spaces) : HermesSolverContainer<Scalar>(block)
{
    m_picardSolver = QSharedPointer<PicardSolver<Scalar> >(new PicardSolver<Scalar>(block->weakForm().data(), castConst(desmartize(spaces))));
    m_picardSolver.data()->set_verbose_output(true);
    m_picardSolver.data()->set_verbose_callback(processSolverOutput);
    m_picardSolver.data()->set_picard_tol(block->nonlinearTolerance());
    m_picardSolver.data()->set_picard_max_iter(block->nonlinearSteps());
    //m_picardSolver.data()->set_max_allowed_residual_norm(1e15);
}

template <typename Scalar>
void PicardSolverContainer<Scalar>::projectPreviousSolution(Scalar* solutionVector, Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > spaces,
                                                            Hermes::vector<QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > > solutions)
{
    if(solutions.empty())
    {
        int ndof = Space<Scalar>::get_num_dofs(castConst(desmartize(spaces)));
        memset(solutionVector, 0, ndof*sizeof(Scalar));
    }
    else
    {
        OGProjection<double> ogProjection;
        ogProjection.project_global(castConst(desmartize(spaces)),
                                    desmartize(solutions),
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
QMap<FieldInfo*, Mesh*> Solver<Scalar>::readMesh()
{
    // load the mesh file
    QMap<FieldInfo*, Mesh*> meshes = readMeshesFromFile(tempProblemFileName() + ".xml");

    // check that all boundary edges have a marker assigned
    QSet<int> boundaries;
    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        Mesh* mesh = meshes[fieldInfo];
        for (int i = 0; i < mesh->get_max_node_id(); i++)
        {
            Node *node = mesh->get_node(i);


            if ((node->used == 1 && node->ref < 2 && node->type == 1))
            {
                int marker = atoi(mesh->get_boundary_markers_conversion().get_user_marker(node->marker).marker.c_str());

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

            foreach(Mesh* delMesh, meshes)
                delete delMesh;

            meshes.clear();
            return meshes;
        }
        boundaries.clear();
    }

    Util::problem()->setMeshesInitial(meshes);

    return meshes;
}


template <typename Scalar>
void Solver<Scalar>::createSpace(QMap<FieldInfo*, Mesh*> meshes, MultiSolutionArray<Scalar>& msa)
{
    Hermes::vector<QSharedPointer<Space<Scalar> > > space;

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
                    ExactSolutionScalarAgros<double> *function = plugin->exactSolution(problemId, form, meshes[fieldInfo]);
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
            Space<Scalar>* actualSpace;
            switch (fieldInfo->module()->spaces()[i+1].type())
            {
            case HERMES_L2_SPACE:
                actualSpace = new L2Space<Scalar>(meshes[fieldInfo], fieldInfo->polynomialOrder() + fieldInfo->module()->spaces()[i+1].orderAdjust());
                break;
            case HERMES_H1_SPACE:
                actualSpace = new H1Space<Scalar>(meshes[fieldInfo], bcs[i + m_block->offset(field)], fieldInfo->polynomialOrder() + fieldInfo->module()->spaces()[i+1].orderAdjust());
                break;
            default:
                assert(0);
                break;
            }

            //cout << "Space " << i << "dofs: " << actualSpace->get_num_dofs() << endl;
            space.push_back(QSharedPointer<Space<Scalar> >(actualSpace));

            // set order by element
            foreach(SceneLabel* label, Util::scene()->labels->items())
            {
                if (!label->marker(fieldInfo)->isNone() &&
                        (fieldInfo->labelPolynomialOrder(label) != fieldInfo->polynomialOrder()))
                {
                    space.at(i)->set_uniform_order(fieldInfo->labelPolynomialOrder(label),
                                                   QString::number(Util::scene()->labels->items().indexOf(label)).toStdString());
                }
            }
        }

        // unload plugin
        // loader.unload();
    }


    msa.createEmpty(space.size());
    msa.setSpaces(space);
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

        if (m_block->adaptivityType() == AdaptivityType_None)
        {
            select = new Hermes::Hermes2D::RefinementSelectors::H1ProjBasedSelector<Scalar>(Hermes::Hermes2D::RefinementSelectors::H2D_HP_ANISO,
                                                                                            Util::config()->convExp,
                                                                                            H2DRS_DEFAULT_ORDER);
        }
        else
        {
            switch (m_block->adaptivityType())
            {
            case AdaptivityType_H:
                select = new Hermes::Hermes2D::RefinementSelectors::HOnlySelector<Scalar>();
                break;
            case AdaptivityType_P:
                select = new Hermes::Hermes2D::RefinementSelectors::POnlySelector<Scalar>(H2DRS_DEFAULT_ORDER, 1, 1);
                /*
                select = new Hermes::Hermes2D::RefinementSelectors::H1ProjBasedSelector<Scalar>(Hermes::Hermes2D::RefinementSelectors::H2D_P_ANISO,
                                                                                                Util::config()->convExp,
                                                                                                H2DRS_DEFAULT_ORDER);
                */
                break;
            case AdaptivityType_HP:
                select = new Hermes::Hermes2D::RefinementSelectors::H1ProjBasedSelector<Scalar>(Hermes::Hermes2D::RefinementSelectors::H2D_HP_ANISO,
                                                                                                Util::config()->convExp,
                                                                                                H2DRS_DEFAULT_ORDER);
                break;
            }
        }

        // add refinement selector
        selector.push_back(select);
    }
}

template <typename Scalar>
void Solver<Scalar>::deleteSelectors(Hermes::vector<RefinementSelectors::Selector<Scalar> *>& selector)
{
    foreach(RefinementSelectors::Selector<Scalar> *select, selector)
    {
        delete select;
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

template <typename Scalar>
Hermes::vector<QSharedPointer<Space<Scalar> > > Solver<Scalar>::createCoarseSpace()
{
    Hermes::vector<QSharedPointer<Space<Scalar> > > space;

    foreach(Field* field, m_block->fields())
    {
        MultiSolutionArray<Scalar> multiSolution = Util::solutionStore()->multiSolution(Util::solutionStore()->lastTimeAndAdaptiveSolution(field->fieldInfo(), SolutionMode_Normal));
        for(int comp = 0; comp < field->fieldInfo()->module()->numberOfSolutions(); comp++)
        {
            Space<Scalar>* oldSpace = multiSolution.component(comp).space.data();

            Mesh *mesh = new Mesh(); //TODO probably leak ... where is the mesh released
            mesh->copy(oldSpace->get_mesh());

            // TODO: double -> Scalar
            Space<double>::ReferenceSpaceCreator spaceCreator(oldSpace, mesh, 0);
            space.push_back(QSharedPointer<Space<Scalar> >(spaceCreator.create_ref_space()));
        }
    }

    return space;
}

template <typename Scalar>
void Solver<Scalar>::solveOneProblem(Scalar *solutionVector, MultiSolutionArray<Scalar> msa, MultiSolutionArray<Scalar>* previousMsa, int adaptivityStep)
{
    Hermes::HermesCommonApi.set_param_value(Hermes::matrixSolverType, Util::problem()->config()->matrixSolver());

    m_hermesSolverContainer = HermesSolverContainer<Scalar>::factory(m_block, msa.spaces());

    try
    {
        if(previousMsa)
            m_hermesSolverContainer->projectPreviousSolution(solutionVector, msa.spaces(), previousMsa->solutions());
        else
            m_hermesSolverContainer->projectPreviousSolution(solutionVector, msa.spaces(), Hermes::vector<QSharedPointer<Solution<Scalar> > >());

        m_hermesSolverContainer->settableSpaces()->set_spaces(castConst(desmartize(msa.spaces())));
        m_hermesSolverContainer->setMatrixRhsOutput(m_solverName, adaptivityStep);
        m_hermesSolverContainer->solve(solutionVector);

        // todo: this probably should not be done here, since we return solution vector
        Solution<Scalar>::vector_to_solutions(solutionVector, castConst(desmartize(msa.spaces())), desmartize(msa.solutions()));

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



//    // Nonlinear solver
//    if (m_block->linearityType() == LinearityType_Newton)
//    {
//        assert(0);
////        m_discreteProblem = new DiscreteProblem<Scalar>(wf.data(), castConst(desmartize(msa.spaces())));

//        // Perform Newton's iteration and translate the resulting coefficient vector into a Solution.
//        NewtonSolver<Scalar> newton(m_discreteProblem);
//        newton.set_verbose_output(true);
//        newton.set_verbose_callback(processSolverOutput);
//        newton.set_newton_tol(m_block->nonlinearTolerance());
//        newton.set_newton_max_iter(m_block->nonlinearSteps());

//        newton.set_max_allowed_residual_norm(1e15);
//        try
//        {
//            if(previousMsa)
//            {
//                OGProjection<double> ogProjection;
//                ogProjection.project_global(castConst(desmartize(msa.spaces())),
//                                            desmartize(previousMsa->solutions()),
//                                            solutionVector, m_block->projNormTypeVector());

//            }
//            else
//                memset(solutionVector, 0, ndof*sizeof(Scalar));

//            newton.solve(solutionVector);

//            // todo: this probably should not be done here, since we return solution vector
//            Solution<Scalar>::vector_to_solutions(newton.get_sln_vector(), castConst(desmartize(msa.spaces())), desmartize(msa.solutions()));

//            // old way - delete the FE problem.
//            delete m_discreteProblem;

//            // TODO: temporarily disabled
//            /*
//            Util::log()->printDebug(m_solverID, QObject::tr("Newton's solver - assemble/solve/total: %1/%2/%3 s").
//                                    arg(milisecondsToTime(newton.get_assemble_time() * 1000.0).toString("mm:ss.zzz")).
//                                    arg(milisecondsToTime(newton.get_solve_time() * 1000.0).toString("mm:ss.zzz")).
//                                    arg(milisecondsToTime((newton.get_assemble_time() + newton.get_solve_time()) * 1000.0).toString("mm:ss.zzz")));
//            msa.setAssemblyTime(newton.get_assemble_time() * 1000.0);
//            msa.setSolveTime(newton.get_solve_time() * 1000.0);
//            */
//        }
//        catch (Hermes::Exceptions::Exception e)
//        {
//            QString error = QString(e.what());
//            Util::log()->printDebug(m_solverID, QObject::tr("Newton's iteration failed: %1").arg(error));
//            throw;
//        }
//    }

//    // Picard solver
//    if (m_block->linearityType() == LinearityType_Picard)
//    {
//        assert(0); // redo after refact with solutionVector
////        // Initialize the FE problem.
////        if(! m_discreteProblem)
////            m_discreteProblem = new DiscreteProblemLinear<Scalar>(wf.data(), castConst(desmartize(msa.spaces())));
////        else
////            m_discreteProblem->set_spaces(castConst(desmartize(msa.spaces())));

////        Hermes::vector<Solution<Scalar>* > slns;
////        for (int i = 0; i < msa.spaces().size(); i++)
////        {
////            QSharedPointer<Hermes::Hermes2D::Space<Scalar> > space = msa.spaces().at(i);
////            Hermes::Hermes2D::Space<Scalar> *spc = space.data();
////            slns.push_back(new Hermes::Hermes2D::ConstantSolution<double>(spc->get_mesh(), 0));
////        }
////        PicardSolver<Scalar> picard((DiscreteProblemLinear<Scalar>*)m_discreteProblem);
////        picard.setPreviousSolutions(slns);
////        picard.set_picard_tol(m_block->nonlinearTolerance());
////        picard.set_picard_max_iter(m_block->nonlinearSteps());
////        picard.set_verbose_output(true);
////        picard.set_verbose_callback(processSolverOutput);

////        try
////        {
////            picard.solve();
////            Solution<Scalar>::vector_to_solutions(picard.get_sln_vector(), castConst(desmartize(msa.spaces())), desmartize(msa.solutions()));

////            /*
////            Util::log()->printDebug(m_solverID, QObject::tr("Newton's solver - assemble/solve/total: %1/%2/%3 s").
////                                    arg(milisecondsToTime(picard.get_assemble_time() * 1000.0).toString("mm:ss.zzz")).
////                                    arg(milisecondsToTime(picard.get_solve_time() * 1000.0).toString("mm:ss.zzz")).
////                                    arg(milisecondsToTime((picard.get_assemble_time() + picard.get_solve_time()) * 1000.0).toString("mm:ss.zzz")));
////            msa.setAssemblyTime(picard.get_assemble_time() * 1000.0);
////            msa.setSolveTime(picard.get_solve_time() * 1000.0);
////            */
////        }
////        catch (Hermes::Exceptions::Exception e)
////        {
////            QString error = QString("%1").arg(e.what());
////            Util::log()->printDebug(m_solverID, QObject::tr("Picard's solver failed: %1").arg(error));
////            throw;
////        }
//    }
}

template <typename Scalar>
void Solver<Scalar>::solveSimple(int timeStep, int adaptivityStep, bool solutionExists)
{
    SolutionMode solutionMode = solutionExists ? SolutionMode_Normal : SolutionMode_NonExisting;

    MultiSolutionArray<Scalar> multiSolutionArray =
            Util::solutionStore()->multiSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, solutionMode));

    // to be used as starting vector for the Newton solver
    MultiSolutionArray<Scalar> previousTSMultiSolutionArray;
    if((m_block->isTransient() && m_block->linearityType() != LinearityType_Linear) && (timeStep > 0))
        previousTSMultiSolutionArray = Util::solutionStore()->multiSolutionPreviousCalculatedTS(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal));

    //cout << "Solving with " << Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(multiSolutionArray.spaces()))) << " dofs" << endl;

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(multiSolutionArray.spaces()))) == 0)
    {
        Util::log()->printDebug(m_solverID, QObject::tr("DOF is zero"));
        throw(AgrosSolverException("DOF is zero"));
    }

    cout << QString("updating with time %1\n").arg(Util::problem()->actualTime()).toStdString() << endl;

    // update timedep values
    foreach (Field* field, m_block->fields())
        field->fieldInfo()->module()->updateTimeFunctions(Util::problem()->actualTime());

    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(desmartize(multiSolutionArray.spaces()), Util::problem()->actualTime());

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

    int ndof = Space<Scalar>::get_num_dofs(castConst(desmartize(multiSolutionArray.spaces())));
    //Scalar* coefVec = new Scalar[ndof];
    Scalar* coefVec = m_lastVector.createNew(ndof);

    solveOneProblem(coefVec, multiSolutionArray, previousTSMultiSolutionArray.size() != 0 ? &previousTSMultiSolutionArray : NULL, adaptivityStep);
    //Solution<Scalar>::vector_to_solutions(coefVec, castConst(desmartize(multiSolutionArray.spaces())), desmartize(multiSolutionArray.solutions()));

    multiSolutionArray.setTime(Util::problem()->actualTime());

    // output
    BlockSolutionID solutionID;
    solutionID.group = m_block;
    solutionID.timeStep = timeStep;
    solutionID.adaptivityStep = adaptivityStep;

    cout << "saving solution, time step " << timeStep << endl;

    Util::solutionStore()->addSolution(solutionID, multiSolutionArray);

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

    MultiSolutionArray<Scalar> multiSolutionArray =
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

    int ndof = Space<Scalar>::get_num_dofs(castConst(desmartize(multiSolutionArray.spaces())));
    Scalar* coefVec2 = new Scalar[ndof];
    MultiSolutionArray<Scalar> multiSolutionArray2 = multiSolutionArray.copySpaces();
    multiSolutionArray2.createNewSolutions();

    // solve, for nonlinear solver use solution obtained by BDFA method as an initial vector
    solveOneProblem(coefVec2, multiSolutionArray2, timeStep > 0 ? &multiSolutionArray : NULL, adaptivityStep);

    double nextTimeStepLength = Util::problem()->config()->constantTimeStepLength();
    bool refuseThisStep = false;


    double error = Global<Scalar>::calc_abs_errors (desmartize(multiSolutionArray.solutions()), desmartize(multiSolutionArray2.solutions()));

    if(error > MAX_TOLERANCE_MULTIPLY_TO_ACCEPT * Util::problem()->config()->timeMethodTolerance().number())
        refuseThisStep = true;

    // this guess is based on assymptotic considerations (diploma thesis of Pavel Kus)
    nextTimeStepLength = pow(Util::problem()->config()->timeMethodTolerance().number() / error,
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

    // todo : Maybe delete coefVec2.

    // cout << "error: " << error << "(" << absError << ", " << absError / norm << ") -> step size " << Util::problem()->actualTimeStepLength() << " -> " << nextTimeStepLength << ", change " << pow(Util::problem()->config()->timeMethodTolerance().number()/error, 1./(Util::problem()->config()->timeOrder() + 1)) << endl;

    m_block->weakForm().clear();

    delete bdf2Table;
    return NextTimeStep(nextTimeStepLength, refuseThisStep);
}

template <typename Scalar>
void Solver<Scalar>::createInitialSpace(int timeStep)
{
    // read mesh from file
    QMap<FieldInfo*, Mesh*> meshes = readMesh();
    if (meshes.isEmpty())
        throw(AgrosSolverException("Meshes are empty"));

    MultiSolutionArray<Scalar> msa;

    // create essential boundary conditions and space
    createSpace(meshes, msa);

    msa.setTime(Util::problem()->actualTime());

    // create solutions
    msa.createNewSolutions();

    BlockSolutionID solutionID(m_block, timeStep, 0, SolutionMode_NonExisting);
    Util::solutionStore()->addSolution(solutionID, msa);
}

template <typename Scalar>
void Solver<Scalar>::solveReferenceAndProject(int timeStep, int adaptivityStep, bool solutionExists)
{
    SolutionMode solutionMode = solutionExists ? SolutionMode_Normal : SolutionMode_NonExisting;
    MultiSolutionArray<Scalar> msa = Util::solutionStore()->multiSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, solutionMode));
    MultiSolutionArray<Scalar> msaRef;

    // to be used as starting vector for the Newton solver
    MultiSolutionArray<Scalar> previousTSMultiSolutionArray;
    if((m_block->isTransient() && m_block->linearityType() != LinearityType_Linear) && (timeStep > 0))
        previousTSMultiSolutionArray = Util::solutionStore()->multiSolutionPreviousCalculatedTS(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal));

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(msa.spaces()))) == 0)
    {
        Util::log()->printDebug(m_solverID, QObject::tr("DOF is zero"));
        throw(AgrosSolverException("DOF is zero"));
    }

    // update timedep values
    foreach (Field* field, m_block->fields())
        field->fieldInfo()->module()->updateTimeFunctions(Util::problem()->actualTime());

    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(desmartize(msa.spaces()), Util::problem()->actualTime());

    BDF2ATable bdf2ATable;
    if(m_block->isTransient())
    {
        bdf2ATable.setOrder(min(timeStep, Util::problem()->config()->timeOrder()));
        bdf2ATable.setPreviousSteps(Util::problem()->timeStepLengths());
    }
    m_block->setWeakForm(QSharedPointer<WeakFormAgros<double> >(new WeakFormAgros<double>(m_block)));
    m_block->weakForm().data()->set_current_time(Util::problem()->actualTime());
    m_block->weakForm().data()->registerForms(&bdf2ATable);
    // msaRef.setSpaces(smartize(*Space<Scalar>::construct_refined_spaces(desmartize(msa.spaces()))));

    // create refined spaces
    Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaces = desmartize(msa.spaces());
    Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spacesRef;
    for (unsigned int i = 0; i < spaces.size(); i++)
    {
        Hermes::Hermes2D::Space<Scalar> *space = spaces.at(i);

        Mesh::ReferenceMeshCreator meshCreator(space->get_mesh());
        Mesh *mesh = meshCreator.create_ref_mesh();

        Space<double>::ReferenceSpaceCreator spaceCreator(space, mesh);
        spacesRef.push_back(spaceCreator.create_ref_space());
    }
    msaRef.setSpaces(smartize(spacesRef));

    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(desmartize(msaRef.spaces()), Util::problem()->actualTime());

    // create solutions
    msaRef.createNewSolutions();

    int ndof = Space<Scalar>::get_num_dofs(castConst(desmartize(msaRef.spaces())));
    Scalar* coeffVec = new Scalar[ndof];

    // solve reference problem
    solveOneProblem(coeffVec, msaRef, previousTSMultiSolutionArray.size() != 0 ? &previousTSMultiSolutionArray : NULL, adaptivityStep);

    // project the fine mesh solution onto the coarse mesh.
    Hermes::Hermes2D::OGProjection<Scalar> ogProjection;
    ogProjection.project_global(castConst(msa.spacesNaked()),
                                msaRef.solutionsNaked(),
                                msa.solutionsNaked());

    msa.setTime(Util::problem()->actualTime());
    msaRef.setTime(Util::problem()->actualTime());

    Util::solutionStore()->removeSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, solutionMode));
    Util::solutionStore()->addSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal), msa);
    Util::solutionStore()->addSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Reference), msaRef);
}

template <typename Scalar>
bool Solver<Scalar>::createAdaptedSpace(int timeStep, int adaptivityStep)
{
    MultiSolutionArray<Scalar> msa = Util::solutionStore()->multiSolution(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionMode_Normal));
    MultiSolutionArray<Scalar> msaRef = Util::solutionStore()->multiSolution(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionMode_Reference));

    Hermes::vector<Hermes::Hermes2D::ProjNormType> projNormType;
    Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *> selector;
    initSelectors(projNormType, selector);

    MultiSolutionArray<Scalar> msaNew = msa.copySpaces();
    msaNew.createNewSolutions();

    // calculate element errors and total error estimate.
    //cout << "adaptivity called with space " << msa.spacesNaked().at(0) << endl;
    Adapt<Scalar> adaptivity(msaNew.spacesNaked(), projNormType);
    adaptivity.set_verbose_output(true);

    // calculate error estimate for each solution component and the total error estimate.
    double error = adaptivity.calc_err_est(msa.solutionsNaked(), msaRef.solutionsNaked()) * 100;
    // cout << "ERROR " << error << endl;
    // set adaptive error
    // msa.setAdaptiveError(error);

    bool adapt = error >= m_block->adaptivityTolerance() && Hermes::Hermes2D::Space<Scalar>::get_num_dofs(msaNew.spacesNaked()) < Util::config()->maxDofs;
    // cout << "adapt " << adapt << ", error " << error << ", adpat tol " << m_block->adaptivityTolerance() << ", num dofs " <<  Hermes::Hermes2D::Space<Scalar>::get_num_dofs(msaNew.spacesNaked()) << ", max dofs " << Util::config()->maxDofs << endl;

    double initialDOFs = Hermes::Hermes2D::Space<Scalar>::get_num_dofs(msaNew.spacesNaked());

    // condition removed, adapt allways to allow to perform single adaptivity step in the future.
    // should be refacted.
    //    if (adapt)
    //    {
    cout << "*** starting adaptivity. dofs before adapt " << Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(msaNew.spacesNaked())) << "tr " << Util::config()->threshold <<
            ", st " << Util::config()->strategy << ", reg " << Util::config()->meshRegularity << endl;
    try
    {
        bool noref = adaptivity.adapt(selector,
                                      Util::config()->threshold,
                                      Util::config()->strategy,
                                      Util::config()->meshRegularity);
    }
    catch (Hermes::Exceptions::Exception e)
    {
        QString error = QString(e.what());
        Util::log()->printDebug(m_solverID, QObject::tr("Adaptive process failed: %1").arg(error));
        throw;
    }

    // cout << "last refined " << adaptivity.get_last_refinements().size() << endl;
    // cout << "adapted space dofs: " << Space<Scalar>::get_num_dofs(castConst(msaNew.spacesNaked())) << ", noref " << noref << endl;

    // store solution
    // msaNew.setTime(Util::problem()->actualTime());
    Util::solutionStore()->addSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_NonExisting), msaNew);

    Util::log()->printMessage(m_solverID, QObject::tr("adaptivity step (error = %1, DOFs = %2/%3)").
                              arg(error).
                              arg(initialDOFs).
                              arg(Space<Scalar>::get_num_dofs(castConst(msaNew.spacesNaked()))));

    deleteSelectors(selector);
    return adapt;
}


template <typename Scalar>
void Solver<Scalar>::solveInitialTimeStep()
{
    Util::log()->printDebug(m_solverID, QObject::tr("initial time step"));

    MultiSolutionArray<Scalar> multiSolutionArray;

    // read mesh from file
    QMap<FieldInfo*, Mesh*> meshes = readMesh();
    if (meshes.isEmpty())
        throw(AgrosSolverException("No meshes set"));

    // create essential boundary conditions and space
    createSpace(meshes, multiSolutionArray);

    int totalComp = 0;
    foreach(Field* field, m_block->fields())
    {
        for (int comp = 0; comp < field->fieldInfo()->module()->numberOfSolutions(); comp++)
        {
            // constant initial solution
            ConstantSolution<double> *initial = new ConstantSolution<double>(meshes[field->fieldInfo()], field->fieldInfo()->initialCondition().number());
            multiSolutionArray.setSolution(QSharedPointer<Solution<Scalar> >(initial), totalComp);
            totalComp++;
        }
    }

    BlockSolutionID solutionID(m_block, 0, 0, SolutionMode_Normal);
    Util::solutionStore()->addSolution(solutionID, multiSolutionArray);
}

//template <typename Scalar>
//void Solver<Scalar>::solveTimeStep()
//{
//    BlockSolutionID previousSolutionID = Util::solutionStore()->lastTimeAndAdaptiveSolution(m_block, SolutionMode_Normal);
//    MultiSolutionArray<Scalar> multiSolutionArray = Util::solutionStore()->
//            multiSolution(previousSolutionID);

//    createNewSolutions(multiSolutionArray);

//    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(multiSolutionArray.spaces()))) == 0)
//    {
//        Util::log()->printDebug(m_solverID, QObject::tr("DOF is zero"));
//        throw(AgrosSolverException("DOF is zero"));
//    }

//    multiSolutionArray.setTime(Util::problem()->actualTime());
//    Util::log()->printDebug(m_solverID, QObject::tr("solve time step, actual time is %1 s").arg(Util::problem()->actualTime()));

//    // update timedep values
//    foreach (Field* field, m_block->fields())
//        field->fieldInfo()->module()->updateTimeFunctions(Util::problem()->actualTime());

//    // update essential bc values
//    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(desmartize(multiSolutionArray.spaces()),
//                                                                Util::problem()->actualTime());

//    m_wf->set_current_time(Util::problem()->actualTime());

//    m_wf->delete_all();
//    m_wf->registerForms();

//    solveOneProblem(multiSolutionArray);

//    // output
//    BlockSolutionID solutionID;
//    solutionID.group = m_block;
//    solutionID.timeStep = previousSolutionID.timeStep + 1;

//    Util::solutionStore()->addSolution(solutionID, multiSolutionArray);

//}


template class VectorStore<double>;
template class LinearSolverContainer<double>;
template class NewtonSolverContainer<double>;
template class PicardSolverContainer<double>;
template class Solver<double>;
