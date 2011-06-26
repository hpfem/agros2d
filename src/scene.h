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
#include "config.h"

#include "hermes2d/hermes_field.h"

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
class SceneSolution;
class ScriptEngineRemote;

class ProblemDialog;
class SceneTransformDialog;

namespace Hermes
{
    namespace Module
    {
        struct ModuleAgros;
    }
}

class ProblemInfo
{
public:
    QString name;
    QDate date;
    QString fileName;
    PhysicField physicField();
    ProblemType problemType;
    int numberOfRefinements;
    int polynomialOrder;
    MeshType meshType;
    AdaptivityType adaptivityType;
    int adaptivitySteps;
    double adaptivityTolerance; // percent
    QString scriptStartup;
    QString description;

    // linearity
    LinearityType linearityType;
    double linearityNonlinearTolerance; // percent
    int linearityNonlinearSteps;

    // harmonic
    double frequency;

    // transient
    AnalysisType analysisType;
    Value timeStep;
    Value timeTotal;
    Value initialCondition;

    // matrix solver
    MatrixSolverType matrixSolver;

    // module
    Hermes::Module::ModuleAgros *module;

    ProblemInfo()
    {
        m_hermes = NULL;
        module = NULL;
        clear();
    }

    ~ProblemInfo()
    {
        if (m_hermes) delete m_hermes;
    }

    void clear();

    inline bool isLinear() const { return (linearityType == LinearityType_Linear); }

    void setHermes(HermesField *hermes);
    inline HermesField *hermes() const { return m_hermes; }

    inline QString labelX() { return ((problemType == ProblemType_Planar) ? "X" : "R");  }
    inline QString labelY() { return ((problemType == ProblemType_Planar) ? "Y" : "Z");  }
    inline QString labelZ() { return ((problemType == ProblemType_Planar) ? "Z" : "a");  }

private:
    HermesField *m_hermes;
};

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

class Scene : public QObject
{
    Q_OBJECT

public slots:
    void doNewNode(const Point &point = Point());
    void doNewEdge();
    void doNewLabel(const Point &point = Point());
    void doDeleteSelected();
    void doNewBoundary();
    void doNewMaterial();
    void doTransform();
    void doClearSolution();
    void doProblemProperties();

signals:
    void invalidated();
    void defaultValues();
    void fileNameChanged(const QString &fileName);

public:
    QList<SceneNode *> nodes;
    QList<SceneEdge *> edges;
    QList<SceneLabel *> labels;

    QList<SceneBoundary *> boundaries;
    QList<SceneMaterial *> materials;

    QAction *actNewNode;
    QAction *actNewEdge;
    QAction *actNewLabel;
    QAction *actDeleteSelected;
    QAction *actNewBoundary;
    QAction *actNewMaterial;
    QAction *actProblemProperties;
    QAction *actClearSolution;
    QAction *actTransform;

    Scene();
    ~Scene();

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
    SceneBoundary *getBoundary(const QString &name);
    bool setBoundary(const QString &name, SceneBoundary *boundary);
    void replaceBoundary(SceneBoundary *boundary);

    void addMaterial(SceneMaterial *material);
    void removeMaterial(SceneMaterial *material);
    void setMaterial(SceneMaterial *material); // set label marker to selected labels
    SceneMaterial *getMaterial(const QString &name);
    bool setMaterial(const QString &name, SceneMaterial *material);
    void replaceMaterial(SceneMaterial *material);

    void clear();

    RectPoint boundingBox() const;

    void selectNone();
    void selectAll(SceneMode sceneMode);
    int selectedCount();
    void highlightNone();
    void deleteSelected();

    void transformTranslate(const Point &point, bool copy);
    void transformRotate(const Point &point, double angle, bool copy);
    void transformScale(const Point &point, double scaleFactor, bool copy);

    inline ProblemInfo *problemInfo() { return m_problemInfo; }
    void setProblemInfo(ProblemInfo *problemInfo) { clear(); delete m_problemInfo; m_problemInfo = problemInfo; emit defaultValues(); }

    inline void refresh() { emit invalidated(); }
    inline SceneSolution *sceneSolution() const { return m_sceneSolution; }

    void readFromDxf(const QString &fileName);
    void writeToDxf(const QString &fileName);
    ErrorResult readFromFile(const QString &fileName);
    ErrorResult writeToFile(const QString &fileName);

    inline QUndoStack *undoStack() const { return m_undoStack; }

private:
    QUndoStack *m_undoStack;
    ProblemInfo *m_problemInfo;

    // scene solution
    SceneSolution *m_sceneSolution;

    void createActions();

private slots:
    void doInvalidated();
};

// **************************************************************************************

class Util
{
public:
    static void createSingleton();
    static Util* singleton();
    static inline Scene *scene() { return Util::singleton()->m_scene; }
    static inline QCompleter *completer() { return Util::singleton()->m_completer; }
    static inline Config *config() { return Util::singleton()->m_config; }

protected:
    Util();
    Util(const Util &);
    Util & operator = (const Util &);
    ~Util();

private:
    static Util *m_singleton;

    Scene *m_scene;
    QCompleter *m_completer;
    Config *m_config;
    ScriptEngineRemote *m_scriptEngineRemote;
};

#endif /* SCENE_H */
