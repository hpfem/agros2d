#ifndef VIEWPROBLEM_H
#define VIEWPROBLEM_H

#include "scene.h"
#include "sceneview.h"

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
    QTreeWidgetItem *functionsNode;

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

#endif // VIEWPROBLEM_H
