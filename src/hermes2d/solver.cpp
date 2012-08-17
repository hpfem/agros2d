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
#include "logview.h"
#include "util.h"
#include "../weakform/src/weakform_factory.h"

using namespace Hermes::Hermes2D;

template<typename Scalar>
class NewtonSolverAgros : public NewtonSolver<Scalar>
{
public:
    NewtonSolverAgros(DiscreteProblem<Scalar>* dp)
        : NewtonSolver<Scalar>(dp) {}

    virtual void onInitialization()
    {

    }

    virtual void onStepBegin()
    {

    }

    virtual void onStepEnd()
    {
        // implement to the chart
        // qDebug() << "residual_norm = " << Global<Scalar>::get_l2_norm(this->residual);
    }

    virtual void onFinish()
    {

    }
};

template<typename Scalar>
class PicardSolverAgros : public PicardSolver<Scalar>
{
public:
    PicardSolverAgros(DiscreteProblemLinear<Scalar>* dp, Solution<Scalar>* sln_prev_iter)
        : PicardSolver<Scalar>(dp, sln_prev_iter) {}
    PicardSolverAgros(DiscreteProblemLinear<Scalar>* dp, Hermes::vector<Solution<Scalar>* > slns_prev_iter)
        : PicardSolver<Scalar>(dp, slns_prev_iter) {}

    virtual void onInitialization()
    {

    }

    virtual void onStepBegin()
    {

    }

    virtual void onStepEnd()
    {
        // implement to the chart
        // qDebug() << "residual_norm = " << Global<Scalar>::get_l2_norm(this->residual);
    }

    virtual void onFinish()
    {

    }
};

int DEBUG_COUNTER = 0;

void processSolverOutput(const char* aha)
{
    QString str = QString(aha).trimmed();
    Util::log()->printMessage(QObject::tr("Solver"), str.replace("---- ", ""));
}

template <typename Scalar>
void Solver<Scalar>::init(WeakFormAgros<Scalar> *wf, Block* block)
{
    m_block = block;
    m_wf = wf;

    m_solverID = QObject::tr("Solver") + " (";
    QListIterator<Field*> iter(m_block->fields());
    while (iter.hasNext())
    {
        m_solverID += iter.next()->fieldInfo()->fieldId();
        if (iter.hasNext())
            m_solverID += ", ";
    }
    m_solverID += ")";
}

