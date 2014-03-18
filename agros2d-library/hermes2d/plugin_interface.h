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
#include "hermes2d/weak_form.h"

#include "../../resources_source/classes/module_xml.h"

class PositionInfo;

//template <typename Scalar>
class AGROS_LIBRARY_API AgrosExtFunction : public Hermes::Hermes2D::UExtFunction<double>
{
public:
    AgrosExtFunction(const FieldInfo* fieldInfo, const WeakFormAgros<double>* wfAgros);
    ~AgrosExtFunction();
    // todo: this is dangerous. Order should be determined from the type of ExtFunction
    // for consants should be 0, for nonlinearities more. Hom much?
    // todo:
    // todo:
    virtual void ord(Hermes::Hermes2D::Func<Hermes::Ord>** ext, Hermes::Hermes2D::Func<Hermes::Ord>** u_ext, Hermes::Hermes2D::Func<Hermes::Ord>* result) const
    {
        result->val[0] = Hermes::Ord(1);
    }

    virtual void init() {}

protected:
    const FieldInfo* m_fieldInfo;
    const WeakFormAgros<double>* m_wfAgros;
   // int m_formsOffset;

};


// used for quantities, that are not present in a given particular analysis
// something has to be pushed to weakform, since the individual forms expect ext[i] with indices i
// taken from general list of quantities in volume, not from lists in particular analysis
// (the reason is that this way we can generate only 1 form which can be used for all analyses, where appropriate
class AGROS_LIBRARY_API AgrosEmptyExtFunction : public AgrosExtFunction
{
public:
    AgrosEmptyExtFunction() : AgrosExtFunction(NULL, 0) {}
    virtual void value(int n, Hermes::Hermes2D::Func<double>** ext, Hermes::Hermes2D::Func<double> **u_ext, Hermes::Hermes2D::Func<double> *result, Hermes::Hermes2D::Geom<double> *geometry) const
    {
        result->val[0] = 123456;
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
    AgrosSpecialExtFunction(const FieldInfo* fieldInfo, const WeakFormAgros<double>* wfAgros, SpecialFunctionType type, int count = 0);
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

    double valueFromTable(int hermesMarker, double h) const;
};

struct LocalPointValue
{
    LocalPointValue()
    {
        this->scalar = 0.0;
        this->vector = Point();
        this->material = NULL;
    }

    LocalPointValue(double scalar, Point vector, Material *material)
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
    LocalValue(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType, const Point &point)
        : m_fieldInfo(fieldInfo), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep), m_solutionType(solutionType), m_point(point) {}

    // point
    inline Point point() { return m_point; }

    // variables
    QMap<QString, LocalPointValue> values() const { return m_values; }

    virtual void calculate() = 0;

protected:
    // point
    Point m_point;
    // field info
    const FieldInfo *m_fieldInfo;
    int m_timeStep;
    int m_adaptivityStep;
    SolutionMode m_solutionType;

    // variables
    QMap<QString, LocalPointValue> m_values;
};

class IntegralValue
{
public:
    IntegralValue(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType)
        : m_fieldInfo(fieldInfo), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep), m_solutionType(solutionType) {}

    // variables
    inline QMap<QString, double> values() const { return m_values; }

protected:
    // field info
    const FieldInfo *m_fieldInfo;
    int m_timeStep;
    int m_adaptivityStep;
    SolutionMode m_solutionType;

    // variables
    QMap<QString, double> m_values;
};

const int OFFSET_NON_DEF = -100;

template<typename Scalar>
class AGROS_LIBRARY_API FormAgrosInterface
{
public:
    FormAgrosInterface(const WeakFormAgros<Scalar>* weakFormAgros);

    // source or single marker
    virtual void setMarkerSource(const Marker *marker);
    inline const Marker *markerSource() { assert(m_markerSource); return m_markerSource; }

    // target marker
    virtual void setMarkerTarget(const Marker *marker);
    inline const Marker *markerTarget() { assert(m_markerTarget); return m_markerTarget; }

    // time discretisation table
    inline void setTimeDiscretisationTable(BDF2Table** table) { m_table = table; }

    // volume (area) of the marker
    void setMarkerVolume(double volume) { m_markerVolume = volume; }
    inline double markerVolume() const { return m_markerVolume; }

protected:
    // source or single marker
    const Marker *m_markerSource;
    // target marker
    const Marker *m_markerTarget;
    // time discretisation table
    BDF2Table **m_table;

//    // the offset of position in the stiffness matrix for the case of hard coupling; could be done some other way
//    // for example, generated form ...something(heat_matrix_linear, etc)...._1_3 could have variables holding 1 and 3 (the original position,
//    // before the shift) offsetI and offsetJ than could be obtained as different of form->i (j), the real position
//    // and the original position
//    int m_offsetI;
//    int m_offsetJ;

    const WeakFormAgros<Scalar> *m_wfAgros;

    double m_markerVolume;
};

// weakforms
template<typename Scalar>
class AGROS_LIBRARY_API MatrixFormVolAgros : public Hermes::Hermes2D::MatrixFormVol<Scalar>, public FormAgrosInterface<Scalar>
{
public:
    MatrixFormVolAgros(unsigned int i, unsigned int j,  const WeakFormAgros<Scalar>* wfAgros)
        : Hermes::Hermes2D::MatrixFormVol<Scalar>(i, j), FormAgrosInterface<Scalar>(wfAgros) {}
protected:
};

