#include "hermes_harmonicmagnetic.h"
#include "hermes_forms.h"

static HarmonicMagneticEdge *harmonicmagneticEdge;
static HarmonicMagneticLabel *harmonicmagneticLabel;
static bool harmonicmagneticPlanar;
static double frequency;

int harmonicmagnetic_bc_types(int marker)
{
    switch (harmonicmagneticEdge[marker].type)
    {
    case PHYSICFIELDBC_NONE:
        return BC_NONE;
        break;
    case PHYSICFIELDBC_HARMONIC_MAGNETIC_VECTOR_POTENTIAL:
        return BC_ESSENTIAL;
        break;
    case PHYSICFIELDBC_HARMONIC_MAGNETIC_SURFACE_CURRENT:
        return BC_NATURAL;
        break;
    }
}

scalar harmonicmagnetic_bc_values_real(int marker, double x, double y)
{
    return harmonicmagneticEdge[marker].value;
}

scalar harmonicmagnetic_bc_values_imag(int marker, double x, double y)
{
    return harmonicmagneticEdge[marker].value;
}

template<typename Real, typename Scalar>
Scalar harmonicmagnetic_bilinear_form_real_real(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (harmonicmagneticPlanar)
        return 1.0 / (harmonicmagneticLabel[e->marker].permeability * MU0) * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v);
    else
        return 1.0 / (harmonicmagneticLabel[e->marker].permeability * MU0) * (int_u_dvdx_over_x<Real, Scalar>(n, wt, u, v, e) + int_grad_u_grad_v<Real, Scalar>(n, wt, u, v));
}

