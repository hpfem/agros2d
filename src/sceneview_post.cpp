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

#include "sceneview_post.h"

#include "util/global.h"

#include "sceneview_common2d.h"
#include "sceneview_data.h"
#include "scene.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "particle/particle_tracing.h"

#include "logview.h"

#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

PostHermes::PostHermes()
{
    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(clear()));

    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(refresh()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(refresh()));
}

PostHermes::~PostHermes()
{
    clear();
}

void PostHermes::processInitialMesh()
{
    if (Agros2D::problem()->isMeshed() && Agros2D::problem()->configView()->showInitialMeshView)
    {
        Agros2D::log()->printMessage(tr("MeshView"), tr("initial mesh with %1 elements").arg(Agros2D::scene()->activeViewField()->initialMesh()->get_num_active_elements()));

        // init linearizer for initial mesh
        try
        {
            Hermes::Hermes2D::ZeroSolution<double> initial(Agros2D::scene()->activeViewField()->initialMesh());

            m_linInitialMeshView.free();
            m_linInitialMeshView.process_solution(&initial);
        }
        catch (Hermes::Exceptions::Exception& e)
        {
            Agros2D::log()->printError("MeshView", QObject::tr("Linearizer processing failed: %1").arg(e.what()));
        }
    }
}

void PostHermes::processSolutionMesh()
{
    if (Agros2D::problem()->isSolved() && Agros2D::problem()->configView()->showSolutionMeshView)
    {
        // ERROR: FIX component(0)
        Agros2D::log()->printMessage(tr("MeshView"), tr("solution mesh with %1 elements").arg(Agros2D::scene()->activeMultiSolutionArray().solutions().at(0)->get_mesh()->get_num_active_elements()));

        // init linearizer for solution mesh
        // ERROR: FIX component(0)
        const Hermes::Hermes2D::Mesh *mesh = Agros2D::scene()->activeMultiSolutionArray().solutions().at(0)->get_mesh();
        Hermes::Hermes2D::ZeroSolution<double> solution(mesh);

        m_linSolutionMeshView.free();
        m_linSolutionMeshView.process_solution(&solution);
    }
}

void PostHermes::processOrder()
{
    // init linearizer for order view
    if (Agros2D::problem()->isSolved() && Agros2D::problem()->configView()->showOrderView)
    {
        Agros2D::log()->printMessage(tr("MeshView"), tr("polynomial order"));

        // ERROR: FIX component(0)
        m_orderView.free();
        m_orderView.process_space(Agros2D::scene()->activeMultiSolutionArray().spaces().at(0));
    }
}

void PostHermes::processRangeContour()
{
    if (Agros2D::problem()->isSolved() && Agros2D::problem()->configView()->showContourView)
    {
        bool contains = false;
        foreach (Module::LocalVariable variable, Agros2D::scene()->activeViewField()->viewScalarVariables())
        {
            if (variable.id() == Agros2D::problem()->configView()->contourVariable)
            {
                contains = true;
                break;
            }
        }

        if (Agros2D::problem()->configView()->contourVariable == "" || !contains)
        {
            // default values
            Agros2D::problem()->configView()->contourVariable = Agros2D::scene()->activeViewField()->defaultViewScalarVariable().id();
        }

        Agros2D::log()->printMessage(tr("PostView"), tr("contour view (%1)").arg(Agros2D::problem()->configView()->contourVariable));

        QString variableName = Agros2D::problem()->configView()->contourVariable;
        Module::LocalVariable variable = Agros2D::scene()->activeViewField()->localVariable(variableName);

        Hermes::Hermes2D::Filter<double> *slnContourView = NULL;
        if (variable.isScalar())
            slnContourView = viewScalarFilter(Agros2D::scene()->activeViewField()->localVariable(Agros2D::problem()->configView()->contourVariable),
                                              PhysicFieldVariableComp_Scalar);
        else
            slnContourView = viewScalarFilter(Agros2D::scene()->activeViewField()->localVariable(Agros2D::problem()->configView()->contourVariable),
                                              PhysicFieldVariableComp_Magnitude);

        m_linContourView.free();
        m_linContourView.process_solution(slnContourView,
                                          Hermes::Hermes2D::H2D_FN_VAL_0,
                                          paletteQualityToDouble(Agros2D::problem()->configView()->linearizerQuality));

        // deformed shape
        if (Agros2D::problem()->configView()->deformContour)
            Agros2D::scene()->activeViewField()->deformShape(m_linContourView.get_vertices(), m_linContourView.get_num_vertices());

        delete slnContourView;
    }
}

