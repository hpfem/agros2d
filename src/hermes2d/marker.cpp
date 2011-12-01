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

Marker::Marker(std::string field, std::string name)
{
    this->field = field;
    this->name = name;
}

Marker::~Marker()
{
    values.clear();
}

const Value Marker::getValue(std::string id)
{
    if (id != "")
        return values[id];

    return Value();
}

const map<string, Value> Marker::getValues() const
{
    return values;
}

void Marker::evaluate(std::string id, double time)
{
    values[id].evaluate(time);
}


Boundary::Boundary(std::string field, std::string name, std::string type,
                   std::map<std::string, Value> values) : Marker(field, name)
{
    // name and type
    setType(type);
    this->values = values;

    // set values
    if (name != "none")
    {
        if (this->values.size() == 0)
        {
            Hermes::Module::BoundaryType *boundary_type = Util::scene()->fieldInfo("TODO")->module()->get_boundary_type(type);
            for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin(); it < boundary_type->variables.end(); ++it)
            {
                Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);
                this->values[variable->id] = Value(QString::number(variable->default_value));
            }
        }
    }
}



Material::Material(std::string field, std::string name,
                   std::map<std::string, Value> values) : Marker(field, name)
{
    // name and type
    this->values = values;

    // set values
    if (name != "none")
    {
        if (this->values.size() == 0)
        {
            Hermes::vector<Hermes::Module::MaterialTypeVariable *> materials = Util::scene()->fieldInfo(field)->module()->material_type_variables;
            for (Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = materials.begin(); it < materials.end(); ++it)
            {
                Hermes::Module::MaterialTypeVariable *variable = ((Hermes::Module::MaterialTypeVariable *) *it);
                this->values[variable->id] = Value(QString::number(variable->default_value));
            }
        }
    }
}
