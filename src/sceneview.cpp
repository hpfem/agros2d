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

#include "sceneview_data.h"
#include "sceneview.h"

SceneViewSettings::SceneViewSettings()
{
    defaultValues();
}

void SceneViewSettings::defaultValues()
{
    scalarRangeMin = 0;
    scalarRangeMax = 1;

    // visible objects
    showGeometry = true;
    showGrid = true;
    showInitialMesh = false;

    postprocessorShow = SceneViewPostprocessorShow_ScalarView;

    showContours = false;
    showVectors = false;
    showSolutionMesh = false;

    contourPhysicFieldVariable = Util::scene()->problemInfo()->hermes()->contourPhysicFieldVariable();

    scalarPhysicFieldVariable = Util::scene()->problemInfo()->hermes()->scalarPhysicFieldVariable();
    scalarPhysicFieldVariableComp = Util::scene()->problemInfo()->hermes()->scalarPhysicFieldVariableComp();
    scalarRangeAuto = true;

    vectorPhysicFieldVariable = Util::scene()->problemInfo()->hermes()->vectorPhysicFieldVariable();
}

// *******************************************************************************************************

SceneView::SceneView(QWidget *parent): QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    m_mainWindow = (QMainWindow *) parent;
    m_scene = Util::scene();

    m_normals = NULL;

    createActions();
    createMenu();

    doDefaultValues();

    setMinimumSize(400, 400);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::DefaultContextMenu);

#ifdef Q_WS_X11
    setFont(QFont("Monospace", 9));
#endif
#ifdef Q_WS_WIN
    setFont(QFont("Courier New", 9));
#endif
#ifdef Q_WS_MAC
    setFont(QFont("Monaco", 12));
#endif
}

SceneView::~SceneView()
{
}

void SceneView::createActions()
{
    // scene - zoom
    actSceneZoomIn = new QAction(icon("zoom-in"), tr("Zoom in"), this);
    actSceneZoomIn->setShortcut(QKeySequence::ZoomIn);
    actSceneZoomIn->setStatusTip(tr("Zoom in"));
    connect(actSceneZoomIn, SIGNAL(triggered()), this, SLOT(doZoomIn()));

    actSceneZoomOut = new QAction(icon("zoom-out"), tr("Zoom out"), this);
    actSceneZoomOut->setShortcut(QKeySequence::ZoomOut);
    actSceneZoomOut->setStatusTip(tr("Zoom out"));
    connect(actSceneZoomOut, SIGNAL(triggered()), this, SLOT(doZoomOut()));

    actSceneZoomBestFit = new QAction(icon("zoom-original"), tr("Zoom best fit"), this);
    actSceneZoomBestFit->setStatusTip(tr("Best fit"));
    connect(actSceneZoomBestFit, SIGNAL(triggered()), this, SLOT(doZoomBestFit()));

    actSceneZoomRegion = new QAction(icon("zoom-best-fit"), tr("Zoom region"), this);
    actSceneZoomRegion->setStatusTip(tr("Zoom region"));
    actSceneZoomRegion->setCheckable(true);

    // scene - operate on items
    actSceneModeNode = new QAction(icon("scene-node"), tr("Operate on &nodes"), this);
    actSceneModeNode->setShortcut(Qt::Key_F5);
    actSceneModeNode->setStatusTip(tr("Operate on nodes"));
    actSceneModeNode->setCheckable(true);

    actSceneModeEdge = new QAction(icon("scene-edge"), tr("Operate on &edges"), this);
    actSceneModeEdge->setShortcut(Qt::Key_F6);
    actSceneModeEdge->setStatusTip(tr("Operate on edges"));
    actSceneModeEdge->setCheckable(true);

    actSceneModeLabel = new QAction(icon("scene-label"), tr("Operate on &labels"), this);
    actSceneModeLabel->setShortcut(Qt::Key_F7);
    actSceneModeLabel->setStatusTip(tr("Operate on labels"));
    actSceneModeLabel->setCheckable(true);

    actSceneModePostprocessor = new QAction(icon("scene-postprocessor"), tr("&Postprocessor"), this);
    actSceneModePostprocessor->setShortcut(Qt::Key_F8);
    actSceneModePostprocessor->setStatusTip(tr("Postprocessor"));
    actSceneModePostprocessor->setCheckable(true);

    actSceneModeGroup = new QActionGroup(this);
    actSceneModeGroup->addAction(actSceneModeNode);
    actSceneModeGroup->addAction(actSceneModeEdge);
    actSceneModeGroup->addAction(actSceneModeLabel);
    actSceneModeGroup->addAction(actSceneModePostprocessor);
    connect(actSceneModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(doSceneModeSet(QAction *)));

    // material
    actMaterialGroup = new QActionGroup(this);
    connect(actMaterialGroup, SIGNAL(triggered(QAction *)), this, SLOT(doMaterialGroup(QAction *)));

    // boundary
    actBoundaryGroup = new QActionGroup(this);
    connect(actBoundaryGroup, SIGNAL(triggered(QAction *)), this, SLOT(doBoundaryGroup(QAction *)));

    // show
    actShowSolutionMesh = new QAction(tr("Solution mesh"), this);
    actShowSolutionMesh->setCheckable(true);

    actShowContours = new QAction(tr("Contours"), this);
    actShowContours->setCheckable(true);

    actShowVectors = new QAction(tr("Vectors"), this);
    actShowVectors->setCheckable(true);

    actShowGroup = new QActionGroup(this);
    actShowGroup->setExclusive(false);
    connect(actShowGroup, SIGNAL(triggered(QAction *)), this, SLOT(doShowGroup(QAction *)));
    actShowGroup->addAction(actShowSolutionMesh);
    actShowGroup->addAction(actShowContours);
    actShowGroup->addAction(actShowVectors);

    // postprocessor group
    actPostprocessorModeLocalPointValue = new QAction(icon("mode-localpointvalue"), tr("Local Values"), this);
    actPostprocessorModeLocalPointValue->setCheckable(true);

    actPostprocessorModeSurfaceIntegral = new QAction(icon("mode-surfaceintegral"), tr("Surface Integrals"), this);
    actPostprocessorModeSurfaceIntegral->setCheckable(true);

    actPostprocessorModeVolumeIntegral = new QAction(icon("mode-volumeintegral"), tr("Volume Integrals"), this);
    actPostprocessorModeVolumeIntegral->setCheckable(true);

    actPostprocessorModeGroup = new QActionGroup(this);
    connect(actPostprocessorModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(doPostprocessorModeGroup(QAction*)));
    actPostprocessorModeGroup->addAction(actPostprocessorModeLocalPointValue);
    actPostprocessorModeGroup->addAction(actPostprocessorModeSurfaceIntegral);
    actPostprocessorModeGroup->addAction(actPostprocessorModeVolumeIntegral);

    // scene properties
    actSceneViewProperties = new QAction(icon("scene-properties"), tr("&Scene properties"), this);
    actSceneViewProperties->setShortcut(Qt::Key_F12);
    connect(actSceneViewProperties, SIGNAL(triggered()), this, SLOT(doSceneViewProperties()));

    // object properties
    actSceneObjectProperties = new QAction(icon("scene-properties"), tr("Object properties"), this);
    connect(actSceneObjectProperties, SIGNAL(triggered()), this, SLOT(doSceneObjectProperties()));

    // select region
    actSceneViewSelectRegion = new QAction(icon("scene-select-region"), tr("&Select region"), this);
    actSceneViewSelectRegion->setStatusTip(tr("Select region"));
    actSceneViewSelectRegion->setCheckable(true);

    actSceneViewSelectMarker = new QAction(icon(""), tr("Select by marker"), this);
    actSceneViewSelectMarker->setStatusTip(tr("Select by marker"));
    connect(actSceneViewSelectMarker, SIGNAL(triggered()), this, SLOT(doSelectMarker()));
}

void SceneView::createMenu()
{
    mnuInfo = new QMenu(this);

    QMenu *mnuModeGroup = new QMenu(tr("Set mode"), this);
    mnuModeGroup->addAction(actSceneModeNode);
    mnuModeGroup->addAction(actSceneModeEdge);
    mnuModeGroup->addAction(actSceneModeLabel);
    mnuModeGroup->addAction(actSceneModePostprocessor);

    mnuInfo->addAction(m_scene->actNewNode);
    mnuInfo->addAction(m_scene->actNewEdge);
    mnuInfo->addAction(m_scene->actNewLabel);
    mnuInfo->addSeparator();
    mnuInfo->addAction(m_scene->actNewEdgeMarker);
    mnuInfo->addAction(m_scene->actNewLabelMarker);
    mnuInfo->addSeparator();
    mnuInfo->addAction(actSceneViewSelectRegion);
    mnuInfo->addAction(m_scene->actTransform);
    mnuInfo->addSeparator();
    mnuInfo->addMenu(mnuModeGroup);
    mnuInfo->addSeparator();
    mnuInfo->addAction(actSceneObjectProperties);
    mnuInfo->addAction(m_scene->actProblemProperties);
    mnuInfo->addAction(actSceneViewProperties);
}

void SceneView::initializeGL()
{
    glShadeModel(GL_SMOOTH);    
}

void SceneView::resizeGL(int w, int h)
{
    setupViewport(w, h);

    if (m_scene->sceneSolution()->isSolved())
    {
        paletteFilter();
        paletteUpdateTexAdjust();
        paletteCreate();

        updateGL();
    }
}

