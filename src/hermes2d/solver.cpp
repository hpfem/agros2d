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
    PicardSolverAgros(DiscreteProblemLinear<Scalar>* dp) : PicardSolver<Scalar>(dp) {}

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
void Solver<Scalar>::init(Block* block)
{
    m_block = block;

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
            if(fieldInfo->module()->spaceType(i) == HERMES_L2_SPACE)
                actualSpace = new L2Space<Scalar>(meshes[fieldInfo], fieldInfo->polynomialOrder() + fieldInfo->module()->spaceOrderAdjust(i));
            else if(fieldInfo->module()->spaceType(i) == HERMES_H1_SPACE)
                actualSpace = new H1Space<Scalar>(meshes[fieldInfo], bcs[i + m_block->offset(field)], fieldInfo->polynomialOrder() + fieldInfo->module()->spaceOrderAdjust(i));
            else
                assert(0);
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
            Mesh* mesh = new Mesh();
            mesh->copy(oldSpace->get_mesh());

            space.push_back(QSharedPointer<Space<Scalar> >(oldSpace->dup(mesh)));
        }
    }

    return space;
}

template <typename Scalar>
void Solver<Scalar>::solveOneProblem(WeakFormAgros<Scalar> *wf, Scalar *solutionVector, MultiSolutionArray<Scalar> msa, MultiSolutionArray<Scalar>* previousMsa)
{
    Hermes::HermesCommonApi.setParamValue(Hermes::matrixSolverType, Util::problem()->config()->matrixSolver());
    int ndof = Space<Scalar>::get_num_dofs(castConst(desmartize(msa.spaces())));

    // Linear solver
    if (m_block->linearityType() == LinearityType_Linear)
    {
        // Initialize the FE problem.
        DiscreteProblemLinear<Scalar> dp(wf, castConst(desmartize(msa.spaces())));

        LinearSolver<Scalar> linear(&dp);
        try
        {
            linear.solve();

            memcpy(solutionVector, linear.get_sln_vector(), ndof * sizeof(Scalar));

            // todo: this probably should not be done here, since we return solution vector
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
        DiscreteProblem<Scalar> dp(wf, castConst(desmartize(msa.spaces())));

        // Perform Newton's iteration and translate the resulting coefficient vector into a Solution.
        NewtonSolverAgros<Scalar> newton(&dp);
        newton.set_verbose_output(true);
        newton.set_verbose_callback(processSolverOutput);
        newton.set_newton_tol(m_block->nonlinearTolerance());
        newton.set_newton_max_iter(m_block->nonlinearSteps());

        newton.set_max_allowed_residual_norm(1e15);
        try
        {
            if(previousMsa)
            {
                OGProjection<double> ogProjection;
                ogProjection.project_global(castConst(desmartize(msa.spaces())),
                                            desmartize(previousMsa->solutions()),
                                            solutionVector, m_block->projNormTypeVector());

            }
            else
                memset(solutionVector, 0, ndof*sizeof(Scalar));

            newton.solve(solutionVector);

            // todo: this probably should not be done here, since we return solution vector
            Solution<Scalar>::vector_to_solutions(newton.get_sln_vector(), castConst(desmartize(msa.spaces())), desmartize(msa.solutions()));

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
            QString error = QString(e.what());
            Util::log()->printDebug(m_solverID, QObject::tr("Newton's iteration failed: %1").arg(error));
            throw;
        }
    }

    // Picard solver
    if (m_block->linearityType() == LinearityType_Picard)
    {
        assert(0); // redo after refact with solutionVector
        // Initialize the FE problem.
        DiscreteProblemLinear<Scalar> dp(wf, castConst(desmartize(msa.spaces())));

        Hermes::vector<Solution<Scalar>* > slns;
        for (int i = 0; i < msa.spaces().size(); i++)
        {
            QSharedPointer<Hermes::Hermes2D::Space<Scalar> > space = msa.spaces().at(i);
            Hermes::Hermes2D::Space<Scalar> *spc = space.data();
            slns.push_back(new Hermes::Hermes2D::ConstantSolution<double>(spc->get_mesh(), 0));
        }
        PicardSolverAgros<Scalar> picard(&dp);
        picard.setPreviousSolutions(slns);
        picard.set_picard_tol(m_block->nonlinearTolerance());
        picard.set_picard_max_iter(m_block->nonlinearSteps());
        picard.set_verbose_output(true);
        picard.set_verbose_callback(processSolverOutput);

        try
        {
            picard.solve();
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

    MultiSolutionArray<Scalar> multiSolutionArray =
            Util::solutionStore()->multiSolution(BlockSolutionID(m_block, timeStep, adaptivityStep, solutionMode));;

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

    // update timedep values
    foreach (Field* field, m_block->fields())
        field->fieldInfo()->module()->updateTimeFunctions(Util::problem()->actualTime());

    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(desmartize(multiSolutionArray.spaces()), Util::problem()->actualTime());

    BDF2ATable bdf2ATable;
    //cout << "using time order" << min(timeStep, Util::problem()->config()->timeOrder()) << endl;
    bdf2ATable.setOrder(min(timeStep, Util::problem()->config()->timeOrder()));
    bdf2ATable.setPreviousSteps(Util::problem()->timeStepLengths());

    WeakFormAgros<double> wf(m_block);
    wf.set_current_time(Util::problem()->actualTime());
    wf.registerForms(&bdf2ATable);

    int ndof = Space<Scalar>::get_num_dofs(castConst(desmartize(multiSolutionArray.spaces())));
    Scalar coefVec[ndof];

    solveOneProblem(&wf, coefVec, multiSolutionArray, previousTSMultiSolutionArray.size() != 0 ? &previousTSMultiSolutionArray : NULL);
    //Solution<Scalar>::vector_to_solutions(coefVec, castConst(desmartize(multiSolutionArray.spaces())), desmartize(multiSolutionArray.solutions()));

    multiSolutionArray.setTime(Util::problem()->actualTime());

    // output
    BlockSolutionID solutionID;
    solutionID.group = m_block;
    solutionID.timeStep = timeStep;
    solutionID.adaptivityStep = adaptivityStep;

    Util::solutionStore()->addSolution(solutionID, multiSolutionArray);
}

template <typename Scalar>
double Solver<Scalar>::estimateTimeStepLenght(int timeStep)
{
    MultiSolutionArray<Scalar> multiSolutionArray =
            Util::solutionStore()->multiSolution(Util::solutionStore()->lastTimeAndAdaptiveSolution(m_block, SolutionMode_Normal));;

    BDF2BTable bdf2BTable;
    //cout << "using time order" << min(timeStep, Util::problem()->config()->timeOrder()) << endl;
    bdf2BTable.setOrder(min(timeStep, Util::problem()->config()->timeOrder()));
    bdf2BTable.setPreviousSteps(Util::problem()->timeStepLengths());

    WeakFormAgros<double> wf2(m_block);
    wf2.set_current_time(Util::problem()->actualTime());
    wf2.registerForms(&bdf2BTable);

    int ndof = Space<Scalar>::get_num_dofs(castConst(desmartize(multiSolutionArray.spaces())));
    Scalar coefVec2[ndof];
    MultiSolutionArray<Scalar> multiSolutionArray2 = multiSolutionArray.copySpaces();
    multiSolutionArray2.createNewSolutions();

    // solve, for nonlinear solver use solution obtained by BDFA method as an initial vector
    solveOneProblem(&wf2, coefVec2, multiSolutionArray2, timeStep > 0 ? &multiSolutionArray : NULL);

    double error = Global<Scalar>::calc_rel_errors(desmartize(multiSolutionArray.solutions()), desmartize(multiSolutionArray2.solutions()));

    double absError = Global<Scalar>::calc_abs_errors(desmartize(multiSolutionArray.solutions()), desmartize(multiSolutionArray2.solutions()));
    double norm = Global<Scalar>::calc_norms(desmartize(multiSolutionArray.solutions()));

    // todo: if error too big, refuse step and recalculate

    // this guess is based on assymptotic considerations (diploma thesis of Pavel Kus)
    double nextTimeStepLength = pow(Util::problem()->config()->timeMethodTolerance().number() / error,
                             1.0 / (Util::problem()->config()->timeOrder() + 1)) * Util::problem()->actualTimeStepLength();

    Util::log()->printDebug(m_solverID, QString("time adaptivity, rel. error %1, step size %2 -> %3 (%4 %)").
                            arg(absError / norm).
                            arg(Util::problem()->actualTimeStepLength()).
                            arg(nextTimeStepLength).
                            arg(nextTimeStepLength / Util::problem()->actualTimeStepLength()*100.));

   // cout << "error: " << error << "(" << absError << ", " << absError / norm << ") -> step size " << Util::problem()->actualTimeStepLength() << " -> " << nextTimeStepLength << ", change " << pow(Util::problem()->config()->timeMethodTolerance().number()/error, 1./(Util::problem()->config()->timeOrder() + 1)) << endl;
    return nextTimeStepLength;
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
    //cout << "using time order" << min(timeStep, Util::problem()->config()->timeOrder()) << endl;
    bdf2ATable.setOrder(min(timeStep, Util::problem()->config()->timeOrder()));
    bdf2ATable.setPreviousSteps(Util::problem()->timeStepLengths());

    WeakFormAgros<double> wf(m_block);
    wf.set_current_time(Util::problem()->actualTime());
    wf.registerForms(&bdf2ATable);

    msaRef.setSpaces(smartize(*Space<Scalar>::construct_refined_spaces(desmartize(msa.spaces()))));
    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(desmartize(msaRef.spaces()), Util::problem()->actualTime());

    // create solutions
    msaRef.createNewSolutions();

    int ndof = Space<Scalar>::get_num_dofs(castConst(desmartize(msaRef.spaces())));
    Scalar* coeffVec = new Scalar[ndof];

    // solve reference problem
    solveOneProblem(&wf, coeffVec, msaRef, previousTSMultiSolutionArray.size() != 0 ? &previousTSMultiSolutionArray : NULL);

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
