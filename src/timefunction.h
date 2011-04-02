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

class TimeFunction
{
public:
    Point point;

    TimeFunction();
    TimeFunction(const QString &function, int count = 200);
    ~TimeFunction();

    void setFunction(const QString &function, double time_min, double time_max, double N = 200, bool quiet = true);
    inline QString function() const { return m_function; }

    inline bool isValid() const { return (m_values.count() > 0); }
    bool isTimeDep() const { return m_function.contains("time"); }
    bool check() const;

    inline QString getError() const { return m_error; }
    void showError();

    inline double timeMin() const { return m_time_min; }
    inline double timeMax() const { return m_time_max; }
    inline void setTimeMin(double time_min) { m_time_min = time_min; }
    inline void setTimeMax(double time_max) { m_time_max = time_max; }

    inline double count() const { return m_count; }
    inline void setCount(int count) { m_count = count; }

    inline QList<double> times() const { return m_times; }
    inline QList<double> values() const { return m_values; }

    bool fillValues(bool quiet = true);
    double value(double time) const;

private:
    double m_time_min;
    double m_time_max;
    int m_count;

    QString m_function;
    QString m_error;

    QList<double> m_times;
    QList<double> m_values;    
};

class Chart;

class TimeFunctionEdit: public QWidget
{
    Q_OBJECT

public:
    TimeFunctionEdit(QWidget *parent = 0);

    void setTimeFunction(const TimeFunction &timeFunction);
    inline TimeFunction timeFunction() const { return m_timeFunction; }

private slots:
    void doOpenDialog();

private:
    QLineEdit *txtFunction;
    QPushButton *btnEdit;

    void createControls();

    TimeFunction m_timeFunction;
};

class TimeFunctionDialog: public QDialog
{
    Q_OBJECT

public:
    TimeFunctionDialog(QWidget *parent = 0);
    ~TimeFunctionDialog();

    inline TimeFunction timeFunction() const { return m_timeFunction; }
    void setTimeFunction(const TimeFunction &timeFunction);

private:
    TimeFunction m_timeFunction;
    Chart *chart;
    QwtPlotCurve *chartCurve;

    QwtPlotPicker *picker;

    QPushButton *btnOk;
    QPushButton *btnClose;
    QPushButton *btnPlot;

    QLabel *lblInfoError;
    QLineEdit *txtFunction;

    void createControls();

private slots:
    void doAccept();
    void doReject();

    void doPlot();
    void doMoved(const QPoint &pos);
};

#endif // TIMEFUNCTION_H
