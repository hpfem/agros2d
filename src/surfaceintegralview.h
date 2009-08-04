#ifndef SURFACEINTEGRALVIEW_H
#define SURFACEINTEGRALVIEW_H

#include <QtGui/QDockWidget>
#include <QtGui/QTreeWidget>

#include "util.h"
#include "scene.h"

class SurfaceIntegralValue;
class SurfaceIntegralValueElectrostatic;
class SurfaceIntegralValueMagnetostatic;
class SurfaceIntegralValueHarmonicMagnetic;
class SurfaceIntegralValueHeat;
class SurfaceIntegralValueCurrent;

SurfaceIntegralValue *surfaceIntegralValueFactory();
QStringList surfaceIntegralValueHeaderFactory(PhysicField physicField);

class SurfaceIntegralValueView : public QDockWidget
{
    Q_OBJECT

public slots:
    void doShowSurfaceIntegral(SurfaceIntegralValue *surfaceIntegralValue);

public:
    SurfaceIntegralValueView(QWidget *parent = 0);
    ~SurfaceIntegralValueView();

private:
    QTreeWidget *trvWidget;

    void addValue(QTreeWidgetItem *parent, QString name, QString text, QString unit);

    void showElectrostatic(SurfaceIntegralValueElectrostatic *surfaceIntegralValueElectrostatic);
    void showMagnetostatic(SurfaceIntegralValueMagnetostatic *surfaceIntegralValueMagnetostatic);
    void showHarmonicMagnetic(SurfaceIntegralValueHarmonicMagnetic *surfaceIntegralValueHarmonicMagnetic);
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

    virtual QStringList variables() = 0;

protected:
};

// ******************************************************************************************************************

class SurfaceIntegralValueElectrostatic : public SurfaceIntegralValue
{
public:
    double surfaceCharge;

    SurfaceIntegralValueElectrostatic();
    QStringList variables();
};

// ******************************************************************************************************************

class SurfaceIntegralValueMagnetostatic : public SurfaceIntegralValue
{
public:
    SurfaceIntegralValueMagnetostatic();
    QStringList variables();
};

// ******************************************************************************************************************

class SurfaceIntegralValueHarmonicMagnetic : public SurfaceIntegralValue
{
public:
    SurfaceIntegralValueHarmonicMagnetic();
    QStringList variables();
};

// ******************************************************************************************************************

class SurfaceIntegralValueHeat : public SurfaceIntegralValue
{
public:
    double averageTemperature;
    double temperatureDifference;
    double heatFlux;

    SurfaceIntegralValueHeat();
    QStringList variables();
};

// ******************************************************************************************************************

class SurfaceIntegralValueCurrent : public SurfaceIntegralValue
{
public:
    double currentDensity;

    SurfaceIntegralValueCurrent();
    QStringList variables();
};
// ******************************************************************************************************************

#endif // SURFACEINTEGRALVIEW_H