void SceneView::setupViewport(int w, int h)
{
    glViewport(0, 0, w, h);

    m_aspect = (double) w/(double) h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if ((m_sceneMode == SceneMode_Postprocessor) &&
        (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3D || 
         m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid))
    {
        gluPerspective(0.0, m_aspect, 1.0, 1000.0);
    }
    else
    {
        glOrtho(3.0, w-6.0, h-6.0, 3.0, -10.0, -10.0);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void SceneView::paintGL()
{
    glClearColor(Util::config()->colorBackground.redF(), Util::config()->colorBackground.greenF(), Util::config()->colorBackground.blueF(), 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if ((m_sceneMode == SceneMode_Postprocessor) &&
        (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3D || m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid))
    {
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    // transform
    glLoadIdentity();
    glScaled(m_scale/m_aspect, m_scale, m_scale);

    if ((m_sceneMode == SceneMode_Postprocessor) &&
        (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3D || m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid))
    {
        glRotated(m_rotation.x, 1.0, 0.0, 0.0);
        glRotated(m_rotation.y, 0.0, 0.0, 1.0);
    }

    glTranslated(-m_offset.x, -m_offset.y, -m_offset.z);

    if (m_sceneViewSettings.showGrid) paintGrid();
    if (m_scene->sceneSolution()->isSolved())
    {
        if (m_sceneMode == SceneMode_Postprocessor)
        {
            switch (m_sceneViewSettings.postprocessorShow)
            {
            case SceneViewPostprocessorShow_ScalarView:
                paintScalarField();
                break;
            case SceneViewPostprocessorShow_ScalarView3D:
                paintScalarField3D();
                break;
            case SceneViewPostprocessorShow_ScalarView3DSolid:
                paintScalarField3DSolid();
                break;
            case SceneViewPostprocessorShow_Order:
                paintOrder();
                break;
            }

            if (m_sceneViewSettings.showContours) paintContours();
            if (m_sceneViewSettings.showVectors) paintVectors();
            if (m_sceneViewSettings.showSolutionMesh) paintSolutionMesh();
        }
    }

    if (m_sceneViewSettings.showInitialMesh) paintInitialMesh();
    if (m_sceneViewSettings.showGeometry) paintGeometry();

    if (m_sceneMode == SceneMode_Postprocessor)
    {
        if (actPostprocessorModeVolumeIntegral->isChecked()) paintPostprocessorSelectedVolume();
        if (actPostprocessorModeSurfaceIntegral->isChecked()) paintPostprocessorSelectedSurface();
    }

    if (Util::config()->showRulers) paintRulers();

    paintZoomRegion();
    paintSnapToGrid();
    paintChartLine();
    paintSceneModeLabel();
}


void SceneView::clearGLLists()
{
    glDeleteLists(1, 5);

    m_listContours = -1;
    m_listVectors = -1;
    m_listScalarField = -1;
    m_listScalarField3D = -1;
    m_listOrder = -1;

    updateGL();
}

// paint *****************************************************************************************************************************

void SceneView::paintGrid()
{
    Point cornerMin = position(Point(0, 0));
    Point cornerMax = position(Point(contextWidth(), contextHeight()));

    int step = (((int) ((cornerMax - cornerMin).x / Util::config()->gridStep) + 1) / 5);
    if (step > 0.0)
    {
        glColor3f(Util::config()->colorGrid.redF(),
                  Util::config()->colorGrid.greenF(),
                  Util::config()->colorGrid.blueF());
        glLineWidth(1.0);
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, 0x1C47);
        glBegin(GL_LINES);

        if ((((cornerMax.x-cornerMin.x)/Util::config()->gridStep + (cornerMin.y-cornerMax.y)/Util::config()->gridStep) < 200) &&
             ((cornerMax.x-cornerMin.x)/Util::config()->gridStep > 0) && ((cornerMin.y-cornerMax.y)/Util::config()->gridStep > 0))
        {
            // vertical lines
            for (int i = 0; i<cornerMax.x/Util::config()->gridStep; i++)
            {
                if ((step > 0) && i % step == 0)
                    glColor3f(Util::config()->colorCross.redF(),
                              Util::config()->colorCross.greenF(),
                              Util::config()->colorCross.blueF());
                else
                    glColor3f(Util::config()->colorGrid.redF(),
                              Util::config()->colorGrid.greenF(),
                              Util::config()->colorGrid.blueF());
                glVertex2d(i*Util::config()->gridStep, cornerMin.y);
                glVertex2d(i*Util::config()->gridStep, cornerMax.y);
            }
            for (int i = 0; i>cornerMin.x/Util::config()->gridStep; i--)
            {
                if ((step > 0) && i % step == 0)
                    glColor3f(Util::config()->colorCross.redF(),
                              Util::config()->colorCross.greenF(),
                              Util::config()->colorCross.blueF());
                else
                    glColor3f(Util::config()->colorGrid.redF(),
                              Util::config()->colorGrid.greenF(),
                              Util::config()->colorGrid.blueF());
                glVertex2d(i*Util::config()->gridStep, cornerMin.y);
                glVertex2d(i*Util::config()->gridStep, cornerMax.y);
            }

            // horizontal lines
            for (int i = 0; i<cornerMin.y/Util::config()->gridStep; i++)
            {
                if ((step > 0) && i % step == 0)
                    glColor3f(Util::config()->colorCross.redF(),
                              Util::config()->colorCross.greenF(),
                              Util::config()->colorCross.blueF());
                else
                    glColor3f(Util::config()->colorGrid.redF(),
                              Util::config()->colorGrid.greenF(),
                              Util::config()->colorGrid.blueF());
                glVertex2d(cornerMin.x, i*Util::config()->gridStep);
                glVertex2d(cornerMax.x, i*Util::config()->gridStep);
            }
            for (int i = 0; i>cornerMax.y/Util::config()->gridStep; i--)
            {
                if ((step > 0) && i % step == 0)
                    glColor3f(Util::config()->colorCross.redF(),
                              Util::config()->colorCross.greenF(),
                              Util::config()->colorCross.blueF());
                else
                    glColor3f(Util::config()->colorGrid.redF(),
                              Util::config()->colorGrid.greenF(),
                              Util::config()->colorGrid.blueF());
                glVertex2d(cornerMin.x, i*Util::config()->gridStep);
                glVertex2d(cornerMax.x, i*Util::config()->gridStep);
            }
        }
        glEnd();
        glDisable(GL_LINE_STIPPLE);
    }

    // axes
    glColor3f(Util::config()->colorCross.redF(),
              Util::config()->colorCross.greenF(),
              Util::config()->colorCross.blueF());
    glLineWidth(1.0);
    glBegin(GL_LINES);
    // y axis
    glVertex2d(0, cornerMin.y);
    glVertex2d(0, cornerMax.y);
    // x axis
    glVertex2d(cornerMin.x, 0);
    glVertex2d(cornerMax.x, 0);
    glEnd();
}

void SceneView::paintRulers()
{
    Point cornerMin = position(Point(0, 0));
    Point cornerMax = position(Point(contextWidth(), contextHeight()));

    // rulers
    double step = (((int) ((cornerMax - cornerMin).x / Util::config()->gridStep) + 1) / 5) * Util::config()->gridStep;

    Point size((2.0/contextWidth()*fontMetrics().width(" "))/m_scale*m_aspect,
               (2.0/contextHeight()*fontMetrics().height())/m_scale);

    if (step > 0.0)
    {
        QString text;

        if (((cornerMax.x-cornerMin.x)/step > 0) && ((cornerMin.y-cornerMax.y)/step > 0))
        {
            glColor3f(0.3, 0.2, 0.0);

            // horizontal ticks
            for (int i = 0; i<cornerMax.x/step; i++)
            {
                text = QString::number(i*step, 'g', 4);
                renderTextPos(i*step - size.x*text.size() / 2.0, cornerMax.y + size.x / 4.0, 0.0, text);
            }
            for (int i = 0; i>cornerMin.x/step; i--)
            {
                text = QString::number(i*step, 'g', 4);
                renderTextPos(i*step - size.x*text.size() / 2.0, cornerMax.y + size.x / 4.0, 0.0, text);
            }

            // vertical ticks
            for (int i = 0; i<cornerMin.y/step; i++)
            {
                text = QString::number(i*step, 'g', 4);
                renderTextPos(cornerMin.x + size.x/4.0, i*step - size.y / 4.0, 0.0, text);
            }
            for (int i = 0; i>cornerMax.y/step; i--)
            {
                text = QString::number(i*step, 'g', 4);
                renderTextPos(cornerMin.x + size.x/4.0, i*step - size.y / 4.0, 0.0, text);
            }
        }
    }
}

void SceneView::paintGeometry()
{
    // edges
    foreach (SceneEdge *edge, m_scene->edges)
    {
        // edge with marker
        if (m_sceneMode == SceneMode_OperateOnEdges && edge->marker->type == PhysicFieldBC_None)
        {
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, 0x8FFF);
        }

        glColor3f(Util::config()->colorEdges.redF(),
                  Util::config()->colorEdges.greenF(),
                  Util::config()->colorEdges.blueF());
        glLineWidth(Util::config()->edgeWidth);
        if (edge->isHighlighted)
        {
            glColor3f(Util::config()->colorHighlighted.redF(),
                      Util::config()->colorHighlighted.greenF(),
                      Util::config()->colorHighlighted.blueF());
            glLineWidth(Util::config()->edgeWidth + 2.0);
        }
        if (edge->isSelected)
        {
            glColor3f(Util::config()->colorSelected.redF(),
                      Util::config()->colorSelected.greenF(),
                      Util::config()->colorSelected.blueF());
            glLineWidth(Util::config()->edgeWidth + 2.0);
        }

        if (edge->angle == 0)
        {
            glBegin(GL_LINES);
            glVertex2d(edge->nodeStart->point.x, edge->nodeStart->point.y);
            glVertex2d(edge->nodeEnd->point.x, edge->nodeEnd->point.y);
            glEnd();
        }
        else
        {
            Point center = edge->center();
            double radius = edge->radius();
            double startAngle = atan2(center.y - edge->nodeStart->point.y, center.x - edge->nodeStart->point.x) / M_PI*180.0 - 180.0;

            drawArc(center, radius, startAngle, edge->angle, edge->angle/2);
        }

        glDisable(GL_LINE_STIPPLE);
        glLineWidth(1.0);
    }

    // nodes
    if (!(m_sceneMode == SceneMode_Postprocessor))
    {
        foreach (SceneNode *node, m_scene->nodes)
        {
            glColor3f(Util::config()->colorNodes.redF(),
                      Util::config()->colorNodes.greenF(),
                      Util::config()->colorNodes.blueF());
            glPointSize(Util::config()->nodeSize);
            glBegin(GL_POINTS);
            glVertex2d(node->point.x, node->point.y);
            glEnd();

            glColor3f(Util::config()->colorBackground.redF(),
                      Util::config()->colorBackground.greenF(),
                      Util::config()->colorBackground.blueF());
            glPointSize(Util::config()->nodeSize - 2.0);
            glBegin(GL_POINTS);
            glVertex2d(node->point.x, node->point.y);
            glEnd();

            if ((node->isSelected) || (node->isHighlighted))
            {
                if (node->isHighlighted)
                    glColor3f(Util::config()->colorHighlighted.redF(),
                              Util::config()->colorHighlighted.greenF(),
                              Util::config()->colorHighlighted.blueF());
                if (node->isSelected)
                    glColor3f(Util::config()->colorSelected.redF(),
                              Util::config()->colorSelected.greenF(),
                              Util::config()->colorSelected.blueF());

                glPointSize(Util::config()->nodeSize - 2.0);
                glBegin(GL_POINTS);
                glVertex2d(node->point.x, node->point.y);
                glEnd();
            }
        }

        glLineWidth(1.0);
    }
    // labels
    if (!(m_sceneMode == SceneMode_Postprocessor))
    {
        foreach (SceneLabel *label, m_scene->labels)
        {
            glColor3f(Util::config()->colorLabels.redF(),
                      Util::config()->colorLabels.greenF(),
                      Util::config()->colorLabels.blueF());
            glPointSize(Util::config()->labelSize);
            glBegin(GL_POINTS);
            glVertex2d(label->point.x, label->point.y);
            glEnd();

            glColor3f(Util::config()->colorBackground.redF(),
                      Util::config()->colorBackground.greenF(),
                      Util::config()->colorBackground.blueF());
            glPointSize(Util::config()->labelSize - 2.0);
            glBegin(GL_POINTS);
            glVertex2d(label->point.x, label->point.y);
            glEnd();

            if ((label->isSelected) || (label->isHighlighted))
            {
                if (label->isHighlighted)
                    glColor3f(Util::config()->colorHighlighted.redF(),
                              Util::config()->colorHighlighted.greenF(),
                              Util::config()->colorHighlighted.blueF());
                if (label->isSelected)
                    glColor3f(Util::config()->colorSelected.redF(),
                              Util::config()->colorSelected.greenF(),
                              Util::config()->colorSelected.blueF());

                glPointSize(Util::config()->labelSize - 2.0);
                glBegin(GL_POINTS);
                glVertex2d(label->point.x, label->point.y);
                glEnd();
            }
            glLineWidth(1.0);

            if (m_sceneMode == SceneMode_OperateOnLabels)
            {
                glColor3f(0.1, 0.1, 0.1);

                Point point;
                point.x = 2.0/contextWidth()*m_aspect*fontMetrics().width(label->marker->name)/m_scale/2.0;
                point.y = 2.0/contextHeight()*fontMetrics().height()/m_scale;

                renderTextPos(label->point.x-point.x, label->point.y-point.y, 0.0, label->marker->name, false);
            }

            // area size
            if ((m_sceneMode == SceneMode_OperateOnLabels) || (m_sceneViewSettings.showInitialMesh))
            {
                double radius = sqrt(label->area/M_PI);
                glColor3f(0, 0.95, 0.9);
                glBegin(GL_LINE_LOOP);
                for (int i = 0; i<360; i = i + 10)
                {
                    glVertex2d(label->point.x + radius*cos(i/180.0*M_PI), label->point.y + radius*sin(i/180.0*M_PI));
                }
                glEnd();
            }
        }
    }
}

void SceneView::paintInitialMesh()
{
    if (!m_scene->sceneSolution()->isMeshed()) return;

    // draw initial mesh    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(Util::config()->colorInitialMesh.redF(),
              Util::config()->colorInitialMesh.greenF(),
              Util::config()->colorInitialMesh.blueF());
    glLineWidth(1.3);

    // triangles
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < m_scene->sceneSolution()->mesh()->get_num_elements(); i++)
    {
        Element *element = m_scene->sceneSolution()->mesh()->get_element(i);
        if (element->is_triangle())
        {
            glVertex2d(element->vn[0]->x, element->vn[0]->y);
            glVertex2d(element->vn[1]->x, element->vn[1]->y);
            glVertex2d(element->vn[2]->x, element->vn[2]->y);
        }
    }
    glEnd();
}

