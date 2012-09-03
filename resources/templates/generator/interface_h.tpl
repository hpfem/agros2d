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

#ifndef {{ID}}_INTERFACE_H
#define {{ID}}_INTERFACE_H

#include <QObject>

#include "util.h"
#include "hermes2d.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

#include "hermes2d/marker.h"

#include "hermes2d/plugin_interface.h"

class {{CLASS}}Interface : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)

public:

    virtual ~{{CLASS}}Interface() {}

    inline virtual QString fieldId() { return "{{ID}}"; }

    // weakforms
    virtual Hermes::Hermes2D::MatrixFormVol<double> *matrixFormVol(const ProblemID problemId, int i, int j,                                                                  
                                                                   Material *materialSource, Material *materialTarget, int offsetI, int offsetJ );

    virtual Hermes::Hermes2D::VectorFormVol<double> *vectorFormVol(const ProblemID problemId, int i, int j,
                                                                   Material *materialSource, Material *materialTarget, int offsetI, int offsetJ);

    virtual Hermes::Hermes2D::MatrixFormSurf<double> *matrixFormSurf(const ProblemID problemId, int i, int j,
                                                                     Boundary *boundary, int offsetI, int offsetJ);

    virtual Hermes::Hermes2D::VectorFormSurf<double> *vectorFormSurf(const ProblemID problemId, int i, int j,
                                                                     Boundary *boundary, int offsetI, int offsetJ);

    virtual Hermes::Hermes2D::ExactSolutionScalar<double> *exactSolution(const ProblemID problemId, int i,Hermes::Hermes2D::Mesh *mesh, Boundary *boundary);

    // postprocessor

    // filter
    virtual Hermes::Hermes2D::Filter<double> *filter(FieldInfo *fieldInfo,
                                                     Hermes::vector<Hermes::Hermes2D::MeshFunction<double> *> sln,
                                                     const QString &variable,
                                                     PhysicFieldVariableComp physicFieldVariableComp);

    // local values
    virtual LocalValue *localValue(FieldInfo *fieldInfo, const Point &point);
    // surface integrals
    virtual IntegralValue *surfaceIntegral(FieldInfo *fieldInfo);
    // volume integrals
    virtual IntegralValue *volumeIntegral(FieldInfo *fieldInfo);

    // force calculation
    Point3 force(FieldInfo *fieldInfo, const Point3 &point, const Point3 &velocity);
};

#endif // {{ID}}_INTERFACE_H
