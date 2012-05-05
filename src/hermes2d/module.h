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
#include "hermes2d.h"

extern double actualTime;

namespace XMLModule
{
class module;
class quantity;
class boundary;
class localvariable;
class gui;
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
template <typename Scalar> class ViewScalarFilter;
class ParserFormExpression;
class ParserFormEssential;

class ProgressItemSolve;

class FieldInfo;
class CouplingInfo;

template<typename Scalar>
class InitialCondition : public Hermes::Hermes2D::ExactSolutionScalar<Scalar>
{
public:
    InitialCondition(Hermes::Hermes2D::Mesh *mesh, double constant_value) : Hermes::Hermes2D::ExactSolutionScalar<double>(mesh),
        constant_value(constant_value) {}

    virtual Scalar value(double x, double y) const
    {
        return constant_value;
    }

    virtual void derivatives(double x, double y, Scalar& dx, Scalar& dy) const
    {
        dx = 0;
        dy = 0;
    }

    virtual Hermes::Ord ord(Hermes::Ord x, Hermes::Ord y) const
    {
        return Hermes::Ord(0);
    }

    Hermes::Hermes2D::MeshFunction<Scalar>* clone()
    {
        InitialCondition<Scalar>* ic = new InitialCondition<Scalar>(this->mesh, this->constant_value);
        return ic;
    }

private:
    double constant_value;
};


//template <typename Scalar>
//class ViewScalarFilter;

enum WFType
{
    WFType_MatVol,
    WFType_MatSurf,
    WFType_VecVol,
    WFType_VecSurf
};

template <typename Scalar>
class WeakFormAgros : public Hermes::Hermes2D::WeakForm<Scalar>
{
public:
    //TODO
    //TODO coupling a sourceSolution asi obalit do nejake tridy
    //TODO mozna by se melo udelat neco jako CouplingInfo (obdoba fieldInfo), a tam by se teprv ziskal Coupling, jako se ziska Module
    WeakFormAgros(Block* block);

    void registerForms();

    //    // previous solution
    //    QList<Hermes::Hermes2D::MeshFunction<Scalar> *> solution;

private:
    //marker_second has to be specified for coupling forms. couplingInfo only for weak couplings
    void registerForm(WFType type, Field* field, QString area, ParserFormExpression* form, int offsetI, int offsetJ,
                      Marker* marker, SceneMaterial* marker_second = NULL, CouplingInfo* couplingInfo = NULL);
    void addForm(WFType type, Hermes::Hermes2D::Form<Scalar>* form);

    Block* m_block;
};

namespace Module
{
// local variable
struct LocalVariable
{
    struct Expression
    {
        Expression(const QString &scalar = "", const QString &comp_x = "", const QString &comp_y = "")
            : scalar(scalar), comp_x(comp_x), comp_y(comp_y) {}

        // expressions
        QString scalar;
        QString comp_x;
        QString comp_y;
    };

    LocalVariable(const QString &id = "",
                  const QString &name = "",
                  const QString &shortname = "",
                  const QString &unit = "",
                  const QString &unit_html = "")
        : id(id),
          name(name),
          shortname(shortname),
          unit(unit),
          unit_html(unit),
          isScalar(true),
          expr(Expression()) {}
    LocalVariable(XMLModule::localvariable lv, CoordinateType problemType, AnalysisType analysisType);

    // id
    QString id;
    // name
    QString name;
    // short name
    QString shortname;
    QString shortname_html;
    // unit
    QString unit;
    QString unit_html;

    // is scalar variable
    bool isScalar;

    // expressions
    Expression expr;
};

// material property
struct MaterialTypeVariable
{
    MaterialTypeVariable() : id(""), shortname(""), default_value(0) {}
    MaterialTypeVariable(const QString &id, QString shortname,
                         double default_value = 0);
    MaterialTypeVariable(XMLModule::quantity quant);

    // id
    QString id;
    // short name
    QString shortname;
    // default value
    double default_value;
};

// boundary condition type variable
struct BoundaryTypeVariable
{
    BoundaryTypeVariable() : id(""), shortname(""), default_value(0) {}
    BoundaryTypeVariable(const QString &id, QString shortname,
                         double default_value = 0);
    BoundaryTypeVariable(XMLModule::quantity quant);

