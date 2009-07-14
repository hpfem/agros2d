#include "hermes_harmonicmagnetic.h"

inline double int_u_dvdx_over_x(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
    Quad2D* quad = fu->get_quad_2d();

    int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order() + 1;
    limit_order(o);
    fu->set_quad_order(o);
    fv->set_quad_order(o);

    double *dvdx, *dvdy;
    fv->get_dx_dy_values(dvdx, dvdy);
    double* uval = fu->get_fn_values();
    double* x = ru->get_phys_x(o);

    double result;
    h1_integrate_dd_expression(t_dvdx * uval[i] / x[i]);
    return result;
}

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

scalar harmonicmagnetic_bilinear_form_real_real(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
    int marker = rv->get_active_element()->marker;

    if (harmonicmagneticIsPlanar)
        return 1.0 / (harmonicmagneticLabel[marker].permeability * MU0) * int_grad_u_grad_v(fu, fv, ru, rv);
    else
        return 1.0 / (harmonicmagneticLabel[marker].permeability * MU0) * (int_u_dvdx_over_x(fu, fv, ru, rv) + int_grad_u_grad_v(fu, fv, ru, rv));
}

scalar harmonicmagnetic_bilinear_form_real_imag(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
    int marker = rv->get_active_element()->marker;

    if (harmonicmagneticIsPlanar)
        return - 2 * M_PI * Util::scene()->projectInfo().frequency * harmonicmagneticLabel[marker].conductivity * int_u_v(fu, fv, ru, rv);
    else
        return - 2 * M_PI * Util::scene()->projectInfo().frequency * harmonicmagneticLabel[marker].conductivity * int_u_v(fu, fv, ru, rv);
}

scalar harmonicmagnetic_bilinear_form_imag_real(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
    int marker = rv->get_active_element()->marker;

    if (harmonicmagneticIsPlanar)
        return + 2 * M_PI * Util::scene()->projectInfo().frequency * harmonicmagneticLabel[marker].conductivity * int_u_v(fu, fv, ru, rv);
    else
        return + 2 * M_PI * Util::scene()->projectInfo().frequency * harmonicmagneticLabel[marker].conductivity * int_u_v(fu, fv, ru, rv);
}

scalar harmonicmagnetic_bilinear_form_imag_imag(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
    int marker = rv->get_active_element()->marker;

    if (harmonicmagneticIsPlanar)
        return 1.0 / (harmonicmagneticLabel[marker].permeability * MU0) * int_grad_u_grad_v(fu, fv, ru, rv);
    else
        return 1.0 / (harmonicmagneticLabel[marker].permeability * MU0) * (int_u_dvdx_over_x(fu, fv, ru, rv) + int_grad_u_grad_v(fu, fv, ru, rv));
}

scalar harmonicmagnetic_linear_form_real(RealFunction* fv, RefMap* rv)
{
    int marker = rv->get_active_element()->marker;

    if (harmonicmagneticIsPlanar)
        return harmonicmagneticLabel[marker].current_density_real * int_v(fv, rv);
    else
        return harmonicmagneticLabel[marker].current_density_real * int_v(fv, rv);
}

scalar harmonicmagnetic_linear_form_imag(RealFunction* fv, RefMap* rv)
{
    int marker = rv->get_active_element()->marker;

    if (harmonicmagneticIsPlanar)
        return harmonicmagneticLabel[marker].current_density_imag * int_v(fv, rv);
    else
        return harmonicmagneticLabel[marker].current_density_imag * int_v(fv, rv);
}

SolutionArray *harmonicmagnetic_main(SolverDialog *solverDialog,
                                     const char *fileName,
                                     HarmonicMagneticEdge *edge,
                                     HarmonicMagneticLabel *label,
                                     int numberOfRefinements,
                                     int polynomialOrder,
                                     int adaptivitySteps,
                                     double adaptivityTolerance,
                                     bool isPlanar)
{
    harmonicmagneticEdge = edge;
    harmonicmagneticLabel = label;
    harmonicmagneticIsPlanar = isPlanar;

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
    wf.add_biform(0, 0, harmonicmagnetic_bilinear_form_real_real);
    wf.add_biform(0, 1, harmonicmagnetic_bilinear_form_real_imag);
    wf.add_biform(1, 0, harmonicmagnetic_bilinear_form_imag_real);
    wf.add_biform(1, 1, harmonicmagnetic_bilinear_form_imag_imag);
    wf.add_liform(0, harmonicmagnetic_linear_form_real);
    wf.add_liform(1, harmonicmagnetic_linear_form_imag);

    // initialize the linear solver
    UmfpackSolver umfpack;
    Solution *sln1 = new Solution();
    Solution *sln2 = new Solution();
    Solution rsln1, rsln2;

    // assemble the stiffness matrix and solve the system
    double error;
    int i;
    for (i = 0; i<(adaptivitySteps+1); i++)
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
        if (adaptivitySteps > 0)
        {
            H1OrthoHP hp(2, &spacereal, &spaceimag);
            error = hp.calc_error_2(sln1, sln2, &rsln1, &rsln2) * 100;

            // emit signal
            solverDialog->doShowMessage(QObject::tr("Relative error: ") + QString::number(error, 'f', 5) + " %");

            if (error < adaptivityTolerance || sys.get_num_dofs() >= NDOF_STOP) break;
            hp.adapt(0.3, 0, 0, 0);
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
