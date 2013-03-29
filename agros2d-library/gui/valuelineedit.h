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

#ifndef VALUELINEEDIT_H
#define VALUELINEEDIT_H

#include "util.h"
#include "util/point.h"
#include "value.h"

#include "datatable.h"

class QwtPlotCurve;
class ChartBasic;
class DataTable;
class FieldInfo;

class ValueLineEdit : public QWidget
{
    Q_OBJECT

public:
    ValueLineEdit(QWidget *parent = 0, bool hasTimeDep = false, bool hasNonlin = false);
    ~ValueLineEdit();

    double number();
    void setNumber(double number);

    Value value();
    void setValue(const Value &value);

    inline void setMinimum(double min) { m_minimum = min; }
    inline void setMinimumSharp(double min) { m_minimumSharp = min; }
    inline void setMaximum(double max) { m_maximum = max; }
    inline void setMaximumSharp(double max) { m_maximumSharp = max; }
    inline void setCondition(QString condition) { m_condition = condition; }

    inline void setLabelX(const QString &labelX) { m_labelX = labelX; }
    inline QString labelX() const { return m_labelX; }
    inline void setLabelY(const QString &labelY) { m_labelY = labelY; }
    inline QString labelY() const { return m_labelY; }

public slots:
    bool evaluate(bool quiet = true);
    bool checkCondition(double value);

signals:
    void editingFinished();
    void textChanged(QString);
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

    QString m_condition;

    bool m_hasTimeDep;
    bool m_hasNonlin;
    DataTable m_table;
    QString m_labelX;
    QString m_labelY;

    QLineEdit *txtLineEdit;
    QLabel *lblValue;
    QLabel *lblInfo;

#ifdef Q_WS_MAC
    QToolButton *btnEditTimeDep;
    QToolButton *btnDataTableDelete;
    QToolButton *btnDataTableDialog;
#else
    QPushButton *btnEditTimeDep;
    QPushButton *btnDataTableDelete;
    QPushButton *btnDataTableDialog;
#endif

    void setLayoutValue();
    void setValueLabel(const QString &text, QColor color, bool isVisible);

private slots:
    void doOpenValueTimeDialog();
    void doOpenDataTableDelete();
    void doOpenDataTableDialog();
};

#endif // VALUELINEEDIT_H