void SceneView::paintSolutionMesh()
{
    if (!m_isSolutionPrepared) return;

    // draw solution mesh
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(Util::config()->colorSolutionMesh.redF(),
              Util::config()->colorSolutionMesh.greenF(),
              Util::config()->colorSolutionMesh.blueF());
    glLineWidth(1.0);

    // triangles
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < m_scene->sceneSolution()->sln()->get_mesh()->get_num_elements(); i++)
    {
        Element *element = m_scene->sceneSolution()->sln()->get_mesh()->get_element(i);
        if (element->is_triangle())
        {
            glVertex2d(element->vn[0]->x, element->vn[0]->y);
            glVertex2d(element->vn[1]->x, element->vn[1]->y);
            glVertex2d(element->vn[2]->x, element->vn[2]->y);
        }
    }
    glEnd();
}

void SceneView::paintOrder()
{
    if (!m_isSolutionPrepared) return;

    if (m_listOrder == -1)
    {
        m_listOrder = glGenLists(1);
        glNewList(m_listOrder, GL_COMPILE);

        // order scalar view
        m_scene->sceneSolution()->ordView().lock_data();

        double3* vert = m_scene->sceneSolution()->ordView().get_vertices();
        int3* tris = m_scene->sceneSolution()->ordView().get_triangles();

        // draw mesh
        int min = 11;
        int max = 1;
        for (int i = 0; i < m_scene->sceneSolution()->ordView().get_num_triangles(); i++)
        {
            if (vert[tris[i][0]][2] < min) min = vert[tris[i][0]][2];
            if (vert[tris[i][0]][2] > max) max = vert[tris[i][0]][2];
        }

        // triangles
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_scene->sceneSolution()->ordView().get_num_triangles(); i++)
        {
            int color = vert[tris[i][0]][2];
            glColor3d(paletteOrder[color][0], paletteOrder[color][1], paletteOrder[color][2]);

            glVertex2d(vert[tris[i][0]][0], vert[tris[i][0]][1]);
            glVertex2d(vert[tris[i][1]][0], vert[tris[i][1]][1]);
            glVertex2d(vert[tris[i][2]][0], vert[tris[i][2]][1]);
        }
        glEnd();
        m_scene->sceneSolution()->ordView().unlock_data();

        // order color map
        glPushMatrix();
        glLoadIdentity();

        glScaled(2.0 / contextWidth(), 2.0 / contextHeight(), 1.0);
        glTranslated(- contextWidth() / 2.0, -contextHeight() / 2.0, 0.0);

        glDisable(GL_DEPTH_TEST);

        // dimensions
        int textWidth = fontMetrics().width("00");
        int textHeight = fontMetrics().height();
        Point scaleSize = Point(20 + 3 * textWidth, (20 + max * (2 * textHeight) - textHeight / 2.0 + 2));
        Point scaleBorder = Point(10.0, 10.0);
        double scaleLeft = (contextWidth() - (20 + 3 * textWidth));

        // blended rectangle
        drawBlend(Point(scaleLeft, scaleBorder.y), Point(scaleLeft + scaleSize.x - scaleBorder.x, scaleBorder.y + scaleSize.y),
                  0.91, 0.91, 0.91);

        // bars
        glBegin(GL_QUADS);
        for (int i = 1; i < max+1; i++)
        {
            glColor3f(0.0, 0.0, 0.0);
            glVertex2d(scaleLeft + 10,                                 scaleBorder.y + 10 + (i-1)*(2 * textHeight));
            glVertex2d(scaleLeft + 10 + 3 * textWidth - scaleBorder.x, scaleBorder.y + 10 + (i-1)*(2 * textHeight));
            glVertex2d(scaleLeft + 10 + 3 * textWidth - scaleBorder.x, scaleBorder.y + 12 + (i )*(2 * textHeight) - textHeight / 2.0);
            glVertex2d(scaleLeft + 10,                                 scaleBorder.y + 12 + (i )*(2 * textHeight) - textHeight / 2.0);

            glColor3d(paletteOrder[i][0], paletteOrder[i][1], paletteOrder[i][2]);
            glVertex2d(scaleLeft + 12,                                     scaleBorder.y + 12 + (i-1)*(2 * textHeight));
            glVertex2d(scaleLeft + 10 + 3 * textWidth - 2 - scaleBorder.x, scaleBorder.y + 12 + (i-1)*(2 * textHeight));
            glVertex2d(scaleLeft + 10 + 3 * textWidth - 2 - scaleBorder.x, scaleBorder.y + 10 + (i  )*(2 * textHeight) - textHeight / 2.0);
            glVertex2d(scaleLeft + 12,                                     scaleBorder.y + 10 + (i  )*(2 * textHeight) - textHeight / 2.0);
        }
        glEnd();

        // labels
        glColor3f(0.0, 0.0, 0.0);
        for (int i = 1; i < max + 1; i++)
        {
            int sizeNumber = fontMetrics().width(QString::number(i));
            renderText(scaleLeft + 10 + 1.5 * textWidth - sizeNumber,
                       scaleBorder.y + 10.0 + (i-1)*(2.0 * textHeight) + textHeight / 2.0,
                       0.0,
                       QString::number(i));
        }

        glPopMatrix();

        glEndList();
    }
    else
    {
        glCallList(m_listOrder);
    }
}

void SceneView::paintColorBar(double min, double max)
{
    glPushMatrix();
    glLoadIdentity();

    glScaled(2.0 / contextWidth(), 2.0 / contextHeight(), 1.0);
    glTranslated(-contextWidth() / 2.0, -contextHeight() / 2.0, 0.0);

    glDisable(GL_DEPTH_TEST);

    // dimensions
    int textWidth = fontMetrics().width(QString::number(-1.0, '+e', 1)) + 3;
    int textHeight = fontMetrics().height();
    Point scaleSize = Point(45.0 + textWidth, contextHeight() - 20.0);
    Point scaleBorder = Point(10.0, 10.0);
    double scaleLeft = (contextWidth() - (45.0 + textWidth));
    int numTicks = 11;

    // blended rectangle
    drawBlend(Point(scaleLeft, scaleBorder.y), Point(scaleLeft + scaleSize.x - scaleBorder.x, scaleBorder.y + scaleSize.y),
              0.91, 0.91, 0.91);

    // palette border
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + 10.0);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + 10.0);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 10.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 10.0);
    glEnd();

    // palette
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, 1);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBegin(GL_QUADS);
    if (fabs(m_sceneViewSettings.scalarRangeMin - m_sceneViewSettings.scalarRangeMax) > EPS_ZERO)
        glTexCoord1d(m_texScale + m_texShift);
    else
        glTexCoord1d(m_texShift);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + scaleSize.y - 12.0);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + scaleSize.y - 12.0);
    glTexCoord1d(m_texShift);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + 12.0);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + 12.0);
    glEnd();
    glDisable(GL_TEXTURE_1D);

    // ticks
    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (int i = 1; i < numTicks+1; i++)
    {
        double tickY = (scaleSize.y - 20.0) / (numTicks - 1.0);

        glVertex2d(scaleLeft + 10.0, scaleBorder.y + 10.0 + i*tickY);
        glVertex2d(scaleLeft + 15.0, scaleBorder.y + 10.0 + i*tickY);
        glVertex2d(scaleLeft + 25.0, scaleBorder.y + 10.0 + i*tickY);
        glVertex2d(scaleLeft + 30.0, scaleBorder.y + 10.0 + i*tickY);
    }
    glEnd();

    // labels
    for (int i = 1; i < numTicks+1; i++)
    {
        double value = 0.0;
        if (!Util::config()->scalarRangeLog)
            value = min + (double) (i-1) / (numTicks-1) * (max - min);
        else
            value = min + (double) pow(Util::config()->scalarRangeBase, ((i-1) / (numTicks-1)))/Util::config()->scalarRangeBase * (max - min);

        if (fabs(value) < EPS_ZERO) value = 0.0;
        double tickY = (scaleSize.y - 20.0) / (numTicks - 1.0);

        renderText(scaleLeft + 33.0 + ((value >= 0.0) ? fontMetrics().width("-") : 0.0),
                   scaleBorder.y + 10.0 + (i-1)*tickY - textHeight / 4.0,
                   0.0,
                   QString::number(value, '+e', 1));
    }

    glPopMatrix();
}

void SceneView::paintScalarField()
{
    if (!m_isSolutionPrepared) return;

    if (m_listScalarField == -1)
    {
        m_listScalarField = glGenLists(1);
        glNewList(m_listScalarField, GL_COMPILE);

        // range
        double irange = 1.0 / (m_sceneViewSettings.scalarRangeMax - m_sceneViewSettings.scalarRangeMin);
        // special case: constant solution
        if (fabs(m_sceneViewSettings.scalarRangeMin - m_sceneViewSettings.scalarRangeMax) < EPS_ZERO)
            irange = 1.0;

        m_scene->sceneSolution()->linScalarView().lock_data();

        double3* linVert = m_scene->sceneSolution()->linScalarView().get_vertices();
        int3* linTris = m_scene->sceneSolution()->linScalarView().get_triangles();
        Point point[3];
        double value[3];

        glEnable(GL_TEXTURE_1D);
        glBindTexture(GL_TEXTURE_1D, 1);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glPolygonOffset(1.0, 1.0);

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_scene->sceneSolution()->linScalarView().get_num_triangles(); i++)
        {
            point[0].x = linVert[linTris[i][0]][0];
            point[0].y = linVert[linTris[i][0]][1];
            value[0]   = linVert[linTris[i][0]][2];
            point[1].x = linVert[linTris[i][1]][0];
            point[1].y = linVert[linTris[i][1]][1];
            value[1]   = linVert[linTris[i][1]][2];
            point[2].x = linVert[linTris[i][2]][0];
            point[2].y = linVert[linTris[i][2]][1];
            value[2]   = linVert[linTris[i][2]][2];

            if (!m_sceneViewSettings.scalarRangeAuto)
            {
                double avgValue = (value[0] + value[1] + value[2]) / 3.0;
                if (avgValue < m_sceneViewSettings.scalarRangeMin || avgValue > m_sceneViewSettings.scalarRangeMax)
                    continue;
            }

            if (Util::config()->scalarRangeLog)
                glTexCoord2d(log10(1.0 + (Util::config()->scalarRangeBase-1.0)*(value[0] - m_sceneViewSettings.scalarRangeMin) * irange)/log10(Util::config()->scalarRangeBase) * m_texScale + m_texShift, 0.0);
            else
                glTexCoord2d((value[0] - m_sceneViewSettings.scalarRangeMin) * irange * m_texScale + m_texShift, 0.0);
            glVertex2d(point[0].x, point[0].y);

            if (Util::config()->scalarRangeLog)
                glTexCoord2d(log10(1.0 + (Util::config()->scalarRangeBase-1.0)*(value[1] - m_sceneViewSettings.scalarRangeMin) * irange)/log10(Util::config()->scalarRangeBase) * m_texScale + m_texShift, 0.0);
            else
                glTexCoord2d((value[1] - m_sceneViewSettings.scalarRangeMin) * irange * m_texScale + m_texShift, 0.0);
            glVertex2d(point[1].x, point[1].y);

            if (Util::config()->scalarRangeLog)
                glTexCoord2d(log10(1.0 + (Util::config()->scalarRangeBase-1.0)*(value[2] - m_sceneViewSettings.scalarRangeMin) * irange)/log10(Util::config()->scalarRangeBase) * m_texScale + m_texShift, 0.0);
            else
                glTexCoord2d((value[2] - m_sceneViewSettings.scalarRangeMin) * irange * m_texScale + m_texShift, 0.0);
            glVertex2d(point[2].x, point[2].y);
        }
        glEnd();
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_TEXTURE_1D);

        m_scene->sceneSolution()->linScalarView().unlock_data();

        glEndList();
    }
    else
    {
        glCallList(m_listScalarField);
    }

    paintColorBar(m_sceneViewSettings.scalarRangeMin, m_sceneViewSettings.scalarRangeMax);
}