template<typename Scalar>
class AGROS_LIBRARY_API VectorFormVolAgros : public Hermes::Hermes2D::VectorFormVol<Scalar>, public FormAgrosInterface<Scalar>
{
public:
    VectorFormVolAgros(unsigned int i, const WeakFormAgros<Scalar>* wfAgros)
        : Hermes::Hermes2D::VectorFormVol<Scalar>(i), FormAgrosInterface<Scalar>(wfAgros) {}
protected:
};

template<typename Scalar>
class AGROS_LIBRARY_API MatrixFormSurfAgros : public Hermes::Hermes2D::MatrixFormSurf<Scalar>, public FormAgrosInterface<Scalar>
{
public:
    MatrixFormSurfAgros(unsigned int i, unsigned int j, const WeakFormAgros<Scalar>* wfAgros)
        : Hermes::Hermes2D::MatrixFormSurf<Scalar>(i, j), FormAgrosInterface<Scalar>(wfAgros) {}
};

template<typename Scalar>
class AGROS_LIBRARY_API VectorFormSurfAgros : public Hermes::Hermes2D::VectorFormSurf<Scalar>, public FormAgrosInterface<Scalar>
{
public:
    VectorFormSurfAgros(unsigned int i, const WeakFormAgros<Scalar>* wfAgros)
        : Hermes::Hermes2D::VectorFormSurf<Scalar>(i), FormAgrosInterface<Scalar>(wfAgros) {}
};

template<typename Scalar>
class AGROS_LIBRARY_API ExactSolutionScalarAgros : public Hermes::Hermes2D::ExactSolutionScalar<Scalar>, public FormAgrosInterface<Scalar>
{
public:
    ExactSolutionScalarAgros(Hermes::Hermes2D::MeshSharedPtr mesh)
        : Hermes::Hermes2D::ExactSolutionScalar<Scalar>(mesh), FormAgrosInterface<Scalar>(nullptr) {}
};


// plugin interface
class AGROS_LIBRARY_API PluginInterface
{
public:
    inline PluginInterface() : m_module(NULL), m_coupling(NULL) {}
    virtual ~PluginInterface() {}

    virtual QString fieldId() = 0;

    inline XMLModule::field *module() const { assert(m_module); return m_module; }
    inline XMLModule::coupling *coupling() const { assert(m_coupling); return m_coupling; }

    // weak forms
    virtual MatrixFormVolAgros<double> *matrixFormVol(const ProblemID problemId, FormInfo *form, const WeakFormAgros<double>* wfAgros, Material *material) = 0;
    virtual VectorFormVolAgros<double> *vectorFormVol(const ProblemID problemId, FormInfo *form, const WeakFormAgros<double>* wfAgros, Material *material) = 0;
    virtual MatrixFormSurfAgros<double> *matrixFormSurf(const ProblemID problemId, FormInfo *form, const WeakFormAgros<double>* wfAgros, Boundary *boundary) = 0;
    virtual VectorFormSurfAgros<double> *vectorFormSurf(const ProblemID problemId, FormInfo *form, const WeakFormAgros<double>* wfAgros, Boundary *boundary) = 0;

    virtual ExactSolutionScalarAgros<double> *exactSolution(const ProblemID problemId, FormInfo *form, Hermes::Hermes2D::MeshSharedPtr mesh) = 0;

    // offsetI .. for hard coupling
    virtual AgrosExtFunction *extFunction(const ProblemID problemId, QString id, bool derivative, bool linearized, const WeakFormAgros<double>* wfAgros) = 0;

    // error calculators
    virtual Hermes::Hermes2D::ErrorCalculator<double> *errorCalculator(const FieldInfo *fieldInfo,
                                                                       const QString &calculator, Hermes::Hermes2D::CalculatedErrorType errorType) = 0;

    // postprocessor
    // filter
    virtual Hermes::Hermes2D::MeshFunctionSharedPtr<double> filter(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                                                                   Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<double> > sln,
                                                                   const QString &variable,
                                                                   PhysicFieldVariableComp physicFieldVariableComp) = 0;

    // local values
    virtual LocalValue *localValue(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType, const Point &point) = 0;
    // surface integrals
    virtual IntegralValue *surfaceIntegral(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType) = 0;
    // volume integrals
    virtual IntegralValue *volumeIntegral(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType) = 0;
    // force calculation
    virtual Point3 force(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                         Hermes::Hermes2D::Element *element, SceneMaterial *material, const Point3 &point, const Point3 &velocity) = 0;
    virtual bool hasForce(const FieldInfo *fieldInfo) = 0;

    // localization
    virtual QString localeName(const QString &name) = 0;
    // description
    virtual QString localeDescription() = 0;

protected:
    XMLModule::field *m_module;
    XMLModule::coupling *m_coupling;
};


QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(PluginInterface, "agros2d.PluginInterface/1.0")
QT_END_NAMESPACE

#endif // PLUGIN_INTERFACE_H
