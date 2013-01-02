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

#ifndef SCENE_H
#define SCENE_H

#include "util.h"
#include "util/enums.h"
#include "util/conf.h"

#include "hermes2d/solutiontypes.h"

#include <dl_dxf.h>
#include <dl_creationadapter.h>

struct HermesElectrostatic;
struct HermesField;

class Scene;
class SceneNode;
class SceneEdge;
class SceneLabel;
class SceneBoundary;
class SceneMaterial;
struct SceneViewSettings;
struct LoopsInfo;

class SceneNodeContainer;
class SceneEdgeContainer;
class SceneLabelContainer;

class SceneBoundaryContainer;
class SceneMaterialContainer;

class ScriptEngineRemote;

class ProblemWidget;
class SceneTransformDialog;
class ProgressItemSolve;
class CouplingInfo;

class Problem;
class SolutionStore;
class Log;

class PluginInterface;

QString generateSvgGeometry(QList<SceneEdge *> edges);

class DxfFilter : public DL_CreationAdapter
{
public:
    DxfFilter(Scene *scene);

    void addArc(const DL_ArcData& a);
    void addLine(const DL_LineData& l);
    void addCircle(const DL_CircleData& c);
private:
    Scene *m_scene;
};

class NewMarkerAction : public QAction
{
    Q_OBJECT

public:
    NewMarkerAction(QIcon icon, QObject* parent, QString field);

public slots:
    void doTriggered();

signals:
    void triggered(QString);

private:
    QString field;
};

class Scene : public QObject
{
    Q_OBJECT

public slots:
    void doNewNode(const Point &point = Point());
    void doNewEdge();
    void doNewLabel(const Point &point = Point());
    void doDeleteSelected();

    void doNewBoundary();
    void doNewBoundary(QString field);

    void doNewMaterial();
    void doNewMaterial(QString field);

    void doFieldsChanged();

signals:
    void invalidated();
    void cleared();

    void defaultValues();
    void fileNameChanged(const QString &fileName);

public:
    Scene();
    ~Scene();

    SceneNodeContainer* nodes;
    SceneEdgeContainer* edges;
    SceneLabelContainer* labels;

    SceneBoundaryContainer *boundaries;
    SceneMaterialContainer *materials;

    QAction *actNewNode;
    QAction *actNewEdge;
    QAction *actNewLabel;
    QAction *actDeleteSelected;

    QAction *actNewBoundary;
    QMap<QString, QAction*> actNewBoundaries;

    QAction *actNewMaterial;
    QMap<QString, QAction*> actNewMaterials;

    QAction *actTransform;

    // OBSOLETE - DO NOT USE *************************************************************
    SceneNode *addNode(SceneNode *node);
    void removeNode(SceneNode *node);
    SceneNode *getNode(const Point &point);

    SceneEdge *addEdge(SceneEdge *edge);
    void removeEdge(SceneEdge *edge);
    SceneEdge *getEdge(const Point &pointStart, const Point &pointEnd, double angle);

    SceneLabel *addLabel(SceneLabel *label);
    void removeLabel(SceneLabel *label);
    SceneLabel *getLabel(const Point &point);

    void addBoundary(SceneBoundary *boundary);
    void removeBoundary(SceneBoundary *boundary);
    void setBoundary(SceneBoundary *boundary); // set edge marker to selected edges
    SceneBoundary *getBoundary(FieldInfo *field, const QString &name);

    void addMaterial(SceneMaterial *material);
    void removeMaterial(SceneMaterial *material);
    void setMaterial(SceneMaterial *material); // set label marker to selected labels
    SceneMaterial *getMaterial(FieldInfo *field, const QString &name);

    bool checkGeometryAssignement();
    // OBSOLETE - DO NOT USE *************************************************************

    void clear();

    RectPoint boundingBox() const;

    void selectNone();
    void selectAll(SceneGeometryMode sceneMode);
    int selectedCount();
    void deleteSelected();

    void highlightNone();
    int highlightedCount();

    void moveSelectedNodesAndEdges(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy);
    void moveSelectedLabels(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy);
    void transformTranslate(const Point &point, bool copy);
    void transformRotate(const Point &point, double angle, bool copy);
    void transformScale(const Point &point, double scaleFactor, bool copy);

    inline void invalidate() { emit invalidated(); }

    inline FieldInfo* activeViewField() const { assert(m_activeViewField); return m_activeViewField; }
    void setActiveViewField(FieldInfo* fieldInfo);

    inline int activeTimeStep() const { return m_activeTimeStep; }
    void setActiveTimeStep(int ts);

    inline int activeAdaptivityStep() const { return m_activeAdaptivityStep; }
    void setActiveAdaptivityStep(int as) { m_activeAdaptivityStep = as; }

    inline SolutionMode activeSolutionType() const { return m_activeSolutionMode; }
    void setActiveSolutionType(SolutionMode st) { m_activeSolutionMode = st; }

    // active MultiSolutionArray
    MultiSolutionArray<double> activeMultiSolutionArray();

    void readFromDxf(const QString &fileName);
    void writeToDxf(const QString &fileName);
    ErrorResult readFromFile(const QString &fileName);
    ErrorResult writeToFile(const QString &fileName);
    void readSolutionFromFile(const QString &fileName);
    void writeSolutionToFile(const QString &fileName);

    void checkEdge(SceneEdge *edge);
    void checkNode(SceneNode *node);
    void checkNodeConnect(SceneNode *node);
    void checkGeometry();
    ErrorResult checkGeometryResult();

    void addBoundaryAndMaterialMenuItems(QMenu* menu, QWidget* parent);

    inline QUndoStack *undoStack() const { return m_undoStack; }

private:
    QUndoStack *m_undoStack;

    FieldInfo* m_activeViewField;
    int m_activeTimeStep;
    int m_activeAdaptivityStep;
    SolutionMode m_activeSolutionMode;

    void createActions();

private slots:
    void doInvalidated();
};

#endif /* SCENE_H */
