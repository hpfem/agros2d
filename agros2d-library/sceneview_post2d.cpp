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
#include "util/global.h"
#include "util/loops.h"

#include "scene.h"
#include "hermes2d/problem.h"
#include "logview.h"

#include "util/constants.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "scenemarkerselectdialog.h"
#include "resultsview.h"

#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/plugin_interface.h"
#include "hermes2d/solutionstore.h"

#include "pythonlab/pythonengine_agros.h"

#include <deal.II/numerics/fe_field_function.h>
#include <deal.II/grid/grid_tools.h>

SceneViewPost2D::SceneViewPost2D(PostDeal *postDeal, QWidget *parent)
    : SceneViewCommon2D(postDeal, parent),
      m_listContours(-1),
      m_listVectors(-1),
      m_listScalarField(-1),
      m_selectedPoint(Point())
{
    createActionsPost2D();

    connect(this, SIGNAL(mousePressed(Point)), this, SLOT(selectedPoint(Point)));

    connect(Agros2D::scene(), SIGNAL(defaultValues()), this, SLOT(clear()));
    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(clear()));

    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(refresh()));
    connect(m_postDeal, SIGNAL(processed()), this, SLOT(refresh()));

    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(setControls()));
    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(setControls()));
}

SceneViewPost2D::~SceneViewPost2D()
{
}

void SceneViewPost2D::createActionsPost2D()
{
    // scene mode
    actSceneModePost2D = new QAction(iconView(), tr("Post 2D"), this);
    actSceneModePost2D->setShortcut(tr("Ctrl+4"));
    actSceneModePost2D->setCheckable(true);

    // point
    actSelectPoint = new QAction(icon("select-by-point"), tr("Local point value"), this);
    connect(actSelectPoint, SIGNAL(triggered()), this, SLOT(selectPoint()));

    // marker
    actSelectByMarker = new QAction(icon("select-by-marker"), tr("Select by marker"), this);
    connect(actSelectByMarker, SIGNAL(triggered()), this, SLOT(selectByMarker()));

    // postprocessor group
    actPostprocessorModeNothing = new QAction(icon("mode-nothing"), tr("Nothing"), this);
    actPostprocessorModeNothing->setCheckable(true);

    actPostprocessorModeLocalPointValue = new QAction(icon("mode-localpointvalue"), tr("Local Values"), this);
    actPostprocessorModeLocalPointValue->setCheckable(true);

    actPostprocessorModeSurfaceIntegral = new QAction(icon("mode-surfaceintegral"), tr("Surface Integrals"), this);
    actPostprocessorModeSurfaceIntegral->setCheckable(true);

    actPostprocessorModeVolumeIntegral = new QAction(icon("mode-volumeintegral"), tr("Volume Integrals"), this);
    actPostprocessorModeVolumeIntegral->setCheckable(true);

    actPostprocessorModeGroup = new QActionGroup(this);
    actPostprocessorModeGroup->addAction(actPostprocessorModeNothing);
    actPostprocessorModeGroup->addAction(actPostprocessorModeLocalPointValue);
    actPostprocessorModeGroup->addAction(actPostprocessorModeSurfaceIntegral);
    actPostprocessorModeGroup->addAction(actPostprocessorModeVolumeIntegral);
    connect(actPostprocessorModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(doPostprocessorModeGroup(QAction*)));

    actExportVTKScalar = new QAction(tr("Export VTK scalar..."), this);
    connect(actExportVTKScalar, SIGNAL(triggered()), this, SLOT(exportVTKScalarView()));

    actExportVTKContours = new QAction(tr("Export VTK contours..."), this);
    connect(actExportVTKContours, SIGNAL(triggered()), this, SLOT(exportVTKContourView()));
}