void PostHermes::processRangeScalar()
{
    if (Agros2D::problem()->isSolved() && ((Agros2D::problem()->configView()->showScalarView) || (Agros2D::problem()->configView()->showPost3D == SceneViewPost3DMode_ScalarView3D)))
    {
        bool contains = false;
        foreach (Module::LocalVariable variable, Agros2D::scene()->activeViewField()->viewScalarVariables())
        {
            if (variable.id() == Agros2D::problem()->configView()->scalarVariable)
            {
                contains = true;
                break;
            }
        }

        if (Agros2D::problem()->configView()->scalarVariable == "" || !contains)
        {
            // default values
            Module::LocalVariable variable = Agros2D::scene()->activeViewField()->defaultViewScalarVariable();
            Agros2D::problem()->configView()->scalarVariable = variable.id();
            Agros2D::problem()->configView()->scalarVariableComp = variable.isScalar() ? PhysicFieldVariableComp_Scalar : PhysicFieldVariableComp_Magnitude;
        }

        Agros2D::log()->printMessage(tr("PostView"), tr("scalar view (%1)").arg(Agros2D::problem()->configView()->scalarVariable));

        Hermes::Hermes2D::Filter<double> *slnScalarView = viewScalarFilter(Agros2D::scene()->activeViewField()->localVariable(Agros2D::problem()->configView()->scalarVariable),
                                                                           Agros2D::problem()->configView()->scalarVariableComp);

        // QTime time;
        // time.start();
        // qDebug() << "process scalar: start";
        m_linScalarView.free();
        m_linScalarView.process_solution(slnScalarView,
                                         Hermes::Hermes2D::H2D_FN_VAL_0,
                                         paletteQualityToDouble(Agros2D::problem()->configView()->linearizerQuality));

        // qDebug() << "process scalar: " << time.elapsed();

        // deformed shape
        if (Agros2D::problem()->configView()->deformScalar)
            Agros2D::scene()->activeViewField()->deformShape(m_linScalarView.get_vertices(),
                                                             m_linScalarView.get_num_vertices());

        if (Agros2D::problem()->configView()->scalarRangeAuto)
        {
            Agros2D::problem()->configView()->scalarRangeMin = m_linScalarView.get_min_value();
            Agros2D::problem()->configView()->scalarRangeMax = m_linScalarView.get_max_value();
        }

        delete slnScalarView;
    }
}

void PostHermes::processRangeVector()
{
    if (Agros2D::problem()->isSolved() && Agros2D::problem()->configView()->showVectorView)
    {
        bool contains = false;
        foreach (Module::LocalVariable variable, Agros2D::scene()->activeViewField()->viewVectorVariables())
        {
            if (variable.id() == Agros2D::problem()->configView()->vectorVariable)
            {
                contains = true;
                break;
            }
        }

        if (Agros2D::problem()->configView()->vectorVariable == "" || !contains)
        {
            // default values
            Agros2D::problem()->configView()->vectorVariable = Agros2D::scene()->activeViewField()->defaultViewVectorVariable().id();
        }

        Agros2D::log()->printMessage(tr("PostView"), tr("vector view (%1)").arg(Agros2D::problem()->configView()->vectorVariable));

        Hermes::Hermes2D::Filter<double> *slnVectorXView = viewScalarFilter(Agros2D::scene()->activeViewField()->localVariable(Agros2D::problem()->configView()->vectorVariable),
                                                                            PhysicFieldVariableComp_X);

        Hermes::Hermes2D::Filter<double> *slnVectorYView = viewScalarFilter(Agros2D::scene()->activeViewField()->localVariable(Agros2D::problem()->configView()->vectorVariable),
                                                                            PhysicFieldVariableComp_Y);

        m_vecVectorView.free();
        m_vecVectorView.process_solution(slnVectorXView, slnVectorYView,
                                         Hermes::Hermes2D::H2D_FN_VAL_0, Hermes::Hermes2D::H2D_FN_VAL_0,
                                         Hermes::Hermes2D::Views::HERMES_EPS_LOW);

        // deformed shape
        if (Agros2D::problem()->configView()->deformVector)
            Agros2D::scene()->activeViewField()->deformShape(m_vecVectorView.get_vertices(),
                                                             m_vecVectorView.get_num_vertices());

        delete slnVectorXView;
        delete slnVectorYView;
    }
}

