#include "hermes_current.h"

static CurrentEdge *currentEdge;
static CurrentLabel *currentLabel;
static bool currentPlanar;

int current_bc_types(int marker)
{
    switch (currentEdge[marker].type)
    {
    case PHYSICFIELDBC_NONE:
        return BC_NONE;
        break;
    case PHYSICFIELDBC_CURRENT_POTENTIAL:
        return BC_ESSENTIAL;
        break;
    case PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW:
        return BC_NATURAL;
        break;
    }
}

scalar current_bc_values(int marker, double x, double y)
{
    return currentEdge[marker].value;
}

scalar current_linear_form_surf(RealFunction* fv, RefMap* rv, EdgePos* ep)
{
    int marker = ep->marker;

    double J = 0.0;

    if (currentEdge[marker].type == PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW)
        J = currentEdge[marker].value;

    if (currentPlanar)
        return J * surf_int_v(fv, rv, ep);
    else
        return J * 2 * M_PI * surf_int_x_v(fv, rv, ep);
}

scalar current_bilinear_form(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
    int marker = rv->get_active_element()->marker;

    if (currentPlanar)
        return currentLabel[marker].conductivity * int_grad_u_grad_v(fu, fv, ru, rv);
    else
        return currentLabel[marker].conductivity * 2 * M_PI * int_x_grad_u_grad_v(fu, fv, ru, rv);
}

scalar current_linear_form(RealFunction* fv, RefMap* rv)
{
    return 0.0;
    /*
    int marker = rv->get_active_element()->marker;

    if (currentIsPlanar)
        return int_v(fv, rv);
    else
        return 2 * M_PI * int_x_v(fv, rv);
    */
}

SolutionArray *current_main(SolverDialog *solverDialog,
                                  const char *fileName,
                                  CurrentEdge *edge,
                                  CurrentLabel *label)
{
    currentEdge = edge;
    currentLabel = label;
    currentPlanar = (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR);

    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    // load the mesh file
    Mesh mesh;
    mesh.load(fileName);
    for (int i = 0; i < Util::scene()->problemInfo().numberOfRefinements; i++)
        mesh.refine_all_elements(0);

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    // initialize the shapeset and the cache
    H1Shapeset shapeset;
    PrecalcShapeset pss(&shapeset);

    // create an H1 space
    H1Space space(&mesh, &shapeset);
    space.set_bc_types(current_bc_types);
    space.set_bc_values(current_bc_values);
    space.set_uniform_order(Util::scene()->problemInfo().polynomialOrder);
    space.assign_dofs(0);

    // initialize the weak formulation
    WeakForm wf(1);
    wf.add_biform(0, 0, current_bilinear_form);
    wf.add_liform(0, current_linear_form);
    // wf.add_biform_surf(0, 0, current_bilinear_form_surf);
    wf.add_liform_surf(0, current_linear_form_surf);

    // initialize the linear solver
    UmfpackSolver umfpack;
    Solution *sln = new Solution();
    Solution rsln;

    // assemble the stiffness matrix and solve the system
    double error;
    int i;
    for (i = 0; i<(Util::scene()->problemInfo().adaptivitySteps+1); i++)
    {
        space.assign_dofs();

        // initialize the linear system
        LinSystem sys(&wf, &umfpack);
        sys.set_spaces(1, &space);
        sys.set_pss(1, &pss);
        sys.assemble();
        sys.solve(1, sln);

        RefSystem rs(&sys);
        rs.assemble();
        rs.solve(1, &rsln);

        // calculate errors and adapt the solution
        if (Util::scene()->problemInfo().adaptivitySteps > 0)
        {
            H1OrthoHP hp(1, &space);
            error = hp.calc_error(sln, &rsln) * 100;

            // emit signal
            solverDialog->doShowMessage(QObject::tr("Relative error: ") + QString::number(error, 'f', 5) + " %", false);

            if (error < Util::scene()->problemInfo().adaptivityTolerance || sys.get_num_dofs() >= NDOF_STOP) break;
            hp.adapt(0.3);
        }
    }

    // output
    space.assign_dofs();

    SolutionArray *solutionArray = new SolutionArray();
    solutionArray->order1 = new Orderizer();
    solutionArray->order1->process_solution(&space);
    solutionArray->sln1 = sln;
    solutionArray->adaptiveError = error;
    solutionArray->adaptiveSteps = i-1;

    return solutionArray;
}
