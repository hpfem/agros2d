#ifndef SCENE_H
#define SCENE_H

#include <QtGui/QMessageBox>
#include <QObject>
#include <QList>
#include <QDate>
#include <QDomDocument>
#include <QTextStream>
#include <QLocale>
#include <QFile>
#include <QAction>

#include <dl_dxf.h>
#include <dl_creationadapter.h>

#include "locale.h"
#include "util.h"
#include "scenemarker.h"
#include "scenebasic.h"
#include "projectdialog.h"
#include "scenetransformdialog.h"

#include "scenehermes.h"
#include "scenesolution.h"
#include "hermes2d/hermes_electrostatic.h"
#include "hermes2d/hermes_magnetostatic.h"
#include "hermes2d/hermes_heat.h"
#include "hermes2d/hermes_elasticity.h"

class ThreadSolver;

class SceneNode;
class SceneEdge;
class SceneLabel;

class SceneEdgeMarker;
class SceneLabelMarker;

struct SceneViewSettings;

class SceneSolution;

class ProjectDialog;
class SceneTransformDialog;

struct ProjectInfo
{
    QString name;
    QDate date;
    QString fileName;
    ProblemType problemType;
    PhysicField physicField;
    int numberOfRefinements;
    int polynomialOrder;
    int adaptivitySteps;
    double adaptivityTolerance;

    ProjectInfo()
    {
        clear();
    }

    void clear()
    {
        name = "unnamed";
        date = QDate::currentDate();
        fileName = "";
        physicField = PHYSICFIELD_ELECTROSTATIC;
        problemType = PROBLEMTYPE_PLANAR;

        numberOfRefinements = 1;
        polynomialOrder = 3;
        adaptivitySteps = 0;
        adaptivityTolerance = 1.0; // %
    }
};

class DxfFilter : public DL_CreationAdapter
{
public:
    DxfFilter(Scene *scene);

    void addArc(const DL_ArcData& a);
    void addLine(const DL_LineData& d);
private:
    Scene *m_scene;
};

class Scene : public QObject {
    Q_OBJECT

public slots:
    void doNewNode(const Point &point = Point());
    void doNewEdge();
    void doNewLabel();
    void doNewEdgeMarker();
    void doNewLabelMarker();  
    void doTransform();
    void doProjectProperties();

signals:
    void invalidated();
    void solved();

public:
    QList<SceneNode *> nodes;
    QList<SceneEdge *> edges;
    QList<SceneLabel *> labels;

    QList<SceneEdgeMarker *> edgeMarkers;
    QList<SceneLabelMarker *> labelMarkers;

    QAction *actNewNode;
    QAction *actNewEdge;
    QAction *actNewLabel;
    QAction *actNewEdgeMarker;
    QAction *actNewLabelMarker;
    QAction *actProjectProperties;
    QAction *actTransform;

    // Settings *settings;

    Scene();
    ~Scene();

    SceneNode *addNode(SceneNode *node);
    void removeNode(SceneNode *node);

    SceneEdge *addEdge(SceneEdge *edge);
    void removeEdge(SceneEdge *edge);

    SceneLabel *addLabel(SceneLabel *label);
    void removeLabel(SceneLabel *label);

    void addEdgeMarker(SceneEdgeMarker *edgeMarker);
    void removeEdgeMarker(SceneEdgeMarker *edgeMarker);
    void setEdgeMarker(SceneEdgeMarker *edgeMarker); // set edge marker to selected edges

    void addLabelMarker(SceneLabelMarker *labelMarker);
    void removeLabelMarker(SceneLabelMarker *labelMarker);
    void setLabelMarker(SceneLabelMarker *labelMarker); // set label marker to selected labels

    void clear();

    RectPoint boundingBox();

    void selectNone();
    void highlightNone();
    void deleteSelected();

    void transformTranslate(const Point &point, bool copy);
    void transformRotate(const Point &point, double angle, bool copy);
    void transformScale(const Point &point, double scaleFactor, bool copy);

    inline bool isMeshed() { return m_isMeshed; }
    inline bool setMeshed(bool value) { m_isMeshed = value; }

    inline ProjectInfo &projectInfo() { return m_projectInfo; }

    inline void refresh() { emit invalidated(); }
    void createMesh();
    void solve();
    inline ThreadSolver *solver() { return m_solver; }
    inline SceneSolution *sceneSolution() { return m_sceneSolution; }

    int writeToTriangle();
    bool triangle2mesh(const QString &source, const QString &destination);
    void readFromDxf(const QString &fileName);
    void writeToDxf(const QString &fileName);
    void readFromFile(const QString &fileName);
    void writeToFile(const QString &fileName);

protected:

private:
    bool m_isSolved;
    bool m_isMeshed;

    ProjectInfo m_projectInfo;

    SceneSolution *m_sceneSolution;

    // Triangle and Hermes solve
    ThreadSolver *m_solver;

    void createActions();

private slots:
    void doSolved();
    void doInvalidated();
};


#endif /* SCENE_H */
