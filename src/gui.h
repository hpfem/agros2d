#ifndef GUI_H
#define GUI_H

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

void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable);
void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable);
void fillComboBoxTimeStep(QComboBox *cmbFieldVariable);
void addTreeWidgetItemValue(QTreeWidgetItem *parent, const QString &name, const QString &text, const QString &unit);

class SLineEditScript : public QLineEdit
{
    Q_OBJECT
public:
    SLineEditScript(QWidget *parent = 0) : QLineEdit(parent)
    {
    }
};

class SLineEditDouble : public QLineEdit
{
    Q_OBJECT
public:
    SLineEditDouble(double val = 0, QWidget *parent = 0) : QLineEdit(parent)
    {
        setValue(val);
    }

    inline double value() { return text().toDouble(); }
    inline void setValue(double value) { setText(QString::number(value)); }
};

class SLineEditValue : public QWidget
{
    Q_OBJECT
public:
    SLineEditValue(QWidget *parent = 0);

    Value value();
    double number();
    void setValue(Value value);

public slots:
    bool evaluate();

private:
    double m_number;

    QLineEdit *txtLineEdit;
    QLabel *lblValue;
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


#endif // GUI_H
