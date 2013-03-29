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

#ifndef VALUETIMEDIALOG_H
#define VALUETIMEDIALOG_H

#include "util.h"
#include "util/point.h"
#include "value.h"

#include "datatable.h"

class QwtPlotCurve;
class ChartBasic;
class DataTable;
class FieldInfo;
class ValueLineEdit;

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

#endif // VALUETIMEDIALOG_H
