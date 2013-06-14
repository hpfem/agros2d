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

enum Phase  {
    Phase_Init,
    Phase_DFDetermined,
    Phase_JacobianReused,
    Phase_Finished
};

template <typename Scalar>
class NewtonSolverAgros : public Hermes::Hermes2D::NewtonSolver<Scalar>
{
public:
    NewtonSolverAgros(Block *block);

    virtual bool on_initialization();
    virtual bool on_initial_step_end();
    virtual bool on_step_begin();
    virtual bool on_step_end();
    virtual bool on_finish();
    virtual void on_damping_factor_updated();
    virtual void on_reused_jacobian_step_begin();

    void clearSteps();

    inline QVector<double> steps() const { return m_steps; }
    inline QVector<double> damping() const { return m_damping; }
    inline QVector<double> errors() const { return m_errors; }

protected:
    Block* m_block;

    QVector<double> m_steps;
    QVector<double> m_damping;
    QVector<double> m_errors;

    void setError(Phase phase);
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
                                 Hermes::vector<SpaceSharedPtr<Scalar> > spaces,
                                 Hermes::vector<MeshFunctionSharedPtr<Scalar> > solutions);

    virtual void solve(Scalar* previousSolutionVector) = 0;
    virtual Hermes::Hermes2D::Mixins::SettableSpaces<Scalar>* setTableSpaces() = 0;
    virtual void setWeakFormulation(Hermes::Hermes2D::WeakForm<Scalar>* wf) = 0;

    virtual void setMatrixRhsOutput(QString solverName, int adaptivityStep) = 0;
    void setMatrixRhsOutputGen(Hermes::Hermes2D::Mixins::MatrixRhsOutput<Scalar>* solver, QString solverName, int adaptivityStep);

    virtual void matrixUnchangedDueToBDF(bool unchanged) {}
    virtual LinearMatrixSolver<Scalar> *linearSolver() = 0;

    inline Scalar *slnVector() { return m_slnVector; }

    // solver factory
    static HermesSolverContainer<Scalar> *factory(Block* block);

protected:
    Block* m_block;
    Scalar *m_slnVector;

    bool m_constJacobianPossible;
};

template <typename Scalar>
class LinearSolverContainer : public HermesSolverContainer<Scalar>
{
public:
    LinearSolverContainer(Block* block);
    ~LinearSolverContainer();

    void solve(Scalar* previousSolutionVector);
    virtual void setMatrixRhsOutput(QString solverName, int adaptivityStep) { this->setMatrixRhsOutputGen(m_linearSolver, solverName, adaptivityStep); }
    virtual Hermes::Hermes2D::Mixins::SettableSpaces<Scalar>* setTableSpaces() { return m_linearSolver; }
    virtual void setWeakFormulation(Hermes::Hermes2D::WeakForm<Scalar>* wf) {m_linearSolver->set_weak_formulation(wf); }
    virtual void matrixUnchangedDueToBDF(bool unchanged);
    virtual LinearMatrixSolver<Scalar> *linearSolver() { return m_linearSolver->get_linear_solver(); }

private:
    Hermes::Hermes2D::LinearSolver<Scalar> *m_linearSolver;
};

template <typename Scalar>
class NewtonSolverContainer : public HermesSolverContainer<Scalar>
{
public:
    NewtonSolverContainer(Block* block);
    ~NewtonSolverContainer();

    virtual void solve(Scalar* previousSolutionVector);
    virtual void setMatrixRhsOutput(QString solverName, int adaptivityStep) { this->setMatrixRhsOutputGen(m_newtonSolver, solverName, adaptivityStep); }
    virtual Hermes::Hermes2D::Mixins::SettableSpaces<Scalar>* setTableSpaces() { return m_newtonSolver; }
    virtual void setWeakFormulation(Hermes::Hermes2D::WeakForm<Scalar>* wf) { m_newtonSolver->set_weak_formulation(wf); }
    virtual LinearMatrixSolver<Scalar> *linearSolver() { return m_newtonSolver->get_linear_solver(); }

    NewtonSolverAgros<Scalar> *solver() const { return m_newtonSolver; }

private:
    NewtonSolverAgros<Scalar> *m_newtonSolver;
};

template <typename Scalar>
class PicardSolverContainer : public HermesSolverContainer<Scalar>
{
public:
    PicardSolverContainer(Block* block);
    ~PicardSolverContainer();

    virtual void solve(Scalar* previousSolutionVector);
    virtual void setMatrixRhsOutput(QString solverName, int adaptivityStep) { this->setMatrixRhsOutputGen(m_picardSolver, solverName, adaptivityStep); }
    virtual Hermes::Hermes2D::Mixins::SettableSpaces<Scalar>* setTableSpaces() { return m_picardSolver; }
    virtual void setWeakFormulation(Hermes::Hermes2D::WeakForm<Scalar>* wf) { m_picardSolver->set_weak_formulation(wf); }
    virtual LinearMatrixSolver<Scalar> *linearSolver() { return m_picardSolver->get_linear_solver(); }

private:
    Hermes::Hermes2D::PicardSolver<Scalar> *m_picardSolver;
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

    HermesSolverContainer<Scalar> *m_hermesSolverContainer;

    Hermes::vector<SpaceSharedPtr<Scalar> > m_actualSpaces;

    QString m_solverID;
    QString m_solverName;
    QString m_solverCode;

    // elapsed time
    double m_elapsedTime;

    // to be used in advanced time step adaptivity
    double m_averageErrorToLenghtRatio;

    void initSelectors(Hermes::vector<Hermes::Hermes2D::NormType>& projNormType,
                       Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *>& selectors);
    void deleteSelectors(Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *>& selectors);

    Scalar *solveOneProblem(Hermes::vector<SpaceSharedPtr<Scalar> > spaces, int adaptivityStep, Hermes::vector<MeshFunctionSharedPtr<Scalar> > previousSolution = Hermes::vector<MeshFunctionSharedPtr<Scalar> >());

    void clearActualSpaces();
    void setActualSpaces(Hermes::vector<SpaceSharedPtr<Scalar> > spaces);
    Hermes::vector<SpaceSharedPtr<Scalar> > actualSpaces() { return m_actualSpaces;}
    Hermes::vector<SpaceSharedPtr<Scalar> > deepMeshAndSpaceCopy(Hermes::vector<SpaceSharedPtr<Scalar> > spaces, bool createReference);
};

#endif // SOLVER_H
