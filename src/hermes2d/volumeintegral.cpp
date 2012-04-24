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

VolumeIntegralValue::VolumeIntegralValue(FieldInfo *fieldInfo) : m_fieldInfo(fieldInfo)
{
    parser = new Parser(fieldInfo);
    initParser();

    // update time functions
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
    {
        QList<double> timeLevels = Util::solutionStore()->timeLevels(Util::scene()->activeViewField());
        m_fieldInfo->module()->update_time_functions(timeLevels[Util::scene()->activeTimeStep()]);
    }

    // solution
    for (int k = 0; k < m_fieldInfo->module()->number_of_solution(); k++)
    {
        FieldSolutionID fsid(m_fieldInfo, Util::scene()->activeTimeStep(), Util::scene()->activeAdaptivityStep(), Util::scene()->activeSolutionType());
        sln.push_back(Util::solutionStore()->multiSolution(fsid).component(k).sln.get());
    }

    calculate();
}

VolumeIntegralValue::~VolumeIntegralValue()
{
    delete parser;
}

void VolumeIntegralValue::initParser()
{
    for (Hermes::vector<Hermes::Module::Integral *>::iterator it = m_fieldInfo->module()->volume_integral.begin();
         it < m_fieldInfo->module()->volume_integral.end(); ++it )
    {
        mu::Parser *pars = m_fieldInfo->module()->get_parser(m_fieldInfo);

        pars->SetExpr(((Hermes::Module::Integral *) *it)->expression.scalar);

        parser->parser.push_back(pars);

        values[*it] = 0.0;
    }
}

void VolumeIntegralValue::calculate()
{
    if (Util::problem()->isSolved())
    {
        double px;
        double py;
        double *pvalue = new double[m_fieldInfo->module()->number_of_solution()];
        double *pdx = new double[m_fieldInfo->module()->number_of_solution()];
        double *pdy = new double[m_fieldInfo->module()->number_of_solution()];

        double **value = new double*[m_fieldInfo->module()->number_of_solution()];
        double **dudx = new double*[m_fieldInfo->module()->number_of_solution()];
        double **dudy = new double*[m_fieldInfo->module()->number_of_solution()];

        for (Hermes::vector<mu::Parser *>::iterator it = parser->parser.begin(); it < parser->parser.end(); ++it )
        {
            ((mu::Parser *) *it)->DefineVar(Util::problem()->config()->labelX().toLower().toStdString(), &px);
            ((mu::Parser *) *it)->DefineVar(Util::problem()->config()->labelY().toLower().toStdString(), &py);

            for (int k = 0; k < m_fieldInfo->module()->number_of_solution(); k++)
            {
                std::stringstream number;
                number << (k+1);

                ((mu::Parser *) *it)->DefineVar("value" + number.str(), &pvalue[k]);
                ((mu::Parser *) *it)->DefineVar("d" + Util::problem()->config()->labelX().toLower().toStdString() + number.str(), &pdx[k]);
                ((mu::Parser *) *it)->DefineVar("d" + Util::problem()->config()->labelY().toLower().toStdString() + number.str(), &pdy[k]);

            }

            parser->initParserMaterialVariables();
        }

        Hermes::Hermes2D::Quad2D *quad = &Hermes::Hermes2D::g_quad_2d_std;

        sln[0]->set_quad_2d(quad);

        Hermes::Hermes2D::Mesh *mesh = sln[0]->get_mesh();
        Hermes::Hermes2D::Element *e;

        foreach (SceneLabel *label, Util::scene()->labels->items())
        {
            if (label->isSelected)
            {
                SceneMaterial *material = label->getMarker(m_fieldInfo);
                int index = Util::scene()->labels->items().indexOf(label);

                parser->setParserVariables(material, NULL,
                                           pvalue[0], pdx[0], pdy[0]);

                for_all_active_elements(e, mesh)
                {
                    if (mesh->get_element_markers_conversion().get_user_marker(e->marker).marker == QString::number(index).toStdString())
                    {
                        Hermes::Hermes2D::update_limit_table(e->get_mode());

                        for (int k = 0; k < m_fieldInfo->module()->number_of_solution(); k++)
                            sln[k]->set_active_element(e);

                        Hermes::Hermes2D::RefMap *ru = sln[0]->get_refmap();

                        int o = 0;
                        for (int k = 0; k < m_fieldInfo->module()->number_of_solution(); k++)
                            o += sln[k]->get_fn_order();
                        o += ru->get_inv_ref_order();

                        // coordinates
                        double *x = ru->get_phys_x(o);
                        double *y = ru->get_phys_y(o);

                        {
                            using namespace Hermes::Hermes2D;
                            // TODO: HERMES_MODE_TRIANGLE
                            // TODO: PK for macro have to use "using namespace"...
                            limit_order(o, Hermes::Hermes2D::HERMES_MODE_TRIANGLE);
                        }

                        // solution
                        for (int k = 0; k < m_fieldInfo->module()->number_of_solution(); k++)
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
                        for (Hermes::vector<Hermes::Module::Integral *>::iterator it = m_fieldInfo->module()->volume_integral.begin();
                             it < m_fieldInfo->module()->volume_integral.end(); ++it )
                        {
                            double result = 0.0;

                            try
                            {
                                // TODO: HERMES_MODE_TRIANGLE
                                double3* pt = quad->get_points(o, Hermes::Hermes2D::HERMES_MODE_TRIANGLE);
                                // TODO: HERMES_MODE_TRIANGLE
                                int np = quad->get_num_points(o, Hermes::Hermes2D::HERMES_MODE_TRIANGLE);

                                for (int i = 0; i < np; i++)
                                {
                                    px = x[i];
                                    py = y[i];

                                    for (int k = 0; k < m_fieldInfo->module()->number_of_solution(); k++)
                                    {
                                        pvalue[k] = value[k][i];
                                        pdx[k] = dudx[k][i];
                                        pdy[k] = dudy[k][i];
                                    }

                                    if (ru->is_jacobian_const())
                                    {
                                        result += pt[i][2] * ru->get_const_jacobian() * parser->parser[n]->Eval();
                                    }
                                    else
                                    {
                                        double* jac = ru->get_jacobian(o);
                                        result += pt[i][2] * jac[i] * parser->parser[n]->Eval();
                                    }
                                }

                                values[*it] += result;
                            }
                            catch (mu::Parser::exception_type &e)
                            {
                                std::cout << "Volume integral: " << ((Hermes::Module::LocalVariable *) *it)->name <<
                                             " (" << ((Hermes::Module::LocalVariable *) *it)->id << ") " <<
                                             ((Hermes::Module::LocalVariable *) *it)->name << " - '" <<
                                             parser->parser[n]->GetExpr() << "' - " << e.GetMsg() << std::endl;
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
