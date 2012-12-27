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

#ifndef PYTHONLABPARTICLETRACING_H
#define PYTHONLABPARTICLETRACING_H

#include "util.h"
#include "util/global.h"

#include "scene.h"

#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

class Solution;
class SceneViewPreprocessor;
class SceneViewMesh;
class SceneViewPost2D;
class SceneViewPost3D;
class PostHermes;

class PyParticleTracing
{
public:
    PyParticleTracing() {}
    ~PyParticleTracing() {}

    void setInitialPosition(double x, double y);
    void initialPosition(double &x, double &y)
    {
        x = Agros2D::problem()->configView()->particleStart.x;
        y = Agros2D::problem()->configView()->particleStart.y;
    }
    void setInitialVelocity(double x, double y);
    void initialVelocity(double &x, double &y)
    {
        x = Agros2D::problem()->configView()->particleStartVelocity.x;
        y = Agros2D::problem()->configView()->particleStartVelocity.y;
    }

    void setParticleMass(double mass);
    double particleMass() { return Agros2D::problem()->configView()->particleMass; }
    void setParticleCharge(double charge);
    double particleCharge() { return Agros2D::problem()->configView()->particleConstant; }
    void setDragForceDensity(double rho);
    double dragForceDensity() { return Agros2D::problem()->configView()->particleDragDensity; }
    void setDragForceReferenceArea(double area);
    double dragForceReferenceArea() { return Agros2D::problem()->configView()->particleDragReferenceArea; }
    void setDragForceCoefficient(double coeff);
    double dragForceCoefficient() { return Agros2D::problem()->configView()->particleDragCoefficient; }
    void setCustomForce(map<char*, double> force);
    void customForce(map<string, double> &force);
    void setIncludeRelativisticCorrection(int include);
    int includeRelativisticCorrection() { return Agros2D::problem()->configView()->particleIncludeRelativisticCorrection; }
    void setReflectOnDifferentMaterial(int reflect);
    int reflectOnDifferentMaterial() { return Agros2D::problem()->configView()->particleReflectOnDifferentMaterial; }
    void setReflectOnBoundary(int reflect);
    int reflectOnBoundary() { return Agros2D::problem()->configView()->particleReflectOnBoundary; }
    void setCoefficientOfRestitution(double coeff);
    int coefficientOfRestitution() { return Agros2D::problem()->configView()->particleCoefficientOfRestitution; }
    void setMaximumTolerance(double tolerance);
    double maximumTolerance() { return Agros2D::problem()->configView()->particleMaximumRelativeError; }
    void setMaximumNumberOfSteps(int steps);
    int maximumNumberOfSteps() { return Agros2D::problem()->configView()->particleMaximumNumberOfSteps; }
    void setMinimumStep(int step);
    int minimumStep() { return Agros2D::problem()->configView()->particleMinimumStep; }

    void solve();

    inline int length() { return m_positions.length(); }
    void positions(vector<double> &x, vector<double> &y, vector<double> &z);
    void velocities(vector<double> &x, vector<double> &y, vector<double> &z);
    void times(std::vector<double> &time);

private:
    // position and velocity
    QList<Point3> m_positions;
    QList<Point3> m_velocities;
    QList<double> m_times;
};

#endif // PYTHONLABPARTICLETRACING_H
