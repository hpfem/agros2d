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
#include "hermes_field.h"

struct HermesHeat : public HermesField
{
    Q_OBJECT
public:    
    PhysicField physicField() const { return PhysicField_Heat; }

    inline int numberOfSolution() const { return 1; }
    inline bool hasSteadyState() const { return true; }
    inline bool hasHarmonic() const { return false; }
    inline bool hasTransient() const { return true; }

    void readEdgeMarkerFromDomElement(QDomElement *element);
    void writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker);
    void readLabelMarkerFromDomElement(QDomElement *element);
    void writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker);

    LocalPointValue *localPointValue(const Point &point);
    QStringList localPointValueHeader();

    SurfaceIntegralValue *surfaceIntegralValue();
    QStringList surfaceIntegralValueHeader();

    VolumeIntegralValue *volumeIntegralValue();
    QStringList volumeIntegralValueHeader();

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PhysicFieldBC_Heat_Temperature ||
                                                                          physicFieldBC == PhysicFieldBC_Heat_Flux); }
    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PhysicFieldVariable_Heat_Temperature ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Heat_TemperatureGradient ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Heat_Flux ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Heat_Conductivity); }

    SceneEdgeMarker *newEdgeMarker();
    SceneEdgeMarker *newEdgeMarker(PyObject *self, PyObject *args);
    SceneEdgeMarker *modifyEdgeMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *newLabelMarker();
    SceneLabelMarker *newLabelMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *modifyLabelMarker(PyObject *self, PyObject *args);

    QList<SolutionArray *> solve(ProgressItemSolve *progressItemSolve);
    virtual void updateTimeFunctions(double time);

    inline PhysicFieldVariable contourPhysicFieldVariable() { return PhysicFieldVariable_Heat_Temperature; }
    inline PhysicFieldVariable scalarPhysicFieldVariable() { return PhysicFieldVariable_Heat_Temperature; }
    inline PhysicFieldVariableComp scalarPhysicFieldVariableComp() { return PhysicFieldVariableComp_Scalar; }
    inline PhysicFieldVariable vectorPhysicFieldVariable() { return PhysicFieldVariable_Heat_Flux; }

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Heat_Temperature), PhysicFieldVariable_Heat_Temperature);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Heat_TemperatureGradient), PhysicFieldVariable_Heat_TemperatureGradient);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Heat_Flux), PhysicFieldVariable_Heat_Flux);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Heat_Conductivity), PhysicFieldVariable_Heat_Conductivity);
    }

    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Heat_TemperatureGradient), PhysicFieldVariable_Heat_TemperatureGradient);
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Heat_Flux), PhysicFieldVariable_Heat_Flux);
    }

    void showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue);
    void showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue);
    void showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue);

    ViewScalarFilter *viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
};

class LocalPointValueHeat : public LocalPointValue
{
public:
    double volume_heat;
    double thermal_conductivity;
    double temperature;
    Point F;
    Point G;

    LocalPointValueHeat(const Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
};

class SurfaceIntegralValueHeat : public SurfaceIntegralValue
{
protected:
    void calculateVariables(int i);

public:
    double averageTemperature;
    double temperatureDifference;
    double heatFlux;

    SurfaceIntegralValueHeat();

    QStringList variables();
};

class VolumeIntegralValueHeat : public VolumeIntegralValue
{
protected:
    void calculateVariables(int i);
    void initSolutions();

public:
    double averageTemperature;
    double averageTemperatureGradientX;
    double averageTemperatureGradientY;
    double averageTemperatureGradient;
    double averageHeatFluxX;
    double averageHeatFluxY;
    double averageHeatFlux;

    VolumeIntegralValueHeat();
    QStringList variables();
};

class ViewScalarFilterHeat : public ViewScalarFilter
{
public:
    ViewScalarFilterHeat(Hermes::vector<MeshFunction *> sln, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) :
            ViewScalarFilter(sln, physicFieldVariable, physicFieldVariableComp) {};

protected:
    void calculateVariable(int i);
};

class SceneEdgeHeatMarker : public SceneEdgeMarker
{
public:
    Value temperature;
    Value heatFlux;
    Value h;
    Value externalTemperature;

    SceneEdgeHeatMarker(const QString &name, PhysicFieldBC type, Value temperature);
    SceneEdgeHeatMarker(const QString &name, PhysicFieldBC type, Value heatFlux, Value h, Value externalTemperature);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneLabelHeatMarker : public SceneLabelMarker
{
public:
    Value thermal_conductivity;
    Value volume_heat;
    Value density;
    Value specific_heat;

    SceneLabelHeatMarker(const QString &name, Value volume_heat, Value thermal_conductivity, Value density, Value specific_heat);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class DSceneEdgeHeatMarker : public DSceneEdgeMarker
{
    Q_OBJECT

public:
    DSceneEdgeHeatMarker(SceneEdgeHeatMarker *edgeEdgeHeatMarker, QWidget *parent);
    ~DSceneEdgeHeatMarker();

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

class DSceneLabelHeatMarker : public DSceneLabelMarker
{
    Q_OBJECT

public:
    DSceneLabelHeatMarker(QWidget *parent, SceneLabelHeatMarker *labelHeatMarker);

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
