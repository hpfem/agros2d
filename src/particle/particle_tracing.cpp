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
}

ParticleTracing::~ParticleTracing()
{
}

void ParticleTracing::clear()
{
    // clear lists
    m_positionsList.clear();
    m_velocitiesList.clear();
    m_timesList.clear();

    m_velocityMin =  numeric_limits<double>::max();
    m_velocityMax = -numeric_limits<double>::max();

    m_materials.clear();
}

bool ParticleTracing::newtonEquations(FieldInfo* fieldInfo,
                                      double step,
                                      Point3 position,
                                      Point3 velocity,
                                      Point3 *newposition,
                                      Point3 *newvelocity)
{
    // Lorentz force
    Point3 forceLorentz;
    try
    {
        forceLorentz = fieldInfo->plugin()->force(fieldInfo, m_materials[fieldInfo], position, velocity)
                * Agros2D::problem()->configView()->particleConstant;
    }
    catch (AgrosException e)
    {
//        Agros2D::log()->printWarning(tr("Particle Tracing"), e.what());
        qDebug() << "Particle Tracing warning: " << e.what();
        return false;
    }

    // custom force
    Point3 forceCustom = Agros2D::problem()->configView()->particleCustomForce;

    // Drag force
    Point3 velocityReal = (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar) ?
                velocity : Point3(velocity.x, velocity.y, position.x * velocity.z);
    Point3 forceDrag;
    if (velocityReal.magnitude() > 0.0)
        forceDrag = velocityReal.normalizePoint() *
                - 0.5 * Agros2D::problem()->configView()->particleDragDensity * velocityReal.magnitude() * velocityReal.magnitude() * Agros2D::problem()->configView()->particleDragCoefficient * Agros2D::problem()->configView()->particleDragReferenceArea;

    // Total force
    Point3 totalForce = forceLorentz + forceDrag + forceCustom;

    // relativistic correction
    double mass = Agros2D::problem()->configView()->particleMass;
    if (Agros2D::problem()->configView()->particleIncludeRelativisticCorrection)
        mass = Agros2D::problem()->configView()->particleMass / (sqrt(1.0 - (velocity.magnitude() * velocity.magnitude()) / (SPEEDOFLIGHT * SPEEDOFLIGHT)));

    // Total acceleration
    Point3 totalAccel = totalForce / mass;

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
        (*newvelocity).z = (totalAccel.z / position.x - 2 / position.x * velocity.x * velocity.z) * step; // alpha
    }

    return true;
}

