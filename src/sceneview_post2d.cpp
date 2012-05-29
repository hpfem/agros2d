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

#include "sceneview_post2d.h"
#include "util.h"
#include "scene.h"
#include "hermes2d/problem.h"
#include "logview.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "scenemarkerselectdialog.h"
#include "resultsview.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/post_values.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"

Post2DHermes::Post2DHermes()
{
    clear();
}

Post2DHermes::~Post2DHermes()
{
}

void Post2DHermes::clear()
{
    m_contourIsPrepared = false;
    m_scalarIsPrepared = false;
    m_vectorIsPrepared = false;
}

void Post2DHermes::processRangeContour()
{
    m_contourIsPrepared = false;

    if (Util::problem()->isSolved() && Util::config()->showContourView)
    {
        bool contains = false;
        foreach (Module::LocalVariable *variable, Util::scene()->activeViewField()->module()->viewScalarVariables())
        {
            if (variable->id() == Util::config()->contourVariable)
            {
                contains = true;
                break;
            }
        }

        if (Util::config()->contourVariable == "" || !contains)
        {
            // default values
            Util::config()->contourVariable = Util::scene()->activeViewField()->module()->defaultViewScalarVariable()->id();
        }

        Util::log()->printMessage(tr("Post2DView"), tr("contour view (%1)").arg(Util::config()->contourVariable));

        QString variableName = Util::config()->contourVariable;
        Module::LocalVariable* variable = Util::scene()->activeViewField()->module()->localVariable(variableName);
        if (!variable)
            qDebug() << "error, trying to get variable " << variableName << " from module " << Util::scene()->activeViewField()->fieldId();

        ViewScalarFilter<double> *slnContourView = NULL;
        if (variable->isScalar())
            slnContourView = Util::scene()->activeViewField()->module()->viewScalarFilter(Util::scene()->activeViewField()->module()->localVariable(Util::config()->contourVariable),
                                                                                            PhysicFieldVariableComp_Scalar);
        else
            slnContourView = Util::scene()->activeViewField()->module()->viewScalarFilter(Util::scene()->activeViewField()->module()->localVariable(Util::config()->contourVariable),
                                                                                            PhysicFieldVariableComp_Magnitude);

        m_linContourView.process_solution(slnContourView,
                                          Hermes::Hermes2D::H2D_FN_VAL_0,
                                          Util::config()->linearizerQuality);

        // deformed shape
        if (Util::config()->deformContour)
            Util::scene()->activeViewField()->module()->deformShape(m_linContourView.get_vertices(), m_linContourView.get_num_vertices());

        delete slnContourView;

        m_contourIsPrepared = true;
    }
}

void Post2DHermes::processRangeScalar()
{
    m_scalarIsPrepared = false;

    if (Util::problem()->isSolved() && Util::config()->showScalarView)
    {
        bool contains = false;
        foreach (Module::LocalVariable *variable, Util::scene()->activeViewField()->module()->viewScalarVariables())
        {
            if (variable->id() == Util::config()->scalarVariable)
            {
                contains = true;
                break;
            }
        }

        if (Util::config()->scalarVariable == "" || !contains)
        {
            // default values
            Util::config()->scalarVariable = Util::scene()->activeViewField()->module()->defaultViewScalarVariable()->id();
            Util::config()->scalarVariableComp = Util::scene()->activeViewField()->module()->defaultViewScalarVariableComp();
        }

        Util::log()->printMessage(tr("Post2DView"), tr("scalar view (%1)").arg(Util::config()->scalarVariable));

        ViewScalarFilter<double> *slnScalarView = Util::scene()->activeViewField()->module()->viewScalarFilter(Util::scene()->activeViewField()->module()->localVariable(Util::config()->scalarVariable),
                                                                                                                 Util::config()->scalarVariableComp);

        m_linScalarView.process_solution(slnScalarView,
                                         Hermes::Hermes2D::H2D_FN_VAL_0,
                                         Util::config()->linearizerQuality);

        // deformed shape
        if (Util::config()->deformScalar)
            Util::scene()->activeViewField()->module()->deformShape(m_linScalarView.get_vertices(),
                                                                     m_linScalarView.get_num_vertices());

        if (Util::config()->scalarRangeAuto)
        {
            Util::config()->scalarRangeMin = m_linScalarView.get_min_value();
            Util::config()->scalarRangeMax = m_linScalarView.get_max_value();
        }

        delete slnScalarView;

        m_scalarIsPrepared = true;
    }
}

