#ifndef SCENEFUNCTION_H
#define SCENEFUNCTION_H

#include <QMetaType>
#include <QVariant>
#include <QString>

#include <qwt_scale_map.h>
#include <qwt_symbol.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_printfilter.h>
#include <qwt_counter.h>
#include <qwt_legend.h>
#include <qwt_text.h>
#include <qwt_plot.h>

#include <math.h>

#include "util.h"
#include "gui.h"

class SLineEdit;

struct Point;

class Chart;

class SceneFunction;

Q_DECLARE_METATYPE(SceneFunction *);

class SceneFunction
{
public:
    QString name;
    QString function;

    SceneFunction(const QString &name, const QString &function);
    ~SceneFunction();

    int showDialog(QWidget *parent);

    double evaluate(double number) throw (const QString &);

    QString script();
    QVariant variant();

private:
    QScriptEngine *m_engine;
};

// ************************************************************************************************************************

class DSceneFunction: public QDialog
{
    Q_OBJECT

public:
    SceneFunction *m_sceneFunction;

    DSceneFunction(SceneFunction *sceneFunction, QWidget *parent = 0);
    ~DSceneFunction();

private:
    Chart *chart;

    QLineEdit *txtName;
    SLineEdit *txtFunction;
    SLineEdit *txtStart;
    SLineEdit *txtEnd;

    void createControls();
    void load();
    bool save();

private slots:
    void doAccept();
    void doReject();

    void doSaveImage();
    void doPlot();
};

#endif // SCENEFUNCTION_H
