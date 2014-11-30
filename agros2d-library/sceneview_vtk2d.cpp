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
#include <vtkQuad.h>
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
#include <vtkAxisActor2D.h>
#include <vtkArrowSource.h>
#include <vtkGlyph2D.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkGeometryFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkObjectFactory.h>
#include <vtkLineSource.h>

// *************************************************************************************************

//GridlineActors::GridlineActors()
//{
//    /*!
//     * Default constructor
//     */
//    num_horizontal_lines = 0;
//    num_vertical_lines = 0;
//    num_depth_lines = 0;
//    GridlineActorVectorHorizontal = 0;
//    GridlineActorVectorVertical = 0;
//    GridlineActorVectorDepth = 0;
//}
//GridlineActors::~GridlineActors()
//{
//    /*!
//     * Destructor
//     */
//    if(GridlineActorVectorHorizontal != 0)
//    {
//        delete [] GridlineActorVectorHorizontal;
//    }
//    if(GridlineActorVectorVertical != 0)
//    {
//        delete [] GridlineActorVectorVertical;
//    }
//    if(GridlineActorVectorDepth != 0)
//    {
//        delete [] GridlineActorVectorDepth;
//    }
//}
//void GridlineActors::createGridxy(double bounds[],int height_spacing, int width_spacing, int line_width, int r, int g, int b, int opacity, int z_plane_value)
//{
//    /*!
//     * Create a 2D xy grid
//     */
//    num_depth_lines = 0;

//    double r_color = r/256.0;
//    double g_color = g/256.0;
//    double b_color = b/256.0;
//    double line_opacity = opacity/100.0;

//    vtkSmartPointer<vtkProperty> lineproperty = vtkSmartPointer<vtkProperty>::New();
//    lineproperty->SetColor(r_color,g_color,b_color);
//    lineproperty->SetOpacity(line_opacity);
//    lineproperty->SetLineWidth(line_width);

//    //Manually create lines
//    /// horizontal lines
//    num_horizontal_lines = (bounds[3] - bounds[2]) / height_spacing + 1;
//    if(GridlineActorVectorHorizontal != 0)
//    {
//        delete [] GridlineActorVectorHorizontal;
//    }
//    GridlineActorVectorHorizontal = new vtkSmartPointer<vtkActor>[num_horizontal_lines];

//    unsigned int horizontal_line_index = 0;
//    for (double increment = bounds[2]; increment < bounds[3]; increment+=height_spacing, horizontal_line_index++)
//    {
//        // Create two points, P0 and P1
//        double p0[3] = {bounds[0], increment, z_plane_value};
//        double p1[3] = {bounds[1], increment, z_plane_value};

//        vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
//        lineSource->SetPoint1(p0);
//        lineSource->SetPoint2(p1);
//        lineSource->Update();

//        // Visualize
//        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//        mapper->SetInputConnection(lineSource->GetOutputPort());
//        //Create GridlineActor
//        vtkSmartPointer<vtkActor> GridlineActor = vtkSmartPointer<vtkActor>::New();
//        GridlineActor->SetProperty(lineproperty);
//        //Store it to the GridlineActorVector
//        GridlineActorVectorHorizontal[horizontal_line_index] = GridlineActor;

//        //Add the GridlineActor to the m_renderer
//        GridlineActor->SetMapper(mapper);
//        GridlineActor->SetPickable(0);
//    }

//    /// vertical lines
//    num_vertical_lines = (bounds[1] - bounds[0]) / width_spacing + 1;
//    if(GridlineActorVectorVertical != 0)
//    {
//        delete [] GridlineActorVectorVertical;
//    }
//    GridlineActorVectorVertical = new vtkSmartPointer<vtkActor>[num_vertical_lines];

//    unsigned int vertical_line_index = 0;
//    for (double increment = bounds[0]; increment < bounds[1]; increment+=width_spacing, vertical_line_index++)
//    {
//        // Create two points, P0 and P1
//        double p0[3] = {increment, bounds[2], z_plane_value};
//        double p1[3] = {increment, bounds[3], z_plane_value};

//        vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
//        lineSource->SetPoint1(p0);
//        lineSource->SetPoint2(p1);
//        lineSource->Update();

//        // Visualize
//        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//        mapper->SetInputConnection(lineSource->GetOutputPort());
//        vtkSmartPointer<vtkActor> GridlineActor = vtkSmartPointer<vtkActor>::New();
//        GridlineActor->SetProperty(lineproperty);
//        GridlineActorVectorVertical[vertical_line_index] = GridlineActor;
//        GridlineActor->SetMapper(mapper);
//        GridlineActor->SetPickable(0);
//    }
//}
//void GridlineActors::createGridxz(double bounds[],int height_spacing, int width_spacing, int line_width, int r, int g, int b, int opacity, int z_plane_value)
//{
//    /*!
//     * Create a 2D xz grid
//     */
//    num_depth_lines = 0;
//    double r_color = r/256.0;
//    double g_color = g/256.0;
//    double b_color = b/256.0;
//    double line_opacity = opacity/100.0;