void Post2DHermes::processRangeVector()
{
    m_vectorIsPrepared = false;

    if (Util::problem()->isSolved() && Util::config()->showVectorView)
    {
        bool contains = false;
        foreach (Module::LocalVariable *variable, Util::scene()->activeViewField()->module()->viewVectorVariables())
        {
            if (variable->id() == Util::config()->vectorVariable)
            {
                contains = true;
                break;
            }
        }

        if (Util::config()->vectorVariable == "" || !contains)
        {
            // default values
            Util::config()->vectorVariable = Util::scene()->activeViewField()->module()->defaultViewVectorVariable()->id();
        }

        Util::log()->printMessage(tr("Post2DView"), tr("vector view (%1)").arg(Util::config()->vectorVariable));

        ViewScalarFilter<double> *slnVectorXView = Util::scene()->activeViewField()->module()->viewScalarFilter(Util::scene()->activeViewField()->module()->localVariable(Util::config()->vectorVariable),
                                                                                                                  PhysicFieldVariableComp_X);

        ViewScalarFilter<double> *slnVectorYView = Util::scene()->activeViewField()->module()->viewScalarFilter(Util::scene()->activeViewField()->module()->localVariable(Util::config()->vectorVariable),
                                                                                                                  PhysicFieldVariableComp_Y);

        m_vecVectorView.process_solution(slnVectorXView, slnVectorYView,
                                         Hermes::Hermes2D::H2D_FN_VAL_0, Hermes::Hermes2D::H2D_FN_VAL_0,
                                         Hermes::Hermes2D::Views::HERMES_EPS_LOW);

        // deformed shape
        if (Util::config()->deformVector)
            Util::scene()->activeViewField()->module()->deformShape(m_vecVectorView.get_vertices(),
                                                                     m_vecVectorView.get_num_vertices());

        delete slnVectorXView;
        delete slnVectorYView;

        m_vectorIsPrepared = true;
    }
}

void Post2DHermes::processSolved()
{
    m_contourIsPrepared = false;
    m_scalarIsPrepared = false;
    m_vectorIsPrepared = false;

    processRangeContour();
    processRangeScalar();
    processRangeVector();
    //    QTimer::singleShot(0, this, SLOT(processRangeContour()));
    //    QTimer::singleShot(0, this, SLOT(processRangeScalar()));
    //    QTimer::singleShot(0, this, SLOT(processRangeVector()));
    emit processed();
}

// ************************************************************************************************

SceneViewPost2D::SceneViewPost2D(QWidget *parent) : SceneViewCommon2D(parent),
    m_listContours(-1),
    m_listVectors(-1),
    m_listScalarField(-1),
    m_selectedPoint(Point())
{
    createActionsPost2D();

    connect(this, SIGNAL(mousePressed(Point)), this, SLOT(selectedPoint(Point)));
    connect(Util::scene(), SIGNAL(invalidated()), this, SLOT(refresh()));
    connect(Util::scene(), SIGNAL(defaultValues()), this, SLOT(clear()));

    m_post2DHermes = new Post2DHermes();

    connect(Util::problem(), SIGNAL(solved()), this, SLOT(refresh()));

    connect(m_post2DHermes, SIGNAL(processed()), this, SLOT(updateGL()));
}

SceneViewPost2D::~SceneViewPost2D()
{
    if (m_post2DHermes)
        delete m_post2DHermes;
}

void SceneViewPost2D::createActionsPost2D()
{
    // scene mode
    actSceneModePost2D = new QAction(iconView(), tr("Post 2D"), this);
    actSceneModePost2D->setShortcut(Qt::Key_F7);
    actSceneModePost2D->setStatusTip(tr("Postprocessor 2D"));
    actSceneModePost2D->setCheckable(true);

    // point
    actSelectPoint = new QAction(icon("select-by-point"), tr("Local point value"), this);
    connect(actSelectPoint, SIGNAL(triggered()), this, SLOT(selectPoint()));

    // marker
    actSelectByMarker = new QAction(icon("select-by-marker"), tr("Select by marker"), this);
    actSelectByMarker->setStatusTip(tr("Select by marker"));
    connect(actSelectByMarker, SIGNAL(triggered()), this, SLOT(selectByMarker()));

    // postprocessor group
    actPostprocessorModeLocalPointValue = new QAction(icon("mode-localpointvalue"), tr("Local Values"), this);
    actPostprocessorModeLocalPointValue->setCheckable(true);

    actPostprocessorModeSurfaceIntegral = new QAction(icon("mode-surfaceintegral"), tr("Surface Integrals"), this);
    actPostprocessorModeSurfaceIntegral->setCheckable(true);

    actPostprocessorModeVolumeIntegral = new QAction(icon("mode-volumeintegral"), tr("Volume Integrals"), this);
    actPostprocessorModeVolumeIntegral->setCheckable(true);

    actPostprocessorModeGroup = new QActionGroup(this);
    actPostprocessorModeGroup->addAction(actPostprocessorModeLocalPointValue);
    actPostprocessorModeGroup->addAction(actPostprocessorModeSurfaceIntegral);
    actPostprocessorModeGroup->addAction(actPostprocessorModeVolumeIntegral);
    connect(actPostprocessorModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(doPostprocessorModeGroup(QAction*)));

    actExportVTKScalar = new QAction(tr("Export VTK scalar..."), this);
    actExportVTKScalar->setStatusTip(tr("Export scalar view as VTK file"));
    connect(actExportVTKScalar, SIGNAL(triggered()), this, SLOT(exportVTKScalarView()));

    // show
    actShowSolutionMesh = new QAction(tr("Solution mesh"), this);
    actShowSolutionMesh->setCheckable(true);

    actShowContours = new QAction(tr("Contours"), this);
    actShowContours->setCheckable(true);

    actShowVectors = new QAction(tr("Vectors"), this);
    actShowVectors->setCheckable(true);

    actShowGroup = new QActionGroup(this);
    actShowGroup->setExclusive(false);
    connect(actShowGroup, SIGNAL(triggered(QAction *)), this, SLOT(showGroup(QAction *)));
    actShowGroup->addAction(actShowSolutionMesh);
    actShowGroup->addAction(actShowContours);
    actShowGroup->addAction(actShowVectors);
}

