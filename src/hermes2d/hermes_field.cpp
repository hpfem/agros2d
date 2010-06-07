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

#include "hermes_field.h"

#include "hermes_general.h"
#include "hermes_electrostatic.h"
#include "hermes_magnetic.h"
#include "hermes_heat.h"
#include "hermes_current.h"
#include "hermes_elasticity.h"
#include "hermes_flow.h"

#include "scene.h"
#include "h2d_reader.h"

// #include <InpMtx.h>

bool isPlanar;
AnalysisType analysisType;
double frequency;
double actualTime;
double timeStep;

HermesField *hermesFieldFactory(PhysicField physicField)
{
    switch (physicField)
    {
    case PhysicField_General:
        return new HermesGeneral();
    case PhysicField_Electrostatic:
        return new HermesElectrostatic();
    case PhysicField_Magnetic:
        return new HermesMagnetic();
    case PhysicField_Heat:
        return new HermesHeat();
    case PhysicField_Current:
        return new HermesCurrent();
    case PhysicField_Elasticity:
        return new HermesElasticity();
    case PhysicField_Flow:
        return new HermesFlow();
    default:
        std::cerr << "Physical field '" + QString::number(physicField).toStdString() + "' is not implemented. hermesObjectFactory()" << endl;
        throw;
        break;
    }
}

Mesh *readMeshFromFile(const QString &fileName)
{
    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    // load the mesh file
    Mesh *mesh = new Mesh();
    H2DReader meshloader;
    meshloader.load(fileName.toStdString().c_str(), mesh);

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return mesh;
}

