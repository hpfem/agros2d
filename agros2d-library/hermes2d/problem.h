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
    enum CalculationType
    {
        CalculationType_Mesh,
        CalculationType_Solve,
        CalculationType_SolveAdaptiveStep,
        CalculationType_SolveTimeStep
    };

    CalculationThread();

    void startCalculation(CalculationType type);

//public slots:
//    void stopRunning();

protected:
   virtual void run();

signals:
   void signalValueUpdated(QString);

private:
    CalculationType m_calculationType;

};


/// intented as central for solution process
/// shielded from gui and QT
/// holds data describing individual fields, means of coupling and solutions
class AGROS_LIBRARY_API Problem : public QObject
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

    void clearedSolution();

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
    void mesh();
    // solve
    void solve();
    void solveAdaptiveStep();

    // check geometry
    bool checkGeometry();

    bool isSolved() const;
    bool isSolving() const { return m_isSolving; }
    bool isMeshed() const;
    bool isMeshing() const { return m_isMeshing; }
    bool isAborted() const { return m_abort; }
    bool isPreparedForAction() const { return !isMeshing() && !isSolving() && !m_isPostprocessingRunning; }

    inline QAction *actionMesh() { return actMesh; }
    inline QAction *actionSolve() { return actSolve; }
    inline QAction *actionSolveAdaptiveStep() { return actSolveAdaptiveStep; }

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

    Block* blockOfField(const FieldInfo* fieldInfo) const;

    void synchronizeCouplings();
    inline QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > couplingInfos() const { return m_couplingInfos; }
    inline CouplingInfo* couplingInfo(FieldInfo* sourceField, FieldInfo* targetField) { assert (m_couplingInfos.contains(QPair<FieldInfo*, FieldInfo* >(sourceField, targetField)));
                                                                                        return m_couplingInfos[QPair<FieldInfo*, FieldInfo* >(sourceField, targetField)]; }
    inline CouplingInfo* couplingInfo(const QString &sourceFieldId, const QString &targetFieldId) { return couplingInfo(fieldInfo(sourceFieldId), fieldInfo(targetFieldId)); }
    inline bool hasCoupling(FieldInfo* sourceField, FieldInfo* targetField) { return (m_couplingInfos.contains(QPair<FieldInfo*, FieldInfo* >(sourceField, targetField))); }
    inline bool hasCoupling(const QString &sourceFieldId, const QString &targetFieldId) { return hasCoupling(fieldInfo(sourceFieldId), fieldInfo(targetFieldId)); }

    inline QTime timeElapsed() const { return m_lastTimeElapsed; }

    double actualTimeStepLength() const;
    QList<double> timeStepLengths() const { return m_timeStepLengths; }
    double timeStepToTime(int timeStepIndex) const;
    double timeStepToTotalTime(int timeStepIndex) const;
    int timeToTimeStep(double time) const;

    int actualTimeStep() {return m_timeStepLengths.size(); }


    // has two meainings. During the calculation it is calculated automaticaly from timeStepLengths
    // during postprocessing it has to be set manualy by the function setActualTimePostprocessing()
    inline double actualTime() const {return m_actualTime;}
    void setActualTimePostprocessing(double time) { m_actualTime = time; }

    // terminology: time levels are actual times, whre calculations are performed
    int numTimeLevels() {return m_timeStepLengths.size() + 1; }

    // sets next time step length. If it would mean exceeding total time, smaller time step is used instead
    // to fit the desired total time period. If we are allready at the end of the interval, returns false. True otherwise (to continue)
    bool defineActualTimeStepLength(double ts);
    void refuseLastTimeStepLength();

    // read initial meshes and solution
    void readInitialMeshesFromFile(bool emitMeshed = true, std::auto_ptr<XMLSubdomains::domain> xmldomain = std::auto_ptr<XMLSubdomains::domain>());
    void readSolutionsFromFile();

    QList<QPair<double, bool> > timeStepHistory() const { return m_timeHistory; }

    QString timeUnit();

    void setIsPostprocessingRunning(bool pr = true) { m_isPostprocessingRunning = pr; }

private:
    ProblemConfig *m_config;
    ProblemSetting *m_setting;

    QList<Block *> m_blocks;

    QMap<QString, FieldInfo *> m_fieldInfos;
    QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > m_couplingInfos;

    QTime m_lastTimeElapsed;

    bool m_isSolving;
    bool m_isMeshing;
    bool m_abort;

    bool m_isPostprocessingRunning;

    CalculationThread *m_calculationThread;

    QAction *actMesh;
    QAction *actSolve;
    QAction *actSolveAdaptiveStep;

    // determined in create structure to speed up the calculation
    bool m_isNonlinear;

    QList<double> m_timeStepLengths;
    double m_actualTime;

    // has to be called allways when m_timeStepLengths are modified during the calculation
    void updateActualTimeDuringCalculation();

    QList<QPair<double, bool> > m_timeHistory;

    bool skipThisTimeStep(Block* block);

    bool mesh(bool emitMeshed);
    bool meshAction(bool emitMeshed);
    void solveInit(bool reCreateStructure = true);
    void solve(bool adaptiveStepOnly, bool commandLine);
    void solveAction(); // called by solve, can throw SolverException

    void solveAdaptiveStepAction();
    void stepMessage(Block* block);

    friend class CalculationThread;
    friend class PyProblem;
    friend class AgrosSolver;

private slots:
    void doMeshWithGUI();
    void doSolveWithGUI();
    void doSolveAdaptiveStepWithGUI();
};

#endif // PROBLEM_H
