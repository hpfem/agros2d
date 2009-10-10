#ifndef SURFACEINTEGRALVIEW_H
#define SURFACEINTEGRALVIEW_H

#include "util.h"
#include "gui.h"

class SurfaceIntegralValue
{
public:
    double length;
    double surface;

    SurfaceIntegralValue();

    virtual QStringList variables() = 0;
};

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
};

#endif // SURFACEINTEGRALVIEW_H
