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
    PhysicField physicField() const { return PhysicField_RF; }

    int numberOfSolution() const;
    inline bool hasSteadyState() const { return false; }
    inline bool hasHarmonic() const { return true; }
    inline bool hasTransient() const { return false; }
    inline bool hasNonlinearity() const { return false; }

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

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PhysicFieldBC_RF_ElectricField ||
                                                                          physicFieldBC == PhysicFieldBC_RF_MagneticField ||
                                                                          physicFieldBC == PhysicFieldBC_RF_MatchedBoundary ||
                                                                          physicFieldBC == PhysicFieldBC_RF_Port); }
    inline bool teModeCheck(TEMode teMode) { return (teMode == TEMode_0 || teMode == TEMode_1 || teMode == TEMode_2); }

    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PhysicFieldVariable_RF_MagneticField ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_MagneticFieldXReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_MagneticFieldXImag ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_MagneticFieldXReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_MagneticFluxDensity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_MagneticFluxDensityXReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_MagneticFluxDensityXImag ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_MagneticFluxDensityYReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_MagneticFluxDensityYImag ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_ElectricField ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_ElectricFieldReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_ElectricFieldImag ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_PoyntingVector ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_PoyntingVectorReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_PoyntingVectorImag ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_PowerLosses ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_Permittivity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_Permeability ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_Conductivity ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_CurrentDensityReal ||
                                                                                            physicFieldVariable == PhysicFieldVariable_RF_CurrentDensityImag); }

    SceneBoundary *newBoundary();
    SceneBoundary *newBoundary(PyObject *self, PyObject *args);
    SceneBoundary *modifyBoundary(PyObject *self, PyObject *args);
    SceneMaterial *newMaterial();
    SceneMaterial *newMaterial(PyObject *self, PyObject *args);
    SceneMaterial *modifyMaterial(PyObject *self, PyObject *args);

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
    double current_density_real;
    double current_density_imag;

    double electric_field_real;
    double electric_field_imag;

    double magnetic_field_realX;
    double magnetic_field_imagX;
    double magnetic_field_realY;
    double magnetic_field_imagY;
    double flux_density_realX;
    double flux_density_imagX;
    double flux_density_realY;
    double flux_density_imagY;
    double poynting_vector_real;
    double poynting_vector_imag;

    LocalPointValueRF(const Point &point);
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

class SceneBoundaryRF : public SceneBoundary
{
public:
    Value value_real;
    Value value_imag;
    TEMode mode;
    Value power;
    Value phase;
    Value height;

    SceneBoundaryRF(const QString &name, PhysicFieldBC type, Value value_real, Value value_imag);
    SceneBoundaryRF(const QString &name, PhysicFieldBC type, TEMode mode, Value power, Value phase, Value height);
    SceneBoundaryRF(const QString &name, PhysicFieldBC type, Value height);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneMaterialRF : public SceneMaterial
{
public:
    Value permittivity;
    Value permeability;
    Value conductivity;
    Value current_density_real;
    Value current_density_imag;

    SceneMaterialRF(const QString &name, Value permittivity, Value permeability, Value conductivity, Value current_density_real, Value current_density_imag);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneBoundaryRFDialog : public SceneBoundaryDialog
{
    Q_OBJECT

public:
    SceneBoundaryRFDialog(SceneBoundaryRF *edgeRFMarker, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    QLabel *lblValueUnitReal;
    QLabel *lblValueUnitImag;
    QComboBox *cmbType;
    QComboBox *cmbMode;
    ValueLineEdit *txtValueReal;
    ValueLineEdit *txtValueImag;
    ValueLineEdit *txtHeight;


private slots:
    void doTypeChanged(int index);
};

class SceneMaterialRFDialog : public SceneMaterialDialog
{
    Q_OBJECT

public:
    SceneMaterialRFDialog(QWidget *parent, SceneMaterialRF *labelRFMarker);

protected:
    void createContent();

    void load();
    bool save();

private:
    ValueLineEdit *txtPermittivity;
    ValueLineEdit *txtPermeability;
    ValueLineEdit *txtConductivity;
    ValueLineEdit *txtCurrentDensityReal;
    ValueLineEdit *txtCurrentDensityImag;

};

#endif // RF_H
