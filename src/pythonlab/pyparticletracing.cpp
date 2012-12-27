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
    if (!Agros2D::problem()->isSolved())
        throw invalid_argument(QObject::tr("Problem is not solved.").toStdString());

    // store values
    // double particleStartingRadius = Agros2D::problem()->configView()->particleStartingRadius;
    // int particleNumberOfParticles = Agros2D::problem()->configView()->particleNumberOfParticles;
    Agros2D::problem()->configView()->particleStartingRadius = 0.0;
    Agros2D::problem()->configView()->particleNumberOfParticles = 1;

    ParticleTracing particleTracing;
    particleTracing.computeTrajectoryParticle(false);

    m_positions = particleTracing.positions();
    m_velocities = particleTracing.velocities();
    m_times = particleTracing.times();

    currentPythonEngineAgros()->postHermes()->refresh();

    // restore values
    // Agros2D::problem()->configView()->particleStartingRadius = particleStartingRadius;
    // Agros2D::problem()->configView()->particleNumberOfParticles = particleNumberOfParticles;
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
    RectPoint rect = Agros2D::scene()->boundingBox();

    if (x < rect.start.x || x > rect.end.x)
        throw out_of_range(QObject::tr("x coordinate is out of range.").toStdString());
    if (y < rect.start.y || y > rect.end.y)
        throw out_of_range(QObject::tr("y coordinate is out of range.").toStdString());

    Agros2D::problem()->configView()->particleStart = Point(x, y);
    Agros2D::scene()->invalidate();
}

void PyParticleTracing::setInitialVelocity(double x, double y)
{
    Agros2D::problem()->configView()->particleStartVelocity = Point(x, y);
    Agros2D::scene()->invalidate();
}

void PyParticleTracing::setParticleMass(double mass)
{
    if (mass <= 0.0)
        throw out_of_range(QObject::tr("Mass must be positive.").toStdString());

    Agros2D::problem()->configView()->particleMass = mass;
    Agros2D::scene()->invalidate();
}

void PyParticleTracing::setParticleCharge(double charge)
{
    Agros2D::problem()->configView()->particleConstant = charge;
    Agros2D::scene()->invalidate();
}

void PyParticleTracing::setDragForceDensity(double rho)
{
    if (rho < 0.0)
        throw out_of_range(QObject::tr("Density cannot be negative.").toStdString());

    Agros2D::problem()->configView()->particleDragDensity = rho;
    Agros2D::scene()->invalidate();
}

void PyParticleTracing::setDragForceReferenceArea(double area)
{
    if (area < 0.0)
        throw out_of_range(QObject::tr("Area cannot be negative.").toStdString());

    Agros2D::problem()->configView()->particleDragReferenceArea = area;
    Agros2D::scene()->invalidate();
}

void PyParticleTracing::setDragForceCoefficient(double coeff)
{
    if (coeff < 0.0)
        throw out_of_range(QObject::tr("Coefficient cannot be negative.").toStdString());

    Agros2D::problem()->configView()->particleDragCoefficient = coeff;
    Agros2D::scene()->invalidate();
}

void PyParticleTracing::setIncludeRelativisticCorrection(int include)
{
    Agros2D::problem()->configView()->particleIncludeRelativisticCorrection = include;
    Agros2D::scene()->invalidate();
}

void PyParticleTracing::setReflectOnDifferentMaterial(int reflect)
{
    Agros2D::problem()->configView()->particleReflectOnDifferentMaterial = reflect;
    Agros2D::scene()->invalidate();
}

void PyParticleTracing::setReflectOnBoundary(int reflect)
{
    Agros2D::problem()->configView()->particleReflectOnBoundary = reflect;
    Agros2D::scene()->invalidate();
}

void PyParticleTracing::setCoefficientOfRestitution(double coeff)
{
    if (coeff < 0.0)
        throw out_of_range(QObject::tr("Coefficient of restitution must be between 0 (collide inelastically) and 1 (collide elastically).").toStdString());

    Agros2D::problem()->configView()->particleCoefficientOfRestitution = coeff;
    Agros2D::scene()->invalidate();
}

void PyParticleTracing::setMaximumTolerance(double tolerance)
{
    if (tolerance < 0.0)
        throw out_of_range(QObject::tr("Tolerance cannot be negative.").toStdString());

    Agros2D::problem()->configView()->particleMaximumRelativeError = tolerance;
    Agros2D::scene()->invalidate();
}

void PyParticleTracing::setMaximumNumberOfSteps(int steps)
{
    if (steps < 0.0)
        throw out_of_range(QObject::tr("Maximum number of steps cannot be negative.").toStdString());

    Agros2D::problem()->configView()->particleMaximumNumberOfSteps = steps;
    Agros2D::scene()->invalidate();
}

void PyParticleTracing::setMinimumStep(int step)
{
    if (step < 0.0)
        throw out_of_range(QObject::tr("Minimum step cannot be negative.").toStdString());

    Agros2D::problem()->configView()->particleMinimumStep = step;
    Agros2D::scene()->invalidate();
}
