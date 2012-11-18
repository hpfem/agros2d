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
    inline bool initialMeshIsPrepared() { return !m_linInitialMeshView.is_empty(); }
    inline Hermes::Hermes2D::Views::Linearizer &linInitialMeshView() { return m_linInitialMeshView; }
    inline bool solutionMeshIsPrepared() { return !m_linSolutionMeshView.is_empty(); }
    inline Hermes::Hermes2D::Views::Linearizer &linSolutionMeshView() { return m_linSolutionMeshView; }

    // order view
    inline bool orderIsPrepared() { return !m_orderView.is_empty(); }
    Hermes::Hermes2D::Views::Orderizer &ordView() { return m_orderView; }

    // contour
    inline bool contourIsPrepared() { return !m_linContourView.is_empty(); }
    inline Hermes::Hermes2D::Views::Linearizer &linContourView() { return m_linContourView; }

    // scalar view
    inline bool scalarIsPrepared() { return !m_linScalarView.is_empty(); }
    inline Hermes::Hermes2D::Views::Linearizer &linScalarView() { return m_linScalarView; }

    // vector view
    inline bool vectorIsPrepared() { return !m_vecVectorView.is_empty(); }
    inline Hermes::Hermes2D::Views::Vectorizer &vecVectorView() { return m_vecVectorView; }

    // particle tracing
    inline bool particleTracingIsPrepared() { return m_particleTracingIsPrepared; }
    inline QList<QList<Point3> > particleTracingPositionsList() const { return m_particleTracingPositionsList; }
    inline QList<QList<Point3> > particleTracingVelocitiesList() const { return m_particleTracingVelocitiesList; }
    inline QList<QList<double> > particleTracingTimesList() const { return m_particleTracingTimesList; }
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
    Hermes::Hermes2D::Views::Linearizer m_linInitialMeshView;

    // solution mesh
    Hermes::Hermes2D::Views::Linearizer m_linSolutionMeshView;

    // order view
    Hermes::Hermes2D::Views::Orderizer m_orderView;

    // contour
    Hermes::Hermes2D::Views::Linearizer m_linContourView;

    // scalar view
    Hermes::Hermes2D::Views::Linearizer m_linScalarView; // linealizer for scalar view

    // vector view
    Hermes::Hermes2D::Views::Vectorizer m_vecVectorView; // vectorizer for vector view

    // particle tracing
    bool m_particleTracingIsPrepared;
    QList<QList<Point3> > m_particleTracingPositionsList;
    QList<QList<Point3> > m_particleTracingVelocitiesList;
    QList<QList<double> > m_particleTracingTimesList;
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

    virtual void clearGLLists() {}
};

class SceneViewPostInterface : public SceneViewCommon
{
    Q_OBJECT

public:
    SceneViewPostInterface(PostHermes *postHermes, QWidget *parent = 0);

    inline PostHermes *postHermes() { assert(m_postHermes); return m_postHermes; }

protected:
    double m_texScale;
    double m_texShift;

    GLuint m_textureScalar;

    PostHermes *m_postHermes;

    virtual void initializeGL();

    void paintScalarFieldColorBar(double min, double max);
    void paintParticleTracingColorBar(double min, double max, bool is2D = true);

    // palette
    const double *paletteColor2(const int pos) const;
    const double *paletteColor(double x) const;
    const double *paletteColorOrder(int n) const;
    void paletteCreate();

protected slots:
    virtual void clearGLLists() {}
};

#endif // SCENEVIEW_POST_H
