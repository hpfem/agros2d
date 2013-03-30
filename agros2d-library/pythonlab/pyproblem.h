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
        PyProblem(bool clearProblem);
        ~PyProblem() {}

        // clear and refresh
        void clearSolution();
        void clear();
        void refresh();

        // coordinate type
        inline const std::string getCoordinateType() { return coordinateTypeToStringKey(Agros2D::problem()->config()->coordinateType()).toStdString(); }
        void setCoordinateType(const std::string coordinateType);

        // mesh type
        inline const std::string getMeshType() { return meshTypeToStringKey(Agros2D::problem()->config()->meshType()).toStdString(); }
        void setMeshType(const std::string meshType);

        // matrix solver
        inline const std::string getMatrixSolver() { return matrixSolverTypeToStringKey(Agros2D::problem()->config()->matrixSolver()).toStdString(); }
        void setMatrixSolver(const std::string matrixSolver);

        // frequency
        inline double getFrequency() { return Agros2D::problem()->config()->frequency(); }
        void setFrequency(const double frequency);

        // time step method
        inline const std::string getTimeStepMethod() { return timeStepMethodToStringKey(Agros2D::problem()->config()->timeStepMethod()).toStdString(); }
        void setTimeStepMethod(const std::string timeStepMethod);

        // time method order
        inline int getTimeMethodOrder() { return Agros2D::problem()->config()->timeOrder(); }
        void setTimeMethodOrder(const int timeMethodOrder);

        // time method tolerance
        inline double getTimeMethodTolerance() { return Agros2D::problem()->config()->timeMethodTolerance(); }
        void setTimeMethodTolerance(const double timeMethodTolerance);

        // time total
        inline double getTimeTotal() { return Agros2D::problem()->config()->timeTotal(); }
        void setTimeTotal(const double timeTotal);

        // time steps
        inline int getNumConstantTimeSteps() { return Agros2D::problem()->config()->timeNumConstantTimeSteps(); }
        void setNumConstantTimeSteps(const int timeSteps);

        // coupling
        std::string getCouplingType(const std::string sourceField, const std::string targetField);
        void setCouplingType(const std::string sourceField, const std::string targetField, const std::string type);

        //mesh and solve
        void mesh();
        void solve();
        void solveAdaptiveStep();

        // time elapsed
        double timeElapsed();

        // time steps
        void timeStepsLength(vector<double> &steps);

private:
        void checkExistingFields(QString sourceField, QString targetField);
};

#endif // PYTHONLABPROBLEM_H
