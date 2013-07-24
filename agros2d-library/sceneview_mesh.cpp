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

#include "sceneview_mesh.h"

#include "util.h"
#include "util/global.h"
#include "util/constants.h"

#include "scene.h"
#include "hermes2d/field.h"
#include "logview.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/module.h"

SceneViewMesh::SceneViewMesh(PostHermes *postHermes, QWidget *parent)
    : SceneViewCommon2D(postHermes, parent)
{
    createActionsMesh();

    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(clear()));

    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(refresh()));
    connect(m_postHermes, SIGNAL(processed()), this, SLOT(refresh()));

    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(setControls()));
    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(setControls()));
}

SceneViewMesh::~SceneViewMesh()
{
}

void SceneViewMesh::createActionsMesh()
{
    // scene mode
    actSceneModeMesh = new QAction(iconView(), tr("Mesh"), this);
    actSceneModeMesh->setShortcut(tr("Ctrl+3"));
    actSceneModeMesh->setCheckable(true);

    actExportVTKOrder = new QAction(tr("Export VTK order..."), this);
    connect(actExportVTKOrder, SIGNAL(triggered()), this, SLOT(exportVTKOrderView()));

    actExportVTKMesh = new QAction(tr("Export VTK mesh..."), this);
    connect(actExportVTKMesh, SIGNAL(triggered()), this, SLOT(exportVTKMesh()));
}

void SceneViewMesh::refresh()
{
    m_arrayInitialMesh.clear();
    m_arraySolutionMesh.clear();
    m_arrayOrderMesh.clear();
    m_arrayOrderMeshColor.clear();

    setControls();

    SceneViewCommon::refresh();
}

void SceneViewMesh::setControls()
{
    // actions
    actSceneModeMesh->setEnabled(Agros2D::problem()->isMeshed());
    actExportVTKMesh->setEnabled(Agros2D::problem()->isSolved());
    actExportVTKOrder->setEnabled(Agros2D::problem()->isSolved());
}

void SceneViewMesh::clear()
{
    setControls();

    SceneViewCommon2D::clear();
    if (Agros2D::problem()->isMeshed() || Agros2D::problem()->isSolved())
        doZoomBestFit();
}

void SceneViewMesh::exportVTKMesh(const QString &fileName)
{
    exportVTK(fileName, true);
}

void SceneViewMesh::exportVTKOrderView(const QString &fileName)
{
    exportVTK(fileName, false);
}


void SceneViewMesh::exportVTK(const QString &fileName, bool exportMeshOnly)
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

        Hermes::Hermes2D::Views::Orderizer orderView;
        if (exportMeshOnly)
            orderView.save_mesh_vtk(postHermes()->activeMultiSolutionArray().spaces().at(0),
                                    fn.toLatin1().data());
        else
            orderView.save_orders_vtk(postHermes()->activeMultiSolutionArray().spaces().at(0),
                                      fn.toLatin1().data());

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

void SceneViewMesh::paintGL()
{
    if (!isVisible()) return;
    makeCurrent();

    glClearColor(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundRed).toInt() / 255.0,
                 Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundGreen).toInt() / 255.0,
                 Agros2D::problem()->setting()->value(ProblemSetting::View_ColorBackgroundBlue).toInt() / 255.0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    // grid
    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowGrid).toBool()) paintGrid();

    QTime time;

    // view
    if (Agros2D::problem()->isSolved() && m_postHermes->isProcessed())
    {
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowOrderView).toBool()) paintOrder();
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowSolutionMeshView).toBool()) paintSolutionMesh();
    }

    // initial mesh
    if (Agros2D::problem()->isMeshed() && m_postHermes->isProcessed())
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowInitialMeshView).toBool()) paintInitialMesh();

    // geometry
    paintGeometry();

    if (Agros2D::problem()->isSolved() && m_postHermes->isProcessed())
    {
        // bars
        if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowOrderView).toBool()
                && Agros2D::problem()->setting()->value(ProblemSetting::View_ShowOrderColorBar).toBool())
            paintOrderColorBar();
    }

    // rulers
    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowRulers).toBool())
    {
        paintRulers();
        paintRulersHints();
    }

    // axes
    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowAxes).toBool()) paintAxes();

    paintZoomRegion();
}

void SceneViewMesh::paintGeometry()
{
    loadProjection2d(true);

    // edges
    foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
    {
        glColor3d(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorEdgesRed).toInt() / 255.0,
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorEdgesGreen).toInt() / 255.0,
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorEdgesBlue).toInt() / 255.0);
        glLineWidth(Agros2D::problem()->setting()->value(ProblemSetting::View_EdgeWidth).toInt());

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

