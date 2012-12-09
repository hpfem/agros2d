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

#include "sceneview_post2d.h"
#include "sceneview_post3d.h"

#include "particle/particle_tracing.h"

void PyParticleTracing::solve()
{
    if (!Util::problem()->isSolved())
        throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());

    // store values
    // double particleStartingRadius = Util::config()->particleStartingRadius;
    // int particleNumberOfParticles = Util::config()->particleNumberOfParticles;
    Util::config()->particleStartingRadius = 0.0;
    Util::config()->particleNumberOfParticles = 1;

    ParticleTracing particleTracing;
    particleTracing.computeTrajectoryParticle(false);

    m_positions = particleTracing.positions();
    m_velocities = particleTracing.velocities();
    m_times = particleTracing.times();

    currentPythonEngineAgros()->postHermes()->refresh();

    // restore values
    // Util::config()->particleStartingRadius = particleStartingRadius;
    // Util::config()->particleNumberOfParticles = particleNumberOfParticles;
}

void PyParticleTracing::positions(std::vector<double> &x,
                                  std::vector<double> &y,
                                  std::vector<double> &z)
{
    std::vector<double> outX;
    std::vector<double> outY;
    std::vector<double> outZ;
    for (int i = 0; i < length(); i++)
    {
        outX.push_back(m_positions[i].x);
        outY.push_back(m_positions[i].y);
        outZ.push_back(m_positions[i].z);
    }

    x = outX;
    y = outY;
    z = outZ;
}

void PyParticleTracing::velocities(std::vector<double> &x,
                                   std::vector<double> &y,
                                   std::vector<double> &z)
{
    std::vector<double> outX;
    std::vector<double> outY;
    std::vector<double> outZ;
    for (int i = 0; i < length(); i++)
    {
        outX.push_back(m_velocities[i].x);
        outY.push_back(m_velocities[i].y);
        outZ.push_back(m_velocities[i].z);
    }

    x = outX;
    y = outY;
    z = outZ;
}

void PyParticleTracing::times(std::vector<double> &time)
{
    std::vector<double> outTime;
    for (int i = 0; i < length(); i++)
    {
        outTime.push_back(m_times[i]);
    }

    time = outTime;
}

void PyParticleTracing::setInitialPosition(double x, double y)
{
    RectPoint rect = Util::scene()->boundingBox();

    if (x < rect.start.x || x > rect.end.x)
        throw out_of_range(QObject::tr("x coordinate is out of range.").toStdString());
    if (y < rect.start.y || y > rect.end.y)
        throw out_of_range(QObject::tr("y coordinate is out of range.").toStdString());

    Util::config()->particleStart = Point(x, y);
    Util::scene()->invalidate();
}

void PyParticleTracing::setInitialVelocity(double x, double y)
{
    Util::config()->particleStartVelocity = Point(x, y);
    Util::scene()->invalidate();
}

void PyParticleTracing::setParticleMass(double mass)
{
    if (mass <= 0.0)
        throw out_of_range(QObject::tr("Mass must be positive.").toStdString());

    Util::config()->particleMass = mass;
    Util::scene()->invalidate();
}

void PyParticleTracing::setParticleCharge(double charge)
{
    Util::config()->particleConstant = charge;
    Util::scene()->invalidate();
}

void PyParticleTracing::setDragForceDensity(double rho)
{
    if (rho < 0.0)
        throw out_of_range(QObject::tr("Density cannot be negative.").toStdString());

    Util::config()->particleDragDensity = rho;
    Util::scene()->invalidate();
}

void PyParticleTracing::setDragForceReferenceArea(double area)
{
    if (area < 0.0)
        throw out_of_range(QObject::tr("Area cannot be negative.").toStdString());

    Util::config()->particleDragReferenceArea = area;
    Util::scene()->invalidate();
}

void PyParticleTracing::setDragForceCoefficient(double coeff)
{
    if (coeff < 0.0)
        throw out_of_range(QObject::tr("Coefficient cannot be negative.").toStdString());

    Util::config()->particleDragCoefficient = coeff;
    Util::scene()->invalidate();
}

void PyParticleTracing::setIncludeRelativisticCorrection(int include)
{
    Util::config()->particleIncludeRelativisticCorrection = include;
    Util::scene()->invalidate();
}

void PyParticleTracing::setReflectOnDifferentMaterial(int reflect)
{
    Util::config()->particleReflectOnDifferentMaterial = reflect;
    Util::scene()->invalidate();
}

void PyParticleTracing::setReflectOnBoundary(int reflect)
{
    Util::config()->particleReflectOnBoundary = reflect;
    Util::scene()->invalidate();
}

void PyParticleTracing::setCoefficientOfRestitution(double coeff)
{
    if (coeff < 0.0)
        throw out_of_range(QObject::tr("Coefficient of restitution must be between 0 (collide inelastically) and 1 (collide elastically).").toStdString());

    Util::config()->particleCoefficientOfRestitution = coeff;
    Util::scene()->invalidate();
}

void PyParticleTracing::setMaximumTolerance(double tolerance)
{
    if (tolerance < 0.0)
        throw out_of_range(QObject::tr("Tolerance cannot be negative.").toStdString());

    Util::config()->particleMaximumRelativeError = tolerance;
    Util::scene()->invalidate();
}

void PyParticleTracing::setMaximumNumberOfSteps(int steps)
{
    if (steps < 0.0)
        throw out_of_range(QObject::tr("Maximum number of steps cannot be negative.").toStdString());

    Util::config()->particleMaximumNumberOfSteps = steps;
    Util::scene()->invalidate();
}

void PyParticleTracing::setMinimumStep(int step)
{
    if (step < 0.0)
        throw out_of_range(QObject::tr("Minimum step cannot be negative.").toStdString());

    Util::config()->particleMinimumStep = step;
    Util::scene()->invalidate();
}
