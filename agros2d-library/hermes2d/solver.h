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

class Block;
class FieldInfo;

template <typename Scalar>
class ExactSolutionScalarAgros;

class SceneBoundary;

namespace Module {
    class ErrorCalculator;
}

class SolverAgros
{
public:
    SolverAgros(Block *block) : m_block(block) {}

    enum Phase
    {
        Phase_Init,
        Phase_DFDetermined,
        Phase_JacobianReused,
        Phase_Finished
    };

    inline QVector<double> steps() const { return m_steps; }
    inline QVector<double> damping() const { return m_damping; }
    inline QVector<double> residualNorms() const { return m_residualNorms; }
    inline QVector<double> solutionNorms() const { return m_solutionNorms; }

    void clearSteps();

protected:
    Block* m_block;

    virtual void setError(Phase phase) = 0;

    QVector<double> m_steps;
    QVector<double> m_damping;
    QVector<double> m_residualNorms;
    QVector<double> m_solutionNorms;
};

class AgrosExternalSolverOctave : public ExternalSolver<double>
{
public:
    AgrosExternalSolverOctave(CSCMatrix<double> *m, SimpleVector<double> *rhs);
    void solve();
    void solve(double* initial_guess);

private:
    QProcess *m_process;
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
                                 Hermes::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > spaces,
                                 Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<Scalar> > solutions);

    virtual void solve(Scalar* previousSolutionVector) = 0;
    virtual Hermes::Hermes2D::Mixins::SettableSpaces<Scalar>* setTableSpaces() = 0;
    virtual void setWeakFormulation(Hermes::Hermes2D::WeakForm<Scalar>* wf) = 0;

    virtual void setMatrixRhsOutput(QString solverName, int adaptivityStep) = 0;
    void setMatrixRhsOutputGen(Hermes::Algebra::Mixins::MatrixRhsOutput<Scalar>* solver, QString solverName, int adaptivityStep);

    virtual void matrixUnchangedDueToBDF(bool unchanged) {}
    virtual Hermes::Algebra::LinearMatrixSolver<Scalar> *linearSolver() = 0;

    inline Scalar *slnVector() { return m_slnVector; }

    // solver factory
    static QSharedPointer<HermesSolverContainer<Scalar> > factory(Block* block);

protected:
    Block* m_block;
    Scalar *m_slnVector;

    bool m_constJacobianPossible;
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
    bool createAdaptedSpace(int timeStep, int adaptivityStep, bool forceAdaptation = false);

    // to be used in solveAdaptivityStep
    void resumeAdaptivityProcess(int adaptivityStep);

private:
    Block* m_block;

    QSharedPointer<HermesSolverContainer<Scalar> > m_hermesSolverContainer;

    Hermes::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > m_actualSpaces;

    QString m_solverID;
    QString m_solverName;
    QString m_solverCode;

    // elapsed time
    double m_elapsedTime;

    // to be used in advanced time step adaptivity
    double m_averageErrorToLenghtRatio;

    void initSelectors(Hermes::vector<QSharedPointer<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> > >& selectors);

    Scalar *solveOneProblem(Hermes::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > spaces, int adaptivityStep,
                            Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<Scalar> > previousSolution = Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<Scalar> >());

    void clearActualSpaces();
    void setActualSpaces(Hermes::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > spaces);
    Hermes::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > actualSpaces() { return m_actualSpaces;}
    Hermes::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > deepMeshAndSpaceCopy(Hermes::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > spaces, bool createReference);
};

#endif // SOLVER_H
