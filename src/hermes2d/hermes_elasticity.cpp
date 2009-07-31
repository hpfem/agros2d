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

scalar elasticity_bilinear_form_0_0(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
    int marker = rv->get_active_element()->marker;
    return int_a_dudx_dvdx_b_dudy_dvdy(elasticityLabel[marker].lambda()+2*elasticityLabel[marker].mu(), fu, elasticityLabel[marker].mu(), fv, ru, rv);
}

scalar elasticity_bilinear_form_0_1(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
    int marker = rv->get_active_element()->marker;
    return int_a_dudx_dvdy_b_dudy_dvdx(elasticityLabel[marker].lambda(), fv, elasticityLabel[marker].mu(), fu, rv, ru);
}

scalar elasticity_bilinear_form_1_0(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
    int marker = rv->get_active_element()->marker;
    return int_a_dudx_dvdy_b_dudy_dvdx(elasticityLabel[marker].lambda(), fu, elasticityLabel[marker].mu(), fv, ru, rv);
}

scalar elasticity_bilinear_form_1_1(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
    int marker = rv->get_active_element()->marker;
    return int_a_dudx_dvdx_b_dudy_dvdy(elasticityLabel[marker].mu(), fu, elasticityLabel[marker].lambda()+2*elasticityLabel[marker].mu(), fv, ru, rv);
}

scalar elasticity_linear_form_surf(RealFunction* fv, RefMap* rv, EdgePos* ep)
{
    return elasticityEdge[ep->marker].forceY * surf_int_v(fv, rv, ep);
}

SolutionArray *elasticity_main(const char *fileName,
                               ElasticityEdge *edge,
                               ElasticityLabel *label)
{
    elasticityEdge = edge;
    elasticityLabel = label;
    elasticityPlanar = (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR);

    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    // load the mesh file
    Mesh xmesh, ymesh;
    xmesh.load(fileName);
    for (int i = 0; i < Util::scene()->problemInfo().numberOfRefinements; i++)
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
    xdisp.set_uniform_order(Util::scene()->problemInfo().polynomialOrder);
    int ndof = xdisp.assign_dofs(0);

    // create the y displacement space
    H1Space ydisp(&ymesh, &shapeset);
    ydisp.set_bc_types(elasticity_bc_types_y);
    ydisp.set_bc_values(elasticity_bc_values_y);
    ydisp.set_uniform_order(Util::scene()->problemInfo().polynomialOrder);
    ydisp.assign_dofs();

    // initialize the weak formulation
    WeakForm wf(2);    
    wf.add_biform(0, 0, elasticity_bilinear_form_0_0, UNSYM);
    wf.add_biform(0, 1, elasticity_bilinear_form_0_1, UNSYM);
    wf.add_biform(1, 0, elasticity_bilinear_form_1_0, UNSYM);
    wf.add_biform(1, 1, elasticity_bilinear_form_1_1, UNSYM);
    wf.add_liform_surf(1, elasticity_linear_form_surf);

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