void PostHermes::processParticleTracing()
{
    if (Agros2D::problem()->isSolved() && Agros2D::problem()->configView()->showParticleView)
    {
        Agros2D::log()->printMessage(tr("PostView"), tr("particle view"));

        // clear lists
        foreach (QList<Point3> list, m_particleTracingPositionsList)
            list.clear();
        m_particleTracingPositionsList.clear();

        foreach (QList<Point3> list, m_particleTracingVelocitiesList)
            list.clear();
        m_particleTracingVelocitiesList.clear();

        foreach (QList<double> list, m_particleTracingTimesList)
            list.clear();
        m_particleTracingTimesList.clear();

        m_particleTracingVelocityMin =  numeric_limits<double>::max();
        m_particleTracingVelocityMax = -numeric_limits<double>::max();

        for (int k = 0; k < Agros2D::problem()->configView()->particleNumberOfParticles; k++)
        {
            // position and velocity cache
            ParticleTracing particleTracing;
            try
            {
                particleTracing.computeTrajectoryParticle(k > 0);
            }
            catch (...)
            {
                m_particleTracingVelocityMin = 0.0;
                m_particleTracingVelocityMax = 0.0;

                return;
            }

            m_particleTracingPositionsList.append(particleTracing.positions());
            m_particleTracingVelocitiesList.append(particleTracing.velocities());
            m_particleTracingTimesList.append(particleTracing.times());

            // velocity min and max value
            if (particleTracing.velocityMin() < m_particleTracingVelocityMin) m_particleTracingVelocityMin = particleTracing.velocityMin();
            if (particleTracing.velocityMax() > m_particleTracingVelocityMax) m_particleTracingVelocityMax = particleTracing.velocityMax();

            Agros2D::log()->printMessage(tr("Particle Tracing"), tr("Particle %1: %2 steps, final time %3 s").
                                         arg(k + 1).
                                         arg(particleTracing.times().count()).
                                         arg(particleTracing.times().last()));
        }
    }
}

void PostHermes::refresh()
{
    clear();

    if (Agros2D::problem()->isMeshed())
        processMeshed();

    if (Agros2D::problem()->isSolved())
        processSolved();

    emit processed();
}

void PostHermes::clear()
{
    m_linInitialMeshView.free();
    m_linSolutionMeshView.free();
    m_orderView.free();

    m_linContourView.free();
    m_linScalarView.free();
    m_vecVectorView.free();
}

void PostHermes::processMeshed()
{
    processInitialMesh();
}

void PostHermes::processSolved()
{
    // temporary use 3/4 of max threads
    int threads = omp_get_max_threads() * 3/4;
    Hermes::Hermes2D::Hermes2DApi.set_integral_param_value(Hermes::Hermes2D::numThreads, threads);

    processSolutionMesh();
    processOrder();

    processRangeContour();
    processRangeScalar();
    processRangeVector();
    processParticleTracing();

    // restore settings
    Hermes::Hermes2D::Hermes2DApi.set_integral_param_value(Hermes::Hermes2D::numThreads, Agros2D::configComputer()->numberOfThreads);
}

Hermes::Hermes2D::Filter<double> *PostHermes::viewScalarFilter(Module::LocalVariable physicFieldVariable,
                                                               PhysicFieldVariableComp physicFieldVariableComp)
{
    // update time functions
    /*
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
        m_fieldInfo->module()->update_time_functions(Agros2D::problem()->time());
    */

    FieldInfo* activeViewField = Agros2D::scene()->activeViewField();
    assert(activeViewField);

    Hermes::vector<Hermes::Hermes2D::MeshFunction<double> *> sln;
    for (int k = 0; k < activeViewField->numberOfSolutions(); k++)
        sln.push_back(Agros2D::scene()->activeMultiSolutionArray().solutions().at(k));

    return activeViewField->plugin()->filter(activeViewField,
                                             sln,
                                             physicFieldVariable.id(),
                                             physicFieldVariableComp);
}

