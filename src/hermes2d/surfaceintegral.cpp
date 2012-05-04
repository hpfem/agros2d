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

#include "scene.h"
#include "scenebasic.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarkerdialog.h"
#include "surfaceintegral.h"
#include "field.h"
#include "problem.h"

#include "hermes2d.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/solutionstore.h"

SurfaceIntegralValue::SurfaceIntegralValue(FieldInfo *fieldInfo) : m_fieldInfo(fieldInfo)
{
    parser = new Parser(fieldInfo);
    initParser();

    // update time functions
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
    {
        QList<double> timeLevels = Util::solutionStore()->timeLevels(Util::scene()->activeViewField());
        m_fieldInfo->module()->updateTimeFunctions(timeLevels[Util::scene()->activeTimeStep()]);
    }

    // solution
    for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
    {
        FieldSolutionID fsid(m_fieldInfo, Util::scene()->activeTimeStep(), Util::scene()->activeAdaptivityStep(), Util::scene()->activeSolutionType());
        sln.push_back(Util::solutionStore()->multiSolution(fsid).component(k).sln.get());
    }

    calculate();
}

SurfaceIntegralValue::~SurfaceIntegralValue()
{
    delete parser;
}

void SurfaceIntegralValue::initParser()
{
    mu::Parser *pars = m_fieldInfo->module()->expressionParser();

    foreach (Module::Integral *integral, m_fieldInfo->module()->surfaceIntegrals())
    {
        pars->SetExpr(integral->expr.scalar.toStdString());
        parser->parser.push_back(pars);

        values[integral] = 0.0;
    }
}

