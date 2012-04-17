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
#include "weakform_parser.h"
#include "logview.h"
#include "../weakform/src/weakform_factory.h"

using namespace Hermes::Hermes2D;

template <typename Scalar>
void Solver<Scalar>::init(WeakFormAgros<Scalar> *wf, Block* block)
{
    m_block = block;
    m_wf = wf;
    isError = false;
}

template <typename Scalar>
QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> Solver<Scalar>::readMesh()
{
    // load the mesh file
    cout << "reading mesh in solver " << tempProblemFileName().toStdString() + ".xml" << endl;
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

                assert(marker >= 0);

                if (Util::scene()->edges->at(marker)->getMarker(fieldInfo) == SceneBoundaryContainer::getNone(fieldInfo))
                    boundaries.insert(marker);
            }
        }

        if (boundaries.count() > 0)
        {
            QString markers;
            foreach (int marker, boundaries)
                markers += QString::number(marker) + ", ";
            markers = markers.left(markers.length() - 2);

            Util::log()->printError(QObject::tr("Solver"), QObject::tr("boundary edges '%1' does not have a boundary marker").arg(markers));

            foreach(Mesh* delMesh, meshes)
                delete delMesh;
            meshes.clear();
            return meshes;
        }
        boundaries.clear();

        refineMesh(fieldInfo, mesh, true, true);
    }

    Util::problem()->setMeshesInitial(meshes);

    return meshes;
}


template <typename Scalar>
void Solver<Scalar>::createSpace(QMap<FieldInfo*, Mesh*> meshes, MultiSolutionArray<Scalar>& msa)
{
    Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > space;

    cout << "---- createSpace()" << endl;
    // essential boundary conditions
    Hermes::vector<Hermes::Hermes2D::EssentialBCs<double> *> bcs;
    for (int i = 0; i < m_block->numSolutions(); i++)
        bcs.push_back(new Hermes::Hermes2D::EssentialBCs<double>());

    foreach(Field* field, m_block->m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        int index = 0;
        foreach(SceneEdge* edge, Util::scene()->edges->items())
        {
            SceneBoundary *boundary = edge->getMarker(fieldInfo);

            if (boundary && (!boundary->isNone()))
            {
                //printf(" ---- chci typ %s\n", boundary->getType().data());
                Hermes::Module::BoundaryType *boundary_type = fieldInfo->module()->get_boundary_type(boundary->getType());

                cout << " ---- bdr type " << boundary_type->id.data() << ", " << boundary_type->name.data() << endl;
                for (Hermes::vector<ParserFormEssential *>::iterator it = boundary_type->essential.begin();
                     it < boundary_type->essential.end(); ++it)
                {
                    ParserFormEssential *form = ((ParserFormEssential *) *it);

                    Hermes::Hermes2D::EssentialBoundaryCondition<Scalar> *custom_form = NULL;

                    // compiled form
                    if (fieldInfo->weakFormsType() == WeakFormsType_Compiled)
                    {
                        string problemId = fieldInfo->module()->fieldid + "_" +
                                analysisTypeToStringKey(fieldInfo->module()->get_analysis_type()).toStdString()  + "_" +
                                coordinateTypeToStringKey(fieldInfo->module()->get_coordinate_type()).toStdString();

                        Hermes::Hermes2D::ExactSolutionScalar<double> * function = factoryExactSolution<double>(problemId, form->i, meshes[fieldInfo], boundary);
                        custom_form = new Hermes::Hermes2D::DefaultEssentialBCNonConst<double>(QString::number(index).toStdString(),
                                                                                               function);
                    }

                    if (!custom_form && fieldInfo->weakFormsType() == WeakFormsType_Compiled)
                        Util::log()->printMessage(QObject::tr("Weakform"), QObject::tr("Cannot find compiled VectorFormEssential()."));

                    // interpreted form
                    if (!custom_form || fieldInfo->weakFormsType() == WeakFormsType_Interpreted)
                    {
                        {
                            CustomExactSolution<double> *function = new CustomExactSolution<double>(meshes[fieldInfo],
                                                                                                    form->expression,
                                                                                                    boundary);
                            custom_form = new Hermes::Hermes2D::DefaultEssentialBCNonConst<double>(QString::number(index).toStdString(),
                                                                                                   function);
                        }
                    }

                    if (custom_form)
                    {
                        bcs[form->i - 1 + m_block->offset(field)]->add_boundary_condition(custom_form);
                        cout << "adding BC i: " << form->i - 1 + m_block->offset(field) << " ( form i " << form->i << ", " << m_block->offset(field) << "), expression: " << form->expression << endl;
                    }
                }
            }
            index++;
        }


        // create space
        for (int i = 0; i < fieldInfo->module()->number_of_solution(); i++)
        {
            space.push_back(shared_ptr<Space<Scalar> >(new Hermes::Hermes2D::H1Space<Scalar>(meshes[fieldInfo], bcs[i + m_block->offset(field)], fieldInfo->polynomialOrder())));

            int j = 0;
            // set order by element
            foreach(SceneLabel* label, Util::scene()->labels->items()){
                if (!label->getMarker(fieldInfo)->isNone())
                {
                    space.at(i)->set_uniform_order(label->polynomialOrder > 0 ? label->polynomialOrder : fieldInfo->polynomialOrder(),
                                                   QString::number(j).toStdString());
                    j++;
                }
            }
        }
    }

    msa.createEmpty(space.size());
    msa.setSpaces(space);
}