// ************************************************************************************************

SceneViewPostInterface::SceneViewPostInterface(PostHermes *postHermes, QWidget *parent)
    : SceneViewCommon(parent),
      m_postHermes(postHermes),
      m_textureScalar(-1)
{
}

void SceneViewPostInterface::initializeGL()
{
    clearGLLists();

    SceneViewCommon::initializeGL();
}

const QVector3D SceneViewPostInterface::paletteColor2(const int pos) const
{
    int n = (int) (pos / (PALETTEENTRIES / Agros2D::problem()->configView()->paletteSteps)) * (PALETTEENTRIES / Agros2D::problem()->configView()->paletteSteps);

    if (n < 0)
        n = 0;
    else if (n > PALETTEENTRIES - 1)
        n = PALETTEENTRIES - 1;

    const double *colors = NULL;
    switch (Agros2D::problem()->configView()->paletteType)
    {
    case Palette_Agros2D:
        colors = paletteDataAgros2D[n];
    }

    assert(colors);
    return QVector3D(colors[0], colors[1], colors[2]);
}

const double* SceneViewPostInterface::paletteColor(double x) const
{
    switch (Agros2D::problem()->configView()->paletteType)
    {
    case Palette_Agros2D:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataAgros2D[n];
    }
        break;
    case Palette_Jet:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataJet[n];
    }
        break;
    case Palette_Copper:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataCopper[n];
    }
        break;
    case Palette_Hot:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataHot[n];
    }
        break;
    case Palette_Cool:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataCool[n];
    }
        break;
    case Palette_Bone:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataBone[n];
    }
        break;
    case Palette_Pink:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataPink[n];
    }
        break;
    case Palette_Spring:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataSpring[n];
    }
        break;
    case Palette_Summer:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataSummer[n];
    }
        break;
    case Palette_Autumn:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataAutumn[n];
    }
        break;
    case Palette_Winter:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataWinter[n];
    }
        break;
    case Palette_HSV:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataHSV[n];
    }
        break;
    case Palette_BWAsc:
    {
        static double color[3];
        color[0] = color[1] = color[2] = x;
        return color;
    }
        break;
    case Palette_BWDesc:
    {
        static double color[3];
        color[0] = color[1] = color[2] = 1.0 - x;
        return color;
    }
        break;
    default:
        qWarning() << QString("Undefined: %1.").arg(Agros2D::problem()->configView()->paletteType);
        return NULL;
    }
}

const double* SceneViewPostInterface::paletteColorOrder(int n) const
{
    switch (Agros2D::problem()->configView()->orderPaletteOrderType)
    {
    case PaletteOrder_Hermes:
        return paletteOrderHermes[n];
    case PaletteOrder_Jet:
        return paletteOrderJet[n];
    case PaletteOrder_Copper:
        return paletteOrderCopper[n];
    case PaletteOrder_Hot:
        return paletteOrderHot[n];
    case PaletteOrder_Cool:
        return paletteOrderCool[n];
    case PaletteOrder_Bone:
        return paletteOrderBone[n];
    case PaletteOrder_Pink:
        return paletteOrderPink[n];
    case PaletteOrder_Spring:
        return paletteOrderSpring[n];
    case PaletteOrder_Summer:
        return paletteOrderSummer[n];
    case PaletteOrder_Autumn:
        return paletteOrderAutumn[n];
    case PaletteOrder_Winter:
        return paletteOrderWinter[n];
    case PaletteOrder_HSV:
        return paletteOrderHSV[n];
    case PaletteOrder_BWAsc:
        return paletteOrderBWAsc[n];
    case PaletteOrder_BWDesc:
        return paletteOrderBWDesc[n];
    default:
        qWarning() << QString("Undefined: %1.").arg(Agros2D::problem()->configView()->orderPaletteOrderType);
        return NULL;
    }
}