template <typename Scalar>
QMap<FieldInfo*, Mesh*> Solver<Scalar>::readMesh()
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

                if (Util::scene()->edges->at(marker)->marker(fieldInfo) == SceneBoundaryContainer::getNone(fieldInfo))
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

        refineMesh(fieldInfo, mesh, true, true);
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
        int index = 0;
        foreach(SceneEdge* edge, Util::scene()->edges->items())
        {
            SceneBoundary *boundary = edge->marker(fieldInfo);

            if (boundary && (!boundary->isNone()))
            {
                Module::BoundaryType *boundary_type = fieldInfo->module()->boundaryType(boundary->getType());

                foreach (FormInfo *form, boundary_type->essential())
                {
                    EssentialBoundaryCondition<Scalar> *custom_form = NULL;

                    string problemId = fieldInfo->fieldId().toStdString() + "_" +
                            analysisTypeToStringKey(fieldInfo->module()->analysisType()).toStdString()  + "_" +
                            coordinateTypeToStringKey(fieldInfo->module()->coordinateType()).toStdString() + "_" +
                            ((field->fieldInfo()->linearityType() == LinearityType_Newton) ? "newton" : "linear") + "_";

                    ExactSolutionScalar<double> * function = factoryExactSolution<double>(problemId, form->i, meshes[fieldInfo], boundary);
                    custom_form = new DefaultEssentialBCNonConst<double>(QString::number(index).toStdString(), function);

                    if (!custom_form)
                        Util::log()->printMessage(QObject::tr("Weakform"), QObject::tr("Cannot find compiled EssentialBoundaryCondition()."));

                    if (custom_form)
                    {
                        bcs[form->i - 1 + m_block->offset(field)]->add_boundary_condition(custom_form);
                        //  cout << "adding BC i: " << form->i - 1 + m_block->offset(field) << " ( form i " << form->i << ", " << m_block->offset(field) << "), expression: " << form->expression << endl;
                    }
                }
            }
            index++;
        }

        // create space
        for (int i = 0; i < fieldInfo->module()->numberOfSolutions(); i++)
        {
            Space<Scalar>* actualSpace;
            if(fieldInfo->module()->spaceType(i) == HERMES_L2_SPACE)
                actualSpace = new L2Space<Scalar>(meshes[fieldInfo], fieldInfo->polynomialOrder() + fieldInfo->module()->spaceOrderAdjust(i));
            else if(fieldInfo->module()->spaceType(i) == HERMES_H1_SPACE)
                actualSpace = new H1Space<Scalar>(meshes[fieldInfo], bcs[i + m_block->offset(field)], fieldInfo->polynomialOrder() + fieldInfo->module()->spaceOrderAdjust(i));
            else
                assert(0);
            cout << "Space " << i << "dofs: " << actualSpace->get_num_dofs() << endl;
            space.push_back(QSharedPointer<Space<Scalar> >(actualSpace));

            int j = 0;
            // set order by element
            foreach(SceneLabel* label, Util::scene()->labels->items()){
                if (!label->marker(fieldInfo)->isNone())
                {
                    // TODO: set order in space
                    // space.at(i)->set_uniform_order(label->polynomialOrder > 0 ? label->polynomialOrder : fieldInfo->polynomialOrder() + fieldInfo->module()->spaceOrderAdjust(i),
                    //                                QString::number(j).toStdString());
                    space.at(i)->set_uniform_order(fieldInfo->polynomialOrder() + fieldInfo->module()->spaceOrderAdjust(i),
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
        msa.setSolution(QSharedPointer<Solution<double> >(new Solution<double>(mesh)), comp);
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
            Mesh* mesh = new Mesh();
            mesh->copy(oldSpace->get_mesh());

            space.push_back(QSharedPointer<Space<Scalar> >(oldSpace->dup(mesh)));
        }
    }

    return space;
}

template <typename Scalar>
void Solver<Scalar>::solveOneProblem(MultiSolutionArray<Scalar> msa, MultiSolutionArray<Scalar>* previousMsa)
{
    Hermes::HermesCommonApi.setParamValue(Hermes::matrixSolverType, Util::problem()->config()->matrixSolver());

    // Linear solver
    if (m_block->linearityType() == LinearityType_Linear)
    {
        // Initialize the FE problem.
        DiscreteProblemLinear<Scalar> dp(m_wf, castConst(desmartize(msa.spaces())));

        LinearSolver<Scalar> linear(&dp);
        try
        {
            linear.solve();
            Solution<Scalar>::vector_to_solutions(linear.get_sln_vector(), castConst(desmartize(msa.spaces())), desmartize(msa.solutions()));

            /*
            Util::log()->printDebug(m_sovlerID, QObject::tr("Linear solver - assemble/solve/total: %1/%2/%3 s").
                                    arg(milisecondsToTime(linear.get_assemble_time() * 1000.0).toString("mm:ss.zzz")).
                                    arg(milisecondsToTime(linear.get_solve_time() * 1000.0).toString("mm:ss.zzz")).
                                    arg(milisecondsToTime((linear.get_assemble_time() + linear.get_solve_time()) * 1000.0).toString("mm:ss.zzz")));
            msa.setAssemblyTime(linear.get_assemble_time() * 1000.0);
            msa.setSolveTime(linear.get_solve_time() * 1000.0);
            */
        }
        catch (Hermes::Exceptions::Exception e)
        {
            QString error = QString("%1").arg(e.what());
            Util::log()->printDebug(m_solverID, QObject::tr("Linear solver failed: %1").arg(error));
            throw;
        }
    }

    // Nonlinear solver
    if (m_block->linearityType() == LinearityType_Newton)
    {
        // Initialize the FE problem.
        DiscreteProblem<Scalar> dp(m_wf, castConst(desmartize(msa.spaces())));

        // Perform Newton's iteration and translate the resulting coefficient vector into a Solution.
        NewtonSolverAgros<Scalar> newton(&dp);
        newton.set_verbose_output(true);
        newton.set_verbose_callback(processSolverOutput);
        newton.set_newton_tol(m_block->nonlinearTolerance());
        newton.set_newton_max_iter(m_block->nonlinearSteps());

        newton.set_max_allowed_residual_norm(1e15);
        try
        {
            int ndof = Space<Scalar>::get_num_dofs(castConst(desmartize(msa.spaces())));

            // Initial coefficient vector for the Newton's method.
            Scalar* coeff_vec = new Scalar[ndof];

            if(previousMsa)
            {
                OGProjection<double> ogProjection;
                ogProjection.project_global(castConst(desmartize(msa.spaces())),
                                            desmartize(previousMsa->solutions()),
                                            coeff_vec, m_block->projNormTypeVector());

            }
            else
                memset(coeff_vec, 0, ndof*sizeof(Scalar));

            newton.solve(coeff_vec);
            Solution<Scalar>::vector_to_solutions(newton.get_sln_vector(), castConst(desmartize(msa.spaces())), desmartize(msa.solutions()));

            Util::log()->printDebug(m_solverID, QObject::tr("Newton's solver - assemble/solve/total: %1/%2/%3 s").
                                    arg(milisecondsToTime(newton.get_assemble_time() * 1000.0).toString("mm:ss.zzz")).
                                    arg(milisecondsToTime(newton.get_solve_time() * 1000.0).toString("mm:ss.zzz")).
                                    arg(milisecondsToTime((newton.get_assemble_time() + newton.get_solve_time()) * 1000.0).toString("mm:ss.zzz")));
            msa.setAssemblyTime(newton.get_assemble_time() * 1000.0);
            msa.setSolveTime(newton.get_solve_time() * 1000.0);
            delete coeff_vec;
        }
        catch (Hermes::Exceptions::Exception e)
        {
            QString error = QString(e.what());
            Util::log()->printDebug(m_solverID, QObject::tr("Newton's iteration failed: %1").arg(error));
            throw;
        }
    }

    // Picard solver
    if (m_block->linearityType() == LinearityType_Picard)
    {
        // Initialize the FE problem.
        DiscreteProblemLinear<Scalar> dp(m_wf, castConst(desmartize(msa.spaces())));

        Hermes::vector<Solution<Scalar>* > slns;
        for (int i = 0; i < msa.spaces().size(); i++)
        {
            QSharedPointer<Hermes::Hermes2D::Space<Scalar> > space = msa.spaces().at(i);
            Hermes::Hermes2D::Space<Scalar> *spc = space.data();
            slns.push_back(new Hermes::Hermes2D::ConstantSolution<double>(spc->get_mesh(), 0));
        }
        PicardSolverAgros<Scalar> picard(&dp, slns);
        // picard.attach_timer(&timer);
        picard.set_verbose_output(true);
        picard.set_verbose_callback(processSolverOutput);

        try
        {
            picard.solve(m_block->nonlinearTolerance(), m_block->nonlinearSteps());
            Solution<Scalar>::vector_to_solutions(picard.get_sln_vector(), castConst(desmartize(msa.spaces())), desmartize(msa.solutions()));

            /*
            Util::log()->printDebug(m_solverID, QObject::tr("Newton's solver - assemble/solve/total: %1/%2/%3 s").
                                    arg(milisecondsToTime(picard.get_assemble_time() * 1000.0).toString("mm:ss.zzz")).
                                    arg(milisecondsToTime(picard.get_solve_time() * 1000.0).toString("mm:ss.zzz")).
                                    arg(milisecondsToTime((picard.get_assemble_time() + picard.get_solve_time()) * 1000.0).toString("mm:ss.zzz")));
            msa.setAssemblyTime(picard.get_assemble_time() * 1000.0);
            msa.setSolveTime(picard.get_solve_time() * 1000.0);
            */
        }
        catch (Hermes::Exceptions::Exception e)
        {
            QString error = QString("%1").arg(e.what());
            Util::log()->printDebug(m_solverID, QObject::tr("Picard's solver failed: %1").arg(error));
            throw;
        }
    }
}

template <typename Scalar>
void Solver<Scalar>::solveSimple(int timeStep, int adaptivityStep, bool solutionExists)
{
    SolutionMode solutionMode = solutionExists ? SolutionMode_Normal : SolutionMode_NonExisting;
    Util::log()->printDebug(m_solverID, QObject::tr("solve"));

    MultiSolutionArray<Scalar> multiSolutionArray =
            Util::solutionStore()->multiSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, solutionMode));;

    MultiSolutionArray<Scalar> previousTSMultiSolutionArray;
    if(timeStep > 0)
        previousTSMultiSolutionArray = Util::solutionStore()->multiSolutionPreviousCalculatedTS(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_Normal));;

    cout << "Solving with " << Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(multiSolutionArray.spaces()))) << " dofs" << endl;

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(desmartize(multiSolutionArray.spaces()))) == 0)
    {
        Util::log()->printDebug(m_solverID, QObject::tr("DOF is zero"));
        throw(AgrosSolverException("DOF is zero"));
    }

    // update timedep values
    foreach (Field* field, m_block->fields())
        field->fieldInfo()->module()->updateTimeFunctions(Util::problem()->actualTime());

    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(desmartize(multiSolutionArray.spaces()), Util::problem()->actualTime());
    m_wf->set_current_time(Util::problem()->actualTime());

    m_wf->delete_all();
    m_wf->registerForms();

    solveOneProblem(multiSolutionArray, timeStep > 0 ? &previousTSMultiSolutionArray : NULL);

    multiSolutionArray.setTime(Util::problem()->actualTime());

    // output
    BlockSolutionID solutionID;
    solutionID.group = m_block;
    solutionID.timeStep = timeStep;
    solutionID.adaptivityStep = adaptivityStep;

    Util::solutionStore()->addSolution(solutionID, multiSolutionArray);
}

