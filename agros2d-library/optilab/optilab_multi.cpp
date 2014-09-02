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

#include <Python.h>

#include "optilab_multi.h"
#include "pythonlab/pythonengine.h"

OptilabMulti::OptilabMulti(OptilabWindow *parent) : QWidget(parent), optilabMain(parent)
{
    // XY
    chartXY = new QCustomPlot(this);
    chartXY->setMinimumHeight(300);
    chartXY->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    // main chart
    chartXY->addGraph();
    // chart->graph(0)->setLineStyle(QCPGraph::lsLine);
    // chart->graph(0)->setPen(QColor(50, 50, 50, 255));
    chartXY->graph(0)->setLineStyle(QCPGraph::lsNone);
    chartXY->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 6));
    //connect(chartXY, SIGNAL(plottableClick(QCPAbstractPlottable*, QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*, QMouseEvent*)));
    // highlight
    chartXY->addGraph();
    chartXY->graph(1)->setPen(QColor(255, 50, 50, 255));
    chartXY->graph(1)->setLineStyle(QCPGraph::lsNone);
    chartXY->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 8));

    cmbChartX = new QComboBox(this);
    cmbChartY = new QComboBox(this);

    QPushButton *btnPlotXY = new QPushButton(tr("Plot"), this);
    connect(btnPlotXY, SIGNAL(clicked()), this, SLOT(refreshChart()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch(1);
    layoutButtons->addWidget(btnPlotXY, 0, 0);

    QGridLayout *layoutChartXYControls = new QGridLayout();
    layoutChartXYControls->addWidget(new QLabel(tr("Variable X:")), 0, 0);
    layoutChartXYControls->addWidget(cmbChartX, 0, 1);
    layoutChartXYControls->addWidget(new QLabel(tr("Variable Y:")), 1, 0);
    layoutChartXYControls->addWidget(cmbChartY, 1, 1);
    layoutChartXYControls->addWidget(new QLabel(""), 19, 0);
    layoutChartXYControls->setRowStretch(19, 1);
    layoutChartXYControls->addLayout(layoutButtons, 20, 0, 1, 2);

    QHBoxLayout *layoutChartXY = new QHBoxLayout();
    layoutChartXY->addLayout(layoutChartXYControls);
    layoutChartXY->addWidget(chartXY);

    QWidget *widChartXY = new QWidget(this);
    widChartXY->setLayout(layoutChartXY);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widChartXY, 0, 0);

    setLayout(layout);
}

void OptilabMulti::refreshVariables()
{
    cmbChartX->clear();
    cmbChartY->clear();

    cmbChartX->addItem("index", "system.index");
    cmbChartY->addItem("index", "system.index");

    QString str = QString("agros2d_post_variables = variant.optilab_interface._optilab_mp.variable_keys(only_scalars = True)");
    currentPythonEngine()->runExpression(str);

    // extract values
    PyObject *result = PyDict_GetItemString(currentPythonEngine()->dict(), "agros2d_post_variables");
    if (result)
    {
        Py_INCREF(result);
        for (int i = 0; i < PyList_Size(result); i++)
        {
            PyObject *d = PyList_GetItem(result, i);
            Py_INCREF(d);

            QString name = QString::fromWCharArray(PyUnicode_AsUnicode(d));

            cmbChartX->addItem(name + " (var.)", "variable." + name);
            cmbChartY->addItem(name + " (var.)", "variable." + name);

            Py_XDECREF(d);
        }
        Py_XDECREF(result);
    }

    // remove variables
    currentPythonEngine()->runExpression("del agros2d_post_variables");

    str = QString("agros2d_post_parameters = variant.optilab_interface._optilab_mp.parameter_keys(only_scalars = True)");
    qDebug() << str;
    currentPythonEngine()->runExpression(str);

    // extract values
    result = PyDict_GetItemString(currentPythonEngine()->dict(), "agros2d_post_parameters");
    if (result)
    {
        Py_INCREF(result);
        for (int i = 0; i < PyList_Size(result); i++)
        {
            PyObject *d = PyList_GetItem(result, i);
            Py_INCREF(d);

            QString name = QString::fromWCharArray(PyUnicode_AsUnicode(d));

            cmbChartX->addItem(name + " (par.)", "parameter." + name);
            cmbChartY->addItem(name + " (par.)", "parameter." + name);

            Py_XDECREF(d);
        }
        Py_XDECREF(result);
    }

    // remove variables
    currentPythonEngine()->runExpression("del agros2d_post_parameters");
}