//    vtkSmartPointer<vtkProperty> lineproperty = vtkSmartPointer<vtkProperty>::New();
//    lineproperty->SetColor(r_color,g_color,b_color);
//    lineproperty->SetOpacity(line_opacity);
//    lineproperty->SetLineWidth(line_width);

//    //Manually create lines
//    /// horizontal lines
//    num_horizontal_lines = (bounds[5] - bounds[4]) / height_spacing + 1;
//    if(GridlineActorVectorHorizontal != 0)
//    {
//        delete [] GridlineActorVectorHorizontal;
//    }
//    GridlineActorVectorHorizontal = new vtkSmartPointer<vtkActor>[num_horizontal_lines];

//    unsigned int horizontal_line_index = 0;
//    for (double increment = bounds[4]; increment < bounds[5]; increment+=height_spacing, horizontal_line_index++)
//    {
//        // Create two points, P0 and P1
//        double p0[3] = {bounds[0], 0.0, increment};
//        double p1[3] = {bounds[1], 0.0, increment};

//        vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
//        lineSource->SetPoint1(p0);
//        lineSource->SetPoint2(p1);
//        lineSource->Update();

//        // Visualize
//        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//        mapper->SetInputConnection(lineSource->GetOutputPort());
//        //Create GridlineActor
//        vtkSmartPointer<vtkActor> GridlineActor = vtkSmartPointer<vtkActor>::New();
//        GridlineActor->SetProperty(lineproperty);
//        //Store it to the GridlineActorVector
//        GridlineActorVectorHorizontal[horizontal_line_index] = GridlineActor;

//        //Add the GridlineActor to the m_renderer
//        GridlineActor->SetMapper(mapper);
//        GridlineActor->SetPickable(0);
//    }

//    /// vertical lines
//    num_vertical_lines = (bounds[1] - bounds[0]) / width_spacing + 1;
//    if(GridlineActorVectorVertical != 0)
//    {
//        delete [] GridlineActorVectorVertical;
//    }
//    GridlineActorVectorVertical = new vtkSmartPointer<vtkActor>[num_vertical_lines];

//    unsigned int vertical_line_index = 0;
//    for (double increment = bounds[0]; increment < bounds[1]; increment+=width_spacing, vertical_line_index++)
//    {
//        // Create two points, P0 and P1
//        double p0[3] = {increment, 0.0, bounds[4]};
//        double p1[3] = {increment, 0.0, bounds[5]};

//        vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
//        lineSource->SetPoint1(p0);
//        lineSource->SetPoint2(p1);
//        lineSource->Update();

//        // Visualize
//        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//        mapper->SetInputConnection(lineSource->GetOutputPort());
//        vtkSmartPointer<vtkActor> GridlineActor = vtkSmartPointer<vtkActor>::New();
//        GridlineActor->SetProperty(lineproperty);
//        GridlineActorVectorVertical[vertical_line_index] = GridlineActor;
//        GridlineActor->SetMapper(mapper);
//        GridlineActor->SetPickable(0);
//    }
//}
//void GridlineActors::createGridyz(double bounds[],int height_spacing, int width_spacing, int line_width, int r, int g, int b, int opacity, int z_plane_value)
//{
//    /*!
//     * Create a 2D yz grid
//     */
//    num_depth_lines = 0;
//    double r_color = r/256.0;
//    double g_color = g/256.0;
//    double b_color = b/256.0;
//    double line_opacity = opacity/100.0;

//    vtkSmartPointer<vtkProperty> lineproperty = vtkSmartPointer<vtkProperty>::New();
//    lineproperty->SetColor(r_color,g_color,b_color);
//    lineproperty->SetOpacity(line_opacity);
//    lineproperty->SetLineWidth(line_width);

//    //Manually create lines
//    /// horizontal lines
//    num_horizontal_lines = (bounds[5] - bounds[4]) / height_spacing + 1;
//    if(GridlineActorVectorHorizontal != 0)
//    {
//        delete [] GridlineActorVectorHorizontal;
//    }
//    GridlineActorVectorHorizontal = new vtkSmartPointer<vtkActor>[num_horizontal_lines];

//    unsigned int horizontal_line_index = 0;
//    for (double increment = bounds[4]; increment < bounds[5]; increment+=height_spacing, horizontal_line_index++)
//    {
//        // Create two points, P0 and P1
//        double p0[3] = {0.0, bounds[2], increment};
//        double p1[3] = {0.0, bounds[3], increment};

//        vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
//        lineSource->SetPoint1(p0);
//        lineSource->SetPoint2(p1);
//        lineSource->Update();

//        // Visualize
//        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//        mapper->SetInputConnection(lineSource->GetOutputPort());
//        //Create GridlineActor
//        vtkSmartPointer<vtkActor> GridlineActor = vtkSmartPointer<vtkActor>::New();
//        GridlineActor->SetProperty(lineproperty);
//        //Store it to the GridlineActorVector
//        GridlineActorVectorHorizontal[horizontal_line_index] = GridlineActor;

//        //Add the GridlineActor to the m_renderer
//        GridlineActor->SetMapper(mapper);
//        GridlineActor->SetPickable(0);
//    }

