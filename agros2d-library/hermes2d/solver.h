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

#ifndef SOLVER_H
#define SOLVER_H

#include "util.h"
#include "solutiontypes.h"

#undef signals
#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/dofs/dof_tools.h>
#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#define signals public

class Block;
class FieldInfo;

template <typename Scalar>
class ExactSolutionScalarAgros;

class SceneBoundary;

class SolverDeal
{
public:
    SolverDeal(const FieldInfo *fieldInfo, int initialOrder = 2);
    ~SolverDeal();

    inline dealii::Vector<double> *solution() { return m_solution; }
    inline dealii::DoFHandler<2> *doFHandler() { return m_doFHandler; }
    inline dealii::Triangulation<2> *triangulation() { return m_triangulation; }

    virtual void setup();

    virtual void assembleSystem();
    virtual void assembleDirichlet() = 0;

    virtual void solve();

protected:
    const FieldInfo *m_fieldInfo;

    dealii::Triangulation<2> *m_triangulation;
    dealii::DoFHandler<2> *m_doFHandler;
    dealii::FESystem<2> *m_fe;
    dealii::Vector<double> *m_solution;

    dealii::ConstraintMatrix hanging_node_constraints;
    dealii::SparsityPattern sparsity_pattern;

    dealii::SparseMatrix<double> system_matrix;
    dealii::Vector<double> system_rhs;

    void solveUMFPACK();
    void solveCG();
};

namespace Module {
    class ErrorCalculator;
}

class SolverAgros
{
public:
    SolverAgros(Block *block) : m_block(block), m_jacobianCalculations(0), m_phase(Phase_Undefined) {}

    enum Phase
    {
        Phase_Undefined,
        Phase_Init,
        Phase_DFDetermined,
        Phase_JacobianReused,
        Phase_Finished
    };

    inline QVector<double> steps() const { return m_steps; }
    inline QVector<double> damping() const { return m_damping; }
    inline QVector<double> residualNorms() const { return m_residualNorms; }
    inline QVector<double> solutionNorms() const { return m_solutionNorms; }
    inline QVector<double> relativeChangeOfSolutions() const { return m_relativeChangeOfSolutions; }
    inline int jacobianCalculations() const { return m_jacobianCalculations; }

    inline Phase phase() const { return m_phase; }

    void clearSteps();

protected:
    Block* m_block;
    Phase m_phase;

    virtual void setError() = 0;

    QVector<double> m_steps;
    QVector<double> m_damping;
    QVector<double> m_residualNorms;
    QVector<double> m_solutionNorms;
    QVector<double> m_relativeChangeOfSolutions;
    int m_jacobianCalculations;
};

class AgrosExternalSolverExternal : public QObject, public ExternalSolver<double>
{
    Q_OBJECT

public:
    AgrosExternalSolverExternal(CSCMatrix<double> *m, SimpleVector<double> *rhs);
    void solve();
    void solve(double* initial_guess);

    virtual void setSolverCommand() = 0;

protected:
    QProcess *m_process;

    QString command;

    QString fileMatrix;
    QString fileRHS;
    QString fileInitial;
    QString fileSln;

    double *initialGuess;

protected slots:
    void processError(QProcess::ProcessError error);
    void processFinished(int exitCode);
};

class AgrosExternalSolverMUMPS : public AgrosExternalSolverExternal
{
public:
    AgrosExternalSolverMUMPS(CSCMatrix<double> *m, SimpleVector<double> *rhs);

    virtual void setSolverCommand();
    virtual void free();
};

class AgrosExternalSolverUMFPack : public AgrosExternalSolverExternal
{
public:
    AgrosExternalSolverUMFPack(CSCMatrix<double> *m, SimpleVector<double> *rhs);

    virtual void setSolverCommand();
    virtual void free();
};

struct TimeStepInfo
{
    TimeStepInfo(double len, bool ref = false) : length(len), refuse(ref) {}
    double length;
    bool refuse;
};

template <typename Scalar>
class HermesSolverContainer
{
public:
    HermesSolverContainer(Block* block) : m_block(block), m_slnVector(NULL), m_constJacobianPossible(false) {}
    virtual ~HermesSolverContainer() {}

    void projectPreviousSolution(Scalar* solutionVector,
                                 std::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > spaces,
                                 std::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<Scalar> > solutions);

    virtual void solve(Scalar* previousSolutionVector) = 0;
    virtual Hermes::Hermes2D::Mixins::SettableSpaces<Scalar>* setTableSpaces() = 0;
    virtual void setWeakFormulation(Hermes::Hermes2D::WeakFormSharedPtr<Scalar> wf) = 0;

    virtual void setMatrixRhsOutput(QString solverName, int adaptivityStep) = 0;
    void setMatrixRhsOutputGen(Hermes::Algebra::Mixins::MatrixRhsOutput<Scalar>* solver, QString solverName, int adaptivityStep);

    virtual void matrixUnchangedDueToBDF(bool unchanged) {}
    virtual Hermes::Algebra::LinearMatrixSolver<Scalar> *linearSolver() = 0;

    inline Scalar *slnVector() { return m_slnVector; }
    virtual SolverAgros *solver() const = 0;

    // solver factory
    static QSharedPointer<HermesSolverContainer<Scalar> > factory(Block* block);

protected:
    Block* m_block;
    Scalar *m_slnVector;

    bool m_constJacobianPossible;
};

// solve
class ProblemSolverDeal
{
public:
    ProblemSolverDeal();

    void init();

    void solveSimple(int timeStep, int adaptivityStep);

private:
    SolverDeal *m_solverDeal;
};

// solve
template <typename Scalar>
class ProblemSolver
{
public:
    ProblemSolver() : m_hermesSolverContainer(NULL) {}
    ~ProblemSolver();

    void init(Block* block);

    void createInitialSpace();
    void solveInitialTimeStep();

    // returns the value of the next time step lenght (for transient problems), using BDF2 approximation
    TimeStepInfo estimateTimeStepLength(int timeStep, int adaptivityStep);

    void solveSimple(int timeStep, int adaptivityStep);
    void solveReferenceAndProject(int timeStep, int adaptivityStep);
    bool createAdaptedSpace(int timeStep, int adaptivityStep);

    // to be used in solveAdaptivityStep
    void resumeAdaptivityProcess(int adaptivityStep);

private:
    Block* m_block;

    QSharedPointer<HermesSolverContainer<Scalar> > m_hermesSolverContainer;

    std::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > m_actualSpaces;

    QString m_solverID;
    QString m_solverName;
    QString m_solverCode;

    // elapsed time
    double m_elapsedTime;

    // to be used in advanced time step adaptivity
    double m_averageErrorToLenghtRatio;

    void initSelectors(std::vector<QSharedPointer<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> > >& selectors);

    Scalar *solveOneProblem(std::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > spaces, int adaptivityStep, std::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<Scalar> > previousSolution = std::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<Scalar> >());

    void clearActualSpaces();
    void setActualSpaces(std::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > spaces);
    std::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > actualSpaces() { return m_actualSpaces;}
    std::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > deepMeshAndSpaceCopy(std::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > spaces, bool createReference);
};

#endif // SOLVER_H
