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
        x = Agros2D::config()->particleStart.x;
        y = Agros2D::config()->particleStart.y;
    }
    void setInitialVelocity(double x, double y);
    void initialVelocity(double &x, double &y)
    {
        x = Agros2D::config()->particleStartVelocity.x;
        y = Agros2D::config()->particleStartVelocity.y;
    }
    void setParticleMass(double mass);
    double particleMass() { return Agros2D::config()->particleMass; }
    void setParticleCharge(double charge);
    double particleCharge() { return Agros2D::config()->particleConstant; }
    void setDragForceDensity(double rho);
    double dragForceDensity() { return Agros2D::config()->particleDragDensity; }
    void setDragForceReferenceArea(double area);
    double dragForceReferenceArea() { return Agros2D::config()->particleDragReferenceArea; }
    void setDragForceCoefficient(double coeff);
    double dragForceCoefficient() { return Agros2D::config()->particleDragCoefficient; }
    void setIncludeRelativisticCorrection(int include);
    int includeRelativisticCorrection() { return Agros2D::config()->particleIncludeRelativisticCorrection; }
    void setReflectOnDifferentMaterial(int reflect);
    int reflectOnDifferentMaterial() { return Agros2D::config()->particleReflectOnDifferentMaterial; }
    void setReflectOnBoundary(int reflect);
    int reflectOnBoundary() { return Agros2D::config()->particleReflectOnBoundary; }
    void setCoefficientOfRestitution(double coeff);
    int coefficientOfRestitution() { return Agros2D::config()->particleCoefficientOfRestitution; }
    void setMaximumTolerance(double tolerance);
    double maximumTolerance() { return Agros2D::config()->particleMaximumRelativeError; }
    void setMaximumNumberOfSteps(int steps);
    int maximumNumberOfSteps() { return Agros2D::config()->particleMaximumNumberOfSteps; }
    void setMinimumStep(int step);
    int minimumStep() { return Agros2D::config()->particleMinimumStep; }

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
