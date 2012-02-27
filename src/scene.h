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
#include "conf.h"

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

class SceneNodeContainer;
class SceneEdgeContainer;
class SceneLabelContainer;

class SceneBoundaryContainer;
class SceneMaterialContainer;

template <typename Scalar> class SceneSolution;

class ScriptEngineRemote;

class ProblemDialog;
class SceneTransformDialog;
class ProgressItemSolve;
class CouplingInfo;

class Problem;

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
    CoordinateType coordinateType;

    // harmonic
    double frequency;

    // transient
    Value timeStep;
    Value timeTotal;

    // matrix solver
    Hermes::MatrixSolverType matrixSolver;

    // mesh type
    MeshType meshType;

    QString startupscript;
    QString description;

    inline QString labelX() { return ((coordinateType == CoordinateType_Planar) ? "X" : "R");  }
    inline QString labelY() { return ((coordinateType == CoordinateType_Planar) ? "Y" : "Z");  }
    inline QString labelZ() { return ((coordinateType == CoordinateType_Planar) ? "Z" : "a");  }

    void clear();
};

class FieldInfo
{
public:
    FieldInfo(ProblemInfo* parent, QString fieldId = "");
    ~FieldInfo();

    void clear();

    inline Hermes::Module::ModuleAgros *module() const { return m_module; }

    QString fieldId() { return m_fieldId; }
    AnalysisType analysisType() { return m_analysisType; }
    void setAnalysisType(AnalysisType analysisType);

    QString name() { return m_parent->name; }
    QDate date() { return m_parent->date; }
    QString fileName() { return m_parent->fileName; }
    CoordinateType coordinateType() { return m_parent->coordinateType; }
    double frequency() { return m_parent->frequency; }
    Value timeStep() {return m_parent->timeStep; }
    Value timeTotal() {return m_parent->timeTotal; }

    QString labelX() { return m_parent->labelX();  }
    QString labelY() { return m_parent->labelY();  }
    QString labelZ() { return m_parent->labelZ();  }

    inline ProblemInfo* problemInfo() { return m_parent; }

//    int numberOfSolutions() const;

    // linearity
    LinearityType linearityType;
    double nonlinearTolerance; // percent
    int nonlinearSteps;

    int numberOfRefinements;
    int polynomialOrder;

    AdaptivityType adaptivityType;
    int adaptivitySteps;
    double adaptivityTolerance; // percent

    // transient
    Value initialCondition;

    // weakforms
    WeakFormsType weakFormsType;
private:
    /// module
    Hermes::Module::ModuleAgros *m_module;

    /// pointer to problem info, whose this object is a "subfield"
    ProblemInfo *m_parent;

    /// unique field info
    QString m_fieldId;

    // analysis type
    AnalysisType m_analysisType;
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

    void doTransform();
    void doClearSolution();
    void doProblemProperties();
    void doFieldsChanged();

signals:
    void invalidated();
    void cleared();

    /// emited when an field is added or removed. Menus need to adjusted
    void fieldsChanged();

    void defaultValues();
    void fileNameChanged(const QString &fileName);

public:

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

    QAction *actProblemProperties;
    QAction *actClearSolutions;
    QAction *actTransform;

    Scene();
    ~Scene();

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
    SceneBoundary *getBoundary(const QString &name);

    void addMaterial(SceneMaterial *material);
    void removeMaterial(SceneMaterial *material);
    void setMaterial(SceneMaterial *material); // set label marker to selected labels
    SceneMaterial *getMaterial(const QString &name);
    // OBSOLETE - DO NOT USE *************************************************************

    void clear();

    RectPoint boundingBox() const;

    void selectNone();
    void selectAll(SceneGeometryMode sceneMode);
    int selectedCount();
    void highlightNone();
    void deleteSelected();

    void transformTranslate(const Point &point, bool copy);
    void transformRotate(const Point &point, double angle, bool copy);
    void transformScale(const Point &point, double scaleFactor, bool copy);

    inline ProblemInfo *problemInfo() { return m_problemInfo; }
    void setProblemInfo(ProblemInfo *problemInfo) { clear(); delete m_problemInfo; m_problemInfo = problemInfo; emit defaultValues(); }

    inline QMap<QString, FieldInfo *> fieldInfos() const { return m_fieldInfos; }
    inline FieldInfo *fieldInfo(QString name) { assert(m_fieldInfos.contains(name)); return m_fieldInfos[name]; }
    inline FieldInfo *fieldInfo(std::string name) { return fieldInfo(QString::fromStdString(name)); }
    inline FieldInfo *fieldInfo(const char* name) { return fieldInfo(QString::fromAscii(name)); }

    //TODO remove
    inline FieldInfo *fieldInfo() { if(m_fieldInfos.size() == 0) return NULL;  return *m_fieldInfos.begin(); }

    inline QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > couplingInfos() const { return m_couplingInfos; }
    inline CouplingInfo* couplingInfo(FieldInfo* sourceField, FieldInfo* targetField);
    inline void setCouplingInfos(QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > couplingInfos) { m_couplingInfos = couplingInfos; }

    void synchronizeCouplings();

    bool hasField(QString field) { return m_fieldInfos.contains(field); }

    void addField(FieldInfo *field);
    void removeField(FieldInfo *field);

    inline void refresh() { emit invalidated(); }
    inline SceneSolution<double> *sceneSolution(FieldInfo* fieldInfo) const { return m_sceneSolutions[fieldInfo]; }
    inline SceneSolution<double> *activeSceneSolution() const { return m_sceneSolutions[activeViewField()]; }
    inline FieldInfo* activeViewField() const { assert(m_activeViewField); return m_activeViewField; }
    void setActiveViewField(FieldInfo* fieldInfo) { m_activeViewField = fieldInfo; }

    void readFromDxf(const QString &fileName);
    void writeToDxf(const QString &fileName);
    ErrorResult readFromFile(const QString &fileName);
    ErrorResult writeToFile(const QString &fileName);

    void addBoundaryAndMaterialMenuItems(QMenu* menu, QWidget* parent);

    inline QUndoStack *undoStack() const { return m_undoStack; }

    void createSolutions();

private:
    QUndoStack *m_undoStack;

    ProblemInfo* m_problemInfo;
    QMap<QString, FieldInfo *>  m_fieldInfos;
    QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* >  m_couplingInfos;

    QMap<FieldInfo*, SceneSolution<double>* > m_sceneSolutions;
    FieldInfo* m_activeViewField;

    void createActions();

    // clears all solutions and remove them
    void clearSolutions();


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
    static inline Problem *problem() { return Util::singleton()->m_problem; }

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
    Problem *m_problem;
    ScriptEngineRemote *m_scriptEngineRemote;
};

#endif /* SCENE_H */
