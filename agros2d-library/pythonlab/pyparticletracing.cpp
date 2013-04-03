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

void PyParticleTracing::setNumberOfParticles(int particles)
{
    if (particles < 1)
        throw out_of_range(QObject::tr("Number of particles must be bigger then 1.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleNumberOfParticles, particles);
}

void PyParticleTracing::setStartingRadius(double radius)
{
    if (radius < 0.00)
        throw out_of_range(QObject::tr("Particles dispersion must be possitive.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleStartingRadius, radius);
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
        throw out_of_range(QObject::tr("The x coordinate is out of range.").toStdString());
    if (y < rect.start.y || y > rect.end.y)
        throw out_of_range(QObject::tr("The y coordinate is out of range.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleStartX, x);
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleStartY, y);
}

void PyParticleTracing::setInitialVelocity(double x, double y)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleStartVelocityX, x);
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleStartVelocityY, y);
}

void PyParticleTracing::setParticleMass(double mass)
{
    if (mass <= 0.0)
        throw out_of_range(QObject::tr("Mass must be positive.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleMass, mass);
}

void PyParticleTracing::setParticleCharge(double charge)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleConstant, charge);
}

void PyParticleTracing::setDragForceDensity(double rho)
{
    if (rho < 0.0)
        throw out_of_range(QObject::tr("Density cannot be negative.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleDragDensity, rho);
}

void PyParticleTracing::setDragForceReferenceArea(double area)
{
    if (area < 0.0)
        throw out_of_range(QObject::tr("Area cannot be negative.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleDragReferenceArea, area);
}

void PyParticleTracing::setDragForceCoefficient(double coeff)
{
    if (coeff < 0.0)
        throw out_of_range(QObject::tr("Coefficient cannot be negative.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleDragCoefficient, coeff);
}

void PyParticleTracing::setCustomForce(double x, double y, double z)
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleCustomForceX, x);
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleCustomForceY, y);
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleCustomForceZ, z);
}

void PyParticleTracing::getCustomForce(double &x, double &y, double &z)
{
    x = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCustomForceX).toDouble();
    y = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCustomForceY).toDouble();
    z = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCustomForceZ).toDouble();
}

void PyParticleTracing::setButcherTableType(const std::string &tableType)
{
    if (butcherTableTypeStringKeys().contains(QString::fromStdString(tableType)))
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleButcherTableType, butcherTableTypeFromStringKey(QString::fromStdString(tableType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(butcherTableTypeStringKeys())).toStdString());
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
    if (coeff < 0.0)
        throw out_of_range(QObject::tr("Coefficient of restitution must be between 0 (collide inelastically) and 1 (collide elastically).").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleCoefficientOfRestitution, coeff);
}

void PyParticleTracing::setMaximumTolerance(double tolerance)
{
    if (tolerance < 0.0)
        throw out_of_range(QObject::tr("Tolerance cannot be negative.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleMaximumRelativeError, tolerance);
}

void PyParticleTracing::setMaximumNumberOfSteps(int steps)
{
    if (steps < 0.0)
        throw out_of_range(QObject::tr("Maximum number of steps cannot be negative.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleMaximumNumberOfSteps, steps);
}

void PyParticleTracing::setMinimumStep(int step)
{
    if (step < 0.0)
        throw out_of_range(QObject::tr("Minimum step cannot be negative.").toStdString());

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleMinimumStep, step);
}
