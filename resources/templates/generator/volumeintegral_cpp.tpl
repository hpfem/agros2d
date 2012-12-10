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

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/field.h"
#include "hermes2d/block.h"
#include "hermes2d/problem.h"
#include "hermes2d/solutionstore.h"

#include "util.h"
#include "util/global.h"

#include "scene.h"
#include "scenelabel.h"
#include "logview.h"

{{CLASS}}VolumeIntegral::{{CLASS}}VolumeIntegral(FieldInfo *fieldInfo)
    : IntegralValue(fieldInfo)
{
    calculate();
}

void {{CLASS}}VolumeIntegral::calculate()
{
    m_values.clear();

    if (Agros2D::problem()->isSolved())
    {
        // update time functions
        if (m_fieldInfo->analysisType() == AnalysisType_Transient)
        {
            QList<double> timeLevels = Agros2D::solutionStore()->timeLevels(Agros2D::scene()->activeViewField());
            m_fieldInfo->module()->updateTimeFunctions(timeLevels[Agros2D::scene()->activeTimeStep()]);
        }

        // solutions
        Hermes::vector<Hermes::Hermes2D::Solution<double> *> sln;
        for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
        {
            // todo: do it better! - I could use reference solution. This way I ignore selected active adaptivity step and solution mode
            FieldSolutionID fsid(m_fieldInfo, Agros2D::scene()->activeTimeStep(), Agros2D::solutionStore()->lastAdaptiveStep(m_fieldInfo, SolutionMode_Normal, Agros2D::scene()->activeTimeStep()), SolutionMode_Normal);
            sln.push_back(Agros2D::solutionStore()->multiSolution(fsid).component(k).sln.data());
        }

        double **value = new double*[m_fieldInfo->module()->numberOfSolutions()];
        double **dudx = new double*[m_fieldInfo->module()->numberOfSolutions()];
        double **dudy = new double*[m_fieldInfo->module()->numberOfSolutions()];

        Hermes::Hermes2D::Quad2D *quad = &Hermes::Hermes2D::g_quad_2d_std;

        sln[0]->set_quad_2d(quad);

        const Hermes::Hermes2D::Mesh* mesh = sln[0]->get_mesh();
        Hermes::Hermes2D::Element *e;

        foreach (SceneLabel *label, Agros2D::scene()->labels->items())
        {
            if (label->isSelected())
            {
                int index = Agros2D::scene()->labels->items().indexOf(label);

                SceneMaterial *material = label->marker(m_fieldInfo);

                for_all_active_elements(e, mesh)
                {
                    if (m_fieldInfo->initialMesh().data()->get_element_markers_conversion().get_user_marker(e->marker).marker == QString::number(index).toStdString())
                    {
                        Hermes::Hermes2D::update_limit_table(e->get_mode());

                        for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
                            sln[k]->set_active_element(e);

                        Hermes::Hermes2D::RefMap *ru = sln[0]->get_refmap();

                        int o = 0;
                        for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
                            o += sln[k]->get_fn_order();
                        o += ru->get_inv_ref_order();

                        // coordinates
                        double *x = ru->get_phys_x(o);
                        double *y = ru->get_phys_y(o);

                        {
                            Hermes::Hermes2D::limit_order(o, e->get_mode());
                        }

                        // solution
                        for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
                        {
                            sln[k]->set_quad_order(o, Hermes::Hermes2D::H2D_FN_VAL | Hermes::Hermes2D::H2D_FN_DX | Hermes::Hermes2D::H2D_FN_DY);
                            // value
                            value[k] = sln[k]->get_fn_values();
                            // derivative
                            sln[k]->get_dx_dy_values(dudx[k], dudy[k]);
                        }
                        Hermes::Hermes2D::update_limit_table(e->get_mode());

                        double3* pt = quad->get_points(o, e->get_mode());
                        int np = quad->get_num_points(o, e->get_mode());
                        double* jac = ru->get_jacobian(o);

                        // expressions
                        {{#VARIABLE_SOURCE}}
                        if ((m_fieldInfo->module()->analysisType() == {{ANALYSIS_TYPE}})
                                && (m_fieldInfo->module()->coordinateType() == {{COORDINATE_TYPE}}))
                        {
                            double result = 0.0;
                            for (int i = 0; i < np; i++)
                            {
                                if (ru->is_jacobian_const())
                                    result += pt[i][2] * ru->get_const_jacobian() * ({{EXPRESSION}});
                                else
                                    result += pt[i][2] * jac[i] * ({{EXPRESSION}});
                            }
                            m_values[m_fieldInfo->module()->volumeIntegral("{{VARIABLE}}")] += result;
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
