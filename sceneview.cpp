#include "sceneview.h"

SceneView::SceneView(Scene *scene, QWidget *parent): QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    this->m_scene = scene;

    createActions();
    createMenu();

    doDefaults();

    setMinimumSize(200, 200);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::DefaultContextMenu);
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

    actSceneZoomBestFit = new QAction(icon("zoom-best-fit"), tr("Zoom best fit"), this);
    actSceneZoomBestFit->setStatusTip(tr("Best fit"));
    connect(actSceneZoomBestFit, SIGNAL(triggered()), this, SLOT(doZoomBestFit()));

    actSceneZoomRegion = new QAction(icon("zoom-region"), tr("Zoom region"), this);
    actSceneZoomRegion->setStatusTip(tr("Zoom region"));
    actSceneZoomRegion->setCheckable(true);

    // scene - operate on items
    actSceneModeNode = new QAction(icon("scene-node"), tr("Operate on &nodes"), this);
    actSceneModeNode->setStatusTip(tr("Operate on nodes"));
    actSceneModeNode->setCheckable(true);

    actSceneModeEdge = new QAction(icon("scene-edge"), tr("Operate on &edges"), this);
    actSceneModeEdge->setStatusTip(tr("Operate on edges"));
    actSceneModeEdge->setCheckable(true);

    actSceneModeLabel = new QAction(icon("scene-label"), tr("Operate on &labels"), this);
    actSceneModeLabel->setStatusTip(tr("Operate on labels"));
    actSceneModeLabel->setCheckable(true);

    actSceneModePostprocessor = new QAction(icon("scene-postprocessor"), tr("&Postprocessor"), this);
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

    actShowScalarField = new QAction(tr("Scalar Field"), this);
    actShowScalarField->setCheckable(true);

    actShowVectors = new QAction(tr("Vectors"), this);
    actShowVectors->setCheckable(true);

    actShowGroup = new QActionGroup(this);
    actShowGroup->setExclusive(false);
    connect(actShowGroup, SIGNAL(triggered(QAction *)), this, SLOT(doShowGroup(QAction *)));
    actShowGroup->addAction(actShowSolutionMesh);
    actShowGroup->addAction(actShowContours);
    actShowGroup->addAction(actShowScalarField);
    actShowGroup->addAction(actShowVectors);

    // postprocessor group
    actPostprocessorModeLocalPointValue = new QAction(icon("mode-localpointvalue"), "Local Values", this);
    actPostprocessorModeLocalPointValue->setCheckable(true);

    actPostprocessorModeSurfaceIntegral = new QAction(icon("mode-surfaceintegral"), "Surface Integrals", this);
    actPostprocessorModeSurfaceIntegral->setCheckable(true);

    actPostprocessorModeVolumeIntegral = new QAction(icon("mode-volumeintegral"), "Volume Integrals", this);
    actPostprocessorModeVolumeIntegral->setCheckable(true);

    actPostprocessorModeGroup = new QActionGroup(this);
    connect(actPostprocessorModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(doPostprocessorModeGroup(QAction*)));
    actPostprocessorModeGroup->addAction(actPostprocessorModeLocalPointValue);
    actPostprocessorModeGroup->addAction(actPostprocessorModeSurfaceIntegral);
    actPostprocessorModeGroup->addAction(actPostprocessorModeVolumeIntegral);

    // properties
    actSceneViewProperties = new QAction(icon("scene-properties"), tr("&Properties"), this);
    actSceneViewProperties->setStatusTip(tr("Properties"));
    connect(actSceneViewProperties, SIGNAL(triggered()), this, SLOT(doSceneViewProperties()));

    // select region
    actSceneViewSelectRegion = new QAction(icon("scene-select-region"), tr("Select region"), this);
    actSceneViewSelectRegion->setStatusTip(tr("Select region"));
    actSceneViewSelectRegion->setCheckable(true);
}

void SceneView::createMenu()
{
    mnuInfo = new QMenu(this);
    mnuMarkerGroup = new QMenu(tr("Set marker"), this);

    // show group
    mnuShowGroup = new QMenu(tr("Show"), this);
    mnuShowGroup->addAction(actShowSolutionMesh);
    mnuShowGroup->addAction(actShowContours);
    mnuShowGroup->addAction(actShowScalarField);
    mnuShowGroup->addAction(actShowVectors);

    mnuInfo->addAction(m_scene->actNewNode);
    mnuInfo->addAction(m_scene->actNewEdge);
    mnuInfo->addAction(m_scene->actNewLabel);
    mnuInfo->addSeparator();
    mnuInfo->addAction(m_scene->actTransform);
    mnuInfo->addSeparator();
    mnuInfo->addMenu(mnuMarkerGroup);
    mnuInfo->addMenu(mnuShowGroup);
    mnuInfo->addSeparator();
    mnuInfo->addAction(actSceneViewProperties);
}

void SceneView::initializeGL()
{
    glClearColor(0.99, 0.99, 0.99, 0);
    glShadeModel(GL_FLAT);
    // glDisable(GL_LIGHTING);
    // glDisable(GL_DEPTH_TEST);

    setupViewport(width(), height());
}

void SceneView::paintGL()
{
    setupViewport(width(), height());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glShadeModel(GL_SMOOTH);

    glLoadIdentity();
    glScaled(m_scale/m_drawScaleY, m_scale/m_drawScaleX, 0);
    glTranslated(-m_offset.x, -m_offset.y, 0);


    if (m_sceneViewSettings.showGrid) paintGrid();
    if (m_scene->sceneSolution()->isSolved())
    {
        if (m_sceneMode == SCENEMODE_POSTPROCESSOR)
        {
            if (m_sceneViewSettings.showOrder) paintOrder();
            if (m_sceneViewSettings.showScalarField) paintScalarField();
            if (m_sceneViewSettings.showVectors) paintVectors();
            if (m_sceneViewSettings.showContours) paintContours();
        }
    }
    if (m_sceneViewSettings.showSolutionMesh) paintSolutionMesh();
    if (m_sceneViewSettings.showInitialMesh) paintInitialMesh();
    if (m_sceneViewSettings.showGeometry) paintGeometry();

    if (m_sceneMode == SCENEMODE_POSTPROCESSOR)
    {
        if (actPostprocessorModeVolumeIntegral->isChecked()) paintPostprocessorSelectedVolume();
        if (actPostprocessorModeSurfaceIntegral->isChecked()) paintPostprocessorSelectedSurface();
    }

    // zoom or select region
    if (!m_regionPos.isNull())
    {
        Point posStart = position(Point(m_regionPos.x(), m_regionPos.y()));
        Point posEnd = position(Point(m_lastPos.x(), m_lastPos.y()));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor4d(1.0, 0.8, 0.7, 0.65);
        glBegin(GL_QUADS);
        glVertex2d(posStart.x, posStart.y);
        glVertex2d(posEnd.x, posStart.y);
        glVertex2d(posEnd.x, posEnd.y);
        glVertex2d(posStart.x, posEnd.y);
        glEnd();
        glDisable(GL_BLEND);
    }

    paintSceneModeLabel();
}