void SceneView::paintScalarField3D()
{
    if (!m_isSolutionPrepared) return;

    if (m_listScalarField3D == -1)
    {
        m_listScalarField3D = glGenLists(1);
        glNewList(m_listScalarField3D, GL_COMPILE);

        // range
        double irange = 1.0 / (m_sceneViewSettings.scalarRangeMax - m_sceneViewSettings.scalarRangeMin);
        // special case: constant solution
        if (fabs(m_sceneViewSettings.scalarRangeMin - m_sceneViewSettings.scalarRangeMax) < 1e-8) { irange = 1.0; m_sceneViewSettings.scalarRangeMin -= 0.5; }

        m_scene->sceneSolution()->linScalarView().lock_data();

        double3* linVert = m_scene->sceneSolution()->linScalarView().get_vertices();
        int3* linTris = m_scene->sceneSolution()->linScalarView().get_triangles();
        Point point[3];
        double value[3];

        double max = qMax(m_scene->boundingBox().width(), m_scene->boundingBox().height());

        if (Util::config()->scalarView3DLighting)
        {
            glEnable(GL_LIGHTING);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
        else
        {
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        }

        glPushMatrix();
        glScaled(1.0, 1.0, max/4.0 * 1.0/(fabs(m_sceneViewSettings.scalarRangeMin - m_sceneViewSettings.scalarRangeMax)));

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_1D);
        glBindTexture(GL_TEXTURE_1D, 1);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0, 1.0);

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_scene->sceneSolution()->linScalarView().get_num_triangles(); i++)
        {
            point[0].x = linVert[linTris[i][0]][0];
            point[0].y = linVert[linTris[i][0]][1];
            value[0]   = linVert[linTris[i][0]][2];
            point[1].x = linVert[linTris[i][1]][0];
            point[1].y = linVert[linTris[i][1]][1];
            value[1]   = linVert[linTris[i][1]][2];
            point[2].x = linVert[linTris[i][2]][0];
            point[2].y = linVert[linTris[i][2]][1];
            value[2]   = linVert[linTris[i][2]][2];

            if (!m_sceneViewSettings.scalarRangeAuto)
            {
                double avgValue = (value[0] + value[1] + value[2]) / 3.0;
                if (avgValue < m_sceneViewSettings.scalarRangeMin || avgValue > m_sceneViewSettings.scalarRangeMax)
                    continue;
            }

            double delta = 0.0;

            if (Util::config()->scalarView3DLighting)
                glNormal3d(m_normals[linTris[i][0]][0], m_normals[linTris[i][0]][1], -m_normals[linTris[i][0]][2]);
            glTexCoord2d((value[0] - m_sceneViewSettings.scalarRangeMin) * irange * m_texScale + m_texShift, 0.0);
            glVertex3d(point[0].x, point[0].y, - delta - (value[0] - m_sceneViewSettings.scalarRangeMin));
            // glVertex3d(point[0].x, point[0].y, - delta - value[0]);

            if (Util::config()->scalarView3DLighting)
                glNormal3d(m_normals[linTris[i][1]][0], m_normals[linTris[i][1]][1], -m_normals[linTris[i][1]][2]);
            glTexCoord2d((value[1] - m_sceneViewSettings.scalarRangeMin) * irange * m_texScale + m_texShift, 0.0);
            glVertex3d(point[1].x, point[1].y, - delta - (value[1] - m_sceneViewSettings.scalarRangeMin));
            // glVertex3d(point[1].x, point[1].y, - delta - value[1]);

            if (Util::config()->scalarView3DLighting)
                glNormal3d(m_normals[linTris[i][2]][0], m_normals[linTris[i][2]][1], -m_normals[linTris[i][2]][2]);
            glTexCoord2d((value[2] - m_sceneViewSettings.scalarRangeMin) * irange * m_texScale + m_texShift, 0.0);
            glVertex3d(point[2].x, point[2].y, - delta - (value[2] - m_sceneViewSettings.scalarRangeMin));
            // glVertex3d(point[2].x, point[2].y, - delta - value[2]);
        }
        glEnd();
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_TEXTURE_1D);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        glPopMatrix();

        m_scene->sceneSolution()->linScalarView().unlock_data();

        glEndList();
    }
    else
    {
        glCallList(m_listScalarField3D);
    }

    paintColorBar(m_sceneViewSettings.scalarRangeMin, m_sceneViewSettings.scalarRangeMax);
}

void SceneView::paintScalarField3DSolid()
{
}

void SceneView::paintContours()
{
    if (!m_isSolutionPrepared) return;

    if (m_listContours == -1)
    {
        m_listContours = glGenLists(1);
        glNewList(m_listContours, GL_COMPILE);

        m_scene->sceneSolution()->linContourView().lock_data();

        double3* tvert = m_scene->sceneSolution()->linContourView().get_vertices();
        int3* tris = m_scene->sceneSolution()->linContourView().get_triangles();

        // transform variable
        double rangeMin =  1e100;
        double rangeMax = -1e100;

        double3* vert = new double3[m_scene->sceneSolution()->linContourView().get_num_vertices()];
        for (int i = 0; i < m_scene->sceneSolution()->linContourView().get_num_vertices(); i++)
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
        glColor3f(Util::config()->colorContours.redF(),
                  Util::config()->colorContours.greenF(),
                  Util::config()->colorContours.blueF());
        glBegin(GL_LINES);

        for (int i = 0; i < m_scene->sceneSolution()->linContourView().get_num_triangles(); i++)
        {
            if (finite(vert[tris[i][0]][2]) && finite(vert[tris[i][1]][2]) && finite(vert[tris[i][2]][2]))
            {
                paintContoursTri(vert, &tris[i], step);
            }
        }
        glEnd();

        delete vert;

        m_scene->sceneSolution()->linContourView().unlock_data();

        glEndList();
    }
    else
    {
        glCallList(m_listContours);
    }
}

void SceneView::paintContoursTri(double3* vert, int3* tri, double step)
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

void SceneView::paintVectors()
{
    if (!m_isSolutionPrepared) return;

    if (m_listVectors == -1)
    {
        m_listVectors = glGenLists(1);
        glNewList(m_listVectors, GL_COMPILE);

        double vectorRangeMin = m_scene->sceneSolution()->vecVectorView().get_min_value();
        double vectorRangeMax = m_scene->sceneSolution()->vecVectorView().get_max_value();

        double irange = 1.0 / (vectorRangeMax - vectorRangeMin);
        if (fabs(vectorRangeMin - vectorRangeMax) < EPS_ZERO) return;

        RectPoint rect = m_scene->boundingBox();
        double gs = (rect.width() + rect.height()) / Util::config()->vectorCount;

        // paint
        m_scene->sceneSolution()->vecVectorView().lock_data();

        double4* vecVert = m_scene->sceneSolution()->vecVectorView().get_vertices();
        int3* vecTris = m_scene->sceneSolution()->vecVectorView().get_triangles();

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_scene->sceneSolution()->vecVectorView().get_num_triangles(); i++)
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

                        double value = sqrt(sqr(dx) + sqr(dy));
                        double angle = atan2(dy, dx);

                        if (Util::config()->vectorProportional)
                        {
                            dx = ((value - vectorRangeMin) * irange) * Util::config()->vectorScale * gs * cos(angle);
                            dy = ((value - vectorRangeMin) * irange) * Util::config()->vectorScale * gs * sin(angle);
                        }
                        else
                        {
                            dx = Util::config()->vectorScale * gs * cos(angle);
                            dy = Util::config()->vectorScale * gs * sin(angle);
                        }

                        double dm = sqrt(sqr(dx) + sqr(dy));

                        // color
                        if (Util::config()->vectorColor)
                        {
                            double color = 0.7 - 0.7 * (value - vectorRangeMin) * irange;
                            glColor3f(color, color, color);
                        }
                        else
                        {
                            glColor3f(Util::config()->colorVectors.redF(),
                                      Util::config()->colorVectors.greenF(),
                                      Util::config()->colorVectors.blueF());
                        }

                        glVertex2d(point.x + dm/5.0 * cos(angle - M_PI_2), point.y + dm/5.0 * sin(angle - M_PI_2));
                        glVertex2d(point.x + dm/5.0 * cos(angle + M_PI_2), point.y + dm/5.0 * sin(angle + M_PI_2));
                        glVertex2d(point.x + dm     * cos(angle),          point.y + dm     * sin(angle));
                    }
                }
            }
        }
        glEnd();
        glDisable(GL_POLYGON_OFFSET_FILL);

        m_scene->sceneSolution()->vecVectorView().unlock_data();

        glEndList();
    }
    else
    {
        glCallList(m_listVectors);
    }
}

void SceneView::paintSceneModeLabel()
{
    QString text = "";

    glDisable(GL_DEPTH_TEST);

    switch (m_sceneMode)
    {
    case SceneMode_OperateOnNodes:
        text = tr("Operate on nodes");
        break;
    case SceneMode_OperateOnEdges:
        text = tr("Operate on edges");
        break;
    case SceneMode_OperateOnLabels:
        text = tr("Operate on labels");
        break;
    case SceneMode_Postprocessor:
        switch (m_sceneViewSettings.postprocessorShow)
        {
        case SceneViewPostprocessorShow_ScalarView:
        case SceneViewPostprocessorShow_ScalarView3D:
        case SceneViewPostprocessorShow_ScalarView3DSolid:
            text = physicFieldVariableString(m_sceneViewSettings.scalarPhysicFieldVariable);
            if (m_sceneViewSettings.scalarPhysicFieldVariableComp != PhysicFieldVariableComp_Scalar)
                text += " - " + physicFieldVariableCompString(m_sceneViewSettings.scalarPhysicFieldVariableComp);
            break;
        case SceneViewPostprocessorShow_Order:
            text = tr("Polynomial order");
            break;
        default:
            text = tr("Postprocessor");
        }
        break;
    }


    glPushMatrix();
    glLoadIdentity();

    glScaled(2.0/contextWidth(), 2.0/contextHeight(), 1.0);
    glTranslated(-contextWidth() / 2.0, -contextHeight() / 2.0, 0.0);

    glDisable(GL_DEPTH_TEST);

    // render viewport label
    QFont fontLabel = font();
    fontLabel.setPointSize(fontLabel.pointSize() + 1);

    Point posText = Point((contextWidth()-QFontMetrics(fontLabel).width(text)) / 2.0,
                          (contextHeight() - QFontMetrics(fontLabel).height() / 1.3));

    // blended rectangle
    double xs = posText.x - QFontMetrics(fontLabel).width(" ");
    double ys = posText.y - QFontMetrics(fontLabel).height() / 3.0;
    double xe = xs + QFontMetrics(fontLabel).width(text + "  ");
    double ye = contextHeight();

    drawBlend(Point(xs, ys), Point(xe, ye), 0.8, 0.8, 0.8, 0.93);

    // text
    glColor3f(0.0, 0.0, 0.0);
    renderText(posText.x, posText.y, 0.0, text, fontLabel);

    glPopMatrix();
}

