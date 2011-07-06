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

#include "boundary.h"
#include "module.h"

#include "util.h"

Boundary::Boundary(std::string name, std::string type)
{
    // name and type
    this->name = name;
    this->type = type;

    // set values
    if (type != "")
    {
        Hermes::Module::BoundaryType *boundary = Util::scene()->problemInfo()->module()->get_boundary_type(type);
        for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary->variables.begin(); it < boundary->variables.end(); ++it)
        {
            Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);
            values[variable] = Value("0");
        }
    }
}

Boundary::~Boundary()
{
    values.clear();
}

Hermes::Module::BoundaryTypeVariable *Boundary::get_boundary_type_variable(std::string id)
{
    for (std::map<Hermes::Module::BoundaryTypeVariable *, Value>::iterator it = values.begin(); it != values.end(); ++it)
    {
        if (it->first->id == id)
            return it->first;
    }
    return NULL;
}

Value Boundary::get_value(std::string id)
{
    Hermes::Module::BoundaryTypeVariable *variable = get_boundary_type_variable(id);

    if (variable)
        return values[variable];

    return Value();
}