template <typename Scalar>
void  Solver<Scalar>::createNewSolutions(MultiSolutionArray<Scalar>& msa)
{
    for(int comp = 0; comp < msa.size(); comp++)
    {
        Mesh* mesh = msa.component(comp).space->get_mesh();
        msa.setSolution(shared_ptr<Solution<double> >(new Solution<double>(mesh)), comp);
    }
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
        if (m_block->adaptivityType() != AdaptivityType_None)
        {
            // add norm
            projNormType.push_back(Util::config()->projNormType);

            switch (m_block->adaptivityType())
            {
            case AdaptivityType_H:
                select = new Hermes::Hermes2D::RefinementSelectors::HOnlySelector<Scalar>();
                break;
            case AdaptivityType_P:
                select = new Hermes::Hermes2D::RefinementSelectors::H1ProjBasedSelector<Scalar>(Hermes::Hermes2D::RefinementSelectors::H2D_P_ANISO,
                                                                                                Util::config()->convExp,
                                                                                                H2DRS_DEFAULT_ORDER);
                break;
            case AdaptivityType_HP:
                select = new Hermes::Hermes2D::RefinementSelectors::H1ProjBasedSelector<Scalar>(Hermes::Hermes2D::RefinementSelectors::H2D_HP_ANISO,
                                                                                                Util::config()->convExp,
                                                                                                H2DRS_DEFAULT_ORDER);
                break;
            }
            // add refinement selector
            selector.push_back(select);
        }
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
//            msaInitial.setSolution(shared_ptr<Solution<Scalar> >(initial), totalComp);
//            totalComp++;
//        }
//    }
//    BlockSolutionID solutionID(m_block, 0, 0, SolutionType_Normal);
//    Util::solutionStore()->saveSolution(solutionID, msaInitial);
//}

template <typename Scalar>
Hermes::vector<shared_ptr<Space<Scalar> > > Solver<Scalar>::createCoarseSpace()
{
    Hermes::vector<shared_ptr<Space<Scalar> > > space;

    foreach(Field* field, m_block->m_fields)
    {
        MultiSolutionArray<Scalar> multiSolution = Util::solutionStore()->multiSolution(Util::solutionStore()->lastTimeAndAdaptiveSolution(field->fieldInfo(), SolutionType_Normal));
        for(int comp = 0; comp < field->fieldInfo()->module()->number_of_solution(); comp++)
        {
            Space<Scalar>* oldSpace = multiSolution.component(comp).space.get();
            Mesh* mesh = new Mesh();
            mesh->copy(oldSpace->get_mesh());

            space.push_back(shared_ptr<Space<Scalar> >(oldSpace->dup(mesh)));
        }
    }

    return space;
}


int DEBUG_COUNTER = 0;

template <typename Scalar>
bool Solver<Scalar>::solveOneProblem(MultiSolutionArray<Scalar> msa)
{
    // Initialize the FE problem.
    DiscreteProblem<Scalar> dp(m_wf, castConst(desmartize(msa.spaces())));

    // Linear solver
    // TODO original linear solver, now not used. We use Newton solver for linear problems instead. It should be put back....
    //    if (m_block->linearityType() == LinearityType_Linear)
    /*
    if(0)
    {
        // set up the solver, matrix, and rhs according to the solver selection.
        Hermes::Algebra::SparseMatrix<Scalar> *matrix = create_matrix<Scalar>(Hermes::SOLVER_UMFPACK);
        Hermes::Algebra::Vector<Scalar> *rhs = create_vector<Scalar>(Hermes::SOLVER_UMFPACK);
        Hermes::Algebra::LinearSolver<Scalar> *solver = create_linear_solver<Scalar>(Hermes::SOLVER_UMFPACK, matrix, rhs);

        // assemble the linear problem.
        dp.assemble(matrix, rhs);

        if (solver->solve())
        {
            Solution<Scalar>::vector_to_solutions(solver->get_sln_vector(), castConst(desmartize(msa.spaces())), desmartize(msa.solutions()));

            for(int i = 0; i < solver->get_matrix_size(); i++)
                cout << solver->get_sln_vector()[i] << ", ";
            cout << "  <- reseni " << endl;
            FILE *f;
            char fileName[30];
            sprintf(fileName, "rhs%d.txt", DEBUG_COUNTER);
            f = fopen(fileName, "w");
            rhs->dump(f, "rhs");
            fclose(f);

            sprintf(fileName, "mat%d.txt", DEBUG_COUNTER);
            f = fopen(fileName, "w");
            matrix->dump(f, "matrix");
            fclose(f);
            DEBUG_COUNTER++;

            Views::Linearizer lin;
            bool mode_3D = true;
            lin.save_solution_vtk(msa.solutions()[0].get(), "sln.vtk", "SLN", mode_3D);
        }
        else
        {
            m_progressItemSolve->emitMessage(QObject::tr("Linear solver failed."), true);
            return false;
        }

        delete matrix;
        delete rhs;
        delete solver;
    }
    */

    // Nonlinear solver
    if ((m_block->linearityType() == LinearityType_Newton) || (m_block->linearityType() == LinearityType_Linear))
    {
        Hermes::TimePeriod timer;

        // Perform Newton's iteration and translate the resulting coefficient vector into a Solution.
        NewtonSolver<Scalar> newton(&dp, Hermes::SOLVER_UMFPACK);
        newton.attach_timer(&timer);

        //newton.set_max_allowed_residual_norm(1e15);
        try
        {
            int ndof = Space<Scalar>::get_num_dofs(castConst(desmartize(msa.spaces())));

            // Initial coefficient vector for the Newton's method.
            Scalar* coeff_vec = new Scalar[ndof];
            memset(coeff_vec, 0, ndof*sizeof(Scalar));

            newton.solve(coeff_vec, m_block->nonlinearTolerance(), m_block->nonlinearSteps());

            Solution<Scalar>::vector_to_solutions(newton.get_sln_vector(), castConst(desmartize(msa.spaces())), desmartize(msa.solutions()));

            Util::log()->printDebug("Solver", QObject::tr("Newton's solver - assemble/solve/total: %1/%2/%3 s").
                                    arg(milisecondsToTime(newton.get_assemble_time() * 1000.0).toString("mm:ss.zzz")).
                                    arg(milisecondsToTime(newton.get_solve_time() * 1000.0).toString("mm:ss.zzz")).
                                    arg(milisecondsToTime((newton.get_assemble_time() + newton.get_solve_time()) * 1000.0).toString("mm:ss.zzz")));
            msa.setAssemblyTime(newton.get_assemble_time());
            msa.setSolveTime(newton.get_solve_time());
            delete coeff_vec;
        }
        catch(Hermes::Exceptions::Exception e)
        {
            QString error = QString(e.getMsg());
            Util::log()->printDebug(QObject::tr("Solver"), QObject::tr("Newton's iteration failed: %1").arg(error));
            return false;
        }
    }

    //    if (m_fieldInfo->linearityType == LinearityType_Picard)
    //    {
    //    }

    return true;

}

template <typename Scalar>
bool Solver<Scalar>::solveSimple()
{
    Util::log()->printDebug(QObject::tr("Solver"), QObject::tr("solve"));

    // read mesh from file
    QMap<FieldInfo*, Mesh*> meshes = readMesh();
    if (meshes.isEmpty())
        return false;

    MultiSolutionArray<Scalar> multiSolutionArray;

    // create essential boundary conditions and space
    createSpace(meshes, multiSolutionArray);

    // create solutions
    createNewSolutions(multiSolutionArray);

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(multiSolutionArray.spaces()))) == 0)
    {
        Util::log()->printDebug(QObject::tr("Solver"), QObject::tr("DOF is zero"));
        return false;
    }

    double actualTime = 0.0;

    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(desmartize(multiSolutionArray.spaces()), actualTime);
    m_wf->set_current_time(actualTime);

    m_wf->delete_all();
    m_wf->registerForms();

    if (!solveOneProblem(multiSolutionArray))
        isError = true;

    // output
    if (!isError)
    {
        BlockSolutionID solutionID;
        solutionID.group = m_block;
        solutionID.timeStep = 0;

        Util::solutionStore()->saveSolution(solutionID, multiSolutionArray);
    }

    return !isError;
}

