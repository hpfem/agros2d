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

class PreprocessorWidget : public QWidget
{
    Q_OBJECT

public slots:
    void refresh();
    void doProperties();
    void doDelete();

public:
    PreprocessorWidget(SceneViewPreprocessor *sceneView, QWidget *parent = 0);
    ~PreprocessorWidget();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    SceneViewPreprocessor *m_sceneViewPreprocessor;

    QTreeWidget *trvWidget;

    QAction *actProperties;
    QAction *actDelete;
    
    QMenu *mnuPreprocessor;

    void createActions();
    void createControls();
    void createMenu();
    void createToolBar();

private slots:
    void doContextMenu(const QPoint &pos);
    void doItemDoubleClicked(QTreeWidgetItem *item, int role);
    void doItemSelected(QTreeWidgetItem *item, int role);
};

#endif // PREPROCESSORVIEW_H
