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

class SceneViewPost2D : public SceneViewCommon2D
{
    Q_OBJECT

public slots:   
    void selectByMarker();
    void selectPoint();
    void doPostprocessorModeGroup(QAction *action);
    void setControls();
    virtual void clear();
    void exportVTKScalarView(const QString &fileName = QString());

public:
    SceneViewPost2D(PostHermes *postHermes, QWidget *parent = 0);
    ~SceneViewPost2D();

    QAction *actSceneModePost2D;

    QAction *actSelectPoint;
    QAction *actSelectByMarker;

    QActionGroup *actPostprocessorModeGroup;
    QAction *actPostprocessorModeLocalPointValue;
    QAction *actPostprocessorModeSurfaceIntegral;
    QAction *actPostprocessorModeVolumeIntegral;

    QAction *actExportVTKScalar;

    virtual QIcon iconView() { return icon("scene-post2d"); }
    virtual QString labelView() { return tr("Postprocessor 2D"); }

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);

    virtual void paintGL();
    virtual void resizeGL(int w, int h);

    void paintGeometry(); // paint edges

    void paintScalarField(); // paint scalar field surface
    void paintContours(); // paint scalar field contours
    void paintContoursTri(double3* vert, int3* tri, double step);
    void paintVectors(); // paint vector field vectors

    void paintPostprocessorSelectedVolume(); // paint selected volume for integration
    void paintPostprocessorSelectedSurface(); // paint selected surface for integration
    void paintPostprocessorSelectedPoint(); // paint point for local values

private:
    // selected point
    Point m_selectedPoint;

    // gl lists
    int m_listContours;
    int m_listVectors;
    int m_listScalarField;

    // QVector<QVector2D> m_arrayScalarField;
    // QVector<QVector3D> m_arrayScalarFieldColors;

    void createActionsPost2D();

private slots:
    void showGroup(QAction *action);
    void selectedPoint(const Point &p);

    virtual void refresh();
    virtual void clearGLLists();
};

#endif // SCENEVIEWPOST2D_H
