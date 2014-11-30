// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

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
    SolverAgros() : m_jacobianCalculations(0), m_phase(Phase_Undefined) {}

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
    Phase m_phase;

    virtual void setError() = 0;

    QVector<double> m_steps;
    QVector<double> m_damping;
    QVector<double> m_residualNorms;
    QVector<double> m_solutionNorms;
    QVector<double> m_relativeChangeOfSolutions;
    int m_jacobianCalculations;
};

/*
class AgrosExternalSolverExternal : public QObject
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
*/

struct TimeStepInfo
{
    TimeStepInfo(double len, bool ref = false) : length(len), refuse(ref) {}
    double length;
    bool refuse;
};

// solve
class ProblemSolverDeal
{
public:
    ProblemSolverDeal();

    void init();

    void solve(int timeStep);
    void solveSimple(FieldInfo *fieldInfo, int timeStep);
    void solveAdaptive(FieldInfo *fieldInfo, int timeStep);

private:
    SolverDeal *m_solverDeal;
};

// solve
template <typename Scalar>
class ProblemSolver
{
public:
    ProblemSolver() {}
    ~ProblemSolver();

    // void init(Block* block);

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
    // std::vector<SpaceSharedPtr<Scalar> > m_actualSpaces;

    QString m_solverID;
    QString m_solverName;
    QString m_solverCode;

    // elapsed time
    double m_elapsedTime;

    // to be used in advanced time step adaptivity
    double m_averageErrorToLenghtRatio;

    // Scalar *solveOneProblem(std::vector<SpaceSharedPtr<Scalar> > spaces, int adaptivityStep, std::vector<MeshFunctionSharedPtr<Scalar> > previousSolution = std::vector<MeshFunctionSharedPtr<Scalar> >());

    // std::vector<SpaceSharedPtr<Scalar> > actualSpaces() { return m_actualSpaces;}
    // std::vector<SpaceSharedPtr<Scalar> > deepMeshAndSpaceCopy(std::vector<SpaceSharedPtr<Scalar> > spaces, bool createReference);
};

#endif // SOLVER_H
