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

#include "sceneview_vtk2d.h"

#include "util.h"
#include "util/global.h"
#include "util/constants.h"
#include "util/loops.h"

#include "gui/lineeditdouble.h"
#include "gui/common.h"

#include "particle/particle_tracing.h"

#include "scene.h"
#include "hermes2d/problem.h"
#include "logview.h"

#include "pythonlab/pythonengine_agros.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"

#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem_config.h"

#include <vtkSphereSource.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkTriangle.h>
#include <vtkCellArray.h>
#include <vtkCamera.h>
#include <vtkDoubleArray.h>

// *************************************************************************************************

SceneViewVTK2D::SceneViewVTK2D(PostHermes *postHermes, QWidget *parent)
    : QVTKWidget(parent), m_postHermes(postHermes)
{
    initVTK();
    createControls();

    connect(Agros2D::scene(), SIGNAL(defaultValues()), this, SLOT(clear()));
    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(clear()));

    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(refresh()));
    connect(m_postHermes, SIGNAL(processed()), this, SLOT(refresh()));

    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(setControls()));
    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(setControls()));
}

SceneViewVTK2D::~SceneViewVTK2D()
{
}

void SceneViewVTK2D::initVTK()
{
    // setup window
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();

    // setup renderer
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->SetBackground(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundRed).toInt() / 255.0,
                              Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundGreen).toInt() / 255.0,
                              Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundBlue).toInt() / 255.0);
    renderWindow->AddRenderer(m_renderer);

    SetRenderWindow(renderWindow);
}

void SceneViewVTK2D::createControls()
{
    actSceneModeVTK2D = new QAction(iconView(), tr("VTK 2D"), this);
    actSceneModeVTK2D->setShortcut(tr("Ctrl+8"));
    actSceneModeVTK2D->setCheckable(true);

    // Setup sphere
    /*
    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->Update();
    vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    sphereMapper->SetInputConnection(sphereSource->GetOutputPort());
    vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();
    sphereActor->SetMapper(sphereMapper);

    m_renderer->AddActor(sphereActor);
    */
    m_renderer->ResetCamera();
}

void SceneViewVTK2D::clear()
{
    m_renderer->RemoveAllViewProps();
}

void SceneViewVTK2D::refresh()
{
    if (!Agros2D::problem()->isSolved())
        return;

    clear();

    // scalar
    m_postHermes->linScalarView().lock_data();

    double3* linVert = m_postHermes->linScalarView().get_vertices();
    int3* linTris = m_postHermes->linScalarView().get_triangles();

    vtkSmartPointer<vtkDoubleArray> weights = vtkSmartPointer<vtkDoubleArray>::New();
    weights->SetNumberOfValues(m_postHermes->linScalarView().get_num_vertices());

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for (int i = 0; i < m_postHermes->linScalarView().get_num_vertices(); i++)
    {
        points->InsertNextPoint(linVert[i][0], linVert[i][1], 0.0);
        weights->SetValue(i, linVert[i][2]);
    }
    points->SetData(weights);

    vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
    for (int i = 0; i < m_postHermes->linScalarView().get_num_triangles(); i++)
    {
        vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
        for (int j = 0; j < 3; j++)
        {
            triangle->GetPointIds()->SetId(j, linTris[i][j]);
        }
        triangles->InsertNextCell(triangle);
    }

    m_postHermes->linScalarView().unlock_data();

    // Create a polydata object
    vtkSmartPointer<vtkPolyData> trianglePolyData = vtkSmartPointer<vtkPolyData>::New();

    // Add the geometry and topology to the polydata
    trianglePolyData->SetPoints(points);
    trianglePolyData->SetPolys(triangles);

    double *bounds = trianglePolyData->GetBounds();
    double cx = (bounds[0] + bounds[1]) / 2.0;
    double cy = (bounds[2] + bounds[3]) / 2.0;
    double scale = max(fabs(bounds[0] - cx), max(fabs(bounds[1] - cx), max(fabs(bounds[2] - cy), fabs(bounds[3] - cy))));

    // Create mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetScalarRange(m_postHermes->linScalarView().get_min_value(), m_postHermes->linScalarView().get_max_value());
#if VTK_MAJOR_VERSION <= 5
    mapper->SetInput(trianglePolyData);
#else
    mapper->SetInputData(trianglePolyData);
#endif
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    m_renderer->AddActor(actor);

    //  camera
    vtkCamera *camera = m_renderer->GetActiveCamera();
    camera->ParallelProjectionOn();
    camera->SetParallelScale(scale);
    camera->SetPosition(cx, cy, 1);
    camera->SetFocalPoint(cx, cy, 0);

    m_renderer->ResetCamera();
}

void SceneViewVTK2D::setControls()
{

}

void SceneViewVTK2D::doZoomBestFit()
{

}
