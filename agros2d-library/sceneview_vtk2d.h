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

template <typename Scalar> class SceneSolution;
template <typename Scalar> class ViewScalarFilter;

class SceneViewVTK2D;
class LineEditDouble;

class SceneViewVTK2D : public QVTKWidget
{
    Q_OBJECT

public slots:
    virtual void clear();

public:
    SceneViewVTK2D(PostHermes *postHermes, QWidget *parent = 0);
    ~SceneViewVTK2D();

    QAction *actSceneModeVTK2D;

    virtual QIcon iconView() { return icon("scene-particle"); }
    virtual QString labelView() { return tr("Particle Tracing"); }

    void doZoomBestFit();

protected:
    vtkSmartPointer<vtkCubeAxesActor2D> axesActor();
    vtkSmartPointer<vtkLegendScaleActor> rulersActor();

    vtkSmartPointer<vtkActor> geometryActor();

    vtkSmartPointer<vtkActor> contourActor();
    vtkSmartPointer<vtkActor> scalarActor();
    vtkSmartPointer<vtkScalarBarActor> scalarColorBar();

private:
    PostHermes *m_postHermes;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkLookupTable> m_palette;

    void initVTK();
    void createControls();

private slots:
    virtual void refresh();
    void setControls();
};

#endif // SCENEVIEWVTK2D_H
