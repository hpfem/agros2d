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

#ifndef LOCALPOINT_H
#define LOCALPOINT_H

#include "util.h"
#include "hermes2d.h"

class Parser;

class SceneMaterial;
class FieldInfo;

namespace Hermes
{
    namespace Module
    {
        struct LocalVariable;
    }
}

struct PointValue
{
    PointValue()
    {
        this->scalar = 0.0;
        this->vector = Point();
        this->material = NULL;
    }

    PointValue(double scalar, Point vector, SceneMaterial *material)
    {
        this->scalar = scalar;
        this->vector = vector;
        this->material = material;
    }

    double scalar;
    Point vector;
    SceneMaterial *material;
};

class LocalPointValue
{
public:
    Parser *parser;

    // point
    Point point;

    // variables
    std::map<Hermes::Module::LocalVariable *, PointValue> values;

    LocalPointValue(FieldInfo *fieldInfo, const Point &point);
    ~LocalPointValue();

    void initParser();
    void calculate();

private:
    FieldInfo *fieldInfo;
};

#endif // LOCALPOINT_H
