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

#ifndef PARTICLETRACING_H
#define PARTICLETRACING_H

#include "util.h"
#include "util/enums.h"
#include "util/point.h"
#include "util/global.h"
#include "util/conf.h"

class FieldInfo;
class SceneMaterial;
class MeshHash;

class ParticleTracing : public QObject
{
    Q_OBJECT

public:
    ParticleTracing(QObject *parent=0);
    ~ParticleTracing();

    void clear();

    void computeTrajectoryParticle(bool randomPoint);

    inline QList<Point3> positions() const { return m_positionsList; }
    inline QList<Point3> velocities() const { return m_velocitiesList; }
    inline QList<double> times() const { return m_timesList; }

    inline double velocityMin() const { return m_velocityMin; }
    inline double velocityMax() const { return m_velocityMax; }

private:
    QList<Point3> m_positionsList;
    QList<Point3> m_velocitiesList;
    QList<double> m_timesList;

    double m_velocityMin;
    double m_velocityMax;

    struct MeshCache
    {
        MeshCache() : timeStep(0), adaptivityStep(0), solutionMode(SolutionMode_Normal), mesh(NULL), meshHashes(NULL) {}
        MeshCache(int timeStep, int adaptivityStep, SolutionMode solutionMode, const Hermes::Hermes2D::Mesh *mesh, MeshHash *meshHashes)
            : timeStep(timeStep), adaptivityStep(adaptivityStep), solutionMode(solutionMode), mesh(mesh), meshHashes(meshHashes)
        {}

        ~MeshCache()
        {
            if (meshHashes)
                delete meshHashes;
        }

        int timeStep;
        int adaptivityStep;
        SolutionMode solutionMode;

        const Hermes::Hermes2D::Mesh *mesh;
        MeshHash *meshHashes;
    };

    QMap<FieldInfo *, MeshCache *> m_meshCache;
    QMap<FieldInfo *, Hermes::Hermes2D::Element *> m_activeElement;

    bool newtonEquations(double step,
                         Point3 position,
                         Point3 velocity,
                         Point3 *newposition,
                         Point3 *newvelocity);

    int num_lookups;
    int num_fails;
};



#endif /* PARTICLETRACING_H */
