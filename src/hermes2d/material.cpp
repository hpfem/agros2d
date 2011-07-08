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

#include "material.h"
#include "module.h"

#include "util.h"

Material::Material(std::string name)
{
    // name and type
    this->name = name;

    // set values
    if (name != "none")
    {
        Hermes::vector<Hermes::Module::MaterialType *> materials = Util::scene()->problemInfo()->module()->material_types;
        for (Hermes::vector<Hermes::Module::MaterialType *>::iterator it = materials.begin(); it < materials.end(); ++it)
        {
            Hermes::Module::MaterialType *material_type = ((Hermes::Module::MaterialType *) *it);
            values[material_type] = Value("0");
        }
    }
}

Material::~Material()
{
    values.clear();
}

Hermes::Module::MaterialType *Material::get_material_type(std::string id)
{
    for (std::map<Hermes::Module::MaterialType *, Value>::iterator it = values.begin(); it != values.end(); ++it)
    {
        if (it->first->id == id)
            return it->first;
    }
    return NULL;
}

Value Material::get_value(std::string id)
{
    Hermes::Module::MaterialType *material_type = get_material_type(id);

    if (material_type)
        return values[material_type];

    return Value();
}
