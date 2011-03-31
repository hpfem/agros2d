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

#ifndef ACOUSTIC_H
#define ACOUSTIC_H

#include "util.h"
#include "hermes_field.h"

struct HermesAcoustic : public HermesField
{
    Q_OBJECT
public:
    HermesAcoustic() { m_physicField = PhysicField_Acoustic; }
    virtual ~HermesAcoustic() {}

    inline int numberOfSolution() { return 2; }
    bool hasSteadyState() { return false; }
    bool hasHarmonic() { return true; }
    bool hasTransient()
    {
#ifdef BETA
        return true;
#else
    return false
#endif
    }

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

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PhysicFieldBC_Acoustic_Pressure ||
                                                                          physicFieldBC == PhysicFieldBC_Acoustic_NormalAcceleration ||
                                                                          physicFieldBC == PhysicFieldBC_Acoustic_Impedance); }
    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PhysicFieldVariable_Acoustic_PressureReal); }

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

class LocalPointValueAcoustic : public LocalPointValue
{
public:
    double density;
    double speed;
    double pressure_real;
    double pressure_imag;
    double pressureLevel;
    Point localVelocity;
    Point localAccelaration;

    LocalPointValueAcoustic(const Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
};

class SurfaceIntegralValueAcoustic : public SurfaceIntegralValue
{
protected:
    void calculateVariables(int i);

public:
    SurfaceIntegralValueAcoustic();
    QStringList variables();
};

class VolumeIntegralValueAcoustic : public VolumeIntegralValue
{
protected:
    void calculateVariables(int i);
    void initSolutions();

public:
    VolumeIntegralValueAcoustic();
    QStringList variables();
};

class ViewScalarFilterAcoustic : public ViewScalarFilter
{
public:
    ViewScalarFilterAcoustic(Hermes::vector<MeshFunction *> sln, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) :
            ViewScalarFilter(sln, physicFieldVariable, physicFieldVariableComp) {};

protected:
    void calculateVariable(int i);
};

class SceneEdgeAcousticMarker : public SceneEdgeMarker
{
public:
    Value value_real;

    // transient
    Value transient_amplitude;
    Value transient_frequency;

    SceneEdgeAcousticMarker(const QString &name, PhysicFieldBC type, Value value_real, Value transient_amplitude, Value transient_frequency);
    SceneEdgeAcousticMarker(const QString &name, PhysicFieldBC type, Value value_real);
    SceneEdgeAcousticMarker(const QString &name, PhysicFieldBC type);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneLabelAcousticMarker : public SceneLabelMarker
{
public:
    Value density;
    Value speed;

    SceneLabelAcousticMarker(const QString &name, Value density, Value speed);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class DSceneEdgeAcousticMarker : public DSceneEdgeMarker
{
    Q_OBJECT

public:
    DSceneEdgeAcousticMarker(SceneEdgeAcousticMarker *edgeAcousticMarker, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    QComboBox *cmbType;
    SLineEditValue *txtValue;
    SLineEditValue *txtTransientHarmonicAmplitude;
    SLineEditValue *txtTransientHarmonicFrequency;

private slots:
    void doTypeChanged(int index);
};

class DSceneLabelAcousticMarker : public DSceneLabelMarker
{
    Q_OBJECT

public:
    DSceneLabelAcousticMarker(QWidget *parent, SceneLabelAcousticMarker *labelAcousticMarker);

protected:
    void createContent();

    void load();
    bool save();

private:
    SLineEditValue *txtDensity;
    SLineEditValue *txtSpeed;
};

#endif // ACOUSTIC_H
