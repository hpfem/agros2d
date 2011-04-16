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
    PhysicField physicField() const { return PhysicField_Acoustic; }

    inline int numberOfSolution() const { return 2; }
    inline bool hasSteadyState() const { return false; }
    inline bool hasHarmonic() const { return true; }
    inline bool hasTransient() const
    {
#ifdef BETA
    return true;
#else
     return false;
#endif
    }

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

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PhysicFieldBC_Acoustic_Pressure ||
                                                                          physicFieldBC == PhysicFieldBC_Acoustic_NormalAcceleration ||
                                                                          physicFieldBC == PhysicFieldBC_Acoustic_Impedance ||
                                                                          physicFieldBC == PhysicFieldBC_Acoustic_MatchedBoundary); }
    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PhysicFieldVariable_Acoustic_PressureReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Acoustic_Pressure ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Acoustic_PressureReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Acoustic_PressureImag ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Acoustic_LocalVelocity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Acoustic_LocalAcceleration ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Acoustic_PressureLevel ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Acoustic_Density ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Acoustic_Speed); }

    SceneBoundary *newBoundary();
    SceneBoundary *newBoundary(PyObject *self, PyObject *args);
    SceneBoundary *modifyBoundary(PyObject *self, PyObject *args);
    SceneMaterial *newMaterial();
    SceneMaterial *newMaterial(PyObject *self, PyObject *args);
    SceneMaterial *modifyMaterial(PyObject *self, PyObject *args);

    QList<SolutionArray *> solve(ProgressItemSolve *progressItemSolve);
    virtual void updateTimeFunctions(WeakFormAgros *wf, double time, Hermes::vector<Solution *> sln);

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

class SceneBoundaryAcoustic : public SceneBoundary
{
public:
    Value value_real;

    SceneBoundaryAcoustic(const QString &name, PhysicFieldBC type, Value value_real);
    SceneBoundaryAcoustic(const QString &name, PhysicFieldBC type);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneMaterialAcoustic : public SceneMaterial
{
public:
    Value density;
    Value speed;

    SceneMaterialAcoustic(const QString &name, Value density, Value speed);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneBoundaryAcousticDialog : public SceneBoundaryDialog
{
    Q_OBJECT

public:
    SceneBoundaryAcousticDialog(SceneBoundaryAcoustic *boundary, QWidget *parent);

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

class SceneMaterialAcousticDialog : public SceneMaterialDialog
{
    Q_OBJECT

public:
    SceneMaterialAcousticDialog(SceneMaterialAcoustic *material, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    ValueLineEdit *txtDensity;
    ValueLineEdit *txtSpeed;
};

#endif // ACOUSTIC_H
