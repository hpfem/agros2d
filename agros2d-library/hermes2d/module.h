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

inline double tern(bool condition, double a, double b)
{
    return (condition ? a : b);
}

inline Hermes::Ord tern(bool condition, Hermes::Ord a, Hermes::Ord b)
{
    if (condition)
        return a;
    else
        return b;
}

inline Hermes::Ord tern(bool condition, Hermes::Ord a, double b)
{
    if (condition)
        return a;
    else
        return Hermes::Ord::get_max_order();
}

inline Hermes::Ord tern(bool condition, double a, Hermes::Ord b)
{
    if (condition)
        return Hermes::Ord::get_max_order();
    else
        return b;
}

// to be thrown when string refering to module entity (boundary condition type, etc.) not found
class AgrosModuleException : public AgrosException
{
public:
    AgrosModuleException(const QString &what) : AgrosException(what)
    {
    }
};


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
class surface;
class force;
class localvariable;
class gui;
class space;
class calculator;

class linearity_option;
}
void findVolumeLinearityOpton(XMLModule::linearity_option& option, XMLModule::module *module, AnalysisType analysisType, LinearityType linearityType);

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

struct AGROS_LIBRARY_API FormInfo
{
    FormInfo() : id(""), i(0), j(0), sym(Hermes::Hermes2D::HERMES_NONSYM), variant(WeakFormVariant_Normal), coefficient(1) {}
    FormInfo(const QString &id, int i = 0, int j = 0, Hermes::Hermes2D::SymFlag sym = Hermes::Hermes2D::HERMES_NONSYM)
        : id(id), i(i), j(j), sym(sym), variant(WeakFormVariant_Normal), coefficient(1), condition("") {}

    QString id;

    // position
    int i;
    int j;

    // symmetric flag
    Hermes::Hermes2D::SymFlag sym;

    QString expr_planar;
    QString expr_axi;
    WeakFormVariant variant;
    double coefficient;
    QString condition;
};

template <typename Scalar>
class AGROS_LIBRARY_API WeakFormAgros : public Hermes::Hermes2D::WeakForm<Scalar>
{
public:
    WeakFormAgros(Block* block);
    ~WeakFormAgros();

    void registerForms();
    void updateExtField();
    inline BDF2Table* bdf2Table() { return m_bdf2Table; }

    // prepares individual forms for given analysis and linearity type, as specified in Elements, using information form Templates
    static QList<FormInfo> wfMatrixVolumeSeparated(XMLModule::module* module, AnalysisType analysisType, LinearityType linearityType);
    static QList<FormInfo> wfVectorVolumeSeparated(XMLModule::module* module, AnalysisType analysisType, LinearityType linearityType);

    // puts forms with the same indices (i,j) from Separated to one single form as sum. It is, however, not significantly faster even
    // compared to Separated case without optimalizations, so it will probably not be used
    static QList<FormInfo> wfMatrixVolumeComplete(XMLModule::module* module, AnalysisType analysisType, LinearityType linearityType);
    static QList<FormInfo> wfVectorVolumeComplete(XMLModule::module* module, AnalysisType analysisType, LinearityType linearityType);

private:
    // materialTarget has to be specified for coupling forms. couplingInfo only for weak couplings
    void registerForm(WeakFormKind type, Field *field, QString area, FormInfo form, int offsetI, int offsetJ, Marker *marker);

    // offsetCouplingExt defines position in Ext field where coupling solutions start
    void registerFormCoupling(WeakFormKind type, QString area, FormInfo form, int offsetI, int offsetJ, SceneMaterial *materialSource,
                              SceneMaterial *materialTarget, CouplingInfo *couplingInfo);
    void addForm(WeakFormKind type, Hermes::Hermes2D::Form<Scalar>* form);

    virtual Hermes::Hermes2D::WeakForm<Scalar>* clone() const { return new WeakFormAgros<Scalar>(m_block); }

    Block* m_block;

    BDF2Table* m_bdf2Table;

    // index in EXT field, where start solutions from previous time levels. ( == number of Value ext functions)
    int m_offsetPreviousTimeExt;

    // index in EXT field, where start solutions from weakly coupled fields ( == number of Value ext functions + time components)
    // we have to pass pointer to individual forms, since it may change during the calculation (the order of the BDF method may vary)
    int m_offsetCouplingExt;

    int m_numberOfForms;
};

namespace Module
{
// local variable
class AGROS_LIBRARY_API LocalVariable
{
public:
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
    LocalVariable(const FieldInfo *fieldInfo, XMLModule::localvariable lv, CoordinateType problemType, AnalysisType analysisType);

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
    Space(int i = 1, Hermes::Hermes2D::SpaceType type = Hermes::Hermes2D::HERMES_INVALID_SPACE, int orderAdjust = 0)
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

// error calculator
class AGROS_LIBRARY_API ErrorCalculator
{
public:
    ErrorCalculator(const QString &id = "",
                  const QString &name = "",
                   const QString &expr = "")
        : m_id(id),
          m_name(name),
          m_expression(expr) {}