template <typename Scalar>
bool Solver<Scalar>::solveInitialAdaptivityStep(int timeStep)
{
    Util::log()->printDebug(QObject::tr("Solver"), QObject::tr("initial adaptivity step"));

    // read mesh from file
    QMap<FieldInfo*, Mesh*> meshes = readMesh();
    if (meshes.isEmpty())
        return false;

    MultiSolutionArray<Scalar> msa;

    // create essential boundary conditions and space
    createSpace(meshes, msa);

    // create solutions
    createNewSolutions(msa);

    BlockSolutionID solutionID(m_block, timeStep, 0, SolutionType_NonExisting);
    Util::solutionStore()->saveSolution(solutionID, msa);

    Util::log()->printMessage(QObject::tr("Solver"), QObject::tr("initial adaptivity step"));

    return true;
}

template <typename Scalar>
bool Solver<Scalar>::solveAdaptivityStep(int timeStep, int adaptivityStep)
{
    MultiSolutionArray<Scalar> msa = Util::solutionStore()->multiSolution(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionType_NonExisting));
    MultiSolutionArray<Scalar> msaRef;

    cout << "solve adaptivity step " << adaptivityStep << endl;

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(msa.spaces()))) == 0)
    {
        Util::log()->printDebug(QObject::tr("Solver"), QObject::tr("DOF is zero"));
        return false;
    }

    double actualTime = 0.0;

    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(desmartize(msa.spaces()), actualTime);
    m_wf->set_current_time(actualTime);

    m_wf->delete_all();
    m_wf->registerForms();


    //    // construct refined spaces
    //    Hermes::vector<Space<Scalar> *> spaceReference
    //            = *Space<Scalar>::construct_refined_spaces(desmartize(msa.spaces()));

    msaRef.setSpaces(smartize(*Space<Scalar>::construct_refined_spaces(desmartize(msa.spaces()))));

    // create solutions
    createNewSolutions(msaRef);

    // solve reference problem
    if (!solveOneProblem(msaRef))
    {
        isError = true;
        // break;
    }

    Hermes::vector<Hermes::Hermes2D::ProjNormType> projNormType;
    Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *> selector;
    initSelectors(projNormType, selector);
    // project the fine mesh solution onto the coarse mesh.
    Hermes::Hermes2D::OGProjection<Scalar>::project_global(castConst(msa.spacesNaked()),
                                                           msaRef.solutionsNaked(),
                                                           msa.solutionsNaked(),
                                                           Util::problem()->config()->matrixSolver());

    // output
    if (!isError)
    {
        Util::solutionStore()->removeSolution(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionType_NonExisting));
        Util::solutionStore()->saveSolution(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionType_Normal), msa);
        Util::solutionStore()->saveSolution(BlockSolutionID(m_block, timeStep, adaptivityStep - 1, SolutionType_Reference), msaRef);
    }

    MultiSolutionArray<Scalar> msaNew = msa.copySpaces();
    createNewSolutions(msaNew);

    // calculate element errors and total error estimate.
    //cout << "adaptivity called with space " << msa.spacesNaked().at(0) << endl;
    Adapt<Scalar> adaptivity(msaNew.spacesNaked(), projNormType);

    // calculate error estimate for each solution component and the total error estimate.
    double error = adaptivity.calc_err_est(msa.solutionsNaked(), msaRef.solutionsNaked()) * 100;
    cout << "ERROR " << error << endl;
    msa.setAdaptiveError(error);

    //    // emit signal
    //    m_progressItemSolve->emitMessage(QObject::tr("Adaptivity rel. error (step: %2/%3, DOFs: %4/%5): %1%").
    //                                     arg(error, 0, 'f', 3).
    //                                     arg(i + 1).
    //                                     arg(maxAdaptivitySteps).
    //                                     arg(Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space)).
    //                                     arg(Hermes::Hermes2D::Space<Scalar>::get_num_dofs(spaceReference)), false, 1);
    //    // add error to the list
    //    m_progressItemSolve->addAdaptivityError(error, Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space));

    bool adapt = error >= m_block->adaptivityTolerance() && Hermes::Hermes2D::Space<Scalar>::get_num_dofs(msaNew.spacesNaked()) < Util::config()->maxDofs;
    cout << "adapt " << adapt << ", error " << error << ", adpat tol " << m_block->adaptivityTolerance() << ", num dofs " <<  Hermes::Hermes2D::Space<Scalar>::get_num_dofs(msaNew.spacesNaked()) << ", max dofs " << Util::config()->maxDofs << endl;

    double initialDOFs = Hermes::Hermes2D::Space<Scalar>::get_num_dofs(msaNew.spacesNaked());
    if (adapt)
    {
        cout << "*** starting adaptivity. dofs before adapt " << Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(msaNew.spacesNaked())) << "tr " << Util::config()->threshold <<
                ", st " << Util::config()->strategy << ", reg " << Util::config()->meshRegularity << endl;
        bool noref = adaptivity.adapt(selector,
                                      Util::config()->threshold,
                                      Util::config()->strategy,
                                      Util::config()->meshRegularity);

        cout << "last refined " << adaptivity.get_last_refinements().size() << endl;
        cout << "adapted space dofs: " << Space<Scalar>::get_num_dofs(castConst(msaNew.spacesNaked())) << ", noref " << noref << endl;

        Util::solutionStore()->saveSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionType_NonExisting), msaNew);
    }

    Util::log()->printMessage(QObject::tr("Solver"), QObject::tr("adaptivity step (error = %1, DOFs = %2/%3)").
                              arg(error).
                              arg(initialDOFs).
                              arg(Space<Scalar>::get_num_dofs(castConst(msaNew.spacesNaked()))));

    deleteSelectors(selector);
    return adapt;
}