void SceneViewPost2D::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_A)
    {
        // select all
        if (event->modifiers() & Qt::ControlModifier)
        {
            // select volume integral area
            if (actPostprocessorModeVolumeIntegral->isChecked())
            {
                Agros2D::scene()->selectAll(SceneGeometryMode_OperateOnLabels);
                // remove holes selection
                foreach (SceneLabel *label, Agros2D::scene()->labels->items())
                    if (label->isHole())
                        label->setSelected(false);

                emit mousePressed();
            }

            // select surface integral area
            if (actPostprocessorModeSurfaceIntegral->isChecked())
            {
                Agros2D::scene()->selectAll(SceneGeometryMode_OperateOnEdges);

                emit mousePressed();
            }

            updateGL();
        }
    }

    SceneViewCommon2D::keyPressEvent(event);
}

void SceneViewPost2D::mousePressEvent(QMouseEvent *event)
{
    SceneViewCommon2D::mousePressEvent(event);

    m_lastPos = event->pos();
    Point p = transform(Point(event->pos().x(), event->pos().y()));

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
            try
            {
                dealii::Point<2> pt(p.x, p.y);
                std::pair<typename dealii::Triangulation<2>::active_cell_iterator, dealii::Point<2> > current_cell =
                        dealii::GridTools::find_active_cell_around_point(dealii::MappingQ1<2>(), *m_postDeal->activeViewField()->initialMesh().get(), pt);

                // find marker
                SceneLabel *label = Agros2D::scene()->labels->at(current_cell.first->material_id() - 1);
                label->setSelected(!label->isSelected());

                updateGL();
                emit mousePressed();
            }
            catch (const typename dealii::GridTools::ExcPointNotFound<2> &e)
            {
                // do nothing
            }
        }

        // select surface integral area
        if (actPostprocessorModeSurfaceIntegral->isChecked())
        {
            //  find edge marker
            SceneEdge *edge = SceneEdge::findClosestEdge(p);

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

    glClearColor(COLORBACKGROUND[0], COLORBACKGROUND[1], COLORBACKGROUND[2], 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    // grid
    if (Agros2D::configComputer()->value(Config::Config_ShowGrid).toBool()) paintGrid();

    // view
    if (Agros2D::problem()->isSolved() && m_postDeal->isProcessed())
    {
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarView).toBool()) paintScalarField();
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowContourView).toBool()) paintContours();
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowVectorView).toBool()) paintVectors();
    }

    // geometry
    paintGeometry();

    if (Agros2D::problem()->isSolved() && m_postDeal->isProcessed())
    {
        if (actPostprocessorModeLocalPointValue->isChecked()) paintPostprocessorSelectedPoint();
        if (actPostprocessorModeVolumeIntegral->isChecked()) paintPostprocessorSelectedVolume();
        if (actPostprocessorModeSurfaceIntegral->isChecked()) paintPostprocessorSelectedSurface();

        // bars
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarView).toBool()
                && Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarColorBar).toBool())
            paintScalarFieldColorBar(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble(),
                                     Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble());
    }

    // rulers
    if (Agros2D::configComputer()->value(Config::Config_ShowRulers).toBool())
    {
        paintRulers();
        paintRulersHints();
    }

    // axes
    if (Agros2D::configComputer()->value(Config::Config_ShowAxes).toBool()) paintAxes();

    paintZoomRegion();

    if (Agros2D::problem()->isSolved() && m_postDeal->isProcessed())
    {
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarView).toBool())
        {
            Module::LocalVariable localVariable = postDeal()->activeViewField()->localVariable(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString());
            QString text = Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString() != "" ? localVariable.name() : "";
            if ((PhysicFieldVariableComp) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariableComp).toInt() != PhysicFieldVariableComp_Scalar)
                text += " - " + physicFieldVariableCompString((PhysicFieldVariableComp) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariableComp).toInt());

            emit labelCenter(text);
        }
        else
        {
            emit labelCenter(tr("Postprocessor 2D"));
        }
    }
}

void SceneViewPost2D::resizeGL(int w, int h)
{
    if (Agros2D::problem()->isSolved())
    {
        paletteCreate();
    }

    SceneViewCommon::resizeGL(w, h);
}

void SceneViewPost2D::paintGeometry()
{
    loadProjection2d(true);

    // edges
    foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
    {
        glColor3d(COLOREDGE[0], COLOREDGE[1], COLOREDGE[2]);
        glLineWidth(EDGEWIDTH);

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

            drawArc(center, radius, startAngle, edge->angle());
        }

        glLineWidth(1.0);
    }
}

