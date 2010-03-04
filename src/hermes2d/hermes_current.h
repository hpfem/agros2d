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
    HermesCurrent() { physicField = PhysicField_Current; }

    inline int numberOfSolution() { return 1; }
    bool hasHarmonic() { return false; }
    bool hasTransient() { return false; }

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

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PhysicFieldBC_Magnetic_Temperature ||
                                                                          physicFieldBC == PhysicFieldBC_Heat_Flux); }
    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PhysicFieldVariable_Current_Potential ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Current_ElectricField ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Current_CurrentDensity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Current_Losses ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Current_Conductivity); }

    SceneEdgeMarker *newEdgeMarker();
    SceneEdgeMarker *newEdgeMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *newLabelMarker();
    SceneLabelMarker *newLabelMarker(PyObject *self, PyObject *args);

    QList<SolutionArray *> *solve(SolverDialog *solverDialog);

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
};

class LocalPointValueCurrent : public LocalPointValue
{
public:
    double conductivity;
    double losses;
    double potential;
    Point J;
    Point E;

    LocalPointValueCurrent(Point &point);
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
    double losses;

    VolumeIntegralValueCurrent();

    QStringList variables();
};

class ViewScalarFilterCurrent : public ViewScalarFilter
{
public:
    ViewScalarFilterCurrent(MeshFunction *sln1, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) :
            ViewScalarFilter(sln1, physicFieldVariable, physicFieldVariableComp) {};
    ViewScalarFilterCurrent(MeshFunction *sln1, MeshFunction *sln2, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) :
            ViewScalarFilter(sln1, sln2, physicFieldVariable, physicFieldVariableComp) {};

protected:
    void calculateVariable(int i);
};

class SceneEdgeCurrentMarker : public SceneEdgeMarker {

public:
    Value value;

    SceneEdgeCurrentMarker(const QString &name, PhysicFieldBC type, Value value);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneLabelCurrentMarker : public SceneLabelMarker
{
public:
    Value conductivity;

    SceneLabelCurrentMarker(const QString &name, Value conductivity);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class DSceneEdgeCurrentMarker : public DSceneEdgeMarker
{
    Q_OBJECT

public:
    DSceneEdgeCurrentMarker(SceneEdgeCurrentMarker *edgeCurrentMarker, QWidget *parent);
    ~DSceneEdgeCurrentMarker();

protected:
    void createContent();

    void load();
    bool save();

private:
    QComboBox *cmbType;
    SLineEditValue *txtValue;
};

class DSceneLabelCurrentMarker : public DSceneLabelMarker
{
    Q_OBJECT

public:
    DSceneLabelCurrentMarker(QWidget *parent, SceneLabelCurrentMarker *labelCurrentMarker);
    ~DSceneLabelCurrentMarker();

protected:
    void createContent();

    void load();
    bool save();

private:
    SLineEditValue *txtConductivity;
};

#endif // HERMES_CURRENT_H
