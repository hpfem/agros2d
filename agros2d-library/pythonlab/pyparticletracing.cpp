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

#include "pythonlab/pyparticletracing.h"
#include "pythonlab/pythonengine_agros.h"
#include "particle/particle_tracing.h"

void PyParticleTracing::solve()
{
    if (!Agros2D::problem()->isSolved())
        throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());

    // initial position
    Point3 initialPosition;
    initialPosition.x = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartX).toDouble();
    initialPosition.y = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartY).toDouble();
    initialPosition.z = 0.0;

    // initial velocity
    Point3 initialVelocity;
    initialVelocity.x = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartVelocityX).toDouble();
    initialVelocity.y = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartVelocityY).toDouble();
    initialVelocity.z = 0.0;

    ParticleTracing particleTracing;
    particleTracing.computeTrajectoryParticle(initialPosition, initialVelocity);

    m_positions = particleTracing.positions();
    m_velocities = particleTracing.velocities();
    m_times = particleTracing.times();
}

void PyParticleTracing::positions(vector<double> &x,
                                  vector<double> &y,
                                  vector<double> &z) const
{
    for (int i = 0; i < length(); i++)
    {
        x.push_back(m_positions[i].x);
        y.push_back(m_positions[i].y);
        z.push_back(m_positions[i].z);
    }
}

void PyParticleTracing::velocities(vector<double> &x,
                                   vector<double> &y,
                                   vector<double> &z) const
{
    for (int i = 0; i < length(); i++)
    {
        x.push_back(m_velocities[i].x);
        y.push_back(m_velocities[i].y);
        z.push_back(m_velocities[i].z);
    }
}

void PyParticleTracing::times(vector<double> &time) const
{
    if (m_times.isEmpty())
        throw logic_error(QObject::tr("Trajectories of particles are not solved.").toStdString());

    for (int i = 0; i < length(); i++)
        time.push_back(m_times[i]);
}

void PyParticleTracing::getInitialPosition(vector<double> &position) const
{
    position.push_back(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartX).toDouble());
    position.push_back(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartY).toDouble());
}

void PyParticleTracing::setInitialPosition(const vector<double> &position)
{
    RectPoint rect = Agros2D::scene()->boundingBox();

    double x = position[0];
    double y = position[1];

    if (x < rect.start.x || x > rect.end.x)
        throw out_of_range(QObject::tr("The x coordinate is out of range.").toStdString());
    if (y < rect.start.y || y > rect.end.y)
        throw out_of_range(QObject::tr("The y coordinate is out of range.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleStartX, x);
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleStartY, y);
}

void PyParticleTracing::getInitialVelocity(vector<double> &velocity) const
{
    velocity.push_back(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartVelocityX).toDouble());
    velocity.push_back(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleStartVelocityY).toDouble());
}

void PyParticleTracing::setInitialVelocity(const vector<double> &velocity)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleStartVelocityX, velocity[0]);
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleStartVelocityY, velocity[1]);
}

void PyParticleTracing::setNumberOfParticles(int particles)
{
    if (particles < 1 && particles > 200)
        throw out_of_range(QObject::tr("Number of particles is out of range (1 - 200).").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleNumberOfParticles, particles);
}

void PyParticleTracing::setStartingRadius(double radius)
{
    if (radius < 0.0)
        throw out_of_range(QObject::tr("Particles dispersion cannot be negative.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleStartingRadius, radius);
}

void PyParticleTracing::setParticleMass(double mass)
{
    if (mass < 0.0)
        throw out_of_range(QObject::tr("Particle mass cannot be negative.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleMass, mass);
}

void PyParticleTracing::setParticleCharge(double charge)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleConstant, charge);
}

void PyParticleTracing::setDragForceDensity(double density)
{
    if (density < 0.0)
        throw out_of_range(QObject::tr("Drag force density cannot be negative.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleDragDensity, density);
}

void PyParticleTracing::setDragForceReferenceArea(double area)
{
    if (area < 0.0)
        throw out_of_range(QObject::tr("Drag force area cannot be negative.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleDragReferenceArea, area);
}

void PyParticleTracing::setDragForceCoefficient(double coeff)
{
    if (coeff < 0.0)
        throw out_of_range(QObject::tr("Drag force coefficient cannot be negative.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleDragCoefficient, coeff);
}

void PyParticleTracing::setCustomForce(const vector<double> &force)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleCustomForceX, force[0]);
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleCustomForceY, force[1]);
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleCustomForceZ, force[2]);
}

void PyParticleTracing::getCustomForce(vector<double> &force) const
{
    force.push_back(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCustomForceX).toDouble());
    force.push_back(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCustomForceY).toDouble());
    force.push_back(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCustomForceZ).toDouble());
}

std::string PyParticleTracing::getButcherTableType() const
{
    return butcherTableTypeToStringKey((Hermes::ButcherTableType) Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleButcherTableType).toInt()).toStdString();
}

void PyParticleTracing::setButcherTableType(const std::string &tableType)
{
    if (butcherTableTypeStringKeys().contains(QString::fromStdString(tableType)))
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleButcherTableType, butcherTableTypeFromStringKey(QString::fromStdString(tableType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(butcherTableTypeStringKeys())).toStdString());
}

void PyParticleTracing::setMaximumRelativeError(double error)
{
    if (error < 0.0)
        throw out_of_range(QObject::tr("Maximum relative error cannot be negative.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleMaximumRelativeError, error);
}

void PyParticleTracing::setMaximumNumberOfSteps(int steps)
{
    if (steps < 10 && steps > 100000)
        throw out_of_range(QObject::tr("Maximum number of steps is out of range (10 - 1e5).").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleMaximumNumberOfSteps, steps);
}

void PyParticleTracing::setMinimumStep(int step)
{
    if (step < 0.0)
        throw out_of_range(QObject::tr("Minimum step cannot be negative.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleMinimumStep, step);
}

void PyParticleTracing::setIncludeRelativisticCorrection(bool include)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleIncludeRelativisticCorrection, include);
}

void PyParticleTracing::setReflectOnDifferentMaterial(bool reflect)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleReflectOnDifferentMaterial, reflect);
}

void PyParticleTracing::setReflectOnBoundary(bool reflect)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleReflectOnBoundary, reflect);
}

void PyParticleTracing::setCoefficientOfRestitution(double coeff)
{
    if (coeff < 0.0 && coeff > 1.0)
        throw out_of_range(QObject::tr("Coefficient of restitution must be between 0 (collide inelastically) and 1 (collide elastically).").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleCoefficientOfRestitution, coeff);
}