template<typename Real, typename Scalar>
Scalar harmonicmagnetic_bilinear_form_real_imag(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (harmonicmagneticPlanar)
        return - 2 * M_PI * frequency * harmonicmagneticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        return - 2 * M_PI * frequency * harmonicmagneticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar harmonicmagnetic_bilinear_form_imag_real(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (harmonicmagneticPlanar)
        return + 2 * M_PI * frequency * harmonicmagneticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, u, v);
    else
        return + 2 * M_PI * frequency * harmonicmagneticLabel[e->marker].conductivity * int_u_v<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar harmonicmagnetic_bilinear_form_imag_imag(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (harmonicmagneticPlanar)
        return 1.0 / (harmonicmagneticLabel[e->marker].permeability * MU0) * int_grad_u_grad_v<Real, Scalar>(n, wt, u, v);
    else
        return 1.0 / (harmonicmagneticLabel[e->marker].permeability * MU0) * (int_u_dvdx_over_x<Real, Scalar>(n, wt, u, v, e) + int_grad_u_grad_v<Real, Scalar>(n, wt, u, v));
}

template<typename Real, typename Scalar>
Scalar harmonicmagnetic_linear_form_real(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (harmonicmagneticPlanar)
        return harmonicmagneticLabel[e->marker].current_density_real * int_v<Real, Scalar>(n, wt, v);
    else
        return harmonicmagneticLabel[e->marker].current_density_real * int_v<Real, Scalar>(n, wt, v);
}

template<typename Real, typename Scalar>
Scalar harmonicmagnetic_linear_form_imag(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    if (harmonicmagneticPlanar)
        return harmonicmagneticLabel[e->marker].current_density_imag * int_v<Real, Scalar>(n, wt, v);
    else
        return harmonicmagneticLabel[e->marker].current_density_imag * int_v<Real, Scalar>(n, wt, v);
}

SolutionArray *harmonicmagnetic_main(SolverThread *solverThread,
                                     const char *fileName,
                                     HarmonicMagneticEdge *edge,
                                     HarmonicMagneticLabel *label)
{
    harmonicmagneticEdge = edge;
    harmonicmagneticLabel = label;
    frequency = Util::scene()->problemInfo().frequency;
    harmonicmagneticPlanar = (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR);
    int numberOfRefinements = Util::scene()->problemInfo().numberOfRefinements;
    int polynomialOrder = Util::scene()->problemInfo().polynomialOrder;
    AdaptivityType adaptivityType = Util::scene()->problemInfo().adaptivityType;
    int adaptivitySteps = Util::scene()->problemInfo().adaptivitySteps;
    double adaptivityTolerance = Util::scene()->problemInfo().adaptivityTolerance;

    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    int ndof;

    // load the mesh file
    Mesh mesh;
    mesh.load(fileName);
    for (int i = 0; i < numberOfRefinements; i++)
        mesh.refine_all_elements(0);

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    // initialize the shapeset and the cache
    H1Shapeset shapeset;
    PrecalcShapeset pssreal(&shapeset);
    PrecalcShapeset pssimag(&shapeset);

    // create the x displacement space
    H1Space spacereal(&mesh, &shapeset);
    spacereal.set_bc_types(harmonicmagnetic_bc_types);
    spacereal.set_bc_values(harmonicmagnetic_bc_values_real);
    spacereal.set_uniform_order(polynomialOrder);
    ndof = spacereal.assign_dofs(0);

    // create the y displacement space
    H1Space spaceimag(&mesh, &shapeset);
    spaceimag.set_bc_types(harmonicmagnetic_bc_types);
    spaceimag.set_bc_values(harmonicmagnetic_bc_values_imag);
    spaceimag.set_uniform_order(polynomialOrder);

    // initialize the weak formulation
    WeakForm wf(2);
    wf.add_biform(0, 0, callback(harmonicmagnetic_bilinear_form_real_real));
    wf.add_biform(0, 1, callback(harmonicmagnetic_bilinear_form_real_imag));
    wf.add_biform(1, 0, callback(harmonicmagnetic_bilinear_form_imag_real));
    wf.add_biform(1, 1, callback(harmonicmagnetic_bilinear_form_imag_imag));
    wf.add_liform(0, callback(harmonicmagnetic_linear_form_real));
    wf.add_liform(1, callback(harmonicmagnetic_linear_form_imag));

    // initialize the linear solver
    UmfpackSolver umfpack;
    Solution *sln1 = new Solution();
    Solution *sln2 = new Solution();
    Solution rsln1, rsln2;

    // assemble the stiffness matrix and solve the system
    double error;
    int i;
    int steps = (adaptivityType == ADAPTIVITYTYPE_NONE) ? 1 : adaptivitySteps;
    for (i = 0; i<(steps); i++)
    {
        int ndof = spacereal.assign_dofs(0);
        spaceimag.assign_dofs(ndof);

        // initialize the linear system
        LinSystem sys(&wf, &umfpack);
        sys.set_spaces(2, &spacereal, &spaceimag);
        sys.set_pss(2, &pssreal, &pssimag);
        sys.assemble();
        sys.solve(2, sln1, sln2);

        RefSystem rs(&sys);
        rs.assemble();
        rs.solve(2, &rsln1, &rsln2);

        // calculate errors and adapt the solution
        if (adaptivityType != ADAPTIVITYTYPE_NONE)
        {
            H1OrthoHP hp(2, &spacereal, &spaceimag);
            error = hp.calc_error_2(sln1, sln2, &rsln1, &rsln2) * 100;

            // emit signal
            solverThread->showMessage(QObject::tr("Solver: relative error: %1 %").arg(error, 0, 'f', 5), false);
            if (solverThread->isCanceled()) return NULL;

            if (error < adaptivityTolerance || sys.get_num_dofs() >= NDOF_STOP) break;
            hp.adapt(0.3, 0, (int) adaptivityType);
        }        
    }

    // output
    ndof = spacereal.assign_dofs(0);
    spaceimag.assign_dofs(ndof);

    SolutionArray *solutionArray = new SolutionArray();
    solutionArray->order1 = new Orderizer();
    solutionArray->order1->process_solution(&spacereal);
    solutionArray->order2 = new Orderizer();
    solutionArray->order2->process_solution(&spaceimag);
    solutionArray->sln1 = sln1;
    solutionArray->sln2 = sln2;
    solutionArray->adaptiveError = error;
    solutionArray->adaptiveSteps = i-1;

    return solutionArray;
}
