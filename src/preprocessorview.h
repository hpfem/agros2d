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

#ifndef PREPROCESSORVIEW_H
#define PREPROCESSORVIEW_H

#include "util.h"

#include <QWebView>

class SceneViewPreprocessor;
class FieldsToobar;

class PreprocessorView : public QDockWidget
{
    Q_OBJECT

public slots:
    void doInvalidated();
    void doProperties();
    void doDelete();

public:
    PreprocessorView(SceneViewPreprocessor *sceneView, QWidget *parent = 0);
    ~PreprocessorView();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    SceneViewPreprocessor *m_sceneViewGeometry;

    QSplitter *splitter;

    QTreeWidget *trvWidget;

    QTreeWidgetItem *boundaryConditionsNode;
    QTreeWidgetItem *materialsNode;

    QTreeWidgetItem *geometryNode;
    QTreeWidgetItem *nodesNode;
    QTreeWidgetItem *edgesNode;
    QTreeWidgetItem *labelsNode;

    QAction *actProperties;
    QAction *actDelete;
    
    QMenu *mnuPreprocessor;
    QWebView *webView;

    FieldsToobar *fieldsToolbar;

    void createActions();
    void createControls();
    void createMenu();
    void createToolBar();

    void clearNodes();

private slots:
    void doContextMenu(const QPoint &pos);
    void doItemDoubleClicked(QTreeWidgetItem *item, int role);
    void doItemSelected(QTreeWidgetItem *item, int role);

    void showInfo();
};

#endif // PREPROCESSORVIEW_H
