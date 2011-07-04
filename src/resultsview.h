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

#ifndef LOCALVALUE_H
#define LOCALVALUE_H

#include "util.h"
#include "hermes2d/localpoint.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"

class ValueLineEdit;
class SceneMaterial;
class Solution;

class ResultsView : public QDockWidget
{
    Q_OBJECT

public slots:
    void doPostprocessorModeGroupChanged(SceneModePostprocessor sceneModePostprocessor);
    void doShowResults();

    void doShowPoint();
    void doShowPoint(const Point &m_point);
    void doShowVolumeIntegral();
    void doShowSurfaceIntegral();

public:
    ResultsView(QWidget *parent = 0);

private:
    SceneModePostprocessor m_sceneModePostprocessor;
    Point m_point;

    QPushButton *btnPoint;
    QPushButton *btnMarker;
    QTextEdit *txtView;
    QAction *actPoint;
    QAction *actMarker;

    void createActions();

private slots:
    void doPoint();
    void doMarker();
};

class LocalPointValueDialog : public QDialog
{
    Q_OBJECT
public:
    LocalPointValueDialog(Point point, QWidget *parent = 0);

    Point point();

private:
    QDialogButtonBox *buttonBox;

    ValueLineEdit *txtPointX;
    ValueLineEdit *txtPointY;

private slots:
    void evaluated(bool isError);
};

#endif // LOCALVALUE_H