    // id
    QString id;
    // short name
    QString shortname;
    // default value
    double default_value;
};

// boundary condition type
struct BoundaryType
{
    BoundaryType() : id(""), name("") {}
    BoundaryType(QList<BoundaryTypeVariable> boundary_type_variables,
                 XMLModule::boundary bdy,
                 CoordinateType problem_type);
    ~BoundaryType();

    // id
    QString id;
    // name
    QString name;

    // variables
    QList<BoundaryTypeVariable *> variables;

    // weakform
    QList<ParserFormExpression *> m_wfMatrixSurface;
    QList<ParserFormExpression *> m_wfVectorSurface;

    // essential
    QList<ParserFormEssential *> essential;
};

// surface and volume integral value
struct Integral
{
    struct Expression
    {
        Expression() : scalar("") {}

        // expressions
        QString scalar;
    };

    Integral() : id(""), name(""), shortname(""), shortname_html(""), unit(""), unit_html(""), expr(Expression()) {}

    // id
    QString id;
    // name
    QString name;
    // short name
    QString shortname;
    QString shortname_html;
    // unit
    QString unit;
    QString unit_html;

    // expressions
    Expression expr;
};

// dialog UI
struct DialogUI
{
    DialogUI() {}
    DialogUI(XMLModule::gui ui);

    struct DialogRow
    {
        DialogRow(XMLModule::quantity qty);

        QString id;

        bool nonlin;
        bool timedep;

        QString name;
        QString shortname;
        QString shortname_html;

        QString unit;
        QString unit_html;
        QString unit_latex;

        double default_value;
        QString condition;
    };

    QMap<QString, QList<DialogRow> > groups;

    void clear();
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

    // analyses
    inline QMap<AnalysisType, QString> analyses() const { m_analyses; }

    // constants
    inline QMap<QString, double> constants() const { return m_constants; }

    // macros
    inline QMap<QString, QString> macros() const { return m_macros; }

    // material type variable
    inline QList<MaterialTypeVariable *> materialTypeVariables() const { return m_materialTypeVariables; }

    // boundary type and type variable
    inline QList<BoundaryTypeVariable *> boundaryTypeVariables() const { return m_boundaryTypeVariables; }
    inline QList<BoundaryType *> boundaryTypes() const { return m_boundaryTypes; }
    // default boundary condition
    BoundaryType *boundaryTypeDefault() const { return m_boundaryTypeDefault; }

    // weak forms
    inline QList<ParserFormExpression *> wfMatrixVolumeExpression() const { return m_wfMatrixVolumeExpression; }
    inline QList<ParserFormExpression *> wfVectorVolumeExpression() const { return m_wfVectorVolumeExpression; }

    // all physical variables
    QList<LocalVariable *> variables;

    // material and boundary user interface
    inline DialogUI *materialUI() { return m_materialUI; }
    inline DialogUI *boundaryUI() { return m_boundaryUI; }

    mu::Parser *expressionParser(const QString &expr = "");

    inline virtual void updateTimeFunctions(double time) {}

    virtual inline void deformShape(double3* linVert, int count) {}
    virtual inline void deformShape(double4* linVert, int count) {}

    // coordinate and analysis type
    inline CoordinateType coordinateType() { return m_coordinateType; }
    inline AnalysisType analysisType() { return m_analysisType; }

    // number of solutions
    inline int numberOfSolutions() const { return m_numberOfSolutions; }

    // scalar filter
    ViewScalarFilter<double> *viewScalarFilter(Module::LocalVariable *physicFieldVariable,
                                               PhysicFieldVariableComp physicFieldVariableComp);

    // variable by name
    LocalVariable *localVariable(const QString &m_fieldid);
    BoundaryType *boundaryType(const QString &m_fieldid);
    BoundaryTypeVariable *boundaryTypeVariable(const QString &m_fieldid);
    MaterialTypeVariable *materialTypeVariable(const QString &m_fieldid);

