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

#ifndef {{CLASS}}_EXTFUNCTION_H
#define {{CLASS}}_EXTFUNCTION_H

#include "util.h"
#include <weakform/weakform.h>
#include "hermes2d/plugin_interface.h"
#include "hermes2d/marker.h"
#include "{{ID}}_interface.h"

{{#EXT_FUNCTION}}
class {{EXT_FUNCTION_NAME}} : public AgrosExtFunction
{
public:
    virtual void value (int n, Hermes::Hermes2D::Func<double>** u_ext, Hermes::Hermes2D::Func<double>* result, Hermes::Hermes2D::Geom<double>* geometry) const;
    {{EXT_FUNCTION_NAME}}(const FieldInfo* fieldInfo, const WeakFormAgros<double>* wfAgros);
    Hermes::Hermes2D::Function<double>* clone() const
    {
        return new {{EXT_FUNCTION_NAME}}(this->m_fieldInfo, this->m_wfAgros);
    }
protected:
    QList<QWeakPointer<Value> > {{QUANTITY_SHORTNAME}};
};
{{/EXT_FUNCTION}}

#endif // {{CLASS}}_EXTFUNCTION_H
