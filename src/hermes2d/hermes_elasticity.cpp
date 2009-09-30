#include "hermes_elasticity.h"

static ElasticityEdge *elasticityEdge;
static ElasticityLabel *elasticityLabel;
static bool elasticityPlanar;

int elasticity_bc_types_x(int marker)
{
    switch (elasticityEdge[marker].typeX)
    {
    case PHYSICFIELDBC_NONE:
        return BC_NONE;
        break;
    case PHYSICFIELDBC_ELASTICITY_FIXED:
        return BC_ESSENTIAL;
        break;
    case PHYSICFIELDBC_ELASTICITY_FREE:
        return BC_NATURAL;
        break;
    }
}

int elasticity_bc_types_y(int marker)
{
    switch (elasticityEdge[marker].typeY)
    {
    case PHYSICFIELDBC_NONE:
        return BC_NONE;
        break;
    case PHYSICFIELDBC_ELASTICITY_FIXED:
        return BC_ESSENTIAL;
        break;
    case PHYSICFIELDBC_ELASTICITY_FREE:
        return BC_NATURAL;
        break;
    }
}

scalar elasticity_bc_values_x(int marker, double x, double y)
{
    switch (elasticityEdge[marker].typeX)
    {
    case PHYSICFIELDBC_NONE:
        return 0;
        break;
    case PHYSICFIELDBC_ELASTICITY_FIXED:
        return 0;
        break;
    case PHYSICFIELDBC_ELASTICITY_FREE:
        return elasticityEdge[marker].forceX;
        break;
    }
}

scalar elasticity_bc_values_y(int marker, double x, double y)
{
    switch (elasticityEdge[marker].typeY)
    {
    case PHYSICFIELDBC_NONE:
        return 0;
        break;
    case PHYSICFIELDBC_ELASTICITY_FIXED:
        return 0;
        break;
    case PHYSICFIELDBC_ELASTICITY_FREE:
        return elasticityEdge[marker].forceY;
        break;
    }
}

template<typename Real, typename Scalar>
Scalar elasticity_bilinear_form_0_0(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return (elasticityLabel[e->marker].lambda() + 2*elasticityLabel[e->marker].mu()) * int_dudx_dvdx<Real, Scalar>(n, wt, u, v) +
            elasticityLabel[e->marker].mu() * int_dudy_dvdy<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar elasticity_bilinear_form_0_1(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return elasticityLabel[e->marker].lambda() * int_dudy_dvdx<Real, Scalar>(n, wt, u, v) +
           elasticityLabel[e->marker].mu() * int_dudx_dvdy<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar elasticity_bilinear_form_1_0(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return  elasticityLabel[e->marker].mu() * int_dudy_dvdx<Real, Scalar>(n, wt, u, v) +
            elasticityLabel[e->marker].lambda() * int_dudx_dvdy<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar elasticity_bilinear_form_1_1(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return   elasticityLabel[e->marker].mu() * int_dudx_dvdx<Real, Scalar>(n, wt, u, v) +
            (elasticityLabel[e->marker].lambda() + 2*elasticityLabel[e->marker].mu()) * int_dudy_dvdy<Real, Scalar>(n, wt, u, v);
}

template<typename Real, typename Scalar>
Scalar elasticity_linear_form_surf(int n, double *wt, Func<Real> *v, Geom<Real> *e, ExtData<Scalar> *ext)
{
    return elasticityEdge[e->marker].forceY * int_v<Real, Scalar>(n, wt, v);
}

SolutionArray *elasticity_main(const char *fileName,
                               ElasticityEdge *edge,
                               ElasticityLabel *label)
{
    elasticityEdge = edge;
    elasticityLabel = label;
    elasticityPlanar = (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR);
    int numberOfRefinements = Util::scene()->problemInfo().numberOfRefinements;
    int polynomialOrder = Util::scene()->problemInfo().polynomialOrder;
    int adaptivitySteps = Util::scene()->problemInfo().adaptivitySteps;
    double adaptivityTolerance = Util::scene()->problemInfo().adaptivityTolerance;

    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    // load the mesh file
    Mesh xmesh, ymesh;
    xmesh.load(fileName);
    for (int i = 0; i < numberOfRefinements; i++)
        xmesh.refine_all_elements(0);
    ymesh.copy(&xmesh);

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    // initialize the shapeset and the cache
    H1Shapeset shapeset;
    PrecalcShapeset xpss(&shapeset);
    PrecalcShapeset ypss(&shapeset);

    // create the x displacement space
    H1Space xdisp(&xmesh, &shapeset);
    xdisp.set_bc_types(elasticity_bc_types_x);
    xdisp.set_bc_values(elasticity_bc_values_x);
    xdisp.set_uniform_order(polynomialOrder);
    int ndof = xdisp.assign_dofs(0);

    // create the y displacement space
    H1Space ydisp(&ymesh, &shapeset);
    ydisp.set_bc_types(elasticity_bc_types_y);
    ydisp.set_bc_values(elasticity_bc_values_y);
    ydisp.set_uniform_order(polynomialOrder);
    ydisp.assign_dofs();

    // initialize the weak formulation
    WeakForm wf(2);    
    wf.add_biform(0, 0, callback(elasticity_bilinear_form_0_0), UNSYM);
    wf.add_biform(0, 1, callback(elasticity_bilinear_form_0_1), UNSYM);
    wf.add_biform(1, 0, callback(elasticity_bilinear_form_1_0), UNSYM);
    wf.add_biform(1, 1, callback(elasticity_bilinear_form_1_1), UNSYM);
    wf.add_liform_surf(1, callback(elasticity_linear_form_surf));

    // initialize the linear system and solver
    UmfpackSolver umfpack;
    LinSystem sys(&wf, &umfpack);
    sys.set_spaces(2, &xdisp, &ydisp);
    sys.set_pss(2, &xpss, &ypss);

    // assemble the stiffness matrix and solve the system
    Solution *sln1 = new Solution();
    Solution *sln2 = new Solution();
    sys.assemble();
    sys.solve(2, sln1, sln2);

    // output
    xdisp.assign_dofs();
    ydisp.assign_dofs();

    SolutionArray *solutionArray = new SolutionArray();
    solutionArray->order1 = new Orderizer();
    solutionArray->order1->process_solution(&xdisp);
    solutionArray->order2 = new Orderizer();
    solutionArray->order2->process_solution(&ydisp);
    solutionArray->sln1 = sln1;
    solutionArray->sln2 = sln2;
    // solutionArray->adaptiveError = error;
    // solutionArray->adaptiveSteps = i-1;

    return solutionArray;
}
