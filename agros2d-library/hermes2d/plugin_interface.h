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

#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include <QtPlugin>

#include "util.h"
#include "hermes2d.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

#include "hermes2d/module.h"
#include "hermes2d/marker.h"
#include "hermes2d/field.h"

#include "../../resources_source/classes/coupling_xml.h"

//template <typename Scalar>
class AGROS_LIBRARY_API AgrosExtFunction : public Hermes::Hermes2D::UExtFunction<double>
{
public:
    AgrosExtFunction(FieldInfo* fieldInfo, int offsetI) : UExtFunction(), m_fieldInfo(fieldInfo), m_offsetI(offsetI), m_valuesPointers(nullptr){}
    ~AgrosExtFunction();
    // todo: this is dangerous. Order should be determined from the type of ExtFunction
    // for consants should be 0, for nonlinearities more. Hom much?
    // todo:
    // todo:
    virtual void ord(Hermes::Hermes2D::Func<Hermes::Ord>** u_ext, Hermes::Hermes2D::Func<Hermes::Ord>* result) const
    {
        result->val[0] = Hermes::Ord(1);
    }

    virtual void init() {}

protected:
    Value** m_valuesPointers;
    FieldInfo* m_fieldInfo;
    int m_offsetI;
};

// used for quantities, that are not present in a given particular analysis
// something has to be pushed to weakform, since the individual forms expect ext[i] with indices i
// taken from general list of quantities in volume, not from lists in particular analysis
// (the reason is that this way we can generate only 1 form which can be used for all analyses, where appropriate
class AGROS_LIBRARY_API AgrosEmptyExtFunction : public AgrosExtFunction
{
public:
    AgrosEmptyExtFunction() : AgrosExtFunction(NULL, 0) {}
    virtual void value(int n, Hermes::Hermes2D::Func<double> **u_ext, Hermes::Hermes2D::Func<double> *result, Hermes::Hermes2D::Geom<double> *geometry) const
    {
        // result values are not initialized, but they should never be used.
        // this is not very safe, but done from efficiency reasons
    }
};

class AGROS_LIBRARY_API AgrosSpecialExtFunctionOneMaterial
{
public:
    AgrosSpecialExtFunctionOneMaterial() : m_constantValue(-123456), m_extrapolationLow(-123456), m_extrapolationHi(-123456), m_isValid(false) {}
    AgrosSpecialExtFunctionOneMaterial(QSharedPointer<DataTable> dataTable, double constantValue, double extrapolationLow, double extrapolationHi) :
        m_dataTable(dataTable), m_constantValue(constantValue), m_extrapolationLow(extrapolationLow), m_extrapolationHi(extrapolationHi), m_isValid(true) {}

protected:
    // using pointer from efficiency reasons. Each copy constructor of DataTable calculates approximation.
    QSharedPointer<DataTable> m_dataTable;
    double m_constantValue;
    double m_extrapolationLow;
    double m_extrapolationHi;

    bool m_isValid;

    friend class AgrosSpecialExtFunction;
};

class AGROS_LIBRARY_API AgrosSpecialExtFunction : public AgrosExtFunction
{
public:
    AgrosSpecialExtFunction(FieldInfo* fieldInfo, int offsetI, SpecialFunctionType type, int count = 0);
    ~AgrosSpecialExtFunction() {}
    virtual void init();
    double getValue(int hermesMarker, double h) const;
    virtual double calculateValue(int hermesMarker, double h) const = 0;

protected:
    void createOneTable(int hermesMarker);

    inline bool useInterpolation() const { return m_count > 0; }

    SpecialFunctionType m_type;
    double m_boundLow;
    double m_boundHi;
    int m_count;
    QString m_variant;
    QMap<int , AgrosSpecialExtFunctionOneMaterial> m_data;
    bool m_useTable;

private:
    double valueFromTable(int hermesMarker, double h) const;
};

struct PointValue
{
    PointValue()
    {
        this->scalar = 0.0;
        this->vector = Point();
        this->material = NULL;
    }

