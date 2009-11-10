#ifndef HERMES_CURRENT_H
#define HERMES_CURRENT_H

#include "util.h"
#include "hermes_field.h"

struct HermesCurrent : public HermesField
{
    Q_OBJECT
public:
    HermesCurrent() { physicField = PHYSICFIELD_CURRENT; }

    inline int numberOfSolution() { return 1; }
    bool hasFrequency() { return false; }
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

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PHYSICFIELDBC_HEAT_TEMPERATURE ||
                                                                          physicFieldBC == PHYSICFIELDBC_HEAT_HEAT_FLUX); }
    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PHYSICFIELDVARIABLE_CURRENT_POTENTIAL ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_CURRENT_LOSSES ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY); }

    SceneEdgeMarker *newEdgeMarker();
    SceneEdgeMarker *newEdgeMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *newLabelMarker();
    SceneLabelMarker *newLabelMarker(PyObject *self, PyObject *args);

    QList<SolutionArray *> *solve(SolverThread *solverThread);

    inline PhysicFieldVariable contourPhysicFieldVariable() { return PHYSICFIELDVARIABLE_CURRENT_POTENTIAL; }
    inline PhysicFieldVariable scalarPhysicFieldVariable() { return PHYSICFIELDVARIABLE_CURRENT_POTENTIAL; }
    inline PhysicFieldVariableComp scalarPhysicFieldVariableComp() { return PHYSICFIELDVARIABLECOMP_SCALAR; }
    inline PhysicFieldVariable vectorPhysicFieldVariable() { return PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY; }

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_POTENTIAL), PHYSICFIELDVARIABLE_CURRENT_POTENTIAL);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD), PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY), PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_LOSSES), PHYSICFIELDVARIABLE_CURRENT_LOSSES);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY), PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY);
    }

    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD), PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY), PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY);
    }

    void showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue);
    void showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue);
    void showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue);
};

class LocalPointValueCurrent : public LocalPointValue
{
public:
    double conductivity;
    double losses;
    double potential;
    Point J;
    Point E;

    LocalPointValueCurrent(Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
};

class SurfaceIntegralValueCurrent : public SurfaceIntegralValue
{
public:
    double currentDensity;

    SurfaceIntegralValueCurrent();
    QStringList variables();
};

class VolumeIntegralValueCurrent : public VolumeIntegralValue
{
public:
    double averageElectricFieldX;
    double averageElectricFieldY;
    double averageElectricField;
    double averageCurrentDensityX;
    double averageCurrentDensityY;
    double averageCurrentDensity;
    double losses;

    VolumeIntegralValueCurrent();
    QStringList variables();
};

class SceneEdgeCurrentMarker : public SceneEdgeMarker {

public:
    Value value;

    SceneEdgeCurrentMarker(const QString &name, PhysicFieldBC type, Value value);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneLabelCurrentMarker : public SceneLabelMarker
{
public:
    Value conductivity;

    SceneLabelCurrentMarker(const QString &name, Value conductivity);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class DSceneEdgeCurrentMarker : public DSceneEdgeMarker
{
    Q_OBJECT

public:
    DSceneEdgeCurrentMarker(SceneEdgeCurrentMarker *edgeCurrentMarker, QWidget *parent);
    ~DSceneEdgeCurrentMarker();

protected:
    QLayout *createContent();

    void load();
    bool save();

private:
    QComboBox *cmbType;
    SLineEditValue *txtValue;
};

class DSceneLabelCurrentMarker : public DSceneLabelMarker
{
    Q_OBJECT

public:
    DSceneLabelCurrentMarker(QWidget *parent, SceneLabelCurrentMarker *labelCurrentMarker);
    ~DSceneLabelCurrentMarker();

protected:
    QLayout *createContent();

    void load();
    bool save();

private:
    SLineEditValue *txtConductivity;
};

#endif // HERMES_CURRENT_H
