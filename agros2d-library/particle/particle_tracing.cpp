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

#include "particle_tracing.h"

#include "util.h"
#include "util/xml.h"
#include "util/constants.h"

#include "hermes2d/problem.h"
#include "hermes2d/plugin_interface.h"

#include "util.h"
#include "value.h"
#include "logview.h"
#include "scene.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "hermes2d/field.h"
#include "hermes2d/solutionstore.h"
#include "hermes2d/problem_config.h"

ParticleTracing::ParticleTracing(QObject *parent)
    : QObject(parent)
{
    foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        if(!fieldInfo->plugin()->hasForce(fieldInfo))
            continue;

        // use solution on nearest time step, last adaptivity step possible and if exists, reference solution
        int timeStep = Agros2D::solutionStore()->lastTimeStep(fieldInfo, SolutionMode_Normal);
        int adaptivityStep = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo, SolutionMode_Normal, timeStep);
        SolutionMode solutionMode = SolutionMode_Finer;

        FieldSolutionID fsid(fieldInfo, timeStep, adaptivityStep, solutionMode);
        Hermes::Hermes2D::MeshFunctionSharedPtr<double> sln = Agros2D::solutionStore()->multiArray(fsid).solutions().at(0);

        m_solutionIDs[fieldInfo] = FieldSolutionID(fieldInfo, timeStep, adaptivityStep, solutionMode);
        m_meshes[fieldInfo] = sln->get_mesh();
    }
}

ParticleTracing::~ParticleTracing()
{
}

void ParticleTracing::clear()
{
    // clear lists
    for (int i = 0; m_positionsList.size(); i++)
        m_positionsList[i].clear();
    m_positionsList.clear();
    for (int i = 0; m_velocitiesList.size(); i++)
        m_velocitiesList[i].clear();
    m_velocitiesList.clear();
    for (int i = 0; m_timesList.size(); i++)
        m_timesList[i].clear();
    m_timesList.clear();

    m_velocityMin =  numeric_limits<double>::max();
    m_velocityMax = -numeric_limits<double>::max();
}