    PointValue(double scalar, Point vector, Material *material)
    {
        this->scalar = scalar;
        this->vector = vector;
        this->material = material;
    }

    double scalar;
    Point vector;
    Material *material;
};

class LocalValue
{
public:
    LocalValue(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType, const Point &point)
        : m_fieldInfo(fieldInfo), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep), m_solutionType(solutionType), m_point(point) {}

    // point
    inline Point point() { return m_point; }

    // variables
    QMap<QString, PointValue> values() const { return m_values; }

    virtual void calculate() = 0;

protected:
    // point
    Point m_point;
    // field info
    FieldInfo *m_fieldInfo;
    int m_timeStep;
    int m_adaptivityStep;
    SolutionMode m_solutionType;

    // variables
    QMap<QString, PointValue> m_values;
};

class IntegralValue
{
public:
    IntegralValue(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType)
        : m_fieldInfo(fieldInfo), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep), m_solutionType(solutionType) {}

    // variables
    inline QMap<QString, double> values() const { return m_values; }

protected:
    // field info
    FieldInfo *m_fieldInfo;
    int m_timeStep;
    int m_adaptivityStep;
    SolutionMode m_solutionType;

    // variables
    QMap<QString, double> m_values;
};

const int OFFSET_NON_DEF = -100;

class FormAgrosInterface
{
public:
    FormAgrosInterface(int offsetI, int offsetJ) : m_markerSource(NULL), m_markerTarget(NULL), m_table(NULL), m_offsetI(offsetI), m_offsetJ(offsetJ), m_markerVolume(0.0) {}

    // source or single marker
    virtual void setMarkerSource(Marker *marker) { m_markerSource = marker; }
    inline Marker *markerSource() { assert(m_markerSource); return m_markerSource; }

    // target marker
    virtual void setMarkerTarget(Marker *marker) { m_markerTarget = marker; }
    inline Marker *markerTarget() { assert(m_markerTarget); return m_markerTarget; }

    // time discretisation table
    inline void setTimeDiscretisationTable(BDF2Table** table) { m_table = table; }

    // volume (area) of the marker
    void setMarkerVolume(double volume) { m_markerVolume = volume; }
    inline double markerVolume() const { return m_markerVolume; }

protected:
    // source or single marker
    Marker *m_markerSource;
    // target marker
    Marker *m_markerTarget;
    // time discretisation table
    BDF2Table **m_table;

    // the offset of position in the stiffness matrix for the case of hard coupling; could be done some other way
    // for example, generated form ...something(heat_matrix_linear, etc)...._1_3 could have variables holding 1 and 3 (the original position,
    // before the shift) offsetI and offsetJ than could be obtained as different of form->i (j), the real position
    // and the original position
    int m_offsetI;
    int m_offsetJ;

    double m_markerVolume;
};

// weakforms
template<typename Scalar>
class MatrixFormVolAgros : public Hermes::Hermes2D::MatrixFormVol<Scalar>, public FormAgrosInterface
{
public:
    MatrixFormVolAgros(unsigned int i, unsigned int j, int offsetI, int offsetJ)
        : Hermes::Hermes2D::MatrixFormVol<Scalar>(i, j), FormAgrosInterface(offsetI, offsetJ) {}
};

template<typename Scalar>
class VectorFormVolAgros : public Hermes::Hermes2D::VectorFormVol<Scalar>, public FormAgrosInterface
{
public:
    VectorFormVolAgros(unsigned int i, int offsetI, int offsetJ, int *offsetPreviousTimeExt, int *offsetCouplingExt)
        : Hermes::Hermes2D::VectorFormVol<Scalar>(i), FormAgrosInterface(offsetI, offsetJ), m_offsetPreviousTimeExt(offsetPreviousTimeExt), m_offsetCouplingExt(offsetCouplingExt) {}
protected:
    int *m_offsetPreviousTimeExt;
    int *m_offsetCouplingExt;
};

