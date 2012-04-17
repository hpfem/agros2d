#ifndef PROBLEM_H
#define PROBLEM_H

#include "util.h"
#include "solutiontypes.h"

template <typename Scalar>
class WeakFormAgros;

class FieldInfo;
class CouplingInfo;

class ProgressDialog;
class MeshGeneratorTriangle;
class ProgressItemSolve;
class ProgressItemSolveAdaptiveStep;
class ProgressItemProcessView;

class Problem;

template <typename Scalar>
class Solver;

namespace Hermes
{
    namespace Module
    {
        struct ModuleAgros;
    }
}

class Field
{
public:
    Field(FieldInfo* fieldInfo);
    bool solveInitVariables();
    FieldInfo* fieldInfo() { return m_fieldInfo; }

    // mesh
    void setMeshInitial(Hermes::Hermes2D::Mesh *meshInitial);

public:
//private:
    QList<CouplingInfo* > m_couplingSources;
    FieldInfo* m_fieldInfo;

};

/// represents one or more fields, that are hard-coupled -> produce only 1 weak form
class Block
{
public:
    Block(QList<FieldInfo*> fieldInfos, QList<CouplingInfo*> couplings, ProgressItemSolve* progressItemSolve, Problem* parent);

    Solver<double>* prepareSolver();

    int numSolutions() const;
    int offset(Field* field) const;

    LinearityType linearityType() const;
    bool isTransient() const;

    AdaptivityType adaptivityType() const;
    int adaptivitySteps() const;
    double adaptivityTolerance() const;

    // minimal nonlinear tolerance of individual fields
    double nonlinearTolerance() const;

    //maximal nonlinear steps of individual fields
    int nonlinearSteps() const;

    double timeStep() const;
    int numTimeSteps() const;

//    Field* couplingSourceField(Coupling* coupling) const;
//    Field* couplingTargetField(Coupling* coupling) const;

    bool contains(FieldInfo* fieldInfo) const;
    Field* field(FieldInfo* fieldInfo) const;
    inline Problem* parentProblem() const {return m_parentProblem; }

public:
//private:
    Problem* m_parentProblem;

    WeakFormAgros<double> *m_wf;

    QList<Field*> m_fields;
    QList<CouplingInfo*> m_couplings;
    ProgressItemSolve* m_progressItemSolve;
};

ostream& operator<<(ostream& output, const Block& id);


class ProblemConfig : public QObject
{
    Q_OBJECT
public:
    ProblemConfig(QWidget *parent = 0);

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
    ProblemConfig *m_parent;

    /// unique field info
    QString m_fieldId;

    // analysis type
    AnalysisType m_analysisType;
};

ostream& operator<<(ostream& output, FieldInfo& id);

/// intented as central for solution process
/// shielded from gui and QT
/// holds data describing individual fields, means of coupling and solutions
class Problem : public QObject
{
    Q_OBJECT

signals:
    void timeStepChanged();
    void meshed();
    void solved();

    /// emited when an field is added or removed. Menus need to adjusted
    void fieldsChanged();

public slots:
    // clear problem
    void clearSolution();
    void clearFieldsAndConfig();


public:
    Problem();
    ~Problem();

    inline ProblemConfig *config() { return m_config; }

    void createStructure();

    // mesh
    bool mesh();
    // solve
    void solve();
    void solveAdaptiveStep();

    // check geometry
    bool checkGeometry();

    inline Hermes::Hermes2D::Mesh *meshInitial(FieldInfo* fieldInfo) { return m_meshesInitial[fieldInfo]; }
    Hermes::Hermes2D::Mesh *activeMeshInitial();
    inline QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> meshesInitial() { return m_meshesInitial; }
    inline void setMeshesInitial(QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> meshes) { m_meshesInitial = meshes; }

    bool isSolved() const {  return m_isSolved; }
    bool isMeshed()  const {  return !m_meshesInitial.isEmpty(); }
    bool isSolving() const { return m_isSolving; }

