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

#include "sceneview_data.h"

#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkLine.h>
#include <vtkTriangle.h>
#include <vtkCellArray.h>
#include <vtkCamera.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkScalarBarActor.h>
#include <vtkProperty2D.h>
#include <vtkTextProperty.h>
#include <vtkScalarBarWidget.h>
#include <vtkContourFilter.h>
#include <vtkBandedPolyDataContourFilter.h>

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
    // create own palette
    m_palette = vtkSmartPointer<vtkLookupTable>::New();
    for (int i = 0; i < PALETTEENTRIES; i++)
        m_palette->SetTableValue(i, paletteDataAgros2D[i][0], paletteDataAgros2D[i][1], paletteDataAgros2D[i][2]);

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

    m_palette->SetRange(m_postHermes->linScalarView().get_min_value(), m_postHermes->linScalarView().get_max_value());
    m_palette->Build();

    vtkSmartPointer<vtkActor> scalar = scalarActor();
    m_renderer->AddActor(scalar);

    vtkSmartPointer<vtkActor> contour = contourActor();
    m_renderer->AddActor(contour);

    vtkSmartPointer<vtkActor> geometry = geometryActor();
    m_renderer->AddActor(geometry);

    vtkSmartPointer<vtkScalarBarActor> colorBar = scalarColorBar();
    m_renderer->AddActor2D(colorBar);

    double *bounds = scalar->GetBounds();
    double cx = (bounds[0] + bounds[1]) / 2.0;
    double cy = (bounds[2] + bounds[3]) / 2.0;
    double scale = max(fabs(bounds[0] - cx), max(fabs(bounds[1] - cx), max(fabs(bounds[2] - cy), fabs(bounds[3] - cy))));

    //  camera
    vtkCamera *camera = m_renderer->GetActiveCamera();
    camera->ParallelProjectionOn();
    camera->SetParallelScale(scale);
    camera->SetPosition(cx, cy, 1.0);
    camera->SetFocalPoint(cx, cy, 0);
}

vtkSmartPointer<vtkScalarBarActor> SceneViewVTK2D::scalarColorBar()
{
    // variable
    Module::LocalVariable localVariable = m_postHermes->activeViewField()->localVariable(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString());
    QString str = QString("%1 (%2)").
            arg(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString().isEmpty() ? "" : localVariable.shortname()).
            arg(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString().isEmpty() ? "" : localVariable.unit());

    vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
    scalarBar->SetLookupTable(m_palette);
    scalarBar->SetWidth(0.15);
    scalarBar->SetHeight(0.4);
    scalarBar->SetLabelFormat("%+#6.3e");
    scalarBar->SetTitle(str.toStdString().c_str());
    scalarBar->SetNumberOfLabels(11);

    scalarBar->GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
    scalarBar->GetPositionCoordinate()->SetValue(0.83, 0.02);

    scalarBar->GetTitleTextProperty()->SetFontFamilyToCourier();
    scalarBar->GetTitleTextProperty()->SetVerticalJustificationToCentered();
    scalarBar->GetTitleTextProperty()->BoldOff();
    scalarBar->GetTitleTextProperty()->ItalicOff();
    scalarBar->GetTitleTextProperty()->ShadowOff();
    scalarBar->GetTitleTextProperty()->SetColor(0, 0, 0);

    scalarBar->GetLabelTextProperty()->SetFontFamilyToCourier();
    scalarBar->GetLabelTextProperty()->SetJustificationToRight();
    scalarBar->GetLabelTextProperty()->SetVerticalJustificationToCentered();
    scalarBar->GetLabelTextProperty()->SetFontSize(30);
    scalarBar->GetLabelTextProperty()->BoldOff();
    scalarBar->GetLabelTextProperty()->ItalicOff();
    scalarBar->GetLabelTextProperty()->ShadowOff();
    scalarBar->GetLabelTextProperty()->SetColor(0, 0, 0);

    return scalarBar;
}

vtkSmartPointer<vtkActor> SceneViewVTK2D::geometryActor()
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    foreach (SceneNode *node, Agros2D::scene()->nodes->items())
        points->InsertNextPoint(node->point().x, node->point().y, 0.0);

    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
    {
        if (edge->isStraight())
        {
            vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
            line->GetPointIds()->SetId(0, Agros2D::scene()->nodes->items().indexOf(edge->nodeStart()));
            line->GetPointIds()->SetId(1, Agros2D::scene()->nodes->items().indexOf(edge->nodeEnd()));

            lines->InsertNextCell(line);
        }
        else
        {
            // arc
            // add pseudo nodes
            Point center = edge->center();
            double radius = edge->radius();
            double startAngle = atan2(center.y - edge->nodeStart()->point().y,
                                      center.x - edge->nodeStart()->point().x) - M_PI;

            int segments = edge->angle() / 5.0;
            double theta = deg2rad(edge->angle()) / double(segments);

            for (int j = 1; j < segments; j++)
            {
                double arc = startAngle + j*theta;

                double x = radius * cos(arc);
                double y = radius * sin(arc);

                if (j < segments - 1)
                    points->InsertNextPoint(center.x + x, center.y + y, 0.0);

                vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
                if (j == 0)
                {
                    line->GetPointIds()->SetId(0, Agros2D::scene()->nodes->items().indexOf(edge->nodeStart()));
                    line->GetPointIds()->SetId(1, points->GetNumberOfPoints() - 1);
                }
                else if (j == segments - 1)
                {
                    line->GetPointIds()->SetId(0, points->GetNumberOfPoints() - 1);
                    line->GetPointIds()->SetId(1, Agros2D::scene()->nodes->items().indexOf(edge->nodeEnd()));
                }
                else
                {
                    line->GetPointIds()->SetId(0, points->GetNumberOfPoints() - 1);
                    line->GetPointIds()->SetId(1, points->GetNumberOfPoints() - 2);
                }

                lines->InsertNextCell(line);
            }
        }
    }

    // create a polydata object
    vtkSmartPointer<vtkPolyData> linePolyData = vtkSmartPointer<vtkPolyData>::New();

    // add the geometry and topology to the polydata
    linePolyData->SetPoints(points);
    linePolyData->SetLines(lines);

    // create mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInput(linePolyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0, 0, 0);
    actor->GetProperty()->SetLineWidth(1.8);

    return actor;
}

