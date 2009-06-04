#ifndef SURFACEINTEGRALVIEW_H
#define SURFACEINTEGRALVIEW_H

#include <QtGui/QDockWidget>
#include <QtGui/QTreeWidget>

#include "util.h"
#include "scene.h"

class SurfaceIntegralValue;
class SurfaceIntegralValueElectrostatic;
class SurfaceIntegralValueMagnetostatic;
class SurfaceIntegralValueHeat;

SurfaceIntegralValue *surfaceIntegralValueFactory(Scene *scene);

class SurfaceIntegralValueView : public QDockWidget
{
    Q_OBJECT

public slots:
    void doShowSurfaceIntegral(SurfaceIntegralValue *surfaceIntegralValue);

public:
    SurfaceIntegralValueView(QWidget *parent = 0);

private:
    QTreeWidget *trvWidget;

    void addValue(QTreeWidgetItem *parent, QString name, QString text, QString unit);

    void showElectrostatic(SurfaceIntegralValueElectrostatic *surfaceIntegralValueElectrostatic);
    void showMagnetostatic(SurfaceIntegralValueMagnetostatic *surfaceIntegralValueMagnetostatic);
    void showHeat(SurfaceIntegralValueHeat *surfaceIntegralValueHeat);
};

class SurfaceIntegralValue
{
public:
    double length;
    double surface;

    SurfaceIntegralValue(Scene *scene);

    inline Scene *scene() { return m_scene; }

    virtual QString toString();

protected:
    Scene *m_scene;
};

class SurfaceIntegralValueElectrostatic : public SurfaceIntegralValue
{
public:
    double surfaceCharge;

    SurfaceIntegralValueElectrostatic(Scene *scene);
    QString toString();
};

class SurfaceIntegralValueMagnetostatic : public SurfaceIntegralValue
{
public:
    SurfaceIntegralValueMagnetostatic(Scene *scene);
    QString toString();
};

class SurfaceIntegralValueHeat : public SurfaceIntegralValue
{
public:
    SurfaceIntegralValueHeat(Scene *scene);
    QString toString();
};

#endif // SURFACEINTEGRALVIEW_H
