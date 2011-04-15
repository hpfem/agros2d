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
    PhysicField physicField() const { return PhysicField_General; }

    inline int numberOfSolution() const { return 1; }
    inline bool hasSteadyState() const { return true; }
    inline bool hasHarmonic() const { return false; }
    inline bool hasTransient() const { return false; }

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

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PhysicFieldBC_General_Value ||
                                                                          physicFieldBC == PhysicFieldBC_General_Derivative); }
    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PhysicFieldVariable_Variable ||
                                                                                            physicFieldVariable == PhysicFieldVariable_General_Gradient ||
                                                                                            physicFieldVariable == PhysicFieldVariable_General_Constant); }

    SceneBoundary *newBoundary();
    SceneBoundary *newBoundary(PyObject *self, PyObject *args);
    SceneBoundary *modifyBoundary(PyObject *self, PyObject *args);
    SceneMaterial *newMaterial();
    SceneMaterial *newMaterial(PyObject *self, PyObject *args);
    SceneMaterial *modifyMaterial(PyObject *self, PyObject *args);

    QList<SolutionArray *> solve(ProgressItemSolve *progressItemSolve);

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

    LocalPointValueGeneral(const Point &point);
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

class SceneBoundaryGeneral : public SceneBoundary
{
public:
    Value value;

    SceneBoundaryGeneral(const QString &name, PhysicFieldBC type, Value value);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneMaterialGeneral : public SceneMaterial
{
public:
    Value rightside;
    Value constant;

    SceneMaterialGeneral(const QString &name, Value rightside, Value constant);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneBoundaryGeneralDialog : public SceneBoundaryDialog
{
    Q_OBJECT
public:
    SceneBoundaryGeneralDialog(SceneBoundaryGeneral *edgeGeneralMarker, QWidget *parent);

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

class SceneMaterialGeneralDialog : public SceneMaterialDialog
{
    Q_OBJECT
public:
    SceneMaterialGeneralDialog(QWidget *parent, SceneMaterialGeneral *labelGeneralMarker);

protected:
    void createContent();

    void load();
    bool save();

private:
    ValueLineEdit *txtConstant;
    ValueLineEdit *txtRightSide;
};

#endif // GENERAL_H
