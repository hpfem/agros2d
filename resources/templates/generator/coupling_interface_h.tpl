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

#ifndef {{CLASS}}_INTERFACE_H
#define {{CLASS}}_INTERFACE_H

#include <QObject>

#include "util.h"
#include "hermes2d/plugin_interface.h"

class FieldInfo;
class Boundary;

class {{CLASS}}Interface : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)

public:
    {{CLASS}}Interface();
    virtual ~{{CLASS}}Interface();

    inline virtual QString fieldId() { return "{{ID}}"; }

    // weakforms
    virtual MatrixFormVolAgros<double> *matrixFormVol(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Material *material);
    virtual VectorFormVolAgros<double> *vectorFormVol(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Material *material);
    virtual MatrixFormSurfAgros<double> *matrixFormSurf(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Boundary *boundary) { assert(0); return NULL; }
    virtual VectorFormSurfAgros<double> *vectorFormSurf(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Boundary *boundary) { assert(0); return NULL; }

    virtual ExactSolutionScalarAgros<double> *exactSolution(const ProblemID problemId, FormInfo *form, MeshSharedPtr mesh) { assert(0); return NULL; }

    // postprocessor
    // filter
    virtual Hermes::Hermes2D::Filter<double> *filter(FieldInfo *fieldInfo,
                                                     int timeStep, int adaptivityStep, SolutionMode solutionType,
                                                     Hermes::vector<MeshFunctionSharedPtr<double> > sln,
                                                     const QString &variable,
                                                     PhysicFieldVariableComp physicFieldVariableComp) { assert(0); return NULL; }

    // local values
    virtual LocalValue *localValue(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType, const Point &point) { assert(0); return NULL; }
    // surface integrals
    virtual IntegralValue *surfaceIntegral(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType) { assert(0); return NULL; }
    // volume integrals
    virtual IntegralValue *volumeIntegral(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType) { assert(0); return NULL; }

    // force calculation
    virtual Point3 force(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                         Hermes::Hermes2D::Element *element, const SceneMaterial *material,
                         const Point3 &point, const Point3 &velocity) { assert(0); return Point3(); }
    virtual bool hasForce(FieldInfo *fieldInfo) { return false; }

    // localization
    virtual QString localeName(const QString &name) { assert(0); return NULL; }

    // description of module
    virtual QString localeDescription()  { assert(0); return NULL; }
};

#endif // {{CLASS}}_INTERFACE_H
