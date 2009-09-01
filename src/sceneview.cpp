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
    showGrid = true;
    showGeometry = true;
    showInitialMesh = false;

    postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW;

    showContours = false;
    showVectors = false;
    showSolutionMesh = false;

    scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_NONE;
    scalarPhysicFieldVariableComp = PHYSICFIELDVARIABLECOMP_SCALAR;
    scalarRangeAuto = true;

    switch (Util::scene()->problemInfo().physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        {
            contourPhysicFieldVariable = PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL;
            scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL;
            vectorPhysicFieldVariable = PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD;
        }
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        {
            contourPhysicFieldVariable = PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL;
            scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY;
            scalarPhysicFieldVariableComp = PHYSICFIELDVARIABLECOMP_MAGNITUDE;
            vectorPhysicFieldVariable = PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY;
        }
        break;
    case PHYSICFIELD_HARMONIC_MAGNETIC:
        {
            contourPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL;
            scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL;
            vectorPhysicFieldVariable = PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY;
        }
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        {
            contourPhysicFieldVariable = PHYSICFIELDVARIABLE_HEAT_TEMPERATURE;
            scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_HEAT_TEMPERATURE;
            vectorPhysicFieldVariable = PHYSICFIELDVARIABLE_HEAT_FLUX;
        }
        break;
    case PHYSICFIELD_CURRENT:
        {
            contourPhysicFieldVariable = PHYSICFIELDVARIABLE_CURRENT_POTENTIAL;
            scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_CURRENT_POTENTIAL;
            vectorPhysicFieldVariable = PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY;
        }
        break;
    case PHYSICFIELD_ELASTICITY:
        {
            contourPhysicFieldVariable = PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS;
            scalarPhysicFieldVariable = PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS;
            // vectorPhysicFieldVariable = PHYSICFIELDVARIABLE_HEAT_FLUX;
        }
        break;
    default:
        cerr << "Physical field '" + physicFieldStringKey(Util::scene()->problemInfo().physicField).toStdString() + "' is not implemented. SceneViewSettings::defaultValues()" << endl;
        throw;
        break;
    }
}

void SceneViewSettings::load()
{
    QSettings settings;

    // colors
    colorBackground = settings.value("SceneViewSettings/ColorBackground", QColor::fromRgb(255, 255, 255)).value<QColor>();
    colorGrid = settings.value("SceneViewSettings/ColorGrid", QColor::fromRgb(200, 200, 200)).value<QColor>();
    colorCross = settings.value("SceneViewSettings/ColorCross", QColor::fromRgb(150, 150, 150)).value<QColor>();
    colorNodes = settings.value("SceneViewSettings/ColorNodes", QColor::fromRgb(150, 0, 0)).value<QColor>();
    colorEdges = settings.value("SceneViewSettings/ColorEdges", QColor::fromRgb(0, 0, 150)).value<QColor>();
    colorLabels = settings.value("SceneViewSettings/ColorLabels", QColor::fromRgb(0, 150, 0)).value<QColor>();
    colorContours = settings.value("SceneViewSettings/ColorContours", QColor::fromRgb(0, 0, 0)).value<QColor>();
    colorVectors = settings.value("SceneViewSettings/ColorVectors", QColor::fromRgb(0, 0, 0)).value<QColor>();
    colorInitialMesh = settings.value("SceneViewSettings/ColorInitialMesh", QColor::fromRgb(250, 250, 0)).value<QColor>();
    colorSolutionMesh = settings.value("SceneViewSettings/ColorSolutionMesh", QColor::fromRgb(150, 70, 0)).value<QColor>();
    colorHighlighted = settings.value("SceneViewSettings/ColorHighlighted", QColor::fromRgb(250, 150, 0)).value<QColor>();
    colorSelected = settings.value("SceneViewSettings/ColorSelected", QColor::fromRgb(150, 0, 0)).value<QColor>();

    // geometry
    geometryNodeSize = settings.value("Geometry/NodeSize", 7.0).value<double>();
    geometryEdgeWidth = settings.value("Geometry/EdgeWidth", 2.0).value<double>();
    geometryLabelSize = settings.value("Geometry/LabelSize", 7.0).value<double>();

    // grid
    gridStep = settings.value("SceneViewSettings/GridStep", 0.05).value<double>();

    // countour
    contoursCount = settings.value("SceneViewSettings/ContoursCount", 15).value<int>();

    // scalar view
    paletteType = (PaletteType) settings.value("SceneViewSettings/PaletteType", PALETTE_JET).value<int>();
    paletteFilter = settings.value("SceneViewSettings/PaletteFilter", false).value<bool>();
    paletteSteps = settings.value("SceneViewSettings/PaletteSteps", 30).value<int>();

    // 3d
    scalarView3DLighting = settings.value("SceneViewSettings/ScalarView3DLighting", false).value<bool>();
}

void SceneViewSettings::save()
{
    QSettings settings;

    // colors
    settings.setValue("SceneViewSettings/ColorBackground", colorBackground);
    settings.setValue("SceneViewSettings/ColorGrid", colorGrid);
    settings.setValue("SceneViewSettings/ColorCross", colorCross);
    settings.setValue("SceneViewSettings/ColorNodes", colorNodes);
    settings.setValue("SceneViewSettings/ColorEdges", colorEdges);
    settings.setValue("SceneViewSettings/ColorLabels", colorLabels);
    settings.setValue("SceneViewSettings/ColorContours", colorContours);
    settings.setValue("SceneViewSettings/ColorVectors", colorVectors);
    settings.setValue("SceneViewSettings/ColorInitialMesh", colorInitialMesh);
    settings.setValue("SceneViewSettings/ColorSolutionMesh", colorSolutionMesh);
    settings.setValue("SceneViewSettings/ColorInitialMesh", colorHighlighted);
    settings.setValue("SceneViewSettings/ColorSolutionMesh", colorSelected);

    // geometry
    settings.setValue("Geometry/NodeSize", geometryNodeSize);
    settings.setValue("Geometry/EdgeWidth", geometryEdgeWidth);
    settings.setValue("Geometry/LabelSize", geometryLabelSize);

    // grid
    settings.setValue("SceneViewSettings/GridStep", gridStep);

    // countour
    settings.setValue("SceneViewSettings/ContoursCount", contoursCount);

    // scalar view
    settings.setValue("SceneViewSettings/PaletteType", paletteType);
    settings.setValue("SceneViewSettings/PaletteFilter", paletteFilter);
    settings.setValue("SceneViewSettings/PaletteSteps", paletteSteps);

    // 3d
    settings.setValue("SceneViewSettings/ScalarView3DLighting", scalarView3DLighting);
}

