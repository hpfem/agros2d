#ifndef PROBLEM_H
#define PROBLEM_H

#include "util.h"
#include "solutiontypes.h"

//template <typename Scalar>
//class WeakFormAgros;

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
    void setFileName(const QString &fileName) { m_fileName = fileName; emit changed(); }

    inline CoordinateType coordinateType() const { return m_coordinateType; }
    void setCoordinateType(const CoordinateType coordinateType) { m_coordinateType = coordinateType; emit changed(); }

    inline double frequency() const { return m_frequency; }
    void setFrequency(const double frequency) { m_frequency = frequency; emit changed(); }

    inline Value timeStep() const { return m_timeStep; }
    void setTimeStep(const Value &timeStep) { m_timeStep = timeStep; emit changed(); }

    inline Value timeTotal() const { return m_timeTotal; }
    void setTimeTotal(const Value &timeTotal) { m_timeTotal = timeTotal; emit changed(); }

    inline int numTimeSteps() const { return floor(timeTotal().number() / timeStep().number()); }

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

    QAction *actClearSolutions;

    inline ProblemConfig *config() { return m_config; }

    void createStructure();

    // mesh
    bool mesh();
    // solve
    void solve();
    void solveAction(); //called by previous, can throw SolverException
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

    void synchronizeCouplings();
    inline QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > couplingInfos() const { return m_couplingInfos; }
    inline CouplingInfo* couplingInfo(FieldInfo* sourceField, FieldInfo* targetField) { assert (m_couplingInfos.contains(QPair<FieldInfo*, FieldInfo* >(sourceField, targetField)));
                                                                                        return m_couplingInfos[QPair<FieldInfo*, FieldInfo* >(sourceField, targetField)]; }
    inline CouplingInfo* couplingInfo(const QString &sourceFieldId, const QString &targetFieldId) { return couplingInfo(fieldInfo(sourceFieldId), fieldInfo(targetFieldId)); }
    inline bool hasCoupling(FieldInfo* sourceField, FieldInfo* targetField) { return (m_couplingInfos.contains(QPair<FieldInfo*, FieldInfo* >(sourceField, targetField))); }
    inline bool hasCoupling(const QString &sourceFieldId, const QString &targetFieldId) { return hasCoupling(fieldInfo(sourceFieldId), fieldInfo(targetFieldId)); }
    inline void setCouplingInfos(QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > couplingInfos) { m_couplingInfos = couplingInfos; }

    inline QTime timeElapsed() const { return m_timeElapsed; }

    double actualTime() const { return m_actualTime; }
    void setActualTime(double time) { m_actualTime = time; }
    void addToActualTime(double time) { m_actualTime += time; }

private:
    ProblemConfig *m_config;
    QList<Block*> m_blocks;

    QMap<QString, FieldInfo *> m_fieldInfos;
    QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > m_couplingInfos;

    QTime m_timeElapsed;
    bool m_isSolving;
    int m_timeStep;
    bool m_isSolved;

    // todo: move to Field
    QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> m_meshesInitial; // linearizer only for mesh (on empty solution)

    void solveInit();
    double m_actualTime;
};

#endif // PROBLEM_H
