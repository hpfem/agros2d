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
#include "hermes_field.h"

struct HermesFlow : public HermesField
{
    Q_OBJECT
public:
    HermesFlow() { physicField = PhysicField_Flow; }

    inline int numberOfSolution() { return 3; }
    bool hasHarmonic() { return false; }
    bool hasTransient() { return true; }

    void readEdgeMarkerFromDomElement(QDomElement *element);
    void writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker);
    void readLabelMarkerFromDomElement(QDomElement *element);
    void writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker);

    LocalPointValue *localPointValue(Point point);
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

    SceneEdgeMarker *newEdgeMarker();
    SceneEdgeMarker *newEdgeMarker(PyObject *self, PyObject *args);
    SceneEdgeMarker *modifyEdgeMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *newLabelMarker();
    SceneLabelMarker *newLabelMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *modifyLabelMarker(PyObject *self, PyObject *args);

    QList<SolutionArray *> *solve(ProgressItemSolve *progressItemSolve);

    inline PhysicFieldVariable contourPhysicFieldVariable() { return PhysicFieldVariable_Flow_Velocity; }
    inline PhysicFieldVariable scalarPhysicFieldVariable() { return PhysicFieldVariable_Flow_Velocity; }
    inline PhysicFieldVariableComp scalarPhysicFieldVariableComp() { return PhysicFieldVariableComp_Scalar; }
    inline PhysicFieldVariable vectorPhysicFieldVariable() { return PhysicFieldVariable_Undefined; }

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Flow_Velocity), PhysicFieldVariable_Flow_Velocity);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Flow_VelocityX), PhysicFieldVariable_Flow_VelocityX);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Flow_VelocityY), PhysicFieldVariable_Flow_VelocityY);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Flow_Pressure), PhysicFieldVariable_Flow_Pressure);
    }

    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
    {
        // FIX ME
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Heat_TemperatureGradient), PhysicFieldVariable_Heat_TemperatureGradient);
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

    LocalPointValueFlow(Point &point);
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

class SceneEdgeFlowMarker : public SceneEdgeMarker
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

class SceneLabelFlowMarker : public SceneLabelMarker
{
public:
    Value dynamic_viscosity;
    Value density;

    SceneLabelFlowMarker(const QString &name, Value dynamic_viscosity, Value density);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class DSceneEdgeFlowMarker : public DSceneEdgeMarker
{
    Q_OBJECT

public:
    DSceneEdgeFlowMarker(SceneEdgeFlowMarker *edgeEdgeFlowMarker, QWidget *parent);
    ~DSceneEdgeFlowMarker();

protected:
    void createContent();

    void load();
    bool save();

private:
    QComboBox *cmbType;
    SLineEditValue *txtVelocityX;
    SLineEditValue *txtVelocityY;
    SLineEditValue *txtPressure;

private slots:
    void doTypeChanged(int index);
};

class DSceneLabelFlowMarker : public DSceneLabelMarker
{
    Q_OBJECT

public:
    DSceneLabelFlowMarker(QWidget *parent, SceneLabelFlowMarker *labelFlowMarker);
    ~DSceneLabelFlowMarker();

protected:
    void createContent();

    void load();
    bool save();

private:
    SLineEditValue *txtDynamicViscosity;
    SLineEditValue *txtDensity;
};

#endif // HERMES_FLOW_H
