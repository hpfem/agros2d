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

    SceneEdgeMarker *newEdgeMarker();
    // SceneEdgeMarker *newEdgeMarker(const QString &name, PhysicFieldBC physicFieldBC[], Value *value[]);
    SceneLabelMarker *newLabelMarker();

    SolutionArray *solve(SolverThread *solverThread);

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
    }

    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD), PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD);
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
    double remanence;
    double remanence_angle;
    double potential;
    Point H;
    Point B;
    double wm;

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

    SceneLabelMagnetostaticMarker(const QString &name, Value current_density, Value permeability, Value remanence, Value remanence_angle);

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
};

#endif // MAGNETOSTATIC_H
