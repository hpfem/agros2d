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

#ifndef MAGNETIC_H
#define MAGNETIC_H

#include "util.h"
#include "module.h"

struct ModuleMagnetic : public Hermes::Module::ModuleAgros
{
    Q_OBJECT
public:
    ModuleMagnetic(ProblemType problemType, AnalysisType analysisType) : Hermes::Module::ModuleAgros(problemType, analysisType) {}

    int number_of_solution() const;
    bool has_nonlinearity() const { return false; }

    LocalPointValue *local_point_value(const Point &point);
    SurfaceIntegralValue *surface_integral_value();
    VolumeIntegralValue *volume_integral_value();

    ViewScalarFilter *view_scalar_filter(Hermes::Module::LocalVariable *physicFieldVariable,
                                         PhysicFieldVariableComp physicFieldVariableComp);

    Hermes::vector<SolutionArray *> solve(ProgressItemSolve *progressItemSolve);
    void update_time_functions(double time);

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

class ParserMagnetic : public Parser
{
public: 
    double pmur;
    double pgamma;
    double pjer;
    double pjei;
    double pbrx;
    double pbry;
    double pvx;
    double pvy;
    double pva;

    void setParserVariables(SceneMaterial *material);
};

class LocalPointValueMagnetic : public LocalPointValue
{
public:
    LocalPointValueMagnetic(const Point &point);
};

class SurfaceIntegralValueMagnetic : public SurfaceIntegralValue
{
public:
    SurfaceIntegralValueMagnetic();
};

class VolumeIntegralValueMagnetic : public VolumeIntegralValue
{
public:
    VolumeIntegralValueMagnetic();
};

class ViewScalarFilterMagnetic : public ViewScalarFilter
{
public:
    ViewScalarFilterMagnetic(Hermes::vector<MeshFunction *> sln,
                                   std::string expression);
};

// *******************************************************************************************

class SceneBoundaryMagnetic : public SceneBoundary
{
public:
    Value value_real;
    Value value_imag;

    SceneBoundaryMagnetic(const QString &name, PhysicFieldBC type, Value value_real, Value value_imag);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneMaterialMagnetic : public SceneMaterial
{
public:
    Value permeability;
    Value conductivity;
    Value current_density_real;
    Value current_density_imag;
    Value remanence;
    Value remanence_angle;
    Value velocity_x;
    Value velocity_y;
    Value velocity_angular;

    SceneMaterialMagnetic(const QString &name, Value current_density_real, Value current_density_imag, Value permeability, Value conductivity,
                             Value remanence, Value remanence_angle, Value velocity_x, Value velocity_y, Value velocity_angular);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

// *******************************************************************************************

class SceneEdgeMagneticDialog : public SceneBoundaryDialog
{
    Q_OBJECT

public:
    SceneEdgeMagneticDialog(SceneBoundaryMagnetic *boundary, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    QLabel *lblValueUnit;
    QComboBox *cmbType;
    ValueLineEdit *txtValueReal;
    ValueLineEdit *txtValueImag;

private slots:
    void doTypeChanged(int index);
};

class SceneMaterialMagneticDialog : public SceneMaterialDialog
{
    Q_OBJECT

public:
    SceneMaterialMagneticDialog(QWidget *parent, SceneMaterialMagnetic *material);

protected:
    void createContent();

    void load();
    bool save();

private:
    ValueLineEdit *txtPermeability;
    ValueLineEdit *txtConductivity;
    ValueLineEdit *txtCurrentDensityReal;
    ValueLineEdit *txtCurrentDensityImag;
    ValueLineEdit *txtRemanence;
    ValueLineEdit *txtRemanenceAngle;
    ValueLineEdit *txtVelocityX;
    ValueLineEdit *txtVelocityY;
    ValueLineEdit *txtVelocityAngular;
};

#endif // MAGNETIC_H
