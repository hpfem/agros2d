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

#include <rapidxml.cpp>
#include <rapidxml_utils.cpp>

extern double actualTime;

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
struct Coupling;

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

const bool hardCoupling = true;

template <typename Scalar>
class WeakFormAgros : public Hermes::Hermes2D::WeakForm<Scalar>
{
public:
    //TODO
    //TODO coupling a sourceSolution asi obalit do nejake tridy
    //TODO mozna by se melo udelat neco jako CouplingInfo (obdoba fieldInfo), a tam by se teprv ziskal Coupling, jako se ziska Module
    WeakFormAgros(Block* block, Hermes::Hermes2D::Solution<Scalar>* sourceSolution = NULL);

    void registerForms();

    // previous solution
    Hermes::vector<Hermes::Hermes2D::MeshFunction<Scalar> *> solution;

private:
    void registerForm(WFType type,/* FieldInfo* fieldInfo,*/ string area, Marker* marker, ParserFormExpression* form, int offsetI, int offsetJ);
    void addForm(WFType type, Hermes::Hermes2D::Form<Scalar>* form);

    Block* m_block;
    Hermes::Hermes2D::Solution<Scalar>* m_sourceSolution;
};

namespace Hermes
{

inline std::string analysis_type_tostring(AnalysisType analysisType)
{
    if (analysisType == AnalysisType_SteadyState)
        return "steadystate";
    else if (analysisType == AnalysisType_Harmonic)
        return "harmonic";
    else if (analysisType == AnalysisType_Transient)
        return "transient";
}

namespace Module
{
// local variable
struct LocalVariable
{
    struct Expression
    {
        Expression(std::string scalar = "", std::string comp_x = "", std::string comp_y = "")
            : scalar(scalar), comp_x(comp_x), comp_y(comp_y) {}
        Expression(rapidxml::xml_node<> *node, CoordinateType problem_type);

        // expressions
        std::string scalar;
        std::string comp_x;
        std::string comp_y;
    };

    LocalVariable(std::string id = "", std::string name = "", std::string shortname = "", std::string unit = "", std::string unit_html = "")
        : id(id), name(name), shortname(shortname), unit(unit), unit_html(unit), is_scalar(true), expression(Expression()) {}
    LocalVariable(rapidxml::xml_node<> *variable, CoordinateType problemType, AnalysisType analysisType);

    // id
    std::string id;
    // name
    std::string name;
    // short name
    std::string shortname;
    std::string shortname_html;
    // unit
    std::string unit;
    std::string unit_html;

    // is scalar variable
    bool is_scalar;

    // expressions
    Expression expression;
};

// material property
struct MaterialTypeVariable
{
    MaterialTypeVariable() : id(""), shortname(""), default_value(0) {}
    MaterialTypeVariable(std::string id, std::string shortname,
                         double default_value = 0);
    MaterialTypeVariable(rapidxml::xml_node<> *node);

    // id
    std::string id;
    // short name
    std::string shortname;
    // default value
    double default_value;
};

// boundary condition type variable
struct BoundaryTypeVariable
{
    BoundaryTypeVariable() : id(""), shortname(""), default_value(0) {}
    BoundaryTypeVariable(std::string id, std::string shortname,
                         double default_value = 0);
    BoundaryTypeVariable(rapidxml::xml_node<> *node);

    // id
    std::string id;
    // short name
    std::string shortname;
    // default value
    double default_value;
};

// boundary condition type
struct BoundaryType
{
    BoundaryType() : id(""), name("") {}
    BoundaryType(Hermes::vector<BoundaryTypeVariable> boundary_type_variables,
                 rapidxml::xml_node<> *node,
                 CoordinateType problem_type);
    ~BoundaryType();

    // id
    std::string id;
    // name
    std::string name;

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
        Expression(rapidxml::xml_node<> *node, CoordinateType problem_type);

        // expressions
        std::string scalar;
    };

    Integral() : id(""), name(""), shortname(""), shortname_html(""), unit(""), unit_html(""), expression(Expression()) {}
    Integral(rapidxml::xml_node<> *node, CoordinateType coordinateType, AnalysisType analysisType);

    // id
    std::string id;
    // name
    std::string name;
    // short name
    std::string shortname;
    std::string shortname_html;
    // unit
    std::string unit;
    std::string unit_html;

