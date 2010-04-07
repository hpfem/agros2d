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
#include "scenemarkerselectdialog.h"
#include "solverdialog.h"

#include "hermes2d/hermes_field.h"
#include "hermes2d/hermes_general.h"
#include "hermes2d/hermes_electrostatic.h"
#include "hermes2d/hermes_magnetic.h"
#include "hermes2d/hermes_heat.h"
#include "hermes2d/hermes_current.h"
#include "hermes2d/hermes_elasticity.h"

#include "config.h"

struct HermesObject;
struct HermesGeneral;

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

class Config;

class ProblemInfo
{    
public:
    QString name;
    QDate date;
    QString fileName;
    PhysicField physicField() { return (m_hermes) ? m_hermes->physicField : PhysicField_Undefined; }
    ProblemType problemType;
    int numberOfRefinements;
    int polynomialOrder;
    AdaptivityType adaptivityType;
    int adaptivitySteps;
    double adaptivityTolerance; // percent
    QString scriptStartup;
    QString description;

    // harmonic
    int frequency;

    // transient
    AnalysisType analysisType;
    Value timeStep;
    Value timeTotal;
    Value initialCondition;
          
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
        problemType = ProblemType_Planar;
        analysisType = AnalysisType_SteadyState;

        // hermes object
        if (m_hermes) delete m_hermes;
        m_hermes = new HermesGeneral();
        
        name = QObject::tr("unnamed");
        date = QDate::currentDate();
        fileName = "";
        scriptStartup = "";
        description = "";
        numberOfRefinements = 1;
        polynomialOrder = 2;
        adaptivityType = AdaptivityType_None;
        adaptivitySteps = 0;
        adaptivityTolerance = 1.0;
        
        // harmonic
        frequency = 0.0;
        
        // transient
        timeStep = Value("1.0", false);
        timeTotal = Value("1.0", false);
        initialCondition = Value("0.0", false);
    }
    
    inline void setHermes(HermesField *hermes) { if (m_hermes) delete m_hermes; m_hermes = hermes; }
    inline HermesField *hermes() { return m_hermes; }

    inline QString labelX() { return ((problemType == ProblemType_Planar) ? "X" : "R");  }
    inline QString labelY() { return ((problemType == ProblemType_Planar) ? "Y" : "Z");  }

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
    void defaultValues();
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
    inline void setProblemInfo(ProblemInfo *problemInfo) { m_problemInfo = problemInfo; emit defaultValues(); }
    
    inline void refresh() { emit invalidated(); }
    void createMeshAndSolve(SolverMode solverMode);
    inline SceneSolution *sceneSolution() { return m_sceneSolution; }
    
    void readFromDxf(const QString &fileName);
    void writeToDxf(const QString &fileName);
    ErrorResult readFromFile(const QString &fileName);
    ErrorResult writeToFile(const QString &fileName);
    
    inline QUndoStack *undoStack() { return m_undoStack; }
    
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
    HelpDialog *m_helpDialog;
    QCompleter *m_completer;
    Config *m_config;
};

#endif /* SCENE_H */
