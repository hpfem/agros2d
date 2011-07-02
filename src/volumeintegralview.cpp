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
#include "gui.h"

VolumeIntegralValueView::VolumeIntegralValueView(QWidget *parent): QDockWidget(tr("Volume Integral"), parent)
{
    logMessage("VolumeIntegralValue::VolumeIntegralValueView()");

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
    logMessage("VolumeIntegralValue::~VolumeIntegralValueView()");

    QSettings settings;
    settings.setValue("VolumeIntegralValueView/TreeViewColumn0", trvWidget->columnWidth(0));
    settings.setValue("VolumeIntegralValueView/TreeViewColumn1", trvWidget->columnWidth(1));
}

void VolumeIntegralValueView::createActions()
{
    logMessage("VolumeIntegralValue::createActions()");

    // copy value
    actCopy = new QAction(icon(""), tr("Copy value"), this);
    connect(actCopy, SIGNAL(triggered()), this, SLOT(doCopyValue()));
}

void VolumeIntegralValueView::createMenu()
{
    logMessage("VolumeIntegralValue::createMenu()");

    mnuInfo = new QMenu(this);
    mnuInfo->addAction(actCopy);
}

void VolumeIntegralValueView::doCopyValue()
{
    logMessage("VolumeIntegralValue::doCopyValue()");

    QTreeWidgetItem *item = trvWidget->currentItem();
    if (item)
        QApplication::clipboard()->setText(item->text(1));
}

void VolumeIntegralValueView::doContextMenu(const QPoint &pos)
{
    logMessage("VolumeIntegralValue::doContextMenu()");

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
    logMessage("VolumeIntegralValue::doShowVolumeIntegral()");

    trvWidget->clear();

    if (Util::scene()->sceneSolution()->isSolved())
    {
        VolumeIntegralValue *volumeIntegralValue = Util::scene()->problemInfo()->module()->volume_integral_value();

        QTreeWidgetItem *fieldNode = new QTreeWidgetItem(trvWidget);
        fieldNode->setText(0, QString::fromStdString(Util::scene()->problemInfo()->module()->name));
        fieldNode->setExpanded(true);

        for (std::map<Hermes::Module::Integral *, double>::iterator it = volumeIntegralValue->values.begin(); it != volumeIntegralValue->values.end(); ++it)
            addTreeWidgetItemValue(fieldNode, QString::fromStdString(it->first->name), QString("%1").arg(it->second, 0, 'e', 3), QString::fromStdString(it->first->unit));

        delete volumeIntegralValue;
    }

    trvWidget->resizeColumnToContents(2);
}
