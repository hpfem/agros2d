#ifndef SCENE_H
#define SCENE_H

#include <QDomDocument>

#include <dl_dxf.h>
#include <dl_creationadapter.h>

#include "util.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenefunction.h"
#include "scenesolution.h"

#include "localvalueview.h"
#include "surfaceintegralview.h"
#include "volumeintegralview.h"

#include "helpdialog.h"
#include "problemdialog.h"
#include "scenetransformdialog.h"
#include "solverdialog.h"

#include "hermes2d/hermes_field.h"
#include "hermes2d/hermes_general.h"
#include "hermes2d/hermes_electrostatic.h"
#include "hermes2d/hermes_magnetostatic.h"
#include "hermes2d/hermes_harmonicmagnetic.h"
#include "hermes2d/hermes_heat.h"
#include "hermes2d/hermes_current.h"
#include "hermes2d/hermes_elasticity.h"

struct HermesObject;
struct HermesGeneral;

class ThreadSolver;

class SceneNode;
class SceneEdge;
class SceneLabel;

class SceneEdgeMarker;
class SceneLabelMarker;

class SceneFunction;

struct SceneViewSettings;
class SceneSolution;

class SolverDialog;
class ProblemDialog;
class SceneTransformDialog;
class HelpDialog;

class ProblemInfo
{    
public:
    QString name;
    QDate date;
    QString fileName;
    PhysicField physicField() { return (m_hermes) ? m_hermes->physicField : PHYSICFIELD_UNDEFINED; }
    ProblemType problemType;
    int numberOfRefinements;
    int polynomialOrder;
    AdaptivityType adaptivityType;
    int adaptivitySteps;
    double adaptivityTolerance; // percent
    QString scriptStartup;
    QString description;

    // harmonic magnetic
    int frequency;

    // transient
    AnalysisType analysisType;
    double timeStep;
    double timeTotal;
    double initialCondition;
          
    ProblemInfo()
    {
        m_hermes = NULL;
        clear();
    }

    ~ProblemInfo()
    {
        if (m_hermes) delete m_hermes;
    }
    
    void clear()
    {
        // hermes object
        if (m_hermes) delete m_hermes;
        m_hermes = new HermesGeneral();
        
        name = QObject::tr("unnamed");
        date = QDate::currentDate();
        fileName = "";
        scriptStartup = "";
        description = "";
        problemType = PROBLEMTYPE_PLANAR;
        numberOfRefinements = 1;
        polynomialOrder = 2;
        adaptivityType = ADAPTIVITYTYPE_NONE;
        adaptivitySteps = 0;
        adaptivityTolerance = 1.0;
        
        // harmonic magnetic
        frequency = 0.0;
        
        // transient
        analysisType = ANALYSISTYPE_STEADYSTATE;
        timeStep = 1.0;
        timeTotal = 1.0;
        initialCondition = 0.0;
    }
    
    inline void setHermes(HermesField *hermes) { if (m_hermes) delete m_hermes; m_hermes = hermes; }
    inline HermesField *hermes() { return m_hermes; }

    inline QString labelX() { return ((problemType == PROBLEMTYPE_PLANAR) ? "X" : "R");  }
    inline QString labelY() { return ((problemType == PROBLEMTYPE_PLANAR) ? "Y" : "Z");  }

private:
    HermesField *m_hermes;
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

class Scene : public QObject
{
    Q_OBJECT
    
    public slots:
    void doNewNode(const Point &point = Point());
    void doNewEdge();
    void doNewLabel();
    void doDeleteSelected();
    void doNewEdgeMarker();
    void doNewLabelMarker();
    void doNewFunction();
    void doTransform();
    void doClearSolution();
    void doProblemProperties();
    
signals:
    void invalidated();
    void solved();
    void fileNameChanged(const QString &fileName);
    
public:
    QList<SceneNode *> nodes;
    QList<SceneEdge *> edges;
    QList<SceneLabel *> labels;
    QList<SceneFunction *> functions;
    
    QList<SceneEdgeMarker *> edgeMarkers;
    QList<SceneLabelMarker *> labelMarkers;
    
    QAction *actNewNode;
    QAction *actNewEdge;
    QAction *actNewLabel;
    QAction *actDeleteSelected;
    QAction *actNewEdgeMarker;
    QAction *actNewLabelMarker;
    QAction *actNewFunction;
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
    
    void addEdgeMarker(SceneEdgeMarker *edgeMarker);
    void removeEdgeMarker(SceneEdgeMarker *edgeMarker);
    void setEdgeEdgeMarker(SceneEdgeMarker *edgeMarker); // set edge marker to selected edges
    SceneEdgeMarker *getEdgeMarker(const QString &name);
    bool setEdgeMarker(const QString &name, SceneEdgeMarker *edgeMarker);
    
    void addLabelMarker(SceneLabelMarker *labelMarker);
    void removeLabelMarker(SceneLabelMarker *labelMarker);
    void setLabelLabelMarker(SceneLabelMarker *labelMarker); // set label marker to selected labels
    SceneLabelMarker *getLabelMarker(const QString &name);
    bool setLabelMarker(const QString &name, SceneLabelMarker *labelMarker);
    
    SceneFunction *addFunction(SceneFunction *function);
    void removeFunction(SceneFunction *function);
    
    void clear();
    
    RectPoint boundingBox();
    
    void selectNone();
    void selectAll(SceneMode sceneMode);
    int selectedCount();
    void highlightNone();
    void deleteSelected();
    
    void transformTranslate(const Point &point, bool copy);
    void transformRotate(const Point &point, double angle, bool copy);
    void transformScale(const Point &point, double scaleFactor, bool copy);
    
    inline ProblemInfo *problemInfo() { return m_problemInfo; }
    inline void setProblemInfo(ProblemInfo *problemInfo) { m_problemInfo = problemInfo; }
    
    inline void refresh() { emit invalidated(); }
    void createMeshAndSolve(SolverMode solverMode);
    inline SceneSolution *sceneSolution() { return m_sceneSolution; }
    
    void readFromDxf(const QString &fileName);
    void writeToDxf(const QString &fileName);
    void readFromFile(const QString &fileName);
    void writeToFile(const QString &fileName);
    
    inline QUndoStack *undoStack() { return m_undoStack; }
    
protected:
    
private:    
    QUndoStack *m_undoStack;
    ProblemInfo *m_problemInfo;
    
    // scene solution
    SceneSolution *m_sceneSolution;
    
    // solver dialog
    SolverDialog *solverDialog;
    
    void createActions();
    
private slots:    
    void doInvalidated();
    void doSolved();
};

// **************************************************************************************

class Util
{
public:
    static Util* singleton();
    static inline Scene *scene() { return Util::singleton()->m_scene; }
    static inline HelpDialog *helpDialog() { return Util::singleton()->m_helpDialog; }
    
protected:
    Util();
    Util(const Util &);
    Util & operator = (const Util &);
    ~Util();
    
private:
    static Util *m_singleton;
    
    Scene *m_scene;
    HelpDialog *m_helpDialog;
};

#endif /* SCENE_H */