void SceneViewMesh::paintInitialMesh()
{
    if (!Agros2D::problem()->isMeshed()) return;
    if (!m_postHermes->initialMeshIsPrepared()) return;

    if (m_arrayInitialMesh.isEmpty())
    {
        m_postHermes->linInitialMeshView().lock_data();

        double3* linVert = m_postHermes->linInitialMeshView().get_vertices();
        int2* linEdges = m_postHermes->linInitialMeshView().get_edges();

        // edges
        m_arrayInitialMesh.reserve(2 * m_postHermes->linInitialMeshView().get_num_edges());
        for (int i = 0; i < m_postHermes->linInitialMeshView().get_num_edges(); i++)
        {
            m_arrayInitialMesh.append(QVector2D(linVert[linEdges[i][0]][0], linVert[linEdges[i][0]][1]));
            m_arrayInitialMesh.append(QVector2D(linVert[linEdges[i][1]][0], linVert[linEdges[i][1]][1]));
        }

        m_postHermes->linInitialMeshView().unlock_data();
    }
    else
    {
        loadProjection2d(true);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3d(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorInitialMeshRed).toInt() / 255.0,
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorInitialMeshGreen).toInt() / 255.0,
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorInitialMeshBlue).toInt() / 255.0);
        glLineWidth(1.3);

        glEnableClientState(GL_VERTEX_ARRAY);

        glVertexPointer(2, GL_FLOAT, 0, m_arrayInitialMesh.constData());
        glDrawArrays(GL_LINES, 0, m_arrayInitialMesh.size());

        glDisableClientState(GL_VERTEX_ARRAY);
    }
}


void SceneViewMesh::paintSolutionMesh()
{
    if (!Agros2D::problem()->isSolved()) return;
    if (!m_postHermes->solutionMeshIsPrepared()) return;

    if (m_arraySolutionMesh.isEmpty())
    {
        m_postHermes->linSolutionMeshView().lock_data();

        double3* linVert = m_postHermes->linSolutionMeshView().get_vertices();
        int2* linEdges = m_postHermes->linSolutionMeshView().get_edges();

        // edges
        m_arraySolutionMesh.reserve(2 * m_postHermes->linSolutionMeshView().get_num_edges());
        for (int i = 0; i < m_postHermes->linSolutionMeshView().get_num_edges(); i++)
        {
            m_arraySolutionMesh.append(QVector2D(linVert[linEdges[i][0]][0], linVert[linEdges[i][0]][1]));
            m_arraySolutionMesh.append(QVector2D(linVert[linEdges[i][1]][0], linVert[linEdges[i][1]][1]));
        }

        m_postHermes->linSolutionMeshView().unlock_data();
    }
    else
    {
        loadProjection2d(true);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3d(Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSolutionMeshRed).toInt() / 255.0,
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSolutionMeshGreen).toInt() / 255.0,
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ColorSolutionMeshBlue).toInt() / 255.0);
        glLineWidth(1.3);

        glEnableClientState(GL_VERTEX_ARRAY);

        glVertexPointer(2, GL_FLOAT, 0, m_arraySolutionMesh.constData());
        glDrawArrays(GL_LINES, 0, m_arraySolutionMesh.size());

        glDisableClientState(GL_VERTEX_ARRAY);
    }
}

void SceneViewMesh::paintOrder()
{
    if (!Agros2D::problem()->isSolved()) return;
    if (!m_postHermes->orderIsPrepared()) return;

    if (m_arrayOrderMesh.isEmpty())
    {
        // order scalar view
        m_postHermes->ordView().lock_data();

        double3* vert = m_postHermes->ordView().get_vertices();
        int3* tris = m_postHermes->ordView().get_triangles();

        // draw mesh
        int min = 11;
        int max = 1;
        for (int i = 0; i < m_postHermes->ordView().get_num_triangles(); i++)
        {
            if (vert[tris[i][0]][2] < min) min = vert[tris[i][0]][2];
            if (vert[tris[i][0]][2] > max) max = vert[tris[i][0]][2];
        }

        // triangles
        m_arrayOrderMesh.reserve(3 * m_postHermes->ordView().get_num_triangles());
        m_arrayOrderMeshColor.reserve(3 * m_postHermes->ordView().get_num_triangles());
        for (int i = 0; i < m_postHermes->ordView().get_num_triangles(); i++)
        {
            int color = vert[tris[i][0]][2];
            QVector3D colorVector = QVector3D(paletteColorOrder(color)[0],
                    paletteColorOrder(color)[1],
                    paletteColorOrder(color)[2]);

            m_arrayOrderMesh.append(QVector2D(vert[tris[i][0]][0], vert[tris[i][0]][1]));
            m_arrayOrderMeshColor.append(colorVector);

            m_arrayOrderMesh.append(QVector2D(vert[tris[i][1]][0], vert[tris[i][1]][1]));
            m_arrayOrderMeshColor.append(colorVector);

            m_arrayOrderMesh.append(QVector2D(vert[tris[i][2]][0], vert[tris[i][2]][1]));
            m_arrayOrderMeshColor.append(colorVector);
        }

        m_postHermes->ordView().unlock_data();
    }
    else
    {
        loadProjection2d(true);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);

        glVertexPointer(2, GL_FLOAT, 0, m_arrayOrderMesh.constData());
        glColorPointer(3, GL_FLOAT, 0, m_arrayOrderMeshColor.constData());
        glDrawArrays(GL_TRIANGLES, 0, m_arrayOrderMesh.size());

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);

        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    // paint labels
    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowOrderLabel).toBool())
    {
        loadProjectionViewPort();

        glScaled(2.0 / width(), 2.0 / height(), 1.0);
        glTranslated(-width() / 2.0, -height() / 2.0, 0.0);

        m_postHermes->ordView().lock_data();

        double3* vert = m_postHermes->ordView().get_vertices();
        int* lvert;
        char** ltext;
        double2* lbox;
        int nl = m_postHermes->ordView().get_labels(lvert, ltext, lbox);

        for (int i = 0; i < nl; i++)
        {
            glColor3d(1, 1, 1);
            // if (lbox[i][0]/m_scale*aspect() > size.x && lbox[i][1]/m_scale > size.y)
            {
                Point scr = untransform(vert[lvert[i]][0], vert[lvert[i]][1]);
                printPostAt(scr.x - (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0) / 2.0,
                            scr.y - (m_charDataPost[GLYPH_M].y1 - m_charDataPost[GLYPH_M].y0) / 2.0,
                            ltext[i]);
            }
        }

        m_postHermes->ordView().unlock_data();
    }
}

