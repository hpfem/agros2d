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

#include "timefunction.h"

#include "gui.h"
#include "scripteditordialog.h"
#include "scene.h"

ValueTimeDialog::ValueTimeDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("ValueTimeDialog::ValueTimeDialog()");

    setWindowIcon(icon("timefunction"));
    setWindowTitle(tr("Time function"));

    createControls();
    plotFunction();

    setMinimumSize(600, 400);

    QSettings settings;
    restoreGeometry(settings.value("ValueTimeDialog/Geometry", saveGeometry()).toByteArray());
}

ValueTimeDialog::~ValueTimeDialog()
{
    QSettings settings;
    settings.setValue("ValueTimeDialog/Geometry", saveGeometry());
}

void ValueTimeDialog::setValue(const Value &value)
{
    txtLineEdit->setText(value.text);

    // plot
    plotFunction();
}

void ValueTimeDialog::createControls()
{
    logMessage("ValueTimeDialog::createControls()");

    lblInfoError = new QLabel();

    QPalette palette = lblInfoError->palette();
    palette.setColor(QPalette::WindowText, Qt::red);
    lblInfoError->setPalette(palette);

    txtLineEdit = new QLineEdit(this);
    connect(txtLineEdit, SIGNAL(textChanged(QString)), this, SLOT(checkExpression()));

    // chart
    QwtText text("");

    chart = new Chart(this);
    // axis labels
    text.setFont(QFont("Helvetica", 10, QFont::Normal));
    text.setText(tr("time"));
    chart->setAxisTitle(QwtPlot::xBottom, text);
    text.setText(tr("value"));
    chart->setAxisTitle(QwtPlot::yLeft, text);

    chartCurve = new QwtPlotCurve();
    chartCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    chartCurve->setStyle(QwtPlotCurve::NoCurve);
    chartCurve->setCurveAttribute(QwtPlotCurve::Inverted);
    chartCurve->setYAxis(QwtPlot::yLeft);
    // chartCurve->setSymbol(QwtSymbol(QwtSymbol::Diamond, Qt::red, QPen(Qt::blue, 1), QSize(10,10)));
    chartCurve->attach(chart);

    // chart picker
    picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                               QwtPicker::PointSelection | QwtPicker::DragSelection,
                               QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
                               chart->canvas());
    picker->setRubberBandPen(QColor(Qt::green));
    picker->setRubberBand(QwtPicker::CrossRubberBand);
    picker->setTrackerMode(QwtPicker::ActiveOnly);
    picker->setTrackerPen(QColor(Qt::black));

    connect(picker, SIGNAL(moved(const QPoint &)), SLOT(crossMoved(const QPoint &)));

    QGridLayout *controlsLayout = new QGridLayout();
    controlsLayout->addWidget(chart, 0, 0, 1, 2);
    controlsLayout->addWidget(new QLabel(tr("Function:")), 1, 0);
    controlsLayout->addWidget(txtLineEdit, 1, 1);
    controlsLayout->addWidget(lblInfoError, 2, 1);

    // dialog buttons
    btnOk = new QPushButton(tr("Ok"));
    btnOk->setDefault(true);
    connect(btnOk, SIGNAL(clicked()), this, SLOT(doAccept()));
    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doReject()));
    btnPlot = new QPushButton(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(plotFunction()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnPlot);
    layoutButtons->addWidget(btnOk);
    layoutButtons->addWidget(btnClose);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(controlsLayout);
    layout->addLayout(layoutButtons);

    setLayout(layout);
}

void ValueTimeDialog::crossMoved(const QPoint &pos)
{
    logMessage("ChartDialog::crossMoved()");

    QString info;
    info.sprintf("x=%g, y=%g", chart->invTransform(QwtPlot::xBottom, pos.x()), chart->invTransform(QwtPlot::yLeft, pos.y()));
}

void ValueTimeDialog::checkExpression()
{
    logMessage("ValueTimeDialog::checkExpression()");

    // eval time
    runPythonExpression(QString("time = %1").arg(0.0));

    // eval expression
    ExpressionResult expressionResult;
    expressionResult = runPythonExpression(txtLineEdit->text());
    lblInfoError->setText(expressionResult.error.trimmed());
    if (!expressionResult.error.isEmpty())
        txtLineEdit->setFocus();
}

void ValueTimeDialog::plotFunction()
{
    logMessage("ValueTimeDialog::plotFunction()");

    // plot solution
    int count = 200;

    double *xval = new double[count];
    double *yval = new double[count];

    Util::scene()->problemInfo()->timeTotal.evaluate();
    double totalTime = Util::scene()->problemInfo()->timeTotal.number;

    // time step
    double dt = totalTime / (count + 1);

    Value val(txtLineEdit->text());
    for (int i = 0; i < count; i++)
    {
        xval[i] = i*dt;

        if (!val.evaluate(xval[i], true))
            break;
        yval[i] = val.number;
    }

    chart->setData(xval, yval, count);

    delete [] xval;
    delete [] yval;
}

void ValueTimeDialog::doAccept()
{
    accept();
}

void ValueTimeDialog::doReject()
{
    reject();
}

