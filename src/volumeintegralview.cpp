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

#include "volumeintegralview.h"
#include "scene.h"

VolumeIntegralValue::VolumeIntegralValue()
{
    crossSection = 0;
    volume = 0;
}

void VolumeIntegralValue::calculate()
{
    if (!Util::scene()->sceneSolution()->isSolved())
        return;

    quad = &g_quad_2d_std;

    initSolutions();

    sln1->set_quad_2d(quad);

    Mesh *mesh = sln1->get_mesh();

    for (int i = 0; i<Util::scene()->labels.length(); i++)
    {
        if (Util::scene()->labels[i]->isSelected)
        {
            for_all_active_elements(e, mesh)
            {
                if (e->marker == i)
                {
                    update_limit_table(e->get_mode());

                    sln1->set_active_element(e);
                    if (sln2)
                        sln2->set_active_element(e);

                    ru = sln1->get_refmap();

                    if (!sln2)
                        o = sln1->get_fn_order() + ru->get_inv_ref_order();
                    else
                        o = sln1->get_fn_order() + sln2->get_fn_order() + ru->get_inv_ref_order();

                    limit_order(o);

                    // solution 1
                    sln1->set_quad_order(o, FN_VAL | FN_DX | FN_DY);
                    // value
                    value1 = sln1->get_fn_values();
                    // derivative
                    sln1->get_dx_dy_values(dudx1, dudy1);
                    // coordinates
                    x = ru->get_phys_x(o);
                    y = ru->get_phys_y(o);

                    // solution 2
                    if (sln2)
                    {
                        sln2->set_quad_order(o, FN_VAL | FN_DX | FN_DY);
                        // value
                        value2 = sln2->get_fn_values();
                        // derivative
                        sln2->get_dx_dy_values(dudx2, dudy2);
                    }

                    update_limit_table(e->get_mode());

                    // cross section
                    result = 0.0;
                    h1_integrate_expression(1);
                    crossSection += result;

                    // volume
                    result = 0.0;
                    if (Util::scene()->problemInfo()->problemType == ProblemType_Planar)
                    {
                        h1_integrate_expression(1);
                    }
                    else
                    {
                        h1_integrate_expression(2 * M_PI * x[i]);
                    }
                    volume += result;

                    // other integrals
                    calculateVariables(i);
                }
            }
        }
    }
}

VolumeIntegralValueView::VolumeIntegralValueView(QWidget *parent): QDockWidget(tr("Volume Integral"), parent)
{
    QSettings settings;

    setMinimumWidth(280);
    setObjectName("VolumeIntegralValueView");

    createActions();
    createMenu();

    trvWidget = new QTreeWidget();
    trvWidget->setHeaderHidden(false);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(3);
    trvWidget->setColumnWidth(0, settings.value("VolumeIntegralValueView/TreeViewColumn0", 150).value<int>());
    trvWidget->setColumnWidth(1, settings.value("VolumeIntegralValueView/TreeViewColumn1", 80).value<int>());
    trvWidget->setIndentation(12);

    connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));

    QStringList labels;
    labels << tr("Label") << tr("Value") << tr("Unit");
    trvWidget->setHeaderLabels(labels);

    setWidget(trvWidget);
}

VolumeIntegralValueView::~VolumeIntegralValueView()
{
    QSettings settings;
    settings.setValue("VolumeIntegralValueView/TreeViewColumn0", trvWidget->columnWidth(0));
    settings.setValue("VolumeIntegralValueView/TreeViewColumn1", trvWidget->columnWidth(1));
}

void VolumeIntegralValueView::createActions()
{
    // copy value
    actCopy = new QAction(icon(""), tr("Copy value"), this);
    connect(actCopy, SIGNAL(triggered()), this, SLOT(doCopyValue()));
}

void VolumeIntegralValueView::createMenu()
{
    mnuInfo = new QMenu(this);
    mnuInfo->addAction(actCopy);
}

void VolumeIntegralValueView::doCopyValue()
{
    QTreeWidgetItem *item = trvWidget->currentItem();
    if (item)
        QApplication::clipboard()->setText(item->text(1));
}

void VolumeIntegralValueView::doContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = trvWidget->itemAt(pos);
    if (item)
        if (!item->text(1).isEmpty())
        {
            trvWidget->setCurrentItem(item);
            mnuInfo->exec(QCursor::pos());
        }
}

void VolumeIntegralValueView::doShowVolumeIntegral()
{
    VolumeIntegralValue *volumeIntegralValue = Util::scene()->problemInfo()->hermes()->volumeIntegralValue();

    trvWidget->clear();

    // point
    QTreeWidgetItem *pointGeometry = new QTreeWidgetItem(trvWidget);
    pointGeometry->setText(0, tr("Geometry"));
    pointGeometry->setExpanded(true);

    addTreeWidgetItemValue(pointGeometry, tr("Volume:"), tr("%1").arg(volumeIntegralValue->volume, 0, 'e', 3), tr("m3"));
    addTreeWidgetItemValue(pointGeometry, tr("Cross section:"), tr("%1").arg(volumeIntegralValue->crossSection, 0, 'e', 3), tr("m2"));

    trvWidget->insertTopLevelItem(0, pointGeometry);

    if (Util::scene()->sceneSolution()->isSolved())
        Util::scene()->problemInfo()->hermes()->showVolumeIntegralValue(trvWidget, volumeIntegralValue);

    delete volumeIntegralValue;

    trvWidget->resizeColumnToContents(2);
}
