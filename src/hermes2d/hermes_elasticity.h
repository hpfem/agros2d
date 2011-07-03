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

#ifndef HERMES_ELASTICITY_H
#define HERMES_ELASTICITY_H

#include "util.h"
#include "module.h"

struct ModuleElasticity : public Hermes::Module::ModuleAgros
{
    Q_OBJECT
public:
    ModuleElasticity(ProblemType problemType, AnalysisType analysisType) : Hermes::Module::ModuleAgros(problemType, analysisType) {}

    inline int number_of_solution() const { return 2; }
    bool has_nonlinearity() const { return false; }

    LocalPointValue *local_point_value(const Point &point);
    SurfaceIntegralValue *surface_integral_value();
    VolumeIntegralValue *volume_integral_value();

    ViewScalarFilter *view_scalar_filter(Hermes::Module::LocalVariable *physicFieldVariable,
                                         PhysicFieldVariableComp physicFieldVariableComp);

    void deform_shape(double3* linVert, int count);
    void deform_shape(double4* linVert, int count);

    Hermes::vector<SolutionArray *> solve(ProgressItemSolve *progressItemSolve);

    // rewrite
    void readBoundaryFromDomElement(QDomElement *element);
    void writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker);
    void readMaterialFromDomElement(QDomElement *element);
    void writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker);

    SceneBoundary *newBoundary();
    SceneBoundary *newBoundary(PyObject *self, PyObject *args);
    SceneBoundary *modifyBoundary(PyObject *self, PyObject *args);
    SceneMaterial *newMaterial();
    SceneMaterial *newMaterial(PyObject *self, PyObject *args);
    SceneMaterial *modifyMaterial(PyObject *self, PyObject *args);
};

// *******************************************************************************************

class ParserElasticity : public Parser
{
public:
    double pe;
    double pnu;
    double pfx;
    double pfy;
    double palpha;
    double pt;
    double ptref;

    void setParserVariables(SceneMaterial *material);
};

class LocalPointValueElasticity : public LocalPointValue
{
public:
    LocalPointValueElasticity(const Point &point);
};

class SurfaceIntegralValueElasticity : public SurfaceIntegralValue
{
public:
    SurfaceIntegralValueElasticity();
};

class VolumeIntegralValueElasticity : public VolumeIntegralValue
{
public:
    VolumeIntegralValueElasticity();
};

class ViewScalarFilterElasticity : public ViewScalarFilter
{
public:
    ViewScalarFilterElasticity(Hermes::vector<MeshFunction *> sln,
                                   std::string expression);
};

class SceneBoundaryElasticity : public SceneBoundary
{
public:
    PhysicFieldBC typeX;
    PhysicFieldBC typeY;
    Value forceX;
    Value forceY;
    Value displacementX;
    Value displacementY;

    SceneBoundaryElasticity(const QString &name, PhysicFieldBC typeX, PhysicFieldBC typeY,
                              Value forceX, Value forceY, Value displacementX, Value displacementY);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneMaterialElasticity : public SceneMaterial
{
public:
    Value young_modulus;
    Value poisson_ratio;
    Value forceX;
    Value forceY;
    Value alpha;
    Value temp;
    Value temp_ref;

    SceneMaterialElasticity(const QString &name, Value young_modulus, Value poisson_ratio, Value forceX, Value forceY,
                               Value alpha, Value temp, Value temp_ref);

    // Lame constant
    inline double lambda() const { return (young_modulus.number * poisson_ratio.number) / ((1.0 + poisson_ratio.number) * (1.0 - 2.0*poisson_ratio.number)); }
    inline double mu() const { return young_modulus.number / (2.0*(1.0 + poisson_ratio.number)); }

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneBoundaryElasticityDialog : public SceneBoundaryDialog
{
    Q_OBJECT

public:
    SceneBoundaryElasticityDialog(SceneBoundaryElasticity *boundary, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    QLabel *lblEquationX;
    QLabel *lblEquationImageX;
    QLabel *lblEquationY;
    QLabel *lblEquationImageY;
    QComboBox *cmbTypeX;
    QComboBox *cmbTypeY;
    ValueLineEdit *txtForceX;
    ValueLineEdit *txtForceY;
    ValueLineEdit *txtDisplacementX;
    ValueLineEdit *txtDisplacementY;

private slots:
    void doTypeXChanged(int index);
    void doTypeYChanged(int index);
};

class SceneMaterialElasticityDialog : public SceneMaterialDialog
{
    Q_OBJECT

public:
    SceneMaterialElasticityDialog(SceneMaterialElasticity *material, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    ValueLineEdit *txtYoungModulus;
    ValueLineEdit *txtPoissonNumber;
    ValueLineEdit *txtForceX;
    ValueLineEdit *txtForceY;
    ValueLineEdit *txtAlpha;
    ValueLineEdit *txtTemp;
    ValueLineEdit *txtTempRef;
};

#endif // HERMES_ELASTICITY_H