//    /// vertical lines
//    num_vertical_lines = (bounds[3] - bounds[2]) / width_spacing + 1;
//    if(GridlineActorVectorVertical != 0)
//    {
//        delete [] GridlineActorVectorVertical;
//    }
//    GridlineActorVectorVertical = new vtkSmartPointer<vtkActor>[num_vertical_lines];

//    unsigned int vertical_line_index = 0;
//    for (double increment = bounds[2]; increment < bounds[3]; increment+=width_spacing, vertical_line_index++)
//    {
//        // Create two points, P0 and P1
//        double p0[3] = {0.0, increment, bounds[4]};
//        double p1[3] = {0.0, increment, bounds[5]};

//        vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
//        lineSource->SetPoint1(p0);
//        lineSource->SetPoint2(p1);
//        lineSource->Update();

//        // Visualize
//        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//        mapper->SetInputConnection(lineSource->GetOutputPort());
//        vtkSmartPointer<vtkActor> GridlineActor = vtkSmartPointer<vtkActor>::New();
//        GridlineActor->SetProperty(lineproperty);
//        GridlineActorVectorVertical[vertical_line_index] = GridlineActor;
//        GridlineActor->SetMapper(mapper);
//        GridlineActor->SetPickable(0);
//    }
//}
//void GridlineActors::createGrid3D(double bounds[],int height_spacing, int width_spacing, int depth_spacing, int line_width, int r, int g, int b, int opacity)
//{
//    /*!
//     * Create a 3D grid.
//     * @param bounds[] image bounds.
//     */
//    double r_color = r/256.0;
//    double g_color = g/256.0;
//    double b_color = b/256.0;
//    double line_opacity = opacity/100.0;

//    vtkSmartPointer<vtkProperty> lineproperty = vtkSmartPointer<vtkProperty>::New();
//    lineproperty->SetColor(r_color,g_color,b_color);
//    lineproperty->SetOpacity(line_opacity);
//    lineproperty->SetLineWidth(line_width);

//    //Manually create lines

//    int temp_depth_lines = (bounds[5] - bounds[4]) / depth_spacing + 1;
//    int temp_horizontal_lines = (bounds[3] - bounds[2]) / height_spacing + 1;
//    int temp_vertical_lines = (bounds[1] - bounds[0]) / width_spacing + 1;

//    /// horizontal lines
//    num_horizontal_lines = temp_horizontal_lines*temp_depth_lines;
//    if(GridlineActorVectorHorizontal != 0)
//    {
//        delete [] GridlineActorVectorHorizontal;
//    }
//    GridlineActorVectorHorizontal = new vtkSmartPointer<vtkActor>[num_horizontal_lines];

//    /// vertical lines
//    num_vertical_lines = temp_vertical_lines*temp_depth_lines;
//    if(GridlineActorVectorVertical != 0)
//    {
//        delete [] GridlineActorVectorVertical;
//    }
//    GridlineActorVectorVertical = new vtkSmartPointer<vtkActor>[num_vertical_lines];

//    /// depth lines
//    num_depth_lines = temp_vertical_lines*temp_horizontal_lines;
//    if(GridlineActorVectorDepth != 0)
//    {
//        delete [] GridlineActorVectorDepth;
//    }
//    GridlineActorVectorDepth = new vtkSmartPointer<vtkActor>[num_depth_lines];

//    unsigned int horizontal_line_index = 0;
//    unsigned int vertical_line_index = 0;
//    unsigned int depth_line_index = 0;
//    for (double depthIncrement = bounds[4]; depthIncrement < bounds[5]; depthIncrement+= depth_spacing)
//    {
//        // horizontal lines
//        for (double increment = bounds[2]; increment < bounds[3]; increment+=height_spacing, horizontal_line_index++)
//        {
//            // Create two points, P0 and P1
//            double p0[3] = {bounds[0], increment, depthIncrement};
//            double p1[3] = {bounds[1], increment, depthIncrement};

//            vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
//            lineSource->SetPoint1(p0);
//            lineSource->SetPoint2(p1);
//            lineSource->Update();

//            // Visualize
//            vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//            mapper->SetInputConnection(lineSource->GetOutputPort());
//            //Create GridlineActor
//            vtkSmartPointer<vtkActor> GridlineActor = vtkSmartPointer<vtkActor>::New();
//            GridlineActor->SetProperty(lineproperty);
//            //Store it to the GridlineActorVector
//            GridlineActorVectorHorizontal[horizontal_line_index] = GridlineActor;

//            //Add the GridlineActor to the m_renderer
//            GridlineActor->SetMapper(mapper);
//            GridlineActor->SetPickable(0);
//        }

//        // vertical lines
//        for (double increment = bounds[0]; increment < bounds[1]; increment+=width_spacing, vertical_line_index++)
//        {
//            // Create two points, P0 and P1
//            double p0[3] = {increment, bounds[2], depthIncrement};
//            double p1[3] = {increment, bounds[3], depthIncrement};

//            vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
//            lineSource->SetPoint1(p0);
//            lineSource->SetPoint2(p1);
//            lineSource->Update();

