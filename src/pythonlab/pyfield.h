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

#include "util.h"
#include "scene.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"

class Solution;
class SceneViewPreprocessor;
class SceneViewMesh;
class SceneViewPost2D;
class SceneViewPost3D;
class PostHermes;

// field
class PyField
{
    private:
        FieldInfo *m_fieldInfo;

    public:
        PyField(char *fieldId);
        ~PyField() {}

        FieldInfo *fieldInfo();

        // field id
        inline const char *getFieldId() const { return m_fieldInfo->fieldId().toStdString().c_str(); }

        // analysis type
        inline const char *getAnalysisType() const { return analysisTypeToStringKey(Util::problem()->fieldInfo(m_fieldInfo->fieldId())->analysisType()).toStdString().c_str(); }
        void setAnalysisType(const char *analysisType);

        // number of refinements
        inline int getNumberOfRefinements() const { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->numberOfRefinements(); }
        void setNumberOfRefinements(const int numberOfRefinements);

        // polynomial order
        inline int getPolynomialOrder() const { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->polynomialOrder(); }
        void setPolynomialOrder(const int polynomialOrder);

        // linearity type
        inline const char *getLinearityType() const { return linearityTypeToStringKey(Util::problem()->fieldInfo(m_fieldInfo->fieldId())->linearityType()).toStdString().c_str(); }
        void setLinearityType(const char *linearityType);

        // nonlinear tolerance
        inline double getNonlinearTolerance() const { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->nonlinearTolerance(); }
        void setNonlinearTolerance(const double nonlinearTolerance);

        // nonlinear steps
        inline int getNonlinearSteps() const { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->nonlinearSteps(); }
        void setNonlinearSteps(const int nonlinearSteps);

        // damping coefficient
        inline double getDampingCoeff() const { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->newtonDampingCoeff(); }
        void setDampingCoeff(const double dampingCoeff);

        // automatic damping
        inline bool getAutomaticDamping() const { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->newtonAutomaticDamping(); }
        void setAutomaticDamping(const bool automaticDamping);

        // steps to increase damping coeff
        inline int getDampingNumberToIncrease() const { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->newtonDampingNumberToIncrease(); }
        void setDampingNumberToIncrease(const int dampingNumberToIncrease);

        // picard anderson acceleration
        inline bool getPicardAndersonAcceleration() const { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->picardAndersonAcceleration(); }
        void setPicardAndersonAcceleration(const bool acceleration);

        // picard anderson beta
        inline double getPicardAndersonBeta() const { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->picardAndersonBeta(); }
        void setPicardAndersonBeta(const double beta);

        // picard anderson number of last vectors
        inline int getPicardAndersonNumberOfLastVectors() const { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->picardAndersonNumberOfLastVectors(); }
        void setPicardAndersonNumberOfLastVectors(const int number);

        // adaptivity type
        inline const char *getAdaptivityType() const { return adaptivityTypeToStringKey(Util::problem()->fieldInfo(m_fieldInfo->fieldId())->adaptivityType()).toStdString().c_str(); }
        void setAdaptivityType(const char *adaptivityType);

        // adaptivity tolerance
        inline double getAdaptivityTolerance() const { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->adaptivityTolerance(); }
        void setAdaptivityTolerance(const double adaptivityTolerance);

        // adaptivity steps
        inline int getAdaptivitySteps() const { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->adaptivitySteps(); }
        void setAdaptivitySteps(const int adaptivitySteps);

        // initial condition
        inline double getInitialCondition() const { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->initialCondition().number(); }
        void setInitialCondition(const double initialCondition);

        // time steps skip
        inline int getTimeSkip() const { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->timeSkip().number(); }
        void setTimeSkip(const double timeSkip);

        // boundaries
        void addBoundary(char *name, char *type,
                         map<char*, double> parameters,
                         map<char*, char* > expressions);
        void setBoundary(char *name, char *type,
                         map<char*, double> parameters,
                         map<char *, char *> expressions);
        void removeBoundary(char *name);

        // materials
        void addMaterial(char *name, map<char*, double> parameters,
                         map<char *, char *> expressions,
                         map<char*, vector<double> > nonlin_x,
                         map<char*, vector<double> > nonlin_y);
        void setMaterial(char *name, map<char*, double> parameters,
                         map<char*, char* > expressions,
                         map<char *, vector<double> > nonlin_x,
                         map<char *, vector<double> > nonlin_y);
        void removeMaterial(char *name);

        // solve
        void solve() { assert(0); qDebug() << "Not now :)"; }

        // local values, integrals
        void localValues(double x, double y, map<std::string, double> &results);
        void surfaceIntegrals(vector<int> edges, map<std::string, double> &results);
        void volumeIntegrals(vector<int> labels, map<std::string, double> &results);
};

#endif // PYTHONLABFIELD_H
