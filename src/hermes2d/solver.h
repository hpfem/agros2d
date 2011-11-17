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

class ProgressItemSolve;

template <typename Scalar>
class WeakFormAgros;

template <typename Scalar>
Hermes::vector<const Hermes::Hermes2D::Space<Scalar> *> castConst(Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> space)
{
    Hermes::vector<const Hermes::Hermes2D::Space<Scalar> *> out;
    for (int i = 0; i < space.size(); i++)
        out.push_back(const_cast<const Hermes::Hermes2D::Space<Scalar> *>(space.at(i)));

    return out;
}

template <typename Scalar>
struct SolutionArray
{
    double time;
    double adaptiveError;
    int adaptiveSteps;

    Hermes::Hermes2D::Solution<Scalar> *sln;
    Hermes::Hermes2D::Space<Scalar> *space;

    SolutionArray();
    ~SolutionArray();

    void load(QDomElement element);
    void save(QDomDocument *doc, QDomElement element);
};


// solve
template <typename Scalar>
class SolverAgros
{
public:
    SolverAgros(ProgressItemSolve *progressItemSolve, WeakFormAgros<Scalar> *wf);

    Hermes::vector<SolutionArray<Scalar> *> solve(Hermes::vector<const Hermes::Hermes2D::Space<Scalar> *> spaceParam = Hermes::vector<const Hermes::Hermes2D::Space<Scalar> *>(),
                                                  Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutionParam = Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *>());
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
    ProblemType problemType;
    AnalysisType analysisType;

    LinearityType linearityType;
    double nonlinearTolerance;
    int nonlinearSteps;

    Hermes::MatrixSolverType matrixSolver;
    WeakFormsType weakFormsType;

    // error
    bool isError;

    // mesh file
    Hermes::Hermes2D::Mesh *mesh;

    // weak form
    WeakFormAgros<Scalar> *m_wf;
    ProgressItemSolve *m_progressItemSolve;

    Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> space;
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solution;

    // adaptivity
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutionReference;
    Hermes::vector<Hermes::Hermes2D::ProjNormType> projNormType;
    Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *select;
    Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *> selector;

    void readMesh();
    void createSpace();
    void initSelectors();

    void cleanup();

    bool solveOneProblem(Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> &spaceParam,
                         Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> &solutionParam);



    SolutionArray<Scalar> *solutionArray(Hermes::Hermes2D::Solution<Scalar> *solutionParam,
                                         Hermes::Hermes2D::Space<Scalar> *spaceParam = NULL,
                                         double adaptiveError = 0.0, double adaptiveSteps = 0.0, double time = 0.0);
};

#endif // SOLVER_H
