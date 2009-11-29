#ifndef MAGNETOSTATIC_H
#define MAGNETOSTATIC_H

#include "util.h"
#include "hermes_field.h"

struct HermesMagnetostatic : public HermesField
{
    Q_OBJECT
public:
    HermesMagnetostatic() { physicField = PHYSICFIELD_MAGNETOSTATIC; }

    inline int numberOfSolution() { return 1; }
    bool hasFrequency() { return false; }
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

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL ||
                                                                          physicFieldBC == PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT); }
    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_MAGNETOSTATIC_CONDUCTIVITY ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_MAGNETOSTATIC_REMANENCE ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_MAGNETOSTATIC_REMANENCE_ANGLE ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_MAGNETOSTATIC_VELOCITY ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_MAGNETOSTATIC_CURRENT_DENSITY_VELOCITY ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY); }

    SceneEdgeMarker *newEdgeMarker();
    SceneEdgeMarker *newEdgeMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *newLabelMarker();
    SceneLabelMarker *newLabelMarker(PyObject *self, PyObject *args);

    QList<SolutionArray *> *solve(SolverThread *solverThread);

    inline PhysicFieldVariable contourPhysicFieldVariable() { return PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL; }
    inline PhysicFieldVariable scalarPhysicFieldVariable() { return PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY; }
    inline PhysicFieldVariableComp scalarPhysicFieldVariableComp() { return PHYSICFIELDVARIABLECOMP_MAGNITUDE; }
    inline PhysicFieldVariable vectorPhysicFieldVariable() { return PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY; }

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL), PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD), PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_REMANENCE), PHYSICFIELDVARIABLE_MAGNETOSTATIC_REMANENCE);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_REMANENCE_ANGLE), PHYSICFIELDVARIABLE_MAGNETOSTATIC_REMANENCE_ANGLE);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_CURRENT_DENSITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_CURRENT_DENSITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_CURRENT_DENSITY_VELOCITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_CURRENT_DENSITY_VELOCITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_CURRENT_DENSITY_TOTAL), PHYSICFIELDVARIABLE_MAGNETOSTATIC_CURRENT_DENSITY_TOTAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_POWER_LOSSES), PHYSICFIELDVARIABLE_MAGNETOSTATIC_POWER_LOSSES);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_CONDUCTIVITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_CONDUCTIVITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_VELOCITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_VELOCITY);
    }

    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD), PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_VELOCITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_VELOCITY);
    }

    void showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue);
    void showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue);
    void showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue);
};

class LocalPointValueMagnetostatic : public LocalPointValue
{
public:
    double current_density;
    double permeability;
    double conductivity;
    double remanence;
    double remanence_angle;
    Point velocity;
    double current_density_velocity;
    double potential;
    Point H;
    Point B;
    double wm;
    double pj;

    LocalPointValueMagnetostatic(Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
};

class SurfaceIntegralValueMagnetostatic : public SurfaceIntegralValue
{
public:
    SurfaceIntegralValueMagnetostatic();
    QStringList variables();
};

class VolumeIntegralValueMagnetostatic : public VolumeIntegralValue
{
public:
    double averageMagneticFieldX;
    double averageMagneticFieldY;
    double averageMagneticField;
    double averageFluxDensityX;
    double averageFluxDensityY;
    double averageFluxDensity;
    double energy;
    double forceX;
    double forceY;
    double currentExternal;
    double currentVelocity;
    double currentTotal;
    double losses;
    double torque;

    VolumeIntegralValueMagnetostatic();
    QStringList variables();
};

class SceneEdgeMagnetostaticMarker : public SceneEdgeMarker
{
public:
    Value value;

    SceneEdgeMagnetostaticMarker(const QString &name, PhysicFieldBC type, Value value);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneLabelMagnetostaticMarker : public SceneLabelMarker
{
public:
    Value permeability;
    Value current_density;
    Value remanence;
    Value remanence_angle;
    Value conductivity;
    Value velocity_x;
    Value velocity_y;
    Value velocity_angular;

    SceneLabelMagnetostaticMarker(const QString &name, Value current_density, Value permeability, Value remanence, Value remanence_angle,
                                  Value conductivity, Value velocity_x, Value velocity_y, Value velocity_angular);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class DSceneEdgeMagnetostaticMarker : public DSceneEdgeMarker
{
    Q_OBJECT
public:
    DSceneEdgeMagnetostaticMarker(SceneEdgeMagnetostaticMarker *edgeMagnetostaticMarker, QWidget *parent);
    ~DSceneEdgeMagnetostaticMarker();

protected:
    QLayout *createContent();

    void load();
    bool save();

private:
    QComboBox *cmbType;
    SLineEditValue *txtValue;
};

class DSceneLabelMagnetostaticMarker : public DSceneLabelMarker
{
    Q_OBJECT

public:
    DSceneLabelMagnetostaticMarker(QWidget *parent, SceneLabelMagnetostaticMarker *labelMagnetostaticMarker);
    ~DSceneLabelMagnetostaticMarker();

protected:
    QLayout *createContent();

    void load();
    bool save();

private:
    SLineEditValue *txtPermeability;
    SLineEditValue *txtCurrentDensity;
    SLineEditValue *txtRemanence;
    SLineEditValue *txtRemanenceAngle;
    SLineEditValue *txtConductivity;
    SLineEditValue *txtVelocityX;
    SLineEditValue *txtVelocityY;
    SLineEditValue *txtVelocityAngular;

};

#endif // MAGNETOSTATIC_H
