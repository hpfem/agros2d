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

LocalPointValue::LocalPointValue(Point &point)
{
    this->point = point;

    PointValue val = pointValue(Util::scene()->sceneSolution()->sln1(), point);

    value = val.value;
    derivative = val.derivative;
    labelMarker = val.marker;
}

PointValue LocalPointValue::pointValue(Solution *sln, Point &point)
{
    double tmpValue;
    Point tmpDerivative;
    SceneLabelMarker *tmpLabelMarker = NULL;

    if (Util::scene()->sceneSolution()->isSolved())
    {
        tmpValue = sln->get_pt_value(point.x, point.y, FN_VAL_0);
        if (Util::scene()->problemInfo()->physicField() != PHYSICFIELD_ELASTICITY)
        {
            tmpDerivative.x =  sln->get_pt_value(point.x, point.y, FN_DX_0);
            tmpDerivative.y =  sln->get_pt_value(point.x, point.y, FN_DY_0);
        }
    }

    // find marker
    int index = Util::scene()->sceneSolution()->findTriangleInMesh(Util::scene()->sceneSolution()->mesh(), point);
    if (index != -1)
    {
        Element *element = Util::scene()->sceneSolution()->mesh()->get_element_fast(index);
        tmpLabelMarker = Util::scene()->labels[element->marker]->marker;
    }

    return PointValue(tmpValue, tmpDerivative, tmpLabelMarker);
}

// *************************************************************************************************************************************

LocalPointValueView::LocalPointValueView(QWidget *parent): QDockWidget(tr("Local Values"), parent)
{
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
    trvWidget->setColumnWidth(0, settings.value("LocalPointValueView/TreeViewColumn0", 180).value<int>());
    trvWidget->setColumnWidth(1, settings.value("LocalPointValueView/TreeViewColumn1", 80).value<int>());
    trvWidget->setColumnWidth(2, settings.value("LocalPointValueView/TreeViewColumn2", 20).value<int>());
    trvWidget->setIndentation(12);

    QStringList labels;
    labels << tr("Label") << tr("Value") << tr("Unit");
    trvWidget->setHeaderLabels(labels);

    connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));

    QToolButton *btnPoint = new QToolButton();
    btnPoint->setText(actPoint->text());
    btnPoint->setIcon(actPoint->icon());
    btnPoint->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
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
    QSettings settings;
    settings.setValue("LocalPointValueView/TreeViewColumn0", trvWidget->columnWidth(0));
    settings.setValue("LocalPointValueView/TreeViewColumn1", trvWidget->columnWidth(1));
    settings.setValue("LocalPointValueView/TreeViewColumn2", trvWidget->columnWidth(2));
}

void LocalPointValueView::createActions()
{
    actPoint = new QAction(icon("scene-node"), tr("Local point value"), this);
    connect(actPoint, SIGNAL(triggered()), this, SLOT(doPoint()));
}

void LocalPointValueView::createMenu()
{
    mnuInfo = new QMenu(this);

    mnuInfo->addAction(actPoint);
}

void LocalPointValueView::doPoint()
{
    
    LocalPointValueDialog localPointValueDialog(point);
    if (localPointValueDialog.exec() == QDialog::Accepted)
    {
        doShowPoint(localPointValueDialog.point());
    }
}

void LocalPointValueView::doContextMenu(const QPoint &pos)
{
    mnuInfo->exec(QCursor::pos());
}

void LocalPointValueView::doShowPoint(const Point &point)
{
    // store point
    this->point = point;
    doShowPoint();
}

void LocalPointValueView::doShowPoint()
{
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
}

LocalPointValueDialog::LocalPointValueDialog(Point point, QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon("scene-node"));
    setWindowTitle(tr("Local point value"));

    setModal(true);

    txtPointX = new SLineEditDouble(point.x);
    txtPointY = new SLineEditDouble(point.y);

    QFormLayout *layoutPoint = new QFormLayout();
    layoutPoint->addRow(Util::scene()->problemInfo()->labelX() + " (m):", txtPointX);
    layoutPoint->addRow(Util::scene()->problemInfo()->labelY() + " (m):", txtPointY);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutPoint);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    resize(sizeHint());
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

LocalPointValueDialog::~LocalPointValueDialog()
{
    delete txtPointX;
    delete txtPointY;
}

Point LocalPointValueDialog::point()
{
    return Point(txtPointX->value(), txtPointY->value());
}
