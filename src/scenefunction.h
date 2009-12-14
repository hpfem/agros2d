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
#include "gui.h"

class SLineEdit;

struct Point;

class Chart;

class SceneFunction;

Q_DECLARE_METATYPE(SceneFunction *);

class SceneFunction
{
public:
    QString name;
    QString function;

    SceneFunction(const QString &name, const QString &function);
    ~SceneFunction();

    int showDialog(QWidget *parent);

    double evaluate(double number) throw (const QString &);

    QString script();
    QVariant variant();
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
    Chart *chart;

    QLineEdit *txtName;
    QLineEdit *txtFunction;
    SLineEditDouble *txtStart;
    SLineEditDouble *txtEnd;

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