//            // Visualize
//            vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//            mapper->SetInputConnection(lineSource->GetOutputPort());
//            vtkSmartPointer<vtkActor> GridlineActor = vtkSmartPointer<vtkActor>::New();
//            GridlineActor->SetProperty(lineproperty);
//            GridlineActorVectorVertical[vertical_line_index] = GridlineActor;
//            GridlineActor->SetMapper(mapper);
//            GridlineActor->SetPickable(0);
//        }
//    }
//    for (double y_increment = bounds[2]; y_increment < bounds[3]; y_increment+=height_spacing)
//    {
//        for (double x_increment = bounds[0]; x_increment < bounds[1]; x_increment+=width_spacing, depth_line_index++)
//        {
//            //depth lines
//            // Create two points, P0 and P1
//            double p0[3] = {x_increment, y_increment, bounds[4]};
//            double p1[3] = {x_increment, y_increment, bounds[5]};

//            vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
//            lineSource->SetPoint1(p0);
//            lineSource->SetPoint2(p1);
//            lineSource->Update();

//            // Visualize
//            vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//            mapper->SetInputConnection(lineSource->GetOutputPort());
//            vtkSmartPointer<vtkActor> GridlineActor = vtkSmartPointer<vtkActor>::New();
//            GridlineActor->SetProperty(lineproperty);
//            GridlineActorVectorDepth[depth_line_index] = GridlineActor;

//            //Add the GridlineActor to the m_renderer
//            GridlineActor->SetMapper(mapper);
//            GridlineActor->SetPickable(0);
//        }
//    }
//}
//vtkSmartPointer<vtkActor> GridlineActors::GetHorizontalGridlines(int i)
//{
//    /*!
//     * @return horizontal gridlines
//     */
//    if (i == -1)
//    {
//        return NULL;
//    }
//    return this->GridlineActorVectorHorizontal[i];
//}
//vtkSmartPointer<vtkActor> GridlineActors::GetVerticalGridlines(int i)
//{
//    /*!
//     * @return vertical gridlines
//     */
//    if (i == -1)
//    {
//        return NULL;
//    }
//    return this->GridlineActorVectorVertical[i];
//}
//vtkSmartPointer<vtkActor> GridlineActors::GetDepthGridlines(int i)
//{
//    /*!
//     * @return depth gridlines
//     */
//    if (i == -1)
//    {
//        return NULL;
//    }
//    return this->GridlineActorVectorDepth[i];
//}
//int GridlineActors::NumberOfLines()
//{
//    /*!
//     * @return number of gridlines
//     */
//    return (num_horizontal_lines + num_vertical_lines + num_depth_lines);
//}
//int GridlineActors::NumberOfHorizontalLines()
//{
//    /*!
//     * @return number of horizontal gridlines
//     */
//    return num_horizontal_lines;
//}
//int GridlineActors::NumberOfVerticalLines()
//{
//    /*!
//     * @return number of vertical gridlines
//     */
//    return num_vertical_lines;
//}
//int GridlineActors::NumberOfDepthLines()
//{
//    /*!
//     * @return number of depth gridlines
//     */
//    return num_depth_lines;
//}


// *****************************************************************************************

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

const double *SceneViewVTK2D::paletteColor(const int pos) const
{
    switch ((PaletteType) Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteType).toInt())
    {
    case Palette_Agros2D:
        return paletteDataAgros2D[pos];
    case Palette_Jet:
        return paletteDataJet[pos];
    case Palette_Copper:
        return paletteDataCopper[pos];
    case Palette_Hot:
        return paletteDataHot[pos];
    case Palette_Cool:
        return paletteDataCool[pos];
    case Palette_Bone:
        return paletteDataBone[pos];
    case Palette_Pink:
        return paletteDataPink[pos];
    case Palette_Spring:
        return paletteDataSpring[pos];
    case Palette_Summer:
        return paletteDataSummer[pos];
    case Palette_Autumn:
        return paletteDataAutumn[pos];
    case Palette_Winter:
        return paletteDataWinter[pos];
    case Palette_HSV:
        return paletteDataHSV[pos];
    case Palette_BWAsc:
    {
        static double out[3];
        out[0] = out[1] = out[2] = (double) pos / (PALETTEENTRIES - 1);
        return out;
    }
    case Palette_BWDesc:
    {
        static double out[3];
        out[0] = out[1] = out[2] = 1.0 - (double) pos / (PALETTEENTRIES - 1);
        return out;
    }
    default:
        assert(0);
        return NULL;
    }
}

void SceneViewVTK2D::initVTK()
{
    // setup window
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();

    // vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    // renderWindowInteractor->SetRenderWindow(renderWindow);

    // vtkSmartPointer<MouseInteractorStyle> style = vtkSmartPointer<MouseInteractorStyle>::New();
    // renderWindowInteractor->SetInteractorStyle(style);

    // setup m_renderer
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->SetBackground(COLORBACKGROUND[0], COLORBACKGROUND[1], COLORBACKGROUND[2]);

    renderWindow->AddRenderer(m_renderer);

    SetRenderWindow(renderWindow);

    m_connections = vtkSmartPointer<vtkEventQtSlotConnect>::New();
    m_connections->Connect(this->GetRenderWindow()->GetInteractor(), vtkCommand::LeftButtonPressEvent,
                           this, SLOT(leftButtonPressEvent(vtkObject*, ulong, void*, void*)));

    // m_gridlines = new GridlineActors();
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

    RectPoint rect = Agros2D::scene()->boundingBox();
    double scale = 1.0 * max(rect.width(), rect.height());

    //  camera
    vtkCamera *camera = m_renderer->GetActiveCamera();
    camera->ParallelProjectionOn();
    camera->SetParallelScale(scale);
    camera->SetPosition(rect.start.x + rect.width() / 2.0, rect.start.y + rect.height() / 2.0, 1.0);
    camera->SetFocalPoint(rect.start.x + rect.width() / 2.0, rect.start.y + rect.height() / 2.0, 0.0);
}

