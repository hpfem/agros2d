#ifndef PROBLEM_H
#define PROBLEM_H

#include "util.h"
#include "value.h"
#include "solutiontypes.h"

class FieldInfo;
class CouplingInfo;

class Field;

class Problem;

//template <typename Scalar>
//class Solver;

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
    void setFileName(const QString &fileName) { m_fileName = fileName; }

    // coordinates
    inline CoordinateType coordinateType() const { return m_coordinateType; }
    void setCoordinateType(const CoordinateType coordinateType) { m_coordinateType = coordinateType; emit changed(); }

    // harmonic problem
    inline double frequency() const { return m_frequency; }
    void setFrequency(const double frequency) { m_frequency = frequency; emit changed(); }

    // transient problem
    inline int numConstantTimeSteps() const { return m_numConstantTimeSteps; }
    void setNumConstantTimeSteps(const int numConstantTimeSteps) { m_numConstantTimeSteps = numConstantTimeSteps; emit changed(); }

    inline Value timeTotal() const { return m_timeTotal; }
    void setTimeTotal(const Value &timeTotal) { m_timeTotal = timeTotal; emit changed(); }

    inline double constantTimeStepLength() { return m_timeTotal.number() / m_numConstantTimeSteps; }
    double initialTimeStepLength();

    inline TimeStepMethod timeStepMethod() const {return m_timeStepMethod; }
    void setTimeStepMethod(TimeStepMethod timeStepMethod) { m_timeStepMethod = timeStepMethod; }
    bool isTransientAdaptive() const;

    int timeOrder() const { return m_timeOrder; }
    void setTimeOrder(int timeOrder) {m_timeOrder = timeOrder; }

    inline Value timeMethodTolerance() const { return m_timeMethodTolerance; }
    void setTimeMethodTolerance(Value timeMethodTolerance) {m_timeMethodTolerance = timeMethodTolerance; }

    // matrix
    inline Hermes::MatrixSolverType matrixSolver() const { return m_matrixSolver; }
    void setMatrixSolver(const Hermes::MatrixSolverType matrixSolver) { m_matrixSolver = matrixSolver; emit changed(); }

    // mesh
    inline MeshType meshType() const { return m_meshType; }
    void setMeshType(const MeshType meshType) { m_meshType = meshType; emit changed(); }

    // startup script
    inline QString startupscript() const { return m_startupscript; }
    void setStartupScript(const QString &startupscript) { m_startupscript = startupscript; emit changed(); }

    // description
    inline QString description() const { return m_description; }
    void setDescription(const QString &description) { m_description = description; }

    void refresh() { emit changed(); }

signals:
    void changed();

private:    QString m_name;
    QString m_fileName;
    CoordinateType m_coordinateType;

    // harmonic
    double m_frequency;

    // transient
    Value m_timeTotal;
    int m_numConstantTimeSteps;
    TimeStepMethod m_timeStepMethod;
    int m_timeOrder;
    Value m_timeMethodTolerance;

    // matrix solver
    Hermes::MatrixSolverType m_matrixSolver;

    // mesh type
    MeshType m_meshType;

    QString m_startupscript;
    QString m_description;

};

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

    /// emited when an field is added or removed. Menus need to adjusted
    void couplingsChanged();

public slots:
    // clear problem
    void clearSolution();
    void clearFieldsAndConfig();

public:
    Problem();
    ~Problem();

    QAction *actClearSolutions;

    inline ProblemConfig *config() const { return m_config; }

    void createStructure();

    // mesh
    bool mesh();
    // solve
    void solve();
    void solveAdaptiveStep();

    // check geometry
    bool checkGeometry();

    bool isSolved() const {  return m_isSolved; }
    bool isMeshed() const;
    bool isSolving() const { return m_isSolving; }

    bool isTransient() const;
    bool isHarmonic() const;
    bool isNonlinear() const;

    inline QMap<QString, FieldInfo *> fieldInfos() const { return m_fieldInfos; }
    inline FieldInfo *fieldInfo(const QString &fieldId) { assert(m_fieldInfos.contains(fieldId));
                                                          return m_fieldInfos[fieldId]; }
    inline FieldInfo *fieldInfo(const std::string &name) { return fieldInfo(QString::fromStdString(name)); }
    inline FieldInfo *fieldInfo(const char* name) { return fieldInfo(QString::fromAscii(name)); }
    inline bool hasField(const QString &fieldId) { return m_fieldInfos.contains(fieldId); }
    void addField(FieldInfo *field);
    void removeField(FieldInfo *field);

    Block* blockOfField(FieldInfo* fieldInfo) const;

    void synchronizeCouplings();
    inline QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > couplingInfos() const { return m_couplingInfos; }
    inline CouplingInfo* couplingInfo(FieldInfo* sourceField, FieldInfo* targetField) { assert (m_couplingInfos.contains(QPair<FieldInfo*, FieldInfo* >(sourceField, targetField)));
                                                                                        return m_couplingInfos[QPair<FieldInfo*, FieldInfo* >(sourceField, targetField)]; }
    inline CouplingInfo* couplingInfo(const QString &sourceFieldId, const QString &targetFieldId) { return couplingInfo(fieldInfo(sourceFieldId), fieldInfo(targetFieldId)); }
    inline bool hasCoupling(FieldInfo* sourceField, FieldInfo* targetField) { return (m_couplingInfos.contains(QPair<FieldInfo*, FieldInfo* >(sourceField, targetField))); }
    inline bool hasCoupling(const QString &sourceFieldId, const QString &targetFieldId) { return hasCoupling(fieldInfo(sourceFieldId), fieldInfo(targetFieldId)); }
    inline void setCouplingInfos(QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > couplingInfos) { m_couplingInfos = couplingInfos; emit couplingsChanged(); }

    inline QTime timeElapsed() const { return m_lastTimeElapsed; }

    double actualTime() const;
    double actualTimeStepLength() const;
    QList<double> timeStepLengths() const { return m_timeStepLengths; }
    double timeStepToTime(int timeStepIndex) const;
    int timeToTimeStep(double time) const;

    int actualTimeStep() {return m_timeStepLengths.size(); }

    // terminlolgy: time levels are actual times, whre calculations are performed
    int numTimeLevels() {return m_timeStepLengths.size() + 1; }

    // sets next time step lenght. If it would mean exceeding total time, smaller time step is used instead
    // to fit the desired total time period. If we are allready at the end of the interval, returns false. True otherwise (to continue)
    bool defineActualTimeStepLength(double ts);
    void refuseLastTimeStepLength();

private:
    ProblemConfig *m_config;
    QList<Block*> m_blocks;

    QMap<QString, FieldInfo *> m_fieldInfos;
    QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > m_couplingInfos;

    QTime m_lastTimeElapsed;

    bool m_isSolving;
    int m_timeStep;
    bool m_isSolved;

    QList<double> m_timeStepLengths;

    bool skipThisTimeStep(Block* block);

    void solveInit();
    void solveActionCatchExceptions(bool adaptiveStepOnly); //calls one of following, catches exceptions
    void solveAction(); //called by solve, can throw SolverException
    void solveFinished();

    void solveAdaptiveStepAction();
    void Problem::stepMessage(Block* block);

    // read initial meshes
    void readInitialMeshesFromFile();
};

#endif // PROBLEM_H
