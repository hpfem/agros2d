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

#include "{{ID}}_extfunction.h"

#include "util.h"
#include "util/global.h"

#include "scene.h"
#include "hermes2d.h"
#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/bdf2.h"
 		
{{#EXT_FUNCTION}}
{{EXT_FUNCTION_NAME}}::{{EXT_FUNCTION_NAME}}(const FieldInfo* fieldInfo, const WeakFormAgros<double>* wfAgros) : AgrosExtFunction(fieldInfo, wfAgros)
{
    {{QUANTITY_SHORTNAME}} = m_fieldInfo->valuePointerTable("{{QUANTITY_ID}}");
}

void {{EXT_FUNCTION_NAME}}::value (int n, Hermes::Hermes2D::Func<double>** u_ext, Hermes::Hermes2D::Func<double>* result, Hermes::Hermes2D::Geom<double>* e) const
{
    int labelIndex = m_fieldInfo->hermesMarkerToAgrosLabel(e->elem_marker);
    if(labelIndex == LABEL_OUTSIDE_FIELD)
    {
        for(int i = 0; i < n; i++)
        {
            result->val[i] = 0;
        }
        return;
    }
    assert((labelIndex >= 0) && (labelIndex < {{QUANTITY_SHORTNAME}}.size()));
    const Value* value = {{QUANTITY_SHORTNAME}}[labelIndex].data();
    Offset offset = this->m_wfAgros->offsetInfo(nullptr, this->m_fieldInfo);

    for(int i = 0; i < n; i++)
    {
        result->val[i] = value->{{VALUE_METHOD}}({{DEPENDENCE}});
    }
}
{{/EXT_FUNCTION}}

// ***********************************************************************************************************************************

{{#VALUE_FUNCTION_SOURCE}}
{{VALUE_FUNCTION_FULL_NAME}}::{{VALUE_FUNCTION_FULL_NAME}}(const FieldInfo* fieldInfo, const WeakFormAgros<double>* wfAgros) : AgrosExtFunction(fieldInfo, wfAgros)
{
{{#PARAMETERS_LINEAR}}    {{PARAMETER_NAME}}_pointers = m_fieldInfo->valuePointerTable("{{PARAMETER_ID}}");
{{/PARAMETERS_LINEAR}}
{{#PARAMETERS_NONLINEAR}}    {{PARAMETER_NAME}}_pointers = m_fieldInfo->valuePointerTable("{{PARAMETER_ID}}");
{{/PARAMETERS_NONLINEAR}}
}

double {{VALUE_FUNCTION_FULL_NAME}}::getValue(int hermesMarker, double h) const
{
    int labelIndex = m_fieldInfo->hermesMarkerToAgrosLabel(hermesMarker);
    if(labelIndex == LABEL_OUTSIDE_FIELD)
    {
        return 0.;
    }

{{#PARAMETERS_LINEAR}}    double {{PARAMETER_NAME}} = {{PARAMETER_NAME}}_pointers[labelIndex].data()->numberFromTable(h);
{{/PARAMETERS_LINEAR}}
{{#PARAMETERS_NONLINEAR}}    double {{PARAMETER_NAME}} = {{PARAMETER_NAME}}_pointers[labelIndex].data()->numberFromTable(h);
{{/PARAMETERS_NONLINEAR}}
    double area = m_fieldInfo->labelArea(labelIndex);

    return {{EXPR}};
}

void {{VALUE_FUNCTION_FULL_NAME}}::value (int n, Hermes::Hermes2D::Func<double>** u_ext, Hermes::Hermes2D::Func<double>* result, Hermes::Hermes2D::Geom<double>* e) const
{
    const int fieldID = this->m_fieldInfo->numberId();
    Offset offset = this->m_wfAgros->offsetInfo(nullptr, this->m_fieldInfo);

    int labelIndex = m_fieldInfo->hermesMarkerToAgrosLabel(e->elem_marker);
    if(labelIndex == LABEL_OUTSIDE_FIELD)
    {
        for(int i = 0; i < n; i++)
        {
            result->val[i] = 0;
        }
        return;
    }

{{#PARAMETERS_LINEAR}}    const Value* {{PARAMETER_NAME}}_value = {{PARAMETER_NAME}}_pointers[labelIndex].data();

{{/PARAMETERS_LINEAR}}
{{#PARAMETERS_NONLINEAR}}    const Value* {{PARAMETER_NAME}}_value = {{PARAMETER_NAME}}_pointers[labelIndex].data();
{{/PARAMETERS_NONLINEAR}}
{{#PARAMETERS_LINEAR}}    double {{PARAMETER_NAME}} = {{PARAMETER_NAME}}_value->number();
{{/PARAMETERS_LINEAR}}
    double area = m_fieldInfo->labelArea(labelIndex);

    for(int i = 0; i < n; i++)
    {
        double h = {{DEPENDENCE}};

{{#PARAMETERS_NONLINEAR}}        double {{PARAMETER_NAME}} = {{PARAMETER_NAME}}_value->numberFromTable(h);
{{/PARAMETERS_NONLINEAR}}
        result->val[i] = {{EXPR}};
    }
}
{{/VALUE_FUNCTION_SOURCE}}

// ***********************************************************************************************************************************


{{#SPECIAL_FUNCTION_SOURCE}}
{{SPECIAL_EXT_FUNCTION_FULL_NAME}}::{{SPECIAL_EXT_FUNCTION_FULL_NAME}}(const FieldInfo* fieldInfo, const WeakFormAgros<double>* wfAgros) : AgrosSpecialExtFunction(fieldInfo, wfAgros, specialFunctionTypeFromStringKey("{{TYPE}}"), {{INTERPOLATION_COUNT}})
{
    m_boundLow = {{FROM}};
    m_boundHi = {{TO}};
    m_variant = "{{SELECTED_VARIANT}}";

{{#PARAMETERS}}    {{PARAMETER_NAME}}_pointers = m_fieldInfo->valuePointerTable("{{PARAMETER_ID}}");
{{/PARAMETERS}}
    init();
}

{{SPECIAL_EXT_FUNCTION_FULL_NAME}}::~{{SPECIAL_EXT_FUNCTION_FULL_NAME}}()
{
}

double {{SPECIAL_EXT_FUNCTION_FULL_NAME}}::calculateValue(int hermesMarker, double h) const
{
    int labelIndex = m_fieldInfo->hermesMarkerToAgrosLabel(hermesMarker);

{{#PARAMETERS}}    double {{PARAMETER_NAME}} = {{PARAMETER_NAME}}_pointers[m_fieldInfo->hermesMarkerToAgrosLabel(hermesMarker)].data()->numberFromTable(h);
{{/PARAMETERS}}
    double area = m_fieldInfo->labelArea(labelIndex);
    if(m_useTable)
    {
        if(h < m_boundLow)
            return {{EXTRAPOLATE_LOW}};
        if(h > m_boundHi)
            return {{EXTRAPOLATE_HI}};
    }

    //if(0)
    //{}
    //{{#VARIANT}}else if (this->m_variant == QString("{{ID}}"))
        return {{EXPR}};{{/VARIANT}}
    assert(0);
}

void {{SPECIAL_EXT_FUNCTION_FULL_NAME}}::value(int n, Hermes::Hermes2D::Func<double> **u_ext, Hermes::Hermes2D::Func<double> *result, Hermes::Hermes2D::Geom<double> *e) const
{
    const int fieldID = this->m_fieldInfo->numberId();
    Offset offset = this->m_wfAgros->offsetInfo(nullptr, this->m_fieldInfo);

    for(int i = 0; i < n; i++)
    {
        result->val[i] = getValue(e->elem_marker, {{DEPENDENCE}});
    }
}

{{/SPECIAL_FUNCTION_SOURCE}}

