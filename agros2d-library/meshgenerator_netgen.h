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

#ifndef MESHGENERATOR_NETGEN_H
#define MESHGENERATOR_NETGEN_H

#include "util.h"
#include "meshgenerator.h"

#include "nglib.h"
#include "meshing.hpp"
#include "basegeom.hpp"
#include "meshclass.hpp"
#include "geometry2d.hpp"

class SplineGeometry2dAgros : public netgen::SplineGeometry2d
{
public:
    SplineGeometry2dAgros() : netgen::SplineGeometry2d()
    {
        // global grading factor
        elto0 = 1;
    }

    void AppendMaterial(int index, double mx, bool qm)
    {
      const char *name = QString::number(index).toStdString().c_str();

      char *material = new char[100];
      strcpy(material, name);

      materials.Append(material);
      maxh.Append(mx);
      quadmeshing.Append(qm);
      tensormeshing.Append(false);
      layer.Append(1);
      bcnames.Append(NULL);
    }

    int GetNMaterial() const
    {
        return materials.Size();
    }
};

class MeshGeneratorNetgen : public MeshGenerator
{
    Q_OBJECT

private slots:
    bool writeToNetgen();
    bool readNetgenMeshFormat();

public:
    MeshGeneratorNetgen();

    virtual bool mesh();

private:
    SplineGeometry2dAgros *geom;
};


#endif //MESHGENERATOR_NETGEN_H
