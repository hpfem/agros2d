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

#ifndef GENERAL_H
#define GENERAL_H

#include "util.h"
#include "hermes_field.h"

struct HermesGeneral : public HermesField
{
    Q_OBJECT
public:
    HermesGeneral() { physicField = PhysicField_General; }

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

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PhysicFieldBC_General_Value ||
                                                                          physicFieldBC == PhysicFieldBC_General_Derivative); }
    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PhysicFieldVariable_Variable ||
                                                                                            physicFieldVariable == PhysicFieldVariable_General_Gradient ||
                                                                                            physicFieldVariable == PhysicFieldVariable_General_Constant); }

    SceneEdgeMarker *newEdgeMarker();
    SceneEdgeMarker *newEdgeMarker(PyObject *self, PyObject *args);
    SceneEdgeMarker *modifyEdgeMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *newLabelMarker();
    SceneLabelMarker *newLabelMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *modifyLabelMarker(PyObject *self, PyObject *args);

    QList<SolutionArray *> *solve(ProgressItemSolve *progressItemSolve);

    inline PhysicFieldVariable contourPhysicFieldVariable() { return PhysicFieldVariable_Variable; }
    inline PhysicFieldVariable scalarPhysicFieldVariable() { return PhysicFieldVariable_Variable; }
    inline PhysicFieldVariableComp scalarPhysicFieldVariableComp() { return PhysicFieldVariableComp_Scalar; }
    inline PhysicFieldVariable vectorPhysicFieldVariable() { return PhysicFieldVariable_General_Gradient; }

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Variable), PhysicFieldVariable_Variable);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_General_Gradient), PhysicFieldVariable_General_Gradient);
    }

    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_General_Gradient), PhysicFieldVariable_General_Gradient);
    }

    void showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue);
    void showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue);
    void showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue);

    ViewScalarFilter *viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
};

class LocalPointValueGeneral : public LocalPointValue
{
protected:
    void calculateVariables(int i);

public:
    double variable;
    double rightside;
    Point gradient;
    double constant;

    LocalPointValueGeneral(Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
};

class SurfaceIntegralValueGeneral : public SurfaceIntegralValue
{
protected:
    void calculateVariables(int i);

public:
    SurfaceIntegralValueGeneral();

    QStringList variables();
};

class VolumeIntegralValueGeneral : public VolumeIntegralValue
{
protected:
    void calculateVariables(int i);
    void initSolutions();

public:
    VolumeIntegralValueGeneral();

    QStringList variables();
};

class ViewScalarFilterGeneral : public ViewScalarFilter
{
public:
    ViewScalarFilterGeneral(Hermes::vector<MeshFunction *> sln, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) :
            ViewScalarFilter(sln, physicFieldVariable, physicFieldVariableComp) {};

protected:
    void calculateVariable(int i);
};

class SceneEdgeGeneralMarker : public SceneEdgeMarker
{
public:
    Value value;

    SceneEdgeGeneralMarker(const QString &name, PhysicFieldBC type, Value value);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneLabelGeneralMarker : public SceneLabelMarker
{
public:
    Value rightside;
    Value constant;

    SceneLabelGeneralMarker(const QString &name, Value rightside, Value constant);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class DSceneEdgeGeneralMarker : public DSceneEdgeMarker
{
    Q_OBJECT
public:
    DSceneEdgeGeneralMarker(SceneEdgeGeneralMarker *edgeGeneralMarker, QWidget *parent);
    ~DSceneEdgeGeneralMarker();

protected:
    void createContent();

    void load();
    bool save();

private:
    QComboBox *cmbType;
    SLineEditValue *txtValue;
};

class DSceneLabelGeneralMarker : public DSceneLabelMarker
{
    Q_OBJECT
public:
    DSceneLabelGeneralMarker(QWidget *parent, SceneLabelGeneralMarker *labelGeneralMarker);
    ~DSceneLabelGeneralMarker();

protected:
    void createContent();

    void load();
    bool save();

private:
    SLineEditValue *txtConstant;
    SLineEditValue *txtRightSide;
};

#endif // GENERAL_H
