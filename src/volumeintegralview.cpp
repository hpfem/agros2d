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
    logMessage("VolumeIntegralValue::VolumeIntegralValue()");
}

void VolumeIntegralValue::calculate()
{
    logMessage("VolumeIntegralValue::calculate()");

    if (!Util::scene()->sceneSolution()->isSolved())
        return;

    double px, py;
    double pvalue, pdx, pdy;

    for (Hermes::vector<Hermes::Module::Integral *>::iterator it = Util::scene()->problemInfo()->module->volume_integral.begin();
         it < Util::scene()->problemInfo()->module->volume_integral.end(); ++it )
    {
        mu::Parser *pars = new mu::Parser();

        pars->SetExpr(((Hermes::Module::Integral *) *it)->expression.scalar);

        pars->DefineConst("EPS0", EPS0);
        pars->DefineConst("MU0", MU0);
        pars->DefineConst("PI", M_PI);

        pars->DefineVar("x", &px);
        pars->DefineVar("y", &py);
        pars->DefineVar("value", &pvalue);
        pars->DefineVar("dx", &pdx);
        pars->DefineVar("dy", &pdy);

        parser.push_back(pars);

        values[*it] = 0.0;
    }

    initSolutions();

    Quad2D *quad = &g_quad_2d_std;

    sln1->set_quad_2d(quad);

    Mesh *mesh = sln1->get_mesh();
    Element *e;

    double result;

    SceneMaterial *material;

    for (int i = 0; i<Util::scene()->labels.length(); i++)
    {
        if (Util::scene()->labels[i]->isSelected)
        {
            material = Util::scene()->labels[i]->material;
            prepareParser(material);

            for_all_active_elements(e, mesh)
            {
                if (mesh->get_element_markers_conversion().get_user_marker(e->marker) == QString::number(i).toStdString())
                {
                    update_limit_table(e->get_mode());

                    sln1->set_active_element(e);
                    if (sln2)
                        sln2->set_active_element(e);

                    RefMap *ru = sln1->get_refmap();

                    int o;
                    if (!sln2)
                        o = sln1->get_fn_order() + ru->get_inv_ref_order();
                    else
                        o = sln1->get_fn_order() + sln2->get_fn_order() + ru->get_inv_ref_order();

                    limit_order(o);

                    // solution 1
                    double *value1, *dudx1, *dudy1;
                    sln1->set_quad_order(o, H2D_FN_VAL | H2D_FN_DX | H2D_FN_DY);
                    // value
                    value1 = sln1->get_fn_values();
                    // derivative
                    sln1->get_dx_dy_values(dudx1, dudy1);
                    // coordinates
                    double *x = ru->get_phys_x(o);
                    double *y = ru->get_phys_y(o);

                    // solution 2
                    double *value2, *dudx2, *dudy2;
                    if (sln2)
                    {
                        sln2->set_quad_order(o, H2D_FN_VAL | H2D_FN_DX | H2D_FN_DY);
                        // value
                        value2 = sln2->get_fn_values();
                        // derivative
                        sln2->get_dx_dy_values(dudx2, dudy2);
                    }

                    update_limit_table(e->get_mode());

                    // parse expression
                    int n = 0;
                    for (Hermes::vector<Hermes::Module::Integral *>::iterator it = Util::scene()->problemInfo()->module->volume_integral.begin();
                         it < Util::scene()->problemInfo()->module->volume_integral.end(); ++it )
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
                                pvalue = value1[i];
                                pdx = dudx1[i];
                                pdy = dudy1[i];

                                if (ru->is_jacobian_const())
                                {
                                    result += pt[i][2] * ru->get_const_jacobian() * parser[n]->Eval();
                                }
                                else
                                {
                                    double* jac = ru->get_jacobian(o);
                                    result += pt[i][2] * jac[i] * parser[n]->Eval();
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

    // delete parser
    for (Hermes::vector<mu::Parser *>::iterator it = parser.begin(); it < parser.end(); ++it)
        delete *it;
    parser.clear();
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

    if (Util::scene()->problemInfo()->module)
    {
        if (Util::scene()->sceneSolution()->isSolved())
        {
            VolumeIntegralValue *volumeIntegralValue = Util::scene()->problemInfo()->hermes()->volumeIntegralValue();

            QTreeWidgetItem *fieldNode = new QTreeWidgetItem(trvWidget);
            fieldNode->setText(0, QString::fromStdString(Util::scene()->problemInfo()->module->name));
            fieldNode->setExpanded(true);

            for (std::map<Hermes::Module::Integral *, double>::iterator it = volumeIntegralValue->values.begin(); it != volumeIntegralValue->values.end(); ++it)
            {
                addTreeWidgetItemValue(fieldNode, QString::fromStdString(it->first->name), QString("%1").arg(it->second, 0, 'e', 3), QString::fromStdString(it->first->unit));
            }

            delete volumeIntegralValue;
        }
    }

    trvWidget->resizeColumnToContents(2);
}