void OptilabMulti::refreshChart()
{
    QVector<double> valuesX;
    QVector<double> valuesY;

    QString typeX = cmbChartX->currentData().toString().split(".").at(0);
    QString keyX = cmbChartX->currentData().toString().split(".").at(1);
    QString strX;
    if (typeX == "parameter")
        strX = QString("agros2d_post_values_x = variant.optilab_interface._optilab_mp.parameter('%1')").arg(keyX);
    else if (typeX == "variable")
        strX = QString("agros2d_post_values_x = variant.optilab_interface._optilab_mp.variable('%1')").arg(keyX);
    else if (typeX == "system")
    {
        if (keyX == "index")
        {
            for (int i = 0; i < optilabMain->trvVariants->topLevelItemCount(); i++)
                valuesX.append(i);
        }
    }


    QString typeY = cmbChartY->currentData().toString().split(".").at(0);
    QString keyY = cmbChartY->currentData().toString().split(".").at(1);
    QString strY;
    if (typeY == "parameter")
        strY = QString("agros2d_post_values_y = variant.optilab_interface._optilab_mp.parameter('%1')").arg(keyY);
    else if (typeY == "variable")
        strY = QString("agros2d_post_values_y = variant.optilab_interface._optilab_mp.variable('%1')").arg(keyY);
    else if (typeY == "system")
    {
        if (keyY == "index")
        {
            for (int i = 0; i < optilabMain->trvVariants->topLevelItemCount(); i++)
                valuesY.append(i);
        }
    }

    currentPythonEngine()->runExpression(strX);
    currentPythonEngine()->runExpression(strY);

    // extract values
    PyObject *resultX = PyDict_GetItemString(currentPythonEngine()->dict(), "agros2d_post_values_x");
    PyObject *resultY = PyDict_GetItemString(currentPythonEngine()->dict(), "agros2d_post_values_y");
    if (resultX)
    {
        Py_INCREF(resultX);
        for (int i = 0; i < PyList_Size(resultX); i++)
        {
            PyObject *d = PyList_GetItem(resultX, i);
            Py_INCREF(d);
            valuesX.append(PyFloat_AsDouble(d));
            Py_XDECREF(d);
        }
        Py_XDECREF(resultX);
    }

    if (resultY)
    {
        Py_INCREF(resultY);
        for (int i = 0; i < PyList_Size(resultY); i++)
        {
            PyObject *d = PyList_GetItem(resultY, i);
            Py_INCREF(d);
            valuesY.append(PyFloat_AsDouble(d));
            Py_XDECREF(d);
        }
        Py_XDECREF(resultY);
    }

    chartXY->graph(0)->setData(valuesX, valuesY);
    chartXY->rescaleAxes();
    chartXY->replot();

    // remove variables
    currentPythonEngine()->runExpression("del agros2d_post_values_x; del agros2d_post_values_y");
}

/*
void OptilabMulti::refreshChart()
{
    return;

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

}
*/
/*
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


    if (!selectedX.isEmpty())
        cmbX->setCurrentIndex(cmbX->findText(selectedX));
    if (!selectedY.isEmpty())
        cmbY->setCurrentIndex(cmbY->findText(selectedY));

    refreshChartControls();
    refreshChart();

    chart->rescaleAxes();
    chart->replot();
}
*/
/*
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
*/
/*
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

}
*/
