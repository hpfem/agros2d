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
#include "scene.h"
#include "scenesolution.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "hermes2d/problem.h"
#include "hermes2d/module.h"

MeshHermes::MeshHermes()
{
}


MeshHermes::~MeshHermes()
{
    clear();
}

void MeshHermes::clear()
{

}

void MeshHermes::processOrder()
{
    // init linearizer for order view
    m_orderView.process_space(Util::scene()->activeSceneSolution()->space(0));
}

void MeshHermes::processInitialMesh()
{
    // init linearizer for initial mesh
    Hermes::Hermes2D::ZeroSolution<double> initial(Util::problem()->meshInitial());
    m_linInitialMeshView.process_solution(&initial);
}

void MeshHermes::processSolutionMesh()
{
    // init linearizer for solution mesh
    Hermes::Hermes2D::ZeroSolution<double> solution(Util::scene()->activeSceneSolution()->sln(0)->get_mesh());
    m_linSolutionMeshView.process_solution(&solution);
}

void MeshHermes::meshed()
{
    qDebug("MeshHermes::meshed()");

    processInitialMesh();
}

void MeshHermes::solved()
{
    qDebug("MeshHermes::solved()");

    processSolutionMesh();
    processOrder();
    //TODO timedependence rpoblemsm_timeStep * Util::scene()->problemInfo()->module()->number_of_solution())->space);
}

// ************************************************************************************************

SceneViewMesh::SceneViewMesh(QWidget *parent): SceneViewCommon2D(parent),
    m_listInitialMesh(-1),
    m_listSolutionMesh(-1),
    m_listOrder(-1)
{
    createActionsMesh();

    m_meshHermes = new MeshHermes();

    connect(Util::problem(), SIGNAL(meshed()), this, SLOT(doInvalidated()));
    connect(Util::problem(), SIGNAL(solved()), this, SLOT(doInvalidated()));
}

SceneViewMesh::~SceneViewMesh()
{
    delete m_meshHermes;
}

void SceneViewMesh::createActionsMesh()
{
    // scene mode
    actSceneModeMesh = new QAction(icon("scene-mesh"), tr("Mesh"), this);
    // actSceneModeMesh->setShortcut(Qt::Key_F7);
    actSceneModeMesh->setStatusTip(tr("Mesh"));
    actSceneModeMesh->setCheckable(true);
}

void SceneViewMesh::doInvalidated()
{
    if (m_listInitialMesh != -1) glDeleteLists(m_listInitialMesh, 1);
    if (m_listSolutionMesh != -1) glDeleteLists(m_listSolutionMesh, 1);
    if (m_listOrder != -1) glDeleteLists(m_listOrder, 1);

    m_listInitialMesh = -1;
    m_listSolutionMesh = -1;
    m_listOrder = -1;

    if (Util::problem()->isMeshed()) m_meshHermes->meshed();
    if (Util::problem()->isSolved()) m_meshHermes->solved();

    SceneViewCommon::doInvalidated();
}

void SceneViewMesh::clear()
{
    m_meshHermes->clear();

    SceneViewCommon2D::clear();
}

void SceneViewMesh::paintGL()
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
        if (Util::config()->showOrderView) paintOrder();
        if (Util::config()->showSolutionMeshView) paintSolutionMesh();
    }

    // initial mesh
    if (Util::problem()->isMeshed())
        if (Util::config()->showInitialMeshView) paintInitialMesh();

    // geometry
    paintGeometry();

    if (Util::problem()->isSolved())
    {
        // bars
        if (Util::config()->showOrderView && Util::config()->showOrderColorBar)
            paintOrderColorBar();
    }

    // rulers
    if (Util::config()->showRulers)
    {
        paintRulers();
        paintRulersHints();
    }

    // axes
    if (Util::config()->showAxes) paintAxes();

    // label
    if (Util::config()->showLabel) paintSceneModeLabel(tr("Mesh"));
}

