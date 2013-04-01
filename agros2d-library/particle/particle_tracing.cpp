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
#include "mesh_hash.h"

ParticleTracing::ParticleTracing(QObject *parent)
    : QObject(parent)
{
    // qDebug() << "Creating mesh hashes...";
    foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        if(!fieldInfo->plugin()->hasForce(fieldInfo))
            continue;

        // use solution on nearest time step, last adaptivity step possible and if exists, reference solution
        int timeStep = Agros2D::solutionStore()->lastTimeStep(fieldInfo, SolutionMode_Normal);
        int adaptivityStep = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo, SolutionMode_Normal, timeStep);
        SolutionMode solutionMode = SolutionMode_Finer;

        FieldSolutionID fsid(fieldInfo, timeStep, adaptivityStep, solutionMode);
        MeshFunctionSharedPtr<double> sln = Agros2D::solutionStore()->multiArray(fsid).solutions().at(0);

        // todo: should not be initial mesh, but the solution mesh (will not work for adaptivity)
        // m_meshHashs[fieldInfo] = new MeshHash(fieldInfo->initialMesh());
        m_meshCache[fieldInfo] = new MeshCache(timeStep, adaptivityStep, solutionMode, sln->get_mesh(), new MeshHash(sln->get_mesh()));
    }
    // qDebug() << "finished";

    num_lookups = 0;
    num_fails = 0;
}

ParticleTracing::~ParticleTracing()
{
    foreach (MeshCache *cache, m_meshCache.values())
        delete cache;
    // qDebug() << QString("Total hash lookups %1, failed %2").arg(num_lookups).arg(num_fails);
}

void ParticleTracing::clear()
{
    // clear lists
    m_positionsList.clear();
    m_velocitiesList.clear();
    m_timesList.clear();

    m_velocityMin =  numeric_limits<double>::max();
    m_velocityMax = -numeric_limits<double>::max();
}

Point3 ParticleTracing::force(Point3 position,
                              Point3 velocity)
{
    Point3 totalFieldForce;
    foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        if(!fieldInfo->plugin()->hasForce(fieldInfo))
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
            // first try this way
            num_lookups++;
            try{
                activeElement = m_meshCache[fieldInfo]->meshHashes->getElement(position.x, position.y);
            }
            catch(...)
            {
                qDebug() << "Unknown exception catched in MeshHash";
                activeElement = NULL;
            }

            if(!activeElement)
            {
                num_fails++;
                // check whole domain
                m_activeElement[fieldInfo] = Hermes::Hermes2D::RefMap::element_on_physical_coordinates(m_meshCache[fieldInfo]->mesh,
                                                                                                       position.x, position.y);
                // store active element
                activeElement = m_activeElement[fieldInfo];
            }

        }

        if (activeElement)
        {
            // find material
            SceneLabel *label = Agros2D::scene()->labels->at(atoi(fieldInfo->initialMesh()->get_element_markers_conversion().get_user_marker(activeElement->marker).marker.c_str()));
            SceneMaterial* material = label->marker(fieldInfo);

            assert(!material->isNone());

            try
            {
                fieldForce = fieldInfo->plugin()->force(fieldInfo, m_meshCache[fieldInfo]->timeStep, m_meshCache[fieldInfo]->adaptivityStep, m_meshCache[fieldInfo]->solutionMode,
                                                        activeElement, material, position, velocity)
                        * Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleConstant).toDouble();
            }
            catch (AgrosException e)
            {
                qDebug() << "Particle Tracing warning: " << e.what();
                return Point3();
            }
        }
        totalFieldForce = totalFieldForce + fieldForce;
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
    Point3 totalForce = totalFieldForce + forceDrag + forceCustom;

    return totalForce;
}

bool ParticleTracing::newtonEquations(double step,
                                      Point3 position,
                                      Point3 velocity,
                                      Point3 *newposition,
                                      Point3 *newvelocity)
{
    // relativistic correction
    double mass = Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMass).toDouble();
    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleIncludeRelativisticCorrection).toBool())
    {
        if (velocity.magnitude() < SPEEDOFLIGHT)
            mass = mass / (sqrt(1.0 - (velocity.magnitude() * velocity.magnitude()) / (SPEEDOFLIGHT * SPEEDOFLIGHT)));
        else
            throw AgrosException(tr("Velocity is greater then speed of light."));
    }

    // Total acceleration
    Point3 totalAccel = force(position, velocity) / mass;

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

