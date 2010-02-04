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
    HermesElectrostatic() { physicField = PHYSICFIELD_ELECTROSTATIC; }

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

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL ||
                                                                          physicFieldBC == PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE); }
    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY); }

    SceneEdgeMarker *newEdgeMarker();
    SceneEdgeMarker *newEdgeMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *newLabelMarker();
    SceneLabelMarker *newLabelMarker(PyObject *self, PyObject *args);

    QList<SolutionArray *> *solve(SolverDialog *solverDialog);

    inline PhysicFieldVariable contourPhysicFieldVariable() { return PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL; }
    inline PhysicFieldVariable scalarPhysicFieldVariable() { return PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL; } // PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL
    inline PhysicFieldVariableComp scalarPhysicFieldVariableComp() { return PHYSICFIELDVARIABLECOMP_SCALAR; }
    inline PhysicFieldVariable vectorPhysicFieldVariable() { return PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD; }

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL), PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD), PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT), PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY), PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY), PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY);
    }

    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD), PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT), PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT);
    }

    void showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue);
    void showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue);
    void showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue);

    ViewScalarFilter *viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
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

    LocalPointValueElectrostatic(Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
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
    ViewScalarFilterElectrostatic(MeshFunction *sln1, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) :
            ViewScalarFilter(sln1, physicFieldVariable, physicFieldVariableComp) {};
    ViewScalarFilterElectrostatic(MeshFunction *sln1, MeshFunction *sln2, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) :
            ViewScalarFilter(sln1, sln2, physicFieldVariable, physicFieldVariableComp) {};
protected:
    void calculateVariable(int i);
};

class SceneEdgeElectrostaticMarker : public SceneEdgeMarker
{
public:
    Value value;

    SceneEdgeElectrostaticMarker(const QString &name, PhysicFieldBC type, Value value);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneLabelElectrostaticMarker : public SceneLabelMarker
{
public:
    Value charge_density;
    Value permittivity;

    SceneLabelElectrostaticMarker(const QString &name, Value charge_density, Value permittivity);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class DSceneEdgeElectrostaticMarker : public DSceneEdgeMarker
{
    Q_OBJECT
public:
    DSceneEdgeElectrostaticMarker(SceneEdgeElectrostaticMarker *edgeElectrostaticMarker, QWidget *parent);
    ~DSceneEdgeElectrostaticMarker();

protected:
    void createContent();

    void load();
    bool save();

private:
    QComboBox *cmbType;
    SLineEditValue *txtValue;
};

class DSceneLabelElectrostaticMarker : public DSceneLabelMarker
{
    Q_OBJECT
public:
    DSceneLabelElectrostaticMarker(QWidget *parent, SceneLabelElectrostaticMarker *labelElectrostaticMarker);
    ~DSceneLabelElectrostaticMarker();

protected:
    void createContent();

    void load();
    bool save();

private:
    SLineEditValue *txtPermittivity;
    SLineEditValue *txtChargeDensity;
};

#endif // ELECTROSTATIC_H