void SceneViewPost2D::paintScalarField()
{
    if (!Agros2D::problem()->isSolved()) return;

    loadProjection2d(true);

    if (m_listScalarField == -1)
    {
        if (m_postDeal->scalarValues().isEmpty()) return;

        paletteCreate();

        m_listScalarField = glGenLists(1);
        glNewList(m_listScalarField, GL_COMPILE);

        // range
        double irange = 1.0 / (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble());
        // special case: constant solution
        if (fabs(Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble() - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) < EPS_ZERO)
            irange = 1.0;

        // set texture for coloring
        glEnable(GL_TEXTURE_1D);
        glBindTexture(GL_TEXTURE_1D, m_textureScalar);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

        // set texture transformation matrix
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glTranslated(m_texShift, 0.0, 0.0);
        glScaled(m_texScale, 0.0, 0.0);

        glBegin(GL_TRIANGLES);
        foreach (PostTriangle triangle, m_postDeal->scalarValues())
        {
            if (!Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeAuto).toBool())
            {
                double avgValue = (triangle.values[0] + triangle.values[1] + triangle.values[2]) / 3.0;
                if (avgValue < Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble() || avgValue > Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble())
                    continue;
            }

            for (int j = 0; j < 3; j++)
            {
                if (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeLog).toBool())
                    glTexCoord1d(log10((double) (1 + (Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeBase).toInt() - 1))
                                       * (triangle.values[j] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange) / log10((double) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeBase).toInt()));
                else
                    glTexCoord1d((triangle.values[j] - Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble()) * irange);

                glVertex2d(triangle.vertices[j][0], triangle.vertices[j][1]);
            }
        }
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_TEXTURE_1D);

        // switch-off texture transform
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glEndList();

        glCallList(m_listScalarField);
    }
    else
    {
        glCallList(m_listScalarField);

        /*
        glLineWidth(1.0);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glBegin(GL_TRIANGLES);
        for (Hermes::Hermes2D::Views::Linearizer::Iterator<Hermes::Hermes2D::Views::ScalarLinearizerDataDimensions<LINEARIZER_DATA_TYPE>::triangle_t>
             it = m_postDeal->linScalarView()->triangles_begin(); !it.end; ++it)
        {
            Hermes::Hermes2D::Views::ScalarLinearizerDataDimensions<LINEARIZER_DATA_TYPE>::triangle_t& triangle = it.get();

            for (int j = 0; j < 3; j++)
                glVertex2d(triangle[j][0], triangle[j][1]);
        }
        glEnd();
        */

        /*
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);

        glColorPointer(3, GL_FLOAT, 0, m_arrayScalarFieldColors.constData());
        glVertexPointer(2, GL_FLOAT, 0, m_arrayScalarField.constData());

        glDrawArrays(GL_TRIANGLES, 0, m_arrayScalarField.size());

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        */
    }
}

void SceneViewPost2D::paintContours()
{
    if (!Agros2D::problem()->isSolved()) return;

    loadProjection2d(true);

    if (m_listContours == -1)
    {
        if (m_postDeal->contourValues().isEmpty()) return;

        m_listContours = glGenLists(1);
        glNewList(m_listContours, GL_COMPILE);

        // min max
        double rangeMin =  numeric_limits<double>::max();
        double rangeMax = -numeric_limits<double>::max();

        foreach (PostTriangle triangle, m_postDeal->contourValues())
        {
            for (int i = 0; i < 3; i++)
            {
                if (triangle.values[i] > rangeMax) rangeMax = triangle.values[i];
                if (triangle.values[i] < rangeMin) rangeMin = triangle.values[i];
            }
        }

        // draw contours
        if ((rangeMax-rangeMin) > EPS_ZERO)
        {
            // value range
            double step = (rangeMax-rangeMin) / Agros2D::problem()->setting()->value(ProblemSetting::View_ContoursCount).toInt();

            glLineWidth(Agros2D::problem()->setting()->value(ProblemSetting::View_ContoursWidth).toInt());
            glColor3d(COLORCONTOURS[0], COLORCONTOURS[1], COLORCONTOURS[2]);

            glBegin(GL_LINES);
            foreach (PostTriangle triangle, m_postDeal->contourValues())
            {
                paintContoursTri(triangle, step);
            }
            glEnd();
        }
        glEndList();

        glCallList(m_listContours);
    }
    else
    {
        glCallList(m_listContours);
    }
}

