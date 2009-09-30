#include "hermes_heat.h"
#include "hermes_forms.h"

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

template<typename Real, typename Scalar>
Scalar heat_bilinear_form_surf(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    double h = 0.0;

    if (heatEdge[e->marker].type == PHYSICFIELDBC_HEAT_HEAT_FLUX)
    {
        h = heatEdge[e->marker].h;
    }

    if (heatPlanar)
        return h * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        return h * 2 * M_PI * int_x_u_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar heat_linear_form_surf(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (heatEdge[e->marker].type == PHYSICFIELDBC_NONE)
        return 0.0;

    double q = 0.0;
    double h = 0.0;
    double Text = 0.0;

    if (heatEdge[e->marker].type == PHYSICFIELDBC_HEAT_HEAT_FLUX)
    {
        q = heatEdge[e->marker].heatFlux;
        h = heatEdge[e->marker].h;
        Text = heatEdge[e->marker].externalTemperature;
    }

    if (heatPlanar)
        return (q + Text * h) * int_v<Real, Scalar>(n, wt, v); // FIXME surf_int_v(fv, rv, ep);
    else
        return (q + Text * h) * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);
}

template<typename Real, typename Scalar>
Scalar heat_bilinear_form(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (heatPlanar)
        return heatLabel[e->marker].thermal_conductivity * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v);
    else
        return heatLabel[e->marker].thermal_conductivity * 2 * M_PI * int_x_grad_u_grad_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar heat_linear_form(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (heatPlanar)
        return heatLabel[e->marker].volume_heat * int_v<Real, Scalar>(n, wt, v);
    else
        return heatLabel[e->marker].volume_heat * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);
}

SolutionArray *heat_main(SolverThread *solverThread,
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
    wf.add_biform(0, 0, callback(heat_bilinear_form));
    wf.add_liform(0, callback(heat_linear_form));
    wf.add_biform_surf(0, 0, callback(heat_bilinear_form_surf));
    wf.add_liform_surf(0, callback(heat_linear_form_surf));

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
            solverThread->showMessage(QObject::tr("Solver: relative error: %1 %").arg(error, 0, 'f', 5), false);
            if (solverThread->isCanceled()) return NULL;

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
