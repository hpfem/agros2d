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
    void getInitialPosition(vector<double> &position) const;
    void setInitialPosition(const vector<double> &position);

    // initial velocity
    void getInitialVelocity(vector<double> &velocity) const;
    void setInitialVelocity(const vector<double> &velocity);

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

    // drag force density
    inline double getDragForceDensity() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragDensity).toDouble(); }
    void setDragForceDensity(double density);

    // drag force reference area
    inline double getDragForceReferenceArea() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragReferenceArea).toDouble(); }
    void setDragForceReferenceArea(double area);

    // drag force coefficient
    inline double getDragForceCoefficient() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragCoefficient).toDouble(); }
    void setDragForceCoefficient(double coeff);

    // custom force
    void getCustomForce(vector<double> &force) const;
    void setCustomForce(const vector<double> &force);

    // butcher table
    std::string getButcherTableType() const
    {
        return butcherTableTypeToStringKey((Hermes::ButcherTableType) Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleButcherTableType).toInt()).toStdString();
    }
    void setButcherTableType(const std::string &tableType);

    // relativistic correction
    inline bool getIncludeRelativisticCorrection() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleIncludeRelativisticCorrection).toBool(); }
    void setIncludeRelativisticCorrection(bool include) { Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleIncludeRelativisticCorrection, include); }

    // get maximum relative error
    inline double getMaximumRelativeError() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMaximumRelativeError).toDouble(); }
    void setMaximumRelativeError(double error);

    // maximum number of steps
    inline int getMaximumNumberOfSteps() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMaximumNumberOfSteps).toInt(); }
    void setMaximumNumberOfSteps(int steps);

    // minimum step
    inline int getMinimumStep() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleMinimumStep).toInt(); }
    void setMinimumStep(int step);

    // reflection on different material
    inline bool getReflectOnDifferentMaterial() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleReflectOnDifferentMaterial).toBool(); }
    void setReflectOnDifferentMaterial(bool reflect) { Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleReflectOnDifferentMaterial, reflect); }

    // reflection on boundary
    inline bool getReflectOnBoundary() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleReflectOnBoundary).toBool(); }
    void setReflectOnBoundary(bool reflect) { Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleReflectOnBoundary, reflect); }

    // coefficient of restitution
    inline double getCoefficientOfRestitution() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCoefficientOfRestitution).toDouble(); }
    void setCoefficientOfRestitution(double coeff);

    // collor by velocity
    inline bool getColorByVelocity() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleColorByVelocity).toBool(); }
    void setColorByVelocity(bool show) { Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleColorByVelocity, show); }

    // show points
    inline bool getShowPoints() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleShowPoints).toBool(); }
    void setShowPoints(bool show) { Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleShowPoints, show); }

    // blended faces
    inline bool getShowBlendedFaces() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleShowBlendedFaces).toBool(); }
    void setShowBlendedFaces(bool show) { Agros2D::problem()->setting()->setValue(ProblemSetting::View_ParticleShowBlendedFaces, show); }

    // multiple show particles
    inline int getNumShowParticlesAxi() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleNumShowParticlesAxi).toInt(); }
    void setNumShowParticlesAxi(int particles);

    // solve
    void solve();
    void positions(vector<double> &x, vector<double> &y, vector<double> &z) const;
    void velocities(vector<double> &x, vector<double> &y, vector<double> &z) const;
    void times(vector<double> &time) const;
    inline int length() const { return m_positions.length(); }

private:
    // position and velocity
    QList<Point3> m_positions;
    QList<Point3> m_velocities;
    QList<double> m_times;
};

#endif // PYTHONLABPARTICLETRACING_H
