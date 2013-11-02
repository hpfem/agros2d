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

#ifndef {{ID}}_FORCE_H
#define {{ID}}_FORCE_H

#include <QObject>

#include "util.h"
#include "hermes2d/field.h"
#include "hermes2d.h"

bool hasForce{{CLASS}}(const FieldInfo *fieldInfo);

Point3 force{{CLASS}}(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                      Hermes::Hermes2D::Element *element, SceneMaterial *material, const Point3 &point, const Point3 &velocity = Point3());


#endif // {{ID}}_FORCE_H