void SceneView::paintZoomRegion()
{
    // zoom or select region
    if (m_region)
    {
        Point posStart = position(Point(m_regionPos.x(), m_regionPos.y()));
        Point posEnd = position(Point(m_lastPos.x(), m_lastPos.y()));

        drawBlend(posStart, posEnd,
                  Util::config()->colorHighlighted.redF(),
                  Util::config()->colorHighlighted.greenF(),
                  Util::config()->colorHighlighted.blueF());
    }
}

void SceneView::paintSnapToGrid()
{
    if (m_snapToGrid)
    {
        Point p = position(Point(m_lastPos.x(), m_lastPos.y()));

        Point snapPoint;
        snapPoint.x = round(p.x / Util::config()->gridStep) * Util::config()->gridStep;
        snapPoint.y = round(p.y / Util::config()->gridStep) * Util::config()->gridStep;

        glColor3f(Util::config()->colorHighlighted.redF(),
                  Util::config()->colorHighlighted.greenF(),
                  Util::config()->colorHighlighted.blueF());
        glPointSize(Util::config()->nodeSize - 1.0);
        glBegin(GL_POINTS);
        glVertex2d(snapPoint.x, snapPoint.y);
        glEnd();
    }
}

void SceneView::paintChartLine()
{
    glColor3f(Util::config()->colorSelected.redF(),
              Util::config()->colorSelected.greenF(),
              Util::config()->colorSelected.blueF());
    glLineWidth(3.0);

    glBegin(GL_LINES);
    glVertex2d(m_chartLine.start.x, m_chartLine.start.y);
    glVertex2d(m_chartLine.end.x, m_chartLine.end.y);
    glEnd();
}

const float* SceneView::paletteColor(double x)
{
    switch (Util::config()->paletteType)
    {
    case Palette_Jet:
        {
            if (x < 0.0) x = 0.0;
            else if (x > 1.0) x = 1.0;
            x *= numPalEntries;
            int n = (int) x;
            return paletteDataJet[n];
        }
        break;
    case Palette_Autumn:
        {
            if (x < 0.0) x = 0.0;
            else if (x > 1.0) x = 1.0;
            x *= numPalEntries;
            int n = (int) x;
            return paletteDataAutumn[n];
        }
        break;
    case Palette_Copper:
        {
            if (x < 0.0) x = 0.0;
            else if (x > 1.0) x = 1.0;
            x *= numPalEntries;
            int n = (int) x;
            return paletteDataCopper[n];
        }
        break;
    case Palette_Hot:
        {
            if (x < 0.0) x = 0.0;
            else if (x > 1.0) x = 1.0;
            x *= numPalEntries;
            int n = (int) x;
            return paletteDataHot[n];
        }
        break;
    case Palette_Cool:
        {
            if (x < 0.0) x = 0.0;
            else if (x > 1.0) x = 1.0;
            x *= numPalEntries;
            int n = (int) x;
            return paletteDataCool[n];
        }
        break;
    case Palette_BWAsc:
        {
            static float color[3];
            color[0] = color[1] = color[2] = x;
            return color;
        }
        break;
    case Palette_BWDesc:
        {
            static float color[3];
            color[0] = color[1] = color[2] = 1.0 - x;
            return color;
        }
        break;
    }
}

void SceneView::paletteCreate()
{
    int i;
    unsigned char palette[256][3];
    for (i = 0; i < Util::config()->paletteSteps; i++)
    {
        const float* color = paletteColor((double) i / Util::config()->paletteSteps);
        palette[i][0] = (unsigned char) (color[0] * 255);
        palette[i][1] = (unsigned char) (color[1] * 255);
        palette[i][2] = (unsigned char) (color[2] * 255);
    }
    for (i = Util::config()->paletteSteps; i < 256; i++)
        memcpy(palette[i], palette[Util::config()->paletteSteps-1], 3);

    glBindTexture(GL_TEXTURE_1D, 1);
    glTexImage1D(GL_TEXTURE_1D, 0, 3, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

void SceneView::paletteFilter()
{
    int pal_filter = Util::config()->paletteFilter ? GL_LINEAR : GL_NEAREST;
    glBindTexture(GL_TEXTURE_1D, 1);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, pal_filter);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, pal_filter);
    paletteUpdateTexAdjust();
}

void SceneView::paletteUpdateTexAdjust()
{
    if (Util::config()->paletteFilter)
    {
        m_texScale = (double) (Util::config()->paletteSteps-1) / 256.0;
        m_texShift = 0.5 / 256.0;
    }
    else
    {
        m_texScale = (double) Util::config()->paletteSteps / 256.0;
        m_texShift = 0.0;
    }
}

// events *****************************************************************************************************************************

void SceneView::keyPressEvent(QKeyEvent *event)
{
    Point stepTemp = position(Point(contextWidth(), contextHeight()));
    stepTemp.x = stepTemp.x - m_offset.x;
    stepTemp.y = stepTemp.y - m_offset.y;
    double step = qMin(stepTemp.x, stepTemp.y) / 10.0;

    switch (event->key())
    {
    case Qt::Key_Up:
        {
            m_offset.y -= step;
            doRefresh();
        }
        break;
    case Qt::Key_Down:
        {
            m_offset.y += step;
            doRefresh();
        }
        break;
    case Qt::Key_Left:
        {
            m_offset.x += step;
            doRefresh();
        }
        break;
    case Qt::Key_Right:
        {
            m_offset.x -= step;
            doRefresh();
        }
        break;
    case Qt::Key_Plus:
        {
            doZoomIn();
        }
        break;
    case Qt::Key_Minus:
        {
            doZoomOut();
        }
        break;
    case Qt::Key_Delete:
        {
            m_scene->deleteSelected();
        }
        break;
    case Qt::Key_Space:
        {
            doSceneObjectProperties();
        }
        break;
    case Qt::Key_Escape:
        {
            m_scene->selectNone();
            emit mousePressed();
            doRefresh();
        }
        break;
    default:
        QGLWidget::keyPressEvent(event);
    }

    // snap to grid
    m_snapToGrid = ((Util::config()->snapToGrid) && (event->modifiers() & Qt::ControlModifier) && (m_sceneMode == SceneMode_OperateOnNodes));

    // select all
    if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_A))
    {
        if (m_sceneMode == SceneMode_Postprocessor)
        {
            // select volume integral area
            if (actPostprocessorModeVolumeIntegral->isChecked())
            {
                m_scene->selectAll(SceneMode_OperateOnLabels);
                emit mousePressed();
            }

            // select surface integral area
            if (actPostprocessorModeSurfaceIntegral->isChecked())
            {
                m_scene->selectAll(SceneMode_OperateOnEdges);
                emit mousePressed();
            }
        }
        else
        {
            m_scene->selectAll(m_sceneMode);
        }

        doRefresh();
    }

    // add node with coordinates under mouse pointer
    if ((event->modifiers() & Qt::AltModifier & Qt::ControlModifier) | (event->key() == Qt::Key_N))
    {
        Point p = position(Point(m_lastPos.x(), m_lastPos.y()));
        m_scene->doNewNode(p);
    }
}

void SceneView::keyReleaseEvent(QKeyEvent *event)
{
    if (m_snapToGrid)
    {
        m_snapToGrid = false;
        updateGL();
    }
}

void SceneView::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
    Point p = position(Point(event->pos().x(), event->pos().y()));

    if (event->button() & Qt::LeftButton)
    {
        // zoom region
        if (actSceneZoomRegion->isChecked())
        {
            m_regionPos = m_lastPos;
            actSceneZoomRegion->setChecked(false);
            actSceneZoomRegion->setData(true);
            m_region = true;

            return;
        }

        // select region
        if (actSceneViewSelectRegion->isChecked())
        {
            m_regionPos = m_lastPos;
            actSceneViewSelectRegion->setChecked(false);
            actSceneViewSelectRegion->setData(true);
            m_region = true;

            return;
        }

        if ((m_sceneMode == SceneMode_Postprocessor) &&
            !(m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3D || m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid))
        {
            // local point value
            if (actPostprocessorModeLocalPointValue->isChecked())
                emit mousePressed(p);
            // select volume integral area
            if (actPostprocessorModeVolumeIntegral->isChecked())
            {
                int index = m_scene->sceneSolution()->findTriangleInMesh(m_scene->sceneSolution()->mesh(), p);
                if (index != -1)
                {
                    //  find label marker
                    int labelIndex = m_scene->sceneSolution()->mesh()->get_element_fast(index)->marker;

                    m_scene->labels[labelIndex]->isSelected = !m_scene->labels[labelIndex]->isSelected;
                    updateGL();
                }
                emit mousePressed();
            }
            // select surface integral area
            if (actPostprocessorModeSurfaceIntegral->isChecked())
            {
                //  find edge marker
                SceneEdge *edge = findClosestEdge(p);

                edge->isSelected = !edge->isSelected;
                updateGL();

                emit mousePressed();
            }
        }
    }

    // add node edge or label by mouse click
    if (event->modifiers() & Qt::ControlModifier)
    {
        // add node directly by mouse click
        if (m_sceneMode == SceneMode_OperateOnNodes)
        {
            Point pointNode;

            // snap to grid
            if (m_snapToGrid)
            {
                Point snapPoint = position(Point(m_lastPos.x(), m_lastPos.y()));

                pointNode.x = round(snapPoint.x / Util::config()->gridStep) * Util::config()->gridStep;
                pointNode.y = round(snapPoint.y / Util::config()->gridStep) * Util::config()->gridStep;
            }
            else
            {
                pointNode = p;
            }

            SceneNode *node = new SceneNode(pointNode);
            SceneNode *nodeAdded = m_scene->addNode(node);
            if (nodeAdded == node) m_scene->undoStack()->push(new SceneNodeCommandAdd(node->point));
            updateGL();
        }
        if (m_sceneMode == SceneMode_OperateOnEdges)
        {
            // add edge directly by mouse click
            SceneNode *node = findClosestNode(p);
            if (node)
            {
                if (m_nodeLast == NULL)
                {
                    m_nodeLast = node;
                    m_nodeLast->isSelected = true;
                }
                else
                {
                    if (node != m_nodeLast)
                    {
                        SceneEdge *edge = new SceneEdge(m_nodeLast, node, m_scene->edgeMarkers[0], 0);
                        SceneEdge *edgeAdded = m_scene->addEdge(edge);
                        if (edgeAdded == edge) m_scene->undoStack()->push(new SceneEdgeCommandAdd(edge->nodeStart->point, edge->nodeEnd->point, edge->marker->name, edge->angle));
                    }

                    m_nodeLast->isSelected = false;
                    m_nodeLast = NULL;
                }

                updateGL();
            }
        }
        // add label directly by mouse click
        if (m_sceneMode == SceneMode_OperateOnLabels)
        {
            SceneLabel *label = new SceneLabel(p, m_scene->labelMarkers[0], 0, 0);
            SceneLabel *labelAdded = m_scene->addLabel(label);
            if (labelAdded == label) m_scene->undoStack()->push(new SceneLabelCommandAdd(label->point, label->marker->name, label->area, label->polynomialOrder));
            updateGL();
        }
    }

    if ((event->modifiers() == 0) && (event->button() & Qt::LeftButton))
    {
        // select scene objects
        if (m_sceneMode == SceneMode_OperateOnNodes)
        {
            // select the closest node
            SceneNode *node = findClosestNode(p);
            if (node)
            {
                node->isSelected = !node->isSelected;
                updateGL();
            }
        }

        if (m_sceneMode == SceneMode_OperateOnEdges)
        {
            // select the closest label
            SceneEdge *edge = findClosestEdge(p);
            if (edge)
            {
                edge->isSelected = !edge->isSelected;
                updateGL();
            }
        }
        if (m_sceneMode == SceneMode_OperateOnLabels)
        {
            // select the closest label
            SceneLabel *label = findClosestLabel(p);
            if (label)
            {
                label->isSelected = !label->isSelected;
                updateGL();
            }
        }
    }
}

