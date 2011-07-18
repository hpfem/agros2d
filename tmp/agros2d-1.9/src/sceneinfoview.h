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

#ifndef SCENEINFOVIEW_H
#define SCENEINFOVIEW_H

#include "util.h"

class SceneView;

class SceneInfoView : public QDockWidget
{
    Q_OBJECT

public slots:
    void doInvalidated();
    void doProperties();
    void doDelete();

public:
    SceneInfoView(SceneView *sceneView, QWidget *parent = 0);
    ~SceneInfoView();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    SceneView *m_sceneView;

    QTreeWidget *trvWidget;

    QTreeWidgetItem *problemNode;
    QTreeWidgetItem *problemInfoGeneralNode;
    QTreeWidgetItem *problemInfoSolverNode;
    QTreeWidgetItem *boundaryConditionsNode;

    QTreeWidgetItem *materialsNode;
    QTreeWidgetItem *geometryNode;
    QTreeWidgetItem *nodesNode;
    QTreeWidgetItem *edgesNode;
    QTreeWidgetItem *labelsNode;

    QAction *actProperties;
    QAction *actDelete;
    
    QMenu *mnuInfo;

    void createActions();
    void createTreeView();
    void createMenu();
    void createToolBar();

    void clearNodes();

private slots:
    void doContextMenu(const QPoint &pos);
    void doItemDoubleClicked(QTreeWidgetItem *item, int role);
    void doItemSelected(QTreeWidgetItem *item, int role);
};

#endif // SCENEINFOVIEW_H
