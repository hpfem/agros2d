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

namespace Hermes
{
    namespace Module
    {
        struct ModuleAgros;
    }
}

class ProblemInfo : public QObject
{
    Q_OBJECT
public:
    inline QString labelX() { return ((m_coordinateType == CoordinateType_Planar) ? "X" : "R");  }
    inline QString labelY() { return ((m_coordinateType == CoordinateType_Planar) ? "Y" : "Z");  }
    inline QString labelZ() { return ((m_coordinateType == CoordinateType_Planar) ? "Z" : "a");  }

    void clear();

    inline QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    inline QString fileName() const { return m_fileName; }
    void setFileName(const QString &fileName) { m_fileName = fileName; emit changed(); }

    inline CoordinateType coordinateType() const { return m_coordinateType; }
    void setCoordinateType(const CoordinateType coordinateType) { m_coordinateType = coordinateType; emit changed(); }

    inline double frequency() const { return m_frequency; }
    void setFrequency(const double frequency) { m_frequency = frequency; emit changed(); }

    inline Value timeStep() const { return m_timeStep; }
    void setTimeStep(const Value &timeStep) { m_timeStep = timeStep; emit changed(); }

    inline Value timeTotal() const { return m_timeTotal; }
    void setTimeTotal(const Value &timeTotal) { m_timeTotal = timeTotal; emit changed(); }

    inline Hermes::MatrixSolverType matrixSolver() const { return m_matrixSolver; }
    void setMatrixSolver(const Hermes::MatrixSolverType matrixSolver) { m_matrixSolver = matrixSolver; emit changed(); }

    inline MeshType meshType() const { return m_meshType; }
    void setMeshType(const MeshType meshType) { m_meshType = meshType; emit changed(); }

    inline QString startupscript() const { return m_startupscript; }
    void setStartupScript(const QString &startupscript) { m_startupscript = startupscript; emit changed(); }

    inline QString description() const { return m_description; }
    void setDescription(const QString &description) { m_description = description; }

    void refresh() { emit changed(); }

signals:
    void changed();

private:
    QString m_name;
    QString m_fileName;
    CoordinateType m_coordinateType;

    // harmonic
    double m_frequency;

    // transient
    Value m_timeStep;
    Value m_timeTotal;

    // matrix solver
    Hermes::MatrixSolverType m_matrixSolver;

    // mesh type
    MeshType m_meshType;

    QString m_startupscript;
    QString m_description;
};

class FieldInfo
{
public:
    FieldInfo(QString fieldId = "");
    ~FieldInfo();

    void clear();

    inline Hermes::Module::ModuleAgros *module() const { return m_module; }

    QString fieldId() { return m_fieldId; }
    AnalysisType analysisType() { return m_analysisType; }
    void setAnalysisType(AnalysisType analysisType);

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

ostream& operator<<(ostream& output, FieldInfo& id);

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

    void clearSolutions();
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

    bool checkGeometryAssignement();
    // OBSOLETE - DO NOT USE *************************************************************

    void clear();

    RectPoint boundingBox() const;

    void selectNone();
    void selectAll(SceneGeometryMode sceneMode);
    int selectedCount();
    void highlightNone();
    void deleteSelected();

    void moveSelectedNodesAndEdges(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy);
    void moveSelectedLabels(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy);
    void transformTranslate(const Point &point, bool copy);
    void transformRotate(const Point &point, double angle, bool copy);
    void transformScale(const Point &point, double scaleFactor, bool copy);

    inline ProblemInfo *problemInfo() { return m_problemInfo; }
    void setProblemInfo(ProblemInfo *problemInfo) { clear(); delete m_problemInfo; m_problemInfo = problemInfo; emit defaultValues(); }

    inline QMap<QString, FieldInfo *> fieldInfos() const { return m_fieldInfos; }
    inline FieldInfo *fieldInfo(QString name) { assert(m_fieldInfos.contains(name)); return m_fieldInfos[name]; }
    inline FieldInfo *fieldInfo(std::string name) { return fieldInfo(QString::fromStdString(name)); }
    inline FieldInfo *fieldInfo(const char* name) { return fieldInfo(QString::fromAscii(name)); }

    inline QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > couplingInfos() const { return m_couplingInfos; }
    inline CouplingInfo* couplingInfo(FieldInfo* sourceField, FieldInfo* targetField);
    inline void setCouplingInfos(QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > couplingInfos) { m_couplingInfos = couplingInfos; }

    void synchronizeCouplings();

    bool hasField(QString field) { return m_fieldInfos.contains(field); }

    void addField(FieldInfo *field);
    void removeField(FieldInfo *field);

    inline void refresh() { emit invalidated(); }

    // clears all solutions and remove them
    inline FieldInfo* activeViewField() const { assert(m_activeViewField); return m_activeViewField; }
    void setActiveViewField(FieldInfo* fieldInfo) { m_activeViewField = fieldInfo; }
    inline int activeTimeStep() const { return m_activeTimeStep; }
    void setActiveTimeStep(int ts) { m_activeTimeStep = ts; }
    inline int activeAdaptivityStep() const { return m_activeAdaptivityStep; }
    void setActiveAdaptivityStep(int as) { m_activeAdaptivityStep = as; }
    inline SolutionType activeSolutionType() const { return m_activeSolutionType; }
    void setActiveSolutionType(SolutionType st) { m_activeSolutionType = st; }
    // active MultiSolutionArray
    MultiSolutionArray<double> activeMultiSolutionArray();

    void readFromDxf(const QString &fileName);
    void writeToDxf(const QString &fileName);
    ErrorResult readFromFile(const QString &fileName);
    ErrorResult writeToFile(const QString &fileName);

    void addBoundaryAndMaterialMenuItems(QMenu* menu, QWidget* parent);

    inline QUndoStack *undoStack() const { return m_undoStack; }

   // void createSolutions();

private:
    QUndoStack *m_undoStack;

    ProblemInfo* m_problemInfo;
    QMap<QString, FieldInfo *>  m_fieldInfos;
    QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* >  m_couplingInfos;

    FieldInfo* m_activeViewField;
    int m_activeTimeStep;
    int m_activeAdaptivityStep;
    SolutionType m_activeSolutionType;

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
    static inline Config *config() { return Util::singleton()->m_config; }
    static inline Problem *problem() { return Util::singleton()->m_problem; }
    static inline SolutionStore *solutionStore() { return Util::singleton()->m_solutionStore; }
    static inline Log *log() { return Util::singleton()->m_log; }

protected:
    Util();
    Util(const Util &);
    Util & operator = (const Util &);
    ~Util();

private:
    static Util *m_singleton;

    Scene *m_scene;
    Config *m_config;
    Problem *m_problem;
    SolutionStore *m_solutionStore;
    ScriptEngineRemote *m_scriptEngineRemote;
    Log *m_log;
};

#endif /* SCENE_H */
