#ifndef PROBLEM_H
#define PROBLEM_H

#include "util.h"
#include "value.h"
#include "solutiontypes.h"

class FieldInfo;
class CouplingInfo;

class Field;
class Problem;
class ProblemConfig;
class ProblemSetting;
class PyProblem;

class CalculationThread : public QThread
{
   Q_OBJECT

public:
    CalculationThread(bool adaptiveStep, bool commandLine);

//public slots:
//    void stopRunning();

protected:
   virtual void run();

signals:
   void signalValueUpdated(QString);

private:
    bool isRunning;
    bool adaptiveStep;
    bool commandLine;
};


/// intented as central for solution process
/// shielded from gui and QT
/// holds data describing individual fields, means of coupling and solutions
class AGROS_API Problem : public QObject
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
    void doAbortSolve();

public:
    Problem();
    ~Problem();

    inline ProblemConfig *config() const { return m_config; }
    inline ProblemSetting *setting() const { return m_setting; }

    void createStructure();

    // mesh
    bool mesh();
    // solve
    void solve();
    void solveCommandLine();
    void solveAdaptiveStep();

    // check geometry
    bool checkGeometry();

    bool isSolved() const {  return m_isSolved; }
    bool isMeshed() const;
    bool isSolving() const { return m_isSolving; }
    bool isAborted() const { return m_abortSolve; }

    bool isTransient() const;
    int numTransientFields() const;
    bool isHarmonic() const;
    inline bool isNonlinear() const { return m_isNonlinear; }
    bool determineIsNonlinear() const;
    int numAdaptiveFields() const;

    inline QMap<QString, FieldInfo *> fieldInfos() const { return m_fieldInfos; }
    inline FieldInfo *fieldInfo(const QString &fieldId) { assert(m_fieldInfos.contains(fieldId));
                                                          return m_fieldInfos[fieldId]; }
    inline FieldInfo *fieldInfo(const std::string &name) { return fieldInfo(QString::fromStdString(name)); }
    inline FieldInfo *fieldInfo(const char* name) { return fieldInfo(QString::fromLatin1(name)); }
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
    double timeStepToTotalTime(int timeStepIndex) const;
    int timeToTimeStep(double time) const;

    int actualTimeStep() {return m_timeStepLengths.size(); }

    // terminology: time levels are actual times, whre calculations are performed
    int numTimeLevels() {return m_timeStepLengths.size() + 1; }

    // sets next time step length. If it would mean exceeding total time, smaller time step is used instead
    // to fit the desired total time period. If we are allready at the end of the interval, returns false. True otherwise (to continue)
    bool defineActualTimeStepLength(double ts);
    void refuseLastTimeStepLength();

    // read initial meshes and solution
    void readInitialMeshesFromFile();
    void readSolutionsFromFile();

    QList<QPair<double, bool> > timeStepHistory() const { return m_timeHistory; }

private:
    ProblemConfig *m_config;
    ProblemSetting *m_setting;

    QList<Block*> m_blocks;

    QMap<QString, FieldInfo *> m_fieldInfos;
    QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > m_couplingInfos;

    QTime m_lastTimeElapsed;

    bool m_isSolving;
    bool m_isSolved;

    bool m_abortSolve;

    // determined in create structure to speed up the calculation
    bool m_isNonlinear;

    QList<double> m_timeStepLengths;
    QList<QPair<double, bool> > m_timeHistory;

    bool skipThisTimeStep(Block* block);

    bool meshAction();
    void solveInit(bool reCreateStructure = true);
    void solve(bool adaptiveStepOnly, bool commandLine);
    void solveAction(); // called by solve, can throw SolverException

    void solveAdaptiveStepAction();
    void stepMessage(Block* block);
    friend class CalculationThread;
    friend class PyProblem;
};

#endif // PROBLEM_H
