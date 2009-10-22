#ifndef VOLUMEINTEGRALVIEW_H
#define VOLUMEINTEGRALVIEW_H

#include "util.h"
#include "gui.h"

class VolumeIntegralValue
{
public:
    double volume;
    double crossSection;

    VolumeIntegralValue();

    virtual QStringList variables() = 0;
};

class VolumeIntegralValueView : public QDockWidget
{
    Q_OBJECT

public slots:
    void doShowVolumeIntegral();

public:
    VolumeIntegralValueView(QWidget *parent = 0);
    ~VolumeIntegralValueView();

private:
    QTreeWidget *trvWidget;
};

#endif // VOLUMEINTEGRALVIEW_H
