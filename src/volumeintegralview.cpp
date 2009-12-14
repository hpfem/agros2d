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
    for (int i = 0; i<Util::scene()->labels.length(); i++)
    {
        if (Util::scene()->labels[i]->isSelected)
        {
            crossSection += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_CROSSSECTION);
            volume += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_VOLUME);
        }
    }
}

VolumeIntegralValueView::VolumeIntegralValueView(QWidget *parent): QDockWidget(tr("Volume Integral"), parent)
{
    QSettings settings;

    setMinimumWidth(280);
    setObjectName("VolumeIntegralValueView");

    trvWidget = new QTreeWidget();
    trvWidget->setHeaderHidden(false);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(3);
    trvWidget->setColumnWidth(0, settings.value("VolumeIntegralValueView/TreeViewColumn0", 150).value<int>());
    trvWidget->setColumnWidth(1, settings.value("VolumeIntegralValueView/TreeViewColumn1", 80).value<int>());
    trvWidget->setColumnWidth(2, settings.value("VolumeIntegralValueView/TreeViewColumn2", 20).value<int>());
    trvWidget->setIndentation(12);

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
    settings.setValue("VolumeIntegralValueView/TreeViewColumn2", trvWidget->columnWidth(2));
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
}
