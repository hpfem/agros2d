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

#include "localvalueview.h"
#include "scene.h"
#include "gui.h"
#include "scenemarker.h"
#include "hermes2d.h"

LocalPointValue::LocalPointValue(Point &point)
{
    logMessage("LocalPointValue::LocalPointValue()");

    this->point = point;

    PointValue val = pointValue(Util::scene()->sceneSolution()->sln(), point);

    value = val.value;
    derivative = val.derivative;
    labelMarker = val.marker;
}

PointValue LocalPointValue::pointValue(Solution *sln, Point &point)
{
    logMessage("LocalPointValue::pointValue()");

    double tmpValue;
    Point tmpDerivative;
    SceneLabelMarker *tmpLabelMarker = NULL;

    if (sln)
    {
        int index = Util::scene()->sceneSolution()->findTriangleInMesh(Util::scene()->sceneSolution()->meshInitial(), point);
        if (index != -1)
        {
            if ((Util::scene()->problemInfo()->analysisType == AnalysisType_Transient) &&
                Util::scene()->sceneSolution()->timeStep() == 0)
                // const solution at first time step
                tmpValue = Util::scene()->problemInfo()->initialCondition.number;
            else                
                tmpValue = sln->get_pt_value(point.x, point.y, H2D_FN_VAL_0);

            tmpDerivative.x =  sln->get_pt_value(point.x, point.y, H2D_FN_DX_0);
            tmpDerivative.y =  sln->get_pt_value(point.x, point.y, H2D_FN_DY_0);

            // find marker
            Element *element = Util::scene()->sceneSolution()->meshInitial()->get_element_fast(index);
            tmpLabelMarker = Util::scene()->labels[element->marker]->marker;
        }
    }

    return PointValue(tmpValue, tmpDerivative, tmpLabelMarker);
}

// *************************************************************************************************************************************

LocalPointValueView::LocalPointValueView(QWidget *parent): QDockWidget(tr("Local Values"), parent)
{
    logMessage("LocalPointValueView::LocalPointValueView()");

    QSettings settings;
    
    setMinimumWidth(280);
    setObjectName("LocalPointValueView");

    createActions();
    createMenu();

    trvWidget = new QTreeWidget();
    trvWidget->setHeaderHidden(false);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(3);
    trvWidget->setColumnWidth(0, settings.value("LocalPointValueView/TreeViewColumn0", 150).value<int>());
    trvWidget->setColumnWidth(1, settings.value("LocalPointValueView/TreeViewColumn1", 80).value<int>());
    trvWidget->setIndentation(12);

    QStringList labels;
    labels << tr("Label") << tr("Value") << tr("Unit");
    trvWidget->setHeaderLabels(labels);

    connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));

    QPushButton *btnPoint = new QPushButton();
    btnPoint->setText(actPoint->text());
    btnPoint->setIcon(actPoint->icon());
    btnPoint->setMaximumSize(btnPoint->sizeHint());
    connect(btnPoint, SIGNAL(clicked()), this, SLOT(doPoint()));

    // main widget
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(trvWidget);
    layout->addWidget(btnPoint);
    layout->setContentsMargins(0, 0, 0, 7);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);    
    
    setWidget(widget);
}

LocalPointValueView::~LocalPointValueView()
{
    logMessage("LocalPointValueView::~LocalPointValueView()");

    QSettings settings;
    settings.setValue("LocalPointValueView/TreeViewColumn0", trvWidget->columnWidth(0));
    settings.setValue("LocalPointValueView/TreeViewColumn1", trvWidget->columnWidth(1));
}

void LocalPointValueView::createActions()
{
    logMessage("LocalPointValueView::createActions()");

    // point
    actPoint = new QAction(icon("scene-node"), tr("Local point value"), this);
    connect(actPoint, SIGNAL(triggered()), this, SLOT(doPoint()));

    // copy value
    actCopy = new QAction(icon(""), tr("Copy value"), this);
    connect(actCopy, SIGNAL(triggered()), this, SLOT(doCopyValue()));
}

