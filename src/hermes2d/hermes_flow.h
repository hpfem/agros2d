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

#ifndef HERMES_FLOW_H
#define HERMES_FLOW_H

#include "util.h"
#include "module.h"

struct HermesFlow : public HermesField
{
    Q_OBJECT
public:
    PhysicField physicField() const { return PhysicField_Flow; }

    inline int numberOfSolution() const { return 3; }
    inline bool hasSteadyState() const { return true; }
    inline bool hasHarmonic() const { return false; }
    inline bool hasTransient() const { return true; }

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

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PhysicFieldBC_Flow_Velocity ||
                                                                          physicFieldBC == PhysicFieldBC_Flow_Pressure ||
                                                                          physicFieldBC == PhysicFieldBC_Flow_Outlet ||
                                                                          physicFieldBC == PhysicFieldBC_Flow_Wall); }
    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PhysicFieldVariable_Flow_Velocity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Flow_VelocityX ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Flow_VelocityY ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Flow_Pressure); }

    SceneBoundary *newBoundary();
    SceneBoundary *modifyBoundary(PyObject *self, PyObject *args);
    SceneMaterial *modifyMaterial(PyObject *self, PyObject *args);

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Flow_Velocity), PhysicFieldVariable_Flow_Velocity);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Flow_Pressure), PhysicFieldVariable_Flow_Pressure);
    }

    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Flow_Velocity), PhysicFieldVariable_Flow_Velocity);
    }

    void showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue);
    void showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue);
    void showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue);

    ViewScalarFilter *viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
};

class LocalPointValueFlow : public LocalPointValue
{
public:
    double velocity_x;
    double velocity_y;
    double pressure;

    double dynamic_viscosity;
    double density;

    LocalPointValueFlow(const Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
};

class SurfaceIntegralValueFlow : public SurfaceIntegralValue
{
protected:
    void calculateVariables(int i);

public:
    SurfaceIntegralValueFlow();

    QStringList variables();
};

class VolumeIntegralValueFlow : public VolumeIntegralValue
{
protected:
    void calculateVariables(int i);
    void initSolutions();

public:
    VolumeIntegralValueFlow();
    QStringList variables();
};

class ViewScalarFilterFlow : public ViewScalarFilter
{
public:
    ViewScalarFilterFlow(Hermes::vector<MeshFunction *> sln, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) :
            ViewScalarFilter(sln, physicFieldVariable, physicFieldVariableComp) {}

protected:
    void calculateVariable(int i);
};

class SceneEdgeFlowMarker : public SceneBoundary
{
public:
    Value velocityX;
    Value velocityY;
    Value pressure;

    QString script();
    QMap<QString, QString> data();
    SceneEdgeFlowMarker(const QString &name, PhysicFieldBC type, Value velocityX, Value velocityY, Value pressure);

    int showDialog(QWidget *parent);
};

class SceneLabelFlowMarker : public SceneMaterial
{
public:
    Value dynamic_viscosity;
    Value density;

    SceneLabelFlowMarker(const QString &name, Value dynamic_viscosity, Value density);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class DSceneEdgeFlowMarker : public SceneBoundaryDialog
{
    Q_OBJECT

public:
    DSceneEdgeFlowMarker(SceneEdgeFlowMarker *edgeEdgeFlowMarker, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    QComboBox *cmbType;
    ValueLineEdit *txtVelocityX;
    ValueLineEdit *txtVelocityY;
    ValueLineEdit *txtPressure;

private slots:
    void doTypeChanged(int index);
};

class DSceneLabelFlowMarker : public SceneMaterialDialog
{
    Q_OBJECT

public:
    DSceneLabelFlowMarker(QWidget *parent, SceneLabelFlowMarker *labelFlowMarker);

protected:
    void createContent();

    void load();
    bool save();

private:
    ValueLineEdit *txtDynamicViscosity;
    ValueLineEdit *txtDensity;
};

#endif // HERMES_FLOW_H
