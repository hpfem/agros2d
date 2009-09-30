#include "hermes_electrostatic.h"
#include "hermes_forms.h"

ElectrostaticEdge *electrostaticEdge;
ElectrostaticLabel *electrostaticLabel;
bool electrostaticPlanar;

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

template<typename Real, typename Scalar>
Scalar electrostatic_bilinear_form(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    int marker = e->marker;

    if (electrostaticPlanar)
        return electrostaticLabel[marker].permittivity * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v);
    else
        return electrostaticLabel[marker].permittivity * 2 * M_PI * int_x_grad_u_grad_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar electrostatic_linear_form(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    int marker = e->marker;

    if (electrostaticPlanar)
        return electrostaticLabel[marker].charge_density / EPS0 * int_v<Real, Scalar>(n, wt, v);
    else
        return electrostaticLabel[marker].charge_density / EPS0 * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);
}

SolutionArray *electrostatic_main(SolverThread *solverThread,
                                  const char *fileName,
                                  ElectrostaticEdge *edge,
                                  ElectrostaticLabel *label)
{
    electrostaticEdge = edge;
    electrostaticLabel = label;
    electrostaticPlanar = (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR);
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
    space.set_bc_types(electrostatic_bc_types);
    space.set_bc_values(electrostatic_bc_values);
    space.set_uniform_order(polynomialOrder);
    space.assign_dofs(0);

    // initialize the weak formulation
    WeakForm wf(1);
    wf.add_biform(0, 0, callback(electrostatic_bilinear_form));
    wf.add_liform(0, callback(electrostatic_linear_form));

    // initialize the linear solver
    UmfpackSolver umfpack;
    Solution *sln = new Solution();
    Solution rsln;

    // assemble the stiffness matrix and solve the system
    double error;
    int i;
    int steps = (adaptivityType == ADAPTIVITYTYPE_NONE) ? 1 : adaptivitySteps + 1;
    for (i = 0; i<steps; i++)
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
            solverThread->showMessage(QObject::tr("Solver: relative error is %1 %").arg(error, 0, 'f', 5), false);
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