// input position, velocity: planar x, y, z, axi r, z, phi
// ouput x, y, z
Point3 ParticleTracing::force(int particleIndex,
                              Point3 position,
                              Point3 velocity)
{
    Point3 totalFieldForce;
    foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        if (!fieldInfo->plugin()->hasForce(fieldInfo))
            continue;

        Point3 fieldForce;

        bool elementIsValid = false;
        Hermes::Hermes2D::Element *activeElement = NULL;

        // active element for current field
        if (m_activeElement.contains(fieldInfo))
            activeElement = m_activeElement[fieldInfo];

        if (activeElement)
        {
            double x_reference;
            double y_reference;
            elementIsValid = Hermes::Hermes2D::RefMap::is_element_on_physical_coordinates(activeElement,
                                                                                          position.x, position.y, &x_reference, &y_reference);
        }

        if (!elementIsValid)
        {
            m_activeElement[fieldInfo] = Hermes::Hermes2D::RefMap::element_on_physical_coordinates(true, m_meshes[fieldInfo],
                                                                                                   position.x, position.y);
        }

        if (activeElement)
        {
            // find material
            SceneLabel *label = Agros2D::scene()->labels->at(atoi(fieldInfo->initialMesh()->get_element_markers_conversion().get_user_marker(activeElement->marker).marker.c_str()));
            SceneMaterial* material = label->marker(fieldInfo);

            assert(!material->isNone());

            try
            {
                fieldForce = fieldInfo->plugin()->force(fieldInfo, m_solutionIDs[fieldInfo].timeStep, m_solutionIDs[fieldInfo].adaptivityStep, m_solutionIDs[fieldInfo].solutionMode,
                                                        activeElement, material, position, velocity)
                        * m_particleChargesList[particleIndex];
            }
            catch (AgrosException e)
            {
                qDebug() << "Particle Tracing warning: " << e.what();
                return Point3();
            }
        }
        totalFieldForce = totalFieldForce + fieldForce;
    }

    // particle to particle force
    Point3 forceP2PElectric;
    Point3 forceP2PMagnetic;
    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleP2PElectricForce).toBool() ||
            Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleP2PMagneticForce).toBool())
    {
        for (int i = 0; i < m_positionsList.size(); i++)
        {
            if (particleIndex == i)
                continue;

            int timeLevel = timeToLevel(i, m_timesList[particleIndex].last());
            Point3 particlePosition = m_positionsList[i].at(timeLevel);
            Point3 particleVelocity = m_velocitiesList[i].at(timeLevel);

            double distance = 0.0;
            if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
                distance = Point3(position.x - particlePosition.x,
                                  position.y - particlePosition.y,
                                  position.z - particlePosition.z).magnitude();
            else
                distance = Point3(position.x * cos(position.z) - particlePosition.x * cos(particlePosition.z),
                                  position.y - particlePosition.y,
                                  position.x * sin(position.z) - particlePosition.x * sin(particlePosition.z)).magnitude();

            if (distance > 0)
            {
                if (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleP2PElectricForce).toBool())
                {
                    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
                        forceP2PElectric = forceP2PElectric + Point3(
                                    (position.x - particlePosition.x) / distance,
                                    (position.y - particlePosition.y) / distance,
                                    (position.z - particlePosition.z) / distance)
                                * (m_particleChargesList[particleIndex] * m_particleChargesList[i] / (4 * M_PI * EPS0 * distance * distance));
                    else
                        forceP2PElectric = forceP2PElectric + Point3(
                                    (position.x * cos(position.z) - particlePosition.x * cos(particlePosition.z)) / distance,
                                    (position.y - particlePosition.y) / distance,
                                    (position.x * sin(position.z) - particlePosition.x * sin(particlePosition.z)) / distance)
                                * (m_particleChargesList[particleIndex] * m_particleChargesList[i] / (4 * M_PI * EPS0 * distance * distance));
                }
                if (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleP2PMagneticForce).toBool())
                {
                    Point3 r0, v0;
                    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
                    {
                        r0 = Point3((position.x - particlePosition.x) / distance,
                                    (position.y - particlePosition.y) / distance,
                                    (position.z - particlePosition.z) / distance);
                        v0 = Point3(velocity.x - particleVelocity.x,
                                    velocity.y - particleVelocity.y,
                                    velocity.z - particleVelocity.z);
                    }
                    else
                    {
                        r0 = Point3((position.x * cos(position.z) - particlePosition.x * cos(particlePosition.z)) / distance,
                                    (position.y - particlePosition.y) / distance,
                                    (position.x * sin(position.z) - particlePosition.x * sin(particlePosition.z)) / distance);
                        // TODO: fix velocity
                        assert(0);
                        v0 = Point3(velocity.x * cos(position.z) - particleVelocity.x * cos(particlePosition.z),
                                    velocity.y - particleVelocity.y,
                                    velocity.x * sin(position.z) - particleVelocity.x * sin(particlePosition.z));
                    }

                    forceP2PMagnetic = forceP2PMagnetic + (v0 % v0 % r0)
                            * (m_particleChargesList[particleIndex] * m_particleChargesList[i] * MU0 / (4 * M_PI * distance * distance));

                }
            }
        }
    }

    // custom force
    Point3 forceCustom = Point3(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCustomForceX).toDouble(),
                                Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCustomForceY).toDouble(),
                                Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCustomForceZ).toDouble());

    // Drag force
    Point3 velocityReal = (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar) ?
                velocity : Point3(velocity.x, velocity.y, position.x * velocity.z);
    Point3 forceDrag;
    if (velocityReal.magnitude() > 0.0)
        forceDrag = velocityReal.normalizePoint() *
                - 0.5 * Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragDensity).toDouble()
                * velocityReal.magnitude() * velocityReal.magnitude()
                * Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragCoefficient).toDouble()
                * Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragReferenceArea).toDouble();

    // Total force
    Point3 totalForce = totalFieldForce + forceDrag + forceCustom + forceP2PElectric + forceP2PMagnetic;

    return totalForce;
}

