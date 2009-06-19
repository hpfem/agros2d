#ifndef SCENEMARKER_H
#define SCENEMARKER_H

#include <QtGui/QDoubleValidator>
#include <QtGui/QHBoxLayout>
#include <QtGui/QFormLayout>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QLayout>
#include <QtGui/QGroupBox>
#include <QMetaType>
#include <QString>
#include <QVariant>

#include "util.h"
#include "gui.h"
#include "scene.h"

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

    virtual int showDialog(Scene *scene, QWidget *parent) = 0;

    virtual QString script() = 0;
    QVariant variant();
};

class SceneEdgeMarkerNone : public SceneEdgeMarker {
public:
    SceneEdgeMarkerNone();

    QString script() { return ""; }
    virtual int showDialog(Scene *scene, QWidget *parent) {}
};

// *************************************************************************************************************************************

class SceneEdgeElectrostaticMarker : public SceneEdgeMarker {

public:
    double value;

    SceneEdgeElectrostaticMarker(const QString &name, PhysicFieldBC type, double value);

    QString script();
    int showDialog(Scene *scene, QWidget *parent);
};

// *************************************************************************************************************************************

class SceneEdgeMagnetostaticMarker : public SceneEdgeMarker {

public:
    double value;

    SceneEdgeMagnetostaticMarker(const QString &name, PhysicFieldBC type, double value);

    QString script();
    int showDialog(Scene *scene, QWidget *parent);
};

// *************************************************************************************************************************************

class SceneEdgeHeatMarker : public SceneEdgeMarker {

public:
    double temperature;
    double heatFlux;
    double h;
    double externalTemperature;

    SceneEdgeHeatMarker(const QString &name, PhysicFieldBC type, double temperature);
    SceneEdgeHeatMarker(const QString &name, PhysicFieldBC type, double heatFlux, double h, double externalTemperature);

    QString script();
    int showDialog(Scene *scene, QWidget *parent);
};

// *************************************************************************************************************************************

class SceneEdgeCurrentMarker : public SceneEdgeMarker {

public:
    double value;

    SceneEdgeCurrentMarker(const QString &name, PhysicFieldBC type, double value);

    QString script();
    int showDialog(Scene *scene, QWidget *parent);
};

// *************************************************************************************************************************************

class SceneEdgeElasticityMarker : public SceneEdgeMarker {

public:
    PhysicFieldBC typeX;    
    PhysicFieldBC typeY;
    double forceX;
    double forceY;

    QString script();
    SceneEdgeElasticityMarker(const QString &name, PhysicFieldBC typeX, PhysicFieldBC typeY, double forceX, double forceY);

    int showDialog(Scene *scene, QWidget *parent);
};

// *************************************************************************************************************************************
// *************************************************************************************************************************************
// *************************************************************************************************************************************

class SceneLabelMarker {
public:
    QString name;

    SceneLabelMarker(const QString &name);

    virtual int showDialog(Scene *scene, QWidget *parent) = 0;

    virtual QString script() = 0;
    QVariant variant();
};

class SceneLabelMarkerNone : public SceneLabelMarker {
public:
    SceneLabelMarkerNone();

    QString script() { return ""; }
    virtual int showDialog(Scene *scene, QWidget *parent) {}
};

// *************************************************************************************************************************************

class SceneLabelElectrostaticMarker : public SceneLabelMarker {

public:
    double charge_density;
    double permittivity;

    SceneLabelElectrostaticMarker(const QString &name, double charge_density, double permittivity);

    QString script();
    int showDialog(Scene *scene, QWidget *parent);
};

// *************************************************************************************************************************************

class SceneLabelHeatMarker : public SceneLabelMarker {

public:
    double thermal_conductivity;
    double volume_heat;

    SceneLabelHeatMarker(const QString &name, double volume_heat, double thermal_conductivity);

    QString script();
    int showDialog(Scene *scene, QWidget *parent);
};

// *************************************************************************************************************************************

class SceneLabelCurrentMarker : public SceneLabelMarker {

public:
    double conductivity;

    SceneLabelCurrentMarker(const QString &name, double conductivity);

    QString script();
    int showDialog(Scene *scene, QWidget *parent);
};

// *************************************************************************************************************************************

class SceneLabelMagnetostaticMarker : public SceneLabelMarker {

public:
    double permeability;
    double current_density;

    SceneLabelMagnetostaticMarker(const QString &name, double current_density, double permeability);

    QString script();
    int showDialog(Scene *scene, QWidget *parent);
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
    int showDialog(Scene *scene, QWidget *parent);
};

// *************************************************************************************************************************************

class DSceneEdgeMarker: public QDialog
{
    Q_OBJECT

public:
    DSceneEdgeMarker(QWidget *parent);
    ~DSceneEdgeMarker();

protected:
    SceneEdgeMarker *m_edgeMarker;

    virtual QLayout *createContent() = 0;
    void createDialog();

    virtual void load();
    virtual void save();
    void setSize();

private slots:
    void doAccept();
    void doReject();

private:
    QVBoxLayout *layout;
    QLineEdit *txtName;
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
    void save();

private:
    QComboBox *cmbType;
    SLineEdit *txtValue;
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
    void save();

private:
    QComboBox *cmbType;
    QLineEdit *txtValue;
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
    void save();

private:
    QComboBox *cmbType;
    SLineEdit *txtTemperature;
    SLineEdit *txtHeatFlux;
    SLineEdit *txtHeatTransferCoefficient;
    SLineEdit *txtExternalTemperature;

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
    void save();

private:
    QComboBox *cmbType;
    QLineEdit *txtValue;
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
    void save();

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
    SceneLabelMarker *m_labelMarker;

    virtual QLayout *createContent() = 0;
    void createDialog();
    void setSize();

    virtual void load();
    virtual void save();

private slots:
    void doAccept();
    void doReject();

private:
    QVBoxLayout *layout;
    QLineEdit *txtName;
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
    void save();

private:
    SLineEdit *txtPermittivity;
    SLineEdit *txtChargeDensity;
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
    void save();

private:
    SLineEdit *txtPermeability;
    SLineEdit *txtCurrentDensity;
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
    void save();

private:
    SLineEdit *txtThermalConductivity;
    SLineEdit *txtVolumeHeat;
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
    void save();

private:
    SLineEdit *txtConductivity;
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
    void save();

private:
    SLineEdit *txtYoungModulus;
    SLineEdit *txtPoissonNumber;
};

#endif // SCENEMARKER_H