void SceneView::mouseDoubleClickEvent(QMouseEvent * event)
{
    Point p = position(Point(event->pos().x(), event->pos().y()));

    // zoom best fit
    if (!(event->modifiers() & Qt::ControlModifier))
    {
        if ((event->buttons() & Qt::MidButton) || ((event->buttons() & Qt::LeftButton) && (event->modifiers() & Qt::ShiftModifier)))
        {
            doZoomBestFit();
        }

        if (event->button() & Qt::LeftButton)
        {
            // select scene objects
            m_scene->selectNone();
            if (m_sceneMode == SceneMode_OperateOnNodes)
            {
                // select the closest node
                SceneNode *node = findClosestNode(p);
                if (node)
                {
                    node->isSelected = true;
                    updateGL();
                    if (node->showDialog(this) == QDialog::Accepted)
                    {
                        updateGL();
                    }
                }
            }
            if (m_sceneMode == SceneMode_OperateOnEdges)
            {
                // select the closest label
                SceneEdge *edge = findClosestEdge(p);
                if (edge)
                {
                    edge->isSelected = true;
                    updateGL();
                    if (edge->showDialog(this) == QDialog::Accepted)
                    {
                        updateGL();
                    }
                }
            }
            if (m_sceneMode == SceneMode_OperateOnLabels)
            {
                // select the closest label
                SceneLabel *label = findClosestLabel(p);
                if (label)
                {
                    label->isSelected = true;
                    updateGL();
                    if (label->showDialog(this) == QDialog::Accepted)
                    {
                        updateGL();
                    }
                }
            }
            m_scene->selectNone();
            updateGL();
        }
    }
}

void SceneView::mouseReleaseEvent(QMouseEvent *event)
{
    setCursor(Qt::ArrowCursor);

    // zoom or select region
    actSceneZoomRegion->setChecked(false);
    actSceneViewSelectRegion->setChecked(false);

    if (m_region)
    {
        Point posStart = position(Point(m_regionPos.x(), m_regionPos.y()));
        Point posEnd = position(Point(m_lastPos.x(), m_lastPos.y()));

        if (actSceneZoomRegion->data().value<bool>())
            doZoomRegion(Point(qMin(posStart.x, posEnd.x), qMin(posStart.y, posEnd.y)), Point(qMax(posStart.x, posEnd.x), qMax(posStart.y, posEnd.y)));
        if (actSceneViewSelectRegion->data().value<bool>())
            selectRegion(Point(qMin(posStart.x, posEnd.x), qMin(posStart.y, posEnd.y)), Point(qMax(posStart.x, posEnd.x), qMax(posStart.y, posEnd.y)));

        actSceneZoomRegion->setData(false);
        actSceneViewSelectRegion->setData(false);
    }    

    m_region = false;

    updateGL();
}

void SceneView::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    m_lastPos = event->pos();

    Point p = position(Point(m_lastPos.x(), m_lastPos.y()));

    setToolTip("");

    // zoom or select region
    if (m_region)
        updateGL();

    // snap to grid
    if (m_snapToGrid && !(event->modifiers() & Qt::ControlModifier))
    {
        m_snapToGrid = false;
        updateGL();
    }
    m_snapToGrid = ((Util::config()->snapToGrid) && (event->modifiers() & Qt::ControlModifier) && (m_sceneMode == SceneMode_OperateOnNodes));

    if (m_snapToGrid)
        updateGL();

    // pan
    if ((event->buttons() & Qt::MidButton) || ((event->buttons() & Qt::LeftButton) && (event->modifiers() & Qt::ShiftModifier)))
    {
        setCursor(Qt::PointingHandCursor);

        if ((m_sceneMode == SceneMode_Postprocessor) &&
            (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3D || m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid))
        {
            m_offset.x += 2.0/contextWidth() *(- dx * cos(m_rotation.y/180.0*M_PI) + dy * sin(m_rotation.y/180.0*M_PI))/m_scale*m_aspect;
            m_offset.y += 2.0/contextHeight()*(  dy * cos(m_rotation.y/180.0*M_PI) + dx * sin(m_rotation.y/180.0*M_PI))/m_scale;
            m_offset.z -= 2.0/contextHeight()*(  dy * sin(m_rotation.x/180.0*M_PI))/m_scale;
        }
        else
        {
            m_offset.x -= 2.0/contextWidth() * dx / m_scale*m_aspect;
            m_offset.y += 2.0/contextHeight() * dy / m_scale;
            m_offset.z = 0.0;
        }

        updateGL();
    }

    // rotate
    if ((m_sceneMode == SceneMode_Postprocessor) &&
        (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3D || m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid))
    {
        if (event->buttons() & Qt::LeftButton)
        {
            setCursor(Qt::PointingHandCursor);

            m_rotation.x -= dy;
            m_rotation.y += dx;

            updateGL();
        }
    }

    // hints
    if (event->modifiers() == 0)
    {
        // highlight scene objects
        if (m_sceneMode == SceneMode_OperateOnNodes)
        {
            // highlight the closest node
            SceneNode *node = findClosestNode(p);
            if (node)
            {
                m_scene->highlightNone();
                node->isHighlighted = true;
                setToolTip(tr("<h3>Node</h3>Point: [%1; %2]<br/>Index: %3").
                           arg(node->point.x, 0, 'g', 3).
                           arg(node->point.y, 0, 'g', 3).
                           arg(m_scene->nodes.indexOf(node)));
                updateGL();
            }
        }
        if (m_sceneMode == SceneMode_OperateOnEdges)
        {
            // highlight the closest label
            SceneEdge *edge = findClosestEdge(p);
            if (edge)
            {
                m_scene->highlightNone();
                edge->isHighlighted = true;
                setToolTip(tr("<h3>Edge</h3>Point: [%1; %2] - [%3; %4]<br/>Boundary Condition: %5<br/>Angle: %6 deg.<br/>Index: %7 %8").
                           arg(edge->nodeStart->point.x, 0, 'g', 3).
                           arg(edge->nodeStart->point.y, 0, 'g', 3).
                           arg(edge->nodeEnd->point.x, 0, 'g', 3).
                           arg(edge->nodeEnd->point.y, 0, 'g', 3).
                           arg(edge->marker->name).
                           arg(edge->angle, 0, 'f', 0).
                           arg(m_scene->edges.indexOf(edge)).
                           arg(edge->marker->html()));
                updateGL();
            }
        }
        if (m_sceneMode == SceneMode_OperateOnLabels)
        {
            // highlight the closest label
            SceneLabel *label = findClosestLabel(p);
            if (label)
            {
                m_scene->highlightNone();
                label->isHighlighted = true;
                setToolTip(tr("<h3>Label</h3>Point: [%1; %2]<br/>Material: %3<br/>Triangle area: %4 m<sup>2</sup><br/>Polynomial order: %5<br/>Index: %6 %7").
                           arg(label->point.x, 0, 'g', 3).
                           arg(label->point.y, 0, 'g', 3).
                           arg(label->marker->name).
                           arg(label->area, 0, 'g', 3).
                           arg(label->polynomialOrder).
                           arg(m_scene->labels.indexOf(label)).
                           arg(label->marker->html()));
                updateGL();
            }
        }        
    }

    if (event->modifiers() & Qt::ControlModifier)
    {
        // add edge directly by mouse click - highlight
        if (m_sceneMode == SceneMode_OperateOnEdges)
        {
            // add edge directly by mouse click
            SceneNode *node = findClosestNode(p);
            if (node)
            {
                m_scene->highlightNone();
                node->isHighlighted = true;
                updateGL();
            }
        }
    }


    if (m_snapToGrid)
    {
        Point snapPoint;
        snapPoint.x = round(p.x / Util::config()->gridStep) * Util::config()->gridStep;
        snapPoint.y = round(p.y / Util::config()->gridStep) * Util::config()->gridStep;

        emit mouseMoved(QPointF(snapPoint.x, snapPoint.y));
    }
    else
    {
        emit mouseMoved(QPointF(p.x, p.y));
    }
}

void SceneView::wheelEvent(QWheelEvent *event)
{
    if (Util::config()->zoomToMouse)
    {
        Point posMouse;
        posMouse = Point((2.0/contextWidth()*(event->pos().x() - contextWidth()/2.0))/m_scale*m_aspect,
                        -(2.0/contextHeight()*(event->pos().y() - contextHeight()/2.0))/m_scale);

        m_offset.x += posMouse.x;
        m_offset.y += posMouse.y;

        m_scale = m_scale * pow(1.2, event->delta()/150.0);


        posMouse = Point((2.0/contextWidth()*(event->pos().x() - contextWidth()/2.0))/m_scale*m_aspect,
                        -(2.0/contextHeight()*(event->pos().y() - contextHeight()/2.0))/m_scale);

        m_offset.x -= posMouse.x;
        m_offset.y -= posMouse.y;

        updateGL();
    }
    else
    {
        setZoom(event->delta()/150.0);
    }
}

void SceneView::contextMenuEvent(QContextMenuEvent *event)
{
    actSceneObjectProperties->setEnabled(false);

    // set boundary context menu
    if (m_sceneMode == SceneMode_OperateOnEdges)
        actSceneObjectProperties->setEnabled(m_scene->selectedCount() > 0);

    // set material context menu
    if (m_sceneMode == SceneMode_OperateOnLabels)
        actSceneObjectProperties->setEnabled(m_scene->selectedCount() > 0);


    mnuInfo->exec(event->globalPos());
}

void SceneView::closeEvent(QCloseEvent *event)
{
    event->ignore();
}

// slots *****************************************************************************************************************************

void SceneView::doZoomBestFit()
{
    RectPoint rect = m_scene->boundingBox();
    doZoomRegion(rect.start, rect.end);
}

void SceneView::doZoomIn()
{
    setZoom(1.2);
}

void SceneView::doZoomOut()
{
    setZoom(-1/1.2);
}

