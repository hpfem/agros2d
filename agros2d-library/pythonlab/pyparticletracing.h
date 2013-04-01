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
    void getInitialPosition(double &x, double &y)
    {
        x = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartX).toDouble();
        y = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartY).toDouble();
    }
    void setInitialVelocity(double x, double y);
    void getInitialVelocity(double &x, double &y)
    {
        x = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartVelocityX).toDouble();
        y = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartVelocityY).toDouble();
    }
    void setNumberOfParticles(int particles);
    inline int getNumberOfParticles() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleNumberOfParticles).toInt(); }
    void setStartingRadius(double radius);
    inline double getStartingRadius() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartingRadius).toDouble(); }
    void setParticleMass(double mass);
    inline double getParticleMass() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMass).toDouble(); }
    void setParticleCharge(double charge);
    inline double getParticleCharge() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleConstant).toDouble(); }
    void setDragForceDensity(double rho);
    inline double getDragForceDensity() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragDensity).toDouble(); }
    void setDragForceReferenceArea(double area);
    inline double getDragForceReferenceArea() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragReferenceArea).toDouble(); }
    void setDragForceCoefficient(double coeff);
    inline double getDragForceCoefficient() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragCoefficient).toDouble(); }
    void setCustomForce(double x, double y, double z);
    void getCustomForce(double &x, double &y, double &z);
    void setIncludeRelativisticCorrection(bool include);
    inline bool getIncludeRelativisticCorrection() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleIncludeRelativisticCorrection).toBool(); }
    void setReflectOnDifferentMaterial(bool reflect);
    inline bool getReflectOnDifferentMaterial() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleReflectOnDifferentMaterial).toBool(); }
    void setReflectOnBoundary(bool reflect);
    inline bool getReflectOnBoundary() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleReflectOnBoundary).toBool(); }
    void setCoefficientOfRestitution(double coeff);
    inline double getCoefficientOfRestitution() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCoefficientOfRestitution).toDouble(); }
    void setMaximumTolerance(double tolerance);
    inline double getMaximumTolerance() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMaximumRelativeError).toDouble(); }
    void setMaximumNumberOfSteps(int steps);
    inline int getMaximumNumberOfSteps() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMaximumNumberOfSteps).toInt(); }
    void setMinimumStep(int step);
    inline int getMinimumStep() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMinimumStep).toInt(); }

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
