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

#ifndef RF_H
#define RF_H

#include "util.h"
#include "hermes_field.h"

struct HermesRF : public HermesField
{
    Q_OBJECT
public:
    HermesRF() { m_physicField = PhysicField_RF; }
    virtual ~HermesRF() {}

    int numberOfSolution();
    bool hasHarmonic() { return true; }
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

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PhysicFieldBC_RF_ElectricField ||
                                                                          physicFieldBC == PhysicFieldBC_RF_MagneticField ||
                                                                          physicFieldBC == PhysicFieldBC_RF_Port); }
    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PhysicFieldVariable_RF_MagneticField ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_MagneticFieldReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_MagneticFieldImag ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_ElectricField ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_ElectricFieldReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_ElectricFieldReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_PowerLosses ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_EnergyDensity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_Permittivity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_Permeability ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_Conductivity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_J_Ext_real ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_J_Ext_imag); }

    SceneEdgeMarker *newEdgeMarker();
    SceneEdgeMarker *newEdgeMarker(PyObject *self, PyObject *args);
    SceneEdgeMarker *modifyEdgeMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *newLabelMarker();
    SceneLabelMarker *newLabelMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *modifyLabelMarker(PyObject *self, PyObject *args);

    QList<SolutionArray *> solve(ProgressItemSolve *progressItemSolve);

    PhysicFieldVariable contourPhysicFieldVariable();
    PhysicFieldVariable scalarPhysicFieldVariable();
    PhysicFieldVariableComp scalarPhysicFieldVariableComp();
    PhysicFieldVariable vectorPhysicFieldVariable();

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable);
    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable);

    void showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue);
    void showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue);
    void showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue);

    ViewScalarFilter *viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
};

class LocalPointValueRF : public LocalPointValue
{
public:
    double permittivity;
    double permeability;
    double conductivity;
    double J_ext_real;
    double J_ext_imag;

    double potential_real;
    double potential_imag;

    double electric_field_real;
    double electric_field_imag;

    LocalPointValueRF(Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
};

class SurfaceIntegralValueRF : public SurfaceIntegralValue
{
protected:
    void calculateVariables(int i);

public:
    double forceMaxwellX;
    double forceMaxwellY;

    SurfaceIntegralValueRF();
    QStringList variables();
};

class VolumeIntegralValueRF : public VolumeIntegralValue
{
protected:
    void calculateVariables(int i);
    void initSolutions();

public:
    VolumeIntegralValueRF();
    QStringList variables();
};

class ViewScalarFilterRF : public ViewScalarFilter
{
public:
    ViewScalarFilterRF(Hermes::vector<MeshFunction *> sln, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) :
            ViewScalarFilter(sln, physicFieldVariable, physicFieldVariableComp) {};

protected:
    void calculateVariable(int i);
};

class SceneEdgeRFMarker : public SceneEdgeMarker
{
public:
    Value value_real;
    Value value_imag;
    Value power;
    Value phase;

    SceneEdgeRFMarker(const QString &name, PhysicFieldBC type, Value value_real, Value value_imag);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneLabelRFMarker : public SceneLabelMarker
{
public:
    Value permittivity;
    Value permeability;
    Value conductivity;
    Value J_ext_real;
    Value J_ext_imag;

    SceneLabelRFMarker(const QString &name, Value permittivity, Value permeability, Value conductivity, Value J_ext_real, Value J_ext_imag);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class DSceneEdgeRFMarker : public DSceneEdgeMarker
{
    Q_OBJECT

public:
    DSceneEdgeRFMarker(SceneEdgeRFMarker *edgeRFMarker, QWidget *parent);
    ~DSceneEdgeRFMarker();

protected:
    void createContent();

    void load();
    bool save();

private:
    QComboBox *cmbType;
    SLineEditValue *txtValueReal;
    SLineEditValue *txtValueImag;
};

class DSceneLabelRFMarker : public DSceneLabelMarker
{
    Q_OBJECT

public:
    DSceneLabelRFMarker(QWidget *parent, SceneLabelRFMarker *labelRFMarker);
    ~DSceneLabelRFMarker();

protected:
    void createContent();

    void load();
    bool save();

private:
    SLineEditValue *txtPermittivity;
    SLineEditValue *txtPermeability;
    SLineEditValue *txtConductivity;
    SLineEditValue *txtJ_Ext_real;
    SLineEditValue *txtJ_Ext_imag;
};

#endif // RF_H
