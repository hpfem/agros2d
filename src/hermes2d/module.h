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
#include "weakform_library/weakforms_h1.h"
#include "weakform_library/weakforms_maxwell.h"
#include "weakform_library/weakforms_elasticity.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenesolution.h"
#include "scenemarker.h"

#include "hermes2d/localpoint.h"
#include "hermes2d/volumeintegral.h"
#include "hermes2d/surfaceintegral.h"

#include <rapidxml.cpp>
#include <rapidxml_utils.cpp>

#include "material.h"
#include "boundary.h"
#include "localpoint.h"
#include "weakform_parser.h"

extern double actualTime;

class SceneBoundary;
class SceneMaterial;
struct SceneViewSettings;
template <typename Scalar> struct SolutionArray;

class ProgressItemSolve;

//template <typename Scalar>
//class ViewScalarFilter;

template <typename Scalar>
class WeakFormAgros : public Hermes::Hermes2D::WeakForm<Scalar>
{
public:
    WeakFormAgros(unsigned int neq = 1) : Hermes::Hermes2D::WeakForm<Scalar>(neq) { }

    void registerForms();

    // previous solution
    Hermes::vector<Hermes::Hermes2D::MeshFunction<Scalar> *> solution;
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
        Expression(rapidxml::xml_node<> *node, ProblemType problem_type);

        // expressions
        std::string scalar;
        std::string comp_x;
        std::string comp_y;
    };

    LocalVariable(std::string id = "", std::string name = "", std::string shortname = "", std::string unit = "")
        : id(id), name(name), shortname(shortname), unit(unit), is_scalar(true), expression(Expression()) {}
    LocalVariable(rapidxml::xml_node<> *node, ProblemType problemType, AnalysisType analysisType);

    // id
    std::string id;
    // name
    std::string name;
    // short name
    std::string shortname;
    // unit
    std::string unit;

    // is scalar variable
    bool is_scalar;

    // expressions
    Expression expression;
};

// material property
struct MaterialTypeVariable
{
    MaterialTypeVariable() : id(""), name(""), shortname(""), unit("") {}
    MaterialTypeVariable(std::string id, std::string name,
                         std::string shortname, std::string unit);
    MaterialTypeVariable(rapidxml::xml_node<> *node);

    // id
    std::string id;
    // name
    std::string name;
    // short name
    std::string shortname;
    // unit
    std::string unit;
};

// boundary condition type variable
struct BoundaryTypeVariable
{
    BoundaryTypeVariable() : id(""), name(""), shortname(""), unit("") {}
    BoundaryTypeVariable(std::string id, std::string name,
                         std::string shortname, std::string unit);
    BoundaryTypeVariable(rapidxml::xml_node<> *node);

    // id
    std::string id;
    // name
    std::string name;
    // short name
    std::string shortname;
    // unit
    std::string unit;
};

// boundary condition type
struct BoundaryType
{
    BoundaryType() : id(""), name("") {}
    BoundaryType(Hermes::vector<BoundaryTypeVariable> boundary_type_variables,
                 rapidxml::xml_node<> *node,
                 ProblemType problem_type);
    ~BoundaryType();

    // id
    std::string id;
    // name
    std::string name;

    // variables
    Hermes::vector<BoundaryTypeVariable *> variables;

    // weakform
    Hermes::vector<ParserFormMatrix *> weakform_matrix_surface;
    Hermes::vector<ParserFormVector *> weakform_vector_surface;

    // essential
    std::map<int, BoundaryTypeVariable *> essential;
};

// surface and volume integral value
struct Integral
{
    struct Expression
    {
        Expression() : scalar("") {}
        Expression(rapidxml::xml_node<> *node, ProblemType problem_type);

        // expressions
        std::string scalar;
    };

    Integral() : id(""), name(""), shortname(""), unit(""), expression(Expression()) {}
    Integral(rapidxml::xml_node<> *node, ProblemType problemType, AnalysisType analysisType);

    // id
    std::string id;
    // name
    std::string name;
    // short name
    std::string shortname;
    // unit
    std::string unit;

    // expressions
    Expression expression;    
};

// basic module
struct Module
{
    // id
    std::string id;
    // name
    std::string name;
    // description
    std::string description;

    int steady_state_solutions;
    int harmonic_solutions;
    int transient_solutions;

    // constants
    std::map<std::string, double> constants;

    // material type
    Hermes::vector<MaterialTypeVariable *> material_type_variables;

    // boundary conditions
    Hermes::vector<BoundaryTypeVariable *> boundary_type_variables;
    Hermes::vector<BoundaryType *> boundary_types;

    // weak forms
    Hermes::vector<ParserFormMatrix *> weakform_matrix_volume;
    Hermes::vector<ParserFormVector *> weakform_vector_volume;

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

    // default contructor
    Module(ProblemType problemType, AnalysisType analysisType);
    ~Module();

    // read form xml
    void read(std::string filename);
    // clear
    void clear();

    inline ProblemType get_problem_type() const { return m_problemType; }
    inline AnalysisType get_analysis_type() const { return m_analysisType; }

    // variable by name
    LocalVariable *get_variable(std::string id);
    BoundaryType *get_boundary_type(std::string id);
    BoundaryTypeVariable *get_boundary_type_variable(std::string id);
    MaterialTypeVariable *get_material_type_variable(std::string id);

    // parser
    mu::Parser *get_parser();

