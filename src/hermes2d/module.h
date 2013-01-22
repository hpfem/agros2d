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

#ifndef HERMES_FIELD_H
#define HERMES_FIELD_H

#include "util.h"
#include "util/enums.h"
#include "hermes2d.h"

inline Hermes::Ord sign(Hermes::Ord arg) { return Hermes::Ord(); }

struct ProblemID
{
    ProblemID() :
        sourceFieldId(""), targetFieldId(""),
        analysisTypeSource(AnalysisType_Undefined), analysisTypeTarget(AnalysisType_Undefined),
        coordinateType(CoordinateType_Undefined), linearityType(LinearityType_Undefined),
        couplingType(CouplingType_Undefined) {}

    // TODO: set/get methods
    QString sourceFieldId;
    QString targetFieldId;
    AnalysisType analysisTypeSource;
    AnalysisType analysisTypeTarget;
    CoordinateType coordinateType;
    LinearityType linearityType;
    CouplingType couplingType;

    QString toString()
    {
        // TODO: implement toString() method
        return "TODO";
    }
};

extern double actualTime;

namespace XMLModule
{
class module;
class quantity;
class boundary;
class force;
class localvariable;
class gui;
class space;
}

class Marker;

class Boundary;
class SceneBoundary;
class SceneBoundaryDialog;

class Material;
class SceneMaterial;
class SceneMaterialDialog;

class Field;
class Block;

struct SceneViewSettings;
template <typename Scalar> struct SolutionArray;

class ProgressItemSolve;

class FieldInfo;
class CouplingInfo;

class BDF2Table;

struct FormInfo
{
    FormInfo() : id(""), i(0), j(0), sym(Hermes::Hermes2D::HERMES_NONSYM) {}
    FormInfo(const QString &id, int i, int j = 0, Hermes::Hermes2D::SymFlag sym = Hermes::Hermes2D::HERMES_NONSYM)
        : id(id), i(i), j(j), sym(sym) {}

    QString id;

    // position
    int i;
    int j;

    // symmetric flag
    Hermes::Hermes2D::SymFlag sym;
};

const int maxSourceFieldComponents = 2;


template <typename Scalar>
class WeakFormAgros : public Hermes::Hermes2D::WeakForm<Scalar>
{
public:
    WeakFormAgros(Block* block);
    ~WeakFormAgros();

    void registerForms(BDF2Table *bdf2Table);

private:
    // materialTarget has to be specified for coupling forms. couplingInfo only for weak couplings
    void registerForm(WeakFormKind type, Field *field, QString area, FormInfo *form, int offsetI, int offsetJ, Marker *marker, BDF2Table* bdf2Table = NULL);
    void registerFormCoupling(WeakFormKind type, QString area, FormInfo *form, int offsetI, int offsetJ, SceneMaterial *materialSource,
                              SceneMaterial *materialTarget, CouplingInfo *couplingInfo);
    void addForm(WeakFormKind type, Hermes::Hermes2D::Form<Scalar>* form);

    virtual Hermes::Hermes2D::WeakForm<Scalar>* clone() const { return new WeakFormAgros<Scalar>(m_block); }

    Block* m_block;
};

namespace Module
{
// local variable
struct LocalVariable
{
    struct Expression
    {
        Expression(const QString &scalar = "", const QString &compX = "", const QString &compY = "")
            : m_scalar(scalar), m_compX(compX), m_compY(compY) {}

        // expressions
        inline QString scalar() const { return m_scalar; }
        inline QString compX() const { return m_compX; }
        inline QString compY() const { return m_compY; }

    private:
        // expressions
        QString m_scalar;
        QString m_compX;
        QString m_compY;
    };

    LocalVariable(const QString &id = "",
                  const QString &name = "",
                  const QString &shortname = "",
                  const QString &unit = "",
                  const QString &unitHtml = "")
        : m_id(id),
          m_name(name),
          m_shortname(shortname),
          m_unit(unit),
          m_unitHtml(unitHtml),
          m_isScalar(true),
          m_expression(Expression()) {}
    LocalVariable(XMLModule::localvariable lv, const QString &fieldId, CoordinateType problemType, AnalysisType analysisType);

    // id
    inline QString id() const { return m_id; }
    // name
    inline QString name() const { return m_name; }
    // short name
    inline QString shortname() const { return m_shortname; }
    inline QString shortnameHtml() const { return m_shortnameHtml; }
    // unit
    inline QString unit() const { return m_unit; }
    inline QString unitHtml() const { return m_unitHtml; }

    // is scalar variable
    inline bool isScalar() const { return m_isScalar; }