void SurfaceIntegralValue::calculate()
{
    if (Util::problem()->isSolved())
    {

        double px;
        double py;
        double ptanx;
        double ptany;
        double *pvalue = new double[m_fieldInfo->module()->numberOfSolutions()];
        double *pdx = new double[m_fieldInfo->module()->numberOfSolutions()];
        double *pdy = new double[m_fieldInfo->module()->numberOfSolutions()];

        double **value = new double*[m_fieldInfo->module()->numberOfSolutions()];
        double **dudx = new double*[m_fieldInfo->module()->numberOfSolutions()];
        double **dudy = new double*[m_fieldInfo->module()->numberOfSolutions()];

        foreach (mu::Parser *pars, parser->parser)
        {
            pars->DefineVar(Util::problem()->config()->labelX().toLower().toStdString(), &px);
            pars->DefineVar(Util::problem()->config()->labelY().toLower().toStdString(), &py);
            pars->DefineVar("tan" + Util::problem()->config()->labelX().toLower().toStdString(), &ptanx);
            pars->DefineVar("tan" + Util::problem()->config()->labelY().toLower().toStdString(), &ptany);

            for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
            {
                std::stringstream number;
                number << (k+1);

                pars->DefineVar("value" + number.str(), &pvalue[k]);
                pars->DefineVar("d" + Util::problem()->config()->labelX().toLower().toStdString() + number.str(), &pdx[k]);
                pars->DefineVar("d" + Util::problem()->config()->labelY().toLower().toStdString() + number.str(), &pdy[k]);
            }
        }

        Hermes::Hermes2D::Element *e;
        Hermes::Hermes2D::Quad2D *quad = &Hermes::Hermes2D::g_quad_2d_std;

        for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
            sln[k]->set_quad_2d(quad);

        Hermes::Hermes2D::Mesh* mesh = sln[0]->get_mesh();
        for (int i = 0; i<Util::scene()->edges->length(); i++)
        {
            if (Util::scene()->edges->at(i)->isSelected)
            {
                for_all_active_elements(e, mesh)
                {
                    for (unsigned edge = 0; edge < e->get_num_surf(); edge++)
                    {
                        bool integrate = false;
                        bool boundary = false;

                        if (e->en[edge]->marker != -1) //TODO ??????????
                        {
//                            if (e->en[edge]->bnd == 1 && (atoi(mesh->get_boundary_markers_conversion().get_user_marker(e->en[edge]->marker).marker.c_str())) == i)
//                            {
//                                // boundary
//                                integrate = true;
//                                boundary = true;
//                            }
//                            else if (- atoi(mesh->get_boundary_markers_conversion().get_user_marker(e->en[edge]->marker).marker.c_str()) == i)
//                            {
//                                // inner page
//                                integrate = true;
//                            }
                            if (e->en[edge]->bnd == 1)
                            {
                                boundary = true;
                            }
                            if ((atoi(mesh->get_boundary_markers_conversion().get_user_marker(e->en[edge]->marker).marker.c_str())) == i)
                            {
                                integrate = true;
                            }
                        }

                        // integral
                        if (integrate)
                        {
                            Hermes::Hermes2D::update_limit_table(e->get_mode());

                            int o = 0;
                            for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
                            {
                                o += sln[k]->get_fn_order();
                                sln[k]->set_active_element(e);
                            }

                            Hermes::Hermes2D::RefMap* ru = sln[0]->get_refmap();
                            o += ru->get_inv_ref_order();

                            Hermes::Hermes2D::Quad2D* quad2d = ru->get_quad_2d();
                            // TODO: HERMES_MODE_TRIANGLE
                            int eo = quad2d->get_edge_points(edge, o, Hermes::Hermes2D::HERMES_MODE_TRIANGLE);
                            // TODO: HERMES_MODE_TRIANGLE
                            double3 *pt = quad2d->get_points(eo, Hermes::Hermes2D::HERMES_MODE_TRIANGLE);
                            double3 *tan = ru->get_tangent(edge);

                            for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
                            {
                                sln[k]->set_quad_order(eo, Hermes::Hermes2D::H2D_FN_VAL | Hermes::Hermes2D::H2D_FN_DX | Hermes::Hermes2D::H2D_FN_DY);
                                // value
                                value[k] = sln[k]->get_fn_values();
                                // derivative
                                sln[k]->get_dx_dy_values(dudx[k], dudy[k]);
                            }

                            // x - coordinate
                            double *x = ru->get_phys_x(eo);
                            double *y = ru->get_phys_y(eo);

                            SceneMaterial *material = Util::scene()->labels->at(atoi(Util::problem()->meshInitial(m_fieldInfo)->get_element_markers_conversion().get_user_marker(e->marker).marker.c_str()))->getMarker(m_fieldInfo);
                            parser->initParserMaterialVariables();
                            parser->setParserVariables(material, NULL,
                                                       pvalue[0], pdx[0], pdy[0]);

                            // parse expression
                            int n = 0;
                            foreach (Module::Integral *integral, m_fieldInfo->module()->surfaceIntegrals())
                            {
                                double result = 0.0;

                                try
                                {
                                    // TODO: HERMES_MODE_TRIANGLE
                                    for (int i = 0; i < quad2d->get_num_points(eo, Hermes::Hermes2D::HERMES_MODE_TRIANGLE); i++)
                                    {
                                        px = x[i];
                                        py = y[i];
                                        ptanx = tan[i][0];
                                        ptany = tan[i][1];

                                        for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
                                        {
                                            pvalue[k] = value[k][i];
                                            pdx[k] = dudx[k][i];
                                            pdy[k] = dudy[k][i];
                                        }

                                        result += pt[i][2] * tan[i][2] * 0.5 * (boundary ? 1.0 : 0.5) * parser->parser[n]->Eval();
                                    }

                                    values[integral] += result;
                                }
                                catch (mu::Parser::exception_type &e)
                                {
                                    qDebug() << "Surface integral: " << integral->name << " (" << integral->id << ") " << integral->name << " - " <<
                                                 QString::fromStdString(parser->parser[n]->GetExpr()) << " - " << QString::fromStdString(e.GetMsg());
                                }

                                n++;
                            }
                        }
                    }
                }
            }
        }

        delete [] pvalue;
        delete [] pdx;
        delete [] pdy;

        delete [] value;
        delete [] dudx;
        delete [] dudy;
    }
}