void SceneViewMesh::paintOrderColorBar()
{
    if (!Agros2D::problem()->isSolved() || !Agros2D::problem()->setting()->value(ProblemSetting::View_ShowOrderColorBar).toBool()) return;

    // order scalar view
    m_postHermes->ordView().lock_data();

    double3* vert = m_postHermes->ordView().get_vertices();
    int3* tris = m_postHermes->ordView().get_triangles();

    int min = 11;
    int max = 1;
    for (int i = 0; i < m_postHermes->ordView().get_num_triangles(); i++)
    {
        if (vert[tris[i][0]][2] < min) min = vert[tris[i][0]][2];
        if (vert[tris[i][0]][2] > max) max = vert[tris[i][0]][2];
    }

    m_postHermes->ordView().unlock_data();

    // order color map
    loadProjectionViewPort();

    glScaled(2.0 / width(), 2.0 / height(), 1.0);
    glTranslated(- width() / 2.0, -height() / 2.0, 0.0);

    // dimensions
    int textWidth = 6 * (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0);
    int textHeight = 2 * (m_charDataPost[GLYPH_M].y1 - m_charDataPost[GLYPH_M].y0);
    Point scaleSize = Point(20 + textWidth, (20 + max * (2 * textHeight) - textHeight / 2.0 + 2));
    Point scaleBorder = Point(10.0, (Agros2D::problem()->setting()->value(ProblemSetting::View_ShowRulers).toBool()) ? 1.8 * textHeight : 10.0);
    double scaleLeft = (width() - (20 + textWidth));

    // blended rectangle
    drawBlend(Point(scaleLeft, scaleBorder.y), Point(scaleLeft + scaleSize.x - scaleBorder.x, scaleBorder.y + scaleSize.y),
              0.91, 0.91, 0.91);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // bars
    glBegin(GL_QUADS);
    for (int i = 1; i < max+1; i++)
    {
        glColor3d(0.0, 0.0, 0.0);
        glVertex2d(scaleLeft + 10,                             scaleBorder.y + 10 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + textWidth - scaleBorder.x, scaleBorder.y + 10 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + textWidth - scaleBorder.x, scaleBorder.y + 12 + (i )*(2 * textHeight) - textHeight / 2.0);
        glVertex2d(scaleLeft + 10,                             scaleBorder.y + 12 + (i )*(2 * textHeight) - textHeight / 2.0);

        glColor3d(paletteColorOrder(i)[0], paletteColorOrder(i)[1], paletteColorOrder(i)[2]);
        glVertex2d(scaleLeft + 12,                                 scaleBorder.y + 12 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + textWidth - 2 - scaleBorder.x, scaleBorder.y + 12 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + textWidth - 2 - scaleBorder.x, scaleBorder.y + 10 + (i  )*(2 * textHeight) - textHeight / 2.0);
        glVertex2d(scaleLeft + 12,                                 scaleBorder.y + 10 + (i  )*(2 * textHeight) - textHeight / 2.0);
    }
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    // labels
    glColor3d(1.0, 1.0, 1.0);
    for (int i = 1; i < max + 1; i++)
    {
        printPostAt(scaleLeft + 10 + 3.5 * (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0) - 2 - scaleBorder.x,
                    scaleBorder.y + 10.0 + (i-1)*(2.0 * textHeight) + textHeight / 2.0,
                    QString::number(i));
    }
}