void SceneViewPost2D::keyPressEvent(QKeyEvent *event)
{
    SceneViewCommon2D::keyPressEvent(event);

    switch (event->key())
    {
    case Qt::Key_A:
    {
        // select all
        if (event->modifiers() & Qt::ControlModifier)
        {

            // select volume integral area
            if (actPostprocessorModeVolumeIntegral->isChecked())
            {
                Util::scene()->selectAll(SceneGeometryMode_OperateOnLabels);
                emit mousePressed();
            }

            // select surface integral area
            if (actPostprocessorModeSurfaceIntegral->isChecked())
            {
                Util::scene()->selectAll(SceneGeometryMode_OperateOnEdges);
                emit mousePressed();
            }

            refresh();
        }
    }
        break;
    default:
        QGLWidget::keyPressEvent(event);
    }
}

void SceneViewPost2D::mousePressEvent(QMouseEvent *event)
{
    SceneViewCommon2D::mousePressEvent(event);

    m_lastPos = event->pos();
    Point p = position(Point(event->pos().x(), event->pos().y()));

    if (event->buttons() & Qt::LeftButton)
    {
        // local point value
        if (actPostprocessorModeLocalPointValue->isChecked())
        {
            m_selectedPoint = p;
            emit mousePressed(p);

            updateGL();
        }

        // select volume integral area
        if (actPostprocessorModeVolumeIntegral->isChecked())
        {
            int index = findElementInMesh(Util::problem()->activeMeshInitial(), p);
            if (index != -1)
            {
                SceneLabel *label = Util::scene()->labels->at(atoi(Util::problem()->activeMeshInitial()->get_element_markers_conversion().
                                                                   get_user_marker(Util::problem()->activeMeshInitial()->get_element_fast(index)->marker).marker.c_str()));

                label->setSelected(!label->isSelected());
                updateGL();
            }
            emit mousePressed();
        }

        // select surface integral area
        if (actPostprocessorModeSurfaceIntegral->isChecked())
        {
            //  find edge marker
            SceneEdge *edge = findClosestEdge(p);

            edge->setSelected(!edge->isSelected());
            updateGL();

            emit mousePressed();
        }
    }
}

void SceneViewPost2D::paintGL()
{
    if (!isVisible()) return;
    makeCurrent();

    glClearColor(Util::config()->colorBackground.redF(),
                 Util::config()->colorBackground.greenF(),
                 Util::config()->colorBackground.blueF(), 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    // grid
    if (Util::config()->showGrid) paintGrid();

    // view
    if (Util::problem()->isSolved())
    {
        if (Util::config()->showScalarView) paintScalarField();
        if (Util::config()->showContourView) paintContours();
        if (Util::config()->showVectorView) paintVectors();
    }

    // geometry
    paintGeometry();

    if (Util::problem()->isSolved())
    {
        if (actPostprocessorModeLocalPointValue->isChecked()) paintPostprocessorSelectedPoint();
        if (actPostprocessorModeVolumeIntegral->isChecked()) paintPostprocessorSelectedVolume();
        if (actPostprocessorModeSurfaceIntegral->isChecked()) paintPostprocessorSelectedSurface();

        // bars
        if (Util::config()->showScalarView && Util::config()->showScalarColorBar)
            paintScalarFieldColorBar(Util::config()->scalarRangeMin, Util::config()->scalarRangeMax);
    }

    // rulers
    if (Util::config()->showRulers)
    {
        paintRulers();
        paintRulersHints();
    }

    // axes
    if (Util::config()->showAxes) paintAxes();

    paintZoomRegion();
    paintChartLine();

    if (Util::problem()->isSolved())
    {
        if (Util::config()->showScalarView)
        {
            Module::LocalVariable *localVariable = Util::scene()->activeViewField()->module()->localVariable(Util::config()->scalarVariable);
            if (localVariable)
            {
                QString text = Util::config()->scalarVariable != "" ? localVariable->name() : "";
                if (Util::config()->scalarVariableComp != PhysicFieldVariableComp_Scalar)
                    text += " - " + physicFieldVariableCompString(Util::config()->scalarVariableComp);
                emit labelCenter(text);
            }
        }
        else
        {
            emit labelCenter(tr("Postprocessor 2D"));
        }
    }
}

void SceneViewPost2D::resizeGL(int w, int h)
{
    if (Util::problem()->isSolved())
    {
        paletteFilter(textureScalar());
        paletteCreate(textureScalar());
    }

    SceneViewCommon::resizeGL(w, h);
}

void SceneViewPost2D::paintGeometry()
{
    loadProjection2d(true);

    // edges
    foreach (SceneEdge *edge, Util::scene()->edges->items())
    {
        glColor3d(Util::config()->colorEdges.redF(),
                  Util::config()->colorEdges.greenF(),
                  Util::config()->colorEdges.blueF());
        glLineWidth(Util::config()->edgeWidth);

        if (fabs(edge->angle()) < EPS_ZERO)
        {
            glBegin(GL_LINES);
            glVertex2d(edge->nodeStart()->point().x, edge->nodeStart()->point().y);
            glVertex2d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y);
            glEnd();
        }
        else
        {
            Point center = edge->center();
            double radius = edge->radius();
            double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

            drawArc(center, radius, startAngle, edge->angle(), edge->angle()/2.0);
        }

        glLineWidth(1.0);
    }
}