template <typename Scalar>
bool Solver<Scalar>::solveInitialTimeStep()
{
    MultiSolutionArray<Scalar> multiSolutionArray;

    // read mesh from file
    QMap<FieldInfo*, Mesh*> meshes = readMesh();
    if (meshes.isEmpty())
        return false;

    // create essential boundary conditions and space
    createSpace(meshes, multiSolutionArray);

    int totalComp = 0;
    foreach(Field* field, m_block->m_fields)
    {
        for (int comp = 0; comp < field->fieldInfo()->module()->number_of_solution(); comp++)
        {
            // constant initial solution
            InitialCondition<double> *initial = new InitialCondition<double>(meshes[field->fieldInfo()], field->fieldInfo()->initialCondition().number());
            multiSolutionArray.setSolution(shared_ptr<Solution<Scalar> >(initial), totalComp);
            totalComp++;
        }
    }

    BlockSolutionID solutionID(m_block, 0, 0, SolutionType_Normal);
    Util::solutionStore()->saveSolution(solutionID, multiSolutionArray);

    return true;
}

template <typename Scalar>
bool Solver<Scalar>::solveTimeStep(double timeStep)
{
    BlockSolutionID previousSolutionID = Util::solutionStore()->lastTimeAndAdaptiveSolution(m_block, SolutionType_Normal);
    MultiSolutionArray<Scalar> multiSolutionArray = Util::solutionStore()->multiSolution(previousSolutionID);

    createNewSolutions(multiSolutionArray);

    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(multiSolutionArray.spaces()))) == 0)
    {
        Util::log()->printDebug(QObject::tr("Solver"), QObject::tr("DOF is zero"));
        return false;
    }

    double actualTime = Util::solutionStore()->lastTime(m_block) + timeStep;
    multiSolutionArray.setTime(actualTime);
    Util::log()->printDebug(QObject::tr("Solver"), QObject::tr("solve time step, actual time is %1 s").arg(actualTime));

    // update essential bc values
    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(desmartize(multiSolutionArray.spaces()), actualTime);

    // update timedep values
    foreach(Field* field, m_block->m_fields)
        field->fieldInfo()->module()->update_time_functions(actualTime);

    m_wf->set_current_time(actualTime);

    m_wf->delete_all();
    m_wf->registerForms();

    if (!solveOneProblem(multiSolutionArray))
        isError = true;

    // output
    if (!isError)
    {
        BlockSolutionID solutionID;
        solutionID.group = m_block;
        solutionID.timeStep = previousSolutionID.timeStep + 1;

        Util::solutionStore()->saveSolution(solutionID, multiSolutionArray);
    }

    return !isError;
}