void SceneView::resizeGL(int width, int height)
{
    setupViewport(width, height);
}

void SceneView::setupViewport(int width, int height)
{
    int side = qMax(width, height);

    glViewport(5, 5, width-10, height-10);

    m_drawScaleX = 1.0;
    m_drawScaleY = 1.0;

    if (width < height)
    {
        m_drawScaleX = ((double) side)/((double) width);
    }
    else
    {
        m_drawScaleY = ((double) side)/((double) height);
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, 0, 0);
}

// paint *****************************************************************************************************************************

void SceneView::paintGrid()
{
    Point cornerMin = position(Point(0, 0));
    Point cornerMax = position(Point(width(), height()));

    glDisable(GL_TEXTURE_1D);
    glColor3f(0.85, 0.85, 0.85);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x1C47);
    glBegin(GL_LINES);
    // vertical lines
    if ((((cornerMax.x-cornerMin.x)/sceneViewSettings().gridStep + (cornerMin.y-cornerMax.y)/sceneViewSettings().gridStep) < 200) &&
        ((cornerMax.x-cornerMin.x)/sceneViewSettings().gridStep > 0) &&
        ((cornerMin.y-cornerMax.y)/sceneViewSettings().gridStep > 0))
    {
        for (int i = 0; i<cornerMax.x/sceneViewSettings().gridStep; i++)
        {
            glVertex2d(i*sceneViewSettings().gridStep, cornerMin.y);
            glVertex2d(i*sceneViewSettings().gridStep, cornerMax.y);
        }
        for (int i = 0; i>cornerMin.x/sceneViewSettings().gridStep; i--)
        {
            glVertex2d(i*sceneViewSettings().gridStep, cornerMin.y);
            glVertex2d(i*sceneViewSettings().gridStep, cornerMax.y);
        }
        // horizontal lines

        for (int i = 0; i<cornerMin.y/sceneViewSettings().gridStep; i++)
        {
            glVertex2d(cornerMin.x, i*sceneViewSettings().gridStep);
            glVertex2d(cornerMax.x, i*sceneViewSettings().gridStep);
        }
        for (int i = 0; i>cornerMax.y/sceneViewSettings().gridStep; i--)
        {
            glVertex2d(cornerMin.x, i*sceneViewSettings().gridStep);
            glVertex2d(cornerMax.x, i*sceneViewSettings().gridStep);
        }
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    // axes
    glColor3f(0.5, 1.0, 0.7);
    glBegin(GL_LINES);
    // y axis
    glVertex2d(0, cornerMin.y);
    glVertex2d(0, cornerMax.y);
    // x axis
    glVertex2d(cornerMin.x, 0);
    glVertex2d(cornerMax.x, 0);
    glEnd();
}