void SceneViewPostInterface::paletteCreate()
{
    int paletteSteps = Agros2D::problem()->configView()->paletteFilter ? 100 : Agros2D::problem()->configView()->paletteSteps;

    unsigned char palette[256][3];
    for (int i = 0; i < paletteSteps; i++)
    {
        const double* color = paletteColor((double) i / paletteSteps);
        palette[i][0] = (unsigned char) (color[0] * 255);
        palette[i][1] = (unsigned char) (color[1] * 255);
        palette[i][2] = (unsigned char) (color[2] * 255);
    }
    for (int i = paletteSteps; i < 256; i++)
        memcpy(palette[i], palette[paletteSteps-1], 3);

    if (m_textureScalar == -1)
        glDeleteTextures(1, &m_textureScalar);
    glGenTextures(1, &m_textureScalar);

    int palFilter = Agros2D::problem()->configView()->paletteFilter ? GL_LINEAR : GL_NEAREST;
    glBindTexture(GL_TEXTURE_1D, m_textureScalar);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, palFilter);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, palFilter);
    glTexImage1D(GL_TEXTURE_1D, 0, 3, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);

    // adjust palette
    if (Agros2D::problem()->configView()->paletteFilter)
    {
        m_texScale = (double) (paletteSteps-1) / 256.0;
        m_texShift = 0.5 / 256.0;
    }
    else
    {
        m_texScale = (double) paletteSteps / 256.0;
        m_texShift = 0.0;
    }
}

void SceneViewPostInterface::paintScalarFieldColorBar(double min, double max)
{
    if (!Agros2D::problem()->isSolved() || !Agros2D::problem()->configView()->showScalarColorBar) return;

    loadProjectionViewPort();

    glScaled(2.0 / width(), 2.0 / height(), 1.0);
    glTranslated(-width() / 2.0, -height() / 2.0, 0.0);

    // dimensions
    int textWidth = m_fontPost->glyphs[GLYPH_M].width * (QString::number(-1.0, '+e', Agros2D::problem()->configView()->scalarDecimalPlace).length() + 1);
    int textHeight = m_fontPost->height;
    Point scaleSize = Point(45.0 + textWidth, 20*textHeight); // height() - 20.0
    Point scaleBorder = Point(10.0, (Agros2D::problem()->configView()->showRulers) ? 1.8 * textHeight : 10.0);
    double scaleLeft = (width() - (45.0 + textWidth));
    int numTicks = 11;

    // blended rectangle
    drawBlend(Point(scaleLeft, scaleBorder.y), Point(scaleLeft + scaleSize.x - scaleBorder.x, scaleBorder.y + scaleSize.y),
              0.91, 0.91, 0.91);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // palette border
    glColor3d(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + 10.0);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + 10.0);
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    // palette
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, m_textureScalar);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glBegin(GL_QUADS);
    if (fabs(min - max) > EPS_ZERO)
        glTexCoord1d(m_texScale + m_texShift);
    else
        glTexCoord1d(m_texShift);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + scaleSize.y - 52.0);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + scaleSize.y - 52.0);
    glTexCoord1d(m_texShift);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + 12.0);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + 12.0);
    glEnd();

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glDisable(GL_TEXTURE_1D);

    // ticks
    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (int i = 1; i < numTicks; i++)
    {
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 15.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 25.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
    }
    glEnd();

    // line
    glLineWidth(1.0);
    glBegin(GL_LINES);
    glVertex2d(scaleLeft + 5.0, scaleBorder.y + scaleSize.y - 31.0);
    glVertex2d(scaleLeft + scaleSize.x - 15.0, scaleBorder.y + scaleSize.y - 31.0);
    glEnd();

    // labels
    for (int i = 1; i < numTicks+1; i++)
    {
        double value = 0.0;
        if (!Agros2D::problem()->configView()->scalarRangeLog)
            value = min + (double) (i-1) / (numTicks-1) * (max - min);
        else
            value = min + (double) pow(Agros2D::problem()->configView()->scalarRangeBase, ((i-1) / (numTicks-1)))/Agros2D::problem()->configView()->scalarRangeBase * (max - min);

        if (fabs(value) < EPS_ZERO) value = 0.0;
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        printPostAt(scaleLeft + 33.0 + ((value >= 0.0) ? m_fontPost->glyphs[GLYPH_M].width : 0.0),
                    scaleBorder.y + 10.0 + (i-1)*tickY - textHeight / 4.0,
                    QString::number(value, '+e', Agros2D::problem()->configView()->scalarDecimalPlace));
    }

    // variable
    Module::LocalVariable localVariable = Agros2D::scene()->activeViewField()->localVariable(Agros2D::problem()->configView()->scalarVariable);
    QString str = QString("%1 (%2)").
            arg(Agros2D::problem()->configView()->scalarVariable != "" ? localVariable.shortname() : "").
            arg(Agros2D::problem()->configView()->scalarVariable != "" ? localVariable.unit() : "");

    printPostAt(scaleLeft + scaleSize.x / 2.0 - m_fontPost->glyphs[GLYPH_M].width * str.count() / 2.0,
                scaleBorder.y + scaleSize.y - 20.0,
                str);
}

