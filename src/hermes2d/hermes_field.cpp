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
// #include "hermes_flow.h"
#include "hermes_rf.h"
#include "hermes_acoustic.h"
#include "progressdialog.h"

#include "mesh/h2d_reader.h"

double actualTime;

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
        // return new HermesFlow();
    case PhysicField_RF:
        return new HermesRF();
    case PhysicField_Acoustic:
        return new HermesAcoustic();
    default:
        std::cerr << "Physical field '" + QString::number(physicField).toStdString() + "' is not implemented. hermesObjectFactory()" << endl;
        throw;
        break;
    }
}

void readMeshDirtyFix()
{
    // fix precalulating matrices for mapping of curved elements

    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    std::ostringstream os;
    os << "vertices =" << std::endl <<
          "{" << std::endl <<
          "{ 0, 0 }," << std::endl <<
          "{ 1, 0 }," << std::endl <<
          "{ 0, 1 }" << std::endl <<
          "}" << std::endl <<
          "elements =" << std::endl <<
          "{" << std::endl <<
          "{ 0, 1, 2, 0 }" << std::endl <<
          "}" << std::endl <<
          "boundaries =" << std::endl <<
          "{" << std::endl <<
          "{ 0, 1, 1 }," << std::endl <<
          "{ 1, 2, 1 }," << std::endl <<
          "{ 2, 0, 1 }" << std::endl <<
          "}" << std::endl <<
          "curves =" << std::endl <<
          "{" << std::endl <<
          "{ 1, 2, 90 }" << std::endl <<
          "}";

    Mesh mesh;
    H2DReader meshloader;
    meshloader.load_str(os.str().c_str(), &mesh);

    // set system locale
    setlocale(LC_NUMERIC, plocale);
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

void refineMesh(Mesh *mesh, bool refineGlobal, bool refineTowardsEdge)
{
    // refine mesh - global
    if (refineGlobal)
        for (int i = 0; i < Util::scene()->problemInfo()->numberOfRefinements; i++)
            mesh->refine_all_elements(0);

    // refine mesh - boundary
    if (refineTowardsEdge)
        for (int i = 0; i < Util::scene()->edges.count(); i++)
            if (Util::scene()->edges[i]->refineTowardsEdge > 0)
                mesh->refine_towards_boundary(QString::number(((Util::scene()->edges[i]->boundary->type != PhysicFieldBC_None) ? i + 1 : -i)).toStdString(),
                                              Util::scene()->edges[i]->refineTowardsEdge);
}

// return geom type
GeomType convertProblemType(ProblemType problemType)
{
    return (problemType == ProblemType_Planar ? HERMES_PLANAR : HERMES_AXISYM_Y);
}

QList<SolutionArray *> solveSolutioArray(ProgressItemSolve *progressItemSolve,
                                         Hermes::vector<EssentialBCs> bcs,
                                         WeakFormAgros *wf)
{
    SolutionAgros solutionAgros(progressItemSolve, wf);

    QList<SolutionArray *> solutionArrayList = solutionAgros.solveSolutioArray(bcs);
    return solutionArrayList;
}

// *********************************************************************************************************************************************

SolutionAgros::SolutionAgros(ProgressItemSolve *progressItemSolve, WeakFormAgros *wf)
{
    analysisType = Util::scene()->problemInfo()->analysisType;
    polynomialOrder = Util::scene()->problemInfo()->polynomialOrder;
    adaptivityType = Util::scene()->problemInfo()->adaptivityType;
    adaptivitySteps = Util::scene()->problemInfo()->adaptivitySteps;
    adaptivityTolerance = Util::scene()->problemInfo()->adaptivityTolerance;
    adaptivityMaxDOFs = Util::scene()->problemInfo()->adaptivityMaxDOFs;
    numberOfSolution = Util::scene()->problemInfo()->hermes()->numberOfSolution();
    timeTotal = Util::scene()->problemInfo()->timeTotal.number;
    timeStep = Util::scene()->problemInfo()->timeStep.number;
    initialCondition = Util::scene()->problemInfo()->initialCondition.number;

    linearityType = Util::scene()->problemInfo()->linearityType;
    linearityNonlinearTolerance = Util::scene()->problemInfo()->linearityNonlinearTolerance;
    linearityNonlinearSteps = Util::scene()->problemInfo()->linearityNonlinearSteps;

    matrixSolver = Util::scene()->problemInfo()->matrixSolver;

    m_progressItemSolve = progressItemSolve;
    m_wf = wf;
}

QList<SolutionArray *> SolutionAgros::solveSolutioArray(Hermes::vector<EssentialBCs> bcs)
{
    QTime time;

    // solution agros array
    QList<SolutionArray *> solutionArrayList;

    // load the mesh file
    mesh = readMeshFromFile(tempProblemFileName() + ".mesh");
    refineMesh(mesh, true, true);

    // create an H1 space
    Hermes::vector<Space *> space;
    // create hermes solution array
    Hermes::vector<Solution *> solution;
    // create reference solution
    Hermes::vector<Solution *> solutionReference;

    // projection norms
    Hermes::vector<ProjNormType> projNormType;

    // prepare selector
    Hermes::vector<RefinementSelectors::Selector *> selector;

    // error marker
    bool isError = false;

    RefinementSelectors::Selector *select = NULL;
    switch (adaptivityType)
    {
    case AdaptivityType_H:
        select = new RefinementSelectors::HOnlySelector();
        break;
    case AdaptivityType_P:
        select = new RefinementSelectors::H1ProjBasedSelector(RefinementSelectors::H2D_P_ANISO,
                                                              Util::config()->convExp,
                                                              H2DRS_DEFAULT_ORDER);
        break;
    case AdaptivityType_HP:
        select = new RefinementSelectors::H1ProjBasedSelector(RefinementSelectors::H2D_HP_ANISO,
                                                              Util::config()->convExp,
                                                              H2DRS_DEFAULT_ORDER);
        break;
    }

    for (int i = 0; i < numberOfSolution; i++)
    {
        space.push_back(new H1Space(mesh, &bcs[i], polynomialOrder));

        // set order by element
        for (int j = 0; j < Util::scene()->labels.count(); j++)
            if (Util::scene()->labels[j]->material != Util::scene()->materials[0])
                space.at(i)->set_uniform_order(Util::scene()->labels[j]->polynomialOrder > 0 ? Util::scene()->labels[j]->polynomialOrder : polynomialOrder,
                                               QString::number(j).toStdString());

        // solution agros array
        solution.push_back(new Solution());

        if (adaptivityType != AdaptivityType_None)
        {
            // add norm
            projNormType.push_back(Util::config()->projNormType);
            // add refinement selector
            selector.push_back(select);
        }
    }

    // check for DOFs
    if (Space::get_num_dofs(space) == 0)
    {
        m_progressItemSolve->emitMessage(QObject::tr("DOF is zero"), true);
    }
    else
    {
        for (int i = 0; i < numberOfSolution; i++)
        {
            // transient
            if (analysisType == AnalysisType_Transient)
            {
                // constant initial solution
                solution.at(i)->set_const(mesh, initialCondition);
                solutionArrayList.append(solutionArray(solution.at(i)));
            }

            // nonlinear
            if ((linearityType != LinearityType_Linear) && (analysisType != AnalysisType_Transient))
            {
                solution.at(i)->set_const(mesh, 0.0);
            }
        }

        actualTime = 0.0;

        // update time function
        Util::scene()->problemInfo()->hermes()->updateTimeFunctions(actualTime);

        m_wf->set_current_time(actualTime);
        m_wf->solution = solution;
        m_wf->registerForms();

        // emit message
        if (adaptivityType != AdaptivityType_None)
            m_progressItemSolve->emitMessage(QObject::tr("Adaptivity type: %1").arg(adaptivityTypeString(adaptivityType)), false);

        double error = 0.0;

        // solution
        int maxAdaptivitySteps = (adaptivityType == AdaptivityType_None) ? 1 : adaptivitySteps;
        int actualAdaptivitySteps = -1;
        for (int i = 0; i<maxAdaptivitySteps; i++)
        {
            // set up the solver, matrix, and rhs according to the solver selection.
            SparseMatrix *matrix = create_matrix(matrixSolver);
            Vector *rhs = create_vector(matrixSolver);
            Solver *solver = create_linear_solver(matrixSolver, matrix, rhs);

            if (adaptivityType == AdaptivityType_None)
            {
                if (analysisType != AnalysisType_Transient)
                    solve(space, solution, solver, matrix, rhs);
            }
            else
            {
                // construct globally refined reference mesh and setup reference space.
                Hermes::vector<Space *> spaceReference = *Space::construct_refined_spaces(space);

                // assemble reference problem.
                solve(spaceReference, solution, solver, matrix, rhs);

                // copy solution
                for (int j = 0; j < numberOfSolution; j++)
                {
                    solutionReference.push_back(new Solution());
                    solutionReference.at(j)->copy(solution.at(j));
                }

                if (!isError)
                {
                    // project the fine mesh solution onto the coarse mesh.
                    OGProjection::project_global(space, solutionReference, solution, matrixSolver);

                    // Calculate element errors and total error estimate.
                    Adapt adaptivity(space, projNormType);

                    // Calculate error estimate for each solution component and the total error estimate.
                    Hermes::vector<double> err_est_rel;
                    error = adaptivity.calc_err_est(solution,
                                                    solutionReference,
                                                    &err_est_rel) * 100;

                    // emit signal
                    m_progressItemSolve->emitMessage(QObject::tr("Adaptivity rel. error (step: %2/%3, DOFs: %4): %1% s").
                                                     arg(error, 0, 'f', 3).
                                                     arg(i + 1).
                                                     arg(maxAdaptivitySteps).
                                                     arg(Space::get_num_dofs(space)), false, 1);
                    // add error to the list
                    m_progressItemSolve->addAdaptivityError(error, Space::get_num_dofs(space));

                    if (error < adaptivityTolerance || Space::get_num_dofs(space) >= adaptivityMaxDOFs)
                    {
                        break;
                    }
                    if (i != maxAdaptivitySteps-1) adaptivity.adapt(selector,
                                                                    Util::config()->threshold,
                                                                    Util::config()->strategy,
                                                                    Util::config()->meshRegularity);
                    actualAdaptivitySteps = i+1;
                }

                if (m_progressItemSolve->isCanceled())
                {
                    isError = true;
                    break;
                }

                // delete reference space
                for (int i = 0; i < spaceReference.size(); i++)
                {
                    delete spaceReference.at(i)->get_mesh();
                    delete spaceReference.at(i);
                }
                spaceReference.clear();

                // delete reference solution
                for (int i = 0; i < solutionReference.size(); i++)
                    delete solutionReference.at(i);
                solutionReference.clear();
            }

            // clean up.
            delete solver;
            delete matrix;
            delete rhs;
        }

        // delete selector
        if (select) delete select;
        selector.clear();

        // timesteps
        if (!isError)
        {
            SparseMatrix *matrix = NULL;
            Vector *rhs = NULL;
            Solver *solver = NULL;

            // allocate dp for transient solution
            DiscreteProblem *dpTran = NULL;
            if (analysisType == AnalysisType_Transient)
            {
                // set up the solver, matrix, and rhs according to the solver selection.
                matrix = create_matrix(matrixSolver);
                rhs = create_vector(matrixSolver);
                solver = create_linear_solver(matrixSolver, matrix, rhs);
                // solver->set_factorization_scheme(HERMES_REUSE_FACTORIZATION_COMPLETELY);

                dpTran = new DiscreteProblem(m_wf, space, true);
            }

            int timesteps = (analysisType == AnalysisType_Transient) ? floor(timeTotal/timeStep) : 1;
            for (int n = 0; n<timesteps; n++)
            {
                // set actual time
                actualTime = (n+1)*timeStep;

                // update essential bc values
                Space::update_essential_bc_values(space, actualTime);
                // update timedep values
                Util::scene()->problemInfo()->hermes()->updateTimeFunctions(actualTime);

                m_wf->set_current_time(actualTime);
                m_wf->registerForms();

                // transient
                if ((timesteps > 1) && (linearityType == LinearityType_Linear))
                    isError = !solveLinear(dpTran, space, solution,
                                           solver, matrix, rhs);
                if ((timesteps > 1) && (linearityType != LinearityType_Linear))
                    isError = !solve(space, solution,
                                     solver, matrix, rhs);

                // output
                for (int i = 0; i < numberOfSolution; i++)
                {
                    solutionArrayList.append(solutionArray(solution.at(i), space.at(i), error, actualAdaptivitySteps, (n+1)*timeStep));
                }

                if (analysisType == AnalysisType_Transient)
                    m_progressItemSolve->emitMessage(QObject::tr("Transient time step (%1/%2): %3 s").
                                                     arg(n+1).
                                                     arg(timesteps).
                                                     arg(actualTime, 0, 'e', 2), false, n+2);
                if (m_progressItemSolve->isCanceled())
                {
                    isError = true;
                    break;
                }
            }

            // clean up
            if (solver) delete solver;
            if (matrix) delete matrix;
            if (rhs) delete rhs;

            if (dpTran) delete dpTran;
        }
    }
    // delete mesh
    delete mesh;

    // delete space
    for (unsigned int i = 0; i < space.size(); i++)
    {
        // delete space.at(i)->get_mesh();
        delete space.at(i);
    }
    space.clear();

    // delete last solution
    for (unsigned int i = 0; i < solution.size(); i++)
        delete solution.at(i);
    solution.clear();

    if (isError)
    {
        for (int i = 0; i < solutionArrayList.count(); i++)
            delete solutionArrayList.at(i);
        solutionArrayList.clear();
    }
    return solutionArrayList;
}

bool SolutionAgros::solveLinear(DiscreteProblem *dp,
                                Hermes::vector<Space *> space,
                                Hermes::vector<Solution *> solution,
                                Solver *solver, SparseMatrix *matrix, Vector *rhs)
{
    QTime time;
    time.start();
    dp->assemble(matrix, rhs);
    qDebug() << "assemble: " << time.elapsed();

    if(solver->solve())
    {
        Solution::vector_to_solutions(solver->get_solution(), space, solution);
        return true;
    }
    else
    {
        m_progressItemSolve->emitMessage(QObject::tr("Matrix solver failed."), true, 1);
        return false;
    }
}

bool SolutionAgros::solve(Hermes::vector<Space *> space,
                          Hermes::vector<Solution *> solution,
                          Solver *solver, SparseMatrix *matrix, Vector *rhs)
{
    bool isError = false;
    if (linearityType == LinearityType_Linear)
    {
        DiscreteProblem dpLin(m_wf, space, true);

        isError = !solveLinear(&dpLin, space, solution,
                               solver, matrix, rhs);

        return !isError;
    }

    if (linearityType == LinearityType_Picard)
    {
        /*
        DiscreteProblem dpNonlinPicard(m_wf, space, true);

        // create Picard solution
        Hermes::vector<Solution *> solutionPicard;
        for (int i = 0; i < numberOfSolution; i++)
            solutionPicard.push_back(new Solution());

        // perform the Picard's iteration
        for (int i = 0; i < linearityNonlinearSteps; i++)
        {
            isError = !solveLinear(&dpNonlinPicard, space, solutionPicard,
                                   solver, matrix, rhs, false);

            ProjNormType projNormTypeTMP = HERMES_H1_NORM;
            // ProjNormType projNormTypeTMP = HERMES_L2_NORM;

            // calc error
            double *val_sol, *val_solpic, *val_diff;
            double error = 0.0;
            for (int i = 0; i < numberOfSolution; i++)
            {
                error += calc_abs_error(solution.at(i), solutionPicard.at(i), projNormTypeTMP) * 100.0
                        / calc_norm(&sln_new, HERMES_H1_NORM) * 100;

                val_sol = solution.at(i)->get_fn_values();
                val_solpic = solutionPicard.at(i)->get_fn_values();

                val_diff = new double[Space::get_num_dofs(space)];
                for (int j = 0; j < Space::get_num_dofs(space); j++)
                    val_diff[j] = val_solpic[j]; //  + 0.5 * (val_sol[j] - val_solpic[j]);
            }

            // emit signal
            m_progressItemSolve->emitMessage(QObject::tr("Picards method rel. error (%2/%3): %1%").
                                             arg(error, 0, 'f', 5).
                                             arg(i + 1).
                                             arg(linearityNonlinearSteps), false, 1);

            // add error to the list
            m_progressItemSolve->addNonlinearityError(error);

            if (error < linearityNonlinearTolerance)
            {
                // FIXME - clean up
                break;
            }

            // copy solution
            for (int i = 0; i < numberOfSolution; i++)
            {
                // if (error > 100.0)
                //     solution.at(i)->multiply(0.5);
                // else
                solution.at(i)->copy(solutionPicard.at(i));
            }
            // Solution::vector_to_solutions(val_solpic, space, solution);
            // Solution::vector_to_solutions(solutionPicard., space, solution);
        }

        for (int i = 0; i < solutionPicard.size(); i++)
            delete solutionPicard.at(i);
        solutionPicard.clear();

        return !isError;
        */
    }

    if (linearityType == LinearityType_Newton)
    {
        /*
        // project the initial condition on the FE space to obtain initial
        // coefficient vector for the Newton's method.
        info("Projecting to obtain initial vector for the Newton's method.");
        double *coeff_vec = new double[Space::get_num_dofs(space)];
        OGProjection::project_global(space, solution.at(0), coeff_vec,
                                     Util::scene()->problemInfo()->matrixSolver);

        DiscreteProblem dpNonlinNewton(wf, space, false);

        // The Newton's loop.
        double damping_coeff = 1.0;

        // perform the Picard's iteration
        for (int i = 0; i < linearityNonlinearSteps; i++)
        {
            // assemble the Jacobian matrix and residual vector.
            dpNonlinNewton.assemble(coeff_vec, matrix, rhs, false);

            // Multiply the residual vector with -1 since the matrix
            // equation reads J(Y^n) \deltaY^{n+1} = -F(Y^n).
            rhs->change_sign();

            // Calculate the l2-norm of residual vector
            double error = get_l2_norm(rhs);

            // emit signal
            m_progressItemSolve->emitMessage(QObject::tr("Newton’s method rel. error (%2/%3): %1")
                                             .arg(error, 0, 'f', 5)
                                             .arg(i)
                                             .arg(linearityNonlinearSteps), false, 1);

            // add error to the list
            m_progressItemSolve->addNonlinearityError(error);

            // if residual norm is within tolerance, or the maximum number of iteration has been reached, then quit.
            if (error < linearityNonlinearTolerance)
                break;

            // Solve the linear system.
            isError = solver->solve();
            if (!isError)
                error("Matrix solver failed.\n");

            // add \deltaY^{n+1} to Y^n.
            for (int j = 0; j < Space::get_num_dofs(space); j++)
                coeff_vec[j] += damping_coeff * solver->get_solution()[j];
        }

        Solution::vector_to_solutions(coeff_vec, space, solution);

        delete [] coeff_vec;
        */
    }
}

SolutionArray *SolutionAgros::solutionArray(Solution *sln, Space *space, double adaptiveError, double adaptiveSteps, double time)
{
    SolutionArray *solution = new SolutionArray();
    solution->order = new Orderizer();
    if (space) solution->order->process_space(space);
    solution->sln = new Solution();
    if (sln) solution->sln->copy(sln);
    solution->adaptiveError = adaptiveError;
    solution->adaptiveSteps = adaptiveSteps;
    solution->time = time;

    return solution;
}

// *********************************************************************************************************************************************

ViewScalarFilter::ViewScalarFilter(Hermes::vector<MeshFunction *> sln, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
    : Filter(sln)
{
    m_physicFieldVariable = physicFieldVariable;
    m_physicFieldVariableComp = physicFieldVariableComp;
}

double ViewScalarFilter::get_pt_value(double x, double y, int item)
{
    return 0.0;
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

    material = Util::scene()->labels[Util::scene()->sceneSolution()->agrosMaterialMarker(e->marker)]->material;

    for (int i = 0; i < np; i++)
    {
        calculateVariable(i);
    }

    if (nodes->present(order))
    {
        assert(nodes->get(order) == cur_node);
        ::free(nodes->get(order));
    }
    nodes->add(node, order);
    cur_node = node;
}