    // expressions
    inline Expression expression() const { return m_expression; }

private:
    // id
    QString m_id;
    // name
    QString m_name;
    // short name
    QString m_shortname;
    QString m_shortnameHtml;
    // unit
    QString m_unit;
    QString m_unitHtml;

    // is scalar variable
    bool m_isScalar;

    // expressions
    Expression m_expression;
};

struct Space
{
    Space(int i = 1, Hermes::Hermes2D::SpaceType type = Hermes::Hermes2D::HERMES_H1_SPACE, int orderAdjust = 0)
        : m_i(i), m_type(type), m_orderAdjust(orderAdjust) {}

    // expressions
    inline int i() const { return m_i; }
    inline Hermes::Hermes2D::SpaceType type() const { return m_type; }
    inline int orderAdjust() const { return m_orderAdjust; }

private:
    // expressions
    int m_i;
    Hermes::Hermes2D::SpaceType m_type;
    int m_orderAdjust;
};


// force
struct Force
{
    Force(const QString &compX = "", const QString &compY = "", const QString &compZ = "")
        : m_compX(compX), m_compY(compY), m_compZ(compZ) {}

    // expressions
    inline QString compX() const { return m_compX; }
    inline QString compY() const { return m_compY; }
    inline QString compZ() const { return m_compZ; }

private:
    // expressions
    QString m_compX;
    QString m_compY;
    QString m_compZ;
};

// material property
struct MaterialTypeVariable
{
    MaterialTypeVariable() : m_id(""), m_shortname(""), m_defaultValue(0),  m_expressionNonlinear(""), m_isTimeDep(false) {}
    MaterialTypeVariable(const QString &id, const QString &shortname,
                         const QString &expressionNonlinear = "", bool isTimedep = false)
        : m_id(id), m_shortname(shortname),
          m_expressionNonlinear(expressionNonlinear), m_isTimeDep(isTimedep) {}
    MaterialTypeVariable(XMLModule::quantity quant);

    // id
    inline QString id() const { return m_id; }
    // short name
    inline QString shortname() const { return m_shortname; }
    // nonlinear expression
    inline QString expressionNonlinear() const { return m_expressionNonlinear; }
    inline bool isNonlinear() const { return !m_expressionNonlinear.isEmpty(); }
    // timedep
    inline bool isTimeDep() const { return m_isTimeDep; }

private:
    // id
    QString m_id;
    // short name
    QString m_shortname;
    // default value
    double m_defaultValue;
    // nonlinear expression
    QString m_expressionNonlinear;
    // timedep
    bool m_isTimeDep;
};

// boundary condition type variable
struct BoundaryTypeVariable
{
    BoundaryTypeVariable()
        : m_id(""), m_shortname(""), m_defaultValue(0),
          m_isTimeDep(false), m_isSpaceDep(false) {}
    BoundaryTypeVariable(const QString &id, QString shortname,
                         bool isTimedep = false, bool isSpaceDep = false)
        : m_id(id), m_shortname(shortname),
          m_isTimeDep(isTimedep), m_isSpaceDep(isSpaceDep) {}
    BoundaryTypeVariable(XMLModule::quantity quant);

    // id
    inline QString id() const { return m_id; }
    // short name
    inline QString shortname() const { return m_shortname; }
    // timedep
    inline bool isTimeDep() const { return m_isTimeDep; }
    // spacedep
    inline bool isSpaceDep() const { return m_isSpaceDep; }

private:
    // id
    QString m_id;
    // short name
    QString m_shortname;
    // default value
    double m_defaultValue;
    // timedep
    bool m_isTimeDep;
    // spacedep
    bool m_isSpaceDep;
};

// boundary condition type
struct BoundaryType
{
    BoundaryType() : m_id(""), m_name(""), m_equation("") {}
    BoundaryType(QList<BoundaryTypeVariable> boundary_type_variables,
                 XMLModule::boundary bdy,
                 CoordinateType problem_type);
    ~BoundaryType();

    // id
    inline QString id() const { return m_id; }
    // name
    inline QString name() const { return m_name; }

    // variables
    inline QList<BoundaryTypeVariable *> variables() const { return m_variables; }

    // weakform
    inline QList<FormInfo *> wfMatrixSurface() const { return m_wfMatrixSurface; }
    inline QList<FormInfo *> wfVectorSurface() const { return m_wfVectorSurface; }

    // essential
    inline QList<FormInfo *> essential() const { return m_essential; }

    // latex equation
    inline QString equation() { return m_equation; }

private:
    // id
    QString m_id;
    // name
    QString m_name;

    // variables
    QList<BoundaryTypeVariable *> m_variables;