void SceneView::paintGeometry()
{
    double drawScaleMax = qMax(m_drawScaleX, m_drawScaleY);

    glDisable(GL_TEXTURE_1D);

    // edges
    foreach (SceneEdge *edge, m_scene->edges)
    {
        glColor3f(0.0, 0.1, 0.6);
        glLineWidth(2.0);
        if (edge->isHighlighted)
        {
            glColor3d(0.8, 0.5, 0.2);
            glLineWidth(3.0);
        }
        if (edge->isSelected)
        {
            glColor3f(1.0, 0.0, 0.0);
            glLineWidth(3.0);
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

            drawArc(center, radius, startAngle, edge->angle, edge->angle/5);
        }
        glLineWidth(1.0);
    }

    // nodes
    if (!(m_sceneMode == SCENEMODE_POSTPROCESSOR))
    {
        foreach (SceneNode *node, m_scene->nodes)
        {
            glColor3f(0.7, 0, 0);
            glPointSize(7.0);
            glBegin(GL_POINTS);
            glVertex2d(node->point.x, node->point.y);
            glEnd();

            glColor3f(1.0, 1.0, 1.0);
            glPointSize(5.0);
            glBegin(GL_POINTS);
            glVertex2d(node->point.x, node->point.y);
            glEnd();

            if ((node->isSelected) || (node->isHighlighted))
            {
                if (node->isHighlighted) glColor3d(0.8, 0.5, 0.2);
                if (node->isSelected) glColor3f(1.0, 0.0, 0.0);

                glPointSize(5.0);
                glBegin(GL_POINTS);
                glVertex2d(node->point.x, node->point.y);
                glEnd();
            }
        }

        glLineWidth(1.0);
    }
    // labels
    if (!(m_sceneMode == SCENEMODE_POSTPROCESSOR))
    {
        foreach (SceneLabel *label, m_scene->labels) {
            glColor3f(0.0, 0.7, 0.0);

            glPointSize(7.0);
            glBegin(GL_POINTS);
            glVertex2d(label->point.x, label->point.y);
            glEnd();

            glColor3f(1.0, 1.0, 1.0);
            glPointSize(5.0);
            glBegin(GL_POINTS);
            glVertex2d(label->point.x, label->point.y);
            glEnd();

            if ((label->isSelected) || (label->isHighlighted))
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                if (label->isHighlighted) glColor3d(0.8, 0.5, 0.2);
                if (label->isSelected) glColor3f(1.0, 0.0, 0.0);

                glPointSize(5.0);
                glBegin(GL_POINTS);
                glVertex2d(label->point.x, label->point.y);
                glEnd();
            }
            glLineWidth(1.0);

            if (m_sceneMode == SCENEMODE_OPERATE_ON_LABELS)
            {
                glColor3f(0.1, 0.1, 0.1);

                Point point;
                point.x = 2.0/width()*m_drawScaleY*fontMetrics().width(label->marker->name)/m_scale/2.0;
                point.y = 2.0/height()*m_drawScaleX*fontMetrics().height()/m_scale;

                renderText(label->point.x-point.x, label->point.y-point.y, 0, label->marker->name);
            }

            // area size
            if ((m_sceneMode == SCENEMODE_OPERATE_ON_LABELS) || (m_sceneViewSettings.showInitialMesh))
            {
                double radius = sqrt(label->area/(2.0*M_PI));
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
    // draw initial mesh
    glDisable(GL_TEXTURE_1D);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(0.83, 0.83, 0.0);

    // triangles
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < m_scene->sceneSolution()->mesh().get_num_elements(); i++)
    {
        Element *element = m_scene->sceneSolution()->mesh().get_element(i);
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
    if (m_scene->sceneSolution()->isSolved())
    {
        // draw solution mesh
        glDisable(GL_TEXTURE_1D);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3d(0.1, 0.1, 0.0);

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
}

void SceneView::paintOrder()
{
    if (m_scene->sceneSolution()->isSolved())
    {
        m_scene->sceneSolution()->ordView().lock_data();

        double3* vert = m_scene->sceneSolution()->ordView().get_vertices();
        int3* tris = m_scene->sceneSolution()->ordView().get_triangles();

        // draw mesh
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        int min = 11;
        int max = 1;
        for (int i = 0; i < m_scene->sceneSolution()->ordView().get_num_triangles(); i++)
        {
            if (vert[tris[i][0]][2] < min) min = vert[tris[i][0]][2];
            if (vert[tris[i][0]][2] > max) max = vert[tris[i][0]][2];
        }
        int num_boxes = max - min + 1;

        // triangles
        const float* color;
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_scene->sceneSolution()->ordView().get_num_triangles(); i++)
        {
            int color = vert[tris[i][0]][2];
            glColor3d(palette_order[color][0], palette_order[color][1], palette_order[color][2]);

            glVertex2d(vert[tris[i][0]][0], vert[tris[i][0]][1]);
            glVertex2d(vert[tris[i][1]][0], vert[tris[i][1]][1]);
            glVertex2d(vert[tris[i][2]][0], vert[tris[i][2]][1]);
        }
        glEnd();
        m_scene->sceneSolution()->ordView().unlock_data();

        // boxes
        glPushMatrix();
        glLoadIdentity();

        // glScaled(m_drawScaleY/m_drawScaleX, m_drawScaleX/m_drawScaleY, 1.0);

        double bottom = -0.98;
        double left = -0.98;
        double box_width = 0.08;
        double box_height = 0.09;
        double box_height_space = 0.02;
        double border = 0.02;

        // blend box
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0, 1.0, 1.0, 0.75);
        glBegin(GL_QUADS);
        glVertex2d(left - border, bottom - border);
        glVertex2d(left + box_width + border, bottom - border);
        glVertex2d(left + box_width + border, bottom + max*box_height);
        glVertex2d(left - border, bottom + max*box_height);
        glEnd();

        glBegin(GL_QUADS);
        for (int i = 0; i < max; i++)
        {
            glColor3d(palette_order[i][0], palette_order[i][1], palette_order[i][2]);

            glVertex2d(left, bottom + (i*box_height));
            glVertex2d(left + box_width, bottom + (i*box_height));
            glVertex2d(left + box_width, bottom + ((i+1)*box_height-box_height_space));
            glVertex2d(left, bottom + ((i+1)*box_height-box_height_space));
        }
        glEnd();

        // labels
        for (int i = 0; i < max; i++)
        {
            glColor3f(0.0, 0.0, 0.0);
            double w = 2.0*fontMetrics().width(QString::number(i+1))/height();            
            renderText(left + box_width/2.0 - w/2.0, bottom + (i*box_height) + (box_height - box_height_space)/2.0-0.015, 0.0, QString::number(i+1),
                       QFont("Helvetica", 10, QFont::Normal));
        }

        glPopMatrix();
    }
}

void SceneView::paintColorBar(double min, double max)
{
    glPushMatrix();
    glLoadIdentity();

    double k = 700.0/(double) height() * (m_drawScaleX/m_drawScaleY);

    double labels_width = 0.18*k;
    double scale_width = 0.04*k;
    double scale_height = 0.93;
    double scale_numticks = 9;
    double border = 0.007*k;
    double border_scale = 0.05*k;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_1D);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // background
    const int b = 5;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0, 1.0, 1.0, 0.75);
    glBegin(GL_QUADS);
    glVertex2d(1.0 - scale_width - labels_width - border_scale, - scale_height - border_scale);
    glVertex2d(1.0 - border_scale/2.0, - scale_height - border_scale);
    glVertex2d(1.0 - border_scale/2.0, scale_height + border_scale);
    glVertex2d(1.0 - scale_width - labels_width - border_scale, scale_height + border_scale);
    glEnd();

    // palette
    glDisable(GL_BLEND);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2d(1.0 - scale_width - labels_width - border, - scale_height - border);
    glVertex2d(1.0 - labels_width + border, - scale_height - border);
    glVertex2d(1.0 - labels_width + border, scale_height + border);
    glVertex2d(1.0 - scale_width - labels_width - border, scale_height + border);
    glEnd();

    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, 1);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBegin(GL_QUADS);
    glTexCoord1d(m_texScale + m_texShift);
    glVertex2d(1.0 - labels_width, scale_height);
    glVertex2d(1.0 - scale_width - labels_width, scale_height);
    glTexCoord1d(m_texShift);
    glVertex2d(1.0 - scale_width - labels_width, - scale_height);
    glVertex2d(1.0 - labels_width, - scale_height);
    glEnd();

    // ticks
    glColor3f(0, 0, 0);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_STIPPLE);
    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (int i = 0; i < scale_numticks; i++)
    {
        double y_tick = scale_height - (double) (i+1) * 2*scale_height / (scale_numticks+1);

        glVertex2d(1.0 - scale_width - labels_width - border, y_tick);
        glVertex2d(1.0 - scale_width - labels_width + border, y_tick);
        glVertex2d(1.0 - labels_width - border, y_tick);
        glVertex2d(1.0 - labels_width + border, y_tick);
    }
    glEnd();

    // labels
    double h = 2.0*fontMetrics().height()/height();

    for (int i = 0; i < scale_numticks+2; i++)
    {
        double value = min + (double) i * (max - min) / (scale_numticks+1);
        if (fabs(value) < 1e-8) value = 0.0;
        double y_tick = - scale_height + (double) i * 2*scale_height / (scale_numticks+1);

        renderText(1.0 - labels_width + 0.022*(m_drawScaleX/m_drawScaleY), y_tick-h/4.0, 0, QString::number(value, '+e', 1));
    }

    glPopMatrix();
}

