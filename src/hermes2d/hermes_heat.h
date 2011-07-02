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

#ifndef HEAT_H
#define HEAT_H

#include "util.h"
#include "module.h"

struct ModuleHeat : public Hermes::Module::ModuleAgros
{
    Q_OBJECT
public:
    ModuleHeat(ProblemType problemType, AnalysisType analysisType) : Hermes::Module::ModuleAgros(problemType, analysisType) {}

    inline int number_of_solution() const { return 1; }
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

class ParserHeat : public Parser
{
public:
    double plambda;
    double prho;
    double pcp;
    double pq;

    void setParserVariables(SceneMaterial *material);
};

// *******************************************************************************************

class LocalPointValueHeat : public LocalPointValue
{
public:
    LocalPointValueHeat(const Point &point);
};

class SurfaceIntegralValueHeat : public SurfaceIntegralValue
{
public:
    SurfaceIntegralValueHeat();
};

class VolumeIntegralValueHeat : public VolumeIntegralValue
{
public:
    VolumeIntegralValueHeat();
};

class ViewScalarFilterHeat : public ViewScalarFilter
{
public:
    ViewScalarFilterHeat(Hermes::vector<MeshFunction *> sln,
                                 std::string expression);
};

// *******************************************************************************************

class SceneBoundaryHeat : public SceneBoundary
{
public:
    Value temperature;
    Value heatFlux;
    Value h;
    Value externalTemperature;

    SceneBoundaryHeat(const QString &name, PhysicFieldBC type, Value temperature);
    SceneBoundaryHeat(const QString &name, PhysicFieldBC type, Value heatFlux, Value h, Value externalTemperature);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneMaterialHeat : public SceneMaterial
{
public:
    Value thermal_conductivity;
    Value volume_heat;
    Value density;
    Value specific_heat;

    SceneMaterialHeat(const QString &name, Value volume_heat, Value thermal_conductivity, Value density, Value specific_heat);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneBoundaryHeatDialog : public SceneBoundaryDialog
{
    Q_OBJECT

public:
    SceneBoundaryHeatDialog(SceneBoundaryHeat *boundary, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    QComboBox *cmbType;
    ValueLineEdit *txtTemperature;
    ValueLineEdit *txtHeatFlux;
    ValueLineEdit *txtHeatTransferCoefficient;
    ValueLineEdit *txtExternalTemperature;

private slots:
    void doTypeChanged(int index);
};

class SceneMaterialHeatDialog : public SceneMaterialDialog
{
    Q_OBJECT

public:
    SceneMaterialHeatDialog(SceneMaterialHeat *material, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    ValueLineEdit *txtThermalConductivity;
    ValueLineEdit *txtVolumeHeat;
    ValueLineEdit *txtDensity;
    ValueLineEdit *txtSpecificHeat;
};

#endif // HEAT_H
