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

#include "{{ID}}_errorcalculator.h"

#include "util.h"
#include "util/global.h"


#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/solutionstore.h"

#include "hermes2d/plugin_interface.h"

{{#CALCULATOR_SOURCE}}
template<typename Scalar>
class {{CLASS}}ErrorCalculatorNorm_{{COORDINATE_TYPE}}_{{LINEARITY_TYPE}}_{{ANALYSIS_TYPE}}_{{ID_CALCULATOR}} : public Hermes::Hermes2D::NormFormVol<Scalar>
{
public:
    {{CLASS}}ErrorCalculatorNorm_{{COORDINATE_TYPE}}_{{LINEARITY_TYPE}}_{{ANALYSIS_TYPE}}_{{ID_CALCULATOR}}<Scalar>(FieldInfo *fieldInfo, int i, int j)
        : Hermes::Hermes2D::NormFormVol<Scalar>(i, j), m_fieldInfo(fieldInfo) {}

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u, Hermes::Hermes2D::Func<Scalar> *v, Hermes::Hermes2D::Geom<double> *e) const
    {
        SceneLabel *label = Agros2D::scene()->labels->at(atoi(m_fieldInfo->initialMesh()->get_element_markers_conversion().get_user_marker(e->elem_marker).marker.c_str()));
        SceneMaterial *material = label->marker(m_fieldInfo);

        {{#VARIABLE_SOURCE}}
        const Value *{{VARIABLE_SHORT}} = material->valueNakedPtr(QLatin1String("{{VARIABLE}}"));{{/VARIABLE_SOURCE}}

        Scalar result = Scalar(0);
        for (int i = 0; i < n; i++)
            result += wt[i] * ({{EXPRESSION}});

        return result;
    }

    FieldInfo *m_fieldInfo;
};

template class {{CLASS}}ErrorCalculatorNorm_{{COORDINATE_TYPE}}_{{LINEARITY_TYPE}}_{{ANALYSIS_TYPE}}_{{ID_CALCULATOR}}<double>;
{{/CALCULATOR_SOURCE}}

template<typename Scalar>
{{CLASS}}ErrorCalculator<Scalar>::{{CLASS}}ErrorCalculator(FieldInfo *fieldInfo, const QString &calculator, Hermes::Hermes2D::CalculatedErrorType errorType)
    : Hermes::Hermes2D::ErrorCalculator<Scalar>(errorType), m_fieldInfo(fieldInfo), m_calculator(calculator)
{
    for(int i = 0; i < m_fieldInfo->numberOfSolutions(); i++)
    {
        {{#CALCULATOR_SOURCE}}
        if ((fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
                && (Agros2D::problem()->config()->coordinateType() == {{COORDINATE_TYPE}})
                && (fieldInfo->linearityType() == {{LINEARITY_TYPE}})
                && calculator == QLatin1String("{{ID_CALCULATOR}}"))
            this->add_error_form(new {{CLASS}}ErrorCalculatorNorm_{{COORDINATE_TYPE}}_{{LINEARITY_TYPE}}_{{ANALYSIS_TYPE}}_{{ID_CALCULATOR}}<Scalar>(fieldInfo, i, i));
        {{/CALCULATOR_SOURCE}}
    }
}

template class {{CLASS}}ErrorCalculator<double>;
