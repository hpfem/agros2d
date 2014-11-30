// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef SCENEVIEWVTK2D_H
#define SCENEVIEWVTK2D_H

#include "util.h"
#include "sceneview_common3d.h"
#include "QVTKWidget.h"

#include <vtkSmartPointer.h>
#include <vtkLookupTable.h>
#include <vtkActor.h>
#include <vtkScalarBarActor.h>
#include <vtkCubeAxesActor2D.h>
#include <vtkLegendScaleActor.h>
#include <vtkEventQtSlotConnect.h>

template <typename Scalar> class SceneSolution;
template <typename Scalar> class ViewScalarFilter;

class SceneViewVTK2D;
class LineEditDouble;

/*! Gridlines class */
//class GridlineActors
//{
//public:
//    GridlineActors();
//    ~GridlineActors();

//    void createGridxy(double bounds[],int height_spacing, int width_spacing, int line_width, int r, int g, int b, int opacity, int z_plane_value);
//    void createGridxz(double bounds[],int height_spacing, int width_spacing, int line_width, int r, int g, int b, int opacity, int z_plane_value);
//    void createGridyz(double bounds[],int height_spacing, int width_spacing, int line_width, int r, int g, int b, int opacity, int z_plane_value);
//    void createGrid3D(double bounds[],int height_spacing, int width_spacing, int depth_spacing, int line_width, int r, int g, int b, int opacity);

//    vtkSmartPointer<vtkActor> GetHorizontalGridlines(int i);
//    vtkSmartPointer<vtkActor> GetVerticalGridlines(int i);
//    vtkSmartPointer<vtkActor> GetDepthGridlines(int i);

//    int NumberOfLines();
//    int NumberOfHorizontalLines();
//    int NumberOfVerticalLines();
//    int NumberOfDepthLines();

//private:
//    vtkSmartPointer<vtkActor>* GridlineActorVectorHorizontal;	/*!< Contains horizontal gridline actors */
//    vtkSmartPointer<vtkActor>* GridlineActorVectorVertical;		/*!< Contains vertical gridline actors */
//    vtkSmartPointer<vtkActor>* GridlineActorVectorDepth;		/*!< Contains depth gridline actors */

//    int num_horizontal_lines;	/*!< Number of horizontal lines */
//    int num_vertical_lines;		/*!< Number of vertical lines */
//    int num_depth_lines;		/*!< Number of depth lines */
//};

class SceneViewVTK2D : public QVTKWidget
{
    Q_OBJECT

public slots:
    virtual void clear();

public:
    SceneViewVTK2D(PostHermes *postHermes, QWidget *parent = 0);
    ~SceneViewVTK2D();

    QAction *actSceneModeVTK2D;

    virtual QIcon iconView() { return icon("scene-post2d"); }
    virtual QString labelView() { return tr("VTK"); }

    void doZoomBestFit();

protected:
    // palette
    const double *paletteColor(const int pos) const;

    vtkSmartPointer<vtkCubeAxesActor2D> axesActor();
    vtkSmartPointer<vtkLegendScaleActor> rulersActor();

    vtkSmartPointer<vtkActor> geometryActor();
    vtkSmartPointer<vtkActor> meshActor();

    vtkSmartPointer<vtkActor> contourActor();
    vtkSmartPointer<vtkActor> scalarActor();
    vtkSmartPointer<vtkActor> vectorActor();
    vtkSmartPointer<vtkScalarBarActor> scalarColorBar();

private:
    PostHermes *m_postHermes;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkEventQtSlotConnect> m_connections;
    vtkSmartPointer<vtkLookupTable> m_palette;
    // GridlineActors *m_gridlines;

    void initVTK();
    void createControls();

private slots:
    virtual void refresh();
    void setControls();

    void leftButtonPressEvent(vtkObject *caller, unsigned long eventId, void *clientData, void *callData);
};

#endif // SCENEVIEWVTK2D_H
