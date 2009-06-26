#include "scenesolution.h"

void dxdyfilter(int n, scalar* a, scalar* dadx, scalar* dady,
                scalar* out, scalar* outdx, scalar* outdy)
{
    for (int i = 0; i < n; i++)
    {
        out[i] = a[i];
        outdx[i] = dadx[i];
        outdy[i] = dady[i];
    }
}


SceneSolution::SceneSolution(Scene *scene)
{
    m_sln1 = NULL;
    m_sln2 = NULL;
    m_slnContourView = NULL;
    m_slnScalarView = NULL;
    m_slnVectorXView = NULL;
    m_slnVectorYView = NULL;

    m_scene = scene;
}

void SceneSolution::clear()
{
    if (m_sln1 != NULL)
    {
        delete m_sln1;
        m_sln1 = NULL;
        // delete m_sln1DXDY;
        // m_sln1DXDY = NULL;
    }
    if (m_sln2 != NULL)
    {
        delete m_sln2;
        m_sln2 = NULL;
    }

    // countour
    if (m_slnContourView != NULL)
    {
        delete m_slnContourView;
        m_slnContourView = NULL;
    }
    
    // scalar
    if (m_slnScalarView != NULL)
    {
        delete m_slnScalarView;
        m_slnScalarView = NULL;
    }
    
    // vector
    if (m_slnVectorXView != NULL)
    {
        delete m_slnVectorXView;
        m_slnVectorXView = NULL;
    }
    if (m_slnVectorYView != NULL)
    {
        delete m_slnVectorYView;
        m_slnVectorYView = NULL;
    }

    // order
    /*
    if (m_ordView != NULL)
    {
        delete m_ordView;
        m_ordView = NULL;
    }
    */
}

