#ifndef LOCALVALUE_H
#define LOCALVALUE_H

#include <QtGui/QDockWidget>
#include <QtGui/QTreeWidget>

#include "util.h"
#include "scene.h"

class Scene;

class LocalPointValue;
class LocalPointValueElectrostatic;
class LocalPointValueMagnetostatic;
class LocalPointValueHeat;
class LocalPointValueElasticity;

LocalPointValue *localPointValueFactory(Point &point, Scene *scene);

class LocalPointValueView : public QDockWidget
{
    Q_OBJECT

public slots:
    void doShowPoint(LocalPointValue *localPointValue);

public:
    LocalPointValueView(Scene *scene, QWidget *parent = 0);

private:
    Scene *m_scene;

    QTreeWidget *trvWidget;

    QAction *actPoint;

    QMenu *mnuInfo;

    void createActions();
    void createMenu();

    void addValue(QTreeWidgetItem *parent, QString name, QString text, QString unit);

    void showElectrostatic(LocalPointValueElectrostatic *localPointValueElectrostatic);
    void showMagnetostatic(LocalPointValueMagnetostatic *localPointValueMagnetostatic);
    void showHeat(LocalPointValueHeat *localPointValueHeat);
    void showElasticity(LocalPointValueElasticity *localPointValueElasticity);

private slots:
    void doPoint();
    void doContextMenu(const QPoint &pos);
};


class LocalPointValue
{
public:
    Point point;

    LocalPointValue(Point &point, Scene *scene);

    inline Scene *scene() { return m_scene; }

    virtual QString toString();

protected:
    Scene *m_scene;
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

    LocalPointValueElectrostatic(Point &point, Scene *scene);
    QString toString();
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

    LocalPointValueMagnetostatic(Point &point, Scene *scene);
    QString toString();
};

class LocalPointValueHeat : public LocalPointValue
{
public:
    double volume_heat;
    double thermal_conductivity;
    double temperature;
    Point F;
    Point G;

    LocalPointValueHeat(Point &point, Scene *scene);
    QString toString();
};

class LocalPointValueElasticity : public LocalPointValue
{
public:
    double young_modulus;
    double poisson_ratio;
    double von_mises_stress;

    LocalPointValueElasticity(Point &point, Scene *scene);
    QString toString();
};

#endif // LOCALVALUE_H