void SceneViewPost2D::setChartLine(const ChartLine &chartLine)
{
    // set line for chart
    m_chartLine = chartLine;

    updateGL();
}


void SceneViewPost2D::paintChartLine()
{
    loadProjection2d(true);

    glColor3d(Util::config()->colorSelected.redF(),
              Util::config()->colorSelected.greenF(),
              Util::config()->colorSelected.blueF());
    glLineWidth(3.0);

    QList<Point> points = m_chartLine.getPoints();

    glBegin(GL_LINES);
    for (int i = 0; i < points.length() - 1; i++)
    {
        glVertex2d(points.at(i).x, points.at(i).y);
        glVertex2d(points.at(i+1).x, points.at(i+1).y);
    }
    glEnd();

    // angle
    glBegin(GL_TRIANGLES);
    if (points.length() > 1)
    {
        double angle = atan2(points.at(points.length() - 1).y - points.at(points.length() - 2).y,
                             points.at(points.length() - 1).x - points.at(points.length() - 2).x);

        RectPoint rect = Util::scene()->boundingBox();
        double dm = (rect.width() + rect.height()) / 40.0;

        // head of an arrow
        double vh1x = points.at(points.length() - 1).x - dm/5.0 * cos(angle - M_PI/2.0) - dm * cos(angle);
        double vh1y = points.at(points.length() - 1).y - dm/5.0 * sin(angle - M_PI/2.0) - dm * sin(angle);
        double vh2x = points.at(points.length() - 1).x - dm/5.0 * cos(angle + M_PI/2.0) - dm * cos(angle);
        double vh2y = points.at(points.length() - 1).y - dm/5.0 * sin(angle + M_PI/2.0) - dm * sin(angle);
        double vh3x = points.at(points.length() - 1).x;
        double vh3y = points.at(points.length() - 1).y;
        glVertex2d(vh1x, vh1y);
        glVertex2d(vh2x, vh2y);
        glVertex2d(vh3x, vh3y);
    }
    glEnd();
}

void SceneViewPost2D::paintScalarField()
{
    if (!Util::problem()->isSolved()) return;
    if (!m_post2DHermes->scalarIsPrepared()) return;

    loadProjection2d(true);

    if (m_listScalarField == -1)
    {
        m_listScalarField = glGenLists(1);
        glNewList(m_listScalarField, GL_COMPILE);

        // range
        double irange = 1.0 / (Util::config()->scalarRangeMax - Util::config()->scalarRangeMin);
        // special case: constant solution
        if (fabs(Util::config()->scalarRangeMax - Util::config()->scalarRangeMin) < EPS_ZERO)
            irange = 1.0;

        m_post2DHermes->linScalarView().lock_data();

        double3* linVert = m_post2DHermes->linScalarView().get_vertices();
        int3* linTris = m_post2DHermes->linScalarView().get_triangles();
        Point point[3];
        double value[3];

        // set texture for coloring
        glEnable(GL_TEXTURE_1D);
        glBindTexture(GL_TEXTURE_1D, textureScalar());
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

        // set texture transformation matrix
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glTranslated(m_texShift, 0.0, 0.0);
        glScaled(m_texScale, 0.0, 0.0);

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_post2DHermes->linScalarView().get_num_triangles(); i++)
        {
            for (int j = 0; j < 3; j++)
            {
                point[j].x = linVert[linTris[i][j]][0];
                point[j].y = linVert[linTris[i][j]][1];
                value[j]   = linVert[linTris[i][j]][2];
            }

            if (!Util::config()->scalarRangeAuto)
            {
                double avgValue = (value[0] + value[1] + value[2]) / 3.0;
                if (avgValue < Util::config()->scalarRangeMin || avgValue > Util::config()->scalarRangeMax)
                    continue;
            }

            for (int j = 0; j < 3; j++)
            {
                if (Util::config()->scalarRangeLog)
                    glTexCoord1d(log10(1.0 + (Util::config()->scalarRangeBase-1.0)*(value[j] - Util::config()->scalarRangeMin) * irange)/log10(Util::config()->scalarRangeBase));
                else
                    glTexCoord1d((value[j] - Util::config()->scalarRangeMin) * irange);
                glVertex2d(point[j].x, point[j].y);
            }
        }
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_TEXTURE_1D);

        // switch-off texture transform
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        m_post2DHermes->linScalarView().unlock_data();

        glEndList();

        glCallList(m_listScalarField);
    }
    else
    {
        glCallList(m_listScalarField);
    }

    /*
    m_post2DHermes->linScalarView().lock_data();
    double3* linVertScalar = m_post2DHermes->linScalarView().get_vertices();
    int3* linTrisScalar = m_post2DHermes->linScalarView().get_triangles();

    // draw initial mesh
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3d(0.0, 0.2, 0.1);
    glLineWidth(1.3);

    // triangles
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < m_post2DHermes->linScalarView().get_num_triangles(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            glVertex2d(linVertScalar[linTrisScalar[i][j]][0], linVertScalar[linTrisScalar[i][j]][1]);
        }
    }
    glEnd();

    m_post2DHermes->linScalarView().unlock_data();

    // mesh
    // init linearizer for solution mesh
    Hermes::Hermes2D::ZeroSolution<double> solution(Util::scene()->activeSceneSolution()->sln(0)->get_mesh());
    Hermes::Hermes2D::Views::Linearizer linSolutionMeshView;
    linSolutionMeshView.process_solution(&solution);

    linSolutionMeshView.lock_data();

    double3* linVert = linSolutionMeshView.get_vertices();
    int3* linEdges = linSolutionMeshView.get_edges();

    // draw initial mesh
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3d(Util::config()->colorSolutionMesh.redF(),
              Util::config()->colorSolutionMesh.greenF(),
              Util::config()->colorSolutionMesh.blueF());
    glLineWidth(1.3);

    // triangles
    glBegin(GL_LINES);
    for (int i = 0; i < linSolutionMeshView.get_num_edges(); i++)
    {
        glVertex2d(linVert[linEdges[i][0]][0], linVert[linEdges[i][0]][1]);
        glVertex2d(linVert[linEdges[i][1]][0], linVert[linEdges[i][1]][1]);
    }
    glEnd();

    linSolutionMeshView.unlock_data();
    */
}