template <typename Scalar>
void Solver<Scalar>::solve(SolverConfig config)
{
    assert(0);
    //  QTime time;

    //  double error = 0.0;

    // mesh file
    Mesh *mesh = NULL;
    //    Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > space;
    //    Hermes::vector<shared_ptr<Hermes::Hermes2D::Solution<Scalar> > > solution;
    MultiSolutionArray<Scalar> multiSolutionArray;
    Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > spaceCoarse;
    Hermes::vector<shared_ptr<Hermes::Hermes2D::Solution<Scalar> > > solutionCoarse;

    Hermes::vector<Hermes::Hermes2D::ProjNormType> projNormType;
    Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *select;
    Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *> selector;

    int lastTimeStep = Util::solutionStore()->lastTimeStep(m_block, SolutionType_Normal);
    int lastAdaptiveStep = Util::solutionStore()->lastAdaptiveStep(m_block, SolutionType_Normal);

    if((config.action == SolverAction_Solve) ||
            ((config.action == SolverAction_TimeStep) && (lastTimeStep < 1)))
    {
        // read mesh from file
        //mesh = readMesh();

        // create essential boundary conditions and space
        //createSpace(mesh, multiSolutionArray);
    }
    else if((config.action == SolverAction_TimeStep) && (lastTimeStep >= 1))
    {
        BlockSolutionID blockSID(m_block, lastTimeStep, lastAdaptiveStep, SolutionType_Normal);
        multiSolutionArray = Util::solutionStore()->multiSolution(blockSID);
    }
    else
        assert(0);

    if((config.action == SolverAction_TimeStep) && (lastTimeStep < 1))
    {
        //createInitialSolution(mesh, multiSolutionArray);
        lastTimeStep = 0;
    }

    if((config.action == SolverAction_AdaptivityStep))
    {
        //        solutionCoarse = createSolution();
        //        spaceCoarse = createCoarseSpace();
        //        // construct refined spaces
        //        space = smartize(*Space<Scalar>::construct_refined_spaces(desmartize(spaceCoarse)));
    }

    //    qDebug() << "nodes: " << mesh->get_num_nodes();
    //    qDebug() << "elements: " << mesh->get_num_elements();
    //    qDebug() << "ndof: " << Hermes::Hermes2D::Space<double>::get_num_dofs(castConst(desmartize(space)));

    // create solutions
    createNewSolutions(multiSolutionArray);

    // init selectors
    //initSelectors();

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(multiSolutionArray.spaces()))) == 0)
    {
        Util::log()->printDebug(QObject::tr("Solver"), QObject::tr("DOF is zero"));
        return;
    }

    double actualTime = 0.0;
    if(config.action == SolverAction_TimeStep)
        actualTime = Util::solutionStore()->lastTime(m_block) + config.timeStep;

    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(desmartize(multiSolutionArray.spaces()), actualTime);
    m_wf->set_current_time(actualTime);

    m_wf->delete_all();
    m_wf->registerForms();

    if (!solveOneProblem(multiSolutionArray))
        isError = true;

    if((config.action == SolverAction_AdaptivityStep))
    {
        //        // project the fine mesh solution onto the coarse mesh.
        //        Hermes::Hermes2D::OGProjection<Scalar>::project_global(spaceCoarse, solution, solutionCoarse, Util::problem()->config()->matrixSolver);

        //        // calculate element errors and total error estimate.
        //        Hermes::Hermes2D::Adapt<Scalar> adaptivity(spaceCoarse, projNormType);

        //        // calculate error estimate for each solution component and the total error estimate.
        //        double error = adaptivity.calc_err_est(solutionCoarse, solution) * 100;

        //        // emit signal
        ////        m_progressItemSolve->emitMessage(QObject::tr("Adaptivity rel. error (step: %2/%3, DOFs: %4/%5): %1%").
        ////                                         arg(error, 0, 'f', 3).
        ////                                         arg(i + 1).
        ////                                         arg(maxAdaptivitySteps).
        ////                                         arg(Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space)).
        ////                                         arg(Hermes::Hermes2D::Space<Scalar>::get_num_dofs(spaceReference)), false, 1);

        //        cout << (QObject::tr("Adaptivity rel. error (step: %2/%3, DOFs: %4/%5): %1%").
        //                 arg(error, 0, 'f', 3).
        //                 arg(lastAdaptiveStep + 1).
        //                 arg(1000).
        //                 arg(Space<Scalar>::get_num_dofs(spaceCoarse)).
        //                 arg(Space<Scalar>::get_num_dofs(space)), false, 1).toStdString() << endl;


        //        // add error to the list
        //        m_progressItemSolve->addAdaptivityError(error, Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space));

        //        if (error < adaptivityTolerance || Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space) >= adaptivityMaxDOFs)
        //        {
        //            break;
        //        }
        //        adaptivity.adapt(selector,
        //                         Util::config()->threshold,
        //                         Util::config()->strategy,
        //                         Util::config()->meshRegularity);

    }

    // output
    if (!isError)
    {
        //        foreach (Field* field, m_block->m_fields)
        //        {
        //            FieldInfo* fieldInfo = field->fieldInfo();
        //            // saving to sceneSolution .. in the future, sceneSolution should use solution from problems internal storage, see previous
        //            Util::scene()->sceneSolution(fieldInfo)->setSolutionArray(multiSolutionArray.fieldPart(m_block, fieldInfo));
        //        }

        BlockSolutionID solutionID;
        solutionID.group = m_block;
        solutionID.timeStep = lastTimeStep + 1;

        Util::solutionStore()->saveSolution(solutionID, multiSolutionArray);

        //        foreach (Field* field, m_block->m_fields)
        //        {
        //            FieldInfo* fieldInfo = field->fieldInfo();

        //            MultiSolutionArray<Scalar> multiSolutionArray;
        //            for(int component = 0; component < fieldInfo->module()->number_of_solution(); component++)
        //            {
        //                int position = component + m_block->offset(field);
        //                SolutionArray<Scalar> solutionArray(solution.at(position), space.at(position), 0, 0, actualTime);
        //                multiSolutionArray.addComponent(solutionArray);
        //            }

        //            // saving to sceneSolution .. in the future, sceneSolution should use solution from problems internal storage, see previous
        //            Util::scene()->sceneSolution(fieldInfo)->setSolutionArray(multiSolutionArray);

        //            FieldSolutionID solutionID;
        //            solutionID.group = fieldInfo;
        //            solutionID.timeStep = lastTimeStep + 1;

        //            Util::solutionStore()->saveSolution(solutionID, multiSolutionArray);
        //        }


    }

    //cleanup();

    if (isError)
    {
        //clear();
    }

}


template class Solver<double>;
