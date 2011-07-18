// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#ifndef TIMEFUNCTION_H
#define TIMEFUNCTION_H

#include "util.h"

class QwtPlotCurve;
class QwtPlotPicker;
class Chart;

struct Value
{
    QString text;
    double number;

    Value() { text = "0"; number = 0;}
    inline Value(const QString &value, bool evaluateExpression = true) { text = value; if (evaluateExpression) evaluate(true); }

    bool evaluate(bool quiet = false);
    bool evaluate(double time, bool quiet = false);

    bool isTimeDep() const;
};

// ****************************************************************************************************

class ValueLineEdit : public QWidget
{
    Q_OBJECT
public:
    ValueLineEdit(QWidget *parent = 0, bool hasTimeDep = false);

    double number();
    void setNumber(double number);

    Value value();
    void setValue(Value value);

    inline void setMinimum(double min) { m_minimum = min; }
    inline void setMinimumSharp(double min) { m_minimumSharp = min; }
    inline void setMaximum(double max) { m_maximum = max; }
    inline void setMaximumSharp(double max) { m_maximumSharp = max; }

public slots:
    bool evaluate(bool quiet = true);
    void doOpenValueTimeDialog();

signals:
    void editingFinished();
    void evaluated(bool isError);

protected:
    virtual QSize sizeHint() const;
    void focusInEvent(QFocusEvent *event);

private:
    double m_minimum;
    double m_minimumSharp;
    double m_maximum;
    double m_maximumSharp;
    double m_number;

    bool m_hasTimeDep;

    QLineEdit *txtLineEdit;
    QLabel *lblValue;
#ifdef Q_WS_MAC
    QToolButton *btnEdit;
#else
    QPushButton *btnEdit;
#endif

    void setLabel(const QString &text, QColor color, bool isVisible);
};

// ****************************************************************************************************

class ValueTimeDialog: public QDialog
{
    Q_OBJECT

public:
    ValueTimeDialog(QWidget *parent = 0);
    ~ValueTimeDialog();

    Value value() const { return Value(txtLineEdit->text()); }
    void setValue(const Value &timeFunction);

private:
    Chart *chart;
    QwtPlotCurve *chartCurve;

    QwtPlotPicker *picker;

    QPushButton *btnOk;
    QPushButton *btnClose;
    QPushButton *btnPlot;

    QLabel *lblInfoError;
    QLineEdit *txtLineEdit;
    ValueLineEdit *txtTimeTotal;
    QComboBox *cmbPresets;

    void createControls();

private slots:
    void doAccept();
    void doReject();

    void checkExpression();
    void plotFunction();
    void crossMoved(const QPoint &pos);
    void presetsChanged(int index);
};

#endif // TIMEFUNCTION_H