    // get expression
    QString expression(LocalVariable *physicFieldVariable,
                       PhysicFieldVariableComp physicFieldVariableComp);

    // local point variables
    inline QList<LocalVariable *> localPointVariables() const { return m_localPointVariables; }

    // surface integrals
    inline QList<Integral *> surfaceIntegrals() const { return m_surfaceIntegrals; }

    // volume integrals
    inline QList<Integral *> volumeIntegrals() const { return m_volumeIntegrals; }

    // view scalar and vector variables
    inline QList<LocalVariable *> viewScalarVariables() const { return m_viewScalarVariables; }
    inline QList<LocalVariable *> viewVectorVariables() const { return m_viewVectorVariables; }

    // default view variables
    inline LocalVariable *defaultViewScalarVariable() const { return m_defaultViewScalarVariable; }
    inline PhysicFieldVariableComp defaultViewScalarVariableComp()
    {
        if (m_defaultViewScalarVariable)
            return m_defaultViewScalarVariable->isScalar ? PhysicFieldVariableComp_Scalar : PhysicFieldVariableComp_Magnitude;
        else
            return PhysicFieldVariableComp_Undefined;
    }
    inline LocalVariable *defaultViewVectorVariable() const { return m_defaultViewVectorVariable; }

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

    // analyses
    QMap<AnalysisType, QString> m_analyses;

    // constants
    QMap<QString, double> m_constants;

    // macros
    QMap<QString, QString> m_macros;

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
    QList<ParserFormExpression *> m_wfMatrixVolumeExpression;
    QList<ParserFormExpression *> m_wfVectorVolumeExpression;

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

/// TODO not good - contains fieldInfo, but may receive more materials (related to different fields
/// TODO fieldInfo shoud be removed, but other things depend on it
class Parser
{
public:
    // parser
    QList<mu::Parser *> parser;
    std::map<QString, double> parser_variables;

    Parser(FieldInfo *fieldInfo);
    Parser(CouplingInfo *fieldInfo);
    ~Parser();

    //not used at all
    //void initParserBoundaryVariables(Boundary *boundary);

    // should be called only for postprocessing - uses m_fieldInfo!
    //TODO bad
    void initParserMaterialVariables();

    // can be called with more materials - coupling - does not use m_fieldInfo, takes fieldInfo from materials
    //TODO bad
    void setParserVariables(QList<Material *> materials, Boundary *boundary,
                            double value = 0.0, double dx = 0.0, double dy = 0.0);

    void setParserVariables(Material* materials, Boundary *boundary,
                            double value = 0.0, double dx = 0.0, double dy = 0.0);

private:
    FieldInfo* m_fieldInfo;
    CouplingInfo* m_couplingInfo;
};

template <typename Scalar>
class ViewScalarFilter : public Hermes::Hermes2D::Filter<Scalar>
{
public:
    ViewScalarFilter(FieldInfo *fieldInfo,
                     Hermes::vector<Hermes::Hermes2D::MeshFunction<Scalar> *> sln,
                     QString expression);
    ~ViewScalarFilter();

    double get_pt_value(double x, double y, int item = Hermes::Hermes2D::H2D_FN_VAL);

    ViewScalarFilter<Scalar>* clone();

protected:
    typename Hermes::Hermes2D::Function<Scalar>::Node* node;

    double px;
    double py;
    double *pvalue;
    double *pdx;
    double *pdy;

    Parser *parser;

    void initParser(const QString &expression);
    void precalculate(int order, int mask);

private:
    FieldInfo *m_fieldInfo;
};

// mesh fix
void readMeshDirtyFix();

// read mesh
QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> readMeshesFromFile(const QString &fileName);
void writeMeshFromFile(const QString &fileName, Hermes::Hermes2D::Mesh *mesh);

void refineMesh(FieldInfo *fieldInfo, Hermes::Hermes2D::Mesh *mesh, bool refineGlobal, bool refineTowardsEdge);

// return geom type
Hermes::Hermes2D::GeomType convertProblemType(CoordinateType problemType);

#endif // HERMES_FIELD_H
