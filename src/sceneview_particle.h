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

#ifndef SCENEVIEWPARTICLETRACING_H
#define SCENEVIEWPARTICLETRACING_H

#include "util.h"
#include "sceneview_common3d.h"

template <typename Scalar> class SceneSolution;
template <typename Scalar> class ViewScalarFilter;

class SceneViewParticleTracing;
class LineEditDouble;

class ParticleTracingWidget : public QWidget
{
    Q_OBJECT

public slots:
    void refresh();
    void updateControls();

public:
    ParticleTracingWidget(SceneViewParticleTracing *sceneView, QWidget *parent = 0);
    ~ParticleTracingWidget();

private:
    SceneViewParticleTracing *m_sceneViewParticleTracing;

    // particle tracing
    QCheckBox *chkParticleIncludeRelativisticCorrection;
    QSpinBox *txtParticleNumberOfParticles;
    LineEditDouble *txtParticleStartingRadius;
    LineEditDouble *txtParticleMass;
    LineEditDouble *txtParticleConstant;
    LineEditDouble *txtParticlePointX;
    LineEditDouble *txtParticlePointY;
    QLabel *lblParticlePointX;
    QLabel *lblParticlePointY;
    QLabel *lblParticleVelocityX;
    QLabel *lblParticleVelocityY;
    LineEditDouble *txtParticleVelocityX;
    LineEditDouble *txtParticleVelocityY;
    QCheckBox *chkParticleReflectOnDifferentMaterial;
    QCheckBox *chkParticleReflectOnBoundary;
    LineEditDouble *txtParticleCoefficientOfRestitution;
    QLabel *lblParticleMotionEquations;
    QLabel *lblParticleCustomForceX;
    QLabel *lblParticleCustomForceY;
    QLabel *lblParticleCustomForceZ;
    LineEditDouble *txtParticleCustomForceX;
    LineEditDouble *txtParticleCustomForceY;
    LineEditDouble *txtParticleCustomForceZ;
    QCheckBox *chkParticleColorByVelocity;
    QCheckBox *chkParticleShowPoints;
    LineEditDouble *txtParticleMaximumRelativeError;
    LineEditDouble *txtParticleMinimumStep;
    QSpinBox *txtParticleMaximumNumberOfSteps;
    LineEditDouble *txtParticleDragDensity;
    LineEditDouble *txtParticleDragCoefficient;
    LineEditDouble *txtParticleDragReferenceArea;

    void createControls();

private slots:
    void doApply();
    void doParticleDefault();
};

class SceneViewParticleTracing : public SceneViewCommon3D
{
    Q_OBJECT

public slots:
    virtual void clear();
    void processParticleTracing();

public:
    SceneViewParticleTracing(PostHermes *postHermes, QWidget *parent = 0);
    ~SceneViewParticleTracing();

    QAction *actSceneModeParticleTracing;

    virtual QIcon iconView() { return icon("scene-particle"); }
    virtual QString labelView() { return tr("Particle Tracing"); }

protected:
    virtual void mousePressEvent(QMouseEvent *event);

    virtual void paintGL();
    virtual void resizeGL(int w, int h);

    void paintGeometryOutline();
    void paintGeometrySurface(bool blend = false);
    void paintParticleTracing();
    void paintParticleTracingColorBar(double min, double max);

private:
    int m_listParticleTracing;

    void createActionsParticleTracing();

    // particle tracing
    ParticleTracing *particleTracing;
    QList<QList<Point3> > m_positionsList;
    QList<QList<Point3> > m_velocitiesList;
    QList<QList<double> > m_timesList;
    double m_positionMin;
    double m_positionMax;
    double m_velocityMin;
    double m_velocityMax;

    inline bool particleTracingIsPrepared() { return !m_positionsList.isEmpty(); }

private slots:
    virtual void refresh();
    virtual void clearGLLists();
    void clearParticleLists();
    void setControls();
};

#endif // SCENEVIEWPARTICLETRACING_H
