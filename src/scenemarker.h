#ifndef SCENEMARKER_H
#define SCENEMARKER_H

#include "util.h"
#include "gui.h"
#include "scene.h"

class SLineEdit;
class SLineEditValue;

class Scene;
class SceneEdgeMarker;
class SceneLabelMarker;

Q_DECLARE_METATYPE(SceneEdgeMarker *);
Q_DECLARE_METATYPE(SceneLabelMarker *);

class SceneEdgeMarker {
public:
    QString name;
    PhysicFieldBC type;

    SceneEdgeMarker(const QString &name, PhysicFieldBC type);

    virtual int showDialog(QWidget *parent) = 0;

    virtual QString script() = 0;
    virtual QMap<QString, QString> data() = 0;
    QVariant variant();
};

class SceneEdgeMarkerNone : public SceneEdgeMarker {
public:
    SceneEdgeMarkerNone();

    QString script() { return ""; }
    QMap<QString, QString> data() { return QMap<QString, QString>(); }
    virtual int showDialog(QWidget *parent) {}
};

// *************************************************************************************************************************************

class SceneEdgeElectrostaticMarker : public SceneEdgeMarker {

public:
    Value value;

    SceneEdgeElectrostaticMarker(const QString &name, PhysicFieldBC type, Value value);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

// *************************************************************************************************************************************

class SceneEdgeMagnetostaticMarker : public SceneEdgeMarker {

public:
    Value value;

    SceneEdgeMagnetostaticMarker(const QString &name, PhysicFieldBC type, Value value);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

// *************************************************************************************************************************************

class SceneEdgeHarmonicMagneticMarker : public SceneEdgeMarker {

public:
    Value value;

    SceneEdgeHarmonicMagneticMarker(const QString &name, PhysicFieldBC type, Value value);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

// *************************************************************************************************************************************

class SceneEdgeHeatMarker : public SceneEdgeMarker {

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

// *************************************************************************************************************************************

class SceneEdgeCurrentMarker : public SceneEdgeMarker {

public:
    Value value;

    SceneEdgeCurrentMarker(const QString &name, PhysicFieldBC type, Value value);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

// *************************************************************************************************************************************

class SceneEdgeElasticityMarker : public SceneEdgeMarker {

public:
    PhysicFieldBC typeX;
    PhysicFieldBC typeY;
    double forceX;
    double forceY;

    QString script();
    QMap<QString, QString> data();
    SceneEdgeElasticityMarker(const QString &name, PhysicFieldBC typeX, PhysicFieldBC typeY, double forceX, double forceY);

    int showDialog(QWidget *parent);
};

// *************************************************************************************************************************************
// *************************************************************************************************************************************
// *************************************************************************************************************************************

class SceneLabelMarker {
public:
    QString name;

    SceneLabelMarker(const QString &name);

    virtual int showDialog(QWidget *parent) = 0;

    virtual QString script() = 0;
    virtual QMap<QString, QString> data() = 0;
    QVariant variant();
};

class SceneLabelMarkerNone : public SceneLabelMarker {
public:
    SceneLabelMarkerNone();

    QString script() { return ""; }
    QMap<QString, QString> data() { return QMap<QString, QString>(); }
    virtual int showDialog(QWidget *parent) {}
};

// *************************************************************************************************************************************

class SceneLabelElectrostaticMarker : public SceneLabelMarker {

public:
    Value charge_density;
    Value permittivity;

    SceneLabelElectrostaticMarker(const QString &name, Value charge_density, Value permittivity);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

// *************************************************************************************************************************************

class SceneLabelMagnetostaticMarker : public SceneLabelMarker {

public:
    Value permeability;
    Value current_density;

    SceneLabelMagnetostaticMarker(const QString &name, Value current_density, Value permeability);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

// *************************************************************************************************************************************

class SceneLabelHarmonicMagneticMarker : public SceneLabelMarker {

public:
    Value permeability;
    Value conductivity;
    Value current_density_real;
    Value current_density_imag;

    SceneLabelHarmonicMagneticMarker(const QString &name, Value current_density_real, Value current_density_imag, Value permeability, Value conductivity);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

// *************************************************************************************************************************************

class SceneLabelHeatMarker : public SceneLabelMarker {

public:
    Value thermal_conductivity;
    Value volume_heat;

    SceneLabelHeatMarker(const QString &name, Value volume_heat, Value thermal_conductivity);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

// *************************************************************************************************************************************

class SceneLabelCurrentMarker : public SceneLabelMarker {

public:
    Value conductivity;

    SceneLabelCurrentMarker(const QString &name, Value conductivity);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

// *************************************************************************************************************************************

class SceneLabelElasticityMarker : public SceneLabelMarker {

public:
    double young_modulus;
    double poisson_ratio;

    SceneLabelElasticityMarker(const QString &name, double young_modulus, double poisson_ratio);