void SceneViewPost2D::paintContoursTri(const PostTriangle &triangle, double step)
{
    // sort the vertices by their value, keep track of the permutation sign.
    int i, idx[3] = { 0, 1, 2 }, perm = 0;
    for (i = 0; i < 2; i++)
    {
        if (triangle.values[idx[0]] > triangle.values[idx[1]])
        {
            std::swap(idx[0], idx[1]);
            perm++;
        }
        if (triangle.values[idx[1]] > triangle.values[idx[2]])
        {
            std::swap(idx[1], idx[2]);
            perm++;
        }
    }
    if (fabs(triangle.values[idx[0]] - triangle.values[idx[2]]) < 1e-3 * fabs(step))
        return;

    // get the first (lowest) contour value
    double val = triangle.values[idx[0]];

    double y = ceil(val / step);
    val = y * step;

    int l1 = 0, l2 = 1;
    int r1 = 0, r2 = 2;

    while (val < triangle.values[idx[r2]])
    {
        double ld = triangle.values[idx[l2]] - triangle.values[idx[l1]];
        double rd = triangle.values[idx[r2]] - triangle.values[idx[r1]];

        // draw a slice of the triangle
        while (val < triangle.values[idx[l2]])
        {
            double lt = (val - triangle.values[idx[l1]]) / ld;
            double rt = (val - triangle.values[idx[r1]]) / rd;

            double x1 = (1.0 - lt) * triangle.vertices[idx[l1]][0] + lt * triangle.vertices[idx[l2]][0];
            double y1 = (1.0 - lt) * triangle.vertices[idx[l1]][1] + lt * triangle.vertices[idx[l2]][1];
            double x2 = (1.0 - rt) * triangle.vertices[idx[r1]][0] + rt * triangle.vertices[idx[r2]][0];
            double y2 = (1.0 - rt) * triangle.vertices[idx[r1]][1] + rt * triangle.vertices[idx[r2]][1];

            if (perm & 1)
            {
                glVertex2d(x1, y1);
                glVertex2d(x2, y2);
            }
            else
            {
                glVertex2d(x2, y2);
                glVertex2d(x1, y1);
            }

            val += step;
        }
        l1 = 1;
        l2 = 2;
    }
}