void SceneView::doZoomRegion(const Point &start, const Point &end)
{
    if (fabs(end.x-start.x) < EPS_ZERO || fabs(end.y-start.y) < EPS_ZERO) return;

    m_offset.x = (start.x+end.x)/2.0;
    m_offset.y = (start.y+end.y)/2.0;

    double sceneWidth = end.x-start.x;
    double sceneHeight = end.y-start.y;

    double maxScene = (((double) contextWidth() / (double) contextHeight()) < (sceneWidth / sceneHeight)) ? sceneWidth/m_aspect : sceneHeight;

    if (maxScene > 0.0)
        m_scale = 1.95/maxScene;

    setZoom(0);
}

void SceneView::doSetChartLine(const Point &start, const Point &end)
{
    // set line for chart
    m_chartLine.start = start;
    m_chartLine.end = end;

    updateGL();
}

void SceneView::doDefaultValues()
{
    m_snapToGrid = false;
    m_region = false;
    m_isSolutionPrepared = false;

    m_scale = 1.0;
    m_offset.x = 0.0;
    m_offset.y = 0.0;

    m_chartLine.start = Point();
    m_chartLine.end = Point();

    m_sceneViewSettings.defaultValues();

    doInvalidated();
    doZoomBestFit();

    actPostprocessorModeLocalPointValue->trigger();
}

void SceneView::doSolved()
{
    // m_isSolutionPrepared = false;
    actSceneModePostprocessor->trigger();
    m_sceneViewSettings.showInitialMesh = false;

    // run in separate thread
    // QTimer::singleShot(0, this, SLOT(doProcessSolution()));
    doProcessSolution();
}

void SceneView::doInvalidated()
{
    resizeGL(width(), height());

    if (!m_scene->sceneSolution()->isSolved())
    {
        if (m_sceneMode == SceneMode_Postprocessor)
            actSceneModeNode->trigger();
    }

    actSceneModePostprocessor->setEnabled(m_scene->sceneSolution()->isSolved());
    actSceneViewSelectMarker->setEnabled(m_scene->sceneSolution()->isSolved());
    actSceneZoomRegion->setEnabled((m_sceneMode != SceneMode_Postprocessor) ||
                                   (m_sceneViewSettings.postprocessorShow != SceneViewPostprocessorShow_ScalarView3D));

    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        // range is changed in every time step
        setRangeContour();
        setRangeScalar();
        setRangeVector();

        clearGLLists();
    }

    emit mousePressed();

    updateGL();
}

void SceneView::doProcessSolution()
{
    if (m_normals)
    {
        delete m_normals;
        m_normals = NULL;
    }

    setRangeContour();
    setRangeScalar();
    setRangeVector();

    clearGLLists();

    m_isSolutionPrepared = true;

    doInvalidated();
}

void SceneView::doRefresh()
{
    paintGL();
    updateGL();
}

void SceneView::doMaterialGroup(QAction *action)
{
    if (SceneLabelMarker *labelMarker = action->data().value<SceneLabelMarker *>())
        m_scene->setLabelLabelMarker(labelMarker);
}

void SceneView::doBoundaryGroup(QAction *action)
{
    if (SceneEdgeMarker *edgeMarker = action->data().value<SceneEdgeMarker *>())
        m_scene->setEdgeEdgeMarker(edgeMarker);
}

void SceneView::doShowGroup(QAction *action)
{
    m_sceneViewSettings.showContours = actShowContours->isChecked();
    m_sceneViewSettings.showVectors = actShowVectors->isChecked();
    m_sceneViewSettings.showSolutionMesh = actShowSolutionMesh->isChecked();

    doInvalidated();
}

void SceneView::doPostprocessorModeGroup(QAction *action)
{
    m_scene->selectNone();
    updateGL();
}

void SceneView::doSceneViewProperties()
{
    SceneViewPostprocessorShow postprocessorShow = m_sceneViewSettings.postprocessorShow;

    SceneViewDialog sceneViewDialog(this, this);
    if (sceneViewDialog.showDialog() == QDialog::Accepted)
    {
        doProcessSolution();

        // set defaults
        if (postprocessorShow != m_sceneViewSettings.postprocessorShow)
        {
            if (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3D ||
                m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid)
            {
                m_rotation.x =  66.0;
                m_rotation.y = -35.0;
                doZoomBestFit();
            }
            else
            {
                m_rotation.x = 0.0;
                m_rotation.y = 0.0;
                doZoomBestFit();
            }
        }
    }
}

void SceneView::doSceneObjectProperties()
{
    if (m_sceneMode == SceneMode_OperateOnEdges)
    {
        if (m_scene->selectedCount() > 1)
        {
            EdgeMarkerDialog edgeMarkerDialog(this);
            edgeMarkerDialog.exec();
        }
        if (m_scene->selectedCount() == 1)
        {
            for (int i = 0; i < m_scene->edges.count(); i++)
            {
                if (m_scene->edges[i]->isSelected)
                    m_scene->edges[i]->showDialog(this);
            }
        }
    }
    if (m_sceneMode == SceneMode_OperateOnLabels)
    {
        if (m_scene->selectedCount() > 1)
        {
            LabelMarkerDialog labelMarkerDialog(this);
            labelMarkerDialog.exec();
        }
        if (m_scene->selectedCount() == 1)
        {
            for (int i = 0; i < m_scene->labels.count(); i++)
            {
                if (m_scene->labels[i]->isSelected)
                    m_scene->labels[i]->showDialog(this);
            }
        }
    }

    m_scene->selectNone();
}

void SceneView::doSceneModeSet(QAction *action)
{
    actSceneModePostprocessor->setEnabled(m_scene->sceneSolution()->isSolved());

    if (actSceneModeNode->isChecked()) m_sceneMode = SceneMode_OperateOnNodes;
    if (actSceneModeEdge->isChecked()) m_sceneMode = SceneMode_OperateOnEdges;
    if (actSceneModeLabel->isChecked()) m_sceneMode = SceneMode_OperateOnLabels;
    if (actSceneModePostprocessor->isChecked()) m_sceneMode = SceneMode_Postprocessor;

    m_scene->highlightNone();
    m_scene->selectNone();

    m_scene->actTransform->setEnabled((m_sceneMode != SceneMode_Postprocessor));
    m_scene->actDeleteSelected->setEnabled((m_sceneMode != SceneMode_Postprocessor));
    actSceneViewSelectRegion->setEnabled((m_sceneMode != SceneMode_Postprocessor));

    actPostprocessorModeLocalPointValue->setEnabled((m_sceneMode == SceneMode_Postprocessor));
    actPostprocessorModeSurfaceIntegral->setEnabled((m_sceneMode == SceneMode_Postprocessor));
    actPostprocessorModeVolumeIntegral->setEnabled((m_sceneMode == SceneMode_Postprocessor));

    switch (m_sceneMode)
    {
    case SceneMode_OperateOnNodes:
        break;
    case SceneMode_OperateOnEdges:
        m_nodeLast = NULL;
        break;
    case SceneMode_OperateOnLabels:
        break;
    case SceneMode_Postprocessor:
        break;
    }

    doInvalidated();

    emit sceneModeChanged(m_sceneMode);
}

void SceneView::doSelectMarker()
{
    SceneMarkerSelectDialog sceneMarkerSelectDialog(this, QApplication::activeWindow());
    sceneMarkerSelectDialog.exec();
}

void SceneView::setRangeContour()
{
    if (m_sceneMode == SceneMode_Postprocessor && m_sceneViewSettings.showContours)
    {
        ViewScalarFilter *viewScalarFilter = m_scene->problemInfo()->hermes()->viewScalarFilter(m_sceneViewSettings.contourPhysicFieldVariable,
                                                                                                PhysicFieldVariableComp_Scalar);

        m_scene->sceneSolution()->setSlnContourView(viewScalarFilter);
    }
}

void SceneView::setRangeScalar()
{
    if ((m_sceneMode == SceneMode_Postprocessor) &&
        (m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView ||
         m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3D ||
         m_sceneViewSettings.postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid))
    {
        ViewScalarFilter *viewScalarFilter = m_scene->problemInfo()->hermes()->viewScalarFilter(m_sceneViewSettings.scalarPhysicFieldVariable,
                                                                                                m_sceneViewSettings.scalarPhysicFieldVariableComp);

        m_scene->sceneSolution()->setSlnScalarView(viewScalarFilter);

        if (m_sceneViewSettings.scalarRangeAuto)
        {
            m_sceneViewSettings.scalarRangeMin = m_scene->sceneSolution()->linScalarView().get_min_value();
            m_sceneViewSettings.scalarRangeMax = m_scene->sceneSolution()->linScalarView().get_max_value();
        }

        if (Util::config()->scalarView3DLighting)
        {
            double max = qMax(m_scene->boundingBox().width(), m_scene->boundingBox().height());

            // lighting
            float light_specular[] = {1.0, 1.0, 1.0, 1.0};
            float light_ambient[]  = {0.3, 0.3, 0.3, 1.0};
            float light_diffuse[]  = {1.0, 1.0, 1.0, 1.0};
            float light_position[] = { -10.0*max, 16.0*max, 10.0*max, 0.0 };

            glEnable(GL_LIGHT0);
            glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
            glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
            glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
            glLightfv(GL_LIGHT0, GL_POSITION, light_position);

            float material_ambient[]  = {1.0, 1.0, 1.0, 1.0};
            float material_diffuse[]  = {0.4, 0.4, 0.4, 1.0};
            float material_specular[] = {0.3, 0.3, 0.3, 1.0};

            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32.0);
            glDisable(GL_COLOR_MATERIAL);

            glShadeModel(GL_SMOOTH);
            glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
            #if defined(GL_LIGHT_MODEL_COLOR_CONTROL) && defined(GL_SEPARATE_SPECULAR_COLOR)
                glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
            #endif

            // calculate normals
            m_scene->sceneSolution()->linScalarView().lock_data();

            int nv = m_scene->sceneSolution()->linScalarView().get_num_vertices();
            int nt = m_scene->sceneSolution()->linScalarView().get_num_triangles();
            double3* vert = m_scene->sceneSolution()->linScalarView().get_vertices();
            int3* tris = m_scene->sceneSolution()->linScalarView().get_triangles();

            m_normals = new double3[nv];
            memset(m_normals, 0, nv * sizeof(double3));

            for (int i = 0; i < nt; i++)
            {
                double ax = (vert[tris[i][1]][0] - vert[tris[i][0]][0]);
                double ay = (vert[tris[i][1]][1] - vert[tris[i][0]][1]);
                double az = (vert[tris[i][1]][2] - vert[tris[i][0]][2]);

                double bx = (vert[tris[i][2]][0] - vert[tris[i][0]][0]);
                double by = (vert[tris[i][2]][1] - vert[tris[i][0]][1]);
                double bz = (vert[tris[i][2]][2] - vert[tris[i][0]][2]);

                double nx = ay * bz - az * by;
                double ny = az * bx - ax * bz;
                double nz = ax * by - ay * bx;

                // normalize
                double l = 1.0 / sqrt(sqr(nx) + sqr(ny) + sqr(nz));
                nx *= l; ny *= l; nz *= l;

                for (int j = 0; j < 3; j++)
                {
                    m_normals[tris[i][j]][0] += nx;
                    m_normals[tris[i][j]][1] += ny;
                    m_normals[tris[i][j]][2] += nz;
                }
            }

            for (int i = 0; i < nv; i++)
            {
                // normalize
                double l = 1.0 / sqrt(sqr(m_normals[i][0]) + sqr(m_normals[i][1]) + sqr(m_normals[i][2]));
                m_normals[i][0] *= l; m_normals[i][1] *= l; m_normals[i][2] *= l;
            }

            m_scene->sceneSolution()->linScalarView().unlock_data();
        }
    }
}

