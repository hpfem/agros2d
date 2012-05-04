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
//    Hermes::vector<Hermes::Hermes2D::MeshFunction<Scalar> *> solution;

private:
    //marker_second has to be specified for coupling forms. couplingInfo only for weak couplings
    void registerForm(WFType type, Field* field, QString area, ParserFormExpression* form, int offsetI, int offsetJ,
                      Marker* marker, SceneMaterial* marker_second = NULL, CouplingInfo* couplingInfo = NULL);
    void addForm(WFType type, Hermes::Hermes2D::Form<Scalar>* form);

    Block* m_block;
};

namespace Hermes
{


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
          is_scalar(true),
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
    bool is_scalar;

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
    BoundaryType(Hermes::vector<BoundaryTypeVariable> boundary_type_variables,
                 XMLModule::boundary bdy,
                 CoordinateType problem_type);
    ~BoundaryType();

    // id
    QString id;
    // name
    QString name;

    // variables
    Hermes::vector<BoundaryTypeVariable *> variables;

    // weakform
    Hermes::vector<ParserFormExpression *> weakform_matrix_surface;
    Hermes::vector<ParserFormExpression *> weakform_vector_surface;

    // essential
    Hermes::vector<ParserFormEssential *> essential;
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

    struct Row
    {
        Row(const QString &id, bool nonlin, bool timedep, QString name, QString shortname, QString shortname_html,
                 QString unit, QString unit_html, QString unit_latex,
                 double default_value, QString condition)
            : id(id), nonlin(nonlin), timedep(timedep), name(name), shortname(shortname), shortname_html(shortname_html),
              unit(unit), unit_html(unit_html), unit_latex(unit_latex),
              default_value(default_value), condition(condition) {}
        Row(XMLModule::quantity qty);

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

    std::map<QString, Hermes::vector<Row> > groups;

    void clear();
};

// basic module
struct ModuleDeprecated
{
    // id
    QString fieldid;
    // name
    QString name;
    // deformed shape
    bool deformed_shape;
    // description
    QString description;

    // analyses
    std::map<QString, QString> analyses;
    int steady_state_solutions;
    int harmonic_solutions;
    int transient_solutions;

    // constants
    std::map<QString, double> constants;

    // macros
    std::map<QString, QString> macros;

    // material type
    Hermes::vector<MaterialTypeVariable *> material_type_variables;

    // boundary conditions
    Hermes::vector<BoundaryTypeVariable *> boundary_type_variables;
    Hermes::vector<BoundaryType *> boundary_types;

    // default boundary condition
    BoundaryType *boundary_type_default;

    // weak forms
    Hermes::vector<ParserFormExpression *> weakform_matrix_volume;
    Hermes::vector<ParserFormExpression *> weakform_vector_volume;

    // all physical variables
    Hermes::vector<LocalVariable *> variables;

    // view
    // scalar and vector variables
    Hermes::vector<LocalVariable *> view_scalar_variables;
    Hermes::vector<LocalVariable *> view_vector_variables;

    // default variables
    LocalVariable *view_default_scalar_variable;
    inline PhysicFieldVariableComp view_default_scalar_variable_comp()
    {
        if (view_default_scalar_variable)
            return view_default_scalar_variable->is_scalar ? PhysicFieldVariableComp_Scalar : PhysicFieldVariableComp_Magnitude;
        else
            return PhysicFieldVariableComp_Undefined;
    }
    LocalVariable *view_default_vector_variable;

    // local point variables
    Hermes::vector<LocalVariable *> local_point;

    // surface integrals
    Hermes::vector<Integral *> surface_integral;

    // volume integrals
    Hermes::vector<Integral *> volume_integral;

    // material and boundary UI
    DialogUI material_ui;
    DialogUI boundary_ui;

    // default contructor
    ModuleDeprecated(CoordinateType problemType, AnalysisType analysisType);
    ~ModuleDeprecated();

    mu::Parser *get_parser();

    // read form xml
    void read(const QString &filename);
    // clear
    void clear();

    inline CoordinateType get_coordinate_type() const { return m_coordinateType; }
    inline AnalysisType get_analysis_type() const { return m_analysisType; }

    // variable by name
    LocalVariable *get_variable(const QString &fieldid);
    BoundaryType *get_boundary_type(const QString &fieldid);
    BoundaryTypeVariable *get_boundary_type_variable(const QString &fieldid);
    MaterialTypeVariable *get_material_type_variable(const QString &fieldid);

    // expression
    QString get_expression(LocalVariable *physicFieldVariable,
                               PhysicFieldVariableComp physicFieldVariableComp);

    int number_of_solution() const;

    Hermes::vector<SolutionArray<double> *> solve(ProgressItemSolve *progressItemSolve);
    Hermes::vector<SolutionArray<double> *> solveAdaptiveStep(ProgressItemSolve *progressItemSolve);
    bool solve_init_variables();

    inline virtual void update_time_functions(double time) {}


    ViewScalarFilter<double> *view_scalar_filter(Hermes::Module::LocalVariable *physicFieldVariable,
                                                 PhysicFieldVariableComp physicFieldVariableComp);

    virtual inline void deform_shape(double3* linVert, int count) {}
    virtual inline void deform_shape(double4* linVert, int count) {}

private:
    CoordinateType m_coordinateType;
    AnalysisType m_analysisType;

    std::auto_ptr<XMLModule::module> module_xsd;
};

}
}

// available modules
std::map<QString, QString> availableModules();
// available analyses
std::map<QString, QString> availableAnalyses(const QString &fieldId);


/// TODO not good - contains fieldInfo, but may receive more materials (related to different fields
/// TODO fieldInfo shoud be removed, but other things depend on it
class Parser
{
public:
    // parser
    Hermes::vector<mu::Parser *> parser;
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
    void setParserVariables(Hermes::vector<Material *> materials, Boundary *boundary,
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
