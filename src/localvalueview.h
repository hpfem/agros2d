#ifndef LOCALVALUE_H
#define LOCALVALUE_H

#include <QtGui/QDockWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QMenu>

#include "util.h"
#include "scene.h"

class Scene;

class LocalPointValue;
class LocalPointValueElectrostatic;
class LocalPointValueMagnetostatic;
class LocalPointValueHarmonicMagnetic;
class LocalPointValueHeat;
class LocalPointValueCurrent;
class LocalPointValueElasticity;

LocalPointValue *localPointValueFactory(Point &point);
QStringList localPointValueHeaderFactory(PhysicField physicField);

class LocalPointValueView : public QDockWidget
{
    Q_OBJECT

public slots:
    void doShowPoint(LocalPointValue *localPointValue);

public:
    LocalPointValueView(QWidget *parent = 0);

private:
    QTreeWidget *trvWidget;
    QAction *actPoint;
    QMenu *mnuInfo;

    void createActions();
    void createMenu();

    void addValue(QTreeWidgetItem *parent, QString name, QString text, QString unit);

    void showElectrostatic(LocalPointValueElectrostatic *localPointValueElectrostatic);
    void showMagnetostatic(LocalPointValueMagnetostatic *localPointValueMagnetostatic);
    void showHarmonicMagnetic(LocalPointValueHarmonicMagnetic *localPointValueHarmonicMagnetic);
    void showHeat(LocalPointValueHeat *localPointValueHeat);
    void showElasticity(LocalPointValueElasticity *localPointValueElasticity);
    void showCurrent(LocalPointValueCurrent *localPointValueCurrent);

private slots:
    void doPoint();
    void doContextMenu(const QPoint &pos);
};


class LocalPointValue
{
public:
    Point point;

    LocalPointValue(Point &point);

    virtual double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) = 0;
    virtual QStringList variables() = 0;

protected:
    Vectorizer m_vec;
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

class LocalPointValueMagnetostatic : public LocalPointValue
{
public:
    double current_density;
    double permeability;
    double potential;
    Point H;
    Point B;
    double wm;

    LocalPointValueMagnetostatic(Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
};

class LocalPointValueHarmonicMagnetic : public LocalPointValue
{
public:
    double current_density_real;
    double current_density_imag;
    double current_density_total_real;
    double current_density_total_imag;
    double current_density_induced_real;
    double current_density_induced_imag;
    double permeability;
    double potential_real;
    double potential_imag;
    Point H_real;
    Point H_imag;
    Point B_real;
    Point B_imag;
    Point FL_real;
    Point FL_imag;
    double pj;
    double wm;

    LocalPointValueHarmonicMagnetic(Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
};

class LocalPointValueHeat : public LocalPointValue
{
public:
    double volume_heat;
    double thermal_conductivity;
    double temperature;
    Point F;
    Point G;

    LocalPointValueHeat(Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
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

class LocalPointValueElasticity : public LocalPointValue
{
public:
    double young_modulus;
    double poisson_ratio;
    double von_mises_stress;

    LocalPointValueElasticity(Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
};

#endif // LOCALVALUE_H
