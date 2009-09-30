#ifndef GUI_H
#define GUI_H

#include <QScriptEngine>
#include <QScriptValue>

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

#include "util.h"
#include "scene.h"
#include "scenemarker.h"

class SceneEdgeMarker;
class SceneLabelMarker;

void fillComboBoxVariable(QComboBox *cmbFieldVariable, PhysicField physicField);

class SLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    SLineEdit(QWidget *parent = 0);
    SLineEdit(const QString &contents, bool hasValidator = true, bool hasScriptEngine = true, QWidget *parent = 0);
    ~SLineEdit();

    double value();
    void setValue(double value);

private:
    QScriptEngine *m_engine;
};

class SLineEditValue : public QLineEdit
{
    Q_OBJECT
public:
    SLineEditValue(QWidget *parent = 0);

    Value value();
    double number();
    bool evaluate();
    void setValue(Value value);

private:
    double m_number;
};

class Chart : public QwtPlot
{
    Q_OBJECT
public:
    Chart(QWidget *parent = 0);
    ~Chart();

    inline QwtPlotCurve *curve() { return m_curve; }
    void saveImage(const QString &fileName = "");

public slots:
   void setData(double *xval, double *yval, int count);

private:
    QwtPlotCurve *m_curve;
};

class CommandDialog : public QDialog
{
    Q_OBJECT
public:
    CommandDialog(QWidget *parent = 0);
    ~CommandDialog();

    inline QString command() { return cmbCommand->currentText(); }

private:
    QComboBox *cmbCommand;
    // QCompleter *completer;

public slots:
   void doAccept();
};

class EdgeMarkerDialog : public QDialog
{
    Q_OBJECT
public:
    EdgeMarkerDialog(QWidget *parent = 0);
    ~EdgeMarkerDialog();

    inline SceneEdgeMarker *marker() { return (cmbMarker->currentIndex() >= 0) ? cmbMarker->itemData(cmbMarker->currentIndex()).value<SceneEdgeMarker *>() : NULL; }

private:
    QComboBox *cmbMarker;

public slots:
    void doAccept();
};

class LabelMarkerDialog : public QDialog
{
    Q_OBJECT
public:
    LabelMarkerDialog(QWidget *parent = 0);
    ~LabelMarkerDialog();

    inline SceneLabelMarker *marker() { return (cmbMarker->currentIndex() >= 0) ? cmbMarker->itemData(cmbMarker->currentIndex()).value<SceneLabelMarker *>() : NULL; }

private:
    QComboBox *cmbMarker;

public slots:
    void doAccept();
};

#endif // GUI_H