    // Lame constant
    inline double lambda() { return (young_modulus * poisson_ratio) / ((1 + poisson_ratio) * (1 - 2*poisson_ratio)); }
    inline double mu() { return young_modulus / (2*(1 + poisson_ratio)); }

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

// *************************************************************************************************************************************

class DSceneEdgeMarker: public QDialog
{
    Q_OBJECT

public:
    DSceneEdgeMarker(QWidget *parent);
    ~DSceneEdgeMarker();

protected:
    QLineEdit *txtName;
    SceneEdgeMarker *m_edgeMarker;

    virtual QLayout *createContent() = 0;
    void createDialog();

    virtual void load();
    virtual bool save();
    void setSize();

private slots:
    void doAccept();
    void doReject();

private:
    QVBoxLayout *layout;
};

// *************************************************************************************************************************************

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

// *************************************************************************************************************************************

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

// *************************************************************************************************************************************

class DSceneEdgeHarmonicMagneticMarker : public DSceneEdgeMarker
{
    Q_OBJECT

public:
    DSceneEdgeHarmonicMagneticMarker(SceneEdgeHarmonicMagneticMarker *edgeHarmonicMagneticMarker, QWidget *parent);
    ~DSceneEdgeHarmonicMagneticMarker();

protected:
    QLayout *createContent();

    void load();
    bool save();

private:
    QComboBox *cmbType;
    SLineEditValue *txtValue;
};

// *************************************************************************************************************************************

class DSceneEdgeHeatMarker : public DSceneEdgeMarker
{
    Q_OBJECT

public:
    DSceneEdgeHeatMarker(SceneEdgeHeatMarker *edgeEdgeHeatMarker, QWidget *parent);
    ~DSceneEdgeHeatMarker();

protected:
    QLayout *createContent();

    void load();
    bool save();

private:
    QComboBox *cmbType;
    SLineEditValue *txtTemperature;
    SLineEditValue *txtHeatFlux;
    SLineEditValue *txtHeatTransferCoefficient;
    SLineEditValue *txtExternalTemperature;

private slots:
    void doTypeChanged(int index);
};

// *************************************************************************************************************************************

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

// *************************************************************************************************************************************

class DSceneEdgeElasticityMarker : public DSceneEdgeMarker
{
    Q_OBJECT

public:
    DSceneEdgeElasticityMarker(SceneEdgeElasticityMarker *edgeEdgeElasticityMarker, QWidget *parent);
    ~DSceneEdgeElasticityMarker();

protected:
    QLayout *createContent();

    void load();
    bool save();

private:
    QComboBox *cmbTypeX;
    QComboBox *cmbTypeY;
    SLineEdit *txtForceX;
    SLineEdit *txtForceY;
};

// *************************************************************************************************************************************

class DSceneLabelMarker: public QDialog
{
    Q_OBJECT

public:
    DSceneLabelMarker(QWidget *parent);
    ~DSceneLabelMarker();

protected:
    QLineEdit *txtName;
    SceneLabelMarker *m_labelMarker;

    virtual QLayout *createContent() = 0;
    void createDialog();
    void setSize();

    virtual void load();
    virtual bool save();

private slots:
    void doAccept();
    void doReject();

private:
    QVBoxLayout *layout;
};

// *************************************************************************************************************************************

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

// *************************************************************************************************************************************

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
};

// *************************************************************************************************************************************

class DSceneLabelHarmonicMagneticMarker : public DSceneLabelMarker
{
    Q_OBJECT

public:
    DSceneLabelHarmonicMagneticMarker(QWidget *parent, SceneLabelHarmonicMagneticMarker *labelHarmonicMagneticMarker);
    ~DSceneLabelHarmonicMagneticMarker();

protected:
    QLayout *createContent();

    void load();
    bool save();

private:
    SLineEditValue *txtPermeability;
    SLineEditValue *txtConductivity;
    SLineEditValue *txtCurrentDensityReal;
    SLineEditValue *txtCurrentDensityImag;
};

// *************************************************************************************************************************************

class DSceneLabelHeatMarker : public DSceneLabelMarker
{
    Q_OBJECT

public:
    DSceneLabelHeatMarker(QWidget *parent, SceneLabelHeatMarker *labelHeatMarker);
    ~DSceneLabelHeatMarker();

protected:
    QLayout *createContent();

    void load();
    bool save();

private:
    SLineEditValue *txtThermalConductivity;
    SLineEditValue *txtVolumeHeat;
};

// *************************************************************************************************************************************

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

// *************************************************************************************************************************************

class DSceneLabelElasticityMarker : public DSceneLabelMarker
{
    Q_OBJECT

public:
    DSceneLabelElasticityMarker(QWidget *parent, SceneLabelElasticityMarker *labelElasticityMarker);
    ~DSceneLabelElasticityMarker();

protected:
    QLayout *createContent();

    void load();
    bool save();

private:
    SLineEdit *txtYoungModulus;
    SLineEdit *txtPoissonNumber;
};

#endif // SCENEMARKER_H
