#ifndef GENERAL_H
#define GENERAL_H

#include "util.h"
#include "hermes_field.h"
#include "hermes2d.h"
#include "solverdialog.h"
#include "solver_umfpack.h"

struct HermesGeneral : public HermesField
{
    Q_OBJECT
public:
    HermesGeneral() { physicField = PHYSICFIELD_GENERAL; }

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

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PHYSICFIELDBC_GENERAL_VALUE ||
                                                                          physicFieldBC == PHYSICFIELDBC_GENERAL_DERIVATIVE); }
    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PHYSICFIELDVARIABLE_GENERAL_VARIABLE ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_GENERAL_GRADIENT ||
                                                                                            physicFieldVariable == PHYSICFIELDVARIABLE_GENERAL_CONSTANT); }

    SceneEdgeMarker *newEdgeMarker();
    SceneEdgeMarker *newEdgeMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *newLabelMarker();
    SceneLabelMarker *newLabelMarker(PyObject *self, PyObject *args);

    QList<SolutionArray *> *solve(SolverThread *solverThread);

    inline PhysicFieldVariable contourPhysicFieldVariable() { return PHYSICFIELDVARIABLE_GENERAL_VARIABLE; }
    inline PhysicFieldVariable scalarPhysicFieldVariable() { return PHYSICFIELDVARIABLE_GENERAL_VARIABLE; }
    inline PhysicFieldVariableComp scalarPhysicFieldVariableComp() { return PHYSICFIELDVARIABLECOMP_SCALAR; }
    inline PhysicFieldVariable vectorPhysicFieldVariable() { return PHYSICFIELDVARIABLE_GENERAL_GRADIENT; }

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_GENERAL_VARIABLE), PHYSICFIELDVARIABLE_GENERAL_VARIABLE);
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_GENERAL_GRADIENT), PHYSICFIELDVARIABLE_GENERAL_GRADIENT);
    }

    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_GENERAL_GRADIENT), PHYSICFIELDVARIABLE_GENERAL_GRADIENT);
    }

    void showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue);
    void showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue);
    void showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue);
};

class LocalPointValueGeneral : public LocalPointValue
{
public:
    double variable;
    double rightside;
    Point gradient;
    double constant;

    LocalPointValueGeneral(Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
};

class SurfaceIntegralValueGeneral : public SurfaceIntegralValue
{
public:
    SurfaceIntegralValueGeneral();
    QStringList variables();
};

class VolumeIntegralValueGeneral : public VolumeIntegralValue
{
public:
    VolumeIntegralValueGeneral();
    QStringList variables();
};

class SceneEdgeGeneralMarker : public SceneEdgeMarker
{
public:
    Value value;

    SceneEdgeGeneralMarker(const QString &name, PhysicFieldBC type, Value value);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneLabelGeneralMarker : public SceneLabelMarker
{
public:
    Value rightside;
    Value constant;

    SceneLabelGeneralMarker(const QString &name, Value rightside, Value constant);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class DSceneEdgeGeneralMarker : public DSceneEdgeMarker
{
    Q_OBJECT
public:
    DSceneEdgeGeneralMarker(SceneEdgeGeneralMarker *edgeGeneralMarker, QWidget *parent);
    ~DSceneEdgeGeneralMarker();

protected:
    QLayout *createContent();

    void load();
    bool save();

private:
    QComboBox *cmbType;
    SLineEditValue *txtValue;
};

class DSceneLabelGeneralMarker : public DSceneLabelMarker
{
    Q_OBJECT
public:
    DSceneLabelGeneralMarker(QWidget *parent, SceneLabelGeneralMarker *labelGeneralMarker);
    ~DSceneLabelGeneralMarker();

protected:
    QLayout *createContent();

    void load();
    bool save();

private:
    SLineEditValue *txtConstant;
    SLineEditValue *txtRightSide;
};

#endif // GENERAL_H
