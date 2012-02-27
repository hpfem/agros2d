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
#include "sceneview_common2d.h"

class SceneViewPost2D : public SceneViewCommon2D
{
    Q_OBJECT

public slots:   
    void doSelectMarker();
    virtual void doDefaultValues();
    void doPostprocessorModeGroup(QAction *action);

public:
    SceneViewPost2D(QWidget *parent = 0);
    ~SceneViewPost2D();

    QActionGroup *actShowGroup;
    QAction *actShowContours;
    QAction *actShowVectors;
    QAction *actShowSolutionMesh;

    QAction *actSceneViewSelectByMarker;

    QActionGroup *actPostprocessorModeGroup;
    QAction *actPostprocessorModeLocalPointValue;
    QAction *actPostprocessorModeSurfaceIntegral;
    QAction *actPostprocessorModeVolumeIntegral;

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);

    virtual void paintGL();
    virtual void resizeGL(int w, int h);

    void paintChartLine();

    void paintGeometry(); // paint edges

    void paintSolutionMesh();
    void paintScalarField(); // paint scalar field surface
    void paintContours(); // paint scalar field contours
    void paintContoursTri(double3* vert, int3* tri, double step);
    void paintVectors(); // paint vector field vectors
    void paintOrder();
    void paintOrderColorBar();

    void paintPostprocessorSelectedVolume(); // paint selected volume for integration
    void paintPostprocessorSelectedSurface(); // paint selected surface for integration

private:
    void createActionsPost2D();

private slots:
    void doShowGroup(QAction *action);
};

#endif // SCENEVIEWPOST2D_H
