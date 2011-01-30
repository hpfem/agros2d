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

#ifndef SCENEFUNCTION_H
#define SCENEFUNCTION_H

#include "util.h"
#include "datatable.h"

class SLineEdit;
class SLineEditDouble;
class SLineEditValue;

struct Point;
class Chart;
class DataFunction;

Q_DECLARE_METATYPE(DataFunction *);

class DataFunction
{
public:
    QString function;
    double start;
    double end;

    DataFunction(const QString &function = "array([0, 1], [0, 1])", double start = 0.0, double end = 0.0);
    ~DataFunction();

    int showDialog(QWidget *parent);

    double evaluate(double number, bool fromTable = false) throw (const QString &);
    bool evaluateValues() throw (const QString &);

    QVariant variant();

    inline int count() { return m_count; }
    double *valuesX() { return m_valuesX; }
    double *valuesY() { return m_valuesY; }

private:
    int m_countDefault;
    int m_count;
    double *m_valuesX;
    double *m_valuesY;
};

// ************************************************************************************************************************

class DDataFunction: public QDialog
{
    Q_OBJECT

public:
    DataFunction *m_sceneFunction;

    DDataFunction(DataFunction *sceneFunction, QWidget *parent = 0);

private:
    Chart *chart;

    QLineEdit *txtFunction;
    QLabel *lblError;
    SLineEditDouble *txtStart;
    SLineEditDouble *txtEnd;

    QPushButton *btnOk;
    QPushButton *btnClose;
    QPushButton *btnPlot;
    QPushButton *btnSaveImage;

    void createControls();
    void load();
    bool save();

private slots:
    void doAccept();
    void doReject();

    void doSaveImage();
    void doPlot();
};


// ************************************************************************************************************************

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
    Chart *chart;

    QPlainTextEdit *lstX;
    QPlainTextEdit *lstY;

    QPushButton *btnOk;
    QPushButton *btnClose;
    QPushButton *btnPlot;

    void createControls();
    void load();
    bool save();

    void parseTable();

private slots:
    void doAccept();
    void doReject();

    void doPlot();   
};

#endif // SCENEFUNCTION_H
