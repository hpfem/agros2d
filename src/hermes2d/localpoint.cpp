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

#include "localpoint.h"

#include "scene.h"
#include "hermes2d.h"

LocalPointValue::LocalPointValue(const Point &point) : point(point)
{
    parser = new Parser();
    initParser();

    for (std::map<std::string, double>::iterator it = parser->parser_variables.begin(); it != parser->parser_variables.end(); ++it)
        parser->parser[0]->DefineVar(it->first, &it->second);

    calculate();
}

LocalPointValue::~LocalPointValue()
{
    delete parser;
}

void LocalPointValue::initParser()
{
    // parser variables
    parser->parser.push_back(Util::scene()->problemInfo()->module()->get_parser());

    // init material variables
    parser->initParserMaterialVariables();
}

void LocalPointValue::calculate()
{
    values.clear();

    this->point = point;
    if (Util::scene()->sceneSolution()->isSolved() &&
            Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        Util::scene()->problemInfo()->module()->update_time_functions(Util::scene()->sceneSolution()->time());

    if (Util::scene()->sceneSolution()->isSolved())
    {
        int index = Util::scene()->sceneSolution()->findElementInMesh(Util::scene()->sceneSolution()->meshInitial(), point);
        if (index != -1)
        {
            // find marker
            Hermes::Hermes2D::Element *e = Util::scene()->sceneSolution()->meshInitial()->get_element_fast(index);
            SceneMaterial *tmpMaterial = Util::scene()->labels[atoi(Util::scene()->sceneSolution()->meshInitial()->get_element_markers_conversion().get_user_marker(e->marker).c_str())]->material;

            // set variables
            double px = point.x;
            double py = point.y;
            parser->parser[0]->DefineVar(Util::scene()->problemInfo()->labelX().toLower().toStdString(), &px);
            parser->parser[0]->DefineVar(Util::scene()->problemInfo()->labelY().toLower().toStdString(), &py);

            double *pvalue = new double[Util::scene()->problemInfo()->module()->number_of_solution()];
            double *pdx = new double[Util::scene()->problemInfo()->module()->number_of_solution()];
            double *pdy = new double[Util::scene()->problemInfo()->module()->number_of_solution()];
            std::vector<Hermes::Hermes2D::Solution<double> *> sln(Util::scene()->problemInfo()->module()->number_of_solution()); //TODO PK <double>

            for (int k = 0; k < Util::scene()->problemInfo()->module()->number_of_solution(); k++)
            {
                // solution
                sln[k] = Util::scene()->sceneSolution()->sln(k + (Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->module()->number_of_solution()));

                double value;
                if ((Util::scene()->problemInfo()->analysisType == AnalysisType_Transient) &&
                        Util::scene()->sceneSolution()->timeStep() == 0)
                    // const solution at first time step
                    value = Util::scene()->problemInfo()->initialCondition.number();
                else
                    value = sln[k]->get_pt_value(point.x, point.y, Hermes::Hermes2D::H2D_FN_VAL_0);

                Point derivative;
                derivative.x = sln[k]->get_pt_value(point.x, point.y, Hermes::Hermes2D::H2D_FN_DX_0);
                derivative.y = sln[k]->get_pt_value(point.x, point.y, Hermes::Hermes2D::H2D_FN_DY_0);

                // set variables
                pvalue[k] = value;
                pdx[k] = derivative.x;
                pdy[k] = derivative.y;

                std::stringstream number;
                number << (k+1);

                parser->parser[0]->DefineVar("value" + number.str(), &pvalue[k]);
                parser->parser[0]->DefineVar("d" + Util::scene()->problemInfo()->labelX().toLower().toStdString() + number.str(), &pdx[k]);
                parser->parser[0]->DefineVar("d" + Util::scene()->problemInfo()->labelY().toLower().toStdString() + number.str(), &pdy[k]);
            }

            // set material variables
            // FIXME
            parser->setParserVariables(tmpMaterial, NULL);

            // parse expression
            for (Hermes::vector<Hermes::Module::LocalVariable *>::iterator it = Util::scene()->problemInfo()->module()->local_point.begin();
                 it < Util::scene()->problemInfo()->module()->local_point.end(); ++it )
            {
                try
                {
                    PointValue pointValue;
                    if (((Hermes::Module::LocalVariable *) *it)->is_scalar)
                    {
                        parser->parser[0]->SetExpr(((Hermes::Module::LocalVariable *) *it)->expression.scalar);
                        pointValue.scalar = parser->parser[0]->Eval();
                    }
                    else
                    {
                        parser->parser[0]->SetExpr(((Hermes::Module::LocalVariable *) *it)->expression.comp_x);
                        pointValue.vector.x = parser->parser[0]->Eval();
                        parser->parser[0]->SetExpr(((Hermes::Module::LocalVariable *) *it)->expression.comp_y);
                        pointValue.vector.y = parser->parser[0]->Eval();
                    }
                    values[*it] = pointValue;

                }
                catch (mu::Parser::exception_type &e)
                {
                    std::cout << e.GetMsg() << endl;
                }
            }

            delete [] pvalue;
            delete [] pdx;
            delete [] pdy;
        }
    }
}
