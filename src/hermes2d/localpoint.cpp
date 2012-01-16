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
#include "scenebasic.h"
#include "scenelabel.h"
#include "scenemarkerdialog.h"
#include "scenesolution.h"
#include "hermes2d.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"

LocalPointValue::LocalPointValue(FieldInfo *fieldInfo, const Point &point) : m_fieldInfo(fieldInfo), point(point)
{
    parser = new Parser(fieldInfo);
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
    if (!m_fieldInfo)
        return;

    // parser variables
    parser->parser.push_back(m_fieldInfo->module()->get_parser(m_fieldInfo));

    // init material variables
    parser->initParserMaterialVariables();
}

void LocalPointValue::calculate()
{
    values.clear();

    this->point = point;
    SceneSolution<double>* sceneSolution = Util::scene()->sceneSolution(m_fieldInfo);
    if (Util::problem()->isSolved() &&
            m_fieldInfo->analysisType() == AnalysisType_Transient)
        m_fieldInfo->module()->update_time_functions(Util::problem()->time());

    if (Util::problem()->isSolved())
    {
        int index = sceneSolution->findElementInMesh(Util::problem()->meshInitial(), point);
        if (index != -1)
        {
            // find marker
            Hermes::Hermes2D::Element *e = Util::problem()->meshInitial()->get_element_fast(index);
            int labelIndex = atoi(Util::problem()->meshInitial()->get_element_markers_conversion().get_user_marker(e->marker).marker.c_str());
            SceneMaterial *tmpMaterial = Util::scene()->labels->at(labelIndex)->getMarker("TODO");

            // set variables
            double px = point.x;
            double py = point.y;
            parser->parser[0]->DefineVar(Util::scene()->problemInfo()->labelX().toLower().toStdString(), &px);
            parser->parser[0]->DefineVar(Util::scene()->problemInfo()->labelY().toLower().toStdString(), &py);

            double *pvalue = new double[m_fieldInfo->module()->number_of_solution()];
            double *pdx = new double[m_fieldInfo->module()->number_of_solution()];
            double *pdy = new double[m_fieldInfo->module()->number_of_solution()];
            std::vector<Hermes::Hermes2D::Solution<double> *> sln(m_fieldInfo->module()->number_of_solution()); //TODO PK <double>

            for (int k = 0; k < m_fieldInfo->module()->number_of_solution(); k++)
            {
                // solution
                sln[k] = sceneSolution->sln(k + (Util::problem()->timeStep() * m_fieldInfo->module()->number_of_solution()));

                double value;
                if ((m_fieldInfo->analysisType() == AnalysisType_Transient) &&
                        Util::problem()->timeStep() == 0)
                    // const solution at first time step
                    value = m_fieldInfo->initialCondition.number();
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
            parser->setParserVariables(tmpMaterial, NULL, pvalue[0], pdx[0], pdy[0]);

            // parse expression
            for (Hermes::vector<Hermes::Module::LocalVariable *>::iterator it = m_fieldInfo->module()->local_point.begin();
                 it < m_fieldInfo->module()->local_point.end(); ++it )
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
                    std::cout << "Local value: " << ((Hermes::Module::LocalVariable *) *it)->name <<
                                 " (" << ((Hermes::Module::LocalVariable *) *it)->id << ") " <<
                                 ((Hermes::Module::LocalVariable *) *it)->name << " - " <<
                                 parser->parser[0]->GetExpr() << " - " << e.GetMsg() << std::endl;
                }
            }

            delete [] pvalue;
            delete [] pdx;
            delete [] pdy;
        }
    }
}