void SceneViewPost2D::paintContours()
{
    if (!Util::problem()->isSolved()) return;
    if (!m_post2DHermes->contourIsPrepared()) return;

    loadProjection2d(true);

    if (m_listContours == -1)
    {
        m_listContours = glGenLists(1);
        glNewList(m_listContours, GL_COMPILE);

        m_post2DHermes->linContourView().lock_data();

        double3* tvert = m_post2DHermes->linContourView().get_vertices();
        int3* tris = m_post2DHermes->linContourView().get_triangles();

        // transform variable
        double rangeMin =  numeric_limits<double>::max();
        double rangeMax = -numeric_limits<double>::max();

        double3* vert = new double3[m_post2DHermes->linContourView().get_num_vertices()];
        for (int i = 0; i < m_post2DHermes->linContourView().get_num_vertices(); i++)
        {
            vert[i][0] = tvert[i][0];
            vert[i][1] = tvert[i][1];
            vert[i][2] = tvert[i][2];

            if (vert[i][2] > rangeMax) rangeMax = tvert[i][2];
            if (vert[i][2] < rangeMin) rangeMin = tvert[i][2];
        }

        // value range
        double step = (rangeMax-rangeMin)/Util::config()->contoursCount;

        // draw contours
        glLineWidth(1.0);
        glColor3d(Util::config()->colorContours.redF(),
                  Util::config()->colorContours.greenF(),
                  Util::config()->colorContours.blueF());

        glBegin(GL_LINES);
        for (int i = 0; i < m_post2DHermes->linContourView().get_num_triangles(); i++)
        {
            if (finite(vert[tris[i][0]][2]) && finite(vert[tris[i][1]][2]) && finite(vert[tris[i][2]][2]))
            {
                paintContoursTri(vert, &tris[i], step);
            }
        }
        glEnd();

        delete vert;

        m_post2DHermes->linContourView().unlock_data();

        glEndList();

        glCallList(m_listContours);
    }
    else
    {
        glCallList(m_listContours);
    }
}

void SceneViewPost2D::paintContoursTri(double3* vert, int3* tri, double step)
{
    // sort the vertices by their value, keep track of the permutation sign
    int i, idx[3], perm = 0;
    memcpy(idx, tri, sizeof(idx));
    for (i = 0; i < 2; i++)
    {
        if (vert[idx[0]][2] > vert[idx[1]][2]) { std::swap(idx[0], idx[1]); perm++; }
        if (vert[idx[1]][2] > vert[idx[2]][2]) { std::swap(idx[1], idx[2]); perm++; }
    }
    if (fabs(vert[idx[0]][2] - vert[idx[2]][2]) < 1e-3 * fabs(step)) return;

    // get the first (lowest) contour value
    double val = vert[idx[0]][2];

    double y = ceil(val / step);
    if (y < val / step) y + 1.0;
    val = y * step;

    int l1 = 0, l2 = 1;
    int r1 = 0, r2 = 2;
    while (val < vert[idx[r2]][2])
    {
        double ld = vert[idx[l2]][2] - vert[idx[l1]][2];
        double rd = vert[idx[r2]][2] - vert[idx[r1]][2];

        // draw a slice of the triangle
        while (val < vert[idx[l2]][2])
        {
            double lt = (val - vert[idx[l1]][2]) / ld;
            double rt = (val - vert[idx[r1]][2]) / rd;

            double x1 = (1.0 - lt) * vert[idx[l1]][0] + lt * vert[idx[l2]][0];
            double y1 = (1.0 - lt) * vert[idx[l1]][1] + lt * vert[idx[l2]][1];
            double x2 = (1.0 - rt) * vert[idx[r1]][0] + rt * vert[idx[r2]][0];
            double y2 = (1.0 - rt) * vert[idx[r1]][1] + rt * vert[idx[r2]][1];

            if (perm & 1) { glVertex2d(x1, y1); glVertex2d(x2, y2); }
            else { glVertex2d(x2, y2); glVertex2d(x1, y1); }

            val += step;
        }
        l1 = 1;
        l2 = 2;
    }
}

