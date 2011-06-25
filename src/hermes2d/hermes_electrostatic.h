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

#ifndef ELECTROSTATIC_H
#define ELECTROSTATIC_H

#include "util.h"
#include "hermes_field.h"

struct HermesElectrostatic : public HermesField
{
    Q_OBJECT
public:
    PhysicField physicField() const { return PhysicField_Electrostatic; }

    inline int numberOfSolution() const { return 1; }
    inline bool hasSteadyState() const { return true; }
    inline bool hasHarmonic() const { return false; }
    inline bool hasTransient() const { return false; }
    inline bool hasNonlinearity() const { return false; }

    void readBoundaryFromDomElement(QDomElement *element);
    void writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker);
    void readMaterialFromDomElement(QDomElement *element);
    void writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker);

    LocalPointValue *localPointValue(const Point &point);
    QStringList localPointValueHeader();

    SurfaceIntegralValue *surfaceIntegralValue();
    QStringList surfaceIntegralValueHeader();

    VolumeIntegralValue *volumeIntegralValue();
    QStringList volumeIntegralValueHeader();

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PhysicFieldBC_Electrostatic_Potential ||
                                                                          physicFieldBC == PhysicFieldBC_Electrostatic_SurfaceCharge); }
    inline bool physicFieldVariableCheck(PhysicFieldVariableDeprecated physicFieldVariable) { return (physicFieldVariable == PhysicFieldVariable_Electrostatic_Potential ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Electrostatic_ElectricField ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Electrostatic_Displacement ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Electrostatic_EnergyDensity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Electrostatic_Permittivity); }

    SceneBoundary *newBoundary();
    SceneBoundary *newBoundary(PyObject *self, PyObject *args);
    SceneBoundary *modifyBoundary(PyObject *self, PyObject *args);
    SceneMaterial *newMaterial();
    SceneMaterial *newMaterial(PyObject *self, PyObject *args);
    SceneMaterial *modifyMaterial(PyObject *self, PyObject *args);

    QList<SolutionArray *> solve(ProgressItemSolve *progressItemSolve);

    inline PhysicFieldVariableDeprecated contourPhysicFieldVariable() { return PhysicFieldVariable_Electrostatic_Potential; }
    inline PhysicFieldVariableDeprecated scalarPhysicFieldVariable() { return PhysicFieldVariable_Electrostatic_Potential; } // PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL
    inline PhysicFieldVariableComp scalarPhysicFieldVariableComp() { return PhysicFieldVariableComp_Scalar; }
    inline PhysicFieldVariableDeprecated vectorPhysicFieldVariable() { return PhysicFieldVariable_Electrostatic_ElectricField; }

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Electrostatic_Potential), PhysicFieldVariable_Electrostatic_Potential);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Electrostatic_ElectricField), PhysicFieldVariable_Electrostatic_ElectricField);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Electrostatic_Displacement), PhysicFieldVariable_Electrostatic_Displacement);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Electrostatic_EnergyDensity), PhysicFieldVariable_Electrostatic_EnergyDensity);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Electrostatic_Permittivity), PhysicFieldVariable_Electrostatic_Permittivity);
    }

    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Electrostatic_ElectricField), PhysicFieldVariable_Electrostatic_ElectricField);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Electrostatic_Displacement), PhysicFieldVariable_Electrostatic_Displacement);
    }

    void showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue);
    void showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue);
    void showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue);

    ViewScalarFilter *viewScalarFilter(Hermes::Module::PhysicFieldVariable *physicFieldVariable,
                                       PhysicFieldVariableComp physicFieldVariableComp);
};

class LocalPointValueElectrostatic : public LocalPointValue
{
public:
    double charge_density;
    double permittivity;
    double potential;
    Point E;
    Point D;
    double we;

    LocalPointValueElectrostatic(const Point &point);
    double variableValue(PhysicFieldVariableDeprecated physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
};

class SurfaceIntegralValueElectrostatic : public SurfaceIntegralValue
{
protected:
    void calculateVariables(int i);

public:
    double surfaceCharge;

    SurfaceIntegralValueElectrostatic();

    QStringList variables();
};

class VolumeIntegralValueElectrostatic : public VolumeIntegralValue
{
protected:
    void calculateVariables(int i);
    void initSolutions();

public:
    double energy;

    VolumeIntegralValueElectrostatic();

    QStringList variables();
};

class ViewScalarFilterElectrostatic : public ViewScalarFilter
{
public:
    ViewScalarFilterElectrostatic(Hermes::vector<MeshFunction *> sln,
                                  Hermes::Module::PhysicFieldVariable *physicFieldVariable,
                                  PhysicFieldVariableComp physicFieldVariableComp) :
            ViewScalarFilter(sln, physicFieldVariable, physicFieldVariableComp) {};

protected:
    void calculateVariable(int i);
};

class SceneBoundaryElectrostatic : public SceneBoundary
{
public:
    Value value;

    SceneBoundaryElectrostatic(const QString &name, PhysicFieldBC type, Value value);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneMaterialElectrostatic : public SceneMaterial
{
public:
    Value charge_density;
    Value permittivity;

    SceneMaterialElectrostatic(const QString &name, Value charge_density, Value permittivity);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneBoundaryElectrostaticDialog : public SceneBoundaryDialog
{
    Q_OBJECT
public:
    SceneBoundaryElectrostaticDialog(SceneBoundaryElectrostatic *material, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    QLabel *lblValueUnit;
    QComboBox *cmbType;
    ValueLineEdit *txtValue;

private slots:
    void doTypeChanged(int index);
};

class SceneMaterialElectrostaticDialog : public SceneMaterialDialog
{
    Q_OBJECT
public:
    SceneMaterialElectrostaticDialog(SceneMaterialElectrostatic *material, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    ValueLineEdit *txtPermittivity;
    ValueLineEdit *txtChargeDensity;
};

#endif // ELECTROSTATIC_H
