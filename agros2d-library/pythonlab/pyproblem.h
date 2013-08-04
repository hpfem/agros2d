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
        void clear();
        void clearSolution();
        void refresh();

        // coordinate type
        inline std::string getCoordinateType() const { return coordinateTypeToStringKey(Agros2D::problem()->config()->coordinateType()).toStdString(); }
        void setCoordinateType(const std::string &coordinateType);

        // mesh type
        inline std::string getMeshType() const { return meshTypeToStringKey(Agros2D::problem()->config()->meshType()).toStdString(); }
        void setMeshType(const std::string &meshType);

        // curvilinear elements
        inline bool getCurvilinearElements() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_MeshCurvilinearElements).toBool(); }
        void setCurvilinearElements(bool curvilinearElements) { Agros2D::problem()->setting()->setValue(ProblemSetting::View_MeshCurvilinearElements, curvilinearElements); }

        // angle segments count
        inline int getAngleSegmentsCount() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_MeshAngleSegmentsCount).toInt(); }
        void setAngleSegmentsCount(int count);

        // frequency
        inline double getFrequency() const { return Agros2D::problem()->config()->value(ProblemConfig::Frequency).toDouble(); }
        void setFrequency(double frequency);

        // time step method
        inline std::string getTimeStepMethod() const { return timeStepMethodToStringKey((TimeStepMethod) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt()).toStdString(); }
        void setTimeStepMethod(const std::string &timeStepMethod);

        // time method order
        inline int getTimeMethodOrder() const { return Agros2D::problem()->config()->value(ProblemConfig::TimeOrder).toInt(); }
        void setTimeMethodOrder(int timeMethodOrder);

        // time method tolerance
        inline double getTimeMethodTolerance() const { return Agros2D::problem()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble(); }
        void setTimeMethodTolerance(double timeMethodTolerance);

        // time total
        inline double getTimeTotal() const { return Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble(); }
        void setTimeTotal(double timeTotal);

        // time steps
        inline int getNumConstantTimeSteps() const { return Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt(); }
        void setNumConstantTimeSteps(int timeSteps);

        // coupling
        std::string getCouplingType(const std::string &sourceField, const std::string &targetField) const;
        void setCouplingType(const std::string &sourceField, const std::string &targetField, const std::string &type);

        // mesh and solve
        void mesh();
        void solve();
        void solveAdaptiveStep();

        // time elapsed
        double timeElapsed() const;

        // time steps
        void timeStepsLength(vector<double> &steps) const;

        // solution vector


private:
        void checkExistingFields(const QString &sourceField, const QString &targetField) const;
};

#endif // PYTHONLABPROBLEM_H
