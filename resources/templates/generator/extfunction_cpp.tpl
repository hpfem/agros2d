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
    Offset offset = this->m_wfAgros->offsetInfo(this->m_fieldInfo, nullptr);

    for(int i = 0; i < n; i++)
    {
        result->val[i] = value->{{VALUE_METHOD}}({{DEPENDENCE}});
    }
}
{{/EXT_FUNCTION}}