void SceneViewVTK2D::refresh()
{
    if (!Agros2D::problem()->isSolved())
        return;

    m_renderer->RemoveAllViewProps();

    // create own palette
    m_palette = vtkSmartPointer<vtkLookupTable>::New();
    for (int i = 0; i < PALETTEENTRIES; i++)
    {
        const double *color = paletteColor(i);
        m_palette->SetTableValue(i, color[0], color[1], color[2]);
    }
    // m_palette->SetNumberOfColors(Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteSteps).toInt());

    double sceneBounds[6];
    m_renderer->ComputeVisiblePropBounds(sceneBounds);
    // m_gridlines->createGridxy(sceneBounds, 2, 2, 1, 128, 128, 128, 75, 1);

    /*
    if (Agros2D::configComputer()->value(Config::Config_ShowGrid).toBool())
    {
        int num_horizontal_lines = m_gridlines->NumberOfHorizontalLines();
        for (int i = 0; i < num_horizontal_lines; i++)
        {
            m_renderer->AddActor(m_gridlines->GetHorizontalGridlines(i));
        }

        int num_vertical_lines = m_gridlines->NumberOfVerticalLines();
        for (int i = 0; i < num_vertical_lines; i++)
        {
            m_renderer->AddActor(m_gridlines->GetVerticalGridlines(i));
        }

        int num_depth_lines = m_gridlines->NumberOfDepthLines();
        for (int i = 0; i < num_depth_lines; i++)
        {
            m_renderer->AddActor(m_gridlines->GetDepthGridlines(i));
        }
    }
    else
    {
        int num_horizontal_lines = m_gridlines->NumberOfHorizontalLines();
        for (int i = 0; i < num_horizontal_lines; i++)
        {
            m_renderer->RemoveActor(m_gridlines->GetHorizontalGridlines(i));
        }

        int num_vertical_lines = m_gridlines->NumberOfVerticalLines();
        for (int i = 0; i < num_vertical_lines; i++)
        {
            m_renderer->RemoveActor(m_gridlines->GetVerticalGridlines(i));
        }

        int num_depth_lines = m_gridlines->NumberOfDepthLines();
        for (int i = 0; i < num_depth_lines; i++)
        {
            m_renderer->RemoveActor(m_gridlines->GetDepthGridlines(i));
        }
    }
    */

    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarView).toBool())
    {
        m_palette->SetRange(m_postHermes->linScalarView()->get_min_value(), m_postHermes->linScalarView()->get_max_value());
        m_palette->Build();

        vtkSmartPointer<vtkActor> scalar = scalarActor();
        m_renderer->AddActor(scalar);
    }

    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowContourView).toBool())
    {
        vtkSmartPointer<vtkActor> contour = contourActor();
        m_renderer->AddActor(contour);
    }

    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowVectorView).toBool())
    {
        vtkSmartPointer<vtkActor> vector = vectorActor();
        m_renderer->AddActor(vector);
    }

    // mesh
    vtkSmartPointer<vtkActor> mesh = meshActor();
    m_renderer->AddActor(mesh);

    vtkSmartPointer<vtkActor> geometry = geometryActor();
    m_renderer->AddActor(geometry);

    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarView).toBool())
    {
        vtkSmartPointer<vtkScalarBarActor> colorBar = scalarColorBar();
        m_renderer->AddActor2D(colorBar);
    }

    vtkSmartPointer<vtkLegendScaleActor> rulers = rulersActor();
    m_renderer->AddActor2D(rulers);

    // axes
    // vtkSmartPointer<vtkCubeAxesActor2D> axes = axesActor();
    // m_renderer->AddActor2D(axes);
}

