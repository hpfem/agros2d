#ifndef SURFACEINTEGRALVIEW_H
#define SURFACEINTEGRALVIEW_H

#include <QtGui/QDockWidget>
#include <QtGui/QTreeWidget>

#include "util.h"
#include "scene.h"

class SurfaceIntegralValue;
class SurfaceIntegralValueElectrostatic2D;
class SurfaceIntegralValueMagnetostatic2D;
class SurfaceIntegralValueHeat2D;

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

    void showElectrostatic2D(SurfaceIntegralValueElectrostatic2D *surfaceIntegralValueElectrostatic2D);
    void showMagnetostatic2D(SurfaceIntegralValueMagnetostatic2D *surfaceIntegralValueMagnetostatic2D);
    void showHeat2D(SurfaceIntegralValueHeat2D *surfaceIntegralValueHeat2D);
};

class SurfaceIntegralValue
{
public:
    double length;

    SurfaceIntegralValue(Scene *scene);

    inline Scene *scene() { return m_scene; }

    virtual QString toString();

protected:
    Scene *m_scene;
};

class SurfaceIntegralValueElectrostatic2D : public SurfaceIntegralValue
{
public:
    SurfaceIntegralValueElectrostatic2D(Scene *scene);
    QString toString();
};

class SurfaceIntegralValueMagnetostatic2D : public SurfaceIntegralValue
{
public:
    SurfaceIntegralValueMagnetostatic2D(Scene *scene);
    QString toString();
};

class SurfaceIntegralValueHeat2D : public SurfaceIntegralValue
{
public:
    SurfaceIntegralValueHeat2D(Scene *scene);
    QString toString();
};

#endif // SURFACEINTEGRALVIEW_H
