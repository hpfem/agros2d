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

#include "localvalueview.h"
#include "surfaceintegralview.h"
#include "volumeintegralview.h"

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

struct LocalVariable
{
    struct Expression
    {
        Expression() : scalar(""), comp_x(""), comp_y("") {}

        Expression(QDomElement *element, ProblemType problem_type);

        // expressions
        std::string scalar;
        std::string comp_x;
        std::string comp_y;

        void read(QDomElement *element, ProblemType problem_type);
    };

    LocalVariable() : id(""), name(""), shortname(""), unit(""),
        expression(Expression()) {}

    LocalVariable(QDomElement *element, ProblemType problemType, AnalysisType analysisType);

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

    void read(QDomElement *element, ProblemType problemType, AnalysisType analysisType);
};

struct Integral
{
    struct Expression
    {
        Expression() : scalar("") {}

        Expression(QDomElement *element, ProblemType problem_type);

        // expressions
        std::string scalar;

        void read(QDomElement *element, ProblemType problem_type);        
    };

    Integral() : id(""), name(""), shortname(""), unit(""),
        expression(Expression()) {}

    Integral(QDomElement *element, ProblemType problemType, AnalysisType analysisType);

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

    void read(QDomElement *element, ProblemType problemType, AnalysisType analysisType);
};

struct Module
{
    // name
    std::string name;
    // description
    std::string description;

    bool has_steady_state;
    bool has_harmonic;
    bool has_transient;

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
    void read(std::string file_name);
    // clear
    void clear();

    inline ProblemType get_problem_type() { return m_problemType; }
    inline AnalysisType get_analysis_type() { return m_analysisType; }

    // get variable by name
    LocalVariable *get_variable(std::string id);

    // get expression
    static std::string get_expression(LocalVariable *physicFieldVariable,
                                      PhysicFieldVariableComp physicFieldVariableComp);

private:
    ProblemType m_problemType;
    AnalysisType m_analysisType;
};

struct ModuleAgros : public Module
{
    ModuleAgros(ProblemType problemType, AnalysisType analysisType) : Module(problemType, analysisType) {}

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable);
    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable);

private:
    void fillComboBox(QComboBox *cmbFieldVariable, Hermes::vector<LocalVariable *> list);
};

}
}

struct HermesField : public QObject
{
    Q_OBJECT
public:
    virtual PhysicField physicField() const { return PhysicField_Undefined; }

    virtual int numberOfSolution() const = 0;
    virtual bool hasSteadyState() const = 0;
    virtual bool hasHarmonic() const = 0;
    virtual bool hasTransient() const = 0;
    virtual bool hasNonlinearity() const = 0;

    virtual void readBoundaryFromDomElement(QDomElement *element) = 0;
    virtual void writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker) = 0;
    virtual void readMaterialFromDomElement(QDomElement *element) = 0;
    virtual void writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker) = 0;

    virtual LocalPointValue *localPointValue(const Point &point) = 0;
    virtual QStringList localPointValueHeader() = 0;

    virtual SurfaceIntegralValue *surfaceIntegralValue() = 0;
    virtual QStringList surfaceIntegralValueHeader() = 0;

    virtual VolumeIntegralValue *volumeIntegralValue() = 0;
    virtual QStringList volumeIntegralValueHeader() = 0;

    virtual bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) = 0;
    virtual bool physicFieldVariableCheck(PhysicFieldVariableDeprecated physicFieldVariable) = 0;

    virtual SceneBoundary *newBoundary() = 0;
    virtual SceneBoundary *newBoundary(PyObject *self, PyObject *args) = 0;
    virtual SceneBoundary *modifyBoundary(PyObject *self, PyObject *args) = 0;
    virtual SceneMaterial *newMaterial() = 0;
    virtual SceneMaterial *newMaterial(PyObject *self, PyObject *args) = 0;
    virtual SceneMaterial *modifyMaterial(PyObject *self, PyObject *args) = 0;

    virtual QList<SolutionArray *> solve(ProgressItemSolve *progressItemSolve) = 0;
    inline virtual void updateTimeFunctions(double time) { }

    virtual ViewScalarFilter *viewScalarFilter(Hermes::Module::LocalVariable *physicFieldVariable,
                                               PhysicFieldVariableComp physicFieldVariableComp) = 0;

    virtual inline void deformShape(double3* linVert, int count) {}
    virtual inline void deformShape(double4* linVert, int count) {}
};

HermesField *hermesFieldFactory(PhysicField physicField);

class ViewScalarFilter : public Filter
{
public:
    ViewScalarFilter(Hermes::vector<MeshFunction *> sln,
                     std::string expression);

    double get_pt_value(double x, double y, int item = H2D_FN_VAL);

protected:
    Node* node;

    // parser variables
    mu::Parser parser;

    double px, py;
    double pvalue, pdx, pdy;

    void precalculate(int order, int mask);
    virtual void prepareParser(SceneMaterial *material) = 0;
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
QList<SolutionArray *> solveSolutioArray(ProgressItemSolve *progressItemSolve,
                                         Hermes::vector<EssentialBCs> bcs,
                                         WeakFormAgros *wf);

// solve
class SolutionAgros
{
public:
    SolutionAgros(ProgressItemSolve *progressItemSolve, WeakFormAgros *wf);

    QList<SolutionArray *> solveSolutioArray(Hermes::vector<EssentialBCs> bcs);
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