template <typename Scalar>
void Solver<Scalar>::createInitialSpace(int timeStep)
{
    Util::log()->printDebug(m_solverID, QObject::tr("initial adaptivity step"));

    // read mesh from file
    QMap<FieldInfo*, Mesh*> meshes = readMesh();
    if (meshes.isEmpty())
        throw(AgrosSolverException("Meshes are empty"));

    MultiSolutionArray<Scalar> msa;

    // create essential boundary conditions and space
    createSpace(meshes, msa);

    msa.setTime(Util::problem()->actualTime());

    // create solutions
    createNewSolutions(msa);

    BlockSolutionID solutionID(m_block, timeStep, 0, SolutionMode_NonExisting);
    Util::solutionStore()->addSolution(solutionID, msa);
}

template <typename Scalar>
void Solver<Scalar>::solveReferenceAndProject(int timeStep, int adaptivityStep, bool solutionExists)
{
    SolutionMode solutionMode = solutionExists ? SolutionMode_Normal : SolutionMode_NonExisting;
    MultiSolutionArray<Scalar> msa = Util::solutionStore()->multiSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, solutionMode));
    MultiSolutionArray<Scalar> msaRef;

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
    m_wf->set_current_time(Util::problem()->actualTime());

    m_wf->delete_all();
    m_wf->registerForms();

    msaRef.setSpaces(smartize(*Space<Scalar>::construct_refined_spaces(desmartize(msa.spaces()))));
    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(desmartize(msaRef.spaces()), Util::problem()->actualTime());

    // create solutions
    createNewSolutions(msaRef);

    // solve reference problem
    solveOneProblem(msaRef);

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
    createNewSolutions(msaNew);

    // calculate element errors and total error estimate.
    //cout << "adaptivity called with space " << msa.spacesNaked().at(0) << endl;
    Adapt<Scalar> adaptivity(msaNew.spacesNaked(), projNormType);

    // calculate error estimate for each solution component and the total error estimate.
    double error = adaptivity.calc_err_est(msa.solutionsNaked(), msaRef.solutionsNaked()) * 100;
    // cout << "ERROR " << error << endl;
    // set adaptive error
    msa.setAdaptiveError(error);

    bool adapt = error >= m_block->adaptivityTolerance() && Hermes::Hermes2D::Space<Scalar>::get_num_dofs(msaNew.spacesNaked()) < Util::config()->maxDofs;
    // cout << "adapt " << adapt << ", error " << error << ", adpat tol " << m_block->adaptivityTolerance() << ", num dofs " <<  Hermes::Hermes2D::Space<Scalar>::get_num_dofs(msaNew.spacesNaked()) << ", max dofs " << Util::config()->maxDofs << endl;

    double initialDOFs = Hermes::Hermes2D::Space<Scalar>::get_num_dofs(msaNew.spacesNaked());

    // condition removed, adapt allways to allow to perform single adaptivity step in the future.
    // should be refacted.
    //    if (adapt)
    //    {
    cout << "*** starting adaptivity. dofs before adapt " << Hermes::Hermes2D::Space<Scalar>::get_num_dofs(castConst(msaNew.spacesNaked())) << "tr " << Util::config()->threshold <<
            ", st " << Util::config()->strategy << ", reg " << Util::config()->meshRegularity << endl;
    bool noref = adaptivity.adapt(selector,
                                  Util::config()->threshold,
                                  Util::config()->strategy,
                                  Util::config()->meshRegularity);

    // cout << "last refined " << adaptivity.get_last_refinements().size() << endl;
    // cout << "adapted space dofs: " << Space<Scalar>::get_num_dofs(castConst(msaNew.spacesNaked())) << ", noref " << noref << endl;

    // store solution
    msaNew.setTime(Util::problem()->actualTime());
    Util::solutionStore()->addSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, SolutionMode_NonExisting), msaNew);
    //    }


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


template class Solver<double>;
