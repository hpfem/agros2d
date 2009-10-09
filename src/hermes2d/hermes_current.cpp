#include "hermes_current.h"
#include "hermes_forms.h"

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

template<typename Real, typename Scalar>
Scalar current_linear_form_surf(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    double J = 0.0;

    if (currentEdge[e->marker].type == PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW)
        J = currentEdge[e->marker].value;

    if (currentPlanar)
        return J * int_v<Real, Scalar>(n, wt, v);
    else
        return J * 2 * M_PI * int_x_v<Real, Scalar>(n, wt, v, e);

}

template<typename Real, typename Scalar>
Scalar current_bilinear_form(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (currentPlanar)
        return currentLabel[e->marker].conductivity * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v);
    else
        return currentLabel[e->marker].conductivity * 2 * M_PI * int_x_grad_u_grad_v<Real, Scalar>(n, wt, u, v, e);
}

template<typename Real, typename Scalar>
Scalar current_linear_form(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return 0.0;
    /*
    if (electrostaticPlanar)
        return electrostaticLabel[marker].charge_density / EPS0 * int_v<Real, Scalar>(n, wt, v);
    else
        return electrostaticLabel[marker].charge_density / EPS0 * 2 * M_PI * int_v<Real, Scalar>(n, wt, v); // FIXME int_x_v
    */
}

SolutionArray *current_main(SolverThread *solverThread,
                                  const char *fileName,
                                  CurrentEdge *edge,
                                  CurrentLabel *label)
{
    currentEdge = edge;
    currentLabel = label;
    currentPlanar = (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR);
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
    space.set_bc_types(current_bc_types);
    space.set_bc_values(current_bc_values);
    space.set_uniform_order(polynomialOrder);
    space.assign_dofs(0);

    // initialize the weak formulation
    WeakForm wf(1);
    wf.add_biform(0, 0, callback(current_bilinear_form));
    wf.add_liform(0, callback(current_linear_form));
    wf.add_liform_surf(0, callback(current_linear_form_surf));

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
        if ((adaptivityType != ADAPTIVITYTYPE_NONE))
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
