#ifndef VOLUMEINTEGRALVIEW_H
#define VOLUMEINTEGRALVIEW_H

#include <QtGui/QDockWidget>
#include <QtGui/QTreeWidget>

#include "util.h"
#include "scene.h"

class VolumeIntegralValue;
class VolumeIntegralValueElectrostatic;
class VolumeIntegralValueMagnetostatic;
class VolumeIntegralValueHeat;
class VolumeIntegralValueCurrent;

VolumeIntegralValue *volumeIntegralValueFactory();
QStringList volumeIntegralValueHeaderFactory(PhysicField physicField);

class VolumeIntegralValueView : public QDockWidget
{
    Q_OBJECT

public slots:
    void doShowVolumeIntegral(VolumeIntegralValue *volumeIntegralValue);

public:
    VolumeIntegralValueView(QWidget *parent = 0);

private:
    QTreeWidget *trvWidget;

    void addValue(QTreeWidgetItem *parent, QString name, QString text, QString unit);

    void showElectrostatic(VolumeIntegralValueElectrostatic *volumeIntegralValueElectrostatic);
    void showMagnetostatic(VolumeIntegralValueMagnetostatic *volumeIntegralValueMagnetostatic);
    void showHeat(VolumeIntegralValueHeat *volumeIntegralValueHeat);
};

class VolumeIntegralValue
{
public:
    double volume;
    double crossSection;

    VolumeIntegralValue();

    virtual QStringList variables() = 0;

protected:
};

class VolumeIntegralValueElectrostatic : public VolumeIntegralValue
{
public:
    double averageElectricFieldX;
    double averageElectricFieldY;
    double averageElectricField;
    double averageDisplacementX;
    double averageDisplacementY;
    double averageDisplacement;
    double energy;

    VolumeIntegralValueElectrostatic();
    QStringList variables();
};

class VolumeIntegralValueMagnetostatic : public VolumeIntegralValue
{
public:
    double averageMagneticFieldX;
    double averageMagneticFieldY;
    double averageMagneticField;
    double averageFluxDensityX;
    double averageFluxDensityY;
    double averageFluxDensity;
    double energy;

    VolumeIntegralValueMagnetostatic();
    QStringList variables();
};

class VolumeIntegralValueHeat : public VolumeIntegralValue
{
public:
    double averageTemperature;
    double averageTemperatureGradientX;
    double averageTemperatureGradientY;
    double averageTemperatureGradient;
    double averageHeatFluxX;
    double averageHeatFluxY;
    double averageHeatFlux;

    VolumeIntegralValueHeat();
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

#endif // VOLUMEINTEGRALVIEW_H