    // id
    inline QString id() const { return m_id; }
    // name
    inline QString name() const { return m_name; }

    // expression
    inline QString expression() const { return m_expression; }

private:
    // id
    QString m_id;
    // name
    QString m_name;

    // expressions
    QString m_expression;
};

// material property
struct MaterialTypeVariable
{
    MaterialTypeVariable() : m_id(""), m_shortname(""), m_defaultValue(0),  m_expressionNonlinear(""), m_isTimeDep(false), m_isBool(false), m_onlyIf(QString()), m_onlyIfNot(QString()){}
    MaterialTypeVariable(const QString &id, const QString &shortname,
                         const QString &expressionNonlinear = "", bool isTimedep = false, bool isBool = false, QString onlyIf = QString(), QString onlyIfNot = QString(), bool isSource = false)
        : m_id(id), m_shortname(shortname),
          m_expressionNonlinear(expressionNonlinear), m_isTimeDep(isTimedep), m_isBool(isBool), m_onlyIf(onlyIf), m_onlyIfNot(onlyIfNot), m_isSource(isSource) {}
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
    // show as checkbox
    inline bool isBool() const { return m_isBool; }
    // enable only if checkbox with id == m_onlyIf is checked
    inline QString onlyIf() const {return m_onlyIf; }
    // enable only if checkbox with id == m_onlyIf is NOT checked
    inline QString onlyIfNot() const {return m_onlyIfNot; }
    // field source
    inline bool isSource() const { return m_isSource; }

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
    // bool parameter
    bool m_isBool;
    // only if
    QString m_onlyIf;
    // only if not
    QString m_onlyIfNot;
    // source
    bool m_isSource;
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
struct AGROS_LIBRARY_API BoundaryType
{
    BoundaryType() : m_id(""), m_name(""), m_equation("") {}
    BoundaryType(const FieldInfo *fieldInfo,
                 QList<BoundaryTypeVariable> boundary_type_variables,
                 XMLModule::boundary bdy);
    ~BoundaryType();

    // id
    inline QString id() const { return m_id; }
    // name
    inline QString name() const { return m_name; }

    // variables
    inline QList<BoundaryTypeVariable> variables() const { return m_variables; }

    // weakform
    static QList<FormInfo> wfMatrixSurface(XMLModule::surface *surface, XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType);
    static QList<FormInfo> wfVectorSurface(XMLModule::surface *surface, XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType);
    inline QList<FormInfo> wfMatrixSurface() const {return m_wfMatrix; }
    inline QList<FormInfo> wfVectorSurface() const {return m_wfVector; }

    // essential
    static QList<FormInfo> essential(XMLModule::surface *surface, XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType);
    inline QList<FormInfo> essential() const {return m_essential; }

    // latex equation
    inline QString equation() { return m_equation; }

private:
    // id
    QString m_id;
    // name
    QString m_name;

    QList<FormInfo> m_wfMatrix;
    QList<FormInfo> m_wfVector;
    QList<FormInfo> m_essential;

    // variables
    QList<BoundaryTypeVariable> m_variables;

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
    DialogRow(const FieldInfo *fieldInfo, XMLModule::quantity qty);

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
    DialogUI(const FieldInfo *fieldInfo, XMLModule::gui ui);

    inline QMap<QString, QList<Module::DialogRow> > groups() const { return m_groups; }
    DialogRow dialogRow(const QString &id);
    void clear();

private:
    QMap<QString, QList<Module::DialogRow> > m_groups;
};

// functions
AGROS_LIBRARY_API void updateTimeFunctions(double time);

// available modules
AGROS_LIBRARY_API QMap<QString, QString> availableModules();

// read and write mesh

// index of quantity in the list of quantities at the begining of the volume section of the XML (NOT the reduced list in individual analysis)
//
void AGROS_LIBRARY_API volumeQuantityProperties(XMLModule::module *module, QMap<QString, int> &quantityOrder, QMap<QString, bool> &quantityIsNonlin);
Hermes::vector<Hermes::Hermes2D::MeshSharedPtr> readMeshFromFileBSON(const QString &fileName);
Hermes::vector<Hermes::Hermes2D::MeshSharedPtr> readMeshFromFileXML(const QString &fileName);
void writeMeshToFileXML(const QString &fileName, Hermes::vector<Hermes::Hermes2D::MeshSharedPtr> meshes);
void writeMeshToFileBSON(const QString &fileName, Hermes::vector<Hermes::Hermes2D::MeshSharedPtr> meshes);

}

#endif // HERMES_FIELD_H
