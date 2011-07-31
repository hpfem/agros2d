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

#include "surfaceintegral.h"

#include "scene.h"
#include "hermes2d.h"

SurfaceIntegralValue::SurfaceIntegralValue()
{
    parser = new Parser();
    initParser();

    calculate();
}

SurfaceIntegralValue::~SurfaceIntegralValue()
{
    delete parser;
}

void SurfaceIntegralValue::initParser()
{
    for (Hermes::vector<Hermes::Module::Integral *>::iterator it = Util::scene()->problemInfo()->module()->surface_integral.begin();
         it < Util::scene()->problemInfo()->module()->surface_integral.end(); ++it )
    {
        mu::Parser *pars = Util::scene()->problemInfo()->module()->get_parser();

        pars->SetExpr(((Hermes::Module::Integral *) *it)->expression.scalar);

        parser->parser.push_back(pars);

        values[*it] = 0.0;
    }
}

void SurfaceIntegralValue::calculate()
{
    logMessage("SurfaceIntegralValue::calculate()");

    if (!Util::scene()->sceneSolution()->isSolved())
        return;

    double px;
    double py;
    double ptanx;
    double ptany;
    double *pvalue = new double[Util::scene()->problemInfo()->module()->number_of_solution()];
    double *pdx = new double[Util::scene()->problemInfo()->module()->number_of_solution()];
    double *pdy = new double[Util::scene()->problemInfo()->module()->number_of_solution()];

    double **value = new double*[Util::scene()->problemInfo()->module()->number_of_solution()];
    double **dudx = new double*[Util::scene()->problemInfo()->module()->number_of_solution()];
    double **dudy = new double*[Util::scene()->problemInfo()->module()->number_of_solution()];

    for (Hermes::vector<mu::Parser *>::iterator it = parser->parser.begin(); it < parser->parser.end(); ++it )
    {
        ((mu::Parser *) *it)->DefineVar("x", &px);
        ((mu::Parser *) *it)->DefineVar("y", &py);
        ((mu::Parser *) *it)->DefineVar("tanx", &ptanx);
        ((mu::Parser *) *it)->DefineVar("tany", &ptany);

        for (int k = 0; k < Util::scene()->problemInfo()->module()->number_of_solution(); k++)
        {
            std::stringstream number;
            number << (k+1);

            ((mu::Parser *) *it)->DefineVar("value" + number.str(), &pvalue[k]);
            ((mu::Parser *) *it)->DefineVar("dx" + number.str(), &pdx[k]);
            ((mu::Parser *) *it)->DefineVar("dy" + number.str(), &pdy[k]);
        }
    }

    Hermes::vector<Hermes::Hermes2D::Solution<double> *>sln; //TODO PK <double>

    Hermes::Hermes2D::Element *e;

    Hermes::Hermes2D::Quad2D *quad = &Hermes::Hermes2D::g_quad_2d_std;

    for (int k = 0; k < Util::scene()->problemInfo()->module()->number_of_solution(); k++)
    {
        sln.push_back(Util::scene()->sceneSolution()->sln(k + (Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->module()->number_of_solution())));

        sln[k]->set_quad_2d(quad);
    }

    Hermes::Hermes2D::Mesh* mesh = sln[0]->get_mesh();
    for (int i = 0; i<Util::scene()->edges.length(); i++)
    {
        SceneEdge *sceneEdge = Util::scene()->edges[i];
        if (sceneEdge->isSelected)
        {
            for_all_active_elements(e, mesh)
            {
                for (unsigned edge = 0; edge < e->nvert; edge++)
                {
                    bool integrate = false;
                    bool boundary = false;

                    if (e->en[edge]->marker != 0)
                    {
                        if (e->en[edge]->bnd == 1 && (atoi(mesh->get_boundary_markers_conversion().get_user_marker(e->en[edge]->marker).c_str())) - 1 == i)
                        {
                            // boundary
                            integrate = true;
                            boundary = true;
                        }
                        else if (- atoi(mesh->get_boundary_markers_conversion().get_user_marker(e->en[edge]->marker).c_str()) == i)
                        {
                            // inner page
                            integrate = true;
                        }
                    }

                    // integral
                    if (integrate)
                    {
                        Hermes::Hermes2D::update_limit_table(e->get_mode());

                        for (int k = 0; k < Util::scene()->problemInfo()->module()->number_of_solution(); k++)
                            sln[k]->set_active_element(e);

                        Hermes::Hermes2D::RefMap* ru = sln[0]->get_refmap();

                        Hermes::Hermes2D::Quad2D* quad2d = ru->get_quad_2d();
                        int eo = quad2d->get_edge_points(edge);
                        double3 *pt = quad2d->get_points(eo);
                        double3 *tan = ru->get_tangent(edge);

                        for (int k = 0; k < Util::scene()->problemInfo()->module()->number_of_solution(); k++)
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

                        SceneMaterial *material = Util::scene()->labels[atoi(Util::scene()->sceneSolution()->meshInitial()->get_element_markers_conversion().get_user_marker(e->marker).c_str())]->material;
                        parser->initParserMaterialVariables();
                        parser->setParserVariables(material, NULL);

                        // parse expression
                        int n = 0;
                        for (Hermes::vector<Hermes::Module::Integral *>::iterator it = Util::scene()->problemInfo()->module()->surface_integral.begin();
                             it < Util::scene()->problemInfo()->module()->surface_integral.end(); ++it )
                        {
                            double result = 0.0;

                            try
                            {
                                for (int i = 0; i < quad2d->get_num_points(eo); i++)
                                {
                                    px = x[i];
                                    py = y[i];
                                    ptanx = tan[i][0];
                                    ptany = tan[i][1];

                                    for (int k = 0; k < Util::scene()->problemInfo()->module()->number_of_solution(); k++)
                                    {
                                        pvalue[k] = value[k][i];
                                        pdx[k] = dudx[k][i];
                                        pdy[k] = dudy[k][i];
                                    }

                                    result += pt[i][2] * tan[i][2] * 0.5 * (boundary ? 1.0 : 0.5) * parser->parser[n]->Eval();
                                }

                                values[*it] += result;
                            }
                            catch (mu::Parser::exception_type &e)
                            {
                                std::cout << e.GetMsg() << endl;
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