void SceneView::paintScalarField()
{
    // range
    double irange = 1.0 / (m_sceneViewSettings.scalarRangeMax - m_sceneViewSettings.scalarRangeMin);
    // special case: constant solution
    if (fabs(m_sceneViewSettings.scalarRangeMin - m_sceneViewSettings.scalarRangeMax) < 1e-8) { irange = 1.0; m_sceneViewSettings.scalarRangeMin -= 0.5; }

    m_scene->sceneSolution()->linScalarView().lock_data();

    double3* linVert = m_scene->sceneSolution()->linScalarView().get_vertices();
    int3* linTris = m_scene->sceneSolution()->linScalarView().get_triangles();
    Point point[3];
    double value[3];

    // draw all triangles
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, 1);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

        glTexCoord2d((value[0] - m_sceneViewSettings.scalarRangeMin) * irange * m_texScale + m_texShift, 0.0);
        glVertex2d(point[0].x, point[0].y);

        glTexCoord2d((value[1] - m_sceneViewSettings.scalarRangeMin) * irange * m_texScale + m_texShift, 0.0);
        glVertex2d(point[1].x, point[1].y);

        glTexCoord2d((value[2] - m_sceneViewSettings.scalarRangeMin) * irange * m_texScale + m_texShift, 0.0);
        glVertex2d(point[2].x, point[2].y);
    }
    glEnd();
    glDisable(GL_TEXTURE_1D);

    m_scene->sceneSolution()->linScalarView().unlock_data();

    paintColorBar(m_sceneViewSettings.scalarRangeMin, m_sceneViewSettings.scalarRangeMax);
}

void SceneView::paintContours()
{
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
    double step = (rangeMax-rangeMin)/m_sceneViewSettings.contoursCount;

    // draw contours
    glDisable(GL_TEXTURE_1D);
    glColor3f(0.0, 0.0, 0.0);
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
    m_scene->sceneSolution()->vecVectorView().lock_data();

    RectPoint rect = m_scene->boundingBox();

    double gs = (rect.width() + rect.height()) / 45.0;

    int nx = ceil(rect.width() / gs);
    int ny = ceil(rect.height() / gs);

    double irange = 1.0 / (m_sceneViewSettings.vectorRangeMax - m_sceneViewSettings.vectorRangeMin);
    // special case: constant solution
    if (fabs(m_sceneViewSettings.vectorRangeMin - m_sceneViewSettings.vectorRangeMax) < 1e-8) { irange = 1.0; m_sceneViewSettings.vectorRangeMin -= 0.5; }

    double4* vecVert = m_scene->sceneSolution()->vecVectorView().get_vertices();
    int3* vecTris = m_scene->sceneSolution()->vecVectorView().get_triangles();

    glColor3f(0.7, 0, 0);
    // glEnable(GL_TEXTURE_1D);
    // glBindTexture(GL_TEXTURE_1D, 1);
    // glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i <= nx; i++)
    {
        for (int j = 0; j <= ny; j++)
        {
            double x = rect.start.x + i*gs;
            double y = rect.start.y + j*gs;

            int index = m_scene->sceneSolution()->findTriangleInVectorizer(m_scene->sceneSolution()->vecVectorView(), Point(x, y));
            if (index > 0)
            {
                double dx = (vecVert[vecTris[index][0]][2] + vecVert[vecTris[index][1]][2] + vecVert[vecTris[index][2]][2]) / 3.0;
                double dy = (vecVert[vecTris[index][0]][3] + vecVert[vecTris[index][1]][3] + vecVert[vecTris[index][2]][3]) / 3.0;

                double value = sqrt(sqr(dx) + sqr(dy));
                double angle = atan2(dy, dx);

                // glTexCoord2d((value - m_sceneViewSettings.vectorRangeMin) * irange * m_texScale + m_texShift, 0.0);

                dx = gs/1.6 * cos(angle);
                dy = gs/1.6 * sin(angle);
                // dx = (dx - m_sceneViewSettings.vectorRangeMin) * irange * gs;
                // dy = (dy - m_sceneViewSettings.vectorRangeMin) * irange * gs;
                double dm = sqrt(sqr(dx) + sqr(dy));

                glVertex2d(x + dm/5.0 * cos(angle - M_PI_2), y + dm/5.0 * sin(angle - M_PI_2));
                glVertex2d(x + dm/5.0 * cos(angle + M_PI_2), y + dm/5.0 * sin(angle + M_PI_2));
                glVertex2d(x + dm * cos(angle), y + dm * sin(angle));
            }
        }
    }
    glEnd();

    m_scene->sceneSolution()->vecVectorView().unlock_data();

    // paintColorBar(m_sceneViewSettings.vectorRangeMin, m_sceneViewSettings.vectorRangeMax);
}

void SceneView::paintSceneModeLabel()
{
    QString text = "";

    glPushMatrix();
    glLoadIdentity();

    switch (m_sceneMode)
    {
    case SCENEMODE_OPERATE_ON_NODES:
        text = tr("Operate on nodes");
        break;
    case SCENEMODE_OPERATE_ON_EDGES:
        text = tr("Operate on edges");
        break;
    case SCENEMODE_OPERATE_ON_LABELS:
        text = tr("Operate on labels");
        break;
    case SCENEMODE_POSTPROCESSOR:
        if (m_sceneViewSettings.showScalarField)
            text = physicFieldVariableString(m_sceneViewSettings.scalarPhysicFieldVariable);
        else
            text = tr("Postprocessor");
        break;
    }

    double w = 2.0*fontMetrics().width(text)/height();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0, 1.0, 1.0, 0.75);
    glBegin(GL_QUADS);
    glVertex2d(0.0 - w/1.5, 0.94);
    glVertex2d(0.0 + w/1.5, 0.94);
    glVertex2d(0.0 + w/1.5, 1.0);
    glVertex2d(0.0 - w/1.5, 1.0);
    glEnd();

    glColor3f(0, 0, 0);
    renderText(0.0 - w/2.0, 0.96, 0.0, text);

    glPopMatrix();
}