    // expression
    std::string get_expression(LocalVariable *physicFieldVariable,
                               PhysicFieldVariableComp physicFieldVariableComp);

    int number_of_solution() const;

    Hermes::vector<SolutionArray<double> *> solve(ProgressItemSolve *progressItemSolve);  //TODO PK <Scalar>
    bool solve_init_variables();

    inline virtual void update_time_functions(double time) {}


    ViewScalarFilter<double> *view_scalar_filter(Hermes::Module::LocalVariable *physicFieldVariable,  //TODO PK <Scalar>
                                                 PhysicFieldVariableComp physicFieldVariableComp);

    virtual inline void deform_shape(double3* linVert, int count) {}
    virtual inline void deform_shape(double4* linVert, int count) {}

private:
    ProblemType m_problemType;
    AnalysisType m_analysisType;
};

struct ModuleAgros : public QObject, public Module
{
    Q_OBJECT
public:
    ModuleAgros(ProblemType problemType, AnalysisType analysisType) : Module(problemType, analysisType) {}

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable);
    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable);
    void fillComboBoxBoundaryCondition(QComboBox *cmbFieldVariable);
    void fillComboBoxMaterialProperties(QComboBox *cmbFieldVariable);

    // rewrite
    virtual SceneBoundary *newBoundary() = 0;
    virtual SceneBoundary *newBoundary(PyObject *self, PyObject *args) = 0;
    virtual SceneBoundary *modifyBoundary(PyObject *self, PyObject *args) = 0;
    virtual SceneMaterial *newMaterial() = 0;
    virtual SceneMaterial *newMaterial(PyObject *self, PyObject *args) = 0;
    virtual SceneMaterial *modifyMaterial(PyObject *self, PyObject *args) = 0;

private:
    void fillComboBox(QComboBox *cmbFieldVariable, Hermes::vector<Hermes::Module::LocalVariable *> list);
};

}
}

// module factory
Hermes::Module::ModuleAgros *moduleFactory(std::string id, ProblemType problem_type, AnalysisType analysis_type);

// boundary dialog factory
SceneBoundaryDialog *boundaryDialogFactory(SceneBoundary *scene_boundary, QWidget *parent);

// material dialog factory
SceneMaterialDialog *materialDialogFactory(SceneMaterial *scene_material, QWidget *parent);

// available modules
std::map<std::string, std::string> availableModules();

class Parser
{
public:
    // parser
    Hermes::vector<mu::Parser *> parser;
    std::map<std::string, double> parser_variables;

    Parser();
    ~Parser();

    void initParserBoundaryVariables(Boundary *boundary);
    void initParserMaterialVariables();
    void setParserVariables(Material *material, Boundary *boundary);
};

template <typename Scalar>
class ViewScalarFilter : public Hermes::Hermes2D::Filter<Scalar>
{
public:  
    ViewScalarFilter(Hermes::vector<Hermes::Hermes2D::MeshFunction<Scalar> *> sln,
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
};

// mesh fix
void readMeshDirtyFix();

// read mesh
Hermes::Hermes2D::Mesh *readMeshFromFile(const QString &fileName);
void writeMeshFromFile(const QString &fileName, Hermes::Hermes2D::Mesh *mesh);

void refineMesh(Hermes::Hermes2D::Mesh *mesh, bool refineGlobal, bool refineTowardsEdge);

// return geom type
Hermes::Hermes2D::GeomType convertProblemType(ProblemType problemType);

// solve
template <typename Scalar>
Hermes::vector<SolutionArray<Scalar> *> solveSolutioArray(ProgressItemSolve *progressItemSolve,
                                                          Hermes::vector<Hermes::Hermes2D::EssentialBCs<Scalar> > bcs,
                                                          WeakFormAgros<Scalar> *wf);

// solve
template <typename Scalar>
class SolutionAgros
{
public:
    SolutionAgros(ProgressItemSolve *progressItemSolve, WeakFormAgros<Scalar> *wf);

    Hermes::vector<SolutionArray<Scalar> *> solveSolutioArray(Hermes::vector<Hermes::Hermes2D::EssentialBCs<Scalar> > bcs);
private:
    int polynomialOrder;
    AdaptivityType adaptivityType;
    int adaptivitySteps;
    double adaptivityTolerance;
    int adaptivityMaxDOFs;
    int numberOfSolution;
    double timeTotal;
    double timeStep;
    double initialCondition;

    AnalysisType analysisType;

    double nonlinearTolerance;
    int nonlinearSteps;

    Hermes::MatrixSolverType matrixSolver;

    // error
    bool isError;

    // mesh file
    Hermes::Hermes2D::Mesh *mesh;

    // weak form
    WeakFormAgros<Scalar> *m_wf;
    ProgressItemSolve *m_progressItemSolve;

    SolutionArray<Scalar> *solutionArray(Hermes::Hermes2D::Solution<Scalar> *sln, Hermes::Hermes2D::Space<Scalar> *space = NULL, double adaptiveError = 0.0, double adaptiveSteps = 0.0, double time = 0.0);

    bool solveLinear(Hermes::Hermes2D::DiscreteProblem<Scalar> *dp,
                     Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> space,
                     Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solution,
                     Hermes::Solvers::LinearSolver<Scalar> *solver, SparseMatrix<Scalar> *matrix, Vector<Scalar> *rhs);
};

#endif // HERMES_FIELD_H
