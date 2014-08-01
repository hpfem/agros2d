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

#include "optilab_multi.h"

OptilabMulti::OptilabMulti(OptilabWindow *parent) : QWidget(parent), optilabMain(parent)
{
    chart = new QCustomPlot(this);
    chart->setMinimumHeight(300);
    chart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    // main chart
    chart->addGraph();
    // chart->graph(0)->setLineStyle(QCPGraph::lsLine);
    // chart->graph(0)->setPen(QColor(50, 50, 50, 255));
    chart->graph(0)->setLineStyle(QCPGraph::lsNone);
    chart->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 6));
    connect(chart, SIGNAL(plottableClick(QCPAbstractPlottable*, QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*, QMouseEvent*)));
    // highlight
    chart->addGraph();
    chart->graph(1)->setPen(QColor(255, 50, 50, 255));
    chart->graph(1)->setLineStyle(QCPGraph::lsNone);
    chart->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 8));

    radChartLine = new QRadioButton(tr("Line"));
    radChartLine->setChecked(true);
    radChartXY = new QRadioButton("X/Y");

    QButtonGroup *chartGroup = new QButtonGroup();
    chartGroup->addButton(radChartLine);
    chartGroup->addButton(radChartXY);
    connect(chartGroup, SIGNAL(buttonClicked(int)), this, SLOT(refreshChartControls()));

    QVBoxLayout *layoutChartType = new QVBoxLayout();
    layoutChartType->addWidget(radChartLine);
    layoutChartType->addWidget(radChartXY);

    QGroupBox *grpChartType = new QGroupBox(tr("Chart type"));
    grpChartType->setLayout(layoutChartType);

    cmbX = new QComboBox(this);
    cmbY = new QComboBox(this);
    QPushButton *btnPlot = new QPushButton(tr("Plot"), this);
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(refreshChartWithAxes()));

    QHBoxLayout *layoutChartButtons = new QHBoxLayout();
    layoutChartButtons->addStretch();
    layoutChartButtons->addWidget(btnPlot);

    QGridLayout *layoutChartCombo = new QGridLayout();
    layoutChartCombo->setColumnStretch(1, 1);
    layoutChartCombo->addWidget(new QLabel(tr("X:")), 0, 0);
    layoutChartCombo->addWidget(cmbX, 0, 1);
    layoutChartCombo->addWidget(new QLabel(tr("Y:")), 1, 0);
    layoutChartCombo->addWidget(cmbY, 1, 1);

    QGroupBox *grpChartCombo = new QGroupBox(tr("Variables"));
    grpChartCombo->setLayout(layoutChartCombo);

    QGridLayout *layout = new QGridLayout();
    // layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(grpChartType, 0, 0);
    layout->addWidget(grpChartCombo, 0, 1);
    layout->addWidget(chart, 1, 0, 1, 2);
    layout->addLayout(layoutChartButtons, 2, 0, 1, 2);

    setLayout(layout);
}


void OptilabMulti::refreshChart()
{
    return;

    /*
    chart->graph(0)->clearData();
    chart->graph(1)->clearData();

    QVector<double> valuesY = outputVariables.values(chart->yAxis->label());

    if (radChartXY->isChecked())
    {
        // xy chart
        QVector<double> valuesX = outputVariables.values(chart->xAxis->label());

        chart->graph(0)->setData(valuesX, valuesY);

        // select current item
        if (trvVariants->currentItem())
        {
            int index = trvVariants->indexOfTopLevelItem(trvVariants->currentItem());
            /*
            QDomNode nodeResult = docXML.elementsByTagName("results").at(0).childNodes().at(index);
            QDomNode nodeSolution = nodeResult.toElement().elementsByTagName("solution").at(0);

            if (nodeSolution.toElement().attribute("solved").toInt() == 1)
            {
                double xv = outputVariables.value(index, chart->xAxis->label());
                double yv = outputVariables.value(index, chart->yAxis->label());

                QVector<double> x(0);
                x.append(xv);
                QVector<double> y(0);
                y.append(yv);

                // qDebug() << x << y;

                chart->graph(1)->setData(x, y);
            }

        }
    }
    else if (radChartLine->isChecked())
    {
        // line chart
        QVector<double> valuesX(valuesY.size());
        for (int i = 0; i < valuesY.size(); i++)
            valuesX[i] = i;

        chart->graph(0)->setData(valuesX, valuesY);

        // select current item
        if (trvVariants->currentItem())
        {
            int index = trvVariants->indexOfTopLevelItem(trvVariants->currentItem());
            /*
            QDomNode nodeResult = docXML.elementsByTagName("results").at(0).childNodes().at(index);
            QDomNode nodeSolution = nodeResult.toElement().elementsByTagName("solution").at(0);

            if (nodeSolution.toElement().attribute("solved").toInt() == 1)
            {
                double xv = outputVariables.variables().keys().indexOf(index);
                double yv = outputVariables.value(index, chart->yAxis->label());

                QVector<double> x(0);
                x.append(xv);
                QVector<double> y(0);
                y.append(yv);

                chart->graph(1)->setData(x, y);
            }

        }
    }

    chart->replot();
    */
}

void OptilabMulti::refreshChartWithAxes()
{
    // set chart variables
    QString selectedX = cmbX->currentText();
    QString selectedY = cmbY->currentText();

    cmbX->clear();
    cmbY->clear();

    /*
    foreach (QString name, outputVariables.names(true))
    {
        cmbX->addItem(name);
        cmbY->addItem(name);
    }
    */

    if (!selectedX.isEmpty())
        cmbX->setCurrentIndex(cmbX->findText(selectedX));
    if (!selectedY.isEmpty())
        cmbY->setCurrentIndex(cmbY->findText(selectedY));

    refreshChartControls();
    refreshChart();

    chart->rescaleAxes();
    chart->replot();
}

void OptilabMulti::refreshChartControls()
{
    // enable only for xy chart
    cmbX->setEnabled(radChartXY->isChecked());

    // xlabel
    if (radChartXY->isChecked())
        chart->xAxis->setLabel(cmbX->currentText());
    else if (radChartLine->isChecked())
        chart->xAxis->setLabel(tr("variant"));

    // ylabel
    chart->yAxis->setLabel(cmbY->currentText());
}

void OptilabMulti::graphClicked(QCPAbstractPlottable *plottable, QMouseEvent *event)
{
    /*
    double x = chart->xAxis->pixelToCoord(event->pos().x());
    double y = chart->yAxis->pixelToCoord(event->pos().y());

    int index = -1;

    // find closest point
    QVector<double> xvalues = outputVariables.values(chart->xAxis->label());
    QVector<double> yvalues = outputVariables.values(chart->yAxis->label());

    if (radChartXY->isChecked())
    {
        double dist = numeric_limits<double>::max();
        for (int i = 0; i < xvalues.size(); i++)
        {
            double mag = Point(xvalues[i] - x,
                               yvalues[i] - y).magnitudeSquared();
            if (mag < dist)
            {
                dist = mag;
                index = outputVariables.variables().keys().at(i);
            }
        }
    }
    else if (radChartLine->isChecked())
    {
        int ind = round(x);
        if (ind < 0) ind = 0;
        if (ind > outputVariables.size() - 1) ind = outputVariables.size();

        index = outputVariables.variables().keys().at(ind);
    }


    if (index != -1)
    {
        trvVariants->topLevelItem(index)->setSelected(true);
        trvVariants->setCurrentItem(trvVariants->topLevelItem(index));

        variantInfo(trvVariants->topLevelItem(index)->data(0, Qt::UserRole).toString());
    }
    */
}