const float* SceneView::paletteColor(double x)
{
    switch (m_sceneViewSettings.paletteType)
    {
    case PALETTE_JET:
        {
            if (x < 0.0) x = 0.0;
            else if (x > 1.0) x = 1.0;
            x *= num_pal_entries;
            int n = (int) x;
            return palette_data_jet[n];
        }
        break;
    case PALETTE_AUTUMN:
        {
            if (x < 0.0) x = 0.0;
            else if (x > 1.0) x = 1.0;
            x *= num_pal_entries;
            int n = (int) x;
            return palette_data_autumn[n];
        }
        break;
    case PALETTE_COPPER:
        {
            if (x < 0.0) x = 0.0;
            else if (x > 1.0) x = 1.0;
            x *= num_pal_entries;
            int n = (int) x;
            return palette_data_copper[n];
        }
        break;
    case PALETTE_HOT:
        {
            if (x < 0.0) x = 0.0;
            else if (x > 1.0) x = 1.0;
            x *= num_pal_entries;
            int n = (int) x;
            return palette_data_hot[n];
        }
        break;
    case PALETTE_COOL:
        {
            if (x < 0.0) x = 0.0;
            else if (x > 1.0) x = 1.0;
            x *= num_pal_entries;
            int n = (int) x;
            return palette_data_cool[n];
        }
        break;
    case PALETTE_BW_ASC:
        {
            static float color[3];
            color[0] = color[1] = color[2] = x;
            return color;
        }
        break;
    case PALETTE_BW_DESC:
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
    for (i = 0; i < m_sceneViewSettings.paletteSteps; i++)
    {
        const float* color = paletteColor((double) i / m_sceneViewSettings.paletteSteps);
        palette[i][0] = (unsigned char) (color[0] * 255);
        palette[i][1] = (unsigned char) (color[1] * 255);
        palette[i][2] = (unsigned char) (color[2] * 255);
    }
    for (i = m_sceneViewSettings.paletteSteps; i < 256; i++)
        memcpy(palette[i], palette[m_sceneViewSettings.paletteSteps-1], 3);

    glBindTexture(GL_TEXTURE_1D, 1);
    glTexImage1D(GL_TEXTURE_1D, 0, 3, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);
#ifndef GL_CLAMP_TO_EDGE // fixme: this is needed on Windows
#define GL_CLAMP_TO_EDGE 0x812F
#endif
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

void SceneView::paletteFilter()
{
    int pal_filter = m_sceneViewSettings.paletteFilter ? GL_LINEAR : GL_NEAREST;
    glBindTexture(GL_TEXTURE_1D, 1);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, pal_filter);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, pal_filter);
    paletteUpdateTexAdjust();
}

void SceneView::paletteUpdateTexAdjust()
{
    if (m_sceneViewSettings.paletteFilter)
    {
        m_texScale = (double) (m_sceneViewSettings.paletteSteps-1) / 256.0;
        m_texShift = 0.5 / 256.0;
    }
    else
    {
        m_texScale = (double) m_sceneViewSettings.paletteSteps / 256.0;
        m_texShift = 0.0;
    }
}

// events *****************************************************************************************************************************