// *******************************************************************************************************

SceneView::SceneView(QWidget *parent): QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    m_mainWindow = (QMainWindow *) parent;
    
    m_normals = NULL;

    createActions();
    createMenu();
    
    doDefaults();
    
    setMinimumSize(200, 200);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    m_sceneViewSettings.load();
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
    
    // properties
    actSceneViewProperties = new QAction(icon("scene-properties"), tr("&Scene properties"), this);
    actSceneViewProperties->setStatusTip(tr("Properties"));
    connect(actSceneViewProperties, SIGNAL(triggered()), this, SLOT(doSceneViewProperties()));
    
    // select region
    actSceneViewSelectRegion = new QAction(icon("scene-select-region"), tr("&Select region"), this);
    actSceneViewSelectRegion->setStatusTip(tr("Select region"));
    actSceneViewSelectRegion->setCheckable(true);

    // fullscreen
    actFullScreen = new QAction(icon(""), tr("Fullscreen mode"), this);
    actFullScreen->setShortcut(QKeySequence(tr("F11")));
    connect(actFullScreen, SIGNAL(triggered()), this, SLOT(doFullScreen()));
}

void SceneView::createMenu()
{
    mnuInfo = new QMenu(this);
    mnuMarkerGroup = new QMenu(tr("Set marker"), this);

    QMenu *mnuModeGroup = new QMenu(tr("Set mode"), this);
    mnuModeGroup->addAction(actSceneModeNode);
    mnuModeGroup->addAction(actSceneModeEdge);
    mnuModeGroup->addAction(actSceneModeLabel);
    mnuModeGroup->addAction(actSceneModePostprocessor);

    mnuInfo->addAction(Util::scene()->actNewNode);
    mnuInfo->addAction(Util::scene()->actNewEdge);
    mnuInfo->addAction(Util::scene()->actNewLabel);
    mnuInfo->addSeparator();
    mnuInfo->addAction(Util::scene()->actNewEdgeMarker);
    mnuInfo->addAction(Util::scene()->actNewLabelMarker);
    mnuInfo->addSeparator();
    mnuInfo->addAction(actSceneViewSelectRegion);
    mnuInfo->addAction(Util::scene()->actTransform);
    mnuInfo->addSeparator();
    mnuInfo->addMenu(mnuMarkerGroup);
    mnuInfo->addMenu(mnuModeGroup);
    mnuInfo->addSeparator();
    mnuInfo->addAction(actSceneViewProperties);
}

void SceneView::initializeGL()
{
    glShadeModel(GL_SMOOTH);
}