void SceneViewPostInterface::paintParticleTracingColorBar(double min, double max, bool is2D)
{
    if (!Agros2D::problem()->isSolved() || !Agros2D::problem()->configView()->showParticleView) return;

    loadProjectionViewPort();

    glScaled(2.0 / width(), 2.0 / height(), 1.0);
    glTranslated(-width() / 2.0, -height() / 2.0, 0.0);

    // dimensions
    int textWidth = m_fontPost->glyphs[GLYPH_M].width * (QString::number(-1.0, '+e', Agros2D::problem()->configView()->scalarDecimalPlace).length() + 1);
    int textHeight = m_fontPost->height;
    Point scaleSize = Point(45.0 + textWidth, 20*textHeight); // contextHeight() - 20.0
    Point scaleBorder = Point(10.0, (Agros2D::problem()->configView()->showRulers) ? 1.8 * textHeight : 10.0);
    double scaleLeft = (width()
                        - (((Agros2D::problem()->configView()->showParticleView && Agros2D::problem()->configView()->showScalarView && is2D) ? scaleSize.x : 0.0) + 45.0 + textWidth));
    int numTicks = 11;

    // blended rectangle
    drawBlend(Point(scaleLeft, scaleBorder.y), Point(scaleLeft + scaleSize.x - scaleBorder.x, scaleBorder.y + scaleSize.y),
              0.91, 0.91, 0.91);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // palette border
    glColor3d(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + 10.0);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + 10.0);
    glEnd();

    // palette
    glBegin(GL_QUADS);
    glColor3d(0.0, 0.0, 0.0);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + scaleSize.y - 52.0);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + scaleSize.y - 52.0);
    glColor3d(0.8, 0.8, 0.8);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + 12.0);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + 12.0);
    glEnd();

    // ticks
    glColor3d(0.0, 0.0, 0.0);
    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (int i = 1; i < numTicks; i++)
    {
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 15.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 25.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
    }
    glEnd();

    // line
    glLineWidth(1.0);
    glBegin(GL_LINES);
    glVertex2d(scaleLeft + 5.0, scaleBorder.y + scaleSize.y - 31.0);
    glVertex2d(scaleLeft + scaleSize.x - 15.0, scaleBorder.y + scaleSize.y - 31.0);
    glEnd();

    // labels
    for (int i = 1; i < numTicks+1; i++)
    {
        double value = min + (double) (i-1) / (numTicks-1) * (max - min);

        if (fabs(value) < EPS_ZERO) value = 0.0;
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        printPostAt(scaleLeft + 33.0 + ((value >= 0.0) ? m_fontPost->glyphs[GLYPH_M].width : 0.0),
                    scaleBorder.y + 10.0 + (i-1)*tickY - textHeight / 4.0,
                    QString::number(value, '+e', Agros2D::problem()->configView()->scalarDecimalPlace));
    }

    // variable
    QString str = QString("%1 (m/s)").arg(tr("Vel."));

    printPostAt(scaleLeft + scaleSize.x / 2.0 - m_fontPost->glyphs[GLYPH_M].width  * str.count() / 2.0,
                scaleBorder.y + scaleSize.y - 20.0,
                str);
}