void LocalPointValueView::createMenu()
{
    logMessage("LocalPointValueView::createMenu()");

    mnuInfo = new QMenu(this);
    mnuInfo->addAction(actPoint);
    mnuInfo->addAction(actCopy);
}

void LocalPointValueView::doPoint()
{
    logMessage("LocalPointValueView::doPoint()");

    LocalPointValueDialog localPointValueDialog(point);
    if (localPointValueDialog.exec() == QDialog::Accepted)
    {
        doShowPoint(localPointValueDialog.point());
    }
}

void LocalPointValueView::doContextMenu(const QPoint &pos)
{
    logMessage("LocalPointValueView::doContextMenu()");

    actCopy->setEnabled(false);
    QTreeWidgetItem *item = trvWidget->itemAt(pos);
    if (item)
        if (!item->text(1).isEmpty())
        {
            trvWidget->setCurrentItem(item);
            actCopy->setEnabled(true);
        }

    mnuInfo->exec(QCursor::pos());
}

void LocalPointValueView::doShowPoint(const Point &point)
{
    logMessage("LocalPointValueView::doShowPoint()");

    // store point
    this->point = point;
    doShowPoint();
}

void LocalPointValueView::doCopyValue()
{
    logMessage("LocalPointValueView::doCopyValue()");

    QTreeWidgetItem *item = trvWidget->currentItem();
    if (item)
        if (!item->text(1).isEmpty())
            QApplication::clipboard()->setText(item->text(1));
}

void LocalPointValueView::doShowPoint()
{
    logMessage("LocalPointValueView::doShowPoint()");

    trvWidget->clear();

    // point
    QTreeWidgetItem *pointNode = new QTreeWidgetItem(trvWidget);
    pointNode->setText(0, tr("Point"));
    pointNode->setExpanded(true);

    addTreeWidgetItemValue(pointNode, Util::scene()->problemInfo()->labelX() + ":", QString("%1").arg(point.x, 0, 'f', 5), tr("m"));
    addTreeWidgetItemValue(pointNode, Util::scene()->problemInfo()->labelY() + ":", QString("%1").arg(point.y, 0, 'f', 5), tr("m"));

    trvWidget->insertTopLevelItem(0, pointNode);

    if (Util::scene()->sceneSolution()->isSolved())
        Util::scene()->problemInfo()->hermes()->showLocalValue(trvWidget, Util::scene()->problemInfo()->hermes()->localPointValue(point));

    trvWidget->resizeColumnToContents(2);
}

LocalPointValueDialog::LocalPointValueDialog(Point point, QWidget *parent) : QDialog(parent)
{
    logMessage("LocalPointValueDialog::LocalPointValueDialog()");

    setWindowIcon(icon("scene-node"));
    setWindowTitle(tr("Local point value"));

    setModal(true);

    txtPointX = new SLineEditValue();
    txtPointX->setNumber(point.x);
    txtPointY = new SLineEditValue();
    txtPointY->setNumber(point.y);

    connect(txtPointX, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtPointY, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    QFormLayout *layoutPoint = new QFormLayout();
    layoutPoint->addRow(Util::scene()->problemInfo()->labelX() + " (m):", txtPointX);
    layoutPoint->addRow(Util::scene()->problemInfo()->labelY() + " (m):", txtPointY);

    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutPoint);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

LocalPointValueDialog::~LocalPointValueDialog()
{
    logMessage("LocalPointValueDialog::~LocalPointValueDialog()");

    delete txtPointX;
    delete txtPointY;
}

Point LocalPointValueDialog::point()
{
    logMessage("LocalPointValueDialog::point()");

    return Point(txtPointX->value().number, txtPointY->value().number);
}

void LocalPointValueDialog::evaluated(bool isError)
{
    logMessage("LocalPointValueDialog::evaluated()");

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!isError);
}