void SceneView::setupViewport()
{
    glViewport(0, 0, width(), height());

    m_aspect = (double) width()/(double) height();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if ((m_sceneMode == SCENEMODE_POSTPROCESSOR) &&
        (m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D || m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID))
    {
        double aspect = ((double) width()/(double) height());
        gluPerspective(0.0, aspect, 1.0, 1000.0);
    }
    else
        glOrtho(5.0, width()-10.0, height()-10.0, 5.0, -10.0, -10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void SceneView::paintGL()
{
    glClearColor(m_sceneViewSettings.colorBackground.redF(), m_sceneViewSettings.colorBackground.greenF(), m_sceneViewSettings.colorBackground.blueF(), 0);
    glClear(GL_COLOR_BUFFER_BIT);

    if ((m_sceneMode == SCENEMODE_POSTPROCESSOR) &&
        (m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D || m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID))
    {
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }     

    setupViewport();

    glScaled(m_scale/m_aspect, m_scale, m_scale);

    if ((m_sceneMode == SCENEMODE_POSTPROCESSOR) &&
        (m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D || m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID))
    {
        glRotated(m_rotation.x, 1.0, 0.0, 0.0);
        glRotated(m_rotation.y, 0.0, 0.0, 1.0);
    }

    glTranslated(-m_offset.x, -m_offset.y, -m_offset.z);

    if (m_sceneViewSettings.showGrid) paintGrid();
    if (Util::scene()->sceneSolution()->isSolved())
    {
        if (m_sceneMode == SCENEMODE_POSTPROCESSOR)
        {
            switch (m_sceneViewSettings.postprocessorShow)
            {
            case SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW:
                paintScalarField();
                break;
            case SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D:
                paintScalarField3D();
                break;
            case SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID:
                paintScalarField3DSolid();
                break;
            case SCENEVIEW_POSTPROCESSOR_SHOW_ORDER:
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
    
    if (m_sceneMode == SCENEMODE_POSTPROCESSOR)
    {
        if (actPostprocessorModeVolumeIntegral->isChecked()) paintPostprocessorSelectedVolume();
        if (actPostprocessorModeSurfaceIntegral->isChecked()) paintPostprocessorSelectedSurface();
    }

    paintZoomRegion();
    paintChartLine();
    paintSceneModeLabel();
}

void SceneView::resizeGL(int width, int height)
{
    setupViewport();
}

// paint *****************************************************************************************************************************

void SceneView::paintGrid()
{
    // if (m_sceneViewSettings.scalarView3D && m_sceneViewSettings.showScalarField && (m_sceneMode == SCENEMODE_POSTPROCESSOR))
    //    return;

    Point cornerMin = position(Point(0, 0));
    Point cornerMax = position(Point(width(), height()));
    
    glColor3f(m_sceneViewSettings.colorGrid.redF(), m_sceneViewSettings.colorGrid.greenF(), m_sceneViewSettings.colorGrid.blueF());
    glLineWidth(1.0);
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
    glColor3f(m_sceneViewSettings.colorCross.redF(), m_sceneViewSettings.colorCross.greenF(), m_sceneViewSettings.colorCross.blueF());
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
    // edges
    foreach (SceneEdge *edge, Util::scene()->edges)
    {
        glColor3f(m_sceneViewSettings.colorEdges.redF(), m_sceneViewSettings.colorEdges.greenF(), m_sceneViewSettings.colorEdges.blueF());
        glLineWidth(m_sceneViewSettings.geometryEdgeWidth);
        if (edge->isHighlighted)
        {
            glColor3f(m_sceneViewSettings.colorHighlighted.redF(), m_sceneViewSettings.colorHighlighted.greenF(), m_sceneViewSettings.colorHighlighted.blueF());
            glLineWidth(m_sceneViewSettings.geometryEdgeWidth + 2.0);
        }
        if (edge->isSelected)
        {
            glColor3f(m_sceneViewSettings.colorSelected.redF(), m_sceneViewSettings.colorSelected.greenF(), m_sceneViewSettings.colorSelected.blueF());
            glLineWidth(m_sceneViewSettings.geometryEdgeWidth + 2.0);
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
        glLineWidth(1.0);
    }
    
    // nodes
    if (!(m_sceneMode == SCENEMODE_POSTPROCESSOR))
    {
        foreach (SceneNode *node, Util::scene()->nodes)
        {
            glColor3f(m_sceneViewSettings.colorNodes.redF(), m_sceneViewSettings.colorNodes.greenF(), m_sceneViewSettings.colorNodes.blueF());
            glPointSize(m_sceneViewSettings.geometryNodeSize);
            glBegin(GL_POINTS);
            glVertex2d(node->point.x, node->point.y);
            glEnd();
            
            glColor3f(m_sceneViewSettings.colorBackground.redF(), m_sceneViewSettings.colorBackground.greenF(), m_sceneViewSettings.colorBackground.blueF());
            glPointSize(m_sceneViewSettings.geometryNodeSize - 2.0);
            glBegin(GL_POINTS);
            glVertex2d(node->point.x, node->point.y);
            glEnd();
            
            if ((node->isSelected) || (node->isHighlighted))
            {
                if (node->isHighlighted) glColor3f(m_sceneViewSettings.colorHighlighted.redF(), m_sceneViewSettings.colorHighlighted.greenF(), m_sceneViewSettings.colorHighlighted.blueF());
                if (node->isSelected) glColor3f(m_sceneViewSettings.colorSelected.redF(), m_sceneViewSettings.colorSelected.greenF(), m_sceneViewSettings.colorSelected.blueF());
                
                glPointSize(m_sceneViewSettings.geometryNodeSize - 2.0);
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
        foreach (SceneLabel *label, Util::scene()->labels)
        {
            glColor3f(m_sceneViewSettings.colorLabels.redF(), m_sceneViewSettings.colorLabels.greenF(), m_sceneViewSettings.colorLabels.blueF());
            glPointSize(m_sceneViewSettings.geometryLabelSize);
            glBegin(GL_POINTS);
            glVertex2d(label->point.x, label->point.y);
            glEnd();
            
            glColor3f(m_sceneViewSettings.colorBackground.redF(), m_sceneViewSettings.colorBackground.greenF(), m_sceneViewSettings.colorBackground.blueF());
            glPointSize(m_sceneViewSettings.geometryLabelSize - 2.0);
            glBegin(GL_POINTS);
            glVertex2d(label->point.x, label->point.y);
            glEnd();
            
            if ((label->isSelected) || (label->isHighlighted))
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                if (label->isHighlighted) glColor3f(m_sceneViewSettings.colorHighlighted.redF(), m_sceneViewSettings.colorHighlighted.greenF(), m_sceneViewSettings.colorHighlighted.blueF());
                if (label->isSelected) glColor3f(m_sceneViewSettings.colorSelected.redF(), m_sceneViewSettings.colorSelected.greenF(), m_sceneViewSettings.colorSelected.blueF());
                
                glPointSize(m_sceneViewSettings.geometryLabelSize - 2.0);
                glBegin(GL_POINTS);
                glVertex2d(label->point.x, label->point.y);
                glEnd();
            }
            glLineWidth(1.0);
            
            if (m_sceneMode == SCENEMODE_OPERATE_ON_LABELS)
            {
                glColor3f(0.1, 0.1, 0.1);
                
                Point point;
                point.x = 2.0/width()*m_aspect*fontMetrics().width(label->marker->name)/m_scale/2.0;
                point.y = 2.0/height()*fontMetrics().height()/m_scale;
                
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
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(m_sceneViewSettings.colorInitialMesh.redF(), m_sceneViewSettings.colorInitialMesh.greenF(), m_sceneViewSettings.colorInitialMesh.blueF());
    glLineWidth(1.0);

    // triangles
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < Util::scene()->sceneSolution()->mesh().get_num_elements(); i++)
    {
        Element *element = Util::scene()->sceneSolution()->mesh().get_element(i);
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
    if (Util::scene()->sceneSolution()->isSolved())
    {
        // draw solution mesh
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(m_sceneViewSettings.colorSolutionMesh.redF(), m_sceneViewSettings.colorSolutionMesh.greenF(), m_sceneViewSettings.colorSolutionMesh.blueF());
        glLineWidth(1.0);

        // triangles
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < Util::scene()->sceneSolution()->sln()->get_mesh()->get_num_elements(); i++)
        {
            Element *element = Util::scene()->sceneSolution()->sln()->get_mesh()->get_element(i);
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
    if (Util::scene()->sceneSolution()->isSolved())
    {
        Util::scene()->sceneSolution()->ordView().lock_data();
        
        double3* vert = Util::scene()->sceneSolution()->ordView().get_vertices();
        int3* tris = Util::scene()->sceneSolution()->ordView().get_triangles();
        
        // draw mesh
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        int min = 11;
        int max = 1;
        for (int i = 0; i < Util::scene()->sceneSolution()->ordView().get_num_triangles(); i++)
        {
            if (vert[tris[i][0]][2] < min) min = vert[tris[i][0]][2];
            if (vert[tris[i][0]][2] > max) max = vert[tris[i][0]][2];
        }
        int num_boxes = max - min + 1;
        
        // triangles
        const float* color;
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < Util::scene()->sceneSolution()->ordView().get_num_triangles(); i++)
        {
            int color = vert[tris[i][0]][2];
            glColor3d(palette_order[color][0], palette_order[color][1], palette_order[color][2]);
            
            glVertex2d(vert[tris[i][0]][0], vert[tris[i][0]][1]);
            glVertex2d(vert[tris[i][1]][0], vert[tris[i][1]][1]);
            glVertex2d(vert[tris[i][2]][0], vert[tris[i][2]][1]);
        }
        glEnd();
        Util::scene()->sceneSolution()->ordView().unlock_data();
        
        // boxes
        glPushMatrix();
        glLoadIdentity();
        
        double k = 700.0/(double) height()/m_aspect;
        
        double bottom = -0.98;
        double left = -0.98;
        double box_width = 0.08*k;
        double box_height = 0.09;
        double box_height_space = 0.02;
        double border = 0.02*k;
        
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
        glDisable(GL_BLEND);
        
        // labels
        for (int i = 0; i < max; i++)
        {
            glColor3f(0.0, 0.0, 0.0);
            double w = 2.0*fontMetrics().width(QString::number(i+1))/height()*k;
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
    
    double k = 700.0/(double) height()/m_aspect;
    
    double labels_width = 0.18*k;
    double scale_width = 0.04*k;
    double scale_height = 0.93;
    double scale_numticks = 9;
    double border = 0.007*k;
    double border_scale = 0.05*k;
    
    glDisable(GL_DEPTH_TEST);
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
    glDisable(GL_BLEND);

    // palette
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
    glDisable(GL_TEXTURE_1D);
    
    // ticks
    glColor3f(0.0, 0.0, 0.0);

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
        
        renderText(1.0 - labels_width + 0.022*(1.0/m_aspect), y_tick-h/4.0, 0, QString::number(value, '+e', 1));
    }
    
    if ((m_sceneMode == SCENEMODE_POSTPROCESSOR) &&
        (m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D || m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID))
        glEnable(GL_DEPTH_TEST);

    glPopMatrix();
}

void SceneView::paintScalarField()
{
    // range
    double irange = 1.0 / (m_sceneViewSettings.scalarRangeMax - m_sceneViewSettings.scalarRangeMin);
    // special case: constant solution
    if (fabs(m_sceneViewSettings.scalarRangeMin - m_sceneViewSettings.scalarRangeMax) < 1e-8) { irange = 1.0; m_sceneViewSettings.scalarRangeMin -= 0.5; }

    Util::scene()->sceneSolution()->linScalarView().lock_data();

    double3* linVert = Util::scene()->sceneSolution()->linScalarView().get_vertices();
    int3* linTris = Util::scene()->sceneSolution()->linScalarView().get_triangles();
    Point point[3];
    double value[3];

    double max = qMax(Util::scene()->boundingBox().width(), Util::scene()->boundingBox().height());

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < Util::scene()->sceneSolution()->linScalarView().get_num_triangles(); i++)
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
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_LIGHTING);

    Util::scene()->sceneSolution()->linScalarView().unlock_data();

    paintColorBar(m_sceneViewSettings.scalarRangeMin, m_sceneViewSettings.scalarRangeMax);
}

void SceneView::paintScalarField3D()
{
    // range
    double irange = 1.0 / (m_sceneViewSettings.scalarRangeMax - m_sceneViewSettings.scalarRangeMin);
    // special case: constant solution
    if (fabs(m_sceneViewSettings.scalarRangeMin - m_sceneViewSettings.scalarRangeMax) < 1e-8) { irange = 1.0; m_sceneViewSettings.scalarRangeMin -= 0.5; }

    Util::scene()->sceneSolution()->linScalarView().lock_data();

    double3* linVert = Util::scene()->sceneSolution()->linScalarView().get_vertices();
    int3* linTris = Util::scene()->sceneSolution()->linScalarView().get_triangles();
    Point point[3];
    double value[3];

    double max = qMax(Util::scene()->boundingBox().width(), Util::scene()->boundingBox().height());

    if (m_sceneViewSettings.scalarView3DLighting)
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

    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < Util::scene()->sceneSolution()->linScalarView().get_num_triangles(); i++)
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

        if (m_sceneViewSettings.scalarView3DLighting)
            glNormal3d(m_normals[linTris[i][0]][0], m_normals[linTris[i][0]][1], -m_normals[linTris[i][0]][2]);
        glTexCoord2d((value[0] - m_sceneViewSettings.scalarRangeMin) * irange * m_texScale + m_texShift, 0.0);
        // glVertex3d(point[0].x, point[0].y, - delta - (value[0] - m_sceneViewSettings.scalarRangeMin));
        glVertex3d(point[0].x, point[0].y, - delta - value[0]);

        if (m_sceneViewSettings.scalarView3DLighting)
            glNormal3d(m_normals[linTris[i][1]][0], m_normals[linTris[i][1]][1], -m_normals[linTris[i][1]][2]);
        glTexCoord2d((value[1] - m_sceneViewSettings.scalarRangeMin) * irange * m_texScale + m_texShift, 0.0);
        // glVertex3d(point[1].x, point[1].y, - delta - (value[1] - m_sceneViewSettings.scalarRangeMin));
        glVertex3d(point[1].x, point[1].y, - delta - value[1]);

        if (m_sceneViewSettings.scalarView3DLighting)
            glNormal3d(m_normals[linTris[i][2]][0], m_normals[linTris[i][2]][1], -m_normals[linTris[i][2]][2]);
        glTexCoord2d((value[2] - m_sceneViewSettings.scalarRangeMin) * irange * m_texScale + m_texShift, 0.0);
        // glVertex3d(point[2].x, point[2].y, - delta - (value[2] - m_sceneViewSettings.scalarRangeMin));
        glVertex3d(point[2].x, point[2].y, - delta - value[2]);
    }
    glEnd();
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_LIGHTING);

    glPopMatrix();

    Util::scene()->sceneSolution()->linScalarView().unlock_data();

    paintColorBar(m_sceneViewSettings.scalarRangeMin, m_sceneViewSettings.scalarRangeMax);
}

void SceneView::paintScalarField3DSolid()
{
}

void SceneView::paintContours()
{
    Util::scene()->sceneSolution()->linContourView().lock_data();
    
    double3* tvert = Util::scene()->sceneSolution()->linContourView().get_vertices();
    int3* tris = Util::scene()->sceneSolution()->linContourView().get_triangles();
    
    // transform variable
    double rangeMin =  1e100;
    double rangeMax = -1e100;
    
    double3* vert = new double3[Util::scene()->sceneSolution()->linContourView().get_num_vertices()];
    for (int i = 0; i < Util::scene()->sceneSolution()->linContourView().get_num_vertices(); i++)
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
    glColor3f(m_sceneViewSettings.colorContours.redF(), m_sceneViewSettings.colorContours.greenF(), m_sceneViewSettings.colorContours.blueF());
    glBegin(GL_LINES);
    glLineWidth(1.0);

    for (int i = 0; i < Util::scene()->sceneSolution()->linContourView().get_num_triangles(); i++)
    {
        if (finite(vert[tris[i][0]][2]) && finite(vert[tris[i][1]][2]) && finite(vert[tris[i][2]][2]))
        {
            paintContoursTri(vert, &tris[i], step);
        }
    }
    glEnd();
    
    delete vert;
    
    Util::scene()->sceneSolution()->linContourView().unlock_data();
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
    Util::scene()->sceneSolution()->vecVectorView().lock_data();
    
    RectPoint rect = Util::scene()->boundingBox();
    
    double gs = (rect.width() + rect.height()) / 45.0;
    
    int nx = ceil(rect.width() / gs);
    int ny = ceil(rect.height() / gs);
    
    double irange = 1.0 / (m_sceneViewSettings.vectorRangeMax - m_sceneViewSettings.vectorRangeMin);
    // special case: constant solution
    if (fabs(m_sceneViewSettings.vectorRangeMin - m_sceneViewSettings.vectorRangeMax) < 1e-8) { irange = 1.0; m_sceneViewSettings.vectorRangeMin -= 0.5; }
    
    double4* vecVert = Util::scene()->sceneSolution()->vecVectorView().get_vertices();
    int3* vecTris = Util::scene()->sceneSolution()->vecVectorView().get_triangles();
    
    glColor3f(m_sceneViewSettings.colorVectors.redF(), m_sceneViewSettings.colorVectors.greenF(), m_sceneViewSettings.colorVectors.blueF());
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
            
            int index = Util::scene()->sceneSolution()->findTriangleInVectorizer(Util::scene()->sceneSolution()->vecVectorView(), Point(x, y));
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
    
    Util::scene()->sceneSolution()->vecVectorView().unlock_data();
    
    // paintColorBar(m_sceneViewSettings.vectorRangeMin, m_sceneViewSettings.vectorRangeMax);
}

void SceneView::paintSceneModeLabel()
{
    QString text = "";
    
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);

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
        switch (m_sceneViewSettings.postprocessorShow)
        {
        case SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW:
        case SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D:
        case SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID:
            text = physicFieldVariableString(m_sceneViewSettings.scalarPhysicFieldVariable);
            break;
        case SCENEVIEW_POSTPROCESSOR_SHOW_ORDER:
            text = tr("Order");
            break;
        default:
            text = tr("Postprocessor");
        }
        break;
    }
    
    double w = 2.0*fontMetrics().width(text)/width();
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0, 1.0, 1.0, 0.75);
    glBegin(GL_QUADS);
    glVertex2d(0.0 - w/1.5, 0.94);
    glVertex2d(0.0 + w/1.5, 0.94);
    glVertex2d(0.0 + w/1.5, 1.0);
    glVertex2d(0.0 - w/1.5, 1.0);
    glEnd();
    glDisable(GL_BLEND);
    
    glColor3f(0.0, 0.0, 0.0);
    renderText((width()-fontMetrics().width(text))/2, 14, text);
    
    if ((m_sceneMode == SCENEMODE_POSTPROCESSOR) &&
        (m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D || m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID))
        glEnable(GL_DEPTH_TEST);

    glPopMatrix();
}

void SceneView::paintZoomRegion()
{
    // zoom or select region
    if (!m_regionPos.isNull())
    {
        Point posStart = position(Point(m_regionPos.x(), m_regionPos.y()));
        Point posEnd = position(Point(m_lastPos.x(), m_lastPos.y()));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor4d(m_sceneViewSettings.colorHighlighted.redF(), m_sceneViewSettings.colorHighlighted.greenF(), m_sceneViewSettings.colorHighlighted.blueF(), 0.75);

        glBegin(GL_QUADS);
        glVertex2d(posStart.x, posStart.y);
        glVertex2d(posEnd.x, posStart.y);
        glVertex2d(posEnd.x, posEnd.y);
        glVertex2d(posStart.x, posEnd.y);
        glEnd();
        glDisable(GL_BLEND);
    }
}

void SceneView::paintChartLine()
{    
    glColor3f(m_sceneViewSettings.colorSelected.redF(), m_sceneViewSettings.colorSelected.greenF(), m_sceneViewSettings.colorSelected.blueF());
    glLineWidth(3.0);

    glBegin(GL_LINES);
    glVertex2d(m_chartLine.start.x, m_chartLine.start.y);
    glVertex2d(m_chartLine.end.x, m_chartLine.end.y);
    glEnd();
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
    RectPoint rect = Util::scene()->boundingBox();
    
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
            Util::scene()->deleteSelected();
        }
        break;
    case Qt::Key_F11:
        {
            doFullScreen();
        }
        break;
    case Qt::Key_Escape:
        {
            Util::scene()->selectNone();
            emit mousePressed(volumeIntegralValueFactory());
            emit mousePressed(surfaceIntegralValueFactory());
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
                Util::scene()->selectAll(SCENEMODE_OPERATE_ON_LABELS);
                emit mousePressed(volumeIntegralValueFactory());
            }
            
            // select surface integral area
            if (actPostprocessorModeSurfaceIntegral->isChecked())
            {
                Util::scene()->selectAll(SCENEMODE_OPERATE_ON_EDGES);
                emit mousePressed(surfaceIntegralValueFactory());
            }
        }
        else
        {
            Util::scene()->selectAll(m_sceneMode);
        }
        
        doRefresh();
    }
    
    // add node with coordinates under mouse pointer
    if ((event->modifiers() & Qt::AltModifier & Qt::ControlModifier) | (event->key() == Qt::Key_N))
    {
        Point p = position(Point(m_lastPos.x(), m_lastPos.y()));
        Util::scene()->doNewNode(p);
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
        
        if ((m_sceneMode == SCENEMODE_POSTPROCESSOR) &&
            !(m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D || m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID))
        {
            // local point value
            if (actPostprocessorModeLocalPointValue->isChecked())
                emit mousePressed(localPointValueFactory(p));
            // select volume integral area
            if (actPostprocessorModeVolumeIntegral->isChecked())
            {
                int index = Util::scene()->sceneSolution()->findTriangleInMesh(Util::scene()->sceneSolution()->mesh(), p);
                if (index > 0)
                {
                    //  find label marker
                    int labelIndex = Util::scene()->sceneSolution()->mesh().get_element_fast(index)->marker;
                    
                    Util::scene()->labels[labelIndex]->isSelected = !Util::scene()->labels[labelIndex]->isSelected;
                    updateGL();
                }
                emit mousePressed(volumeIntegralValueFactory());
            }
            // select surface integral area
            if (actPostprocessorModeSurfaceIntegral->isChecked())
            {
                //  find edge marker
                SceneEdge *edge = findClosestEdge(p);
                
                edge->isSelected = !edge->isSelected;
                updateGL();
                
                emit mousePressed(surfaceIntegralValueFactory());
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
            SceneNode *nodeAdded = Util::scene()->addNode(node);
            if (nodeAdded == node) Util::scene()->undoStack()->push(new SceneNodeCommandAdd(node->point));
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
                    {
                        SceneEdge *edge = new SceneEdge(m_nodeLast, node, Util::scene()->edgeMarkers[0], 0);
                        SceneEdge *edgeAdded = Util::scene()->addEdge(edge);
                        if (edgeAdded == edge) Util::scene()->undoStack()->push(new SceneEdgeCommandAdd(edge->nodeStart->point, edge->nodeEnd->point, edge->marker->name, edge->angle));
                    }
                    
                    m_nodeLast->isSelected = false;
                    m_nodeLast = NULL;
                }
                
                updateGL();
            }
        }
        // add label directly by mouse click
        if (m_sceneMode == SCENEMODE_OPERATE_ON_LABELS)
        {
            SceneLabel *label = new SceneLabel(p, Util::scene()->labelMarkers[0], 0);
            SceneLabel *labelAdded = Util::scene()->addLabel(label);
            if (labelAdded == label) Util::scene()->undoStack()->push(new SceneLabelCommandAdd(label->point, label->marker->name, label->area));
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
            Util::scene()->selectNone();
            if (m_sceneMode == SCENEMODE_OPERATE_ON_NODES)
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
            if (m_sceneMode == SCENEMODE_OPERATE_ON_EDGES)
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
            if (m_sceneMode == SCENEMODE_OPERATE_ON_LABELS)
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
            Util::scene()->selectNone();
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

        if ((m_sceneMode == SCENEMODE_POSTPROCESSOR) &&
            (m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D || m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID))
        {
            m_offset.x += 2.0/width() *(- dx * cos(m_rotation.y/180.0*M_PI) + dy * sin(m_rotation.y/180.0*M_PI))/m_scale*m_aspect;
            m_offset.y += 2.0/height()*(  dy * cos(m_rotation.y/180.0*M_PI) + dx * sin(m_rotation.y/180.0*M_PI))/m_scale;
            m_offset.z -= 2.0/height()*(  dy * sin(m_rotation.x/180.0*M_PI))/m_scale;
        }
        else
        {
            m_offset.x -= 2.0/width() * dx / m_scale*m_aspect;
            m_offset.y += 2.0/height() * dy / m_scale;
            m_offset.z = 0.0;
        }

        updateGL();
    }
    
    // rotate
    if ((m_sceneMode == SCENEMODE_POSTPROCESSOR) &&
        (m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D || m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID))
    {
        if (event->buttons() & Qt::LeftButton)
        {
            setCursor(Qt::PointingHandCursor);

            m_rotation.x -= dy;
            m_rotation.y += dx;

            updateGL();
        }
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
                Util::scene()->highlightNone();
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
                Util::scene()->highlightNone();
                edge->isHighlighted = true;
                setToolTip(QString("<h4>Edge</h4>Point: [%1; %2] - [%3; %4]<br/>Boundary Condition: %5<br/>Angle: %6 deg.").arg(edge->nodeStart->point.x, 0, 'f', 3).arg(edge->nodeStart->point.y, 0, 'f', 3)
                           .arg(edge->nodeEnd->point.x, 0, 'f', 3).arg(edge->nodeEnd->point.y, 0, 'f', 3)
                           .arg(edge->marker->name).arg(edge->angle, 0, 'f', 0));

                updateGL();
            }
        }
        if (m_sceneMode == SCENEMODE_OPERATE_ON_LABELS)
        {
            // highlight the closest label
            SceneLabel *label = findClosestLabel(p);
            if (label)
            {
                Util::scene()->highlightNone();
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
                Util::scene()->highlightNone();
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
        mnuMarkerGroup->setEnabled((Util::scene()->edgeMarkers.count() > 0));
        for (int i = 1; i<Util::scene()->edgeMarkers.count(); i++)
        {
            QAction *actMenuEdgeMarkerItem = new QAction(Util::scene()->edgeMarkers[i]->name, this);
            actMenuEdgeMarkerItem->setData(Util::scene()->edgeMarkers[i]->variant());
            actBoundaryGroup->addAction(actMenuEdgeMarkerItem);
            mnuMarkerGroup->addAction(actMenuEdgeMarkerItem);
        }
    }
    
    // set material context menu
    if (m_sceneMode == SCENEMODE_OPERATE_ON_LABELS)
    {
        mnuMarkerGroup->setEnabled((Util::scene()->labelMarkers.count() > 0));
        for (int i = 1; i<Util::scene()->labelMarkers.count(); i++)
        {
            QAction *actMenuLabelMarkerItem = new QAction(Util::scene()->labelMarkers[i]->name, this);
            actMenuLabelMarkerItem->setData(Util::scene()->labelMarkers[i]->variant());
            actMaterialGroup->addAction(actMenuLabelMarkerItem);
            mnuMarkerGroup->addAction(actMenuLabelMarkerItem);
        }
    }

    mnuInfo->exec(event->globalPos());
}

