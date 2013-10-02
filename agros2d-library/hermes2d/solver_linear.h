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

#ifndef SOLVER_LINEAR_H
#define SOLVER_LINEAR_H

#include "util.h"
#include "solutiontypes.h"
#include "solver.h"

class Block;
class FieldInfo;

template <typename Scalar>
class ExactSolutionScalarAgros;

class SceneBoundary;

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
    virtual LinearMatrixSolver<Scalar> *linearSolver() { return m_linearSolver->get_linear_matrix_solver(); }

private:
    Hermes::Hermes2D::LinearSolver<Scalar> *m_linearSolver;
};

#endif // SOLVER_LINEAR_H
