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

#include "util/global.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

class PyParticleTracing
{
public:
    PyParticleTracing() {}
    ~PyParticleTracing() {}

    // initial position
    void getInitialPosition(double &x, double &y)
    {
        x = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartX).toDouble();
        y = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartY).toDouble();
    }
    void setInitialPosition(double x, double y);

    // initial velocity
    void getInitialVelocity(double &x, double &y)
    {
        x = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartVelocityX).toDouble();
        y = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartVelocityY).toDouble();
    }
    void setInitialVelocity(double x, double y);

    // number of particles
    inline int getNumberOfParticles() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleNumberOfParticles).toInt(); }
    void setNumberOfParticles(int particles);

    // starting radius
    inline double getStartingRadius() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartingRadius).toDouble(); }
    void setStartingRadius(double radius);

    // particle mass
    inline double getParticleMass() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMass).toDouble(); }
    void setParticleMass(double mass);

    // particle charge
    inline double getParticleCharge() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleConstant).toDouble(); }
    void setParticleCharge(double charge);

    // drag force
    inline double getDragForceDensity() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragDensity).toDouble(); }
    void setDragForceDensity(double rho);
    inline double getDragForceReferenceArea() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragReferenceArea).toDouble(); }
    void setDragForceReferenceArea(double area);
    inline double getDragForceCoefficient() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragCoefficient).toDouble(); }
    void setDragForceCoefficient(double coeff);

    // custom force
    void getCustomForce(double &x, double &y, double &z);
    void setCustomForce(double x, double y, double z);

    // solver
    inline std::string getButcherTableType() const
    {
        return butcherTableTypeToStringKey((Hermes::ButcherTableType) Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleButcherTableType).toInt()).toStdString();
    }
    void setButcherTableType(const std::string &tableType);
    inline bool getIncludeRelativisticCorrection() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleIncludeRelativisticCorrection).toBool(); }
    void setIncludeRelativisticCorrection(bool include);
    inline double getMaximumTolerance() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMaximumRelativeError).toDouble(); }
    void setMaximumTolerance(double tolerance);
    inline int getMaximumNumberOfSteps() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMaximumNumberOfSteps).toInt(); }
    void setMaximumNumberOfSteps(int steps);
    inline int getMinimumStep() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMinimumStep).toInt(); }
    void setMinimumStep(int step);

    // reflection
    inline bool getReflectOnDifferentMaterial() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleReflectOnDifferentMaterial).toBool(); }
    void setReflectOnDifferentMaterial(bool reflect);
    inline bool getReflectOnBoundary() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleReflectOnBoundary).toBool(); }
    void setReflectOnBoundary(bool reflect);
    inline double getCoefficientOfRestitution() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCoefficientOfRestitution).toDouble(); }
    void setCoefficientOfRestitution(double coeff);

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