void ParticleTracing::computeTrajectoryParticle(const Point3 initialPosition, const Point3 initialVelocity)
{
    Hermes::ButcherTable butcher((Hermes::ButcherTableType) Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleButcherTableType).toInt());
    QVector<Point3> kp(butcher.get_size());
    QVector<Point3> kv(butcher.get_size());

    clear();

    QTime timePart;
    timePart.start();

    // initial position and velocity
    Point3 position = initialPosition;
    Point3 velocity = initialVelocity;

    // position and velocity cache
    m_positionsList.append(position);
    m_velocitiesList.append(velocity);
    m_timesList.append(0);

    RectPoint bound = Agros2D::scene()->boundingBox();

    double minStep = (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMinimumStep).toDouble() > 0.0)
            ? Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMinimumStep).toDouble() :
              min(bound.width(), bound.height()) / 80.0;
    double relErrorMin = (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMaximumRelativeError).toDouble() > 0.0)
            ? Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMaximumRelativeError).toDouble() / 100 : 1e-6;
    double relErrorMax = 1e-3;
    double dt = velocity.magnitude() > 0
            ? qMax(bound.width(), bound.height()) / velocity.magnitude() / 10 : 1e-11;

    bool stopComputation = false;
    int maxStepsGlobal = 0;
    while (!stopComputation && (maxStepsGlobal < Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMaximumNumberOfSteps).toInt() - 1))
    {
        maxStepsGlobal++;

        // Runge-Kutta steps
        Point3 newPositionH;
        Point3 newVelocityH;

        int maxStepsRKF = 0;
        while (!stopComputation && maxStepsRKF < 100)
        {
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

                if (!newtonEquations(dt, pos, vel, &kp[k], &kv[k]))
                    stopComputation = true;
            }

            Point3 newPositionL = position;
            Point3 newVelocityL = velocity;
            for (int k = 0; k < butcher.get_size() - 1; k++)
            {
                newPositionL = newPositionL + kp[k] * butcher.get_B2(k);
                newVelocityL = newVelocityL + kv[k] * butcher.get_B2(k);
            }

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
            double currentStepLength = (position - newPositionH).magnitude();
            double currentStepVelocity = (velocity - newVelocityH).magnitude();

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
                // qDebug() << "decreased" << dt;
                continue;
            }
            // relative tolerance
            else if ((relError < relErrorMin || relError < EPS_ZERO))
            {
                // increase step
                dt *= 1.1;
                // qDebug() << "increased" << dt;
            }
            break;
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

            if(impact)
            {
                newPositionH.x = intersect.x;
                newPositionH.y = intersect.y;

                stopComputation = true;
            }
            else
            {
                // input vector moved to the origin
                Point vectin = Point(newPositionH.x, newPositionH.y) - intersect;

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
                        / (Point(newPositionH.x, newPositionH.y) - Point(position.x, position.y)).magnitude();

                // output point
                // newPosition.x = intersect.x + (((tangent.x * tangent.x) - (tangent.y * tangent.y)) * vectin.x + 2.0*tangent.x*tangent.y * vectin.y);
                // newPosition.y = intersect.y + (2.0*tangent.x*tangent.y * vectin.x + ((tangent.y * tangent.y) - (tangent.x * tangent.x)) * vectin.y);
                newPositionH.x = intersect.x;
                newPositionH.y = intersect.y;

                // output vector
                Point vectout = (idealReflectedPosition - intersect).normalizePoint();

                // velocity in the direction of output vector
                Point3 oldv = newVelocityH;
                newVelocityH.x = vectout.x * oldv.magnitude() * Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCoefficientOfRestitution).toDouble();
                newVelocityH.y = vectout.y * oldv.magnitude() * Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCoefficientOfRestitution).toDouble();

                // set new timestep
                dt = dt * ratio;

                // qDebug() << "newVelocity: " << newVelocity.toString() << ratio << dt;
            }
        }

        // new values
        velocity = newVelocityH;
        position = newPositionH;

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

    // velocity min and max value
    for (int i = 0; i < m_velocitiesList.length(); i++)
    {
        double velocity = m_velocitiesList[i].magnitude();

        if (velocity < m_velocityMin) m_velocityMin = velocity;
        if (velocity > m_velocityMax) m_velocityMax = velocity;
    }

    // qDebug() << "total: " << timePart.elapsed();
}