void SceneViewPost2D::paintVectors()
{
    if (!Util::problem()->isSolved()) return;
    if (!m_post2DHermes->vectorIsPrepared()) return;

    loadProjection2d(true);

    if (m_listVectors == -1)
    {
        m_listVectors = glGenLists(1);
        glNewList(m_listVectors, GL_COMPILE);

        double vectorRangeMin = m_post2DHermes->vecVectorView().get_min_value();
        double vectorRangeMax = m_post2DHermes->vecVectorView().get_max_value();

        //Add 20% margin to the range
        double vectorRange = vectorRangeMax - vectorRangeMin;
        vectorRangeMin = vectorRangeMin - 0.2*vectorRange;
        vectorRangeMax = vectorRangeMax + 0.2*vectorRange;

        // qDebug() << "SceneViewCommon::paintVectors(), min = " << vectorRangeMin << ", max = " << vectorRangeMax;

        double irange = 1.0 / (vectorRangeMax - vectorRangeMin);
        // if (fabs(vectorRangeMin - vectorRangeMax) < EPS_ZERO) return;

        RectPoint rect = Util::scene()->boundingBox();
        double gs = (rect.width() + rect.height()) / Util::config()->vectorCount;

        // paint
        m_post2DHermes->vecVectorView().lock_data();

        double4* vecVert = m_post2DHermes->vecVectorView().get_vertices();
        int3* vecTris = m_post2DHermes->vecVectorView().get_triangles();

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_post2DHermes->vecVectorView().get_num_triangles(); i++)
        {
            Point a(vecVert[vecTris[i][0]][0], vecVert[vecTris[i][0]][1]);
            Point b(vecVert[vecTris[i][1]][0], vecVert[vecTris[i][1]][1]);
            Point c(vecVert[vecTris[i][2]][0], vecVert[vecTris[i][2]][1]);

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

            double ax = (aa * vecVert[vecTris[i][0]][2] + ab * vecVert[vecTris[i][1]][2] + ac * vecVert[vecTris[i][2]][2]) / area2;
            double bx = (ba * vecVert[vecTris[i][0]][2] + bb * vecVert[vecTris[i][1]][2] + bc * vecVert[vecTris[i][2]][2]) / area2;
            double cx = (ca * vecVert[vecTris[i][0]][2] + cb * vecVert[vecTris[i][1]][2] + cc * vecVert[vecTris[i][2]][2]) / area2;

            double ay = (aa * vecVert[vecTris[i][0]][3] + ab * vecVert[vecTris[i][1]][3] + ac * vecVert[vecTris[i][2]][3]) / area2;
            double by = (ba * vecVert[vecTris[i][0]][3] + bb * vecVert[vecTris[i][1]][3] + bc * vecVert[vecTris[i][2]][3]) / area2;
            double cy = (ca * vecVert[vecTris[i][0]][3] + cb * vecVert[vecTris[i][1]][3] + cc * vecVert[vecTris[i][2]][3]) / area2;

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

                        double z = (vecVert[vecTris[i][p]][0] - vecVert[vecTris[i][l]][0]) * (point.y - vecVert[vecTris[i][l]][1]) -
                                (vecVert[vecTris[i][p]][1] - vecVert[vecTris[i][l]][1]) * (point.x - vecVert[vecTris[i][l]][0]);

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

                        double value = sqrt(Hermes::sqr(dx) + Hermes::sqr(dy));
                        double angle = atan2(dy, dx);

                        if ((Util::config()->vectorProportional) && (fabs(vectorRangeMin - vectorRangeMax) > EPS_ZERO))
                        {
                            if ((value / vectorRangeMax) < 1e-6)
                            {
                                dx = 0.0;
                                dy = 0.0;
                            }
                            else
                            {
                                dx = ((value - vectorRangeMin) * irange) * Util::config()->vectorScale * gs * cos(angle);
                                dy = ((value - vectorRangeMin) * irange) * Util::config()->vectorScale * gs * sin(angle);
                            }
                        }
                        else
                        {
                            dx = Util::config()->vectorScale * gs * cos(angle);
                            dy = Util::config()->vectorScale * gs * sin(angle);
                        }

                        double dm = sqrt(Hermes::sqr(dx) + Hermes::sqr(dy));

                        // color
                        if ((Util::config()->vectorColor) && (fabs(vectorRangeMin - vectorRangeMax) > EPS_ZERO))
                        {
                            double color = 0.7 - 0.7 * (value - vectorRangeMin) * irange;
                            glColor3d(color, color, color);
                        }
                        else
                        {
                            glColor3d(Util::config()->colorVectors.redF(),
                                      Util::config()->colorVectors.greenF(),
                                      Util::config()->colorVectors.blueF());
                        }

                        // Head for an arrow
                        double vh1x = point.x + dm/5.0 * cos(angle - M_PI/2.0) + dm * cos(angle);
                        double vh1y = point.y + dm/5.0 * sin(angle - M_PI/2.0) + dm * sin(angle);
                        double vh2x = point.x + dm/5.0 * cos(angle + M_PI/2.0) + dm * cos(angle);
                        double vh2y = point.y + dm/5.0 * sin(angle + M_PI/2.0) + dm * sin(angle);
                        double vh3x = point.x + dm * cos(angle) + dm * cos(angle);
                        double vh3y = point.y + dm * sin(angle) + dm * sin(angle);
                        glVertex2d(vh1x,vh1y); glVertex2d(vh2x,vh2y); glVertex2d(vh3x,vh3y);

                        // Shaft for an arrow
                        double vs1x = point.x + dm/15.0 * cos(angle + M_PI/2.0) + dm * cos(angle);
                        double vs1y = point.y + dm/15.0 * sin(angle + M_PI/2.0) + dm * sin(angle);
                        double vs2x = point.x + dm/15.0 * cos(angle - M_PI/2.0) + dm * cos(angle);
                        double vs2y = point.y + dm/15.0 * sin(angle - M_PI/2.0) + dm * sin(angle);
                        double vs3x = vs1x - dm * cos(angle);
                        double vs3y = vs1y - dm * sin(angle);
                        double vs4x = vs2x - dm * cos(angle);
                        double vs4y = vs2y - dm * sin(angle);
                        glVertex2d(vs1x,vs1y); glVertex2d(vs2x,vs2y); glVertex2d(vs3x,vs3y);
                        glVertex2d(vs4x,vs4y); glVertex2d(vs3x,vs3y); glVertex2d(vs2x,vs2y);

                    }
                }
            }
        }
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);

        m_post2DHermes->vecVectorView().unlock_data();

        glEndList();

        glCallList(m_listVectors);
    }
    else
    {
        glCallList(m_listVectors);
    }
}

