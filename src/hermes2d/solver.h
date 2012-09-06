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

class AgrosSolverException
{
public:
    AgrosSolverException(QString str) { this->str = str; }
    QString str;
};

// solve
template <typename Scalar>
class Solver
{
public:
    void init(Block* block);
    void clear();

    void solveInitialTimeStep();
    void createInitialSpace(int timeStep);

    // returns the value of the next time step lenght (for transient problems). Either calculated in the case of adaptive or initial
    double solveSimple(int timeStep, int adaptivityStep, bool solutionExists);
    void solveReferenceAndProject(int timeStep, int adaptivityStep, bool solutionExists);
    bool createAdaptedSpace(int timeStep, int adaptivityStep);

private:
    Block* m_block;

    QString m_solverID;

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
    void solveOneProblem(WeakFormAgros<Scalar> *wf, Scalar *solutionVector, MultiSolutionArray<Scalar> msa, MultiSolutionArray<Scalar> *previousMsa = NULL);
    void saveSolution(Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > &spaceParam,
                      Hermes::vector<QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > > &solutionParam,
                      double actualTime);

};

#endif // SOLVER_H
