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

#ifndef SOLVER_LINEAR_H
#define SOLVER_LINEAR_H

#include "util.h"
#include "solutiontypes.h"
#include "solver.h"

class FieldInfo;

template <typename Scalar>
class ExactSolutionScalarAgros;

class SceneBoundary;

template <typename Scalar>
class LinearSolverAgros : public SolverAgros, public LinearSolver<Scalar>
{
public:
    LinearSolverAgros();

protected:
    virtual void setError() {}
};

template <typename Scalar>
class LinearSolverContainer : public HermesSolverContainer<Scalar>
{
public:
    LinearSolverContainer();
    ~LinearSolverContainer();

    void solve(Scalar* previousSolutionVector);
    virtual void setMatrixRhsOutput(QString solverName, int adaptivityStep) { this->setMatrixRhsOutputGen(m_linearSolver, solverName, adaptivityStep); }
    virtual Mixins::SettableSpaces<Scalar>* setTableSpaces() { return m_linearSolver; }
    virtual void setWeakFormulation(WeakFormSharedPtr<Scalar> wf) {m_linearSolver->set_weak_formulation(wf); }
    virtual void matrixUnchangedDueToBDF(bool unchanged);
    virtual LinearMatrixSolver<Scalar> *linearSolver() { return m_linearSolver->get_linear_matrix_solver(); }

    virtual SolverAgros *solver() const { return m_linearSolver; }

private:
    LinearSolverAgros<Scalar> *m_linearSolver;
};

#endif // SOLVER_LINEAR_H
