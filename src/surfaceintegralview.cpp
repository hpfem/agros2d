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

#include "surfaceintegralview.h"

#include "gui.h"
#include "scene.h"

#include "hermes2d.h"

SurfaceIntegralValue::SurfaceIntegralValue()
{
    logMessage("SurfaceIntegralValue::SurfaceIntegralValue()");

    length = 0.0;
    surface = 0.0;    
}

void SurfaceIntegralValue::calculate()
{
    logMessage("SurfaceIntegralValue::calculate()");

    if (!Util::scene()->sceneSolution()->isSolved())
        return;

    Quad2D *quad = &g_quad_2d_std;
    Solution *sln = Util::scene()->sceneSolution()->sln();

    sln->set_quad_2d(quad);

    Mesh* mesh = sln->get_mesh();
    for (int i = 0; i<Util::scene()->edges.length(); i++)
    {
        SceneEdge *sceneEdge = Util::scene()->edges[i];
        if (sceneEdge->isSelected)
        {
            for_all_active_elements(e, mesh)
            {
                for (unsigned edge = 0; edge < e->nvert; edge++)
                {
                    bool integrate = false;
                    boundary = false;

                    if (e->en[edge]->bnd && e->en[edge]->marker-1 == i)
                    {
                        // boundary
                        integrate = true;
                        boundary = true;
                    }
                    else
                    {
                        // inner edge
                        Node *node1 = mesh->get_node(e->en[edge]->p1);
                        Node *node2 = mesh->get_node(e->en[edge]->p2);

                        if ((sceneEdge->distance(Point(node1->x, node1->y)) < EPS_ZERO) &&
                            (sceneEdge->distance(Point(node2->x, node2->y)) < EPS_ZERO))
                        {
                            integrate = true;
                        }
                    }

                    // integral
                    if (integrate)
                    {
                        update_limit_table(e->get_mode());

                        sln->set_active_element(e);
                        RefMap* ru = sln->get_refmap();

                        Quad2D* quad2d = ru->get_quad_2d();
                        int eo = quad2d->get_edge_points(edge);
                        sln->set_quad_order(eo, H2D_FN_VAL | H2D_FN_DX | H2D_FN_DY);
                        pt = quad2d->get_points(eo);
                        tan = ru->get_tangent(edge);

                        // value
                        value = sln->get_fn_values();
                        // derivative
                        sln->get_dx_dy_values(dudx, dudy);
                        // x - coordinate
                        x = ru->get_phys_x(eo);

                        for (int i = 0; i < quad2d->get_num_points(eo); i++)
                        {
                            // length
                            if (boundary)
                                length += pt[i][2] * tan[i][2] / 2.0;
                            else
                                length += pt[i][2] * tan[i][2] / 4.0;

                            // surface
                            if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
                            {
                                if (boundary)
                                    surface += pt[i][2] * tan[i][2] / 2.0;
                                else
                                    surface += pt[i][2] * tan[i][2] / 4.0;
                            }
                            else
                            {
                                if (boundary)
                                    surface += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] / 2.0;
                                else
                                    surface += 2 * M_PI * x[i] * pt[i][2] * tan[i][2] / 4.0;
                            }

                            // other integrals
                            calculateVariables(i);
                        }
                    }
                }
            }
        }
    }
}

SurfaceIntegralValueView::SurfaceIntegralValueView(QWidget *parent): QDockWidget(tr("Surface Integral"), parent)
{
    logMessage("SurfaceIntegralValueView::SurfaceIntegralValueView()");

    QSettings settings;

    setMinimumWidth(280);
    setObjectName("SurfaceIntegralValueView");

    createActions();
    createMenu();

    trvWidget = new QTreeWidget();
    trvWidget->setHeaderHidden(false);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(3);
    trvWidget->setColumnWidth(0, settings.value("SurfaceIntegralValueView/TreeViewColumn0", 150).value<int>());
    trvWidget->setColumnWidth(1, settings.value("SurfaceIntegralValueView/TreeViewColumn1", 80).value<int>());
    trvWidget->setIndentation(12);

    connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));

    QStringList labels;
    labels << tr("Label") << tr("Value") << tr("Unit");
    trvWidget->setHeaderLabels(labels);

    setWidget(trvWidget);
}

SurfaceIntegralValueView::~SurfaceIntegralValueView()
{
    logMessage("SurfaceIntegralValueView::~SurfaceIntegralValueView()");

    QSettings settings;
    settings.setValue("SurfaceIntegralValueView/TreeViewColumn0", trvWidget->columnWidth(0));
    settings.setValue("SurfaceIntegralValueView/TreeViewColumn1", trvWidget->columnWidth(1));
}

void SurfaceIntegralValueView::createActions()
{
    logMessage("SurfaceIntegralValueView::createActions()");

    // copy value
    actCopy = new QAction(icon(""), tr("Copy value"), this);
    connect(actCopy, SIGNAL(triggered()), this, SLOT(doCopyValue()));
}

void SurfaceIntegralValueView::createMenu()
{
    logMessage("SurfaceIntegralValueView::createMenu()");

    mnuInfo = new QMenu(this);
    mnuInfo->addAction(actCopy);
}

void SurfaceIntegralValueView::doCopyValue()
{
    logMessage("SurfaceIntegralValueView::doCopyValue()");

    QTreeWidgetItem *item = trvWidget->currentItem();
    if (item)
        QApplication::clipboard()->setText(item->text(1));
}

void SurfaceIntegralValueView::doContextMenu(const QPoint &pos)
{
    logMessage("SurfaceIntegralValueView::doContextMenu()");

    QTreeWidgetItem *item = trvWidget->itemAt(pos);
    if (item)
        if (!item->text(1).isEmpty())
        {
            trvWidget->setCurrentItem(item);
            mnuInfo->exec(QCursor::pos());
        }
}

void SurfaceIntegralValueView::doShowSurfaceIntegral()
{
    logMessage("SurfaceIntegralValueView::doShowSurfaceIntegral()");

    SurfaceIntegralValue *surfaceIntegralValue = Util::scene()->problemInfo()->hermes()->surfaceIntegralValue();
    trvWidget->clear();

    // point
    QTreeWidgetItem *pointGeometry = new QTreeWidgetItem(trvWidget);
    pointGeometry->setText(0, tr("Geometry"));
    pointGeometry->setExpanded(true);

    addTreeWidgetItemValue(pointGeometry, tr("Length:"), tr("%1").arg(surfaceIntegralValue->length, 0, 'e', 3), tr("m"));
    addTreeWidgetItemValue(pointGeometry, tr("Surface:"), tr("%1").arg(surfaceIntegralValue->surface, 0, 'e', 3), tr("m2"));

    trvWidget->insertTopLevelItem(0, pointGeometry);

    if (Util::scene()->sceneSolution()->isSolved())
        Util::scene()->problemInfo()->hermes()->showSurfaceIntegralValue(trvWidget, surfaceIntegralValue);

    delete surfaceIntegralValue;

    trvWidget->resizeColumnToContents(2);
}
