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

#ifndef DATATABLEDIALOG_H
#define DATATABLEDIALOG_H

#include "util.h"
#include "datatable.h"
#include "gui.h"

class Chart;

class DataTableDialog: public QDialog
{
    Q_OBJECT

public:
    DataTableDialog(QWidget *parent = 0);
    ~DataTableDialog();

    DataTable *table();
    void setTable(const DataTable *table);

private:
    DataTable *m_table;
    Chart *chartValue;
    QwtPlotCurve *chartValueCurveDots;
    Chart *chartDerivative;
    QwtPlotCurve *chartDerivativeCurveDots;

    QPlainTextEdit *lstX;
    QPlainTextEdit *lstY;

    QLabel *lblInfoX;
    QLabel *lblInfoY;
    QLabel *lblInfoError;

    QCheckBox *chkInterpolation;

    QPushButton *btnOk;
    QPushButton *btnClose;
    QPushButton *btnPlot;

    void createControls();
    void load();
    bool save();

    bool parseTable(bool addToTable = true);

private slots:
    void doAccept();
    void doReject();

    void doTextChanged();
    void doPlot();
};

#endif // DATATABLEDIALOG_H
