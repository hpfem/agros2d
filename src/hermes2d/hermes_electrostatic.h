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
    SceneLabelMarker *newLabelMarker();

    SolutionArray *solve(SolverThread *solverThread);

    inline PhysicFieldVariable contourPhysicFieldVariable() { return PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL; }
    inline PhysicFieldVariable scalarPhysicFieldVariable() { return PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL; }
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
public:
    double surfaceCharge;

    SurfaceIntegralValueElectrostatic();
    QStringList variables();
};

class VolumeIntegralValueElectrostatic : public VolumeIntegralValue
{
public:
    double averageElectricFieldX;
    double averageElectricFieldY;
    double averageElectricField;
    double averageDisplacementX;
    double averageDisplacementY;
    double averageDisplacement;
    double energy;

    VolumeIntegralValueElectrostatic();
    QStringList variables();
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
    QLayout *createContent();

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
    QLayout *createContent();

    void load();
    bool save();

private:
    SLineEditValue *txtPermittivity;
    SLineEditValue *txtChargeDensity;
};
#endif // ELECTROSTATIC_H