    inline QMap<QString, FieldInfo *> fieldInfos() const { return m_fieldInfos; }
    inline FieldInfo *fieldInfo(QString name) { assert(m_fieldInfos.contains(name)); return m_fieldInfos[name]; }
    inline FieldInfo *fieldInfo(std::string name) { return fieldInfo(QString::fromStdString(name)); }
    inline FieldInfo *fieldInfo(const char* name) { return fieldInfo(QString::fromAscii(name)); }

    bool hasField(QString field) { return m_fieldInfos.contains(field); }
    void addField(FieldInfo *field);
    void removeField(FieldInfo *field);

    // FIXME: MOVE to couplings ???
    void synchronizeCouplings();
    inline QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > couplingInfos() const { return m_couplingInfos; }
    inline CouplingInfo* couplingInfo(FieldInfo* sourceField, FieldInfo* targetField);
    inline void setCouplingInfos(QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > couplingInfos) { m_couplingInfos = couplingInfos; }

//    inline int timeElapsed() const { return m_timeElapsed; }
//    double adaptiveError() const { return 0; }
//    int adaptiveSteps() const { return 0; }
//    inline void setTimeElapsed(int timeElapsed) { m_timeElapsed = timeElapsed; }

public:
//private:
    ProblemConfig *m_config;
    QList<Block*> m_blocks;

    QMap<QString, FieldInfo *> m_fieldInfos;
    QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > m_couplingInfos;

    // progress dialog
    ProgressDialog *m_progressDialog;
    MeshGeneratorTriangle *m_progressItemMesh;
    ProgressItemSolve *m_progressItemSolve;
    ProgressItemSolveAdaptiveStep *m_progressItemSolveAdaptiveStep;
    ProgressItemProcessView *m_progressItemProcessView;

    int m_timeElapsed;
    bool m_isSolving;
    int m_timeStep;
    bool m_isSolved;

    // todo: move to Field
    QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> m_meshesInitial; // linearizer only for mesh (on empty solution)
};

class SolutionStore
{
public:
    SolutionArray<double> solution(FieldSolutionID solutionID, int component);
    bool contains(FieldSolutionID solutionID);
    MultiSolutionArray<double> multiSolution(FieldSolutionID solutionID);
    MultiSolutionArray<double> multiSolution(BlockSolutionID solutionID);
    void saveSolution(FieldSolutionID solutionID, MultiSolutionArray<double> multiSolution);
    void saveSolution(BlockSolutionID solutionID, MultiSolutionArray<double> multiSolution);
    void replaceSolution(FieldSolutionID solutionID, MultiSolutionArray<double> multiSolution);
    void replaceSolution(BlockSolutionID solutionID, MultiSolutionArray<double> multiSolution);
    void removeSolution(FieldSolutionID solutionID);
    void removeSolution(BlockSolutionID solutionID);

    int lastTimeStep(FieldInfo* fieldInfo, SolutionType solutionType);
    int lastTimeStep(Block* block, SolutionType solutionType);

    double lastTime(FieldInfo* fieldInfo);
    double lastTime(Block* block);

    // last adaptive step for given time step. If time step not given, last time step used implicitly
    int lastAdaptiveStep(FieldInfo* fieldInfo, SolutionType solutionType, int timeStep = -1);
    int lastAdaptiveStep(Block* block, SolutionType solutionType, int timeStep = -1);

    QList<double> timeLevels(FieldInfo* fieldInfo);

    FieldSolutionID lastTimeAndAdaptiveSolution(FieldInfo* fieldInfo, SolutionType solutionType);
    BlockSolutionID lastTimeAndAdaptiveSolution(Block* block, SolutionType solutionType);

    void clearAll();
    void clearOne(FieldSolutionID solutionID);

private:
    QMap<FieldSolutionID, MultiSolutionArray<double> > m_multiSolutions;
};

#endif // PROBLEM_H