void SceneViewPost2D::paintPostprocessorSelectedVolume()
{
    if (!Util::problem()->isMeshed()) return;

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4d(Util::config()->colorSelected.redF(),
              Util::config()->colorSelected.greenF(),
              Util::config()->colorSelected.blueF(),
              0.5);

    // triangles
    for (int i = 0, max = Util::problem()->activeMeshInitial()->get_max_element_id(); i < max; i++)
    {
        Hermes::Hermes2D::Element *element;
        if ((element = Util::problem()->activeMeshInitial()->get_element_fast(i))->used)
        {
            if (element->active)
            {
                if (Util::scene()->labels->at(atoi(Util::problem()->activeMeshInitial()->get_element_markers_conversion().get_user_marker(element->marker).marker.c_str()))->isSelected())
                {
                    if (element->is_triangle())
                    {
                        glBegin(GL_TRIANGLES);
                        glVertex2d(element->vn[0]->x, element->vn[0]->y);
                        glVertex2d(element->vn[1]->x, element->vn[1]->y);
                        glVertex2d(element->vn[2]->x, element->vn[2]->y);
                        glEnd();
                    }
                    else
                    {
                        glBegin(GL_QUADS);
                        glVertex2d(element->vn[0]->x, element->vn[0]->y);
                        glVertex2d(element->vn[1]->x, element->vn[1]->y);
                        glVertex2d(element->vn[2]->x, element->vn[2]->y);
                        glVertex2d(element->vn[3]->x, element->vn[3]->y);
                        glEnd();
                    }
                }
            }
        }
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_OFFSET_FILL);

    // how to get marker from linearizer?
    /*
    if (!m_post2DHermes->isMeshed()) return;

    m_post2DHermes->linInitialMeshView().lock_data();

    double3* linVert = m_post2DHermes->linInitialMeshView().get_vertices();
    int3* linTris = m_post2DHermes->linInitialMeshView().get_triangles();

    // draw initial mesh
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4d(Util::config()->colorSelected.redF(),
              Util::config()->colorSelected.greenF(),
              Util::config()->colorSelected.blueF(),
              0.5);

    // triangles
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < Util::problem()->linSolutionMeshView().get_num_triangles(); i++)
    {
        if (Util::scene()->labels[element->marker - 1]->isSelected)
        {
            glVertex2d(linVert[linTris[i][0]][0], linVert[linTris[i][0]][1]);
            glVertex2d(linVert[linTris[i][1]][0], linVert[linTris[i][1]][1]);
            glVertex2d(linVert[linTris[i][2]][0], linVert[linTris[i][2]][1]);
        }
    }
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_OFFSET_FILL);

    Util::problem()->linSolutionMeshView().unlock_data();
    */
}

void SceneViewPost2D::paintPostprocessorSelectedSurface()
{
    // edges
    foreach (SceneEdge *edge, Util::scene()->edges->items()) {
        glColor3d(Util::config()->colorSelected.redF(), Util::config()->colorSelected.greenF(), Util::config()->colorSelected.blueF());
        glLineWidth(3.0);

        if (edge->isSelected())
        {
            if (edge->isStraight())
            {
                glBegin(GL_LINES);
                glVertex2d(edge->nodeStart()->point().x, edge->nodeStart()->point().y);
                glVertex2d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y);
                glEnd();
            }
            else
            {
                Point center = edge->center();
                double radius = edge->radius();
                double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180 - 180;

                drawArc(center, radius, startAngle, edge->angle(), edge->angle()/5);
            }
        }
        glLineWidth(1.0);
    }
}

