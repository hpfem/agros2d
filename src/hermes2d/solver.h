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

class AgrosSolverException : public AgrosException
{
public:
    AgrosSolverException(const QString &what) : AgrosException(what)
    {
    }
};

struct NextTimeStep
{
    NextTimeStep(double len, bool ref = false) : length(len), refuse(ref) {}
    double length;
    bool refuse;
};


template <typename Scalar>
class HermesSolverContainer
{
public:
    HermesSolverContainer(Block* block) : m_block(block) {}
    virtual void solve(Scalar* solutionVector) = 0;
    virtual void projectPreviousSolution(Scalar* solutionVector, MultiSpace<Scalar> spaces, MultiSolution<Scalar> solutions) {}
    virtual void setMatrixRhsOutput(QString solverName, int adaptivityStep) = 0;
    virtual Hermes::Hermes2D::Mixins::SettableSpaces<Scalar>* settableSpaces() = 0;
    virtual void set_weak_formulation(const Hermes::Hermes2D::WeakForm<Scalar>* wf) = 0;

    void setMatrixRhsOutputGen(Hermes::Hermes2D::Mixins::MatrixRhsOutput<Scalar>* solver, QString solverName, int adaptivityStep);

    // todo: tohle by melo vracet shared pointer
    static HermesSolverContainer* factory(Block* block);

protected:
    Block* m_block;
};

template <typename Scalar>
class LinearSolverContainer : public HermesSolverContainer<Scalar>
{
public:
    LinearSolverContainer(Block* block);
    ~LinearSolverContainer();
    virtual void solve(Scalar* solutionVector);
    virtual void setMatrixRhsOutput(QString solverName, int adaptivityStep) { this->setMatrixRhsOutputGen(m_linearSolver.data(), solverName, adaptivityStep); }
    virtual Hermes::Hermes2D::Mixins::SettableSpaces<Scalar>* settableSpaces() { return m_linearSolver.data(); }
    virtual void set_weak_formulation(const Hermes::Hermes2D::WeakForm<Scalar>* wf) {m_linearSolver->set_weak_formulation(wf);}
;


private:
    QSharedPointer<Hermes::Hermes2D::LinearSolver<Scalar> > m_linearSolver;
};

template <typename Scalar>
class NewtonSolverContainer : public HermesSolverContainer<Scalar>
{
public:
    NewtonSolverContainer(Block* block);
    ~NewtonSolverContainer();
    virtual void projectPreviousSolution(Scalar* solutionVector, MultiSpace<Scalar> spaces, MultiSolution<Scalar> solutions);
    virtual void solve(Scalar* solutionVector);
    virtual void setMatrixRhsOutput(QString solverName, int adaptivityStep) { this->setMatrixRhsOutputGen(m_newtonSolver.data(), solverName, adaptivityStep); }
    virtual Hermes::Hermes2D::Mixins::SettableSpaces<Scalar>* settableSpaces() { return m_newtonSolver.data(); }
    virtual void set_weak_formulation(const Hermes::Hermes2D::WeakForm<Scalar>* wf) {m_newtonSolver->set_weak_formulation(wf);}

private:
    QSharedPointer<Hermes::Hermes2D::NewtonSolver<Scalar> > m_newtonSolver;
};

template <typename Scalar>
class PicardSolverContainer : public HermesSolverContainer<Scalar>
{
public:
    PicardSolverContainer(Block* block);
    ~PicardSolverContainer();
    virtual void projectPreviousSolution(Scalar* solutionVector, MultiSpace<Scalar> spaces, MultiSolution<Scalar> solutions);
    virtual void solve(Scalar* solutionVector);
    virtual void setMatrixRhsOutput(QString solverName, int adaptivityStep) { this->setMatrixRhsOutputGen(m_picardSolver.data(), solverName, adaptivityStep); }
    virtual Hermes::Hermes2D::Mixins::SettableSpaces<Scalar>* settableSpaces() { return m_picardSolver.data(); }
    virtual void set_weak_formulation(const Hermes::Hermes2D::WeakForm<Scalar>* wf) {m_picardSolver->set_weak_formulation(wf);}

private:
    QSharedPointer<Hermes::Hermes2D::PicardSolver<Scalar> > m_picardSolver;
};

// solve
template <typename Scalar>
class Solver
{
public:
    Solver() : m_hermesSolverContainer(NULL) {m_exactSolutionFunctions.clear();}
    ~Solver();

    void init(Block* block);

    void createInitialSpace();
    void solveInitialTimeStep();

    // returns the value of the next time step lenght (for transient problems), using BDF2 approximation
    NextTimeStep estimateTimeStepLenght(int timeStep, int adaptivityStep);

    void solveSimple(int timeStep, int adaptivityStep);
    void solveReferenceAndProject(int timeStep, int adaptivityStep, bool solutionExists);
    bool createAdaptedSpace(int timeStep, int adaptivityStep);

    // for time dependent problems
    void updateExactSolutionFunctions();
private:
    Block* m_block;

    HermesSolverContainer<Scalar>* m_hermesSolverContainer;

    MultiSpace<Scalar> m_actualSpaces;

    QString m_solverID;
    QString m_solverName;

    //VectorStore<Scalar> m_lastVector;

    // elapsed time
    double m_elapsedTime;

    QMap<ExactSolutionScalarAgros<double>*, SceneBoundary *> m_exactSolutionFunctions;

    // weak form
    //WeakFormAgros<Scalar> *m_wf;

    QMap<FieldInfo*, QSharedPointer<Hermes::Hermes2D::Mesh> > readMesh();
    //void createSpace(QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> meshes, MultiSolutionArray<Scalar>& msa);
//    void createInitialSolution(Hermes::Hermes2D::Mesh* mesh, MultiSolutionArray<Scalar>& msa);
    Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > createCoarseSpace();
    void initSelectors(Hermes::vector<Hermes::Hermes2D::ProjNormType>& projNormType,
                       Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *>& selector);
    void deleteSelectors(Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *>& selector);

    void cleanup();

    void solveOneProblem(Scalar* solutionVector, MultiSpace<Scalar> spaces, int adaptivityStep, MultiSolution<Scalar> previousSolution = MultiSolution<Scalar>());

    MultiSpace<Scalar> deepMeshAndSpaceCopy(MultiSpace<Scalar> spaces, bool createReference);
    void saveSolution(BlockSolutionID id, Scalar* solutionVector);

};

#endif // SOLVER_H