void SceneViewMesh::paintGeometry()
{
    loadProjection2d(true);

    // edges
    foreach (SceneEdge *edge, Util::scene()->edges->items())
    {
        glColor3d(Util::config()->colorEdges.redF(),
                  Util::config()->colorEdges.greenF(),
                  Util::config()->colorEdges.blueF());
        glLineWidth(Util::config()->edgeWidth);

        if (fabs(edge->angle) < EPS_ZERO)
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

            drawArc(center, radius, startAngle, edge->angle, edge->angle/2.0);
        }

        glLineWidth(1.0);
    }
}

void SceneViewMesh::paintInitialMesh()
{
    if (!Util::problem()->isMeshed()) return;

    loadProjection2d(true);

    m_meshHermes->linInitialMeshView().lock_data();

    double3* linVert = m_meshHermes->linInitialMeshView().get_vertices();
    int3* linEdges = m_meshHermes->linInitialMeshView().get_edges();

    // draw initial mesh
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3d(Util::config()->colorInitialMesh.redF(),
              Util::config()->colorInitialMesh.greenF(),
              Util::config()->colorInitialMesh.blueF());
    glLineWidth(1.3);

    // triangles
    glBegin(GL_LINES);
    for (int i = 0; i < m_meshHermes->linInitialMeshView().get_num_edges(); i++)
    {
        glVertex2d(linVert[linEdges[i][0]][0], linVert[linEdges[i][0]][1]);
        glVertex2d(linVert[linEdges[i][1]][0], linVert[linEdges[i][1]][1]);
    }
    glEnd();

    m_meshHermes->linInitialMeshView().unlock_data();
}

void SceneViewMesh::paintSolutionMesh()
{
    if (!Util::problem()->isSolved()) return;

    loadProjection2d(true);

    m_meshHermes->linSolutionMeshView().lock_data();

    double3* linVert = m_meshHermes->linSolutionMeshView().get_vertices();
    int3* linEdges = m_meshHermes->linSolutionMeshView().get_edges();

    // draw initial mesh
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3d(Util::config()->colorSolutionMesh.redF(),
              Util::config()->colorSolutionMesh.greenF(),
              Util::config()->colorSolutionMesh.blueF());
    glLineWidth(1.3);

    // triangles
    // qDebug() << Util::problem()->linSolutionMeshView().get_num_edges();
    glBegin(GL_LINES);
    for (int i = 0; i < m_meshHermes->linSolutionMeshView().get_num_edges(); i++)
    {
        glVertex2d(linVert[linEdges[i][0]][0], linVert[linEdges[i][0]][1]);
        glVertex2d(linVert[linEdges[i][1]][0], linVert[linEdges[i][1]][1]);
    }
    glEnd();

    m_meshHermes->linSolutionMeshView().unlock_data();
}

void SceneViewMesh::paintOrder()
{
    if (!Util::problem()->isSolved()) return;

    loadProjection2d(true);

    if (m_listOrder == -1)
    {
        m_listOrder = glGenLists(1);
        glNewList(m_listOrder, GL_COMPILE);

        // order scalar view
        m_meshHermes->ordView().lock_data();

        double3* vert = m_meshHermes->ordView().get_vertices();
        int3* tris = m_meshHermes->ordView().get_triangles();

        // draw mesh
        int min = 11;
        int max = 1;
        for (int i = 0; i < m_meshHermes->ordView().get_num_triangles(); i++)
        {
            if (vert[tris[i][0]][2] < min) min = vert[tris[i][0]][2];
            if (vert[tris[i][0]][2] > max) max = vert[tris[i][0]][2];
        }

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // triangles
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_meshHermes->ordView().get_num_triangles(); i++)
        {
            int color = vert[tris[i][0]][2];
            glColor3d(paletteColorOrder(color)[0], paletteColorOrder(color)[1], paletteColorOrder(color)[2]);

            glVertex2d(vert[tris[i][0]][0], vert[tris[i][0]][1]);
            glVertex2d(vert[tris[i][1]][0], vert[tris[i][1]][1]);
            glVertex2d(vert[tris[i][2]][0], vert[tris[i][2]][1]);
        }
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);

        m_meshHermes->ordView().unlock_data();

        glEndList();

        glCallList(m_listOrder);
    }
    else
    {
        glCallList(m_listOrder);
    }

    // paint labels
    if (Util::config()->orderLabel)
    {
        QFont fontLabel = font();
        fontLabel.setPointSize(fontLabel.pointSize() - 3);

        m_meshHermes->ordView().lock_data();

        double3* vert = m_meshHermes->ordView().get_vertices();
        int* lvert;
        char** ltext;
        double2* lbox;
        int nl = m_meshHermes->ordView().get_labels(lvert, ltext, lbox);

        Point size((2.0/width()*fontMetrics().width(" "))/m_scale2d*aspect(),
                   (2.0/height()*fontMetrics().height())/m_scale2d);

        for (int i = 0; i < nl; i++)
        {
            glColor3d(1, 1, 1);
            // if (lbox[i][0]/m_scale*aspect() > size.x && lbox[i][1]/m_scale > size.y)
            {
                renderText(vert[lvert[i]][0] - size.x / 2.0,
                           vert[lvert[i]][1] - size.y / 2.0,
                           0.0,
                           ltext[i],
                           fontLabel);
            }
        }

        m_meshHermes->ordView().unlock_data();
    }
}