void SceneView::keyPressEvent(QKeyEvent *event)
{
    RectPoint rect = m_scene->boundingBox();

    switch (event->key())
    {
    case Qt::Key_Up:
        {
            m_offset.y += rect.height()/m_scale;
            doRefresh();
        }
        break;
    case Qt::Key_Down:
        {
            m_offset.y -= rect.height()/m_scale;
            doRefresh();
        }
        break;
    case Qt::Key_Left:
        {
            m_offset.x -= rect.width()/m_scale;
            doRefresh();
        }
        break;
    case Qt::Key_Right:
        {
            m_offset.x += rect.width()/m_scale;
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
    case Qt::Key_Escape:
        {
            m_scene->selectNone();
            emit mousePressed(volumeIntegralValueFactory(m_scene));
            emit mousePressed(surfaceIntegralValueFactory(m_scene));
            doRefresh();
        }
        break;
    default:
        QGLWidget::keyPressEvent(event);
    }

    // select all
    if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_A))
    {
        if (m_sceneMode == SCENEMODE_POSTPROCESSOR)
        {
            // select volume integral area
            if (actPostprocessorModeVolumeIntegral->isChecked())
            {
                m_scene->selectAll(SCENEMODE_OPERATE_ON_LABELS);
                emit mousePressed(volumeIntegralValueFactory(m_scene));
            }

            // select surface integral area
            if (actPostprocessorModeSurfaceIntegral->isChecked())
            {
                m_scene->selectAll(SCENEMODE_OPERATE_ON_EDGES);
                emit mousePressed(surfaceIntegralValueFactory(m_scene));
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
            return;
        }

        // select region
        if (actSceneViewSelectRegion->isChecked())
        {
            m_regionPos = m_lastPos;
            actSceneViewSelectRegion->setChecked(false);
            actSceneViewSelectRegion->setData(true);
            return;
        }

        if (m_sceneMode == SCENEMODE_POSTPROCESSOR)
        {
            // local point value
            if (actPostprocessorModeLocalPointValue->isChecked())
                emit mousePressed(localPointValueFactory(p, m_scene));
            // select volume integral area
            if (actPostprocessorModeVolumeIntegral->isChecked())
            {
                int index = m_scene->sceneSolution()->findTriangleInMesh(m_scene->sceneSolution()->mesh(), p);
                if (index > 0)
                {
                    //  find label marker
                    int labelIndex = m_scene->sceneSolution()->mesh().get_element_fast(index)->marker;

                    m_scene->labels[labelIndex]->isSelected = !m_scene->labels[labelIndex]->isSelected;
                    updateGL();
                }
                emit mousePressed(volumeIntegralValueFactory(m_scene));
            }
            // select surface integral area
            if (actPostprocessorModeSurfaceIntegral->isChecked())
            {
                //  find edge marker
                SceneEdge *edge = findClosestEdge(p);

                edge->isSelected = !edge->isSelected;
                updateGL();

                emit mousePressed(surfaceIntegralValueFactory(m_scene));
            }
        }
    }

    // add node edge or label by mouse click
    if (event->modifiers() & Qt::ControlModifier)
    {
        // add node directly by mouse click
        if (m_sceneMode == SCENEMODE_OPERATE_ON_NODES)
        {
            SceneNode *node = new SceneNode(p);
            m_scene->addNode(node);
            updateGL();
        }
        if (m_sceneMode == SCENEMODE_OPERATE_ON_EDGES)
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
                        m_scene->addEdge(new SceneEdge(m_nodeLast, node, m_scene->edgeMarkers[0], 0));

                    m_nodeLast->isSelected = false;
                    m_nodeLast = NULL;
                }

                updateGL();
            }
        }
        // add label directly by mouse click
        if (m_sceneMode == SCENEMODE_OPERATE_ON_LABELS)
        {
            SceneLabel *label = new SceneLabel(p, m_scene->labelMarkers[0], 0);
            m_scene->addLabel(label);
            updateGL();
        }
    }

    if ((event->modifiers() == 0) && (event->button() & Qt::LeftButton))
    {
        // select scene objects
        if (m_sceneMode == SCENEMODE_OPERATE_ON_NODES)
        {
            // select the closest node
            SceneNode *node = findClosestNode(p);
            if (node)
            {
                node->isSelected = !node->isSelected;
                updateGL();
            }
        }

        if (m_sceneMode == SCENEMODE_OPERATE_ON_EDGES)
        {
            // select the closest label
            SceneEdge *edge = findClosestEdge(p);
            if (edge)
            {
                edge->isSelected = !edge->isSelected;
                updateGL();
            }
        }
        if (m_sceneMode == SCENEMODE_OPERATE_ON_LABELS)
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
        if (event->button() & Qt::MidButton)
        {
            doZoomBestFit();
        }

        if (event->button() & Qt::LeftButton)
        {
            // select scene objects
            m_scene->selectNone();
            if (m_sceneMode == SCENEMODE_OPERATE_ON_NODES)
            {
                // select the closest node
                SceneNode *node = findClosestNode(p);
                if (node)
                {
                    node->isSelected = true;
                    updateGL();
                    if (node->showDialog(m_scene, this) == QDialog::Accepted)
                    {
                        updateGL();
                    }
                }
            }
            if (m_sceneMode == SCENEMODE_OPERATE_ON_EDGES)
            {
                // select the closest label
                SceneEdge *edge = findClosestEdge(p);
                if (edge)
                {
                    edge->isSelected = true;
                    updateGL();
                    if (edge->showDialog(m_scene, this) == QDialog::Accepted)
                    {
                        updateGL();
                    }
                }
            }
            if (m_sceneMode == SCENEMODE_OPERATE_ON_LABELS)
            {
                // select the closest label
                SceneLabel *label = findClosestLabel(p);
                if (label)
                {
                    label->isSelected = true;
                    updateGL();
                    if (label->showDialog(m_scene, this) == QDialog::Accepted)
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

    if (!m_regionPos.isNull())
    {
        Point posStart = position(Point(m_regionPos.x(), m_regionPos.y()));
        Point posEnd = position(Point(m_lastPos.x(), m_lastPos.y()));

        if (actSceneZoomRegion->data().value<bool>())
            doZoomRegion(Point(qMin(posStart.x, posEnd.x), qMin(posStart.y, posEnd.y)), Point(qMax(posStart.x, posEnd.x), qMax(posStart.y, posEnd.y)));
        if (actSceneViewSelectRegion->data().value<bool>())
            selectRegion(Point(qMin(posStart.x, posEnd.x), qMin(posStart.y, posEnd.y)), Point(qMax(posStart.x, posEnd.x), qMax(posStart.y, posEnd.y)));

        actSceneZoomRegion->setData(false);
        actSceneViewSelectRegion->setData(false);

        m_regionPos.setX(NULL);
        m_regionPos.setY(NULL);

        updateGL();
    }

}

void SceneView::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    m_lastPos = event->pos();

    Point p = position(Point(m_lastPos.x(), m_lastPos.y()));

    setToolTip("");

    // zoom or select region
    if (event->buttons() & Qt::LeftButton)
    {
        if (!m_regionPos.isNull())
            updateGL();
    }

    // pan
    if (event->buttons() & Qt::MidButton)
    {
        setCursor(Qt::PointingHandCursor);

        m_offset.x -= 2.0/width()*dx/m_scale/m_drawScaleX;
        m_offset.y += 2.0/width()*dy/m_scale/m_drawScaleY;

        updateGL();
    }

    if (event->modifiers() == 0)
    {
        // highlight scene objects
        if (m_sceneMode == SCENEMODE_OPERATE_ON_NODES)
        {
            // highlight the closest node
            SceneNode *node = findClosestNode(p);
            if (node)
            {
                m_scene->highlightNone();
                node->isHighlighted = true;
                setToolTip(QString("<h4>Node</h4>Point: [%1; %2]").arg(node->point.x, 0, 'f', 3).arg(node->point.y, 0, 'f', 3));
                updateGL();
            }
        }
        if (m_sceneMode == SCENEMODE_OPERATE_ON_EDGES)
        {
            // highlight the closest label
            SceneEdge *edge = findClosestEdge(p);
            if (edge)
            {
                m_scene->highlightNone();
                edge->isHighlighted = true;
                setToolTip(QString("<h4>Edge</h4>Point: [%1; %2] - [%3; %4]<br/>Boundary Condition: %5<br/>Angle: %6 deg.").arg(edge->nodeStart->point.x, 0, 'f', 3).arg(edge->nodeStart->point.y, 0, 'f', 3)
                           .arg(edge->nodeEnd->point.x, 0, 'f', 3).arg(edge->nodeEnd->point.y, 0, 'f', 3)
                           .arg(edge->marker->name).arg(edge->angle, 0, 'f', 0));
                // edge->marker->name
                updateGL();
            }
        }
        if (m_sceneMode == SCENEMODE_OPERATE_ON_LABELS)
        {
            // highlight the closest label
            SceneLabel *label = findClosestLabel(p);
            if (label)
            {
                m_scene->highlightNone();
                label->isHighlighted = true;
                setToolTip(QString("<h4>Label</h4>Point: [%1; %2]<br/>Material: %3<br/>Triangle Area: %4 m<sup>2</sup>").arg(label->point.x, 0, 'f', 3).arg(label->point.y, 0, 'f', 3)
                           .arg(label->marker->name).arg(label->area, 0, 'f', 5));
                updateGL();
            }
        }
    }

    if (event->modifiers() & Qt::ControlModifier)
    {
        // add edge directly by mouse click - highlight
        if (m_sceneMode == SCENEMODE_OPERATE_ON_EDGES)
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

    emit mouseMoved(QPointF(p.x, p.y));
}

void SceneView::wheelEvent(QWheelEvent *event)
{
    setZoom(event->delta()/150.0);
}

void SceneView::contextMenuEvent(QContextMenuEvent *event)
{
    mnuMarkerGroup->clear();
    mnuMarkerGroup->setEnabled(false);

    // set boundary context menu
    if (m_sceneMode == SCENEMODE_OPERATE_ON_EDGES)
    {
        mnuMarkerGroup->setEnabled((m_scene->edgeMarkers.count() > 0));
        for (int i = 1; i<m_scene->edgeMarkers.count(); i++)
        {
            QAction *actMenuEdgeMarkerItem = new QAction(m_scene->edgeMarkers[i]->name, this);
            actMenuEdgeMarkerItem->setData(m_scene->edgeMarkers[i]->variant());
            actBoundaryGroup->addAction(actMenuEdgeMarkerItem);
            mnuMarkerGroup->addAction(actMenuEdgeMarkerItem);
        }
    }

    // set material context menu
    if (m_sceneMode == SCENEMODE_OPERATE_ON_LABELS)
    {
        mnuMarkerGroup->setEnabled((m_scene->labelMarkers.count() > 0));
        for (int i = 1; i<m_scene->labelMarkers.count(); i++)
        {
            QAction *actMenuLabelMarkerItem = new QAction(m_scene->labelMarkers[i]->name, this);
            actMenuLabelMarkerItem->setData(m_scene->labelMarkers[i]->variant());
            actMaterialGroup->addAction(actMenuLabelMarkerItem);
            mnuMarkerGroup->addAction(actMenuLabelMarkerItem);
        }
    }

    // show group
    mnuShowGroup->setEnabled((m_sceneMode == SCENEMODE_POSTPROCESSOR));
    actShowSolutionMesh->setChecked(m_sceneViewSettings.showSolutionMesh);
    actShowContours->setChecked(m_sceneViewSettings.showContours);
    actShowScalarField->setChecked(m_sceneViewSettings.showScalarField);
    actShowVectors->setChecked(m_sceneViewSettings.showVectors);

    mnuInfo->exec(event->globalPos());
}

// slots *****************************************************************************************************************************

void SceneView::doInvalidated()
{
    if (m_scene->sceneSolution()->isSolved())
    {
        setRangeContour();
        setRangeScalar();
        setRangeVector();

        paletteFilter();
        paletteUpdateTexAdjust();
        paletteCreate();
    }

    actSceneModePostprocessor->setEnabled(m_scene->sceneSolution()->isSolved());

    updateGL();
}

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

    double maxScene = ((width() / height()) < (sceneWidth / sceneWidth)) ? sceneWidth : sceneHeight;

    m_scale = 1.95/maxScene;
    setZoom(0);
}

void SceneView::doDefaults()
{
    m_scale = 1.0;
    m_offset.x = 0.0;
    m_offset.y = 0.0;

    m_sceneViewSettings.scalarRangeMin = 0;
    m_sceneViewSettings.scalarRangeMax = 1;

    // visible objects
    m_sceneViewSettings.showOrder = false;
    m_sceneViewSettings.showScalarField = true;
    m_sceneViewSettings.showVectors = false;
    m_sceneViewSettings.showContours = false;
    m_sceneViewSettings.showGrid = true;
    m_sceneViewSettings.showGeometry = true;
    m_sceneViewSettings.showInitialMesh = false;
    m_sceneViewSettings.showSolutionMesh = false;

    // settings
    m_sceneViewSettings.gridStep = 0.05;

    m_sceneViewSettings.contoursCount = 15;
    m_sceneViewSettings.paletteType = PALETTE_JET;
    m_sceneViewSettings.paletteFilter = false;
    m_sceneViewSettings.paletteSteps = 30;
    m_sceneViewSettings.scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_NONE;
    m_sceneViewSettings.scalarPhysicFieldVariableComp = PHYSICFIELDVARIABLECOMP_SCALAR;
    m_sceneViewSettings.scalarRangeAuto = true;
    switch (m_scene->projectInfo().physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        {
            m_sceneViewSettings.contourPhysicFieldVariable = PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL;
            m_sceneViewSettings.scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL;
            m_sceneViewSettings.vectorPhysicFieldVariable = PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD;
        }
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        {
            m_sceneViewSettings.contourPhysicFieldVariable = PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL;
            m_sceneViewSettings.scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY;
            m_sceneViewSettings.scalarPhysicFieldVariableComp = PHYSICFIELDVARIABLECOMP_MAGNITUDE;
            m_sceneViewSettings.vectorPhysicFieldVariable = PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY;
        }
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        {
            m_sceneViewSettings.contourPhysicFieldVariable = PHYSICFIELDVARIABLE_HEAT_TEMPERATURE;
            m_sceneViewSettings.scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HEAT_TEMPERATURE;
            m_sceneViewSettings.vectorPhysicFieldVariable = PHYSICFIELDVARIABLE_HEAT_FLUX;
        }
        break;
    case PHYSICFIELD_CURRENT:
        {
            m_sceneViewSettings.contourPhysicFieldVariable = PHYSICFIELDVARIABLE_CURRENT_POTENTIAL;
            m_sceneViewSettings.scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_CURRENT_POTENTIAL;
            m_sceneViewSettings.vectorPhysicFieldVariable = PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY;
        }
        break;
    case PHYSICFIELD_ELASTICITY:
        {
            m_sceneViewSettings.contourPhysicFieldVariable = PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS;
            m_sceneViewSettings.scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS;
            // m_sceneViewSettings.vectorPhysicFieldVariable = PHYSICFIELDVARIABLE_HEAT_FLUX;
        }
        break;
    default:
        cerr << "Physical field '" + physicFieldStringKey(m_scene->projectInfo().physicField).toStdString() + "' is not implemented. SceneView::doDefaults()" << endl;
        throw;
        break;
    }

    doInvalidated();
    doZoomBestFit();

    actPostprocessorModeLocalPointValue->trigger();
}

void SceneView::doSolved()
{
    m_sceneViewSettings.showInitialMesh = false;
    m_sceneViewSettings.showSolutionMesh = false;
    m_sceneViewSettings.showScalarField = true;
    m_sceneViewSettings.showContours = false;
    m_sceneViewSettings.showVectors = false;
    m_sceneViewSettings.showOrder = false;

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
        m_scene->setLabelMarker(labelMarker);
}

void SceneView::doBoundaryGroup(QAction *action)
{
    if (SceneEdgeMarker *edgeMarker = action->data().value<SceneEdgeMarker *>())
        m_scene->setEdgeMarker(edgeMarker);
}

void SceneView::doShowGroup(QAction *action)
{
    m_sceneViewSettings.showSolutionMesh = actShowSolutionMesh->isChecked();
    m_sceneViewSettings.showContours = actShowContours->isChecked();
    m_sceneViewSettings.showScalarField = actShowScalarField->isChecked();
    m_sceneViewSettings.showVectors = actShowVectors->isChecked();

    doInvalidated();
}

void SceneView::doPostprocessorModeGroup(QAction *action)
{
    m_scene->selectNone();
    updateGL();
}

void SceneView::doSceneViewProperties()
{
    SceneViewDialog *sceneViewDialog = new SceneViewDialog(this, this);
    if (sceneViewDialog->showDialog() == QDialog::Accepted)
        doInvalidated();
}

void SceneView::doSceneModeSet(QAction *)
{
    actSceneModePostprocessor->setEnabled(m_scene->sceneSolution()->isSolved());

    if (actSceneModeNode->isChecked()) m_sceneMode = SCENEMODE_OPERATE_ON_NODES;
    if (actSceneModeEdge->isChecked()) m_sceneMode = SCENEMODE_OPERATE_ON_EDGES;
    if (actSceneModeLabel->isChecked()) m_sceneMode = SCENEMODE_OPERATE_ON_LABELS;
    if (actSceneModePostprocessor->isChecked()) m_sceneMode = SCENEMODE_POSTPROCESSOR;

    m_scene->highlightNone();
    m_scene->selectNone();

    m_scene->actTransform->setEnabled((m_sceneMode != SCENEMODE_POSTPROCESSOR));
    actSceneViewSelectRegion->setEnabled((m_sceneMode != SCENEMODE_POSTPROCESSOR));

    actPostprocessorModeLocalPointValue->setEnabled((m_sceneMode == SCENEMODE_POSTPROCESSOR));
    actPostprocessorModeSurfaceIntegral->setEnabled((m_sceneMode == SCENEMODE_POSTPROCESSOR));
    actPostprocessorModeVolumeIntegral->setEnabled((m_sceneMode == SCENEMODE_POSTPROCESSOR));

    switch (m_sceneMode)
    {
    case SCENEMODE_OPERATE_ON_NODES:
        break;
    case SCENEMODE_OPERATE_ON_EDGES:
        m_nodeLast = NULL;
        break;
    case SCENEMODE_OPERATE_ON_LABELS:
        break;
    case SCENEMODE_POSTPROCESSOR:
        break;
    }

    doInvalidated();
}

void SceneView::setRangeContour()
{
    if (m_sceneMode == SCENEMODE_POSTPROCESSOR && m_sceneViewSettings.showContours)
    {
        ViewScalarFilter *viewScalarFilter = new ViewScalarFilter(m_scene->sceneSolution()->sln(),
                                                                  m_scene,
                                                                  m_sceneViewSettings.contourPhysicFieldVariable,
                                                                  PHYSICFIELDVARIABLECOMP_SCALAR);
        m_scene->sceneSolution()->setSlnContourView(viewScalarFilter);
    }
}

void SceneView::setRangeScalar()
{
    if (m_sceneMode == SCENEMODE_POSTPROCESSOR && m_sceneViewSettings.showScalarField)
    {
        ViewScalarFilter *viewScalarFilter;
        if (m_scene->projectInfo().physicField != PHYSICFIELD_ELASTICITY)
            viewScalarFilter = new ViewScalarFilter(m_scene->sceneSolution()->sln(),
                                                    m_scene,
                                                    m_sceneViewSettings.scalarPhysicFieldVariable,
                                                    m_sceneViewSettings.scalarPhysicFieldVariableComp);
        else
            viewScalarFilter = new ViewScalarFilter(m_scene->sceneSolution()->sln1(),
                                                    m_scene->sceneSolution()->sln2(),
                                                    m_scene,
                                                    m_sceneViewSettings.scalarPhysicFieldVariable,
                                                    m_sceneViewSettings.scalarPhysicFieldVariableComp);

        m_scene->sceneSolution()->setSlnScalarView(viewScalarFilter);

        if (m_sceneViewSettings.scalarRangeAuto)
        {
            m_sceneViewSettings.scalarRangeMin = m_scene->sceneSolution()->linScalarView().get_min_value();
            m_sceneViewSettings.scalarRangeMax = m_scene->sceneSolution()->linScalarView().get_max_value();
        }
    }
}

void SceneView::setRangeVector()
{
    if (m_sceneMode == SCENEMODE_POSTPROCESSOR && m_sceneViewSettings.showVectors)
    {
        ViewScalarFilter *viewVectorXFilter = new ViewScalarFilter(m_scene->sceneSolution()->sln(),
                                                                   m_scene,
                                                                   m_sceneViewSettings.vectorPhysicFieldVariable,
                                                                   PHYSICFIELDVARIABLECOMP_X);
        ViewScalarFilter *viewVectorYFilter = new ViewScalarFilter(m_scene->sceneSolution()->sln(),
                                                                   m_scene,
                                                                   m_sceneViewSettings.vectorPhysicFieldVariable,
                                                                   PHYSICFIELDVARIABLECOMP_Y);
        m_scene->sceneSolution()->setSlnVectorView(viewVectorXFilter, viewVectorYFilter);

        if (m_sceneViewSettings.vectorRangeAuto)
        {
            m_sceneViewSettings.vectorRangeMin = m_scene->sceneSolution()->vecVectorView().get_min_value();
            m_sceneViewSettings.vectorRangeMax = m_scene->sceneSolution()->vecVectorView().get_max_value();
        }
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
    case SCENEMODE_OPERATE_ON_NODES:
        foreach (SceneNode *node, m_scene->nodes)
            if (node->point.x >= start.x && node->point.x <= end.x && node->point.y >= start.y && node->point.y <= end.y)
                node->isSelected = true;
        break;
    case SCENEMODE_OPERATE_ON_EDGES:
        foreach (SceneEdge *edge, m_scene->edges)
            if (edge->nodeStart->point.x >= start.x && edge->nodeStart->point.x <= end.x && edge->nodeStart->point.y >= start.y && edge->nodeStart->point.y <= end.y &&
                edge->nodeEnd->point.x >= start.x && edge->nodeEnd->point.x <= end.x && edge->nodeEnd->point.y >= start.y && edge->nodeEnd->point.y <= end.y)
                edge->isSelected = true;
        break;
    case SCENEMODE_OPERATE_ON_LABELS:
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

void SceneView::paintPostprocessorSelectedVolume()
{
    // draw mesh
    glDisable(GL_TEXTURE_1D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor4d(0.2, 0.5, 0.2, 0.7);

    // triangles
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < m_scene->sceneSolution()->mesh().get_num_elements(); i++)
    {
        Element *element = m_scene->sceneSolution()->mesh().get_element(i);
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
    glDisable(GL_TEXTURE_1D);

    // edges
    foreach (SceneEdge *edge, m_scene->edges) {
        glColor3d(0.8, 0.5, 0.2);
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

void SceneView::saveImageToFile(const QString &fileName)
{
    makeCurrent();

    // copy image
    QImage *image = new QImage(grabFrameBuffer());
    image->save(fileName, "PNG");

    delete image;
}
