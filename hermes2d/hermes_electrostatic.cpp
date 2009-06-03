#include "hermes_electrostatic.h"

int electrostatic_bc_types(int marker)
{
    switch (electrostaticEdge[marker].type)
    {
    case PHYSICFIELDBC_NONE:
        return BC_NONE;
        break;
    case PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL:
        return BC_ESSENTIAL;
        break;
    case PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE:
        return BC_NATURAL;
        break;
    }
}

scalar electrostatic_bc_values(int marker, double x, double y)
{
    return electrostaticEdge[marker].value;
}

scalar electrostatic_bilinear_form(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
    int marker = rv->get_active_element()->marker;

    if (electrostaticIsPlanar)
        return electrostaticLabel[marker].permittivity * int_grad_u_grad_v(fu, fv, ru, rv);
    else
        return electrostaticLabel[marker].permittivity * 2 * M_PI * int_x_grad_u_grad_v(fu, fv, ru, rv);
}

scalar electrostatic_linear_form(RealFunction* fv, RefMap* rv)
{
    int marker = rv->get_active_element()->marker;

    if (electrostaticIsPlanar)
        return electrostaticLabel[marker].charge_density / EPS0 * int_v(fv, rv);
    else
        return electrostaticLabel[marker].charge_density / EPS0 * 2 * M_PI * int_x_v(fv, rv);
}

SolutionArray electrostatic_main(const char *fileName, ElectrostaticEdge *edge, ElectrostaticLabel *label, int numberOfRefinements, int polynomialOrder, int adaptivitySteps, bool isPlanar)
{
    electrostaticEdge = edge;
    electrostaticLabel = label;
    electrostaticIsPlanar = isPlanar;

    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    // load the mesh file
    Mesh mesh;
    mesh.load(fileName);
    for (int i = 0; i < numberOfRefinements; i++)
        mesh.refine_all_elements(0);

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    // initialize the shapeset and the cache
    H1Shapeset shapeset;
    PrecalcShapeset pss(&shapeset);

    // create an H1 space
    H1Space space(&mesh, &shapeset);
    space.set_bc_types(electrostatic_bc_types);
    space.set_bc_values(electrostatic_bc_values);
    space.set_uniform_order(polynomialOrder);
    space.assign_dofs(0);

    // initialize the weak formulation
    WeakForm wf(1);
    wf.add_biform(0, 0, electrostatic_bilinear_form);
    wf.add_liform(0, electrostatic_linear_form);

    // initialize the linear solver
    UmfpackSolver umfpack;
    Solution *sln = new Solution();
    Solution rsln;

    // assemble the stiffness matrix and solve the system
    double error;
    int i;
    for (i = 0; i<(adaptivitySteps+1); i++)
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
        if (adaptivitySteps > 0)
        {
            H1OrthoHP hp(1, &space);
            error = hp.calc_error(sln, &rsln) * 100;

            if (error < 0.1 || sys.get_num_dofs() >= NDOF_STOP) break;
            hp.adapt(THRESHOLD, STRATEGY, H_ONLY);
        }
    }

    // order
    space.assign_dofs();
    Orderizer *ord = new Orderizer();
    ord->process_solution(&space);

    return SolutionArray(sln, ord);
}
