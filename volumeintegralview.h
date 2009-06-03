#ifndef VOLUMEINTEGRALVIEW_H
#define VOLUMEINTEGRALVIEW_H

#include <QtGui/QDockWidget>
#include <QtGui/QTreeWidget>

#include "util.h"
#include "scene.h"

class VolumeIntegralValue;
class VolumeIntegralValueElectrostatic2D;
class VolumeIntegralValueMagnetostatic2D;
class VolumeIntegralValueHeat2D;

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

    void showElectrostatic2D(VolumeIntegralValueElectrostatic2D *volumeIntegralValueElectrostatic2D);
    void showMagnetostatic2D(VolumeIntegralValueMagnetostatic2D *volumeIntegralValueMagnetostatic2D);
    void showHeat2D(VolumeIntegralValueHeat2D *volumeIntegralValueHeat2D);
};

class VolumeIntegralValue
{
public:
    double volume;
    double cross_section;

    VolumeIntegralValue(Scene *scene);

    inline Scene *scene() { return m_scene; }

    virtual QString toString();

protected:
    Scene *m_scene;
};

class VolumeIntegralValueElectrostatic2D : public VolumeIntegralValue
{
public:
    double averageElectricFieldX;
    double averageElectricFieldY;
    double averageElectricField;
    double averageDisplacementX;
    double averageDisplacementY;
    double averageDisplacement;
    double energy;

    VolumeIntegralValueElectrostatic2D(Scene *scene);
    QString toString();
};

class VolumeIntegralValueMagnetostatic2D : public VolumeIntegralValue
{
public:
    double averageMagneticFieldX;
    double averageMagneticFieldY;
    double averageMagneticField;
    double averageFluxDensityX;
    double averageFluxDensityY;
    double averageFluxDensity;
    double energy;

    VolumeIntegralValueMagnetostatic2D(Scene *scene);
    QString toString();
};

class VolumeIntegralValueHeat2D : public VolumeIntegralValue
{
public:
    double averageTemperature;
    double averageTemperatureGradientX;
    double averageTemperatureGradientY;
    double averageTemperatureGradient;
    double averageHeatFluxX;
    double averageHeatFluxY;
    double averageHeatFlux;

    VolumeIntegralValueHeat2D(Scene *scene);
    QString toString();
};

#endif // VOLUMEINTEGRALVIEW_H
