// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef SOLVER_PICARD_H
#define SOLVER_PICARD_H

#include "util.h"
#include "solutiontypes.h"
#include "solver.h"

class FieldInfo;

template <typename Scalar>
class ExactSolutionScalarAgros;

class SceneBoundary;

template <typename Scalar>
class PicardSolverAgros : public SolverAgros, public PicardSolver<Scalar>
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
    virtual Mixins::SettableSpaces<Scalar>* setTableSpaces() { return m_picardSolver; }
    virtual void setWeakFormulation(WeakFormSharedPtr<Scalar> wf) { m_picardSolver->set_weak_formulation(wf); }
    virtual LinearMatrixSolver<Scalar> *linearSolver() { return m_picardSolver->get_linear_matrix_solver(); }

    virtual SolverAgros *solver() const { return m_picardSolver; }

private:
    PicardSolverAgros<Scalar> *m_picardSolver;
};

#endif // SOLVER_PICARD_H
