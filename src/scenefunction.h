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

class SLineEdit;
class ValueLineEdit;
class ValueLineEdit;

struct Point;
class ChartBasic;
class SceneFunction;

Q_DECLARE_METATYPE(SceneFunction *);

class SceneFunction
{
public:
    QString name;
    QString function;
    Value start;
    Value end;

    SceneFunction(const QString &name, const QString &function, Value start, Value end);
    ~SceneFunction();

    int showDialog(QWidget *parent);

    double evaluate(double number, bool fromTable = false) throw (const QString &);
    bool evaluateValues() throw (const QString &);

    QString script();
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

class DSceneFunction: public QDialog
{
    Q_OBJECT

public:
    SceneFunction *m_sceneFunction;

    DSceneFunction(SceneFunction *sceneFunction, QWidget *parent = 0);
    ~DSceneFunction();

private:
    ChartBasic *chart;

    QLineEdit *txtName;
    QLineEdit *txtFunction;
    QLabel *lblError;
    ValueLineEdit *txtStart;
    ValueLineEdit *txtEnd;

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
