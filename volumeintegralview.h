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

VolumeIntegralValue *volumeIntegralValueFactory(Scene *scene);

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

    VolumeIntegralValue(Scene *scene);

    inline Scene *scene() { return m_scene; }

    virtual QString toString();

protected:
    Scene *m_scene;
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

    VolumeIntegralValueElectrostatic(Scene *scene);
    QString toString();
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

    VolumeIntegralValueMagnetostatic(Scene *scene);
    QString toString();
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

    VolumeIntegralValueHeat(Scene *scene);
    QString toString();
};

#endif // VOLUMEINTEGRALVIEW_H