void SceneView::setRangeVector()
{
    if (m_sceneMode == SceneMode_Postprocessor && m_sceneViewSettings.showVectors)
    {
        ViewScalarFilter *viewVectorXFilter = m_scene->problemInfo()->hermes()->viewScalarFilter(m_sceneViewSettings.vectorPhysicFieldVariable,
                                                                                                       PhysicFieldVariableComp_X);

        ViewScalarFilter *viewVectorYFilter = m_scene->problemInfo()->hermes()->viewScalarFilter(m_sceneViewSettings.vectorPhysicFieldVariable,
                                                                                                       PhysicFieldVariableComp_Y);

        m_scene->sceneSolution()->setSlnVectorView(viewVectorXFilter, viewVectorYFilter);
    }
}

void SceneView::setZoom(double power)
{
    m_scale = m_scale * pow(1.2, power);

    updateGL();

    Point p(pos().x(), pos().y());
    emit mouseMoved(QPointF(position(p).x, position(p).y));
}

void SceneView::selectRegion(const Point &start, const Point &end)
{
    m_scene->selectNone();

    switch (m_sceneMode)
    {
    case SceneMode_OperateOnNodes:
        foreach (SceneNode *node, m_scene->nodes)
            if (node->point.x >= start.x && node->point.x <= end.x && node->point.y >= start.y && node->point.y <= end.y)
                node->isSelected = true;
        break;
    case SceneMode_OperateOnEdges:
        foreach (SceneEdge *edge, m_scene->edges)
            if (edge->nodeStart->point.x >= start.x && edge->nodeStart->point.x <= end.x && edge->nodeStart->point.y >= start.y && edge->nodeStart->point.y <= end.y &&
                edge->nodeEnd->point.x >= start.x && edge->nodeEnd->point.x <= end.x && edge->nodeEnd->point.y >= start.y && edge->nodeEnd->point.y <= end.y)
                edge->isSelected = true;
        break;
    case SceneMode_OperateOnLabels:
        foreach (SceneLabel *label, m_scene->labels)
            if (label->point.x >= start.x && label->point.x <= end.x && label->point.y >= start.y && label->point.y <= end.y)
                label->isSelected = true;
        break;
    }
}

SceneNode *SceneView::findClosestNode(const Point &point)
{
    SceneNode *nodeClosest = NULL;

    double distance = 1e100;
    foreach (SceneNode *node, m_scene->nodes)
    {
        double nodeDistance = node->distance(point);
        if (node->distance(point) < distance)
        {
            distance = nodeDistance;
            nodeClosest = node;
        }
    }

    return nodeClosest;
}

SceneEdge *SceneView::findClosestEdge(const Point &point)
{
    SceneEdge *edgeClosest = NULL;

    double distance = 1e100;
    foreach (SceneEdge *edge, m_scene->edges)
    {
        double edgeDistance = edge->distance(point);
        if (edge->distance(point) < distance)
        {
            distance = edgeDistance;
            edgeClosest = edge;
        }
    }

    return edgeClosest;
}

SceneLabel *SceneView::findClosestLabel(const Point &point)
{
    SceneLabel *labelClosest = NULL;

    double distance = 1e100;
    foreach (SceneLabel *label, m_scene->labels)
    {
        double labelDistance = label->distance(point);
        if (label->distance(point) < distance)
        {
            distance = labelDistance;
            labelClosest = label;
        }
    }

    return labelClosest;
}

void SceneView::drawArc(const Point &point, double r, double startAngle, double arcAngle, int segments)
{
    double theta = arcAngle / double(segments - 1);

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < segments; i++)
    {
        double arc = (startAngle + i*theta)/180.0*M_PI;

        double x = r * cos(arc);
        double y = r * sin(arc);

        glVertex2d(point.x + x, point.y + y);
    }
    glEnd();
}

void SceneView::drawBlend(Point start, Point end, double red, double green, double blue, double alpha)
{
    // store color
    double color[4];
    glGetDoublev(GL_CURRENT_COLOR, color);

    // blended rectangle
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(red, green, blue, alpha);
    glBegin(GL_QUADS);
    glVertex2d(start.x, start.y);
    glVertex2d(end.x, start.y);
    glVertex2d(end.x, end.y);
    glVertex2d(start.x, end.y);
    glEnd();
    glDisable(GL_BLEND);

    // retrieve color
    glColor4d(color[0], color[1], color[2], color[3]);
}

void SceneView::renderTextPos(double x, double y, double z, const QString &str, bool blend)
{
    if (blend)
    {
        Point size((2.0/contextWidth()*fontMetrics().width(" "))/m_scale*m_aspect,
                   (2.0/contextHeight()*fontMetrics().height())/m_scale);

        double xs = x - size.x / 2.0;
        double ys = y - size.y * 1.15 / 3.2;
        double xe = xs + size.x * (str.size() + 1);
        double ye = ys + size.y * 1.15;

        drawBlend(Point(xs, ys), Point(xe, ye));
    }

    renderText(x, y, z, str);
}

void SceneView::paintPostprocessorSelectedVolume()
{
    if (!m_scene->sceneSolution()->isMeshed()) return;

    // draw mesh
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4d(Util::config()->colorSelected.redF(),
              Util::config()->colorSelected.greenF(),
              Util::config()->colorSelected.blueF(),
              0.5);

    // triangles
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < m_scene->sceneSolution()->mesh()->get_num_elements(); i++)
    {
        Element *element = m_scene->sceneSolution()->mesh()->get_element(i);
        if (m_scene->labels[element->marker]->isSelected)
        {
            if (element->is_triangle())
            {
                glVertex2d(element->vn[0]->x, element->vn[0]->y);
                glVertex2d(element->vn[1]->x, element->vn[1]->y);
                glVertex2d(element->vn[2]->x, element->vn[2]->y);
            }
        }
    }
    glEnd();
    glDisable(GL_BLEND);
}

void SceneView::paintPostprocessorSelectedSurface()
{
    // edges
    foreach (SceneEdge *edge, m_scene->edges) {
        glColor3d(Util::config()->colorSelected.redF(), Util::config()->colorSelected.greenF(), Util::config()->colorSelected.blueF());
        glLineWidth(3.0);

        if (edge->isSelected)
        {
            if (edge->angle == 0)
            {
                glBegin(GL_LINES);
                glVertex2d(edge->nodeStart->point.x, edge->nodeStart->point.y);
                glVertex2d(edge->nodeEnd->point.x, edge->nodeEnd->point.y);
                glEnd();
            }
            else
            {
                Point center = edge->center();
                double radius = edge->radius();
                double startAngle = atan2(center.y - edge->nodeStart->point.y, center.x - edge->nodeStart->point.x) / M_PI*180 - 180;

                drawArc(center, radius, startAngle, edge->angle, edge->angle/5);
            }
        }
        glLineWidth(1.0);
    }
}

ErrorResult SceneView::saveImageToFile(const QString &fileName, int w, int h)
{
    QPixmap pixmap = renderPixmap(w, h);
    if (pixmap.save(fileName, "PNG"))
        resizeGL(width(), height());
    else
        return ErrorResult(ErrorResultType_Critical, tr("Image cannot be saved to the file '%1'.").arg(fileName));

    return ErrorResult();
}

void SceneView::saveImagesForReport(const QString &path, int w, int h)
{
    // store sceneview settings
    SceneViewSettings sceneViewSettingsCopy = m_sceneViewSettings;
    SceneMode sceneModeCopy = m_sceneMode;
    double scaleCopy = m_scale;
    double aspectCopy = m_aspect;
    Point3 offsetCopy = m_offset;
    Point rotationCopy = m_rotation;

    // remove old files
    QFile::remove(path + "/geometry.png");
    QFile::remove(path + "/mesh.png");
    QFile::remove(path + "/scalarview.png");
    QFile::remove(path + "/order.png");

    doZoomBestFit();

    m_sceneViewSettings.showContours = false;
    m_sceneViewSettings.showGeometry = true;
    m_sceneViewSettings.showGrid = true;
    m_sceneViewSettings.showInitialMesh = false;
    // m_sceneViewSettings.showRulers = true;
    m_sceneViewSettings.showSolutionMesh = false;
    m_sceneViewSettings.showVectors = false;

    // geometry
    actSceneModeLabel->trigger();
    ErrorResult resultGeometry = saveImageToFile(path + "/geometry.png", w, h);
    if (resultGeometry.isError())
        resultGeometry.showDialog();
    // m_sceneViewSettings.showRulers = false;

    // mesh
    if (m_scene->sceneSolution()->isMeshed())
    {
        // show only initial mesh
        actSceneModeLabel->trigger();

        m_sceneViewSettings.showInitialMesh = true;
        ErrorResult resultMesh1 = saveImageToFile(path + "/mesh.png", w, h);
        if (resultMesh1.isError())
            resultMesh1.showDialog();
        m_sceneViewSettings.showInitialMesh = false;
    }
    if (m_scene->sceneSolution()->isSolved())
    {
        // when solved show both meshes
        actSceneModePostprocessor->trigger();

        m_sceneViewSettings.postprocessorShow = SceneViewPostprocessorShow_None;
        updateGL();

        m_sceneViewSettings.showInitialMesh = true;
        m_sceneViewSettings.showSolutionMesh = true;
        ErrorResult resultMesh2 = saveImageToFile(path + "/mesh.png", w, h);
        if (resultMesh2.isError())
            resultMesh2.showDialog();
        m_sceneViewSettings.showInitialMesh = false;
        m_sceneViewSettings.showSolutionMesh = false;
    }

    if (m_scene->sceneSolution()->isSolved())
    {
        actSceneModePostprocessor->trigger();

        // last step
        if (m_scene->problemInfo()->hermes()->hasTransient())
            m_scene->sceneSolution()->setTimeStep(m_scene->sceneSolution()->timeStepCount() - 1);

        m_sceneViewSettings.scalarRangeAuto = true;
        m_sceneViewSettings.scalarPhysicFieldVariable = m_scene->problemInfo()->hermes()->scalarPhysicFieldVariable();
        m_sceneViewSettings.scalarPhysicFieldVariableComp = m_scene->problemInfo()->hermes()->scalarPhysicFieldVariableComp();
        m_sceneViewSettings.vectorPhysicFieldVariable = m_scene->problemInfo()->hermes()->vectorPhysicFieldVariable();

        doInvalidated();

        // scalar field
        m_sceneViewSettings.postprocessorShow = SceneViewPostprocessorShow_ScalarView;
        updateGL();
        ErrorResult resultScalarView = saveImageToFile(path + "/scalarview.png", w, h);
        if (resultScalarView.isError())
            resultScalarView.showDialog();

        // order
        m_sceneViewSettings.postprocessorShow = SceneViewPostprocessorShow_Order;
        updateGL();
        ErrorResult resultOrder = saveImageToFile(path + "/order.png", w, h);
        if (resultOrder.isError())
            resultOrder.showDialog();
    }

    // restore sceneview settings
    m_sceneViewSettings = sceneViewSettingsCopy;
    m_sceneMode = sceneModeCopy;
    m_scale = scaleCopy;
    m_aspect = aspectCopy;
    m_offset = offsetCopy;
    m_rotation = rotationCopy;

    if (m_sceneMode == SceneMode_OperateOnNodes) actSceneModeNode->trigger();
    if (m_sceneMode == SceneMode_OperateOnLabels) actSceneModeEdge->isChecked();
    if (m_sceneMode == SceneMode_OperateOnLabels) actSceneModeLabel->isChecked();
    if (m_sceneMode == SceneMode_Postprocessor) actSceneModePostprocessor->isChecked();

    updateGL();
}
