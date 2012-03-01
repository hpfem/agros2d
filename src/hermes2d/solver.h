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

using namespace std::tr1;
class Block;

template <typename entity>
Hermes::vector<entity*> desmartize(Hermes::vector<shared_ptr<entity> > smart_vec)
{
    Hermes::vector<entity*> vec;
    for(int i = 0; i < smart_vec.size(); i++)
        vec.push_back(smart_vec.at(i).get());
    return vec;
}

template <typename entity>
Hermes::vector<shared_ptr<entity> > smartize(Hermes::vector<entity*>  vec)
{
    Hermes::vector<shared_ptr<entity> > smart_vec;
    for(int i = 0; i < vec.size(); i++)
        smart_vec.push_back(shared_ptr<entity>(vec.at(i)));
    return smart_vec;
}

template <typename Scalar>
Hermes::vector<const Hermes::Hermes2D::Space<Scalar> *> castConst(Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> space)
{
    Hermes::vector<const Hermes::Hermes2D::Space<Scalar> *> out;
    for (int i = 0; i < space.size(); i++)
        out.push_back(const_cast<const Hermes::Hermes2D::Space<Scalar> *>(space.at(i)));

    return out;
}

class ProgressItemSolve;
class FieldInfo;

template <typename Scalar>
class WeakFormAgros;

template <typename Scalar>
struct SolutionArray
{
    double time;
    double adaptiveError;
    int adaptiveSteps;

    shared_ptr<Hermes::Hermes2D::Solution<Scalar> > sln;
    shared_ptr<Hermes::Hermes2D::Space<Scalar> > space;

    SolutionArray();
    SolutionArray(shared_ptr<Hermes::Hermes2D::Solution<Scalar> > sln, shared_ptr<Hermes::Hermes2D::Space<Scalar> > space);
    ~SolutionArray();

    void load(QDomElement element);
    void save(QDomDocument *doc, QDomElement element);
};

template <typename Scalar>
class MultiSolutionArray
{
public:
    SolutionArray<Scalar> comp(int component) {/*assert(m_solutionArrays.has(component));*/ return m_solutionArrays.at(component);}
    void add(SolutionArray<Scalar> solutionArray, int component = 0);

private:
    QList<SolutionArray<Scalar> > m_solutionArrays;
};

enum SolverAction
{
    SolverAction_Solve,
    SolverAction_AdaptivityStep,
    SolverAction_TimeStep
};

struct SolverConfig
{
    SolverAction action;
    //TODO more...
};

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

private:
    //FieldInfo *m_fieldInfo;
    Block* m_block;

    // error
    bool isError;

    // mesh file
    Hermes::Hermes2D::Mesh *mesh;

    // weak form
    WeakFormAgros<Scalar> *m_wf;
    ProgressItemSolve *m_progressItemSolve;

    Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > space;
    Hermes::vector<shared_ptr<Hermes::Hermes2D::Solution<Scalar> > > solution;

    // adaptivity
    Hermes::vector<shared_ptr<Hermes::Hermes2D::Solution<Scalar> > > solutionReference;
    Hermes::vector<Hermes::Hermes2D::ProjNormType> projNormType;
    Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *select;
    Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *> selector;

    void readMesh();
    void createSpace();

    // if copyPrevious == true, last solutions will be used (intented for doAdaptivityStep function)
    void createSolutions(bool copyPrevious = false);
    void initSelectors();

    void cleanup();

    bool solveOneProblem(Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > &spaceParam,
                         Hermes::vector<shared_ptr<Hermes::Hermes2D::Solution<Scalar> > > &solutionParam);

};

#endif // SOLVER_H
