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

/// intented as central for solution process
/// shielded from gui and QT
/// holds data describing individual fields, means of coupling and solutions
class Problem : public QObject
{
    Q_OBJECT

signals:
    void timeStepChanged(bool showViewProgress = true);
    void meshed();
    void solved();

public:
    Problem();
    ~Problem();

    // clear problem
    void clear();

    void createStructure();

    // mesh
    bool mesh();
    // solve
    void solve(SolverMode solverMode);
    // check geometry
    bool checkGeometry();

    inline Hermes::Hermes2D::Mesh *meshInitial(FieldInfo* fieldInfo) { return m_meshesInitial[fieldInfo]; }
    Hermes::Hermes2D::Mesh *activeMeshInitial();
    inline QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> meshesInitial() { return m_meshesInitial; }
    inline void setMeshesInitial(QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> meshes) { m_meshesInitial = meshes; }

    // time TODO zatim tady, ale asi presunout
    //void setTimeStep(int timeStep, bool showViewProgress = true) { assert(0); }
    inline int timeStep() const { return m_timeStep; }
    int timeStepCount() const { return 0; }
    double time() const { return 0; }

    bool isSolved() const {  return m_isSolved; }
    bool isMeshed()  const {  return ! m_meshesInitial.isEmpty(); }
    bool isSolving() const { return m_isSolving; }

//    inline int timeElapsed() const { return m_timeElapsed; }
//    double adaptiveError() const { return 0; }
//    int adaptiveSteps() const { return 0; }
//    inline void setTimeElapsed(int timeElapsed) { m_timeElapsed = timeElapsed; }

public:
//private:
    QList<Block*> m_blocks;

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
