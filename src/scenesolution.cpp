#include "scenesolution.h"

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
    double integral = 0.0;

    Quad2D* quad;
    Mesh *mesh;
    Element* e;

    if (m_sln1 != NULL)
    {
        quad = &g_quad_2d_std;
        m_sln1->set_quad_2d(quad);

        mesh = m_sln1->get_mesh();

        for_all_active_elements(e, mesh)
        {
            if (e->marker == labelIndex)
            {
                update_limit_table(e->get_mode());

                RefMap *ru = m_sln1->get_refmap();

                int o;
                if (m_sln2 == NULL)
                    o = m_sln1->get_fn_order() + ru->get_inv_ref_order();
                else
                    o = m_sln1->get_fn_order() + m_sln2->get_fn_order() + ru->get_inv_ref_order();

                limit_order(o);

                scalar *dudx, *dudy, *dvdx, *dvdy;
                scalar *valueu, *valuev;
                double *x;

                // solution 1
                m_sln1->set_quad_order(o, FN_VAL | FN_DX | FN_DY);
                // value
                valueu = m_sln1->get_fn_values();
                // derivative
                m_sln1->get_dx_dy_values(dudx, dudy);
                // x - coordinate
                x = ru->get_phys_x(o);

                m_sln1->set_active_element(e);

                // solution 2
                if (m_sln2 != NULL)
                {
                    m_sln2->set_quad_order(o, FN_VAL | FN_DX | FN_DY);
                    // value
                    valuev = m_sln2->get_fn_values();
                    // derivative
                    m_sln2->get_dx_dy_values(dvdx, dvdy);

                    m_sln2->set_active_element(e);
                }

                update_limit_table(e->get_mode());

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
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
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
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
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
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
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
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
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
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(EPS0 * marker->permittivity.number * dudx[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * EPS0 * marker->permittivity.number * dudx[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT_Y:
                    {
                        SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(EPS0 * marker->permittivity.number * dudy[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * EPS0 * marker->permittivity.number * dudy[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT:
                    {
                        SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(EPS0 * marker->permittivity.number * sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * EPS0 * marker->permittivity.number * sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ENERGY_DENSITY:
                    {
                        SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(0.5 * EPS0 * marker->permittivity.number * (sqr(dudx[i]) + sqr(dudy[i])));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * 0.5 * EPS0 * marker->permittivity.number * (sqr(dudx[i]) + sqr(dudy[i])));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD_X:
                    {
                        SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(-dudy[i] / (marker->permeability.number * MU0));
                        }
                        else
                        {
                            h1_integrate_expression(-2 * M_PI * x[i] * dudy[i] / (marker->permeability.number * MU0));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD_Y:
                    {
                        SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(dudx[i] / (marker->permeability.number * MU0));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * (dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0)) / (marker->permeability.number * MU0));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD:
                    {
                        SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(sqrt(sqr(dudx[i]) + sqr(dudy[i])) / (marker->permeability.number * MU0));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * sqrt(sqr(dudy[i]) + sqr(dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0))) / (marker->permeability.number * MU0));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY_X:
                    {
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
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
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(dudx[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * (dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0)));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY:
                    {
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * sqrt(sqr(dudy[i]) + sqr(dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0))));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_ENERGY_DENSITY:
                    {
                        SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(0.5 * (sqr(dudx[i]) + sqr(dudy[i])) / (marker->permeability.number * MU0));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * 0.5 * sqr(sqrt(sqr(dudy[i]) + sqr(dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0)))) / (marker->permeability.number * MU0));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_REAL:
                    {
                        SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_IMAG:
                    {
                        SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(- 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]);
                        }
                        else
                        {
                            h1_integrate_expression(- 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL:
                    {
                        SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]);
                        }
                        else
                        {
                            h1_integrate_expression(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG:
                    {
                        SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->current_density_imag.number - 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]);
                        }
                        else
                        {
                            h1_integrate_expression(marker->current_density_imag.number - 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_POWER_LOSSES:
                    {
                        SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression((marker->conductivity.number > 0.0) ?
                                                    0.5 / marker->conductivity.number * (
                                                              sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i])
                                                            + sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]))
                                                    :
                                                    0.0);
                        }
                        else
                        {
                            h1_integrate_expression((marker->conductivity.number > 0.0) ?
                                                    0.5 / marker->conductivity.number * (
                                                            sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i])
                                                          + sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]))
                                                    :
                                                    0.0);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_ENERGY_DENSITY:
                    {
                        SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(0.25 * (sqr(dudx[i]) + sqr(dudy[i]) + sqr(dvdx[i]) + sqr(dvdy[i])) / (marker->permeability.number * MU0));
                        }
                        else
                        {
                            h1_integrate_expression((2 * M_PI * x[i] * 0.25 * sqr(sqrt(sqr(dudy[i]) + sqr(dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0)))) / (marker->permeability.number * MU0))
                                                  + (2 * M_PI * x[i] * 0.25 * sqr(sqrt(sqr(dvdy[i]) + sqr(dvdx[i] + ((x[i] > 0) ? valuev[i] / x[i] : 0.0)))) / (marker->permeability.number * MU0)));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_LORENTZ_FORCE_X_REAL:
                    {
                        SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) * dudx[i])
                                                    + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]) * dvdx[i]));
                        }
                        else
                        {
                            h1_integrate_expression(- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) * (dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0)))
                                                    + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]) * (dvdx[i] + ((x[i] > 0) ? valuev[i] / x[i] : 0.0))));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_LORENTZ_FORCE_Y_REAL:
                    {
                        SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) * dudy[i])
                                                    + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]) * dvdy[i]));
                        }
                        else
                        {
                            h1_integrate_expression(- ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) * dudy[i])
                                                    + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]) * dvdy[i]));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_LORENTZ_FORCE_X_IMAG:
                    {
                        SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(((marker->current_density_imag.number - 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]) * dudx[i])
                                                  + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) * dvdx[i]));
                        }
                        else
                        {
                            h1_integrate_expression(((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]) * (dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0)))
                                                  + ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) * (dvdx[i] + ((x[i] > 0) ? valuev[i] / x[i] : 0.0))));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_LORENTZ_FORCE_Y_IMAG:
                    {
                        SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(((marker->current_density_imag.number - 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]) * dudy[i])
                                                  + ((marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) * dvdy[i]));
                        }
                        else
                        {
                            h1_integrate_expression(((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]) * dudy[i])
                                                  + ((marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) * dvdy[i]));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE:
                    {
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(valueu[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * valueu[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT_X:
                    {
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
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
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
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
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
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
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->thermal_conductivity.number * dudx[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * marker->thermal_conductivity.number * dudy[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX_Y:
                    {
                        SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->thermal_conductivity.number * dudy[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * marker->thermal_conductivity.number * dudy[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX:
                    {
                        SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->thermal_conductivity.number * sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * marker->thermal_conductivity.number * sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_ELECTRICFIELD_X:
                    {
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
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
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
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
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
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
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->conductivity.number * dudx[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * marker->conductivity.number * dudx[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_CURRENT_DENSITY_Y:
                    {
                        SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->conductivity.number * dudy[i]);
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * marker->conductivity.number * dudy[i]);
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_CURRENT_DENSITY:
                    {
                        SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->conductivity.number * sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * marker->conductivity.number * sqrt(sqr(dudx[i]) + sqr(dudy[i])));
                        }
                    }
                    break;
                case PHYSICFIELDINTEGRAL_VOLUME_CURRENT_LOSSES:
                    {
                        SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(m_scene->labels[e->marker]->marker);
                        if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                        {
                            h1_integrate_expression(marker->conductivity.number * (sqr(dudx[i]) + sqr(dudy[i])));
                        }
                        else
                        {
                            h1_integrate_expression(2 * M_PI * x[i] * marker->conductivity.number * (sqr(dudx[i]) + sqr(dudy[i])));
                        }
                    }
                    break;
                }

                integral += result;
            }
        }
    }

    // solution 2
    /*
    if (m_sln2 != NULL)
    {
        quad = &g_quad_2d_std;
        m_sln2->set_quad_2d(quad);
        
        mesh = m_sln2->get_mesh();

        for_all_active_elements(e, mesh)
        {
            if (e->marker == labelIndex)
            {
                update_limit_table(e->get_mode());

                m_sln2->set_active_element(e);
                RefMap* ru = m_sln2->get_refmap();

                int o = m_sln2->get_fn_order() + ru->get_inv_ref_order();
                limit_order(o);
                m_sln2->set_quad_order(o, FN_VAL | FN_DX | FN_DY);
                // value
                scalar *valuev = m_sln2->get_fn_values();
                // derivative
                scalar *dvdx, *dvdy;
                m_sln2->get_dx_dy_values(dvdx, dvdy);
                // x - coordinate
                double* x = ru->get_phys_x(o);

                double result = 0;
                switch (physicFieldIntegralVolume)
                {

                integral2 += result;
            }
        }
    }

    if ((physicFieldIntegralVolume == PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_LORENTZ_FORCE_X_REAL) ||
        (physicFieldIntegralVolume == PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_LORENTZ_FORCE_Y_REAL))
    {
        double integral = 0.0;

        Quad2D* quad;
        Mesh *mesh;
        Element* e;

        quad = &g_quad_2d_std;
        m_sln1->set_quad_2d(quad);
        m_sln2->set_quad_2d(quad);

        mesh = m_sln1->get_mesh();

        double result;
        for_all_active_elements(e, mesh)
        {
            result = 0.0;
            if (e->marker == labelIndex)
            {
                SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(m_scene->labels[e->marker]->marker);

                update_limit_table(e->get_mode());

                m_sln1->set_active_element(e);
                m_sln2->set_active_element(e);
                RefMap* ru = m_sln1->get_refmap();

                int o = m_sln1->get_fn_order() + m_sln2->get_fn_order() + ru->get_inv_ref_order();
                limit_order(o);
                m_sln1->set_quad_order(o, FN_VAL | FN_DX | FN_DY);
                m_sln2->set_quad_order(o, FN_VAL | FN_DX | FN_DY);
                // value
                scalar *valueu = m_sln1->get_fn_values();
                scalar *valuev = m_sln2->get_fn_values();
                // derivative
                scalar *dudx, *dudy, *dvdx, *dvdy;
                m_sln1->get_dx_dy_values(dudx, dudy);
                m_sln2->get_dx_dy_values(dvdx, dvdy);
                // x - coordinate
                double* x = ru->get_phys_x(o);

                double3* pt = quad->get_points(o);
                int np = quad->get_num_points(o);
                if (ru->is_jacobian_const())
                {
                    for (int i = 0; i < np; i++)
                    {
                        if (physicFieldIntegralVolume == PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_LORENTZ_FORCE_X_REAL)
                        {
                            if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                                result += pt[i][2] * (- (marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) * dudx[i]
                                                      + (marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]) * dvdx[i]);
                            else
                                result += pt[i][2] * (- (marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) * (dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0))
                                                      + (marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]) * (dvdx[i] + ((x[i] > 0) ? valuev[i] / x[i] : 0.0)));
                        }
                        if (physicFieldIntegralVolume == PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_LORENTZ_FORCE_Y_REAL)
                        {
                            if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                                result += pt[i][2] * (- (marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) * dudy[i]
                                                      + (marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]) * dvdy[i]);
                            else
                                result += pt[i][2] * (- (marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) * dudy[i]
                                                      + (marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]) * dvdy[i]);
                        }
                    }
                    result *= ru->get_const_jacobian();
                }
                else
                {
                    double* jac = ru->get_jacobian(o);
                    for (int i = 0; i < np; i++)
                    {
                        if (physicFieldIntegralVolume == PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_LORENTZ_FORCE_X_REAL)
                            result += pt[i][2] * jac[i] * (- (marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) * dudx[i]
                                                           + (marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]) * dvdx[i]);
                        if (physicFieldIntegralVolume == PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_LORENTZ_FORCE_Y_REAL)
                            result += pt[i][2] * jac[i] * (- (marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) * dudy[i]
                                                           + (marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]) * dvdy[i]);
                    }
                }
            }
            integral += result;
        }

        return integral;
    }
    else
    */
    return integral;
}

double SceneSolution::surfaceIntegral(int edgeIndex, PhysicFieldIntegralSurface physicFieldIntegralSurface)
{  
    double integral = 0.0;
    Quad2D* quad = &g_quad_2d_std;
    m_sln1->set_quad_2d(quad);

    Element* e;
    Mesh* mesh = m_sln1->get_mesh();

    /*
    // mark unpaired edges
    int count = 0;
    for_all_active_elements(e, mesh)
    {
        for (int edge = 0; edge < e->nvert; edge++)
        {
            if (e->marker == 1)
            {
                if (e->en[edge]->id > count) count = e->en[edge]->id;
            }
        }
    }

    int edges[count];
    for (int i = 0; i < count; i++)
        edges[i] = 0;

    for_all_active_elements(e, mesh)
    {
        for (int edge = 0; edge < e->nvert; edge++)
        {
            if (e->marker == 1)
            {
                edges[e->en[edge]->id]++;
            }
        }
    }
    */

    for_all_active_elements(e, mesh)
    {
        for (int edge = 0; edge < e->nvert; edge++)
        {
            if (e->en[edge]->bnd && e->en[edge]->marker-1 == edgeIndex)
                // if (e->marker == 1)
            {
                // if (edges[e->en[edge]->id] != 1) continue;

                update_limit_table(e->get_mode());

                m_sln1->set_active_element(e);
                RefMap* ru = m_sln1->get_refmap();

                Quad2D* quad2d = ru->get_quad_2d();
                int eo = quad2d->get_edge_points(edge);
                m_sln1->set_quad_order(eo, FN_VAL | FN_DX | FN_DY);
                double3* pt = quad2d->get_points(eo);
                double3* tan = ru->get_tangent(edge);

                // value
                scalar* valueu = m_sln1->get_fn_values();
                // derivative
                scalar *dudx, *dudy;
                m_sln1->get_dx_dy_values(dudx, dudy);
                // x - coordinate
                double* x = ru->get_phys_x(eo);

                double result = 0;
                for (int i = 0; i < quad2d->get_num_points(eo); i++)
                {
                    switch (physicFieldIntegralSurface)
                    {
                    case PHYSICFIELDINTEGRAL_SURFACE_LENGTH:
                        {
                            result += pt[i][2] * tan[i][2];
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_SURFACE:
                        {
                            if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                                result += pt[i][2] * tan[i][2];
                            else
                                result += 2 * M_PI * x[i] * pt[i][2] * tan[i][2];
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_ELECTROSTATIC_CHARGE_DENSITY:
                        {
                            SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(m_scene->labels[e->marker]->marker);
                            if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                                result += pt[i][2] * tan[i][2] * EPS0 * marker->permittivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
                            else
                                result += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * EPS0 * marker->permittivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_HEAT_TEMPERATURE:
                        {
                            if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                                result += pt[i][2] * tan[i][2] * valueu[i];
                            else
                                result += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * valueu[i];
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_HEAT_TEMPERATURE_DIFFERENCE:
                        {
                            if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                                result += pt[i][2] * tan[i][2] * (tan[i][0] * dudx[i] + tan[i][1] * dudy[i]);
                            else
                                result += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * (tan[i][0] * dudx[i] + tan[i][1] * dudy[i]);
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_HEAT_FLUX:
                        {
                            SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(m_scene->labels[e->marker]->marker);
                            if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                                result += pt[i][2] * tan[i][2] * marker->thermal_conductivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
                            else
                                result += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * marker->thermal_conductivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
                        }
                        break;
                    case PHYSICFIELDINTEGRAL_SURFACE_CURRENT_CURRENT_DENSITY:
                        {
                            SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(m_scene->labels[e->marker]->marker);
                            if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                                result += pt[i][2] * tan[i][2] * marker->conductivity.number * (tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
                            else
                                result += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] * marker->conductivity.number * (- tan[i][1] * dudx[i] - tan[i][0] * dudy[i]);
                        }
                        break;
                    default:
                        cerr << "Physical field boundary integral '" + QString::number(physicFieldIntegralSurface).toStdString() + "' is not implemented. SceneSolution::surfaceIntegral(int edgeIndex, PhysicFieldIntegralSurface physicFieldIntegralSurface)" << endl;
                        throw;
                        break;
                    }                   
                }

                integral += result;
                // cout << e->en[edge]->marker << " : " << e->en[edge]->id << " : " << result << endl;
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

PointValue SceneSolution::pointValue(const Point &point, Solution *sln)
{
    double value = 0;
    double dx = 0;
    double dy = 0;
    SceneLabelMarker *labelMarker = NULL;
    
    int index = -1;
    if (m_sln1 != NULL)
    {
        value = sln->get_pt_value(point.x, point.y, FN_VAL_0);
        if (m_scene->problemInfo().physicField != PHYSICFIELD_ELASTICITY)
        {
            dx =  sln->get_pt_value(point.x, point.y, FN_DX_0);
            dy =  sln->get_pt_value(point.x, point.y, FN_DY_0);
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
    
    if (m_scene->problemInfo().physicField != PHYSICFIELD_ELASTICITY)
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
    if (m_scene->problemInfo().physicField == PHYSICFIELD_ELASTICITY)
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
    scalar *valueu, *valuev;

    if (sln[0] != NULL)
    {
        sln[0]->set_quad_order(order, FN_VAL | FN_DX | FN_DY);
        sln[0]->get_dx_dy_values(dudx, dudy);
        valueu = sln[0]->get_fn_values();
    }

    if (num == 2)
    {
        sln[1]->set_quad_order(order, FN_VAL | FN_DX | FN_DY);
        sln[1]->get_dx_dy_values(dvdx, dvdy);
        valuev = sln[1]->get_fn_values();
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
                node->values[0][0][i] = valueu[i];
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
                        node->values[0][0][i] = -EPS0 * marker->permittivity.number * dudx[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] = -EPS0 * marker->permittivity.number * dudy[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] = EPS0 * marker->permittivity.number * sqrt(sqr(dudx[i]) + sqr(dudy[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY:
            {
                SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(labelMarker);
                node->values[0][0][i] = 0.5 * EPS0 * marker->permittivity.number * (sqr(dudx[i]) + sqr(dudy[i]));
            }
            break;
        case PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY:
            {
                SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(labelMarker);
                node->values[0][0][i] = marker->permittivity.number;
            }
            break;
        case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE:
            {
                node->values[0][0][i] = valueu[i];
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
                        node->values[0][0][i] =  marker->thermal_conductivity.number * dudx[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] =  marker->thermal_conductivity.number * dudy[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] =  marker->thermal_conductivity.number * sqrt(sqr(dudx[i]) + sqr(dudy[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY:
            {
                SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(labelMarker);
                node->values[0][0][i] = marker->thermal_conductivity.number;
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL:
            {
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = valueu[i];
                }
                else
                {
                    node->values[0][0][i] = valueu[i] * x[i];
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY:
            {
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
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
                            node->values[0][0][i] = dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy[i]) + sqr(dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0)));
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD:
            {
                SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(labelMarker);
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = -dudy[i] / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = dudx[i] / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy[i]) + sqr(dudx[i])) / (marker->permeability.number * MU0);
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
                            node->values[0][0][i] = -dudy[i] / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = (dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0)) / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy[i]) + sqr(dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY:
            {
                SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(labelMarker);
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = 0.5 * (sqr(dudx[i]) + sqr(dudy[i])) / (marker->permeability.number * MU0);
                }
                else
                {
                    node->values[0][0][i] = 0.5 * sqr(sqrt(sqr(dudy[i]) + sqr(dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0)))) / (marker->permeability.number * MU0);
                }
            }
            break;
        case PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY:
            {
                SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(labelMarker);
                node->values[0][0][i] = marker->permeability.number;
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL:
            {
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = sqrt(sqr(valueu[i]) + sqr(valuev[i]));
                }
                else
                {
                    node->values[0][0][i] = sqrt(sqr(valueu[i]) + sqr(valuev[i])) * x[i];
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL_REAL:
            {
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = valueu[i];
                }
                else
                {
                    node->values[0][0][i] = valueu[i] * x[i];
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL_IMAG:
            {
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = valuev[i];
                }
                else
                {
                    node->values[0][0][i] = valuev[i] * x[i];
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY:
            {
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = sqrt(sqr(dudx[i]) + sqr(dvdx[i]) + sqr(dudy[i]) + sqr(dvdy[i]));
                }
                else
                {
                    node->values[0][0][i] = sqrt(sqr(dudy[i]) + sqr(dvdy[i]) + sqr(dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0)) + sqr(dvdx[i] + ((x > 0) ? valuev[i] / x[i] : 0.0)));
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY_REAL:
            {
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = dudy[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - dudx[i];
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
                            node->values[0][0][i] = dudy[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - dudx[i] - ((x[i] > 0) ? valueu[i] / x[i] : 0.0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy[i]) + sqr(dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0)));
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY_IMAG:
            {
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = dvdy[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - dvdx[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dvdy[i]) + sqr(dvdx[i]));
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
                            node->values[0][0][i] = dvdy[i];
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - dvdx[i] - ((x > 0) ? valuev[i] / x[i] : 0.0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dvdy[i]) + sqr(dvdx[i] + ((x > 0) ? valuev[i] / x[i] : 0.0)));
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD:
            {
                SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(labelMarker);
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy[i]) + sqr(dvdy[i])) / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudx[i]) + sqr(dvdx[i])) / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudx[i]) + sqr(dvdx[i]) + sqr(dudy[i]) + sqr(dvdy[i])) / (marker->permeability.number * MU0);
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
                            node->values[0][0][i] = sqrt(sqr(dudy[i]) + sqr(dvdy[i])) / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0)) + sqr(dvdx[i] + ((x > 0) ? valuev[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy[i]) + sqr(dvdy[i]) + sqr(dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0)) + sqr(dvdx[i] + ((x > 0) ? valuev[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD_REAL:
            {
                SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(labelMarker);
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = dudy[i] / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - dudx[i] / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy[i]) + sqr(dudx[i])) / (marker->permeability.number * MU0);
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
                            node->values[0][0][i] = dudy[i] / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - (dudx[i] - ((x[i] > 0) ? valueu[i] / x[i] : 0.0)) / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dudy[i]) + sqr(dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD_IMAG:
            {
                SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(labelMarker);
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    switch (m_physicFieldVariableComp)
                    {
                    case PHYSICFIELDVARIABLECOMP_X:
                        {
                            node->values[0][0][i] = dvdy[i] / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - dvdx[i] / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dvdy[i]) + sqr(dvdx[i])) / (marker->permeability.number * MU0);
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
                            node->values[0][0][i] = dvdy[i] / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_Y:
                        {
                            node->values[0][0][i] = - (dvdx[i] - ((x > 0) ? valuev[i] / x[i] : 0.0)) / (marker->permeability.number * MU0);
                        }
                        break;
                    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                        {
                            node->values[0][0][i] = sqrt(sqr(dvdy[i]) + sqr(dvdx[i] + ((x > 0) ? valuev[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
                        }
                        break;
                    }
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL:
            {
                SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(labelMarker);
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = sqrt(
                            sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) +
                            sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]));
                }
                else
                {
                    node->values[0][0][i] = sqrt(
                            sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) +
                            sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]));
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL:
            {
                SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(labelMarker);
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i];
                }
                else
                {
                    node->values[0][0][i] = marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i];
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG:
            {
                SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(labelMarker);
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = marker->current_density_imag.number - 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i];
                }
                else
                {
                    node->values[0][0][i] = marker->current_density_imag.number - 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i];
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED:
            {
                SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(labelMarker);
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = sqrt(
                            sqr(2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) +
                            sqr(2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]));
                }
                else
                {
                    node->values[0][0][i] = sqrt(
                            sqr(2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) +
                            sqr(2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]));
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_REAL:
            {
                SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(labelMarker);
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i];
                }
                else
                {
                    node->values[0][0][i] = 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i];
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_IMAG:
            {
                SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(labelMarker);
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = - 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i];
                }
                else
                {
                    node->values[0][0][i] = - 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i];
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_POWER_LOSSES:
            {
                SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(labelMarker);
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = (marker->conductivity.number > 0.0) ?
                                            0.5 / marker->conductivity.number * (
                                                    sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) +
                                                    sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]))
                                            :
                                            0.0;
                }
                else
                {
                    node->values[0][0][i] = (marker->conductivity.number > 0.0) ?
                                            0.5 / marker->conductivity.number * (
                                                    sqr(marker->current_density_real.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valuev[i]) +
                                                    sqr(marker->current_density_imag.number + 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * valueu[i]))
                                            :
                                            0.0;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_ENERGY_DENSITY:
            {
                SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(labelMarker);
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_PLANAR)
                {
                    node->values[0][0][i] = 0.25 * (sqr(dudx[i]) + sqr(dvdx[i]) + sqr(dudy[i]) + sqr(dvdy[i])) / (marker->permeability.number * MU0);
                }
                else
                {
                    node->values[0][0][i] = 0.25 * (sqr(dudy[i]) + sqr(dvdy[i]) + sqr(dudx[i] + ((x[i] > 0) ? valueu[i] / x[i] : 0.0)) + sqr(dvdx[i] + ((x > 0) ? valuev[i] / x[i] : 0.0))) / (marker->permeability.number * MU0);
                }
            }
            break;
        case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_PERMEABILITY:
            {
                SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(labelMarker);
                node->values[0][0][i] = marker->permeability.number;
            }
            break;                
        case PHYSICFIELDVARIABLE_CURRENT_POTENTIAL:
            {
                node->values[0][0][i] = valueu[i];
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
                        node->values[0][0][i] = -marker->conductivity.number * dudx[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_Y:
                    {
                        node->values[0][0][i] = -marker->conductivity.number * dudy[i];
                    }
                    break;
                case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                    {
                        node->values[0][0][i] = marker->conductivity.number * sqrt(sqr(dudx[i]) + sqr(dudy[i]));
                    }
                    break;
                }
            }
            break;
        case PHYSICFIELDVARIABLE_CURRENT_LOSSES:
            {
                SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarker);
                node->values[0][0][i] = marker->conductivity.number * (sqr(dudx[i]) + sqr(dudy[i]));
            }
            break;
        case PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY:
            {
                SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(labelMarker);
                node->values[0][0][i] = marker->conductivity.number;
            }
            break;
        case PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS:
            {
                SceneLabelElasticityMarker *marker = dynamic_cast<SceneLabelElasticityMarker *>(labelMarker);
                // stress tensor                
                double tz = marker->lambda() * (dudx[i] + dvdy[i]);
                double tx = tz + 2*marker->mu() * dudx[i];
                double ty = tz + 2*marker->mu() * dvdy[i];
                if (m_scene->problemInfo().problemType == PROBLEMTYPE_AXISYMMETRIC)
                    tz += 2*marker->mu() * valueu[i] / x[i];
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
