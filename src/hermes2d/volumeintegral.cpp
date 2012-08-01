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

#include "volumeintegral.h"
#include "scene.h"
#include "scenebasic.h"
#include "scenelabel.h"
#include "scenemarkerdialog.h"
#include "field.h"
#include "problem.h"

#include "hermes2d.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/solutionstore.h"

VolumeIntegralValue::VolumeIntegralValue(FieldInfo *fieldInfo) : PostprocessorIntegralValue(fieldInfo)
{
    // update time functions
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
    {
        QList<double> timeLevels = Util::solutionStore()->timeLevels(Util::scene()->activeViewField());
        m_fieldInfo->module()->updateTimeFunctions(timeLevels[Util::scene()->activeTimeStep()]);
    }

    // solution
    for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
    {
        // todo: do it better! - I could use reference solution. This way I ignore selected active adaptivity step and solution mode
        FieldSolutionID fsid(m_fieldInfo, Util::scene()->activeTimeStep(), Util::solutionStore()->lastAdaptiveStep(m_fieldInfo, SolutionMode_Normal, Util::scene()->activeTimeStep()), SolutionMode_Normal);
        sln.push_back(Util::solutionStore()->multiSolution(fsid).component(k).sln.data());
    }

    initParser(m_fieldInfo->module()->volumeIntegrals());

    calculate();
}

VolumeIntegralValue::~VolumeIntegralValue()
{
}

void VolumeIntegralValue::calculate()
{
    if (Util::problem()->isSolved())
    {
        double px;
        double py;
        double *pvalue = new double[m_fieldInfo->module()->numberOfSolutions()];
        double *pdx = new double[m_fieldInfo->module()->numberOfSolutions()];
        double *pdy = new double[m_fieldInfo->module()->numberOfSolutions()];

        double **value = new double*[m_fieldInfo->module()->numberOfSolutions()];
        double **dudx = new double*[m_fieldInfo->module()->numberOfSolutions()];
        double **dudy = new double*[m_fieldInfo->module()->numberOfSolutions()];

        foreach (mu::Parser *pars, m_parsers)
        {
            pars->DefineVar(Util::problem()->config()->labelX().toLower().toStdString(), &px);
            pars->DefineVar(Util::problem()->config()->labelY().toLower().toStdString(), &py);

            for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
            {
                std::stringstream number;
                number << (k+1);

                pars->DefineVar("value" + number.str(), &pvalue[k]);
                pars->DefineVar("d" + Util::problem()->config()->labelX().toLower().toStdString() + number.str(), &pdx[k]);
                pars->DefineVar("d" + Util::problem()->config()->labelY().toLower().toStdString() + number.str(), &pdy[k]);
            }
        }

        Hermes::Hermes2D::Quad2D *quad = &Hermes::Hermes2D::g_quad_2d_std;

        sln[0]->set_quad_2d(quad);

        const Hermes::Hermes2D::Mesh* mesh = sln[0]->get_mesh();
        Hermes::Hermes2D::Element *e;

        foreach (SceneLabel *label, Util::scene()->labels->items())
        {
            if (label->isSelected())
            {
                int index = Util::scene()->labels->items().indexOf(label);

                SceneMaterial *material = label->marker(m_fieldInfo);

                setMaterialToParsers(material);

                // set nonlinear parsers
                setNonlinearParsers();

                for_all_active_elements(e, mesh)
                {
                    if (Util::problem()->meshInitial(m_fieldInfo)->get_element_markers_conversion().get_user_marker(e->marker).marker == QString::number(index).toStdString())
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

                        // parse expression
                        int n = 0;
                        foreach (Module::Integral *integral, m_fieldInfo->module()->volumeIntegrals())
                        {
                            double result = 0.0;

                            try
                            {
                                double3* pt = quad->get_points(o, e->get_mode());
                                int np = quad->get_num_points(o, e->get_mode());

                                for (int i = 0; i < np; i++)
                                {
                                    px = x[i];
                                    py = y[i];

                                    for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
                                    {
                                        pvalue[k] = value[k][i];
                                        pdx[k] = dudx[k][i];
                                        pdy[k] = dudy[k][i];
                                    }

                                    // set nonlinear material
                                    setNonlinearMaterial(material);

                                    if (ru->is_jacobian_const())
                                    {
                                        result += pt[i][2] * ru->get_const_jacobian() * m_parsers[n]->Eval();
                                    }
                                    else
                                    {
                                        double* jac = ru->get_jacobian(o);
                                        result += pt[i][2] * jac[i] * m_parsers[n]->Eval();
                                    }
                                }

                                m_values[integral] += result;
                            }
                            catch (mu::Parser::exception_type &e)
                            {
                                qDebug() << "Volume integral: " << integral->name() << " (" << integral->id() << ") " << integral->name() << " - '" <<
                                             QString::fromStdString(m_parsers[n]->GetExpr()) << "' - " << QString::fromStdString(e.GetMsg());
                            }

                            n++;
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