    // weakform
    QList<FormInfo *> m_wfMatrixSurface;
    QList<FormInfo *> m_wfVectorSurface;

    // essential
    QList<FormInfo *> m_essential;

    // latex equation
    QString m_equation;
};

// surface and volume integral value
struct Integral
{
    Integral(const QString &id = "",
             const QString &name = "",
             const QString &shortname = "",
             const QString &shortnameHtml = "",
             const QString &unit = "",
             const QString &unitHtml = "",
             const QString &expression = "")
        : m_id(id),
          m_name(name),
          m_shortname(shortname),
          m_shortnameHtml(shortnameHtml),
          m_unit(unit),
          m_unitHtml(unitHtml),
          m_expression(expression) {}

    // id
    inline QString id() const { return m_id; }
    // name
    inline QString name() const { return m_name; }
    // short name
    inline QString shortname() const { return m_shortname; }
    inline QString shortnameHtml() const { return m_shortnameHtml; }
    // unit
    inline QString unit() const { return m_unit; }
    inline QString unitHtml() const { return m_unitHtml; }

    // expressions
    inline QString expression() const { return m_expression; }

private:
    // id
    QString m_id;
    // name
    QString m_name;
    // short name
    QString m_shortname;
    QString m_shortnameHtml;
    // unit
    QString m_unit;
    QString m_unitHtml;

    // expression
    QString m_expression;
};

// dialog UI
struct DialogRow
{
    DialogRow(XMLModule::quantity qty);

    inline QString id() const { return m_id; }

    inline QString name() const { return m_name; }
    inline QString shortname() const { return m_shortname; }
    inline QString shortnameHtml() const { return m_shortnameHtml; }
    inline QString shortnameDependence() const { return m_shortnameDependence; }
    inline QString shortnameDependenceHtml() const { return m_shortnameDependenceHtml; }

    inline QString unit() const { return m_unit; }
    inline QString unitHtml() const { return m_unitHtml; }
    inline QString unitLatex() const { return m_unitLatex; }

    inline double defaultValue() const { return m_defaultValue; }
    inline QString condition() const { return m_condition; }

private:
    QString m_id;

    QString m_name;
    QString m_shortname;
    QString m_shortnameHtml;
    QString m_shortnameDependence;
    QString m_shortnameDependenceHtml;

    QString m_unit;
    QString m_unitHtml;
    QString m_unitLatex;

    double m_defaultValue;
    QString m_condition;
};

struct DialogUI
{
    DialogUI() {}
    DialogUI(XMLModule::gui ui);

    inline QMap<QString, QList<Module::DialogRow> > groups() const { return m_groups; }
    DialogRow dialogRow(const QString &id);
    void clear();

private:
    QMap<QString, QList<Module::DialogRow> > m_groups;
};

// basic xml module
struct BasicModule
{
    // default contructor
    BasicModule(const QString &fieldId, CoordinateType problemType, AnalysisType analysisType);
    ~BasicModule();

    // name
    inline QString name() const { return m_name; }
    // deformed shape
    inline bool hasDeformableShape() const { return m_hasDeformableShape; }
    // description
    inline QString description() const { return m_description; }

    // constants
    inline QMap<QString, double> constants() const { return m_constants; }

    // macros
    inline QMap<QString, QString> macros() const { return m_macros; }

    // spaces
    inline QMap<int, Module::Space> spaces() const { return m_spaces; }

    // material type variable
    inline QList<MaterialTypeVariable *> materialTypeVariables() const { return m_materialTypeVariables; }

    // boundary type and type variable
    inline QList<BoundaryTypeVariable *> boundaryTypeVariables() const { return m_boundaryTypeVariables; }
    inline QList<BoundaryType *> boundaryTypes() const { return m_boundaryTypes; }
    // default boundary condition
    BoundaryType *boundaryTypeDefault() const { return m_boundaryTypeDefault; }

    // weak forms
    inline QList<FormInfo *> wfMatrixVolumeExpression() const { return m_wfMatrixVolume; }
    inline QList<FormInfo *> wfVectorVolumeExpression() const { return m_wfVectorVolume; }

    // all physical variables
    QList<LocalVariable *> variables;

    // material and boundary user interface
    inline DialogUI *materialUI() { return m_materialUI; }
    inline DialogUI *boundaryUI() { return m_boundaryUI; }

    inline virtual void updateTimeFunctions(double time) {}

    virtual inline void deformShape(double3* linVert, int count) {}
    virtual inline void deformShape(double4* linVert, int count) {}

    // coordinate and analysis type
    inline CoordinateType coordinateType() { return m_coordinateType; }
    inline AnalysisType analysisType() { return m_analysisType; }

