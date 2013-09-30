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

#ifndef PYTHONLABFIELD_H
#define PYTHONLABFIELD_H

#include "util/global.h"
#include "hermes2d/field.h"

class PyField
{
    public:
        PyField(std::string fieldId);
        ~PyField() {}

        template <typename Type>
        void setParameter(const std::string &parameter, Type value)
        {
            assert(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter)));
            m_fieldInfo->setValue(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter)), value);
        }

        inline int getBoolParameter(const std::string &parameter)
        {
            assert(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter)));
            return m_fieldInfo->value(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter))).toBool();
        }

        inline int getIntParameter(const std::string &parameter)
        {
            assert(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter)));
            return m_fieldInfo->value(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter))).toInt();
        }

        inline double getDoubleParameter(const std::string &parameter)
        {
            assert(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter)));
            return m_fieldInfo->value(m_fieldInfo->stringKeyToType(QString::fromStdString(parameter))).toDouble();
        }

        // field id
        inline std::string fieldId() const { return m_fieldInfo->fieldId().toStdString(); }

        // analysis type
        inline std::string getAnalysisType() const { return analysisTypeToStringKey(m_fieldInfo->analysisType()).toStdString(); }
        void setAnalysisType(const std::string &analysisType);

        // linearity type
        inline std::string getLinearityType() const { return linearityTypeToStringKey(m_fieldInfo->linearityType()).toStdString(); }
        void setLinearityType(const std::string &linearityType);

        // automatic damping
        inline std::string getNewtonDampingType() const { return dampingTypeToStringKey((DampingType)m_fieldInfo->value(FieldInfo::NewtonDampingType).toInt()).toStdString(); }
        void setNewtonDampingType(std::string dampingType);

        // adaptivity type
        inline std::string getAdaptivityType() const { return adaptivityTypeToStringKey(m_fieldInfo->adaptivityType()).toStdString(); }
        void setAdaptivityType(const std::string &adaptivityType);

        // matrix solver
        inline std::string getMatrixSolver() const { return matrixSolverTypeToStringKey(m_fieldInfo->matrixSolver()).toStdString(); }
        void setMatrixSolver(const std::string &matrixSolver);

        inline std::string getLinearSolverMethod() const {
             return iterLinearSolverMethodToStringKey((Hermes::Solvers::IterSolverType) m_fieldInfo->value(FieldInfo::LinearSolverIterMethod).toInt()).toStdString();
        }
        void setLinearSolverMethod(const std::string &linearSolverMethod);

        inline std::string getLinearSolverPreconditioner() const {
            return iterLinearSolverPreconditionerTypeToStringKey((Hermes::Solvers::PreconditionerType) m_fieldInfo->value(FieldInfo::LinearSolverIterPreconditioner).toInt()).toStdString();
        }
        void setLinearSolverPreconditioner(const std::string &linearSolverPreconditioner);

        // number of refinements
        inline int getNumberOfRefinements() const { return m_fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt(); }
        void setNumberOfRefinements(int numberOfRefinements);

        // polynomial order
        inline int getPolynomialOrder() const { return m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); }
        void setPolynomialOrder(int polynomialOrder);

        // adaptivity stopping criterion
        inline std::string getAdaptivityStoppingCriterion() const { return adaptivityStoppingCriterionTypeToStringKey((AdaptivityStoppingCriterionType) m_fieldInfo->value(FieldInfo::AdaptivityStoppingCriterion).toInt()).toStdString(); }
        void setAdaptivityStoppingCriterion(const std::string &adaptivityStoppingCriterion);

        // adaptivity norm
        inline std::string getAdaptivityErrorCalculator() const { return m_fieldInfo->value(FieldInfo::AdaptivityErrorCalculator).toString().toStdString(); }
        void setAdaptivityErrorCalculator(const std::string &calculator);

        // initial condition
        inline double getInitialCondition() const { return m_fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble(); }
        void setInitialCondition(double initialCondition);

        // time steps skip
        inline int getTimeSkip() const { return m_fieldInfo->value(FieldInfo::TransientTimeSkip).toInt(); }
        void setTimeSkip(double timeSkip);

        // boundaries
        void addBoundary(const std::string &name, const std::string &type,
                         const map<std::string, double> &parameters,
                         const map<std::string, std::string > &expressions);
        void modifyBoundary(const std::string &name, const std::string &type,
                            const map<std::string, double> &parameters,
                            const map<std::string, std::string> &expressions);
        void removeBoundary(const std::string &name);

        // materials
        void addMaterial(const std::string &name, const map<std::string, double> &parameters,
                         const map<std::string, std::string> &expressions,
                         const map<std::string, vector<double> > &nonlin_x,
                         const map<std::string, vector<double> > &nonlin_y,
                         const map<std::string, map<std::string, std::string> > &settings_map);
        void modifyMaterial(const std::string &name, const map<std::string, double> &parameters,
                            const map<std::string, std::string> &expressions,
                            const map<std::string, vector<double> > &nonlin_x,
                            const map<std::string, vector<double> > &nonlin_y,
                            const map<std::string, map<std::string, std::string> > &settings_map);
        void removeMaterial(const std::string &name);

        // local values, integrals
        void localValues(double x, double y, int timeStep, int adaptivityStep,
                         const std::string &solutionType, map<std::string, double> &results) const;
        void surfaceIntegrals(const vector<int> &edges, int timeStep, int adaptivityStep,
                              const std::string &solutionType, map<std::string, double> &results) const;
        void volumeIntegrals(const vector<int> &labels, int timeStep, int adaptivityStep,
                             const std::string &solutionType, map<std::string, double> &results) const;

        // mesh info
        void initialMeshInfo(map<std::string, int> &info) const;
        void solutionMeshInfo(int timeStep, int adaptivityStep, const std::string &solutionType, map<std::string, int> &info) const;

        // solver info
        void solverInfo(int timeStep, int adaptivityStep, const std::string &solutionType, vector<double> &residual, vector<double> &dampingCoeff, int &jacobianCalculations) const;

        // adaptivity info
        void adaptivityInfo(int timeStep, const std::string &solutionType, vector<double> &error, vector<int> &dofs) const;

        // matrix and RHS
        std::string filenameMatrix(int timeStep, int adaptivityStep) const;
        std::string filenameRHS(int timeStep, int adaptivityStep) const;

private:
    FieldInfo *m_fieldInfo;

    SolutionMode getSolutionMode(const QString &solutionType) const;
    int getTimeStep(int timeStep, SolutionMode solutionMode) const;
    int getAdaptivityStep(int adaptivityStep, int timeStep, SolutionMode solutionMode) const;
};

#endif // PYTHONLABFIELD_H
