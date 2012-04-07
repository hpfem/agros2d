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

#ifndef SCENEVIEWPOST2D_H
#define SCENEVIEWPOST2D_H

#include "util.h"
#include "sceneview_mesh.h"

template <typename Scalar> class SceneSolution;
template <typename Scalar> class ViewScalarFilter;

class FieldInfo;

class Post2DHermes : public QObject
{
    Q_OBJECT

public:
    Post2DHermes();
    ~Post2DHermes();

    void clear();

    // contour
    inline bool contourIsPrepared() { return m_contourIsPrepared; }
    inline Hermes::Hermes2D::Views::Linearizer &linContourView() { return m_linContourView; }

    // scalar view
    inline bool scalarIsPrepared() { return m_scalarIsPrepared; }
    inline Hermes::Hermes2D::Views::Linearizer &linScalarView() { return m_linScalarView; }

    // vector view
    inline bool vectorIsPrepared() { return m_vectorIsPrepared; }
    inline Hermes::Hermes2D::Views::Vectorizer &vecVectorView() { return m_vecVectorView; }

signals:
    void processed();

public slots:
    virtual void processSolved();

private:
    // contour
    bool m_contourIsPrepared;
    Hermes::Hermes2D::Views::Linearizer m_linContourView;

    // scalar view
    bool m_scalarIsPrepared;
    Hermes::Hermes2D::Views::Linearizer m_linScalarView; // linealizer for scalar view

    // vector view
    bool m_vectorIsPrepared;
    Hermes::Hermes2D::Views::Vectorizer m_vecVectorView; // vectorizer for vector view

private slots:
    // process
    void processRangeContour();
    void processRangeScalar();
    void processRangeVector();
};

class SceneViewPost2D : public SceneViewCommon2D
{
    Q_OBJECT

public slots:   
    void doSelectMarker();
    void doPostprocessorModeGroup(QAction *action);
    virtual void doInvalidated();
    virtual void clear();

public:
    SceneViewPost2D(QWidget *parent = 0);
    ~SceneViewPost2D();

    QAction *actSceneModePost2D;

    QActionGroup *actShowGroup;
    QAction *actShowContours;
    QAction *actShowVectors;
    QAction *actShowSolutionMesh;

    QAction *actSceneViewSelectByMarker;

    QActionGroup *actPostprocessorModeGroup;
    QAction *actPostprocessorModeLocalPointValue;
    QAction *actPostprocessorModeSurfaceIntegral;
    QAction *actPostprocessorModeVolumeIntegral;

    virtual QIcon iconView() { return icon("scene-post2d"); }
    virtual QString labelView() { return tr("Postprocessor 2D"); }

protected:
    virtual int textureScalar() { return 2; }

    virtual void keyPressEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);

    virtual void paintGL();
    virtual void resizeGL(int w, int h);

    void paintChartLine();

    void paintGeometry(); // paint edges

    void paintScalarField(); // paint scalar field surface
    void paintContours(); // paint scalar field contours
    void paintContoursTri(double3* vert, int3* tri, double step);
    void paintVectors(); // paint vector field vectors

    void paintPostprocessorSelectedVolume(); // paint selected volume for integration
    void paintPostprocessorSelectedSurface(); // paint selected surface for integration

private:
    // gl lists
    int m_listContours;
    int m_listVectors;
    int m_listScalarField;

    Post2DHermes *m_post2DHermes;

    void createActionsPost2D();

private slots:
    void doShowGroup(QAction *action);
};

#endif // SCENEVIEWPOST2D_H
