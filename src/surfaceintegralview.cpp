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
#include "scene.h"

SurfaceIntegralValue::SurfaceIntegralValue()
{
    length = 0;
    surface = 0;
    for (int i = 0; i<Util::scene()->edges.length(); i++)
    {
        if (Util::scene()->edges[i]->isSelected)
        {
            length += Util::scene()->sceneSolution()->surfaceIntegral(i, PHYSICFIELDINTEGRAL_SURFACE_LENGTH);
            surface += Util::scene()->sceneSolution()->surfaceIntegral(i, PHYSICFIELDINTEGRAL_SURFACE_SURFACE);
        }
    }
}

SurfaceIntegralValueView::SurfaceIntegralValueView(QWidget *parent): QDockWidget(tr("Surface Integral"), parent)
{
    QSettings settings;

    setMinimumWidth(280);
    setObjectName("SurfaceIntegralValueView");

    trvWidget = new QTreeWidget();
    trvWidget->setHeaderHidden(false);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(3);
    trvWidget->setColumnWidth(0, settings.value("SurfaceIntegralValueView/TreeViewColumn0", 150).value<int>());
    trvWidget->setColumnWidth(1, settings.value("SurfaceIntegralValueView/TreeViewColumn1", 80).value<int>());
    trvWidget->setColumnWidth(2, settings.value("SurfaceIntegralValueView/TreeViewColumn2", 20).value<int>());
    trvWidget->setIndentation(12);


    QStringList labels;
    labels << tr("Label") << tr("Value") << tr("Unit");
    trvWidget->setHeaderLabels(labels);

    setWidget(trvWidget);
}

SurfaceIntegralValueView::~SurfaceIntegralValueView()
{
    QSettings settings;
    settings.setValue("SurfaceIntegralValueView/TreeViewColumn0", trvWidget->columnWidth(0));
    settings.setValue("SurfaceIntegralValueView/TreeViewColumn1", trvWidget->columnWidth(1));
    settings.setValue("SurfaceIntegralValueView/TreeViewColumn2", trvWidget->columnWidth(2));
}

void SurfaceIntegralValueView::doShowSurfaceIntegral()
{
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
}
