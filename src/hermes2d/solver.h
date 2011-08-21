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
#include "module.h"

class ProgressItemSolve;

// solve
template <typename Scalar>
class SolutionAgros
{
public:
    SolutionAgros(ProgressItemSolve *progressItemSolve, WeakFormAgros<Scalar> *wf);

    Hermes::vector<SolutionArray<Scalar> *> solveSolutionArray(Hermes::vector<Hermes::Hermes2D::EssentialBCs<Scalar> > bcs);
    Hermes::vector<SolutionArray<Scalar> *> solveSolutionArrayOld(Hermes::vector<Hermes::Hermes2D::EssentialBCs<Scalar> > bcs);
private:
    int polynomialOrder;
    AdaptivityType adaptivityType;
    int adaptivitySteps;
    double adaptivityTolerance;
    int adaptivityMaxDOFs;
    int numberOfSolution;
    double timeTotal;
    double timeStep;
    double initialCondition;

    AnalysisType analysisType;

    double nonlinearTolerance;
    int nonlinearSteps;

    Hermes::MatrixSolverType matrixSolver;

    // error
    bool isError;

    // mesh file
    Hermes::Hermes2D::Mesh *mesh;

    // weak form
    WeakFormAgros<Scalar> *m_wf;
    ProgressItemSolve *m_progressItemSolve;

    Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solution;
    Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> space;
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutionReference;
    Hermes::vector<Hermes::Hermes2D::ProjNormType> projNormType;
    Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *> selector;


    void initCalculation(Hermes::vector<Hermes::Hermes2D::EssentialBCs<Scalar> >& bcs);
    void cleanup();

    bool solveOneProblem(Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaceParam,
                         Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutionParam);



    SolutionArray<Scalar> *solutionArray(Hermes::Hermes2D::Solution<Scalar> *sln, Hermes::Hermes2D::Space<Scalar> *space = NULL, double adaptiveError = 0.0, double adaptiveSteps = 0.0, double time = 0.0);

    bool solveLinear(Hermes::Hermes2D::DiscreteProblem<Scalar> *dp,
                     Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> space,
                     Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solution,
                     Hermes::Solvers::LinearSolver<Scalar> *solver, SparseMatrix<Scalar> *matrix, Vector<Scalar> *rhs);
};

#endif // SOLVER_H