    // number of solutions
    inline int numberOfSolutions() const { return m_numberOfSolutions; }

    // scalar filter
    Hermes::Hermes2D::Filter<double> *viewScalarFilter(Module::LocalVariable *physicFieldVariable,
                                                       PhysicFieldVariableComp physicFieldVariableComp);

    // variable by name
    LocalVariable *localVariable(const QString &id);
    Integral *surfaceIntegral(const QString &id);
    Integral *volumeIntegral(const QString &id);
    BoundaryType *boundaryType(const QString &id);
    BoundaryTypeVariable *boundaryTypeVariable(const QString &id);
    MaterialTypeVariable *materialTypeVariable(const QString &id);

    // get expression
    QString expression(LocalVariable *physicFieldVariable,
                       PhysicFieldVariableComp physicFieldVariableComp);

    // local point variables
    inline QList<LocalVariable *> localPointVariables() const { return m_localPointVariables; }

    // surface integrals
    inline QList<Integral *> surfaceIntegrals() const { return m_surfaceIntegrals; }

    // volume integrals
    inline QList<Integral *> volumeIntegrals() const { return m_volumeIntegrals; }
    // force
    inline Force force() const { return m_force; }

    // view scalar and vector variables
    inline QList<LocalVariable *> viewScalarVariables() const { return m_viewScalarVariables; }
    inline QList<LocalVariable *> viewVectorVariables() const { return m_viewVectorVariables; }

    // default view variables
    inline LocalVariable *defaultViewScalarVariable() const { return m_defaultViewScalarVariable; }
    inline PhysicFieldVariableComp defaultViewScalarVariableComp()
    {
        if (m_defaultViewScalarVariable)
            return m_defaultViewScalarVariable->isScalar() ? PhysicFieldVariableComp_Scalar : PhysicFieldVariableComp_Magnitude;
        else
            return PhysicFieldVariableComp_Undefined;
    }
    inline LocalVariable *defaultViewVectorVariable() const { return m_defaultViewVectorVariable; }

    // latex equation
    inline QString equation() { return m_equation; }

protected:
    // id
    QString m_fieldid;

private:
    // coordinate and analysis type
    CoordinateType m_coordinateType;
    AnalysisType m_analysisType;

    // name
    QString m_name;
    // deformed shape
    bool m_hasDeformableShape;
    // description
    QString m_description;

    // constants
    QMap<QString, double> m_constants;

    // macros
    QMap<QString, QString> m_macros;

    // spaces
    QMap<int, Module::Space> m_spaces;

    // number of solutions
    int m_numberOfSolutions;

    // material type
    QList<MaterialTypeVariable *> m_materialTypeVariables;

    // boundary conditions
    QList<BoundaryTypeVariable *> m_boundaryTypeVariables;
    QList<BoundaryType *> m_boundaryTypes;
    // default boundary condition
    BoundaryType *m_boundaryTypeDefault;

    // local point variables
    QList<LocalVariable *> m_localPointVariables;

    // surface integrals
    QList<Integral *> m_surfaceIntegrals;

    // volume integrals
    QList<Integral *> m_volumeIntegrals;

    // force
    Force m_force;

    // material and boundary user interface
    DialogUI *m_materialUI;
    DialogUI *m_boundaryUI;

    // view scalar and vector variables
    QList<LocalVariable *> m_viewScalarVariables;
    QList<LocalVariable *> m_viewVectorVariables;

    // default variables
    LocalVariable *m_defaultViewScalarVariable;
    LocalVariable *m_defaultViewVectorVariable;

    // weak forms
    QList<FormInfo *> m_wfMatrixVolume;
    QList<FormInfo *> m_wfVectorVolume;

    // latex equation
    QString m_equation;

    // read form xml
    void read(const QString &filename);
    // clear
    void clear();

    // xml module description
    std::auto_ptr<XMLModule::module> module_xsd;
};

}

// available modules
QMap<QString, QString> availableModules();
// available analyses
QMap<AnalysisType, QString> availableAnalyses(const QString &fieldId);

// mesh fix
void readMeshDirtyFix();

// read mesh
Hermes::vector<Hermes::Hermes2D::Mesh *> readMeshFromFile(const QString &fileName);
void writeMeshToFile(const QString &fileName, Hermes::vector<Hermes::Hermes2D::Mesh *> meshes);

void refineMesh(FieldInfo *fieldInfo, Hermes::Hermes2D::Mesh *mesh, bool refineGlobal, bool refineTowardsEdge, bool refineArea);

#endif // HERMES_FIELD_H