double SceneSolution::volumeIntegral(int labelIndex, PhysicFieldIntegralVolume physicFieldIntegralVolume)
{
    Quad2D* quad = &g_quad_2d_std;
    m_sln1->set_quad_2d(quad);
    
    double integral = 0.0;
    Element* e;
    Mesh *mesh = m_sln1->get_mesh();
    
    for_all_active_elements(e, mesh)
    {
        if (e->marker == labelIndex)
        {
            update_limit_table(e->get_mode());
            
            m_sln1->set_active_element(e);
            RefMap* ru = m_sln1->get_refmap();
            
            int o = m_sln1->get_fn_order() + ru->get_inv_ref_order();
            limit_order(o);
            m_sln1->set_quad_order(o, FN_VAL | FN_DX | FN_DY);
            // value
            scalar *uval = m_sln1->get_fn_values();
            // derivative
            scalar *dudx, *dudy;
            m_sln1->get_dx_dy_values(dudx, dudy);
            // x - coordinate
            double* x = ru->get_phys_x(o);
            
            double result = 0;
            switch (physicFieldIntegralVolume)
            {
            case PHYSICFIELDINTEGRAL_VOLUME_CROSSSECTION:
                {
                    h1_integrate_expression(1);
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_VOLUME:
                {
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(1);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i]);
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD_X:
                {
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(dudx[i]);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * dudx[i]);
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD_Y:
                {
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(dudy[i]);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * dudy[i]);
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD:
                {
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT_X:
                {
                    SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(m_scene->labels[e->marker]->marker);
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(EPS0 * marker->permittivity.value * dudx[i]);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * EPS0 * marker->permittivity.value * dudx[i]);
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT_Y:
                {
                    SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(m_scene->labels[e->marker]->marker);
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(EPS0 * marker->permittivity.value * dudy[i]);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * EPS0 * marker->permittivity.value * dudy[i]);
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT:
                {
                    SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(m_scene->labels[e->marker]->marker);
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(EPS0 * marker->permittivity.value * sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * EPS0 * marker->permittivity.value * sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ENERGY_DENSITY:
                {
                    SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(m_scene->labels[e->marker]->marker);
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(0.5 * EPS0 * marker->permittivity.value * (sqr(dudx[i]) + sqr(dudy[i])));
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * 0.5 * EPS0 * marker->permittivity.value * (sqr(dudx[i]) + sqr(dudy[i])));
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD_X:
                {
                    SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(m_scene->labels[e->marker]->marker);
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(-dudy[i] / (marker->permeability.value * MU0));
                    }
                    else
                    {
                        h1_integrate_expression(-2 * M_PI * x[i] * dudy[i] / (marker->permeability.value * MU0));
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD_Y:
                {
                    SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(m_scene->labels[e->marker]->marker);
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(dudx[i] / (marker->permeability.value * MU0));
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * (dudx[i] + ((x[i] > 0) ? uval[i] / x[i] : 0.0)) / (marker->permeability.value * MU0));
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD:
                {
                    SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(m_scene->labels[e->marker]->marker);
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(sqrt(sqr(dudx[i]) + sqr(dudy[i])) / (marker->permeability.value * MU0));
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * sqrt(sqr(dudy[i]) + sqr(dudx[i] + ((x[i] > 0) ? uval[i] / x[i] : 0.0))) / (marker->permeability.value * MU0));
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY_X:
                {
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(-dudy[i]);
                    }
                    else
                    {
                        h1_integrate_expression(-2 * M_PI * x[i] * dudy[i]);
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY_Y:
                {
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(dudx[i]);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * (dudx[i] + ((x[i] > 0) ? uval[i] / x[i] : 0.0)));
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY:
                {
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * sqrt(sqr(dudy[i]) + sqr(dudx[i] + ((x[i] > 0) ? uval[i] / x[i] : 0.0))));
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_ENERGY_DENSITY:
                {
                    SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(m_scene->labels[e->marker]->marker);
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(0.5 * (sqr(dudx[i]) + sqr(dudy[i])) / (marker->permeability.value * MU0));
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * 0.5 * sqr(sqrt(sqr(dudy[i]) + sqr(dudx[i] + ((x[i] > 0) ? uval[i] / x[i] : 0.0)))) / (marker->permeability.value * MU0));
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE:
                {
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(uval[i]);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * uval[i]);
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT_X:
                {
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(dudx[i]);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * dudx[i]);
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT_Y:
                {
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(dudy[i]);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * dudy[i]);
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT:
                {
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX_X:
                {
                    SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(m_scene->labels[e->marker]->marker);
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(marker->thermal_conductivity.value * dudx[i]);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * marker->thermal_conductivity.value * dudy[i]);
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX_Y:
                {
                    SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(m_scene->labels[e->marker]->marker);
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(marker->thermal_conductivity.value * dudy[i]);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * marker->thermal_conductivity.value * dudy[i]);
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX:
                {
                    SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(m_scene->labels[e->marker]->marker);
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(marker->thermal_conductivity.value * sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * marker->thermal_conductivity.value * sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_ELECTRICFIELD_X:
                {
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(dudx[i]);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * dudx[i]);
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_ELECTRICFIELD_Y:
                {
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(dudy[i]);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * dudy[i]);
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_ELECTRICFIELD:
                {
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_CURRENT_DENSITY_X:
                {
                    SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(m_scene->labels[e->marker]->marker);
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(marker->conductivity.value * dudx[i]);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * marker->conductivity.value * dudx[i]);
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_CURRENT_DENSITY_Y:
                {
                    SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(m_scene->labels[e->marker]->marker);
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(marker->conductivity.value * dudy[i]);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * marker->conductivity.value * dudy[i]);
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_CURRENT_DENSITY:
                {
                    SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(m_scene->labels[e->marker]->marker);
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(marker->conductivity.value * sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * marker->conductivity.value * sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                    }
                }
                break;
            case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_LOSSES:
                {
                    SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(m_scene->labels[e->marker]->marker);
                    if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                    {
                        h1_integrate_expression(marker->conductivity.value * (sqr(dudx[i]) + sqr(dudy[i])));
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i] * marker->conductivity.value * (sqr(dudx[i]) + sqr(dudy[i])));
                    }
                }
                break;
            }

            integral += result;
        }
    }
    
    return integral;
}

double SceneSolution::surfaceIntegral(int edgeIndex, PhysicFieldIntegralSurface physicFieldIntegralSurface)
{  
    double integral = 0.0;
    Quad2D* quad = &g_quad_2d_std;
    m_sln1->set_quad_2d(quad);

    Element* e;
    Mesh* mesh = m_sln1->get_mesh();

    for_all_active_elements(e, mesh)
    {
        for (int edge = 0; edge < e->nvert; edge++)
        {
            if (e->en[edge]->bnd && e->en[edge]->marker-1 == edgeIndex)
            {
                // cout << ((e->vn[0]->x-e->vn[1]->x)*e->vn[0]->y - (e->vn[0]->y-e->vn[1]->y)*e->vn[0]->x) << endl;

                update_limit_table(e->get_mode());

                m_sln1->set_active_element(e);
                RefMap* ru = m_sln1->get_refmap();

                Quad2D* quad2d = ru->get_quad_2d();
                int eo = quad2d->get_edge_points(edge);
                m_sln1->set_quad_order(eo, FN_VAL | FN_DX | FN_DY);
                double3* pt = quad2d->get_points(eo);
                double3* tan = ru->get_tangent(edge);
                // value
                scalar* value = m_sln1->get_fn_values();
                // derivative
                scalar *dudx, *dudy;
                m_sln1->get_dx_dy_values(dudx, dudy);
                // x - coordinate
                double* x = ru->get_phys_x(eo);

                for (int i = 0; i < quad2d->get_num_points(eo); i++)
                {
                    switch (physicFieldIntegralSurface)
                    {
                    case PHYSICFIELDINTEGRAL_SURFACE_LENGTH:
                        {
                            integral += pt[i][2] * tan[i][2];
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_SURFACE:
                        {
                            if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                                integral += pt[i][2] * tan[i][2];
                            else
                                integral += 2 * M_PI * x[i] * pt[i][2] * tan[i][2];
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_ELECTROSTATIC_CHARGE_DENSITY:
                        {
                            SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(m_scene->labels[e->marker]->marker);
                            if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                                integral += pt[i][2] * tan[i][2] * EPS0 * marker->permittivity.value * (tan[i][1] * dudx[i] + tan[i][0] * dudy[i]) * m_scene->edges[edgeIndex]->direction();
                            else
                                integral += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * EPS0 * marker->permittivity.value * (tan[i][1] * dudx[i] + tan[i][0] * dudy[i]) * m_scene->edges[edgeIndex]->direction();
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_HEAT_TEMPERATURE:
                        {
                            if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                                integral += pt[i][2] * tan[i][2] * tan[i][1] * value[i];
                            else
                                integral += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * value[i];
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_HEAT_TEMPERATURE_DIFFERENCE:
                        {
                            if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                                integral += pt[i][2] * tan[i][2] * (tan[i][0] * dudx[i] + tan[i][1] * dudy[i]) * m_scene->edges[edgeIndex]->direction();
                            else
                                integral += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * (tan[i][0] * dudx[i] + tan[i][1] * dudy[i]) * m_scene->edges[edgeIndex]->direction();
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_HEAT_FLUX:
                        {
                            SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(m_scene->labels[e->marker]->marker);
                            if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                                integral += pt[i][2] * tan[i][2] * marker->thermal_conductivity.value * (tan[i][1] * dudx[i] + tan[i][0] * dudy[i]) * m_scene->edges[edgeIndex]->direction();
                            else
                                integral += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * marker->thermal_conductivity.value * (tan[i][1] * dudx[i] + tan[i][0] * dudy[i]) * m_scene->edges[edgeIndex]->direction();
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_CURRENT_CURRENT_DENSITY:
                        {
                            SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(m_scene->labels[e->marker]->marker);
                            if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                                integral += pt[i][2] * tan[i][2] * marker->conductivity.value * (tan[i][1] * dudx[i] + tan[i][0] * dudy[i]) * m_scene->edges[edgeIndex]->direction();
                            else
                                integral += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * marker->conductivity.value * (tan[i][1] * dudx[i] + tan[i][0] * dudy[i]) * m_scene->edges[edgeIndex]->direction();
                        }
                        break;
                    default:
                        cerr << "Physical field boundary integral '" + QString::number(physicFieldIntegralSurface).toStdString() + "' is not implemented. SceneSolution::surfaceIntegral(int edgeIndex, PhysicFieldIntegralSurface physicFieldIntegralSurface)" << endl;
                        throw;
                        break;
                    }                   
                }
            }
        }
    }

    return integral / 2.0;
}

int SceneSolution::findTriangleInVectorizer(const Vectorizer &vec, const Point &point)
{
    double4* vecVert = vec.get_vertices();
    int3* vecTris = vec.get_triangles();

    for (int i = 0; i < vec.get_num_triangles(); i++)
    {
        bool inTriangle = true;

        int k;
        double z;
        for (int l = 0; l < 3; l++)
        {
            k = l + 1;
            if (k == 3)
                k = 0;

            z = (vecVert[vecTris[i][k]][0] - vecVert[vecTris[i][l]][0]) * (point.y - vecVert[vecTris[i][l]][1]) -
                (vecVert[vecTris[i][k]][1] - vecVert[vecTris[i][l]][1]) * (point.x - vecVert[vecTris[i][l]][0]);

            if (z < 0)
            {
                inTriangle = false;
                break;
            }
        }

        if (inTriangle)
            return i;
    }

    return -1;
}

int SceneSolution::findTriangleInMesh(Mesh &mesh, const Point &point)
{
    for (int i = 0; i < mesh.get_num_elements(); i++)
    {
        bool inTriangle = true;
        
        Element *element = mesh.get_element_fast(i);
        
        int k;
        double z;
        for (int l = 0; l < 3; l++)
        {
            k = l + 1;
            if (k == 3)
                k = 0;
            
            z = (element->vn[k]->x - element->vn[l]->x) * (point.y - element->vn[l]->y) - (element->vn[k]->y - element->vn[l]->y) * (point.x - element->vn[l]->x);
            
            if (z < 0)
            {
                inTriangle = false;
                break;
            }
        }
        
        if (inTriangle)
            return i;
    }
    
    return -1;
}

PointValue SceneSolution::pointValue(const Point &point)
{
    double value = 0;
    double dx = 0;
    double dy = 0;
    SceneLabelMarker *labelMarker = NULL;
    
    int index = -1;
    if (m_sln1 != NULL)
    {
        value = m_sln1->get_pt_value(point.x, point.y, FN_VAL_0);
        if (m_scene->projectInfo().physicField != PHYSICFIELD_ELASTICITY)
        {
            dx =  m_sln1->get_pt_value(point.x, point.y, FN_DX_0);
            dy =  m_sln1->get_pt_value(point.x, point.y, FN_DY_0);
        }
    }
    
    // find marker
    index = findTriangleInMesh(m_mesh, point);    
    if (index > 0)
    {
        Element *element = m_mesh.get_element_fast(index);
        labelMarker = m_scene->labels[element->marker]->marker;
    }
    
    return PointValue(value, Point(dx, dy), labelMarker);
}

void SceneSolution::setSolutionArray(SolutionArray *solutionArray)
{
    m_sln1 = solutionArray->sln1;
    m_sln2 = solutionArray->sln2;
    
    cout << m_sln1 << endl;

    if (m_scene->projectInfo().physicField != PHYSICFIELD_ELASTICITY)
        m_vec.process_solution(m_sln1, FN_DX_0, m_sln1, FN_DY_0, EPS_NORMAL);

    // order view
    m_ordView = *solutionArray->order1;
    // m_slnOrderView = *solutionArray->order2;

    m_adaptiveError = solutionArray->adaptiveError;
    m_adaptiveSteps = solutionArray->adaptiveSteps;
}

void SceneSolution::setSlnContourView(ViewScalarFilter *slnScalarView)
{
    if (m_slnContourView != NULL)
    {
        delete m_slnContourView;
        m_slnContourView = NULL;
    }
    
    m_slnContourView = slnScalarView;
    m_linContourView.process_solution(m_slnContourView);
}

void SceneSolution::setSlnScalarView(ViewScalarFilter *slnScalarView)
{
    if (m_slnScalarView != NULL)
    {
        delete m_slnScalarView;
        m_slnScalarView = NULL;
    }
    
    m_slnScalarView = slnScalarView;
    m_linScalarView.process_solution(m_slnScalarView, FN_VAL_0);

    // deformed shape
    if (m_scene->projectInfo().physicField == PHYSICFIELD_ELASTICITY)
    {
        double3* linVert = m_linScalarView.get_vertices();

        double min =  1e100;
        double max = -1e100;
        for (int i = 0; i < m_linScalarView.get_num_vertices(); i++)
        {
            double x = linVert[i][0];
            double y = linVert[i][1];

            double dx = m_scene->sceneSolution()->sln1()->get_pt_value(x, y);
            double dy = m_scene->sceneSolution()->sln2()->get_pt_value(x, y);

            double dm = sqrt(sqr(dx) + sqr(dy));

            if (dm < min) min = dm;
            if (dm > max) max = dm;
        }

        RectPoint rect = m_scene->boundingBox();
        double k = qMax(rect.width(), rect.height()) / qMax(min, max) / 15.0;

        for (int i = 0; i < m_linScalarView.get_num_vertices(); i++)
        {
            double x = linVert[i][0];
            double y = linVert[i][1];

            double dx = m_scene->sceneSolution()->sln1()->get_pt_value(x, y);
            double dy = m_scene->sceneSolution()->sln2()->get_pt_value(x, y);

            linVert[i][0] += k*dx;
            linVert[i][1] += k*dy;
        }
    }
}

void SceneSolution::setSlnVectorView(ViewScalarFilter *slnVectorXView, ViewScalarFilter *slnVectorYView)
{
    if (slnVectorXView != NULL)
    {
        delete m_slnVectorXView;
        m_slnVectorXView = NULL;
    }
    if (m_slnVectorYView != NULL)
    {
        delete m_slnVectorYView;
        m_slnVectorYView = NULL;
    }
    
    m_slnVectorXView = slnVectorXView;
    m_slnVectorYView = slnVectorYView;
    
    m_vecVectorView.process_solution(m_slnVectorXView, FN_VAL_0, m_slnVectorYView, FN_VAL_0, EPS_LOW);
}

// **************************************************************************************************************************

ViewScalarFilter::ViewScalarFilter(MeshFunction* sln, Scene *scene, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
        : Filter(sln) \
{
    m_scene = scene;
    m_physicFieldVariable = physicFieldVariable;
    m_physicFieldVariableComp = physicFieldVariableComp;
}

ViewScalarFilter::ViewScalarFilter(MeshFunction* sln1, MeshFunction* sln2, Scene *scene, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
        : Filter(sln1, sln2) \
{
    m_scene = scene;
    m_physicFieldVariable = physicFieldVariable;
    m_physicFieldVariableComp = physicFieldVariableComp;
}

double ViewScalarFilter::get_pt_value(double x, double y, int item)
{
    error("Not implemented");
}

void ViewScalarFilter::precalculate(int order, int mask)
{
    Quad2D* quad = quads[cur_quad];
    int np = quad->get_num_points(order);
    Node* node = new_node(FN_VAL_0, np);
    
    scalar *dudx, *dudy, *dvdx, *dvdy;
    scalar *value;

    if (sln[0] != NULL)
    {
        sln[0]->set_quad_order(order, FN_VAL | FN_DX | FN_DY);
        sln[0]->get_dx_dy_values(dudx, dudy);
        value = sln[0]->get_fn_values();
    }

    if (num == 2)
    {
        sln[1]->set_quad_order(order, FN_DX | FN_DY);
        sln[1]->get_dx_dy_values(dvdx, dvdy);
    }
    
    update_refmap();
    
    double *x = refmap->get_phys_x(order);
    Element* e = refmap->get_active_element();
    
    SceneLabelMarker *labelMarker = m_scene->labels[e->marker]->marker;
    
    for (int i = 0; i < np; i++)
    {
        switch (m_physicFieldVariable)
        {
        case PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL:
            {
                node->values[0][0][i] = value[i];
            }
            break;
        case PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD:
            {
                switch (m_physicFieldVariableComp)
                {
                case PHYSICFIELDVARIABLECOMP_X:
                    {
                        node->values[0][0][i] = -dudx[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] = -dudy[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] = sqrt(sqr(dudx[i]) + sqr(dudy[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT:
            {
                SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(labelMarker);
                
                switch (m_physicFieldVariableComp)
                {
                case PHYSICFIELDVARIABLECOMP_X:
                    {
                        node->values[0][0][i] = -EPS0 * marker->permittivity.value * dudx[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] = -EPS0 * marker->permittivity.value * dudy[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] = EPS0 * marker->permittivity.value * sqrt(sqr(dudx[i]) + sqr(dudy[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY:
            {
                SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(labelMarker);
                node->values[0][0][i] = 0.5 * EPS0 * marker->permittivity.value * (sqr(dudx[i]) + sqr(dudy[i]));
            }
            break;
        case PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY:
            {
                SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(labelMarker);
                node->values[0][0][i] = marker->permittivity.value;
            }
            break;
        case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE:
            {
                node->values[0][0][i] = value[i];
            }
            break;
        case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT:
            {
                switch (m_physicFieldVariableComp)
                {
                case PHYSICFIELDVARIABLECOMP_X:
                    {
                        node->values[0][0][i] = dudx[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] = dudy[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] = sqrt(sqr(dudx[i]) + sqr(dudy[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HEAT_FLUX:
            {
                SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(labelMarker);
                switch (m_physicFieldVariableComp)
                {
                case PHYSICFIELDVARIABLECOMP_X:
                    {
                        node->values[0][0][i] =  marker->thermal_conductivity.value * dudx[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] =  marker->thermal_conductivity.value * dudy[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] =  marker->thermal_conductivity.value * sqrt(sqr(dudx[i]) + sqr(dudy[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY:
            {
                SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(labelMarker);
                node->values[0][0][i] = marker->thermal_conductivity.value;
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL:
            {
                if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = value[i];
                }
                else
                {
                    node->values[0][0][i] = value[i] * x[i];
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY:
            {
                if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = -dudy[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = dudx[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy[i]) + sqr(dudx[i]));
                        }
                        break;
                    }
                }
                else
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = -dudy[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = dudx[i] + value[i] / x[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy[i]) + sqr(dudx[i] + value[i] / x[i]));
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD:
            {
                SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(labelMarker);
                if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = -dudy[i] / (marker->permeability.value * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = dudx[i] / (marker->permeability.value * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy[i]) + sqr(dudx[i])) / (marker->permeability.value * MU0);
                        }
                        break;
                    }
                }
                else
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = -dudy[i] / (marker->permeability.value * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = (dudx[i] + value[i] / x[i]) / (marker->permeability.value * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy[i]) + sqr(dudx[i] + value[i] / x[i])) / (marker->permeability.value * MU0);
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY:
            {
                SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(labelMarker);
                if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = 0.5 * (sqr(dudx[i]) + sqr(dudy[i])) / (marker->permeability.value * MU0);
                }
                else
                {
                    node->values[0][0][i] = 0.5 * sqr(sqrt(sqr(dudy[i]) + sqr(dudx[i] + ((x > 0) ? value[i] / x[i] : 0.0)))) / (marker->permeability.value * MU0);
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY:
            {
                SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(labelMarker);
                node->values[0][0][i] = marker->permeability.value;
            }
            break;
        case PHYSICFIELDVARIABLE_CURRENT_POTENTIAL:
            {
                node->values[0][0][i] = value[i];
            }
            break;
        case PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD:
            {
                switch (m_physicFieldVariableComp)
                {
                case PHYSICFIELDVARIABLECOMP_X:
                    {
                        node->values[0][0][i] = -dudx[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] = -dudy[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] = sqrt(sqr(dudx[i]) + sqr(dudy[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY:
            {
                SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarker);

                switch (m_physicFieldVariableComp)
                {
                case PHYSICFIELDVARIABLECOMP_X:
                    {
                        node->values[0][0][i] = -marker->conductivity.value * dudx[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] = -marker->conductivity.value * dudy[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] = marker->conductivity.value * sqrt(sqr(dudx[i]) + sqr(dudy[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_CURRENT_LOSSES:
            {
                SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarker);
                node->values[0][0][i] = marker->conductivity.value * (sqr(dudx[i]) + sqr(dudy[i]));
            }
            break;
        case PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY:
            {
                SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarker);
                node->values[0][0][i] = marker->conductivity.value;
            }
            break;
        case PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS:
            {
                SceneLabelElasticityMarker *marker = dynamic_cast<SceneLabelElasticityMarker *>(labelMarker);
                // stress tensor                
                double tz = marker->lambda() * (dudx[i] + dvdy[i]);
                double tx = tz + 2*marker->mu() * dudx[i];
                double ty = tz + 2*marker->mu() * dvdy[i];
                if (m_scene->projectInfo().problemType == PROBLEMTYPE_AXISYMMETRIC)
                    tz += 2*marker->mu() * value[i] / x[i];
                double txy = marker->mu() * (dudy[i] + dvdx[i]);

                // Von Mises stress
                node->values[0][0][i] = 1.0/sqrt(2.0) * sqrt(sqr(tx - ty) + sqr(ty - tz) + sqr(tz - tx) + 6*sqr(txy));
            }
            break;
        default:
            cerr << "Physical field variable '" + physicFieldVariableString(m_physicFieldVariable).toStdString() + "' is not implemented. ViewScalarFilter::precalculate(int order, int mask)" << endl;
            throw;
            break;
        }
    }

    replace_cur_node(node);
}

