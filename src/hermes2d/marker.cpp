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

#include "marker.h"
#include "module.h"
#include "module_agros.h"
#include "scene.h"
#include "util.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"

Marker::Marker(FieldInfo *fieldInfo, QString name)
    : fieldInfo(fieldInfo), name(name)
{
    m_isNone = false;
}

Marker::~Marker()
{
    values.clear();
}

Value Marker::getValue(QString id)
{
    if (!id.isEmpty())
        return values[id];

    return Value();
}

const map<QString, Value> Marker::getValues() const
{
    return values;
}

void Marker::evaluate(QString id, double time)
{
    values[id].evaluate(time);
}

bool Marker::evaluateAllVariables()
{
    for (std::map<QString, Value>::iterator it = values.begin(); it != values.end(); ++it)
    {
        if (!it->second.evaluate())
            return false;
    }
    return true;
}

QString Marker::fieldId()
{
    return fieldInfo->fieldId();
}

Boundary::Boundary(FieldInfo *fieldInfo, QString name, QString type,
                   std::map<QString, Value> values) : Marker(fieldInfo, name)
{
    qDebug() << "boundary constructor, name: " << name << ", type: " << type;
    // name and type
    setType(type);
    this->values = values;

    // set values
    if (name != "none")
    {
        if (this->values.size() == 0)
        {
            Hermes::Module::BoundaryType *boundary_type = fieldInfo->module()->get_boundary_type(type);
            foreach (Hermes::Module::BoundaryTypeVariable *variable, boundary_type->variables)
                this->values[variable->id] = Value(QString::number(variable->default_value));
        }
    }
}



Material::Material(FieldInfo *fieldInfo, QString name,
                   std::map<QString, Value> values) : Marker(fieldInfo, name)
{
    // name and type
    this->values = values;

    // set values
    if (name != "none")
    {
        if (this->values.size() == 0)
        {
            foreach (Hermes::Module::MaterialTypeVariable *variable, fieldInfo->module()->material_type_variables)
                this->values[variable->id] = Value(QString::number(variable->default_value));
        }
    }
}
