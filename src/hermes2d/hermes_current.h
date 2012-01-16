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

#ifndef HERMES_CURRENT_H
#define HERMES_CURRENT_H

#include "util.h"
#include "hermes_field.h"

struct HermesCurrent : public HermesField
{
    Q_OBJECT
public:
    PhysicField physicField() const { return PhysicField_Current; }

    inline int numberOfSolution() const { return 1; }
    inline bool hasSteadyState() const { return true; }
    inline bool hasHarmonic() const { return false; }
    inline bool hasTransient() const { return false; }
    inline bool hasNonlinearity() const { return false; }
    inline bool hasParticleTracing() const { return true; }

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

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PhysicFieldBC_Heat_Temperature ||
                                                                          physicFieldBC == PhysicFieldBC_Heat_Flux); }
    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PhysicFieldVariable_Current_Potential ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Current_ElectricField ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Current_CurrentDensity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Current_Losses ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Current_Conductivity); }

    SceneBoundary *newBoundary();
    SceneBoundary *newBoundary(PyObject *self, PyObject *args);
    SceneBoundary *modifyBoundary(PyObject *self, PyObject *args);
    SceneMaterial *newMaterial();
    SceneMaterial *newMaterial(PyObject *self, PyObject *args);
    SceneMaterial *modifyMaterial(PyObject *self, PyObject *args);

    QList<SolutionArray *> solve(ProgressItemSolve *progressItemSolve);

    inline PhysicFieldVariable contourPhysicFieldVariable() { return PhysicFieldVariable_Current_Potential; }
    inline PhysicFieldVariable scalarPhysicFieldVariable() { return PhysicFieldVariable_Current_Potential; }
    inline PhysicFieldVariableComp scalarPhysicFieldVariableComp() { return PhysicFieldVariableComp_Scalar; }
    inline PhysicFieldVariable vectorPhysicFieldVariable() { return PhysicFieldVariable_Current_CurrentDensity; }

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Current_Potential), PhysicFieldVariable_Current_Potential);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Current_ElectricField), PhysicFieldVariable_Current_ElectricField);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Current_CurrentDensity), PhysicFieldVariable_Current_CurrentDensity);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Current_Losses), PhysicFieldVariable_Current_Losses);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Current_Conductivity), PhysicFieldVariable_Current_Conductivity);
    }

    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Current_ElectricField), PhysicFieldVariable_Current_ElectricField);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Current_CurrentDensity), PhysicFieldVariable_Current_CurrentDensity);
    }

    void showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue);
    void showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue);
    void showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue);

    ViewScalarFilter *viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);

    Point3 particleForce(Point point, Point3 velocity = Point3());
    double particleMaterial(Point point);
};

class LocalPointValueCurrent : public LocalPointValue
{
public:
    double conductivity;
    double losses;
    double potential;
    Point J;
    Point E;

    LocalPointValueCurrent(const Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
};

class SurfaceIntegralValueCurrent : public SurfaceIntegralValue
{
protected:
    void calculateVariables(int i);

public:
    double current;

    SurfaceIntegralValueCurrent();

    QStringList variables();
};

class VolumeIntegralValueCurrent : public VolumeIntegralValue
{
protected:
    void calculateVariables(int i);
    void initSolutions();

public:
    double powerLosses;

    VolumeIntegralValueCurrent();

    QStringList variables();
};

class ViewScalarFilterCurrent : public ViewScalarFilter
{
public:
    ViewScalarFilterCurrent(Hermes::vector<MeshFunction *> sln, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) :
            ViewScalarFilter(sln, physicFieldVariable, physicFieldVariableComp) {};

protected:
    void calculateVariable(int i);
};

class SceneBoundaryCurrent : public SceneBoundary {

public:
    Value value;

    SceneBoundaryCurrent(const QString &name, PhysicFieldBC type, Value value);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneMaterialCurrent : public SceneMaterial
{
public:
    Value conductivity;

    SceneMaterialCurrent(const QString &name, Value conductivity);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneBoundaryCurrentDialog : public SceneBoundaryDialog
{
    Q_OBJECT

public:
    SceneBoundaryCurrentDialog(SceneBoundaryCurrent *boundary, QWidget *parent);

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

class SceneMaterialCurrentDialog : public SceneMaterialDialog
{
    Q_OBJECT

public:
    SceneMaterialCurrentDialog(SceneMaterialCurrent *material, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    ValueLineEdit *txtConductivity;
};

#endif // HERMES_CURRENT_H