vtkSmartPointer<vtkScalarBarActor> SceneViewVTK2D::scalarColorBar()
{
    // variable
    Module::LocalVariable localVariable = m_postHermes->activeViewField()->localVariable(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString());
    QString str = QString("%1 (%2)").
            arg(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString().isEmpty() ? "" : localVariable.shortname()).
            arg(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString().isEmpty() ? "" : localVariable.unit());

    vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
    scalarBar->SetMaximumNumberOfColors(Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteSteps).toInt());
    scalarBar->SetLookupTable(m_palette);
    scalarBar->SetWidth(0.1);
    scalarBar->SetHeight(0.4);
    scalarBar->SetLabelFormat("%-#6.3e");
    scalarBar->SetTitle(str.toStdString().c_str());
    scalarBar->SetNumberOfLabels(11);
    scalarBar->SetDrawBackground(1);
    scalarBar->GetBackgroundProperty()->SetColor(0.9, 0.9, 0.9);
    scalarBar->GetBackgroundProperty()->SetOpacity(0.4);

    scalarBar->GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
    scalarBar->GetPositionCoordinate()->SetValue(0.88, 0.06);

    vtkSmartPointer<vtkTextProperty> text = vtkSmartPointer<vtkTextProperty>::New();
    text->ItalicOff();
    text->BoldOff();
    text->ShadowOff();
    text->SetFontFamilyToCourier();
    text->SetColor(0.0, 0.0, 0.0);
    text->SetVerticalJustificationToCentered();

    scalarBar->SetTitleTextProperty(text);
    scalarBar->SetLabelTextProperty(text);

    return scalarBar;
}

vtkSmartPointer<vtkLegendScaleActor> SceneViewVTK2D::rulersActor()
{
    vtkSmartPointer<vtkTextProperty> text = vtkSmartPointer<vtkTextProperty>::New();
    text->ItalicOff();
    text->BoldOff();
    text->ShadowOff();
    text->SetFontFamilyToCourier();
    text->SetColor(0.0, 0.0, 0.0);

    vtkSmartPointer<vtkLegendScaleActor> rulers = vtkSmartPointer<vtkLegendScaleActor>::New();
    rulers->SetTopAxisVisibility(false);
    rulers->SetRightAxisVisibility(false);
    rulers->SetLegendVisibility(false);
    rulers->SetLeftBorderOffset(80);
    // rulers->SetBottomBorderOffset(60);
    rulers->SetCornerOffsetFactor(1);
    rulers->GetLeftAxis()->GetProperty()->SetColor(0.0, 0.0, 0.0);
    rulers->GetLeftAxis()->SetLabelTextProperty(text);
    rulers->GetBottomAxis()->GetProperty()->SetColor(0.0, 0.0, 0.0);
    rulers->GetBottomAxis()->SetLabelTextProperty(text);

    return rulers;
}