void writeMeshFromFile(const QString &fileName, Mesh *mesh)
{
    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    H2DReader meshloader;
    meshloader.save(fileName.toStdString().c_str(), mesh);

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

SolutionArray *solutionArray(Solution *sln, Space *space = NULL, double adaptiveError = 0.0, double adaptiveSteps = 0.0, double time = 0.0)
{
    SolutionArray *solution = new SolutionArray();
    solution->order = new Orderizer();
    if (space) solution->order->process_solution(space);
    solution->sln = new Solution();
    if (sln) solution->sln->copy(sln);
    solution->adaptiveError = adaptiveError;
    solution->adaptiveSteps = adaptiveSteps;
    solution->time = time;

    return solution;
}

void solveSystem(LinSystem *sys,
                 Tuple<Solution *> solution)
{
    int numberOfSolution = Util::scene()->problemInfo()->hermes()->numberOfSolution();

    switch (numberOfSolution)
    {
    case 1:
        sys->solve(numberOfSolution, solution.at(0));
        break;
    case 2:
        sys->solve(numberOfSolution, solution.at(0), solution.at(1));
        break;
    case 3:
        sys->solve(numberOfSolution, solution.at(0), solution.at(1), solution.at(2));
        break;
    }
}

QList<SolutionArray *> *solveSolutioArray(ProgressItemSolve *progressItemSolve,
                                          void (*cbSpace)(Tuple<Space *>),
                                          void (*cbWeakForm)(WeakForm *, Tuple<Solution *>))
{
    int polynomialOrder = Util::scene()->problemInfo()->polynomialOrder;
    AdaptivityType adaptivityType = Util::scene()->problemInfo()->adaptivityType;
    int adaptivitySteps = Util::scene()->problemInfo()->adaptivitySteps;
    double adaptivityTolerance = Util::scene()->problemInfo()->adaptivityTolerance;
    int numberOfSolution = Util::scene()->problemInfo()->hermes()->numberOfSolution();
    double timeTotal = Util::scene()->problemInfo()->timeTotal.number;
    double initialCondition = Util::scene()->problemInfo()->initialCondition.number;

    timeStep = Util::scene()->problemInfo()->timeStep.number;
    isPlanar = (Util::scene()->problemInfo()->problemType == ProblemType_Planar);
    analysisType = Util::scene()->problemInfo()->analysisType;
    frequency = Util::scene()->problemInfo()->frequency;

    // solution agros array
    QList<SolutionArray *> *solutionArrayList = new QList<SolutionArray *>();

    // load the mesh file
    Mesh *mesh = readMeshFromFile(tempProblemFileName() + ".mesh");
    // refine mesh
    for (int i = 0; i < Util::scene()->problemInfo()->numberOfRefinements; i++)
        mesh->refine_all_elements(0);

    // initialize the shapeset
    H1Shapeset shapeset;

    // create shapeset cache
    Tuple<PrecalcShapeset *> pss;
    // create an H1 space
    Tuple<Space *> space;
    // create hermes solution array
    Tuple<Solution *> solution;
    // create reference solution
    Tuple<Solution *> solutionReference;

    for (int i = 0; i < numberOfSolution; i++)
    {
        // cache
        pss.push_back(new PrecalcShapeset(&shapeset));

        // space
        space.push_back(new H1Space(mesh, &shapeset));
        // set order by element
        for (int j = 0; j < Util::scene()->labels.count(); j++)
            space.at(i)->set_uniform_order(Util::scene()->labels[j]->polynomialOrder > 0 ? Util::scene()->labels[j]->polynomialOrder : polynomialOrder, j);

        // solution agros array
        solution.push_back(new Solution());

        // reference solution
        if ((adaptivityType != AdaptivityType_None))
            solutionReference.push_back(new Solution());
    }

    // callback space
    cbSpace(space);

    if (analysisType == AnalysisType_Transient)
    {
        for (int i = 0; i < numberOfSolution; i++)
        {
            // constant initial solution
            solution.at(i)->set_const(mesh, initialCondition);
            solutionArrayList->append(solutionArray(solution.at(i)));
        }
    }

    // initialize the weak formulation
    WeakForm wf(numberOfSolution);
    // callback weakform
    cbWeakForm(&wf, solution);

    // initialize the linear solver
    UmfpackSolver umfpack;

    // prepare selector
    RefinementSelectors::Selector *selector = NULL;
    switch (adaptivityType)
    {
    case AdaptivityType_H:
        selector = new RefinementSelectors::HOnlySelector();
        break;
    case AdaptivityType_P:
        selector = new RefinementSelectors::POnlySelector(H2DRS_DEFAULT_ORDER, 1, 1);
        break;
    case AdaptivityType_HP:
        selector = new RefinementSelectors::H1ProjBasedSelector(RefinementSelectors::H2D_HP_ANISO,
                                                                Util::config()->convExp,
                                                                H2DRS_DEFAULT_ORDER,
                                                                &shapeset);
        break;
    }

    // initialize the linear system
    LinSystem sys(&wf, &umfpack);
    switch (numberOfSolution)
    {
    case 1:
        {
            sys.set_spaces(numberOfSolution, space.at(0));
            sys.set_pss(numberOfSolution, pss.at(0));
            break;
        }
    case 2:
        {
            sys.set_spaces(numberOfSolution, space.at(0), space.at(1));
            sys.set_pss(numberOfSolution, pss.at(0), pss.at(1));
            break;
        }
    case 3:
        {
            sys.set_spaces(numberOfSolution, space.at(0), space.at(1), space.at(2));
            sys.set_pss(numberOfSolution, pss.at(0),  pss.at(1), pss.at(2));
            break;
        }
    }

    // assemble the stiffness matrix and solve the system
    double error;

    // set actual time
    actualTime = 0;

    // error marker
    bool isError = false;

    // solution
    int actualAdaptivitySteps = (adaptivityType == AdaptivityType_None) ? 1 : adaptivitySteps;
    for (int i = 0; i<actualAdaptivitySteps; i++)
    {
        // assign dofs
        int ndofs = 0;
        for (int j = 0; j < numberOfSolution; j++)
            ndofs += space.at(j)->assign_dofs(ndofs);

        sys.assemble();
        if (sys.get_num_dofs() == 0)
        {
            progressItemSolve->emitMessage(QObject::tr("Solver: DOF is zero."), true);
            isError = true;
            break;
        }
        solveSystem(&sys, solution);

        // calculate errors and adapt the solution
        if (adaptivityType != AdaptivityType_None)
        {
            RefSystem rs(&sys);
            rs.assemble();

            // adaptivity
            H1Adapt hp(space);
            switch (numberOfSolution)
            {
            case 1:
                {
                    rs.solve(numberOfSolution, solutionReference.at(0));
                }
                break;
            case 2:
                {
                    rs.solve(numberOfSolution, solutionReference.at(0), solutionReference.at(1));
                }
                break;
            case 3:
                {
                    rs.solve(numberOfSolution, solutionReference.at(0), solutionReference.at(1), solutionReference.at(2));
                }
                break;
            }

            hp.set_solutions(solution, solutionReference);
            error = hp.calc_error(H2D_TOTAL_ERROR_REL | H2D_ELEMENT_ERROR_ABS) * 100;

            // emit signal
            progressItemSolve->emitMessage(QObject::tr("relative error: %1 %").
                                           arg(error, 0, 'f', 5), false, 1);
            if (progressItemSolve->isCanceled())
            {
                isError = true;
                break;
            }

            if (error < adaptivityTolerance || sys.get_num_dofs() >= NDOF_STOP)
            {
                break;
            }
            if (i != actualAdaptivitySteps-1) hp.adapt(selector,
                                                       Util::config()->threshold,
                                                       Util::config()->strategy,
                                                       Util::config()->meshRegularity);
        }
    }

    // delete selector
    if (selector) delete selector;


    // timesteps
    if (!isError)
    {
        int timesteps = (analysisType == AnalysisType_Transient) ? floor(timeTotal/timeStep) : 1;
        for (int n = 0; n<timesteps; n++)
        {
            // set actual time
            actualTime = (n+1)*timeStep;

            if (timesteps > 1)
            {
                // transient
                sys.assemble(true);
                solveSystem(&sys, solution);
            }
            else if (n > 0)
            {
                int ndofs = 0;
                for (int i = 0; i < numberOfSolution; i++)
                    ndofs += space.at(i)->assign_dofs(ndofs);

                sys.assemble();
            }

            // output
            for (int i = 0; i < numberOfSolution; i++)
            {
                solutionArrayList->append(solutionArray(solution.at(i), space.at(i), error, actualAdaptivitySteps, (n+1)*timeStep));
            }

            if (analysisType == AnalysisType_Transient)
                progressItemSolve->emitMessage(QObject::tr("time step: %1/%2").
                                               arg(n+1).
                                               arg(timesteps), false, n+2);
            if (progressItemSolve->isCanceled())
            {
                isError = true;
                break;
            }
        }
    }

    // delete mesh
    delete mesh;

    // delete pss
    for (int i = 0; i < pss.size(); i++)
        delete pss.at(i);
    pss.clear();

    // delete space
    for (int i = 0; i < space.size(); i++)
        delete space.at(i);
    space.clear();

    // delete last solution
    for (int i = 0; i < solution.size(); i++)
        delete solution.at(i);
    solution.clear();

    // delete reference solution
    for (int i = 0; i < solutionReference.size(); i++)
        delete solutionReference.at(i);
    solutionReference.clear();

    if (isError)
    {
        for (int i = 0; i < solutionArrayList->count(); i++)
            delete solutionArrayList->at(i);
        solutionArrayList->clear();
    }

    return solutionArrayList;
}

// *********************************************************************************************************************************************

ViewScalarFilter::ViewScalarFilter(MeshFunction *sln1, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
    : Filter(sln1)
{
    m_physicFieldVariable = physicFieldVariable;
    m_physicFieldVariableComp = physicFieldVariableComp;
}

ViewScalarFilter::ViewScalarFilter(MeshFunction *sln1, MeshFunction *sln2, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
    : Filter(sln1, sln2)
{
    m_physicFieldVariable = physicFieldVariable;
    m_physicFieldVariableComp = physicFieldVariableComp;
}

ViewScalarFilter::ViewScalarFilter(MeshFunction *sln1, MeshFunction *sln2, MeshFunction *sln3, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
    : Filter(sln1, sln2, sln3)
{
    m_physicFieldVariable = physicFieldVariable;
    m_physicFieldVariableComp = physicFieldVariableComp;
}

double ViewScalarFilter::get_pt_value(double x, double y, int item)
{
    error("Not implemented");
}

void ViewScalarFilter::precalculate(int order, int mask)
{
    Quad2D* quad = quads[cur_quad];
    int np = quad->get_num_points(order);
    node = new_node(H2D_FN_DEFAULT, np);

    if (sln[0])
    {
        sln[0]->set_quad_order(order, H2D_FN_VAL | H2D_FN_DX | H2D_FN_DY);
        sln[0]->get_dx_dy_values(dudx1, dudy1);
        value1 = sln[0]->get_fn_values();
    }

    if (num >= 2 && sln[1])
    {
        sln[1]->set_quad_order(order, H2D_FN_VAL | H2D_FN_DX | H2D_FN_DY);
        sln[1]->get_dx_dy_values(dudx2, dudy2);
        value2 = sln[1]->get_fn_values();
    }

    if (num >= 3 && sln[2])
    {
        sln[2]->set_quad_order(order, H2D_FN_VAL | H2D_FN_DX | H2D_FN_DY);
        sln[2]->get_dx_dy_values(dudx3, dudy3);
        value3 = sln[2]->get_fn_values();
    }

    update_refmap();

    x = refmap->get_phys_x(order);
    y = refmap->get_phys_y(order);
    Element *e = refmap->get_active_element();

    labelMarker = Util::scene()->labels[e->marker]->marker;

    for (int i = 0; i < np; i++)
    {
        calculateVariable(i);
    }

    replace_cur_node(node);
}