bool ParticleTracing::newtonEquations(int particleIndex,
                                      double step,
                                      Point3 position,
                                      Point3 velocity,
                                      Point3 *newposition,
                                      Point3 *newvelocity)
{
    // relativistic correction
    double mass = m_particleMassesList[particleIndex];
    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleIncludeRelativisticCorrection).toBool())
    {
        Point3 velocityReal = (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar) ?
                    velocity : Point3(velocity.x, velocity.y, position.x * velocity.z);

        mass = mass / (sqrt(1.0 - (velocityReal.magnitude() * velocityReal.magnitude()) / (SPEEDOFLIGHT * SPEEDOFLIGHT)));
    }

    // Total acceleration
    Point3 totalAccel = force(particleIndex, position, velocity) / mass;

    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
    {
        // position
        *newposition = velocity * step;

        // velocity
        *newvelocity = totalAccel * step;
    }
    else
    {
        (*newposition).x = velocity.x * step; // r
        (*newposition).y = velocity.y * step; // z
        (*newposition).z = velocity.z * step; // alpha

        (*newvelocity).x = (totalAccel.x + velocity.z * velocity.z * position.x) * step; // r
        (*newvelocity).y = (totalAccel.y) * step; // z
        (*newvelocity).z = (position.x < EPS_ZERO) ? 0 : (totalAccel.z / position.x - 2 / position.x * velocity.x * velocity.z) * step; // alpha
    }

    return true;
}

int ParticleTracing::timeToLevel(int particleIndex, double time)
{
    if (m_timesList[particleIndex].size() == 1)
        return 0;
    else if (time >= m_timesList[particleIndex].last())
        return m_timesList[particleIndex].size() - 1;
    else
        for (int i = 0; i < m_timesList[particleIndex].size() - 1; i++)
            if ((m_timesList[particleIndex].at(i) <= time) && (time <= m_timesList[particleIndex].at(i+1)))
                return i;

    assert(0);
}

