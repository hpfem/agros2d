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

#ifndef SCENEVIEW_POST_H
#define SCENEVIEW_POST_H

#include "util.h"
#include "sceneview_common.h"

template <typename Scalar> class SceneSolution;

class PostHermes : public QObject
{
    Q_OBJECT

public:
    PostHermes();
    ~PostHermes();

    // mesh
    inline bool initialMeshIsPrepared() { return m_initialMeshIsPrepared; }
    inline Hermes::Hermes2D::Views::Linearizer &linInitialMeshView() { return m_linInitialMeshView; }
    inline bool solutionMeshIsPrepared() { return m_solutionMeshIsPrepared; }
    inline Hermes::Hermes2D::Views::Linearizer &linSolutionMeshView() { return m_linSolutionMeshView; }

    // order view
    inline bool orderIsPrepared() { return m_orderIsPrepared; }
    Hermes::Hermes2D::Views::Orderizer &ordView() { return m_orderView; }

    // contour
    inline bool contourIsPrepared() { return m_contourIsPrepared; }
    inline Hermes::Hermes2D::Views::Linearizer &linContourView() { return m_linContourView; }

    // scalar view
    inline bool scalarIsPrepared() { return m_scalarIsPrepared; }
    inline Hermes::Hermes2D::Views::Linearizer &linScalarView() { return m_linScalarView; }

    // vector view
    inline bool vectorIsPrepared() { return m_vectorIsPrepared; }
    inline Hermes::Hermes2D::Views::Vectorizer &vecVectorView() { return m_vecVectorView; }

    // particle tracing
    inline bool particleTracingIsPrepared() { return m_particleTracingIsPrepared; }
    inline QList<QList<Point3> > particleTracingPositionsList() const { return m_particleTracingPositionsList; }
    inline QList<QList<Point3> > particleTracingVelocitiesList() const { return m_particleTracingVelocitiesList; }
    inline double particleTracingPositionMin() { return m_particleTracingPositionMin; }
    inline double particleTracingPositionMax() { return m_particleTracingPositionMax; }
    inline double particleTracingVelocityMin() { return m_particleTracingVelocityMin; }
    inline double particleTracingVelocityMax() { return m_particleTracingVelocityMax; }

signals:
    void processed();

public slots:
    void refresh();
    void clear();

private:
    // initial mesh
    bool m_initialMeshIsPrepared;
    Hermes::Hermes2D::Views::Linearizer m_linInitialMeshView;

    // solution mesh
    bool m_solutionMeshIsPrepared;
    Hermes::Hermes2D::Views::Linearizer m_linSolutionMeshView;

    // order view
    bool m_orderIsPrepared;
    Hermes::Hermes2D::Views::Orderizer m_orderView;

    // contour
    bool m_contourIsPrepared;
    Hermes::Hermes2D::Views::Linearizer m_linContourView;

    // scalar view
    bool m_scalarIsPrepared;
    Hermes::Hermes2D::Views::Linearizer m_linScalarView; // linealizer for scalar view

    // vector view
    bool m_vectorIsPrepared;
    Hermes::Hermes2D::Views::Vectorizer m_vecVectorView; // vectorizer for vector view

    // particle tracing
    bool m_particleTracingIsPrepared;
    QList<QList<Point3> > m_particleTracingPositionsList;
    QList<QList<Point3> > m_particleTracingVelocitiesList;
    double m_particleTracingPositionMin;
    double m_particleTracingPositionMax;
    double m_particleTracingVelocityMin;
    double m_particleTracingVelocityMax;


private slots:
    void processMeshed();
    void processSolved();

    void processInitialMesh();
    void processSolutionMesh();
    void processOrder();

    void processRangeContour();
    void processRangeScalar();
    void processRangeVector();
    void processParticleTracing();
};

class SceneViewPostInterface : public SceneViewCommon
{
    Q_OBJECT

public:
    SceneViewPostInterface(PostHermes *postHermes, QWidget *parent = 0);

    inline PostHermes *postHermes() { assert(m_postHermes); return m_postHermes; }

protected:
   // SceneSolution<double> *m_sceneSolution;

    double m_texScale;
    double m_texShift;

    PostHermes *m_postHermes;

    virtual int textureScalar() { return -1; }

    void paintScalarFieldColorBar(double min, double max);
    void paintParticleTracingColorBar(double min, double max);

    // palette
    const double *paletteColor(double x) const;
    const double *paletteColorOrder(int n) const;
    void paletteCreate(int texture);
    void paletteFilter(int texture);
    void paletteUpdateTexAdjust();

public slots:
    void timeStepChanged(bool showViewProgress = false);

protected slots:
    virtual void clearGLLists() {}
};

#endif // SCENEVIEW_POST_H
