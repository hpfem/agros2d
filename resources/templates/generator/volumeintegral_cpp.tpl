// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "{{ID}}_volumeintegral.h"

#include "util.h"
#include "util/global.h"


#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/field.h"
#include "hermes2d/solutionstore.h"

#include "hermes2d/plugin_interface.h"

{{CLASS}}VolumeIntegral::{{CLASS}}VolumeIntegral(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType)
    : IntegralValue(fieldInfo, timeStep, adaptivityStep, solutionType)
{
    calculate();
}

void {{CLASS}}VolumeIntegral::calculate()
{
    m_values.clear();

    FieldSolutionID fsid(m_fieldInfo, m_timeStep, m_adaptivityStep, m_solutionType);
    MultiArray<double> ma = Agros2D::solutionStore()->multiArray(fsid);

    if (Agros2D::problem()->isSolved())
    {
        // update time functions
        if (m_fieldInfo->analysisType() == AnalysisType_Transient)
        {
            QList<double> timeLevels = Agros2D::solutionStore()->timeLevels(m_fieldInfo);
            Module::updateTimeFunctions(timeLevels[m_timeStep]);
        }

        double **value = new double*[m_fieldInfo->numberOfSolutions()];
        double **dudx = new double*[m_fieldInfo->numberOfSolutions()];
        double **dudy = new double*[m_fieldInfo->numberOfSolutions()];

        Hermes::Hermes2D::Quad2D *quad = &Hermes::Hermes2D::g_quad_2d_std;

        ma.solutions().at(0)->set_quad_2d(quad);

        const MeshSharedPtr mesh = ma.solutions().at(0)->get_mesh();
        Hermes::Hermes2D::Element *e;

        foreach (SceneLabel *label, Agros2D::scene()->labels->items())
        {
            if (label->isSelected())
            {
                int index = Agros2D::scene()->labels->items().indexOf(label);

                SceneMaterial *material = label->marker(m_fieldInfo);

                {{#VARIABLE_MATERIAL}}Value material_{{MATERIAL_VARIABLE}} = material->value("{{MATERIAL_VARIABLE}}");
                {{/VARIABLE_MATERIAL}}

                for_all_active_elements(e, mesh)
                {
                    if (m_fieldInfo->initialMesh()->get_element_markers_conversion().get_user_marker(e->marker).marker == QString::number(index).toStdString())
                    {
                        Hermes::Hermes2D::update_limit_table(e->get_mode());

                        for (int k = 0; k < m_fieldInfo->numberOfSolutions(); k++)
                            ma.solutions().at(k)->set_active_element(e);

                        Hermes::Hermes2D::RefMap *ru = ma.solutions().at(0)->get_refmap();

                        int o = 0;
                        for (int k = 0; k < m_fieldInfo->numberOfSolutions(); k++)
                            o += ma.solutions().at(k)->get_fn_order();
                        o += ru->get_inv_ref_order();

                        // coordinates
                        double *x = ru->get_phys_x(o);
                        double *y = ru->get_phys_y(o);

                        {
                            Hermes::Hermes2D::limit_order(o, e->get_mode());
                        }

                        // solution
                        for (int k = 0; k < m_fieldInfo->numberOfSolutions(); k++)
                        {
                            ma.solutions().at(k)->set_quad_order(o, Hermes::Hermes2D::H2D_FN_VAL | Hermes::Hermes2D::H2D_FN_DX | Hermes::Hermes2D::H2D_FN_DY);
                            // value
                            value[k] = ma.solutions().at(k)->get_fn_values();
                            // derivative
                            ma.solutions().at(k)->get_dx_dy_values(dudx[k], dudy[k]);
                        }
                        Hermes::Hermes2D::update_limit_table(e->get_mode());

                        double3* pt = quad->get_points(o, e->get_mode());
                        int np = quad->get_num_points(o, e->get_mode());
                        double* jac = ru->get_jacobian(o);

                        // expressions
                        {{#VARIABLE_SOURCE}}
                        if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
                                && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
                        {
                            double result = 0.0;
                            for (int i = 0; i < np; i++)
                            {
                                if (ru->is_jacobian_const())
                                    result += pt[i][2] * ru->get_const_jacobian() * ({{EXPRESSION}});
                                else
                                    result += pt[i][2] * jac[i] * ({{EXPRESSION}});
                            }
                            m_values["{{VARIABLE}}"] += result;
                        }
                        {{/VARIABLE_SOURCE}}
                    }
                }
            }
        }

        delete [] value;
        delete [] dudx;
        delete [] dudy;
    }
}
