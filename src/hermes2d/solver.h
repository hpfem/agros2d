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
class WeakFormAgros;

class AgrosSolverException : public AgrosException
{
public:
    AgrosSolverException(const QString &what) : AgrosException(what)
    {
    }
};


// In BDF2 method, results of BDF2A and BDF2B can be combined to obtain solution of higher accuracy
// This class stores solution vector from the last calculation (if it should be used in this combination)
// and takes care of proper dealocation of the vector.
template <typename Scalar>
class VectorStore
{
public:
    VectorStore();
    ~VectorStore();

    Scalar* createNew(int length);
    int getLastLength() const { return m_length; }
    Scalar* getLast() const { return m_vector; }

private:
    Scalar* m_vector;
    int m_length;
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
    virtual void projectPreviousSolution(Scalar* solutionVector, Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > spaces,
                                         Hermes::vector<QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > > solutions) {}
    virtual void setMatrixRhsOutput(QString solverName, int adaptivityStep) = 0;
    virtual Hermes::Hermes2D::Mixins::SettableSpaces<Scalar>* settableSpaces() = 0;

    void setMatrixRhsOutputGen(Hermes::Hermes2D::Mixins::MatrixRhsOutput<Scalar>* solver, QString solverName, int adaptivityStep);

protected:
    Block* m_block;
};

template <typename Scalar>
class LinearSolverContainer : public HermesSolverContainer<Scalar>
{
public:
    LinearSolverContainer(Block* block, Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > spaces);
    ~LinearSolverContainer();
    virtual void solve(Scalar* solutionVector);
    virtual void setMatrixRhsOutput(QString solverName, int adaptivityStep) { setMatrixRhsOutputGen(m_linearSolver.data(), solverName, adaptivityStep); }
    virtual Hermes::Hermes2D::Mixins::SettableSpaces<Scalar>* settableSpaces() { return m_linearSolver.data(); }

private:
    QSharedPointer<Hermes::Hermes2D::LinearSolver<Scalar> > m_linearSolver;
};

template <typename Scalar>
class NewtonSolverContainer : public HermesSolverContainer<Scalar>
{
public:
    NewtonSolverContainer(Block* block, Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > spaces);
    ~NewtonSolverContainer();
    virtual void projectPreviousSolution(Scalar* solutionVector, Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > spaces,
                                         Hermes::vector<QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > > solutions);
    virtual void solve(Scalar* solutionVector);
    virtual void setMatrixRhsOutput(QString solverName, int adaptivityStep) { setMatrixRhsOutputGen(m_newtonSolver.data(), solverName, adaptivityStep); }
    virtual Hermes::Hermes2D::Mixins::SettableSpaces<Scalar>* settableSpaces() { return m_newtonSolver.data(); }

private:
    QSharedPointer<Hermes::Hermes2D::NewtonSolver<Scalar> > m_newtonSolver;
};

// solve
template <typename Scalar>
class Solver
{
public:
    Solver() : m_hermesSolverContainer(NULL) {}
    ~Solver();

    void init(Block* block);

    void solveInitialTimeStep();
    void createInitialSpace(int timeStep);

    // returns the value of the next time step lenght (for transient problems), using BDF2 approximation
    NextTimeStep estimateTimeStepLenght(int timeStep, int adaptivityStep);

    void solveSimple(int timeStep, int adaptivityStep, bool solutionExists);
    void solveReferenceAndProject(int timeStep, int adaptivityStep, bool solutionExists);
    bool createAdaptedSpace(int timeStep, int adaptivityStep);

private:
    Block* m_block;

    HermesSolverContainer<Scalar>* m_hermesSolverContainer;

    QString m_solverID;
    QString m_solverName;

    VectorStore<Scalar> m_lastVector;

    // elapsed time
    double m_elapsedTime;

    // weak form
    //WeakFormAgros<Scalar> *m_wf;

    QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> readMesh();
    void createSpace(QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> meshes, MultiSolutionArray<Scalar>& msa);
//    void createInitialSolution(Hermes::Hermes2D::Mesh* mesh, MultiSolutionArray<Scalar>& msa);
    Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > createCoarseSpace();
    void initSelectors(Hermes::vector<Hermes::Hermes2D::ProjNormType>& projNormType,
                       Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *>& selector);
    void deleteSelectors(Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *>& selector);

    void cleanup();

//    bool solveOneProblem(Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > &spaceParam,
//                         Hermes::vector<QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > > &solutionParam);
    void solveOneProblem(Scalar *solutionVector, MultiSolutionArray<Scalar> msa, MultiSolutionArray<Scalar> *previousMsa, int adaptivityStep);
    void saveSolution(Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > &spaceParam,
                      Hermes::vector<QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > > &solutionParam,
                      double actualTime);

};

#endif // SOLVER_H
