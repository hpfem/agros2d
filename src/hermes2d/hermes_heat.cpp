#include "hermes_heat.h"

static HeatEdge *heatEdge;
static HeatLabel *heatLabel;
static bool heatPlanar;

int heat_bc_types(int marker)
{
    switch (heatEdge[marker].type)
    {
    case PHYSICFIELDBC_NONE:
        return BC_NONE;
        break;
    case PHYSICFIELDBC_HEAT_TEMPERATURE:
        return BC_ESSENTIAL;
        break;
    case PHYSICFIELDBC_HEAT_HEAT_FLUX:
        return BC_NATURAL;
        break;
    }
}

scalar heat_bc_values(int marker, double x, double y)
{
    switch (heatEdge[marker].type)
    {
    case PHYSICFIELDBC_HEAT_TEMPERATURE:
        {
            return heatEdge[marker].temperature;
        }
        break;
    case PHYSICFIELDBC_HEAT_HEAT_FLUX:
        {
            return heatEdge[marker].heatFlux;
        }
        break;
    }
}

scalar heat_bilinear_form_surf(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv, EdgePos* ep)
{
    int marker = ep->marker;
    double h = 0.0;

    if (heatEdge[marker].type == PHYSICFIELDBC_HEAT_HEAT_FLUX)
    {
        h = heatEdge[marker].h;
    }

    if (heatPlanar)
        return h * surf_int_u_v(fu, fv, ru, rv, ep);
    else
        return h * 2 * M_PI * surf_int_x_u_v(fu, fv, ru, rv, ep);
}

scalar heat_linear_form_surf(RealFunction* fv, RefMap* rv, EdgePos* ep)
{
    int marker = ep->marker;

    if (heatEdge[marker].type == PHYSICFIELDBC_NONE)
        return 0.0;

    double q = 0.0;
    double h = 0.0;
    double Text = 0.0;

    if (heatEdge[marker].type == PHYSICFIELDBC_HEAT_HEAT_FLUX)
    {
        q = heatEdge[marker].heatFlux;
        h = heatEdge[marker].h;
        Text = heatEdge[marker].externalTemperature;
    }

    if (heatPlanar)
        return (q + Text * h) * surf_int_v(fv, rv, ep);
    else
        return (q + Text * h) * 2 * M_PI * surf_int_x_v(fv, rv, ep);
}


scalar heat_bilinear_form(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
    int marker = rv->get_active_element()->marker;

    if (heatPlanar)
        return heatLabel[marker].thermal_conductivity * int_grad_u_grad_v(fu, fv, ru, rv);
    else
        return heatLabel[marker].thermal_conductivity * 2 * M_PI * int_x_grad_u_grad_v(fu, fv, ru, rv);
}

scalar heat_linear_form(RealFunction* fv, RefMap* rv)
{
    int marker = rv->get_active_element()->marker;

    if (heatPlanar)
        // planar
        return heatLabel[marker].volume_heat * int_v(fv, rv);
    else
        // axisymmetric
        return heatLabel[marker].volume_heat * 2 * M_PI * int_x_v(fv, rv);
}

SolutionArray *heat_main(SolverDialog *solverDialog,
                         const char *fileName,
                         HeatEdge *edge, HeatLabel *label)
{
    heatEdge = edge;
    heatLabel = label;
    heatPlanar = (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR);
    int numberOfRefinements = Util::scene()->problemInfo().numberOfRefinements;
    int polynomialOrder = Util::scene()->problemInfo().polynomialOrder;
    AdaptivityType adaptivityType = Util::scene()->problemInfo().adaptivityType;
    int adaptivitySteps = Util::scene()->problemInfo().adaptivitySteps;
    double adaptivityTolerance = Util::scene()->problemInfo().adaptivityTolerance;

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
    space.set_bc_types(heat_bc_types);
    space.set_bc_values(heat_bc_values);
    space.set_uniform_order(polynomialOrder);
    space.assign_dofs();

    // initialize the weak formulation
    WeakForm wf(1);
    wf.add_biform(0, 0, heat_bilinear_form);
    wf.add_liform(0, heat_linear_form);
    wf.add_biform_surf(0, 0, heat_bilinear_form_surf);
    wf.add_liform_surf(0, heat_linear_form_surf);

    // initialize the linear solver
    UmfpackSolver umfpack;
    Solution *sln = new Solution();
    Solution rsln;

    // assemble the stiffness matrix and solve the system
    double error;
    int i;
    int steps = (adaptivityType == ADAPTIVITYTYPE_NONE) ? 1 : adaptivitySteps;
    for (i = 0; i<(steps); i++)
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
        if (adaptivityType != ADAPTIVITYTYPE_NONE)
        {
            H1OrthoHP hp(1, &space);
            error = hp.calc_error(sln, &rsln) * 100;

            // emit signal
            solverDialog->doShowMessage(QObject::tr("Relative error: %1 %").arg(error, 0, 'f', 5), false);

            if (error < adaptivityTolerance || sys.get_num_dofs() >= NDOF_STOP) break;
            hp.adapt(0.3, 0, (int) adaptivityType);
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