    // expressions
    Expression expression;
};

// dialog UI
struct DialogUI
{
    DialogUI() {}
    DialogUI(rapidxml::xml_node<> *node);

    struct Row
    {
        Row(std::string id, bool nonlin, bool timedep, std::string name, std::string shortname, std::string shortname_html,
                 std::string unit, std::string unit_html, std::string unit_latex,
                 double default_value, std::string condition)
            : id(id), nonlin(nonlin), timedep(timedep), name(name), shortname(shortname), shortname_html(shortname_html),
              unit(unit), unit_html(unit_html), unit_latex(unit_latex),
              default_value(default_value), condition(condition) {}
        Row(rapidxml::xml_node<> *quantity);

        std::string id;

        bool nonlin;
        bool timedep;

        std::string name;
        std::string shortname;
        std::string shortname_html;

        std::string unit;
        std::string unit_html;
        std::string unit_latex;

        double default_value;
        std::string condition;
    };

    std::map<std::string, Hermes::vector<Row> > groups;

    void clear();
};

// basic module
struct Module
{
    // id
    std::string fieldid;
    // name
    std::string name;
    // deformed shape
    bool deformed_shape;
    // description
    std::string description;

    // analyses
    std::map<std::string, std::string> analyses;
    int steady_state_solutions;
    int harmonic_solutions;
    int transient_solutions;

    // constants
    std::map<std::string, double> constants;

    // macros
    std::map<std::string, std::string> macros;

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
    Module(CoordinateType problemType, AnalysisType analysisType);
    ~Module();

    // parser  TODO move this method to FieldInfo???
    mu::Parser *get_parser(FieldInfo* fieldInfo);

    // read form xml
    void read(std::string filename);
    // clear
    void clear();

    inline CoordinateType get_coordinate_type() const { return m_coordinateType; }
    inline AnalysisType get_analysis_type() const { return m_analysisType; }

    // variable by name
    LocalVariable *get_variable(std::string fieldid);
    BoundaryType *get_boundary_type(std::string fieldid);
    BoundaryTypeVariable *get_boundary_type_variable(std::string fieldid);
    MaterialTypeVariable *get_material_type_variable(std::string fieldid);

    // expression
    std::string get_expression(LocalVariable *physicFieldVariable,
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
};

}
}

// available modules
std::map<std::string, std::string> availableModules();
// available analyses
std::map<std::string, std::string> availableAnalyses(std::string fieldId);

class Parser
{
public:
    // parser
    Hermes::vector<mu::Parser *> parser;
    std::map<std::string, double> parser_variables;

    Parser(FieldInfo *fieldInfo);
    ~Parser();

    void initParserBoundaryVariables(Boundary *boundary);
    void initParserMaterialVariables();
    void setParserVariables(Material *material, Boundary *boundary,
                            double value = 0.0, double dx = 0.0, double dy = 0.0);

private:
    FieldInfo *fieldInfo;
};

template <typename Scalar>
class ViewScalarFilter : public Hermes::Hermes2D::Filter<Scalar>
{
public:
    ViewScalarFilter(FieldInfo *fieldInfo,
                     Hermes::vector<Hermes::Hermes2D::MeshFunction<Scalar> *> sln,
                     std::string expression);
    ~ViewScalarFilter();

    double get_pt_value(double x, double y, int item = Hermes::Hermes2D::H2D_FN_VAL);

protected:
    typename Hermes::Hermes2D::Function<Scalar>::Node* node;

    double px;
    double py;
    double *pvalue;
    double *pdx;
    double *pdy;

    Parser *parser;

    void initParser(std::string expression);
    void precalculate(int order, int mask);

private:
    FieldInfo *m_fieldInfo;
};

// mesh fix
void readMeshDirtyFix();

// read mesh
Hermes::Hermes2D::Mesh *readMeshFromFile(const QString &fileName);
void writeMeshFromFile(const QString &fileName, Hermes::Hermes2D::Mesh *mesh);

void refineMesh(FieldInfo *fieldInfo, Hermes::Hermes2D::Mesh *mesh, bool refineGlobal, bool refineTowardsEdge);

// return geom type
Hermes::Hermes2D::GeomType convertProblemType(CoordinateType problemType);

#endif // HERMES_FIELD_H