void ParticleTracing::computeTrajectoryParticles(const QList<Point3> initialPositions, const QList<Point3> initialVelocities,
                                                 const QList<double> particleCharges, const QList<double> particleMasses)
{
    assert(initialPositions.size() == initialVelocities.size());
    assert(initialPositions.size() == particleCharges.size());
    assert(initialPositions.size() == particleMasses.size());
    assert(initialPositions.size() == Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleNumberOfParticles).toInt());

    m_particleChargesList = particleCharges;
    m_particleMassesList = particleMasses;

    Hermes::ButcherTable butcher((Hermes::ButcherTableType) Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleButcherTableType).toInt());

    clear();

    int numberOfParticles = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleNumberOfParticles).toInt();

    QTime timePart;
    timePart.start();

    RectPoint bound = Agros2D::scene()->boundingBox();

    double minStep = (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMinimumStep).toDouble() > 0.0)
            ? Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMinimumStep).toDouble() :
              min(bound.width(), bound.height()) / 80.0;
    double relErrorMin = (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMaximumRelativeError).toDouble() > 0.0)
            ? Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMaximumRelativeError).toDouble() / 100 : 1e-6;
    double relErrorMax = 1e-3;

    // given velocity
    QList<bool> stopComputation;
    QList<int> numberOfSteps;
    QList<double> timeStep;

    // initial positions
    for (int particleIndex = 0; particleIndex < numberOfParticles; particleIndex++)
    {
        // position and velocity cache
        m_positionsList.append(QList<Point3>());
        m_velocitiesList.append(QList<Point3>());
        m_timesList.append(QList<double>());

        m_positionsList[particleIndex].append(initialPositions[particleIndex]);
        m_velocitiesList[particleIndex].append(initialVelocities[particleIndex]);
        m_timesList[particleIndex].append(0);

        stopComputation.append(false);
        numberOfSteps.append(0);

        // timeStep.append(initialVelocities[particleIndex].magnitude() > 0
        //                 ? qMax(bound.width(), bound.height()) / initialVelocities[particleIndex].magnitude() / 10 : 1e-11);
        timeStep.append(1e-11);
    }
    timeStep[0] = 1.5e-9;

    bool globalStopComputation = false;
    while (!globalStopComputation)
    {
        double syncTime = 0.0;
        int syncParticle = -1;
        for (int particleIndex = 0; particleIndex < numberOfParticles; particleIndex++)
            if (m_timesList[particleIndex].last() > syncTime)
            {
                syncTime = m_timesList[particleIndex].last();
                syncParticle = particleIndex;
            }

        double timeStp = 0.0;
        if (syncParticle == -1)
        for (int particleIndex = 0; particleIndex < numberOfParticles; particleIndex++)
            if (timeStep[particleIndex] > timeStp)
            {
                timeStp = timeStep[particleIndex];
                syncParticle = particleIndex;
            }

        // qDebug() << "synctime" << syncTime << syncParticle;

        for (int particleIndex = 0; particleIndex < numberOfParticles; particleIndex++)
        {
            // qDebug() << "particle time" << particleIndex << m_timesList[particleIndex].last() << numberOfSteps[particleIndex];

            // stop on number of steps
            if (numberOfSteps[particleIndex] > Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMaximumNumberOfSteps).toInt() - 1)
                stopComputation[particleIndex] = true;

            // stop on time steps
            if (timeStep[particleIndex] < EPS_ZERO / 100.0)
                stopComputation[particleIndex] = true;

            if (stopComputation[particleIndex])
                continue;

            // sync
            if (!stopComputation[particleIndex] && particleIndex == syncParticle)
            {
                bool otherParticlesIsRunning = false;
                for (int particleIndexOther = 0; particleIndexOther < numberOfParticles; particleIndexOther++)
                    if (particleIndex != particleIndexOther && !stopComputation[particleIndexOther])
                        otherParticlesIsRunning = true;

                if (otherParticlesIsRunning)
                    continue;
            }

            // increase number of steps
            numberOfSteps[particleIndex]++;

            // initial position and velocity
            Point3 position = m_positionsList[particleIndex].last();
            Point3 velocity = m_velocitiesList[particleIndex].last();
            if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Axisymmetric)
                velocity.z = velocity.z / position.x; // v_phi = omega * r
            double currentTimeStep = timeStep[particleIndex];
            // qDebug() << currentTimeStep;

            // Runge-Kutta steps
            Point3 newPositionH;
            Point3 newVelocityH;

            // Butcher tableu
            QVector<Point3> kp(butcher.get_size());
            QVector<Point3> kv(butcher.get_size());

            int maxStepsRKF = 0;
            while (!stopComputation[particleIndex] && maxStepsRKF < 100)
            {
                bool butcherOK = true;

                Point3 pos = position;
                Point3 vel = velocity;
                for (int k = 0; k < butcher.get_size(); k++)
                {
                    Point3 pos = position;
                    Point3 vel = velocity;

                    for (int l = 0; l < butcher.get_size(); l++)
                    {
                        if (l < k)
                        {
                            pos = pos + kp[l] * butcher.get_A(k, l);
                            vel = vel + kv[l] * butcher.get_A(k, l);
                        }
                    }

                    if ((Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleIncludeRelativisticCorrection).toBool())
                            && ((Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar
                                 ? vel.magnitude() : Point3(vel.x, vel.y, pos.x * vel.z).magnitude()) > SPEEDOFLIGHT))
                    {
                        // decrease time step
                        butcherOK = false;
                        break;
                    }

                    newtonEquations(particleIndex, currentTimeStep, pos, vel, &kp[k], &kv[k]);
                }

                if (butcherOK)
                {
                    // low order
                    Point3 newPositionL = position;
                    Point3 newVelocityL = velocity;
                    for (int k = 0; k < butcher.get_size() - 1; k++)
                    {
                        newPositionL = newPositionL + kp[k] * butcher.get_B2(k);
                        newVelocityL = newVelocityL + kv[k] * butcher.get_B2(k);
                    }

                    // high order
                    newPositionH = position;
                    newVelocityH = velocity;
                    for (int k = 0; k < butcher.get_size(); k++)
                    {
                        newPositionH = newPositionH + kp[k] * butcher.get_B(k);
                        newVelocityH = newVelocityH + kv[k] * butcher.get_B(k);
                    }

                    // optimal step estimation
                    double absError = abs(newPositionH.magnitude() - newPositionL.magnitude());
                    double relError = abs(absError / newPositionH.magnitude());
                    double currentStepLength = ((Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar) ?
                                                    (position - newPositionH).magnitude() :
                                                    (Point3(position.x * cos(position.z), position.x * sin(position.z), position.y)
                                                     - Point3(newPositionH.x * cos(newPositionH.z), newPositionH.x * sin(newPositionH.z), newPositionH.y)).magnitude());
                    double currentStepVelocity = ((Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar) ?
                                                      (velocity - newVelocityH).magnitude() :
                                                      (Point3(velocity.x, velocity.y, position.x * velocity.z) - Point3(newVelocityH.x, newVelocityH.y, newPositionH.x * newVelocityH.z)).magnitude());

                    // nearly zero step
                    if (currentStepLength < EPS_ZERO && currentStepVelocity < EPS_ZERO)
                    {
                        qDebug() << QString("Particle %1: time step is too short - refused.").arg(particleIndex);
                        currentTimeStep *= 2.0;
                        continue;
                    }

                    // minimum step
                    if ((currentStepLength > minStep) || (relError > relErrorMax))
                    {
                        // decrease step
                        qDebug() << QString("Particle %1: time step is too long or relative error was exceeded - refused.").arg(particleIndex);
                        currentTimeStep /= 3.0;
                        continue;
                    }
                    // relative tolerance
                    else if ((relError < relErrorMin || relError < EPS_ZERO))
                    {
                        // increase next step
                        // store current time step
                        // qDebug() << QString("Particle %1: time step increased.").arg(particleIndex);
                        timeStep[particleIndex] = currentTimeStep * 1.1;
                        break;
                    }
                    else
                    {
                        // store current time step
                        timeStep[particleIndex] = currentTimeStep;
                        break;
                    }
                }
                else
                {
                    if (currentTimeStep < EPS_ZERO / 100.0)
                    {
                        // store current time step
                        timeStep[particleIndex] = currentTimeStep;

                        // stop computation
                        break;
                    }
                    else
                    {
                        // decrease step
                        qDebug() << QString("Particle %1: the speed of light was exceeded - refused.").arg(particleIndex);
                        currentTimeStep /= 2.0;
                        continue;
                    }
                }
            }

            // check crossing
            QMap<SceneEdge *, Point> intersections;
            foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
            {
                QList<Point> incts = intersection(Point(position.x, position.y), Point(newPositionH.x, newPositionH.y),
                                                  Point(), 0.0, 0.0,
                                                  edge->nodeStart()->point(), edge->nodeEnd()->point(),
                                                  edge->center(), edge->radius(), edge->angle());

                if (incts.length() > 0)
                    foreach (Point p, incts)
                        intersections.insert(edge, p);
            }

            // find the closest intersection
            Point intersect;
            SceneEdge *crossingEdge = NULL;
            double distance = numeric_limits<double>::max();
            for (QMap<SceneEdge *, Point>::const_iterator it = intersections.begin(); it != intersections.end(); ++it)
                if ((it.value() - Point(position.x, position.y)).magnitude() < distance)
                {
                    distance = (it.value() - Point(position.x, position.y)).magnitude();

                    crossingEdge = it.key();
                    intersect = it.value();
                }

            if (crossingEdge && distance > EPS_ZERO)
            {
                bool impact = false;
                foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
                {
                    if ((Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCoefficientOfRestitution).toDouble() < EPS_ZERO) || // no reflection
                            (crossingEdge->marker(fieldInfo) == Agros2D::scene()->boundaries->getNone(fieldInfo)
                             && !Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleReflectOnDifferentMaterial).toBool()) || // inner edge
                            (crossingEdge->marker(fieldInfo) != Agros2D::scene()->boundaries->getNone(fieldInfo)
                             && !Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleReflectOnBoundary).toBool())) // boundary
                        impact = true;
                }

                // current step ration
                if (impact)
                {
                    newPositionH.x = intersect.x;
                    newPositionH.y = intersect.y;

                    // qDebug() << particleIndex << "impact";
                    stopComputation[particleIndex] = true;
                }
                else
                {
                    // input vector moved to the origin
                    Point vectin = Point(newPositionH.x, newPositionH.y) - intersect;

                    // tangent vector
                    Point tangent;
                    if (crossingEdge->isStraight())
                        tangent = (crossingEdge->nodeStart()->point() - crossingEdge->nodeEnd()->point()).normalizePoint();
                    else
                        tangent = Point((intersect.y - crossingEdge->center().y), -(intersect.x - crossingEdge->center().x)).normalizePoint();

                    Point idealReflectedPosition(intersect.x + (((tangent.x * tangent.x) - (tangent.y * tangent.y)) * vectin.x + 2.0*tangent.x*tangent.y * vectin.y),
                                                 intersect.y + (2.0*tangent.x*tangent.y * vectin.x + ((tangent.y * tangent.y) - (tangent.x * tangent.x)) * vectin.y));

                    double ratio = (Point(position.x, position.y) - intersect).magnitude()
                            / (Point(newPositionH.x, newPositionH.y) - Point(position.x, position.y)).magnitude();

                    // output vector
                    Point vectout = (idealReflectedPosition - intersect).normalizePoint();

                    // stop computation (impact distance is very very small)
                    if ((fabs(distance / 100.0 * vectout.x) < EPS_ZERO) && (fabs(distance / 100.0 * vectout.y) < EPS_ZERO))
                        stopComputation[particleIndex] = true;

                    // output point
                    newPositionH.x = intersect.x + distance / 100.0 * vectout.x;
                    newPositionH.y = intersect.y + distance / 100.0 * vectout.y;

                    // velocity in the direction of output vector
                    Point3 oldv = newVelocityH;
                    newVelocityH.x = vectout.x * Point(oldv.x, oldv.y).magnitude() * Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCoefficientOfRestitution).toDouble();
                    newVelocityH.y = vectout.y * Point(oldv.x, oldv.y).magnitude() * Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCoefficientOfRestitution).toDouble();

                    // set new timestep
                    currentTimeStep = currentTimeStep * ratio;
                    timeStep[particleIndex] = currentTimeStep;
                }
            }

            // new values
            velocity = newVelocityH;
            position = newPositionH;

            // add to the lists
            m_timesList[particleIndex].append(m_timesList[particleIndex].last() + currentTimeStep);
            m_positionsList[particleIndex].append(position);

            // velocities in planar and axisymmetric arrangement
            if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
                m_velocitiesList[particleIndex].append(velocity);
            else
                m_velocitiesList[particleIndex].append(Point3(velocity.x, velocity.y, position.x * velocity.z)); // v_phi = omega * r
        }

        // global stop
        bool stop = true;
        for (int particleIndex = 0; particleIndex < numberOfParticles; particleIndex++)
            stop = stop && stopComputation[particleIndex];
        globalStopComputation = stop;
    }

    // velocity min and max value
    for (int i = 0; i < m_velocitiesList.length(); i++)
    {
        for (int j = 0; j < m_velocitiesList[i].length(); j++)
        {
            double velocity = m_velocitiesList[i][j].magnitude();

            if (velocity < m_velocityMin) m_velocityMin = velocity;
            if (velocity > m_velocityMax) m_velocityMax = velocity;
        }
    }

    //qDebug() << "total particle: " << timePart.elapsed();
}