void ParticleTracing::computeTrajectoryParticle(bool randomPoint)
{
    clear();

    QTime timePart;
    timePart.start();

    // initial position
    Point3 position;
    position.x = Agros2D::problem()->configView()->particleStart.x;
    position.y = Agros2D::problem()->configView()->particleStart.y;

    // random point
    if (randomPoint)
    {
        int trials = 0;
        while (true)
        {
            Point3 dp(rand() * (Agros2D::problem()->configView()->particleStartingRadius) / RAND_MAX,
                      rand() * (Agros2D::problem()->configView()->particleStartingRadius) / RAND_MAX,
                      (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar) ? 0.0 : rand() * 2.0*M_PI / RAND_MAX);

            position = Point3(-Agros2D::problem()->configView()->particleStartingRadius / 2,
                              -Agros2D::problem()->configView()->particleStartingRadius / 2,
                              (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar) ? 0.0 : -1.0*M_PI) + position + dp;

            Hermes::Hermes2D::Element *e = Hermes::Hermes2D::RefMap::element_on_physical_coordinates(Agros2D::scene()->activeViewField()->initialMesh(),
                                                                                                     position.x, position.y);
            trials++;
            if (e || trials > 10)
                break;
        }
    }

    // find materials
    foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        // check domain
        Hermes::Hermes2D::Element *element = Hermes::Hermes2D::RefMap::element_on_physical_coordinates(fieldInfo->initialMesh(),
                                                                                                       position.x, position.y);
        if (!element)
        {
            throw (AgrosException(tr("Element [%1, %2] lies out of the mesh ('%3').").arg(position.x).arg(position.x).arg(fieldInfo->name())));
            return;
        }

        // find material
        SceneLabel *label = Agros2D::scene()->labels->at(atoi(fieldInfo->initialMesh()->get_element_markers_conversion().get_user_marker(element->marker).marker.c_str()));
        m_materials[fieldInfo] = label->marker(fieldInfo);
    }

    // initial velocity
    Point3 velocity;
    velocity.x = Agros2D::problem()->configView()->particleStartVelocity.x;
    velocity.y = Agros2D::problem()->configView()->particleStartVelocity.y;

    // position and velocity cache
    m_positionsList.append(position);
    m_velocitiesList.append(velocity);
    m_timesList.append(0);

    RectPoint bound = Agros2D::scene()->boundingBox();

    double minStep = (Agros2D::problem()->configView()->particleMinimumStep > 0.0) ? Agros2D::problem()->configView()->particleMinimumStep : min(bound.width(), bound.height()) / 80.0;
    double relErrorMin = (Agros2D::problem()->configView()->particleMaximumRelativeError > 0.0) ? Agros2D::problem()->configView()->particleMaximumRelativeError/100 : 1e-6;
    double relErrorMax = 1e-3;
    double dt = Agros2D::problem()->configView()->particleStartVelocity.magnitude() > 0 ? qMax(bound.width(), bound.height()) / Agros2D::problem()->configView()->particleStartVelocity.magnitude() / 10
                                                                                        : 1e-11;

    // QTime time;
    // time.start();

    bool stopComputation = false;
    int maxStepsGlobal = 0;
    while (!stopComputation && (maxStepsGlobal < Agros2D::problem()->configView()->particleMaximumNumberOfSteps - 1))
    {
        foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
        {
            maxStepsGlobal++;

            // Runge-Kutta steps
            Point3 newPosition;
            Point3 newVelocity;

            int maxStepsRKF = 0;
            while (!stopComputation && maxStepsRKF < 100)
            {
                // Runge-Kutta-Fehlberg adaptive method
                Point3 k1np;
                Point3 k1nv;
                if (!newtonEquations(fieldInfo,
                                     dt,
                                     position,
                                     velocity,
                                     &k1np, &k1nv))
                    stopComputation = true;

                Point3 k2np;
                Point3 k2nv;
                if (!newtonEquations(fieldInfo,
                                     dt,
                                     position + k1np * 1./4.,
                                     velocity + k1nv * 1./4.,
                                     &k2np, &k2nv))
                    stopComputation = true;

                Point3 k3np;
                Point3 k3nv;
                if (!newtonEquations(fieldInfo,
                                     dt,
                                     position + k1np * 3./32. + k2np * 9./32.,
                                     velocity + k1nv * 3./32. + k2nv * 9./32.,
                                     &k3np, &k3nv))
                    stopComputation = true;

                Point3 k4np;
                Point3 k4nv;
                if (!newtonEquations(fieldInfo,
                                     dt,
                                     position + k1np * 1932./2197. - k2np * 7200./2197. + k3np * 7296./2197.,
                                     velocity + k1nv * 1932./2197. - k2nv * 7200./2197. + k3nv * 7296./2197.,
                                     &k4np, &k4nv))
                    stopComputation = true;

                Point3 k5np;
                Point3 k5nv;
                if (!newtonEquations(fieldInfo,
                                     dt,
                                     position + k1np * 439./216. - k2np * 8. + k3np * 3680./513. - k4np * 845./4104.,
                                     velocity + k1nv * 439./216. - k2nv * 8. + k3nv * 3680./513. - k4nv * 845./4104.,
                                     &k5np, &k5nv))
                    stopComputation = true;

                Point3 k6np;
                Point3 k6nv;
                if (!newtonEquations(fieldInfo,
                                     dt,
                                     position - k1np * 8./27. + k2np * 2. - k3np * 3544./2565. + k4np * 1859./4104. - k5np * 11./40.,
                                     velocity - k1nv * 8./27. + k2nv * 2. - k3nv * 3544./2565. + k4nv * 1859./4104. - k5nv * 11./40.,
                                     &k6np, &k6nv))
                    stopComputation = true;

                // Runge-Kutta order 4
                Point3 np4 = position + k1np * 25./216. + k3np * 1408./2565. + k4np * 2197./4104. - k5np * 1./5.;
                // Point3 nv4 = v + k1nv * 25/216 + k3nv * 1408/2565 + k4nv * 2197/4104 - k5nv * 1/5;

                // Runge-Kutta order 5
                newPosition = position + k1np * 16./135. + k3np * 6656./12825. + k4np * 28561./56430. - k5np * 9./50. + k6np * 2./55.;
                newVelocity = velocity + k1nv * 16./135. + k3nv * 6656./12825. + k4nv * 28561./56430. - k5nv * 9./50. + k6nv * 2./55.;

                // optimal step estimation
                double absError = abs(newPosition.magnitude() - np4.magnitude());
                double relError = abs(absError / newPosition.magnitude());
                double currentStepLength = (position - newPosition).magnitude();
                double currentStepVelocity = (velocity - newVelocity).magnitude();

                // qDebug() << np5.toString();
                // qDebug() << "abs. error: " << absError << ", rel. error: " << relError << ", time step: " << dt << "current step: " << currentStep;

                // zero step
                if (currentStepLength < EPS_ZERO && currentStepVelocity < EPS_ZERO)
                {
                    stopComputation = true;
                    break;
                }

                // minimum step
                if ((currentStepLength > minStep) || (relError > relErrorMax))
                {
                    // decrease step
                    dt /= 3.0;
                    continue;
                }
                // relative tolerance
                else if ((relError < relErrorMin || relError < EPS_ZERO))
                {
                    // increase step
                    dt *= 2.0;
                }
                break;
            }

            // check crossing
            QMap<SceneEdge *, Point> intersections;
            foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
            {
                QList<Point> incts = intersection(Point(position.x, position.y), Point(newPosition.x, newPosition.y),
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
                if ((Agros2D::problem()->configView()->particleCoefficientOfRestitution < EPS_ZERO) || // no reflection
                        (crossingEdge->marker(fieldInfo) == Agros2D::scene()->boundaries->getNone(fieldInfo)
                         && !Agros2D::problem()->configView()->particleReflectOnDifferentMaterial) || // inner edge
                        (crossingEdge->marker(fieldInfo) != Agros2D::scene()->boundaries->getNone(fieldInfo)
                         && !Agros2D::problem()->configView()->particleReflectOnBoundary)) // boundary
                {
                    newPosition.x = intersect.x;
                    newPosition.y = intersect.y;

                    stopComputation = true;
                }
                else
                {
                    // input vector moved to the origin
                    Point vectin = Point(newPosition.x, newPosition.y) - intersect;

                    // tangent vector
                    Point tangent;
                    if (crossingEdge->angle() > 0)
                        tangent = (Point( (intersect.y - crossingEdge->center().y),
                                          -(intersect.x - crossingEdge->center().x))).normalizePoint();
                    else
                        tangent = (crossingEdge->nodeStart()->point() - crossingEdge->nodeEnd()->point()).normalizePoint();

                    Point idealReflectedPosition(intersect.x + (((tangent.x * tangent.x) - (tangent.y * tangent.y)) * vectin.x + 2.0*tangent.x*tangent.y * vectin.y),
                                                 intersect.y + (2.0*tangent.x*tangent.y * vectin.x + ((tangent.y * tangent.y) - (tangent.x * tangent.x)) * vectin.y));

                    double ratio = (Point(position.x, position.y) - intersect).magnitude()
                            / (Point(newPosition.x, newPosition.y) - Point(position.x, position.y)).magnitude();

                    // output point
                    // newPosition.x = intersect.x + (((tangent.x * tangent.x) - (tangent.y * tangent.y)) * vectin.x + 2.0*tangent.x*tangent.y * vectin.y);
                    // newPosition.y = intersect.y + (2.0*tangent.x*tangent.y * vectin.x + ((tangent.y * tangent.y) - (tangent.x * tangent.x)) * vectin.y);
                    newPosition.x = intersect.x;
                    newPosition.y = intersect.y;

                    // output vector
                    Point vectout = (idealReflectedPosition - intersect).normalizePoint();

                    // velocity in the direction of output vector
                    Point3 oldv = newVelocity;
                    newVelocity.x = vectout.x * oldv.magnitude() * Agros2D::problem()->configView()->particleCoefficientOfRestitution;
                    newVelocity.y = vectout.y * oldv.magnitude() * Agros2D::problem()->configView()->particleCoefficientOfRestitution;

                    // set new timestep
                    dt = dt * ratio;

                    // qDebug() << "newVelocity: " << newVelocity.toString() << ratio << dt;
                }
            }

            // new values
            velocity = newVelocity;
            position = newPosition;

            // add to the lists
            m_timesList.append(m_timesList.last() + dt);
            m_positionsList.append(position);

            if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
                m_velocitiesList.append(velocity);
            else
                m_velocitiesList.append(Point3(velocity.x, velocity.y, position.x * velocity.z)); // v_phi = omega * r

            if (stopComputation)
                break;
        }
    }

    // velocity min and max value
    for (int i = 0; i < m_velocitiesList.length(); i++)
    {
        double velocity = m_velocitiesList[i].magnitude();

        if (velocity < m_velocityMin) m_velocityMin = velocity;
        if (velocity > m_velocityMax) m_velocityMax = velocity;
    }

    // qDebug() << "steps: " << steps << "total: " << timePart.elapsed();
}
