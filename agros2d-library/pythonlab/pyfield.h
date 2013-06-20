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

        // field id
        inline std::string fieldId() const { return m_fieldInfo->fieldId().toStdString(); }

        // analysis type
        inline std::string getAnalysisType() const { return analysisTypeToStringKey(m_fieldInfo->analysisType()).toStdString(); }
        void setAnalysisType(const std::string &analysisType);

        // linearity type
        inline std::string getLinearityType() const { return linearityTypeToStringKey(m_fieldInfo->linearityType()).toStdString(); }
        void setLinearityType(const std::string &linearityType);

        // adaptivity type
        inline std::string getAdaptivityType() const { return adaptivityTypeToStringKey(m_fieldInfo->adaptivityType()).toStdString(); }
        void setAdaptivityType(const std::string &adaptivityType);

        // matrix solver
        inline std::string getMatrixSolver() const { return matrixSolverTypeToStringKey(m_fieldInfo->matrixSolver()).toStdString(); }
        void setMatrixSolver(const std::string &matrixSolver);

        // linear solver absolute tolerance
        inline double getLinearSolverAbsoluteTolerance() const { return m_fieldInfo->value(FieldInfo::LinearSolverIterToleranceAbsolute).toDouble(); }
        void setLinearSolverAbsoluteTolerance(double absoluteTolerance);

        // linear solver iterations
        inline int getLinearSolverIterations() const { return m_fieldInfo->value(FieldInfo::LinearSolverIterIters).toInt(); }
        void setLinearSolverIterations(int numberOfIterations);

        // linear solver method
        inline std::string getLinearSolverMethod() const {
             return iterLinearSolverMethodToStringKey((Hermes::Solvers::ParalutionLinearMatrixSolver<double>::ParalutionSolverType) m_fieldInfo->value(FieldInfo::LinearSolverIterMethod).toInt()).toStdString();
        }
        void setLinearSolverMethod(const std::string &linearSolverMethod);

        // linear solver preconditioner
        inline std::string getLinearSolverPreconditioner() const {
            return iterLinearSolverPreconditionerTypeToStringKey((Hermes::Solvers::ParalutionPrecond<double>::ParalutionPreconditionerType) m_fieldInfo->value(FieldInfo::LinearSolverIterPreconditioner).toInt()).toStdString();
        }
        void setLinearSolverPreconditioner(const std::string &linearSolverPreconditioner);

        // number of refinements
        inline int getNumberOfRefinements() const { return m_fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt(); }
        void setNumberOfRefinements(int numberOfRefinements);

        // polynomial order
        inline int getPolynomialOrder() const { return m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); }
        void setPolynomialOrder(int polynomialOrder);

        // convergence measurement
        inline std::string getNonlinearConvergenceMeasurement() const { return nonlinearSolverConvergenceMeasurementToStringKey((Hermes::Hermes2D::NewtonSolverConvergenceMeasurementType) m_fieldInfo->value(FieldInfo::NonlinearConvergenceMeasurement).toInt()).toStdString(); }
        void setNonlinearConvergenceMeasurement(const std::string &nonlinearConvergenceMeasurement);

        // nonlinear tolerance
        inline double getNonlinearTolerance() const { return m_fieldInfo->value(FieldInfo::NonlinearTolerance).toDouble(); }
        void setNonlinearTolerance(double nonlinearTolerance);

        // nonlinear steps
        inline int getNonlinearSteps() const { return m_fieldInfo->value(FieldInfo::NonlinearSteps).toInt(); }
        void setNonlinearSteps(int nonlinearSteps);

        // damping coefficient
        inline double getNewtonDampingCoeff() const { return m_fieldInfo->value(FieldInfo::NewtonDampingCoeff).toDouble(); }
        void setNewtonDampingCoeff(double dampingCoeff);

        // automatic damping
        inline std::string getNewtonDampingType() const { return dampingTypeToStringKey((DampingType)m_fieldInfo->value(FieldInfo::NewtonDampingType).toInt()).toStdString(); }
        void setNewtonDampingType(std::string dampingType);

        // newton reuse jacobian
        inline bool getNewtonReuseJacobian() const { return m_fieldInfo->value(FieldInfo::NewtonReuseJacobian).toBool(); }
        void setNewtonReuseJacobian(bool reuse);

        // steps to increase damping coeff
        inline int getNewtonDampingNumberToIncrease() const { return m_fieldInfo->value(FieldInfo::NewtonStepsToIncreaseDF).toInt(); }
        void setNewtonDampingNumberToIncrease(int dampingNumberToIncrease);

        // sufficient improvement factor for Jacobian reuse
        inline double getNewtonSufficientImprovementFactorForJacobianReuse() const { return m_fieldInfo->value(FieldInfo::NewtonSufImprovForJacobianReuse).toDouble(); }
        void setNewtonSufficientImprovementFactorForJacobianReuse(double sufficientImprovementFactorJacobian);

        // sufficient improvement factor
        inline double getNewtonSufficientImprovementFactor() const { return m_fieldInfo->value(FieldInfo::NewtonSufImprov).toDouble(); }
        void setNewtonSufficientImprovementFactor(double sufficientImprovementFactor);

        // maximum steps with reused Jacobian
        inline int getNewtonMaximumStepsWithReusedJacobian() const { return m_fieldInfo->value(FieldInfo::NewtonMaxStepsReuseJacobian).toInt(); }
        void setNewtonMaximumStepsWithReusedJacobian(int maximumStepsWithReusedJacobian);

        // picard anderson acceleration
        inline bool getPicardAndersonAcceleration() const { return m_fieldInfo->value(FieldInfo::PicardAndersonAcceleration).toBool(); }
        void setPicardAndersonAcceleration(bool acceleration);

        // picard anderson beta
        inline double getPicardAndersonBeta() const { return m_fieldInfo->value(FieldInfo::PicardAndersonBeta).toDouble(); }
        void setPicardAndersonBeta(double beta);

        // picard anderson number of last vectors
        inline int getPicardAndersonNumberOfLastVectors() const { return m_fieldInfo->value(FieldInfo::PicardAndersonNumberOfLastVectors).toInt(); }
        void setPicardAndersonNumberOfLastVectors(int number);

        // adaptivity tolerance
        inline double getAdaptivityTolerance() const { return m_fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble(); }
        void setAdaptivityTolerance(double adaptivityTolerance);

        // adaptivity steps
        inline int getAdaptivitySteps() const { return m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt(); }
        void setAdaptivitySteps(int adaptivitySteps);

        // adaptivity threshold
        inline double getAdaptivityThreshold() const { return m_fieldInfo->value(FieldInfo::AdaptivityThreshold).toDouble(); }
        void setAdaptivityThreshold(double adaptivityThreshold);

        // adaptivity stopping criterion
        inline std::string getAdaptivityStoppingCriterion() const { return adaptivityStoppingCriterionTypeToStringKey((AdaptivityStoppingCriterionType) m_fieldInfo->value(FieldInfo::AdaptivityStoppingCriterion).toInt()).toStdString(); }
        void setAdaptivityStoppingCriterion(const std::string &adaptivityStoppingCriterion);

        // adaptivity norm
        inline std::string getAdaptivityNormType() const { return adaptivityNormTypeToStringKey((Hermes::Hermes2D::NormType) m_fieldInfo->value(FieldInfo::AdaptivityProjNormType).toInt()).toStdString(); }
        void setAdaptivityNormType(const std::string &adaptivityNormType);

        // adaptivity anisotropic
        inline bool getAdaptivityAnisotropic() const { return m_fieldInfo->value(FieldInfo::AdaptivityUseAniso).toBool(); }
        void setAdaptivityAnisotropic(bool adaptivityAnisotropic);

        // adaptivity finer reference
        inline bool getAdaptivityFinerReference() const { return m_fieldInfo->value(FieldInfo::AdaptivityFinerReference).toBool(); }
        void setAdaptivityFinerReference(bool adaptivityFinerReference);

        // adaptivity back steps
        inline int getAdaptivityBackSteps() const { return m_fieldInfo->value(FieldInfo::AdaptivityTransientBackSteps).toInt(); }
        void setAdaptivityBackSteps(int adaptivityBackSteps);

        //adaptivity redone each
        inline int getAdaptivityRedoneEach() const { return m_fieldInfo->value(FieldInfo::AdaptivityTransientRedoneEach).toInt(); }
        void setAdaptivityRedoneEach(int adaptivityRedoneEach);

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
        void solverInfo(int timeStep, int adaptivityStep, const std::string &solutionType, vector<double> &residual, vector<double> &dampingCoeff) const;

        // adaptivity info
        void adaptivityInfo(int timeStep, const std::string &solutionType, vector<double> &error, vector<int> &dofs) const;

private:
    FieldInfo *m_fieldInfo;

    SolutionMode getSolutionMode(const QString &solutionType) const;
    int getTimeStep(int timeStep, SolutionMode solutionMode) const;
    int getAdaptivityStep(int adaptivityStep, int timeStep, SolutionMode solutionMode) const;
};

#endif // PYTHONLABFIELD_H
