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

#include "{{ID}}_surfaceintegral.h"

#include "util.h"
#include "util/global.h"


#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/field.h"
#include "hermes2d/solutionstore.h"

#include "hermes2d/plugin_interface.h"


class {{CLASS}}SurfaceIntegralCalculator : public Hermes::Hermes2D::PostProcessing::SurfaceIntegralCalculator<double>
{
public:
    {{CLASS}}SurfaceIntegralCalculator(FieldInfo *fieldInfo, MeshFunctionSharedPtr<double> source_function, int number_of_integrals)
        : Hermes::Hermes2D::PostProcessing::SurfaceIntegralCalculator<double>(source_function, number_of_integrals), m_fieldInfo(fieldInfo)
    {
    }

    {{CLASS}}SurfaceIntegralCalculator(FieldInfo *fieldInfo, Hermes::vector<MeshFunctionSharedPtr<double> > source_functions, int number_of_integrals)
        : Hermes::Hermes2D::PostProcessing::SurfaceIntegralCalculator<double>(source_functions, number_of_integrals), m_fieldInfo(fieldInfo)
    {
    }

    virtual void integral(int n, double* wt, Hermes::Hermes2D::Func<double> **fns, Hermes::Hermes2D::Geom<double> *e, double* result)
    {
        SceneLabel *label = Agros2D::scene()->labels->at(atoi(m_fieldInfo->initialMesh()->get_element_markers_conversion().get_user_marker(e->elem_marker).marker.c_str()));
        SceneMaterial *material = label->marker(m_fieldInfo);

        double *x = e->x;
        double *y = e->y;

        {{#VARIABLE_MATERIAL}}Value *material_{{MATERIAL_VARIABLE}} = &material->value(QLatin1String("{{MATERIAL_VARIABLE}}"));
        {{/VARIABLE_MATERIAL}}

        // functions
        double **value = new double*[source_functions.size()];
        double **dudx = new double*[source_functions.size()];
        double **dudy = new double*[source_functions.size()];

        for (int i = 0; i < source_functions.size(); i++)
        {
            value[i] = fns[i]->val;
            dudx[i] = fns[i]->dx;
            dudy[i] = fns[i]->dy;
        }

        // expressions
        {{#VARIABLE_SOURCE}}
        if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
                && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
        {
            for (int i = 0; i < n; i++)
                result[{{POSITION}}] += wt[i] * ({{EXPRESSION}});
        }
        {{/VARIABLE_SOURCE}}

        delete [] value;
        delete [] dudx;
        delete [] dudy;
    }

    virtual void order(Hermes::Hermes2D::Func<Hermes::Ord> **fns, Hermes::Ord* result)
    {
        {{#VARIABLE_SOURCE}}result[{{POSITION}}] = Hermes::Ord(21);
        {{/VARIABLE_SOURCE}}
    }

private:
    // field info
    FieldInfo *m_fieldInfo;
};

{{CLASS}}SurfaceIntegral::{{CLASS}}SurfaceIntegral(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType)
    : IntegralValue(fieldInfo, timeStep, adaptivityStep, solutionType)
{
    calculate();
}

void {{CLASS}}SurfaceIntegral::calculate()
{
    m_values.clear();

    FieldSolutionID fsid(m_fieldInfo, m_timeStep, m_adaptivityStep, m_solutionType);
    MultiArray<double> ma = Agros2D::solutionStore()->multiArray(fsid);

    if (Agros2D::problem()->isSolved())
    {
        // update time functions
        if (!Agros2D::problem()->isSolving() && m_fieldInfo->analysisType() == AnalysisType_Transient)
        {
            QList<double> timeLevels = Agros2D::solutionStore()->timeLevels(m_fieldInfo);
            Module::updateTimeFunctions(timeLevels[m_timeStep]);
        }

        Hermes::vector<std::string> markers;
        for (int i = 0; i < Agros2D::scene()->edges->count(); i++)
        {
            SceneEdge *edge = Agros2D::scene()->edges->at(i);
            if (edge->isSelected())
            {
                markers.push_back(QString::number(i).toStdString());
            }
        }

        {{CLASS}}SurfaceIntegralCalculator calc(m_fieldInfo, ma.solutions(), {{INTEGRAL_COUNT}});
        double *values = calc.calculate(markers);

        {{#VARIABLE_SOURCE}}m_values[QLatin1String("{{VARIABLE}}")] = values[{{POSITION}}];
        {{/VARIABLE_SOURCE}}

        delete [] values;
    }
}


/*
void {{CLASS}}SurfaceIntegral::calculate()
{
    m_values.clear();

    FieldSolutionID fsid(m_fieldInfo, m_timeStep, m_adaptivityStep, m_solutionType);
    MultiArray<double> ma = Agros2D::solutionStore()->multiArray(fsid);

    if (Agros2D::problem()->isSolved())
    {
        // update time functions
        if (!Agros2D::problem()->isSolving() && m_fieldInfo->analysisType() == AnalysisType_Transient)
        {
            QList<double> timeLevels = Agros2D::solutionStore()->timeLevels(m_fieldInfo);
            Module::updateTimeFunctions(timeLevels[m_timeStep]);
        }

        double **value = new double*[m_fieldInfo->numberOfSolutions()];
        double **dudx = new double*[m_fieldInfo->numberOfSolutions()];
        double **dudy = new double*[m_fieldInfo->numberOfSolutions()];

        Hermes::Hermes2D::Element *e;
        Hermes::Hermes2D::Quad2D *quad = &Hermes::Hermes2D::g_quad_2d_std;

        for (int k = 0; k < m_fieldInfo->numberOfSolutions(); k++)
            ma.solutions().at(k)->set_quad_2d(quad);

        const MeshSharedPtr mesh = ma.solutions().at(0)->get_mesh();
        for_all_active_elements(e, mesh)
        {
            // set material variable
            SceneMaterial *material = Agros2D::scene()->labels->at(atoi(m_fieldInfo->initialMesh()->get_element_markers_conversion().
                                                                        get_user_marker(e->marker).marker.c_str()))->marker(m_fieldInfo);

            {{#VARIABLE_MATERIAL}}Value *material_{{MATERIAL_VARIABLE}} = &material->value(QLatin1String("{{MATERIAL_VARIABLE}}"));
            {{/VARIABLE_MATERIAL}}

            for (int i = 0; i<Agros2D::scene()->edges->length(); i++)
            {
                if (Agros2D::scene()->edges->at(i)->isSelected())
                {
                    for (unsigned edge = 0; edge < e->get_nvert(); edge++)
                    {
                        bool integrate = false;
                        bool boundary = false;

                        if (e->en[edge]->marker != -1)
                        {
                            if ((e->en[edge]->bnd == 1) && (!e->en[edge]->elem[1]))
                            {
                                boundary = true;
                            }
                            if ((atoi(m_fieldInfo->initialMesh()->get_boundary_markers_conversion().get_user_marker(e->en[edge]->marker).marker.c_str())) == i)
                            {
                                integrate = true;
                            }
                        }

                        // integral
                        if (integrate)
                        {
                            Hermes::Hermes2D::update_limit_table(e->get_mode());

                            int o = 0;
                            for (int k = 0; k < m_fieldInfo->numberOfSolutions(); k++)
                            {
                                o += ma.solutions().at(k)->get_fn_order();
                                ma.solutions().at(k)->set_active_element(e);
                            }

                            Hermes::Hermes2D::RefMap* ru = ma.solutions().at(0)->get_refmap();
                            o += ru->get_inv_ref_order();

                            Hermes::Hermes2D::Quad2D* quad = ru->get_quad_2d();
                            int eo = quad->get_edge_points(edge, o, e->get_mode());
                            double3 *pt = quad->get_points(eo, e->get_mode());
                            double3 *tan = ru->get_tangent(edge);

                            for (int k = 0; k < m_fieldInfo->numberOfSolutions(); k++)
                            {
                                ma.solutions().at(k)->set_quad_order(eo, Hermes::Hermes2D::H2D_FN_VAL | Hermes::Hermes2D::H2D_FN_DX | Hermes::Hermes2D::H2D_FN_DY);
                                // value
                                value[k] = ma.solutions().at(k)->get_fn_values();
                                // derivative
                                ma.solutions().at(k)->get_dx_dy_values(dudx[k], dudy[k]);
                            }

                            // x - coordinate
                            double *x = ru->get_phys_x(eo);
                            double *y = ru->get_phys_y(eo);

                            int np = quad->get_num_points(eo, e->get_mode());

                            // expressions
                            {{#VARIABLE_SOURCE}}
                            if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
                                    && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}}))
                            {
                                double result = 0.0;
                                for (int i = 0; i < np; i++)
                                    result += pt[i][2] * tan[i][2] * 0.5 * (boundary ? 1.0 : 0.5) * ({{EXPRESSION}});
                                m_values[QLatin1String("{{VARIABLE}}")] += result;
                            }
                            {{/VARIABLE_SOURCE}}
                        }
                    }
                }
            }
        }

        delete [] value;
        delete [] dudx;
        delete [] dudy;
    }
}
*/
