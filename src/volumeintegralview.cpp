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

#include "gui.h"
#include "scene.h"
#include "volumeintegralview.h"

#include "hermes2d.h"

VolumeIntegralValue::VolumeIntegralValue()
{
}

VolumeIntegralValue::~VolumeIntegralValue()
{
    delete parser;
}

void VolumeIntegralValue::initParser()
{
    for (Hermes::vector<Hermes::Module::Integral *>::iterator it = Util::scene()->problemInfo()->module()->volume_integral.begin();
         it < Util::scene()->problemInfo()->module()->volume_integral.end(); ++it )
    {
        mu::Parser *pars = Util::scene()->problemInfo()->module()->get_parser();

        pars->SetExpr(((Hermes::Module::Integral *) *it)->expression.scalar);

        parser->parser.push_back(pars);

        values[*it] = 0.0;
    }
}

void VolumeIntegralValue::calculate()
{
    logMessage("VolumeIntegralValue::calculate()");

    if (!Util::scene()->sceneSolution()->isSolved())
        return;

    double px;
    double py;
    double *pvalue = new double[Util::scene()->problemInfo()->module()->number_of_solution()];
    double *pdx = new double[Util::scene()->problemInfo()->module()->number_of_solution()];
    double *pdy = new double[Util::scene()->problemInfo()->module()->number_of_solution()];

    double **value = new double*[Util::scene()->problemInfo()->module()->number_of_solution()];
    double **dudx = new double*[Util::scene()->problemInfo()->module()->number_of_solution()];
    double **dudy = new double*[Util::scene()->problemInfo()->module()->number_of_solution()];

    for (Hermes::vector<mu::Parser *>::iterator it = parser->parser.begin(); it < parser->parser.end(); ++it )
    {
        ((mu::Parser *) *it)->DefineVar("x", &px);
        ((mu::Parser *) *it)->DefineVar("y", &py);

        for (int k = 0; k < Util::scene()->problemInfo()->module()->number_of_solution(); k++)
        {
            std::stringstream number;
            number << (k+1);

            ((mu::Parser *) *it)->DefineVar("value" + number.str(), &pvalue[k]);
            ((mu::Parser *) *it)->DefineVar("dx" + number.str(), &pdx[k]);
            ((mu::Parser *) *it)->DefineVar("dy" + number.str(), &pdy[k]);
        }
    }

    Quad2D *quad = &g_quad_2d_std;

    sln[0]->set_quad_2d(quad);

    Mesh *mesh = sln[0]->get_mesh();
    Element *e;

    for (int i = 0; i<Util::scene()->labels.length(); i++)
    {
        if (Util::scene()->labels[i]->isSelected)
        {
            SceneMaterial *material = Util::scene()->labels[i]->material;
            parser->setParserVariables(material);

            for_all_active_elements(e, mesh)
            {
                if (mesh->get_element_markers_conversion().get_user_marker(e->marker) == QString::number(i).toStdString())
                {
                    update_limit_table(e->get_mode());

                    for (int k = 0; k < Util::scene()->problemInfo()->module()->number_of_solution(); k++)
                        sln[k]->set_active_element(e);

                    RefMap *ru = sln[0]->get_refmap();

                    int o = 0;
                    for (int k = 0; k < Util::scene()->problemInfo()->module()->number_of_solution(); k++)
                        o += sln[k]->get_fn_order();
                    o += ru->get_inv_ref_order();

                    // coordinates
                    double *x = ru->get_phys_x(o);
                    double *y = ru->get_phys_y(o);

                    limit_order(o);

                    // solution
                    for (int k = 0; k < Util::scene()->problemInfo()->module()->number_of_solution(); k++)
                    {
                        sln[k]->set_quad_order(o, H2D_FN_VAL | H2D_FN_DX | H2D_FN_DY);
                        // value
                        value[k] = sln[k]->get_fn_values();
                        // derivative
                        sln[k]->get_dx_dy_values(dudx[k], dudy[k]);
                    }
                    update_limit_table(e->get_mode());

                    // parse expression
                    int n = 0;
                    for (Hermes::vector<Hermes::Module::Integral *>::iterator it = Util::scene()->problemInfo()->module()->volume_integral.begin();
                         it < Util::scene()->problemInfo()->module()->volume_integral.end(); ++it )
                    {
                        double result = 0.0;

                        try
                        {
                            double3* pt = quad->get_points(o);
                            int np = quad->get_num_points(o);

                            for (int i = 0; i < np; i++)
                            {
                                px = x[i];
                                py = y[i];

                                for (int k = 0; k < Util::scene()->problemInfo()->module()->number_of_solution(); k++)
                                {
                                    pvalue[k] = value[k][i];
                                    pdx[k] = dudx[k][i];
                                    pdy[k] = dudy[k][i];
                                }

                                if (ru->is_jacobian_const())
                                {
                                    result += pt[i][2] * ru->get_const_jacobian() * parser->parser[n]->Eval();
                                }
                                else
                                {
                                    double* jac = ru->get_jacobian(o);
                                    result += pt[i][2] * jac[i] * parser->parser[n]->Eval();
                                }
                            }

                            values[*it] += result;
                        }
                        catch (mu::Parser::exception_type &e)
                        {
                            std::cout << e.GetMsg() << endl;
                        }

                        n++;
                    }
                }
            }
        }
    }

    delete [] pvalue;
    delete [] pdx;
    delete [] pdy;

    delete [] value;
    delete [] dudx;
    delete [] dudy;
}

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
        {
            addTreeWidgetItemValue(fieldNode, QString::fromStdString(it->first->name), QString("%1").arg(it->second, 0, 'e', 3), QString::fromStdString(it->first->unit));
        }

        delete volumeIntegralValue;
    }

    trvWidget->resizeColumnToContents(2);
}
