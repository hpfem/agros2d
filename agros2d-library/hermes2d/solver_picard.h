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

#ifndef SOLVER_PICARD_H
#define SOLVER_PICARD_H

#include "util.h"
#include "solutiontypes.h"
#include "solver.h"

class Block;
class FieldInfo;

template <typename Scalar>
class ExactSolutionScalarAgros;

class SceneBoundary;

template <typename Scalar>
class PicardSolverAgros : public SolverAgros, public Hermes::Hermes2D::PicardSolver<Scalar>
{
public:
    PicardSolverAgros(Block *block);

    virtual bool on_initialization();
    virtual bool on_initial_step_end();
    virtual bool on_step_begin();
    virtual bool on_step_end();
    virtual bool on_finish();

protected:
    virtual void setError();
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
    virtual LinearMatrixSolver<Scalar> *linearSolver() { return m_picardSolver->get_linear_matrix_solver(); }

    virtual SolverAgros *solver() const { return m_picardSolver; }

private:
    PicardSolverAgros<Scalar> *m_picardSolver;
};

#endif // SOLVER_PICARD_H