void SceneView::closeEvent(QCloseEvent *event)
{
    event->ignore();
}

// slots *****************************************************************************************************************************

void SceneView::doInvalidated()
{
    if (m_normals == NULL)
    {
        delete m_normals;
        m_normals = NULL;
    }

    if (Util::scene()->sceneSolution()->isSolved())
    {
        setRangeContour();
        setRangeScalar();
        setRangeVector();

        paletteFilter();
        paletteUpdateTexAdjust();
        paletteCreate();
    }
    else
    {
        if (m_sceneMode == SCENEMODE_POSTPROCESSOR)
            actSceneModeNode->trigger();
    }
    
    actSceneModePostprocessor->setEnabled(Util::scene()->sceneSolution()->isSolved());
    
    updateGL();
}

void SceneView::doZoomBestFit()
{
    RectPoint rect = Util::scene()->boundingBox();
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
    
    double maxScene = (((double) width() / (double) height()) < (sceneWidth / sceneHeight)) ? sceneWidth/m_aspect : sceneHeight;
    
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

void SceneView::doDefaults()
{   
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
    m_sceneViewSettings.showGeometry = true;
    m_sceneViewSettings.showGrid = true;
    m_sceneViewSettings.showInitialMesh = false;

    m_sceneViewSettings.postprocessorShow = SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW;

    m_sceneViewSettings.showContours = false;
    m_sceneViewSettings.showVectors = false;
    m_sceneViewSettings.showSolutionMesh = false;

    doInvalidated();
    actSceneModePostprocessor->trigger();
}

void SceneView::doRefresh()
{
    paintGL();
    updateGL();
}

void SceneView::doMaterialGroup(QAction *action)
{
    if (SceneLabelMarker *labelMarker = action->data().value<SceneLabelMarker *>())
        Util::scene()->setLabelMarker(labelMarker);
}

void SceneView::doBoundaryGroup(QAction *action)
{
    if (SceneEdgeMarker *edgeMarker = action->data().value<SceneEdgeMarker *>())
        Util::scene()->setEdgeMarker(edgeMarker);
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
    Util::scene()->selectNone();
    updateGL();
}

void SceneView::doSceneViewProperties()
{
    SceneViewPostprocessorShow postprocessorShow = m_sceneViewSettings.postprocessorShow;

    SceneViewDialog *sceneViewDialog = new SceneViewDialog(this, this);
    if (sceneViewDialog->showDialog() == QDialog::Accepted)
    {
        doInvalidated();

        // set defaults
        if (postprocessorShow != m_sceneViewSettings.postprocessorShow)
        {
            if (m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D || m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID)
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
    delete sceneViewDialog;
}

void SceneView::doFullScreen()
{
    QSettings settings;
    if (isFullScreen())
    {
        setParent(m_mainWindow);
        m_mainWindow->setCentralWidget(this);
        showNormal();

        m_mainWindow->restoreState(settings.value("MainWindow/State", m_mainWindow->saveState()).toByteArray());
    }
    else
    {
        settings.setValue("MainWindow/State", m_mainWindow->saveState());

        setParent(NULL);
        showFullScreen();
    }
}

void SceneView::doSceneModeSet(QAction *)
{
    actSceneModePostprocessor->setEnabled(Util::scene()->sceneSolution()->isSolved());
    
    if (actSceneModeNode->isChecked()) m_sceneMode = SCENEMODE_OPERATE_ON_NODES;
    if (actSceneModeEdge->isChecked()) m_sceneMode = SCENEMODE_OPERATE_ON_EDGES;
    if (actSceneModeLabel->isChecked()) m_sceneMode = SCENEMODE_OPERATE_ON_LABELS;
    if (actSceneModePostprocessor->isChecked()) m_sceneMode = SCENEMODE_POSTPROCESSOR;
    
    Util::scene()->highlightNone();
    Util::scene()->selectNone();
    
    Util::scene()->actTransform->setEnabled((m_sceneMode != SCENEMODE_POSTPROCESSOR));
    Util::scene()->actDeleteSelected->setEnabled((m_sceneMode != SCENEMODE_POSTPROCESSOR));
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
        ViewScalarFilter *viewScalarFilter;
        if (numberOfSolution(Util::scene()->problemInfo().physicField) == 1)
            viewScalarFilter = new ViewScalarFilter(Util::scene()->sceneSolution()->sln(),
                                                    Util::scene(),
                                                    m_sceneViewSettings.contourPhysicFieldVariable,
                                                    PHYSICFIELDVARIABLECOMP_SCALAR);

        if (numberOfSolution(Util::scene()->problemInfo().physicField) == 2)
            viewScalarFilter = new ViewScalarFilter(Util::scene()->sceneSolution()->sln1(),
                                                    Util::scene()->sceneSolution()->sln2(),
                                                    Util::scene(),
                                                    m_sceneViewSettings.contourPhysicFieldVariable,
                                                    PHYSICFIELDVARIABLECOMP_SCALAR);

        Util::scene()->sceneSolution()->setSlnContourView(viewScalarFilter);
    }
}

void SceneView::setRangeScalar()
{
    if ((m_sceneMode == SCENEMODE_POSTPROCESSOR) &&
        (m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW ||
         m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D || 
         m_sceneViewSettings.postprocessorShow == SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID))
    {
        ViewScalarFilter *viewScalarFilter;
        if (numberOfSolution(Util::scene()->problemInfo().physicField) == 1)
            viewScalarFilter = new ViewScalarFilter(Util::scene()->sceneSolution()->sln(),
                                                    Util::scene(),
                                                    m_sceneViewSettings.scalarPhysicFieldVariable,
                                                    m_sceneViewSettings.scalarPhysicFieldVariableComp);

        if (numberOfSolution(Util::scene()->problemInfo().physicField) == 2)
            viewScalarFilter = new ViewScalarFilter(Util::scene()->sceneSolution()->sln1(),
                                                    Util::scene()->sceneSolution()->sln2(),
                                                    Util::scene(),
                                                    m_sceneViewSettings.scalarPhysicFieldVariable,
                                                    m_sceneViewSettings.scalarPhysicFieldVariableComp);

        Util::scene()->sceneSolution()->setSlnScalarView(viewScalarFilter);

        if (m_sceneViewSettings.scalarRangeAuto)
        {
            m_sceneViewSettings.scalarRangeMin = Util::scene()->sceneSolution()->linScalarView().get_min_value();
            m_sceneViewSettings.scalarRangeMax = Util::scene()->sceneSolution()->linScalarView().get_max_value();
        }

        if (m_sceneViewSettings.scalarView3DLighting)
        {
            double max = qMax(Util::scene()->boundingBox().width(), Util::scene()->boundingBox().height());

            // lighting
            float light_specular[] = {1.0, 1.0, 1.0, 1.0};
            float light_ambient[]  = {0.3, 0.3, 0.3, 1.0};
            float light_diffuse[]  = {1.0, 1.0, 1.0, 1.0};   
            float light_position[] = { -5.0*max, 8.0*max, 5.0*max, 0.0 };

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

            glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
#if defined(GL_LIGHT_MODEL_COLOR_CONTROL) && defined(GL_SEPARATE_SPECULAR_COLOR)
            glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
#endif

            // calculate normals
            Util::scene()->sceneSolution()->linScalarView().lock_data();

            int nv = Util::scene()->sceneSolution()->linScalarView().get_num_vertices();
            int nt = Util::scene()->sceneSolution()->linScalarView().get_num_triangles();
            double3* vert = Util::scene()->sceneSolution()->linScalarView().get_vertices();
            int3* tris = Util::scene()->sceneSolution()->linScalarView().get_triangles();

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

            Util::scene()->sceneSolution()->linScalarView().unlock_data();
        }
    }
}

void SceneView::setRangeVector()
{
    if (m_sceneMode == SCENEMODE_POSTPROCESSOR && m_sceneViewSettings.showVectors)
    {
        ViewScalarFilter *viewVectorXFilter = new ViewScalarFilter(Util::scene()->sceneSolution()->sln(),
                                                                   Util::scene(),
                                                                   m_sceneViewSettings.vectorPhysicFieldVariable,
                                                                   PHYSICFIELDVARIABLECOMP_X);
        ViewScalarFilter *viewVectorYFilter = new ViewScalarFilter(Util::scene()->sceneSolution()->sln(),
                                                                   Util::scene(),
                                                                   m_sceneViewSettings.vectorPhysicFieldVariable,
                                                                   PHYSICFIELDVARIABLECOMP_Y);
        Util::scene()->sceneSolution()->setSlnVectorView(viewVectorXFilter, viewVectorYFilter);
        
        if (m_sceneViewSettings.vectorRangeAuto)
        {
            m_sceneViewSettings.vectorRangeMin = Util::scene()->sceneSolution()->vecVectorView().get_min_value();
            m_sceneViewSettings.vectorRangeMax = Util::scene()->sceneSolution()->vecVectorView().get_max_value();
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
    Util::scene()->selectNone();
    
    switch (m_sceneMode)
    {
    case SCENEMODE_OPERATE_ON_NODES:
        foreach (SceneNode *node, Util::scene()->nodes)
            if (node->point.x >= start.x && node->point.x <= end.x && node->point.y >= start.y && node->point.y <= end.y)
                node->isSelected = true;
        break;
    case SCENEMODE_OPERATE_ON_EDGES:
        foreach (SceneEdge *edge, Util::scene()->edges)
            if (edge->nodeStart->point.x >= start.x && edge->nodeStart->point.x <= end.x && edge->nodeStart->point.y >= start.y && edge->nodeStart->point.y <= end.y &&
                edge->nodeEnd->point.x >= start.x && edge->nodeEnd->point.x <= end.x && edge->nodeEnd->point.y >= start.y && edge->nodeEnd->point.y <= end.y)
                edge->isSelected = true;
        break;
    case SCENEMODE_OPERATE_ON_LABELS:
        foreach (SceneLabel *label, Util::scene()->labels)
            if (label->point.x >= start.x && label->point.x <= end.x && label->point.y >= start.y && label->point.y <= end.y)
                label->isSelected = true;
        break;
    }
}

SceneNode *SceneView::findClosestNode(const Point &point)
{
    SceneNode *nodeClosest = NULL;
    
    double distance = 1e100;
    foreach (SceneNode *node, Util::scene()->nodes)
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
    foreach (SceneEdge *edge, Util::scene()->edges)
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
    foreach (SceneLabel *label, Util::scene()->labels)
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor4d(m_sceneViewSettings.colorSelected.redF(), m_sceneViewSettings.colorSelected.greenF(), m_sceneViewSettings.colorSelected.blueF(), 0.5);
    
    // triangles
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < Util::scene()->sceneSolution()->mesh().get_num_elements(); i++)
    {
        Element *element = Util::scene()->sceneSolution()->mesh().get_element(i);
        if (Util::scene()->labels[element->marker]->isSelected)
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
    foreach (SceneEdge *edge, Util::scene()->edges) {
        glColor3d(m_sceneViewSettings.colorSelected.redF(), m_sceneViewSettings.colorSelected.greenF(), m_sceneViewSettings.colorSelected.blueF());
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
    // glReadBuffer(GL_FRONT);

    // copy image
    QImage *image = new QImage(grabFrameBuffer(true));
    image->save(fileName, "PNG");

    delete image;
}
