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

#ifndef MAGNETIC_H
#define MAGNETIC_H

#include "util.h"
#include "hermes_field.h"

struct HermesMagnetic : public HermesField
{
    Q_OBJECT
public:
    HermesMagnetic() { physicField = PhysicField_Magnetic; }

    int numberOfSolution();
    bool hasHarmonic() { return true; }
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

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PhysicFieldBC_Magnetic_VectorPotential ||
                                                                          physicFieldBC == PhysicFieldBC_Magnetic_SurfaceCurrent); }
    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PhysicFieldVariable_Magnetic_VectorPotential ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_VectorPotentialReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_VectorPotentialImag ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_FluxDensity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_FluxDensityReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_FluxDensityImag ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_MagneticField ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_MagneticFieldReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_MagneticFieldImag ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_CurrentDensityTotal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_CurrentDensityTotalReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_CurrentDensityTotalImag ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformImag ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityImag ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_PowerLosses ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_EnergyDensity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_Permeability ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_Conductivity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_Remanence ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_Velocity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Magnetic_LorentzForce); }

    SceneEdgeMarker *newEdgeMarker();
    SceneEdgeMarker *newEdgeMarker(PyObject *self, PyObject *args);
    SceneEdgeMarker *modifyEdgeMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *newLabelMarker();
    SceneLabelMarker *newLabelMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *modifyLabelMarker(PyObject *self, PyObject *args);

    QList<SolutionArray *> *solve(ProgressItemSolve *progressItemSolve);

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

class LocalPointValueMagnetic : public LocalPointValue
{
public:
    double permeability;
    double conductivity;
    double remanence;
    double remanence_angle;
    Point velocity;

    double potential_real;
    double potential_imag;

    double current_density_real;
    double current_density_imag;
    double current_density_total_real;
    double current_density_total_imag;
    double current_density_induced_transform_real;
    double current_density_induced_transform_imag;
    double current_density_induced_velocity_real;
    double current_density_induced_velocity_imag;

    Point H_real;
    Point H_imag;
    Point B_real;
    Point B_imag;
    Point FL_real;
    Point FL_imag;
    double pj;
    double wm;

    LocalPointValueMagnetic(Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
};

class SurfaceIntegralValueMagnetic : public SurfaceIntegralValue
{
protected:
    void calculateVariables(int i);

public:
    double forceMaxwellX;
    double forceMaxwellY;

    SurfaceIntegralValueMagnetic();
    QStringList variables();
};

class VolumeIntegralValueMagnetic : public VolumeIntegralValue
{
protected:
    void calculateVariables(int i);
    void initSolutions();

public:
    double currentReal;
    double currentImag;
    double currentInducedTransformReal;
    double currentInducedTransformImag;
    double currentInducedVelocityReal;
    double currentInducedVelocityImag;
    double currentTotalReal;
    double currentTotalImag;
    double forceLorentzX;
    double forceLorentzY;
    double powerLosses;
    double energy;
    double torque;

    VolumeIntegralValueMagnetic();
    QStringList variables();
};

class ViewScalarFilterMagnetic : public ViewScalarFilter
{
public:
    ViewScalarFilterMagnetic(MeshFunction *sln1, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) :
            ViewScalarFilter(sln1, physicFieldVariable, physicFieldVariableComp) {};
    ViewScalarFilterMagnetic(MeshFunction *sln1, MeshFunction *sln2, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) :
            ViewScalarFilter(sln1, sln2, physicFieldVariable, physicFieldVariableComp) {};

protected:
    void calculateVariable(int i);
};

class SceneEdgeMagneticMarker : public SceneEdgeMarker
{
public:
    Value value_real;
    Value value_imag;

    SceneEdgeMagneticMarker(const QString &name, PhysicFieldBC type, Value value_real, Value value_imag);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneLabelMagneticMarker : public SceneLabelMarker
{
public:
    Value permeability;
    Value conductivity;
    Value current_density_real;
    Value current_density_imag;
    Value remanence;
    Value remanence_angle;
    Value velocity_x;
    Value velocity_y;
    Value velocity_angular;

    SceneLabelMagneticMarker(const QString &name, Value current_density_real, Value current_density_imag, Value permeability, Value conductivity,
                             Value remanence, Value remanence_angle, Value velocity_x, Value velocity_y, Value velocity_angular);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class DSceneEdgeMagneticMarker : public DSceneEdgeMarker
{
    Q_OBJECT

public:
    DSceneEdgeMagneticMarker(SceneEdgeMagneticMarker *edgeMagneticMarker, QWidget *parent);
    ~DSceneEdgeMagneticMarker();

protected:
    void createContent();

    void load();
    bool save();

private:
    QComboBox *cmbType;
    SLineEditValue *txtValueReal;
    SLineEditValue *txtValueImag;
};

class DSceneLabelMagneticMarker : public DSceneLabelMarker
{
    Q_OBJECT

public:
    DSceneLabelMagneticMarker(QWidget *parent, SceneLabelMagneticMarker *labelMagneticMarker);
    ~DSceneLabelMagneticMarker();

protected:
    void createContent();

    void load();
    bool save();

private:
    SLineEditValue *txtPermeability;
    SLineEditValue *txtConductivity;
    SLineEditValue *txtCurrentDensityReal;
    SLineEditValue *txtCurrentDensityImag;
    SLineEditValue *txtRemanence;
    SLineEditValue *txtRemanenceAngle;
    SLineEditValue *txtVelocityX;
    SLineEditValue *txtVelocityY;
    SLineEditValue *txtVelocityAngular;
};

#endif // MAGNETIC_H
