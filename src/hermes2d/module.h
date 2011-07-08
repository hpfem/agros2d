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
#include "weakform_library/h1.h"
#include "weakform_library/maxwell.h"
#include "weakform_library/elasticity.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenesolution.h"
#include "scenemarker.h"

#include "hermes2d/localpoint.h"
#include "hermes2d/volumeintegral.h"
#include "hermes2d/surfaceintegral.h"

#include <rapidxml.cpp>
#include <rapidxml_utils.cpp>

extern double actualTime;

class SceneBoundary;
class SceneMaterial;
struct SceneViewSettings;
struct SolutionArray;

class ProgressItemSolve;

class ViewScalarFilter;

class WeakFormAgros : public WeakForm
{
public:
    WeakFormAgros(unsigned int neq = 1) : WeakForm(neq) { }

    virtual void registerForms() = 0;

    // previous solution
    Hermes::vector<Solution *> solution;
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
        Expression() : scalar(""), comp_x(""), comp_y("") {}
        Expression(rapidxml::xml_node<> *node, ProblemType problem_type);

        // expressions
        std::string scalar;
        std::string comp_x;
        std::string comp_y;
    };

    LocalVariable() : id(""), name(""), shortname(""), unit(""), expression(Expression()) {}
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
struct MaterialType
{
    MaterialType() : id(""), name(""), shortname(""), unit("") {}
    MaterialType(rapidxml::xml_node<> *node);

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
    BoundaryType(rapidxml::xml_node<> *node);
    ~BoundaryType();

    // id
    std::string id;
    // name
    std::string name;

    // variables
    Hermes::vector<BoundaryTypeVariable *> variables;
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

    bool has_steady_state;
    bool has_harmonic;
    bool has_transient;

    // constants
    std::map<std::string, double> constants;

    // material type
    Hermes::vector<MaterialType *> material_types;

    // boundary conditions
    Hermes::vector<BoundaryType *> boundary_types;

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
    MaterialType *get_material_type(std::string id);

    // parser
    mu::Parser *get_parser();

    // expression
    std::string get_expression(LocalVariable *physicFieldVariable,
                               PhysicFieldVariableComp physicFieldVariableComp);

    virtual int number_of_solution() const = 0;
    virtual bool has_nonlinearity() const = 0;

    virtual Hermes::vector<SolutionArray *> solve(ProgressItemSolve *progressItemSolve) = 0;
    bool solve_init_variables();

    inline virtual void update_time_functions(double time) {}


    ViewScalarFilter *view_scalar_filter(Hermes::Module::LocalVariable *physicFieldVariable,
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

    void setParserVariables(SceneMaterial *material);
};

class ViewScalarFilter : public Filter
{
public:  
    ViewScalarFilter(Hermes::vector<MeshFunction *> sln,
                     std::string expression);
    ~ViewScalarFilter();

    double get_pt_value(double x, double y, int item = H2D_FN_VAL);

protected:
    Node* node;

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
Mesh *readMeshFromFile(const QString &fileName);
void writeMeshFromFile(const QString &fileName, Mesh *mesh);

void refineMesh(Mesh *mesh, bool refineGlobal, bool refineTowardsEdge);

// return geom type
GeomType convertProblemType(ProblemType problemType);

// solve
Hermes::vector<SolutionArray *> solveSolutioArray(ProgressItemSolve *progressItemSolve,
                                                  Hermes::vector<EssentialBCs> bcs,
                                                  WeakFormAgros *wf);

// solve
class SolutionAgros
{
public:
    SolutionAgros(ProgressItemSolve *progressItemSolve, WeakFormAgros *wf);

    Hermes::vector<SolutionArray *> solveSolutioArray(Hermes::vector<EssentialBCs> bcs);
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

    LinearityType linearityType;
    double linearityNonlinearTolerance;
    int linearityNonlinearSteps;

    MatrixSolverType matrixSolver;

    // error
    bool isError;

    // mesh file
    Mesh *mesh;

    // weak form
    WeakFormAgros *m_wf;
    ProgressItemSolve *m_progressItemSolve;

    SolutionArray *solutionArray(Solution *sln, Space *space = NULL, double adaptiveError = 0.0, double adaptiveSteps = 0.0, double time = 0.0);

    bool solveLinear(DiscreteProblem *dp,
                     Hermes::vector<Space *> space,
                     Hermes::vector<Solution *> solution,
                     Solver *solver, SparseMatrix *matrix, Vector *rhs);

    bool solve(Hermes::vector<Space *> space,
               Hermes::vector<Solution *> solution,
               Solver *solver, SparseMatrix *matrix, Vector *rhs);
};

// custom forms **************************************************************************************************************************

class CustomVectorFormTimeDep : public WeakForm::VectorFormVol
{
public:
    CustomVectorFormTimeDep(int i, scalar coeff, Solution *solution, GeomType gt = HERMES_PLANAR)
        : WeakForm::VectorFormVol(i), coeff(coeff), gt(gt)
    {
        ext.push_back(solution);
    }
    CustomVectorFormTimeDep(int i, std::string area, scalar coeff, Solution *solution, GeomType gt = HERMES_PLANAR)
        : WeakForm::VectorFormVol(i, area), coeff(coeff), gt(gt)
    {
        ext.push_back(solution);
    }

    virtual scalar value(int n, double *wt, Func<scalar> *u_ext[], Func<double> *v,
                         Geom<double> *e, ExtData<scalar> *ext) const {
        if (gt == HERMES_PLANAR)
            return coeff * int_u_v<double, scalar>(n, wt, ext->fn[0], v);
        else if (gt == HERMES_AXISYM_X)
            return coeff * int_y_u_v<double, scalar>(n, wt, ext->fn[0], v, e);
        else
            return coeff * int_x_u_v<double, scalar>(n, wt, ext->fn[0], v, e);
    }

    virtual Ord ord(int n, double *wt, Func<Ord> *u_ext[], Func<Ord> *v, Geom<Ord> *e,
                    ExtData<Ord> *ext) const {
        if (gt == HERMES_PLANAR)
            return int_u_v<Ord, Ord>(n, wt, ext->fn[0], v);
        else if (gt == HERMES_AXISYM_X)
            return int_y_u_v<Ord, Ord>(n, wt, ext->fn[0], v, e);
        else
            return int_x_u_v<Ord, Ord>(n, wt, ext->fn[0], v, e);
    }

    // This is to make the form usable in rk_time_step().
    virtual WeakForm::VectorFormVol* clone() {
        return new CustomVectorFormTimeDep(*this);
    }

private:
    scalar coeff;
    GeomType gt;
};

#endif // HERMES_FIELD_H
