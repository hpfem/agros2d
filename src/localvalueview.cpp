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

LocalPointValue::LocalPointValue(const Point &point) : point(point)
{
    logMessage("LocalPointValue::LocalPointValue()");
}

void LocalPointValue::calculate()
{
    values.clear();

    this->point = point;
    if (Util::scene()->sceneSolution()->isSolved() &&
            Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        Util::scene()->problemInfo()->hermes()->updateTimeFunctions(Util::scene()->sceneSolution()->time());

    Solution *sln = Util::scene()->sceneSolution()->sln();

    if (sln)
    {
        int index = Util::scene()->sceneSolution()->findElementInMesh(Util::scene()->sceneSolution()->meshInitial(), point);
        if (index != -1)
        {
            double value;
            if ((Util::scene()->problemInfo()->analysisType == AnalysisType_Transient) &&
                    Util::scene()->sceneSolution()->timeStep() == 0)
                // const solution at first time step
                value = Util::scene()->problemInfo()->initialCondition.number;
            else
                value = sln->get_pt_value(point.x, point.y, H2D_FN_VAL_0);

            Point derivative;
            derivative.x = sln->get_pt_value(point.x, point.y, H2D_FN_DX_0);
            derivative.y = sln->get_pt_value(point.x, point.y, H2D_FN_DY_0);

            // find marker
            Element *e = Util::scene()->sceneSolution()->meshInitial()->get_element_fast(index);
            SceneMaterial *tmpMaterial = Util::scene()->labels[atoi(Util::scene()->sceneSolution()->meshInitial()->get_element_markers_conversion().get_user_marker(e->marker).c_str())]->material;

            // parser variables
            mu::Parser parser;

            parser.DefineConst("EPS0", EPS0);
            parser.DefineConst("MU0", MU0);

            double px, py;
            double pvalue, pdx, pdy;

            parser.DefineVar("x", &px);
            parser.DefineVar("y", &py);
            parser.DefineVar("value", &pvalue);
            parser.DefineVar("dx", &pdx);
            parser.DefineVar("dy", &pdy);

            // set variables
            px = point.x;
            py = point.y;
            pvalue = value;
            pdx = derivative.x;
            pdy = derivative.y;

            prepareParser(tmpMaterial, &parser);

            // parse expression
            for (Hermes::vector<Hermes::Module::PhysicFieldVariable *>::iterator it = Util::scene()->problemInfo()->module->local_variables.begin();
                 it < Util::scene()->problemInfo()->module->local_variables.end(); ++it )
            {
                try
                {
                    PointValue pointValue;
                    if (((Hermes::Module::PhysicFieldVariable *) *it)->is_scalar)
                    {
                        parser.SetExpr(((Hermes::Module::PhysicFieldVariable *) *it)->expression.scalar);
                        pointValue.scalar = parser.Eval();
                    }
                    else
                    {
                        parser.SetExpr(((Hermes::Module::PhysicFieldVariable *) *it)->expression.comp_x);
                        pointValue.vector.x = parser.Eval();
                        parser.SetExpr(((Hermes::Module::PhysicFieldVariable *) *it)->expression.comp_y);
                        pointValue.vector.y = parser.Eval();
                    }
                    values[((Hermes::Module::PhysicFieldVariable *) *it)] = pointValue;

                }
                catch (mu::Parser::exception_type &e)
                {
                    std::cout << e.GetMsg() << endl;
                }
            }
        }
    }
}

void LocalPointValue::prepareParser(SceneMaterial *material, mu::Parser *parser)
{
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

    if (Util::scene()->problemInfo()->module)
    {
        if (Util::scene()->sceneSolution()->isSolved())
        {
            QTreeWidgetItem *fieldNode = new QTreeWidgetItem(trvWidget);
            fieldNode->setText(0, QString::fromStdString(Util::scene()->problemInfo()->module->name));
            fieldNode->setExpanded(true);

            trvWidget->insertTopLevelItem(0, pointNode);

            LocalPointValue *value = Util::scene()->problemInfo()->hermes()->localPointValue(point);

            for (std::map<Hermes::Module::PhysicFieldVariable *, PointValue>::iterator it = value->values.begin(); it != value->values.end(); ++it)
            {
                if (it->first->is_scalar)
                {
                    // scalar variable
                    addTreeWidgetItemValue(fieldNode, QString::fromStdString(it->first->name), QString("%1").arg(it->second.scalar, 0, 'e', 3), QString::fromStdString(it->first->unit));
                }
                else
                {
                    // vector variable
                    QTreeWidgetItem *itemVector = new QTreeWidgetItem(fieldNode);
                    itemVector->setText(0, QString::fromStdString(it->first->name));
                    itemVector->setExpanded(true);

                    addTreeWidgetItemValue(itemVector, QString::fromStdString(it->first->shortname) + Util::scene()->problemInfo()->labelX().toLower() + ":", QString("%1").arg(it->second.vector.x, 0, 'e', 3), QString::fromStdString(it->first->unit));
                    addTreeWidgetItemValue(itemVector, QString::fromStdString(it->first->shortname) + Util::scene()->problemInfo()->labelY().toLower() + ":", QString("%1").arg(it->second.vector.y, 0, 'e', 3), QString::fromStdString(it->first->unit));
                    addTreeWidgetItemValue(itemVector, QString::fromStdString(it->first->shortname) + ": ", QString("%1").arg(it->second.vector.magnitude(), 0, 'e', 3), QString::fromStdString(it->first->unit));
                }
            }

            delete value;
        }
    }

    trvWidget->resizeColumnToContents(2);
}

LocalPointValueDialog::LocalPointValueDialog(Point point, QWidget *parent) : QDialog(parent)
{
    logMessage("LocalPointValueDialog::LocalPointValueDialog()");

    setWindowIcon(icon("scene-node"));
    setWindowTitle(tr("Local point value"));

    setModal(true);

    txtPointX = new ValueLineEdit();
    txtPointX->setNumber(point.x);
    txtPointY = new ValueLineEdit();
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