vtkSmartPointer<vtkActor> SceneViewVTK2D::scalarActor()
{
    // scalar
    m_postHermes->linScalarView().lock_data();

    double3* linVert = m_postHermes->linScalarView().get_vertices();
    int3* linTris = m_postHermes->linScalarView().get_triangles();

    vtkSmartPointer<vtkDoubleArray> weights = vtkSmartPointer<vtkDoubleArray>::New();
    weights->SetNumberOfComponents(1);

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    weights->SetName("scalar");
    for (int i = 0; i < m_postHermes->linScalarView().get_num_vertices(); i++)
    {
        points->InsertNextPoint(linVert[i][0], linVert[i][1], 0.0);
        weights->InsertNextValue(linVert[i][2]);
    }

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

    // create a polydata object
    vtkSmartPointer<vtkPolyData> trianglePolyData = vtkSmartPointer<vtkPolyData>::New();

    // add the geometry and topology to the polydata
    trianglePolyData->SetPoints(points);
    trianglePolyData->SetPolys(triangles);
    trianglePolyData->GetPointData()->SetScalars(weights);

    // create mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetScalarRange(m_postHermes->linScalarView().get_min_value(), m_postHermes->linScalarView().get_max_value());
    mapper->SetLookupTable(m_palette);
    mapper->SetInput(trianglePolyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    return actor;
}

vtkSmartPointer<vtkActor> SceneViewVTK2D::contourActor()
{
    // scalar
    m_postHermes->linContourView().lock_data();

    double3* linVert = m_postHermes->linContourView().get_vertices();
    int3* linTris = m_postHermes->linContourView().get_triangles();

    vtkSmartPointer<vtkDoubleArray> weights = vtkSmartPointer<vtkDoubleArray>::New();
    weights->SetNumberOfComponents(1);

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    weights->SetName("scalar");
    for (int i = 0; i < m_postHermes->linContourView().get_num_vertices(); i++)
    {
        points->InsertNextPoint(linVert[i][0], linVert[i][1], 0.0);
        weights->InsertNextValue(linVert[i][2]);
    }

    vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
    for (int i = 0; i < m_postHermes->linContourView().get_num_triangles(); i++)
    {
        vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
        for (int j = 0; j < 3; j++)
        {
            triangle->GetPointIds()->SetId(j, linTris[i][j]);
        }
        triangles->InsertNextCell(triangle);
    }

    m_postHermes->linContourView().unlock_data();

    // create a polydata object
    vtkSmartPointer<vtkPolyData> trianglePolyData = vtkSmartPointer<vtkPolyData>::New();

    // add the geometry and topology to the polydata
    trianglePolyData->SetPoints(points);
    trianglePolyData->SetPolys(triangles);
    trianglePolyData->GetPointData()->SetScalars(weights);

    vtkSmartPointer<vtkContourFilter> bf = vtkSmartPointer<vtkContourFilter>::New();
    bf->SetInput(trianglePolyData);
    bf->GenerateValues(Agros2D::problem()->setting()->value(ProblemSetting::View_ContoursCount).toInt(),
                       m_postHermes->linContourView().get_min_value(), m_postHermes->linContourView().get_max_value());

    // create mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetScalarRange(m_postHermes->linContourView().get_min_value(), m_postHermes->linContourView().get_max_value());
    mapper->SetScalarModeToUseCellData();
    mapper->SetLookupTable(m_palette);
    mapper->SetInputConnection(bf->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorContoursRed).toInt() / 255.0,
                                   Agros2D::problem()->setting()->value(ProblemSetting::View_ColorContoursGreen).toInt() / 255.0,
                                   Agros2D::problem()->setting()->value(ProblemSetting::View_ColorContoursBlue).toInt() / 255.0);
    actor->GetProperty()->SetLineWidth(Agros2D::problem()->setting()->value(ProblemSetting::View_ContoursWidth).toInt());

    return actor;
}

void SceneViewVTK2D::setControls()
{

}

void SceneViewVTK2D::doZoomBestFit()
{

}
