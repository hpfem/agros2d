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

#ifndef PYTHONLABPROBLEM_H
#define PYTHONLABPROBLEM_H

#include "util/global.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

class PyProblem
{
    public:
        PyProblem(bool clearproblem);
        ~PyProblem() {}

        // clear and refresh
        void clear();
        void clearSolution() { Agros2D::problem()->clearSolution(); }
        void refresh();

        // name
        inline const char *getName() { return Agros2D::problem()->config()->name().toStdString().c_str(); }
        void setName(const char *name) { Agros2D::problem()->config()->setName(QString(name)); }

        // coordinate type
        inline const char *getCoordinateType() { return coordinateTypeToStringKey(Agros2D::problem()->config()->coordinateType()).toStdString().c_str(); }
        void setCoordinateType(const char *coordinateType);

        // mesh type
        inline const char *getMeshType() { return meshTypeToStringKey(Agros2D::problem()->config()->meshType()).toStdString().c_str(); }
        void setMeshType(const char *meshType);

        // matrix solver
        inline const char *getMatrixSolver() { return matrixSolverTypeToStringKey(Agros2D::problem()->config()->matrixSolver()).toStdString().c_str(); }
        void setMatrixSolver(const char *matrixSolver);

        // frequency
        inline double getFrequency() { return Agros2D::problem()->config()->frequency(); }
        void setFrequency(const double frequency);

        // time step method
        inline const char *getTimeStepMethod() { return timeStepMethodToStringKey(Agros2D::problem()->config()->timeStepMethod()).toStdString().c_str(); }
        void setTimeStepMethod(const char *timeStepMethod);

        // time method order
        inline int getTimeMethodOrder() { return Agros2D::problem()->config()->timeOrder(); }
        void setTimeMethodOrder(const int timeMethodOrder);

        // time method tolerance
        inline double getTimeMethodTolerance() { return Agros2D::problem()->config()->timeMethodTolerance().number(); }
        void setTimeMethodTolerance(const double timeMethodTolerance);

        // time total
        inline double getTimeTotal() { return Agros2D::problem()->config()->timeTotal().number(); }
        void setTimeTotal(const double timeTotal);

        // time steps
        inline int getNumConstantTimeSteps() { return Agros2D::problem()->config()->numConstantTimeSteps(); }
        void setNumConstantTimeSteps(const int timeSteps);

        // coupling
        char *getCouplingType(const char *sourceField, const char *targetField);
        void setCouplingType(const char *sourceField, const char *targetField, const char *type);

        //mesh and solve
        void mesh();
        void solve();
        void solveAdaptiveStep();

        // time elapsed
        double timeElapsed();

        // time steps
        void timeStepsLength(vector<double> &steps);
};

#endif // PYTHONLABPROBLEM_H
