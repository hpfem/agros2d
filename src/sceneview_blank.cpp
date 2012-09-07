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

#include "sceneview_blank.h"
#include "sceneview_data.h"
#include "scene.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"

SceneViewBlank::SceneViewBlank(QWidget *parent)
    : SceneViewCommon2D(NULL, parent)
{
    connect(Util::problem(), SIGNAL(meshed()), this, SLOT(refresh()));
    connect(Util::problem(), SIGNAL(solved()), this, SLOT(refresh()));
}

SceneViewBlank::~SceneViewBlank()
{
}


void SceneViewBlank::refresh()
{
    doZoomBestFit();

    SceneViewCommon::refresh();
}

void SceneViewBlank::paintGL()
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

    // geometry
    paintGeometry();

    // rulers
    if (Util::config()->showRulers)
    {
        paintRulers();
        paintRulersHints();
    }

    // axes
    if (Util::config()->showAxes) paintAxes();
}

void SceneViewBlank::paintGeometry()
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
