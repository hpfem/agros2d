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
class SurfaceIntegralValueCurrent;

SurfaceIntegralValue *surfaceIntegralValueFactory();

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
    void showCurrent(SurfaceIntegralValueCurrent *surfaceIntegralValueCurrent);
};

// ******************************************************************************************************************

class SurfaceIntegralValue
{
public:
    double length;
    double surface;

    SurfaceIntegralValue();

    virtual QString toString();

protected:
};

// ******************************************************************************************************************

class SurfaceIntegralValueElectrostatic : public SurfaceIntegralValue
{
public:
    double surfaceCharge;

    SurfaceIntegralValueElectrostatic();
    QString toString();
};

// ******************************************************************************************************************

class SurfaceIntegralValueMagnetostatic : public SurfaceIntegralValue
{
public:
    SurfaceIntegralValueMagnetostatic();
    QString toString();
};

// ******************************************************************************************************************

class SurfaceIntegralValueHeat : public SurfaceIntegralValue
{
public:
    double averageTemperature;
    double temperatureDifference;
    double heatFlux;

    SurfaceIntegralValueHeat();
    QString toString();
};

// ******************************************************************************************************************

class SurfaceIntegralValueCurrent : public SurfaceIntegralValue
{
public:
    double currentDensity;

    SurfaceIntegralValueCurrent();
    QString toString();
};
// ******************************************************************************************************************

#endif // SURFACEINTEGRALVIEW_H
