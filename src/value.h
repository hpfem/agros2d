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
#include "util/point.h"
#include "hermes2d/module.h"

#include "datatable.h"

class QwtPlotCurve;
class ChartBasic;
class DataTable;
class FieldInfo;

struct Value
{
    Value();
    Value(FieldInfo *fieldInfo, const QString &value, const DataTable &m_table);
    Value(FieldInfo *fieldInfo, const QString &value, bool evaluateExpression = true);
    Value(const QString &value, bool evaluateExpression = true);
    Value(double value, std::vector<double> x, std::vector<double> y);
    Value(const QString &value, std::vector<double> x, std::vector<double> y);
    Value(FieldInfo *fieldInfo, double value, std::vector<double> x, std::vector<double> y);
    Value(FieldInfo *fieldInfo, const QString &value, std::vector<double> x, std::vector<double> y);
    ~Value();

    double number();

    bool hasExpression();

    inline DataTable table() const { return m_table; }
    bool hasTable() const;

    double value(double key = 0.0);
    double value(const Point &point);
    double value(double time, const Point &point);
    Hermes::Ord value(Hermes::Ord ord);
    double derivative(double key = 0.0);
    Hermes::Ord derivative(Hermes::Ord ord);

    bool evaluate(bool quiet = false);
    bool evaluate(double time, bool quiet);
    bool evaluate(const Point &point, bool quiet);
    bool evaluate(double time, const Point &point, bool quiet = false);

    QString toString() const;
    void fromString(const QString &str);

    inline void setText(const QString &str) { m_isEvaluated = false; m_text = str; }
    inline QString text() const { return m_text; }

    inline FieldInfo *fieldInfo() const { return m_fieldInfo; }

private:
    FieldInfo *m_fieldInfo;
    bool m_isEvaluated;
    QString m_text;
    double m_number;

    DataTable m_table;
};

// ****************************************************************************************************

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
    void evaluated(bool isError);

protected:
    virtual QSize sizeHint() const;
    void focusInEvent(QFocusEvent *event);

private:
    FieldInfo *m_fieldInfo;

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

// ****************************************************************************************************

class ValueTimeDialog: public QDialog
{
    Q_OBJECT

public:
    ValueTimeDialog(QWidget *parent = 0);
    ~ValueTimeDialog();

    Value value() const { return Value(txtLineEdit->text()); }
    void setValue(Value value);

private:
    ChartBasic *chart;
    QwtPlotCurve *chartCurve;

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
    void presetsChanged(int index);
};

#endif // TIMEFUNCTION_H