void SceneViewMesh::paintOrderColorBar()
{
    if (!Util::problem()->isSolved() || !Util::config()->showOrderColorBar) return;

    // order scalar view
    m_meshHermes->ordView().lock_data();

    double3* vert = m_meshHermes->ordView().get_vertices();
    int3* tris = m_meshHermes->ordView().get_triangles();

    int min = 11;
    int max = 1;
    for (int i = 0; i < m_meshHermes->ordView().get_num_triangles(); i++)
    {
        if (vert[tris[i][0]][2] < min) min = vert[tris[i][0]][2];
        if (vert[tris[i][0]][2] > max) max = vert[tris[i][0]][2];
    }

    m_meshHermes->ordView().unlock_data();

    // order color map
    loadProjectionViewPort();

    glScaled(2.0 / width(), 2.0 / height(), 1.0);
    glTranslated(- width() / 2.0, -height() / 2.0, 0.0);

    // dimensions
    int textWidth = fontMetrics().width("00");
    int textHeight = fontMetrics().height();
    Point scaleSize = Point(20 + 3 * textWidth, (20 + max * (2 * textHeight) - textHeight / 2.0 + 2));
    Point scaleBorder = Point(10.0, (Util::config()->showRulers) ? - (m_rulersAreaWidth.y/4.0)*m_scale2d*height() + 20.0 : 10.0);
    double scaleLeft = (width() - (20 + 3 * textWidth));

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
        glVertex2d(scaleLeft + 10,                                 scaleBorder.y + 10 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + 3 * textWidth - scaleBorder.x, scaleBorder.y + 10 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + 3 * textWidth - scaleBorder.x, scaleBorder.y + 12 + (i )*(2 * textHeight) - textHeight / 2.0);
        glVertex2d(scaleLeft + 10,                                 scaleBorder.y + 12 + (i )*(2 * textHeight) - textHeight / 2.0);

        glColor3d(paletteColorOrder(i)[0], paletteColorOrder(i)[1], paletteColorOrder(i)[2]);
        glVertex2d(scaleLeft + 12,                                     scaleBorder.y + 12 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + 3 * textWidth - 2 - scaleBorder.x, scaleBorder.y + 12 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + 3 * textWidth - 2 - scaleBorder.x, scaleBorder.y + 10 + (i  )*(2 * textHeight) - textHeight / 2.0);
        glVertex2d(scaleLeft + 12,                                     scaleBorder.y + 10 + (i  )*(2 * textHeight) - textHeight / 2.0);
    }
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    // labels
    glColor3d(1.0, 1.0, 1.0);
    for (int i = 1; i < max + 1; i++)
    {
        int sizeNumber = fontMetrics().width(QString::number(i));
        renderText(scaleLeft + 10 + 1.5 * textWidth - sizeNumber,
                   scaleBorder.y + 10.0 + (i-1)*(2.0 * textHeight) + textHeight / 2.0,
                   0.0,
                   QString::number(i));
    }
}
