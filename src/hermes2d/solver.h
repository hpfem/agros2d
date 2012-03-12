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

#include <tr1/memory>
#include "util.h"
#include "solutiontypes.h"

using namespace std::tr1;
class Block;

class ProgressItemSolve;
class FieldInfo;

template <typename Scalar>
class WeakFormAgros;


// solve
template <typename Scalar>
class Solver
{
public:
    void init(ProgressItemSolve *progressItemSolve, WeakFormAgros<Scalar> *wf, Block* block);
    void clear();
    void solve(SolverConfig config);
    void doAdaptivityStep();
    void doTimeStep();

    void solveSimple();
    void solveTimeStep(double timeStep);
    void solveInitialTimeStep();

    void solveInitialAdaptivityStep();
    void solveAdaptivityStep();

private:
    //FieldInfo *m_fieldInfo;
    Block* m_block;

    // error
    bool isError;

    // weak form
    WeakFormAgros<Scalar> *m_wf;
    ProgressItemSolve *m_progressItemSolve;

    Hermes::Hermes2D::Mesh* readMesh();
    void createSpace(Hermes::Hermes2D::Mesh* mesh, MultiSolutionArray<Scalar>& msa);
//    void createInitialSolution(Hermes::Hermes2D::Mesh* mesh, MultiSolutionArray<Scalar>& msa);
    Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > createCoarseSpace();
    // if copyPrevious == true, last solutions will be used (intented for doAdaptivityStep function)
    void createNewSolutions(MultiSolutionArray<Scalar>& msa);
    void initSelectors(Hermes::vector<Hermes::Hermes2D::ProjNormType>& projNormType,
                       Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *>& selector);
    void deleteSelectors(Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *>& selector);

    void cleanup();

//    bool solveOneProblem(Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > &spaceParam,
//                         Hermes::vector<shared_ptr<Hermes::Hermes2D::Solution<Scalar> > > &solutionParam);
    bool solveOneProblem(MultiSolutionArray<Scalar> msa);
    void saveSolution(Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > &spaceParam,
                      Hermes::vector<shared_ptr<Hermes::Hermes2D::Solution<Scalar> > > &solutionParam,
                      double actualTime);

};

#endif // SOLVER_H