void SceneViewPost2D::paintPostprocessorSelectedPoint()
{
    glColor3d(Util::config()->colorSelected.redF(), Util::config()->colorSelected.greenF(), Util::config()->colorSelected.blueF());
    glPointSize(8.0);

    glBegin(GL_POINTS);
    glVertex2d(m_selectedPoint.x, m_selectedPoint.y);
    glEnd();
}

void SceneViewPost2D::refresh()
{
    if (m_listContours != -1) glDeleteLists(m_listContours, 1);
    if (m_listVectors != -1) glDeleteLists(m_listVectors, 1);
    if (m_listScalarField != -1) glDeleteLists(m_listScalarField, 1);

    m_listContours = -1;
    m_listVectors = -1;
    m_listScalarField = -1;

    m_post2DHermes->clear();

    setControls();

    if (Util::problem()->isSolved())
    {
        paletteFilter(textureScalar());
        paletteCreate(textureScalar());

        m_post2DHermes->processSolved();
    }

    SceneViewCommon2D::refresh();
}

void SceneViewPost2D::setControls()
{
    // actions
    actSceneModePost2D->setEnabled(Util::problem()->isSolved());
    actPostprocessorModeGroup->setEnabled(Util::problem()->isSolved());
    actSelectByMarker->setEnabled(Util::problem()->isSolved() && (actPostprocessorModeSurfaceIntegral->isChecked() || actPostprocessorModeVolumeIntegral->isChecked()));
    actSelectPoint->setEnabled(Util::problem()->isSolved() && actPostprocessorModeLocalPointValue->isChecked());
    actExportVTKScalar->setEnabled(Util::problem()->isSolved());
}

void SceneViewPost2D::clear()
{
    m_chartLine = ChartLine();

    m_post2DHermes->clear();

    actPostprocessorModeLocalPointValue->trigger();

    setControls();

    SceneViewCommon2D::clear();
}

void SceneViewPost2D::exportVTKScalarView(const QString &fileName)
{
    if (Util::problem()->isSolved())
    {
        QString fn = fileName;

        if (fn.isEmpty())
        {
            // file dialog
            QSettings settings;
            QString dir = settings.value("General/LastVTKDir").toString();

            fn = QFileDialog::getSaveFileName(this, tr("Export vtk file"), dir, tr("VTK files (*.vtk)"));
            if (fn.isEmpty())
                return;

            if (!fn.endsWith(".vtk"))
                fn.append(".vtk");

            // remove existing file
            if (QFile::exists(fn))
                QFile::remove(fn);
        }

        Hermes::Hermes2D::Views::Linearizer linScalarView;
        ViewScalarFilter<double> *slnScalarView = Util::scene()->activeViewField()->module()->viewScalarFilter(Util::scene()->activeViewField()->module()->localVariable(Util::config()->scalarVariable),
                                                                                                                 Util::config()->scalarVariableComp);

        linScalarView.save_solution_vtk(slnScalarView,
                                        fn.toStdString().c_str(),
                                        Util::config()->scalarVariable.toStdString().c_str(),
                                        true);

        delete slnScalarView;

        if (!fn.isEmpty())
        {
            QFileInfo fileInfo(fn);
            if (fileInfo.absoluteDir() != tempProblemDir())
            {
                QSettings settings;
                settings.setValue("General/LastVTKDir", fileInfo.absolutePath());
            }
        }
    }
}

void SceneViewPost2D::selectByMarker()
{
    SceneModePostprocessor mode = (actPostprocessorModeSurfaceIntegral->isChecked()) ? SceneModePostprocessor_SurfaceIntegral : SceneModePostprocessor_VolumeIntegral;

    SceneMarkerSelectDialog sceneMarkerSelectDialog(this, mode, QApplication::activeWindow());
    sceneMarkerSelectDialog.exec();
}

void SceneViewPost2D::selectPoint()
{
    LocalPointValueDialog localPointValueDialog(m_selectedPoint);
    if (localPointValueDialog.exec() == QDialog::Accepted)
    {
        emit mousePressed(localPointValueDialog.point());
        updateGL();
    }
}

void SceneViewPost2D::doPostprocessorModeGroup(QAction *action)
{
    if (actPostprocessorModeLocalPointValue->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_LocalValue);
    if (actPostprocessorModeSurfaceIntegral->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_SurfaceIntegral);
    if (actPostprocessorModeVolumeIntegral->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_VolumeIntegral);

    Util::scene()->selectNone();

    setControls();
    updateGL();
}

void SceneViewPost2D::showGroup(QAction *action)
{
    Util::config()->showSolutionMeshView = actShowSolutionMesh->isChecked();
    Util::config()->showContourView = actShowContours->isChecked();
    Util::config()->showVectorView = actShowVectors->isChecked();

    refresh();
}

void SceneViewPost2D::selectedPoint(const Point &p)
{
    m_selectedPoint = p;
    updateGL();
}
