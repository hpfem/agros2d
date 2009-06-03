#ifndef CHARTDIALOG_H
#define CHARTDIALOG_H

#include <QtGui/QDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QFrame>
#include <QtGui/QPainter>
#include <QtGui/QPaintEngine>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>
#include <QtGui/QPrintDialog>
#include <QFileDialog>
#include <QColor>
#include <QAction>
#include <QPrinter>

#include <qwt_scale_map.h>
#include <qwt_symbol.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_counter.h>
#include <qwt_legend.h>
#include <qwt_text.h>
#include <qwt_plot.h>

#include <math.h>

#include "scene.h"
#include "util.h"

class Chart;

class ChartDialog : public QDialog
{
    Q_OBJECT

public slots:
    void doPlot();
    void doPrint();

public:
    ChartDialog(Scene *scene, QWidget *parent);
    ~ChartDialog();

private:
    int m_N;
    Scene *m_scene;

    QwtPlotZoomer *zoomer;
    QwtPlotPicker *picker;
    QwtPlotPanner *panner;

    QAction *actZoom;

    QGroupBox *controls;
    Chart *chart;

    QAction *actPlot;

    void createControls();

    void showInfo(QString text = QString::null);

private slots:
    void doMoved(const QPoint &);
    void doSelected(const QwtPolygon &);
    void doZoomed(const QwtPolygon &);
    void doEnableZoomMode(bool on);

};

class Chart : public QwtPlot
{
    Q_OBJECT
public:
    Chart(QWidget *parent);

    inline QwtPlotCurve *curve() { return m_curve; }

public slots:
   void setData();

private:
    QwtPlotCurve *m_curve;
    QwtPlotMarker *m_marker;
};

#endif // CHARTDIALOG_H