template<typename Scalar>
class MatrixFormSurfAgros : public Hermes::Hermes2D::MatrixFormSurf<Scalar>, public FormAgrosInterface
{
public:
    MatrixFormSurfAgros(unsigned int i, unsigned int j, int offsetI, int offsetJ)
        : Hermes::Hermes2D::MatrixFormSurf<Scalar>(i, j), FormAgrosInterface(offsetI, offsetJ) {}
};

template<typename Scalar>
class VectorFormSurfAgros : public Hermes::Hermes2D::VectorFormSurf<Scalar>, public FormAgrosInterface
{
public:
    VectorFormSurfAgros(unsigned int i, int offsetI, int offsetJ)
        : Hermes::Hermes2D::VectorFormSurf<Scalar>(i), FormAgrosInterface(offsetI, offsetJ) {}
};

template<typename Scalar>
class ExactSolutionScalarAgros : public Hermes::Hermes2D::ExactSolutionScalar<Scalar>, public FormAgrosInterface
{
public:
    ExactSolutionScalarAgros(Hermes::Hermes2D::MeshSharedPtr mesh)
        : Hermes::Hermes2D::ExactSolutionScalar<Scalar>(mesh), FormAgrosInterface(OFFSET_NON_DEF, OFFSET_NON_DEF) {}
};


// plugin interface
class AGROS_LIBRARY_API PluginInterface
{
public:
    inline PluginInterface() : m_module(NULL), m_coupling(NULL) {}
    virtual ~PluginInterface() {}

    virtual QString fieldId() = 0;

    inline XMLModule::module *module() const { assert(m_module); return m_module; }
    inline XMLCoupling::coupling *coupling() const { assert(m_coupling); return m_coupling; }

    // weak forms
    virtual MatrixFormVolAgros<double> *matrixFormVol(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Material *material) = 0;
    virtual VectorFormVolAgros<double> *vectorFormVol(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Material *material, int *offsetPreviousTimeExt, int *offsetCouplingExt) = 0;
    virtual MatrixFormSurfAgros<double> *matrixFormSurf(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Boundary *boundary) = 0;
    virtual VectorFormSurfAgros<double> *vectorFormSurf(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Boundary *boundary) = 0;

    virtual ExactSolutionScalarAgros<double> *exactSolution(const ProblemID problemId, FormInfo *form, Hermes::Hermes2D::MeshSharedPtr mesh) = 0;

    // offsetI .. for hard coupling
    virtual AgrosExtFunction *extFunction(const ProblemID problemId, QString id, bool derivative, int offsetI) = 0;

    // error calculators
    virtual Hermes::Hermes2D::ErrorCalculator<double> *errorCalculator(FieldInfo *fieldInfo,
                                                                       const QString &calculator, Hermes::Hermes2D::CalculatedErrorType errorType) = 0;

    // postprocessor
    // filter
    virtual Hermes::Hermes2D::MeshFunctionSharedPtr<double> filter(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                                                                   Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<double> > sln,
                                                                   const QString &variable,
                                                                   PhysicFieldVariableComp physicFieldVariableComp) = 0;

    // local values
    virtual LocalValue *localValue(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType, const Point &point) = 0;
    // surface integrals
    virtual IntegralValue *surfaceIntegral(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType) = 0;
    // volume integrals
    virtual IntegralValue *volumeIntegral(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType) = 0;
    // force calculation
    virtual Point3 force(FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                         Hermes::Hermes2D::Element *element, SceneMaterial *material, const Point3 &point, const Point3 &velocity) = 0;
    virtual bool hasForce(FieldInfo *fieldInfo) = 0;

    // localization
    virtual QString localeName(const QString &name) = 0;
    // description
    virtual QString localeDescription() = 0;

protected:
    XMLModule::module *m_module;
    XMLCoupling::coupling *m_coupling;
};


QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(PluginInterface, "agros2d.PluginInterface/1.0")
QT_END_NAMESPACE

#endif // PLUGIN_INTERFACE_H