void SceneViewPost2D::paintVectors()
{
    if (!Agros2D::problem()->isSolved()) return;

    loadProjection2d(true);

    if (m_listVectors == -1)
    {
        // if (!m_postDeal->vecVectorView()) return;

        m_listVectors = glGenLists(1);
        glNewList(m_listVectors, GL_COMPILE);

        RectPoint rect = Agros2D::scene()->boundingBox();
        double gs = (rect.width() + rect.height()) / Agros2D::problem()->setting()->value(ProblemSetting::View_VectorCount).toInt();

        MultiArray ma = m_postDeal->activeMultiSolutionArray();
        dealii::Functions::FEFieldFunction<2> localvalues(*ma.doFHandler(), *ma.solution());

        // min max
        double rangeMin =  numeric_limits<double>::max();
        double rangeMax = -numeric_limits<double>::max();

        int countX = rect.width() / gs;
        int countY = rect.height() / gs;

        QList<dealii::Point<2> > points;
        QList<dealii::Tensor<1, 2> > gradients;
        for (int i = 0; i < countX; i++)
        {
            for (int j = 0; j < countY; j++)
            {
                try
                {
                    dealii::Point<2> point(rect.start.x + i * gs + ((j % 2 == 0) ? 0 : gs / 2.0), rect.start.y + j * gs);
                    dealii::Tensor<1, 2> grad = localvalues.gradient(point);

                    points.append(point);
                    gradients.append(grad);

                    if (grad.norm() > rangeMax) rangeMax = grad.norm();
                    if (grad.norm() < rangeMin) rangeMin = grad.norm();
                }
                catch (const typename dealii::GridTools::ExcPointNotFound<2> &e)
                {
                    continue;
                }
            }
        }

        //Add 20% margin to the range
        double vectorRange = rangeMax - rangeMin;
        rangeMin = rangeMin - 0.2*vectorRange;
        rangeMax = rangeMax + 0.2*vectorRange;

        // qDebug() << "SceneViewCommon::paintVectors(), min = " << vectorRangeMin << ", max = " << vectorRangeMax;

        double irange = 1.0 / (rangeMax - rangeMin);
        // if (fabs(vectorRangeMin - vectorRangeMax) < EPS_ZERO) return;

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < points.size(); i++)
        {
            Point point = Point(points[i][0], points[i][1]);

            double dx = gradients[i][0];
            double dy = gradients[i][1];

            double value = sqrt(dx*dx + dy*dy);
            double angle = atan2(dy, dx);

            if ((Agros2D::problem()->setting()->value(ProblemSetting::View_VectorProportional).toBool()) && (fabs(rangeMin - rangeMax) > EPS_ZERO))
            {
                if ((value / rangeMax) < 1e-6)
                {
                    dx = 0.0;
                    dy = 0.0;
                }
                else
                {
                    dx = ((value - rangeMin) * irange) * Agros2D::problem()->setting()->value(ProblemSetting::View_VectorScale).toDouble() * gs * cos(angle);
                    dy = ((value - rangeMin) * irange) * Agros2D::problem()->setting()->value(ProblemSetting::View_VectorScale).toDouble() * gs * sin(angle);
                }
            }
            else
            {
                dx = Agros2D::problem()->setting()->value(ProblemSetting::View_VectorScale).toDouble() * gs * cos(angle);
                dy = Agros2D::problem()->setting()->value(ProblemSetting::View_VectorScale).toDouble() * gs * sin(angle);
            }

            double dm = sqrt(dx*dx + dy*dy);
            // qDebug() << dx << dy << dm;

            // color
            if ((Agros2D::problem()->setting()->value(ProblemSetting::View_VectorColor).toBool())
                    && (fabs(rangeMin - rangeMax) > EPS_ZERO))
            {
                double color = 0.7 - 0.7 * (value - rangeMin) * irange;
                glColor3d(color, color, color);
            }
            else
            {
                glColor3d(COLORVECTORS[0], COLORVECTORS[1], COLORVECTORS[2]);
            }

            // tail
            Point shiftCenter(0.0, 0.0);
            if ((VectorCenter) Agros2D::problem()->setting()->value(ProblemSetting::View_VectorCenter).toInt() == VectorCenter_Head)
                shiftCenter = Point(- 2.0*dm * cos(angle), - 2.0*dm * sin(angle)); // head
            if ((VectorCenter) Agros2D::problem()->setting()->value(ProblemSetting::View_VectorCenter).toInt() == VectorCenter_Center)
                shiftCenter = Point(- dm * cos(angle), - dm * sin(angle)); // center

            if ((VectorType) Agros2D::problem()->setting()->value(ProblemSetting::View_VectorType).toInt() == VectorType_Arrow)
            {
                // arrow and shaft
                // head for an arrow
                double vh1x = point.x + dm/5.0 * cos(angle - M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                double vh1y = point.y + dm/5.0 * sin(angle - M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                double vh2x = point.x + dm/5.0 * cos(angle + M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                double vh2y = point.y + dm/5.0 * sin(angle + M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                double vh3x = point.x + 2.0 * dm * cos(angle) + shiftCenter.x;
                double vh3y = point.y + 2.0 * dm * sin(angle) + shiftCenter.y;

                glVertex2d(vh1x, vh1y);
                glVertex2d(vh2x, vh2y);
                glVertex2d(vh3x, vh3y);

                // shaft for an arrow
                double vs1x = point.x + dm/15.0 * cos(angle + M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                double vs1y = point.y + dm/15.0 * sin(angle + M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                double vs2x = point.x + dm/15.0 * cos(angle - M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                double vs2y = point.y + dm/15.0 * sin(angle - M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                double vs3x = vs1x - dm * cos(angle);
                double vs3y = vs1y - dm * sin(angle);
                double vs4x = vs2x - dm * cos(angle);
                double vs4y = vs2y - dm * sin(angle);

                glVertex2d(vs1x, vs1y);
                glVertex2d(vs2x, vs2y);
                glVertex2d(vs3x, vs3y);
                glVertex2d(vs4x, vs4y);
                glVertex2d(vs3x, vs3y);
                glVertex2d(vs2x, vs2y);
            }
            else if ((VectorType) Agros2D::problem()->setting()->value(ProblemSetting::View_VectorType).toInt() == VectorType_Cone)
            {
                // cone
                double vh1x = point.x + dm/3.5 * cos(angle - M_PI/2.0) + shiftCenter.x;
                double vh1y = point.y + dm/3.5 * sin(angle - M_PI/2.0) + shiftCenter.y;
                double vh2x = point.x + dm/3.5 * cos(angle + M_PI/2.0) + shiftCenter.x;
                double vh2y = point.y + dm/3.5 * sin(angle + M_PI/2.0) + shiftCenter.y;
                double vh3x = point.x + 2.0 * dm * cos(angle) + shiftCenter.x;
                double vh3y = point.y + 2.0 * dm * sin(angle) + shiftCenter.y;

                glVertex2d(vh1x, vh1y);
                glVertex2d(vh2x, vh2y);
                glVertex2d(vh3x, vh3y);
            }
        }
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);

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
    if (!Agros2D::problem()->isSolved()) return;

    if (Agros2D::scene()->crossings().isEmpty())
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // blended rectangle
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4d(COLORSELECTED[0], COLORSELECTED[1], COLORSELECTED[2], 0.5);

        QMapIterator<SceneLabel*, QList<LoopsInfo::Triangle> > i(Agros2D::scene()->loopsInfo()->polygonTriangles());
        glBegin(GL_TRIANGLES);
        while (i.hasNext())
        {
            i.next();

            if (i.key()->isSelected())
            {
                foreach (LoopsInfo::Triangle triangle, i.value())
                {
                    glVertex2d(triangle.a.x, triangle.a.y);
                    glVertex2d(triangle.b.x, triangle.b.y);
                    glVertex2d(triangle.c.x, triangle.c.y);
                }
            }
        }
        glEnd();

        glDisable(GL_BLEND);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

void SceneViewPost2D::paintPostprocessorSelectedSurface()
{
    if (!Agros2D::problem()->isSolved()) return;

    // edges
    foreach (SceneEdge *edge, Agros2D::scene()->edges->items()) {
        glColor3d(COLORSELECTED[0], COLORSELECTED[1], COLORSELECTED[2]);
        glLineWidth(3.0);

        if (edge->isSelected())
        {
            if (edge->isStraight())
            {
                glBegin(GL_LINES);
                glVertex2d(edge->nodeStart()->point().x, edge->nodeStart()->point().y);
                glVertex2d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y);
                glEnd();

                // connect with inner label, outer normal should be in opposite dirrection, but not allways, depends on geometry!
                /*
                glLineWidth(2.0);
                glBegin(GL_LINES);
                glVertex2d((edge->nodeStart()->point().x + edge->nodeEnd()->point().x) / 2., (edge->nodeStart()->point().y + edge->nodeEnd()->point().y) / 2.);
                SceneLabel* label = Agros2D::scene()->labels->at(edge->innerLabelIdx());
                glVertex2d(label->point().x, label->point().y);

                glEnd();
                */
            }
            else
            {
                Point center = edge->center();
                double radius = edge->radius();
                double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180 - 180;

                drawArc(center, radius, startAngle, edge->angle());
            }
        }
        glLineWidth(1.0);
    }
}

void SceneViewPost2D::paintPostprocessorSelectedPoint()
{
    if (!Agros2D::problem()->isSolved()) return;

    glColor3d(COLORSELECTED[0], COLORSELECTED[1], COLORSELECTED[2]);
    glPointSize(8.0);

    glBegin(GL_POINTS);
    glVertex2d(m_selectedPoint.x, m_selectedPoint.y);
    glEnd();
}

void SceneViewPost2D::clearGLLists()
{
    if (m_listContours != -1) glDeleteLists(m_listContours, 1);
    if (m_listVectors != -1) glDeleteLists(m_listVectors, 1);
    if (m_listScalarField != -1) glDeleteLists(m_listScalarField, 1);

    m_listContours = -1;
    m_listVectors = -1;
    m_listScalarField = -1;
}

void SceneViewPost2D::refresh()
{
    clearGLLists();

    setControls();

    if (Agros2D::problem()->isSolved())
        SceneViewCommon2D::refresh();
}

void SceneViewPost2D::setControls()
{
    // actions
    actSceneModePost2D->setEnabled(Agros2D::problem()->isSolved());
    actPostprocessorModeGroup->setEnabled(Agros2D::problem()->isSolved());
    actSelectByMarker->setEnabled(Agros2D::problem()->isSolved() && (actPostprocessorModeSurfaceIntegral->isChecked() || actPostprocessorModeVolumeIntegral->isChecked()));
    actSelectPoint->setEnabled(Agros2D::problem()->isSolved() && actPostprocessorModeLocalPointValue->isChecked());
    actExportVTKScalar->setEnabled(Agros2D::problem()->isSolved());
    actExportVTKContours->setEnabled(Agros2D::problem()->isSolved());
}

void SceneViewPost2D::clear()
{
    actPostprocessorModeNothing->trigger();

    setControls();

    SceneViewCommon2D::clear();
    if (Agros2D::problem()->isSolved())
        SceneViewCommon::refresh();
}

void SceneViewPost2D::exportVTKScalarView(const QString &fileName)
{
    exportVTK(fileName,
              Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString(),
              (PhysicFieldVariableComp) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariableComp).toInt());
}

void SceneViewPost2D::exportVTKContourView(const QString &fileName)
{
    Module::LocalVariable variable = postDeal()->activeViewField()->localVariable(Agros2D::problem()->setting()->value(ProblemSetting::View_ContourVariable).toString());
    PhysicFieldVariableComp comp = variable.isScalar() ? PhysicFieldVariableComp_Scalar : PhysicFieldVariableComp_Magnitude;

    exportVTK(fileName,
              Agros2D::problem()->setting()->value(ProblemSetting::View_ContourVariable).toString(),
              comp);
}

void SceneViewPost2D::exportVTK(const QString &fileName, const QString &variable, PhysicFieldVariableComp physicFieldVariableComp)
{
    if (Agros2D::problem()->isSolved())
    {
        QString fn = fileName;

        if (fn.isEmpty())
        {
            // file dialog
            QSettings settings;
            QString dir = settings.value("General/LastVTKDir").toString();

            fn = QFileDialog::getSaveFileName(this, tr("Export VTK file"), dir, tr("VTK files (*.vtk)"));
            if (fn.isEmpty())
                return;

            if (!fn.endsWith(".vtk"))
                fn.append(".vtk");

            // remove existing file
            if (QFile::exists(fn))
                QFile::remove(fn);
        }

        std::shared_ptr<PostDataOut> data_out = m_postDeal->viewScalarFilter(postDeal()->activeViewField()->localVariable(variable), physicFieldVariableComp);

        std::ofstream output (fn.toStdString());
        data_out->write_vtk(output);


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
    if (sceneMarkerSelectDialog.exec() == QDialog::Accepted)
        emit mousePressed();
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
    if (actPostprocessorModeNothing->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_Empty);
    if (actPostprocessorModeLocalPointValue->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_LocalValue);
    if (actPostprocessorModeSurfaceIntegral->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_SurfaceIntegral);
    if (actPostprocessorModeVolumeIntegral->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_VolumeIntegral);

    Agros2D::scene()->selectNone();

    setControls();
    updateGL();
}

void SceneViewPost2D::showGroup(QAction *action)
{
    refresh();
}

void SceneViewPost2D::selectedPoint(const Point &p)
{
    m_selectedPoint = p;
    updateGL();
}