vtkSmartPointer<vtkCubeAxesActor2D> SceneViewVTK2D::axesActor()
{
    vtkSmartPointer<vtkCubeAxesActor2D> axes = vtkSmartPointer<vtkCubeAxesActor2D>::New();
    // axes->SetInput(_grid)
    axes->SetCamera(m_renderer->GetActiveCamera());
    axes->SetLabelFormat("%6.4g");
    axes->SetFlyModeToOuterEdges();
    axes->SetFontFactor(0.8);
    axes->SetXLabel("x");
    axes->SetYLabel("y");
    axes->SetNumberOfLabels(5);
    axes->GetAxisTitleTextProperty()->BoldOff();
    axes->GetAxisTitleTextProperty()->ItalicOff();
    axes->GetAxisTitleTextProperty()->ShadowOff();
    axes->GetAxisTitleTextProperty()->SetColor(0, 0, 0);
    axes->GetAxisTitleTextProperty()->SetFontFamilyToCourier();

    return axes;
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

                vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
                if (j == 1)
                {
                    // first line
                    points->InsertNextPoint(center.x + x, center.y + y, 0.0);

                    line->GetPointIds()->SetId(0, Agros2D::scene()->nodes->items().indexOf(edge->nodeStart()));
                    line->GetPointIds()->SetId(1, points->GetNumberOfPoints() - 1);
                }
                else if (j == segments - 1)
                {
                    // last line
                    line->GetPointIds()->SetId(0, points->GetNumberOfPoints() - 1);
                    line->GetPointIds()->SetId(1, Agros2D::scene()->nodes->items().indexOf(edge->nodeEnd()));
                }
                else
                {
                    // general
                    points->InsertNextPoint(center.x + x, center.y + y, 0.0);

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
    mapper->SetInputData(linePolyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0, 0, 0);
    actor->GetProperty()->SetLineWidth(1.8);

    return actor;
}

vtkSmartPointer<vtkActor> SceneViewVTK2D::meshActor()
{
    vtkSmartPointer<vtkDoubleArray> weights = vtkSmartPointer<vtkDoubleArray>::New();
    weights->SetNumberOfComponents(1);
    weights->SetName("scalar");

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

    const std::vector<dealii::Point<2> >& vertices = m_postHermes->activeViewField()->initialMeshDeal().get()->get_vertices();

    for (int i = 0; i < vertices.size(); i++)
    {
        points->InsertNextPoint(vertices[i][0], vertices[i][1], 0.0);
    }

    vtkSmartPointer<vtkCellArray> quads = vtkSmartPointer<vtkCellArray>::New();
    typename dealii::Triangulation<2>::active_cell_iterator ti = m_postHermes->activeViewField()->initialMeshDeal().get()->begin();
    while (ti != m_postHermes->activeViewField()->initialMeshDeal().get()->end())
    {
        vtkSmartPointer<vtkQuad> quad = vtkSmartPointer<vtkQuad>::New();

        quad->GetPointIds()->SetId(0, ti->vertex_index(0));
        quad->GetPointIds()->SetId(1, ti->vertex_index(1));
        quad->GetPointIds()->SetId(3, ti->vertex_index(2)); // must be reverted
        quad->GetPointIds()->SetId(2, ti->vertex_index(3));

        quads->InsertNextCell(quad);

        ++ti;
    }

    // create a polydata object
    vtkSmartPointer<vtkPolyData> quadPolyData = vtkSmartPointer<vtkPolyData>::New();

    // add the geometry and topology to the polydata
    quadPolyData->SetPoints(points);
    quadPolyData->SetPolys(quads);
    // quadPolyData->GetPointData()->SetScalars(weights);

    // create mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(quadPolyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetRepresentationToWireframe();
    actor->GetProperty()->SetColor(0.77, 0.65, 0.35);
    actor->GetProperty()->SetLineWidth(1.3);

    return actor;
}

vtkSmartPointer<vtkActor> SceneViewVTK2D::scalarActor()
{
    vtkSmartPointer<vtkDoubleArray> weights = vtkSmartPointer<vtkDoubleArray>::New();
    weights->SetNumberOfComponents(1);
    weights->SetName("scalar");

    vtkSmartPointer<vtkGenericDataObjectReader> reader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
    reader->SetFileName(QString(tempProblemDir() + "/scalar.vtk").toStdString().c_str());
    reader->Update();

    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = reader->GetUnstructuredGridOutput();

    vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
    surfaceFilter->SetInputData(unstructuredGrid);
    surfaceFilter->Update();

    vtkSmartPointer<vtkPolyData> polyData = surfaceFilter->GetOutput();

    double range[2];
    polyData->GetPointData()->GetScalars()->GetRange(range);

    // create mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetScalarRange(range[0], range[1]);
    mapper->SetLookupTable(m_palette);
    mapper->SetInputData(polyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    return actor;
}

vtkSmartPointer<vtkActor> SceneViewVTK2D::vectorActor()
{
    vtkSmartPointer<vtkDoubleArray> arraysScalar = vtkSmartPointer<vtkDoubleArray>::New();
    arraysScalar->SetNumberOfComponents(1);
    arraysScalar->SetName("vector_size");

    vtkSmartPointer<vtkDoubleArray> arrays = vtkSmartPointer<vtkDoubleArray>::New();
    arrays->SetNumberOfComponents(3);
    arrays->SetName("vector");

    double vectorRangeMin = m_postHermes->vecVectorView()->get_min_value();
    double vectorRangeMax = m_postHermes->vecVectorView()->get_max_value();

    double irange = 1.0 / (vectorRangeMax - vectorRangeMin);

    RectPoint rect = Agros2D::scene()->boundingBox();
    double gs = (rect.width() + rect.height()) / Agros2D::problem()->setting()->value(ProblemSetting::View_VectorCount).toInt();

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    /*
    for (Views::Vectorizer::Iterator<Views::VectorLinearizerDataDimensions<LINEARIZER_DATA_TYPE>::triangle_t>
         it = m_postHermes->vecVectorView()->triangles_begin(); !it.end; ++it)
    {
        Views::VectorLinearizerDataDimensions<LINEARIZER_DATA_TYPE>::triangle_t& triangle = it.get();

        Point a(triangle[0][0], triangle[0][1]);
        Point b(triangle[1][0], triangle[1][1]);
        Point c(triangle[2][0], triangle[2][1]);

        RectPoint r;
        r.start = Point(qMin(qMin(a.x, b.x), c.x), qMin(qMin(a.y, b.y), c.y));
        r.end = Point(qMax(qMax(a.x, b.x), c.x), qMax(qMax(a.y, b.y), c.y));

        // double area
        double area2 = a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y);

        // plane equation
        double aa = b.x*c.y - c.x*b.y;
        double ab = c.x*a.y - a.x*c.y;
        double ac = a.x*b.y - b.x*a.y;
        double ba = b.y - c.y;
        double bb = c.y - a.y;
        double bc = a.y - b.y;
        double ca = c.x - b.x;
        double cb = a.x - c.x;
        double cc = b.x - a.x;

        double ax = (aa * triangle[0][2] + ab * triangle[1][2] + ac * triangle[2][2]) / area2;
        double bx = (ba * triangle[0][2] + bb * triangle[1][2] + bc * triangle[2][2]) / area2;
        double cx = (ca * triangle[0][2] + cb * triangle[1][2] + cc * triangle[2][2]) / area2;

        double ay = (aa * triangle[0][3] + ab * triangle[1][3] + ac * triangle[2][3]) / area2;
        double by = (ba * triangle[0][3] + bb * triangle[1][3] + bc * triangle[2][3]) / area2;
        double cy = (ca * triangle[0][3] + cb * triangle[1][3] + cc * triangle[2][3]) / area2;

        for (int j = floor(r.start.x / gs); j < ceil(r.end.x / gs); j++)
        {
            for (int k = floor(r.start.y / gs); k < ceil(r.end.y / gs); k++)
            {
                Point point(j*gs, k*gs);
                if (k % 2 == 0) point.x += gs/2.0;

                // find in triangle
                bool inTriangle = true;

                for (int l = 0; l < 3; l++)
                {
                    int p = l + 1;
                    if (p == 3)
                        p = 0;

                    double z = (triangle[p][0] - triangle[l][0]) * (point.y - triangle[l][1]) - (triangle[p][1] - triangle[l][1]) * (point.x - triangle[l][0]);

                    if (z < 0)
                    {
                        inTriangle = false;
                        break;
                    }
                }

                if (inTriangle)
                {
                    // view
                    double dx = ax + bx * point.x + cx * point.y;
                    double dy = ay + by * point.x + cy * point.y;

                    double value = sqrt(dx*dx + dy*dy);
                    double angle = atan2(dy, dx);

                    dx = Agros2D::problem()->setting()->value(ProblemSetting::View_VectorScale).toDouble() * gs * cos(angle);
                    dy = Agros2D::problem()->setting()->value(ProblemSetting::View_VectorScale).toDouble() * gs * sin(angle);

                    points->InsertNextPoint(point.x, point.y, 0.0);
                    arrays->InsertNextTuple3(dx, dy, 0.0);
                    if ((Agros2D::problem()->setting()->value(ProblemSetting::View_VectorProportional).toBool()) && (fabs(vectorRangeMin - vectorRangeMax) > EPS_ZERO))
                        arraysScalar->InsertNextValue((value - vectorRangeMin) * irange);
                }
            }
        }
    }
    */
    // create a polydata object
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();

    // add the geometry and topology to the polydata
    polyData->SetPoints(points);
    polyData->GetPointData()->SetVectors(arrays);
    if (Agros2D::problem()->setting()->value(ProblemSetting::View_VectorProportional).toBool())
        polyData->GetPointData()->SetScalars(arraysScalar);

    vtkSmartPointer<vtkArrowSource> arrowSource = vtkSmartPointer<vtkArrowSource>::New();

    vtkSmartPointer<vtkGlyph2D> glyph2D = vtkSmartPointer<vtkGlyph2D>::New();
    glyph2D->SetSourceConnection(arrowSource->GetOutputPort());
    glyph2D->OrientOn();
    glyph2D->SetVectorModeToUseVector();
    glyph2D->SetInputData(polyData);
    glyph2D->SetScaleFactor(0.2);
    glyph2D->Update();

    // create mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(glyph2D->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    return actor;
}

vtkSmartPointer<vtkActor> SceneViewVTK2D::contourActor()
{
    vtkSmartPointer<vtkDoubleArray> weights = vtkSmartPointer<vtkDoubleArray>::New();
    weights->SetNumberOfComponents(1);
    weights->SetName("contour");

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for (Views::Linearizer::Iterator<Views::ScalarLinearizerDataDimensions<LINEARIZER_DATA_TYPE>::vertex_t>
         it = m_postHermes->linContourView()->vertices_begin(); !it.end; ++it)
    {
        Views::ScalarLinearizerDataDimensions<LINEARIZER_DATA_TYPE>::vertex_t& vertex = it.get();

        points->InsertNextPoint(vertex[0], vertex[1], 0.0);
        weights->InsertNextValue(vertex[2]);
    }

    vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
    for (Views::Linearizer::Iterator<Views::triangle_indices_t>
         it = m_postHermes->linContourView()->triangle_indices_begin(); !it.end; ++it)
    {
        Views::triangle_indices_t& triangle_indices = it.get();

        vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
        triangle->GetPointIds()->SetId(0, triangle_indices[0]);
        triangle->GetPointIds()->SetId(1, triangle_indices[1]);
        triangle->GetPointIds()->SetId(2, triangle_indices[2]);

        triangles->InsertNextCell(triangle);
    }

    // create a polydata object
    vtkSmartPointer<vtkPolyData> trianglePolyData = vtkSmartPointer<vtkPolyData>::New();

    // add the geometry and topology to the polydata
    trianglePolyData->SetPoints(points);
    trianglePolyData->SetPolys(triangles);
    trianglePolyData->GetPointData()->SetScalars(weights);

    vtkSmartPointer<vtkContourFilter> bf = vtkSmartPointer<vtkContourFilter>::New();
    bf->SetInputData(trianglePolyData);
    bf->GenerateValues(Agros2D::problem()->setting()->value(ProblemSetting::View_ContoursCount).toInt(),
                       m_postHermes->linContourView()->get_min_value(), m_postHermes->linContourView()->get_max_value());

    // create mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetScalarRange(m_postHermes->linContourView()->get_min_value(), m_postHermes->linContourView()->get_max_value());
    mapper->SetScalarModeToUseCellData();
    mapper->SetLookupTable(m_palette);
    mapper->SetInputConnection(bf->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(COLORCONTOURS[0], COLORCONTOURS[1], COLORCONTOURS[2]);
    actor->GetProperty()->SetLineWidth(Agros2D::problem()->setting()->value(ProblemSetting::View_ContoursWidth).toInt());

    return actor;
}

void SceneViewVTK2D::setControls()
{

}

void SceneViewVTK2D::doZoomBestFit()
{

}


void SceneViewVTK2D::leftButtonPressEvent(vtkObject *caller, unsigned long eventId, void *clientData, void *callData)
{

    qDebug() << caller;
}
