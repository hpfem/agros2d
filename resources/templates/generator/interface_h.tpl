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
#include <QString>

#include "util.h"
#include "hermes2d/plugin_interface.h"

class FieldInfo;
class Boundary;

class {{CLASS}}Interface : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.hpfem.agros2d.{{CLASS}}Interface" FILE "")
#endif

public:
    {{CLASS}}Interface();
    virtual ~{{CLASS}}Interface();

    inline virtual QString fieldId() { return "{{ID}}"; }

    // weakforms
    virtual MatrixFormVolAgros<double> *matrixFormVol(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Material *material);
    virtual VectorFormVolAgros<double> *vectorFormVol(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Material *material, int offsetPreviousTimeExt, int *offsetCouplingExt);
    virtual MatrixFormSurfAgros<double> *matrixFormSurf(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Boundary *boundary);
    virtual VectorFormSurfAgros<double> *vectorFormSurf(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Boundary *boundary);

    virtual ExactSolutionScalarAgros<double> *exactSolution(const ProblemID problemId, FormInfo *form, Hermes::Hermes2D::MeshSharedPtr mesh);

    virtual AgrosExtFunction *extFunction(const ProblemID problemId, QString id, bool derivative);

    // postprocessor
    // filter
    virtual Hermes::Hermes2D::MeshFunctionSharedPtr<double> filter(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                                                 Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<double> > sln,
                                                 const QString &variable,
                                                 PhysicFieldVariableComp physicFieldVariableComp);

    // error calculators
    virtual Hermes::Hermes2D::ErrorCalculator<double> *errorCalculator(FieldInfo *fieldInfo,
                                                                       const QString &calculator, Hermes::Hermes2D::CalculatedErrorType errorType);

    // local values
    virtual LocalValue *localValue(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType, const Point &point);
    // surface integrals
    virtual IntegralValue *surfaceIntegral(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType);
    // volume integrals
    virtual IntegralValue *volumeIntegral(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType);

    // force calculation
    virtual Point3 force(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                         Hermes::Hermes2D::Element *element, SceneMaterial *material,
                         const Point3 &point, const Point3 &velocity);
    virtual bool hasForce(FieldInfo *fieldInfo);


    // localization
    virtual QString localeName(const QString &name);

    // description of module
    virtual QString localeDescription();
};

// ***********************************************************************************************************************************
// Previous declaration of special functions

{{#SPECIAL_FUNCTION_SOURCE}}
template <typename Scalar>
class {{SPECIAL_FUNCTION_FULL_NAME}} : public SpecialFunction<Scalar>
{
public:
{{#PARAMETERS}}    double {{PARAMETER_NAME}};
{{/PARAMETERS}}
    virtual Scalar value(double h) const;
    virtual Scalar extrapolation_low();
    virtual Scalar extrapolation_hi();
};
{{/SPECIAL_FUNCTION_SOURCE}}

// ***********************************************************************************************************************************
// New declaration of special functions

{{#SPECIAL_FUNCTION_SOURCE}}
class {{SPECIAL_EXT_FUNCTION_FULL_NAME}} : public AgrosSpecialExtFunction
{
public:
    {{SPECIAL_EXT_FUNCTION_FULL_NAME}}(FieldInfo* fieldInfo);
    virtual double calculateValue(double h) const;
    virtual void value(int n, Hermes::Hermes2D::Func<double> **u_ext, Hermes::Hermes2D::Func<double> *result, Hermes::Hermes2D::Geom<double> *geometry) const;
private:
{{#PARAMETERS}}    mutable double {{PARAMETER_NAME}};
{{/PARAMETERS}}
    mutable double area;
};
{{/SPECIAL_FUNCTION_SOURCE}}

// ***********************************************************************************************************************************

{{CPP}}

#endif // {{ID}}_INTERFACE_H
