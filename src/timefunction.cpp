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

// TimeFunction timeFunction("sin(2*pi*50*time)");

TimeFunction::TimeFunction()
{
    TimeFunction("0.0", 2);
}

TimeFunction::TimeFunction(const QString &function, int count)
{
    m_function = "";
    m_error = "";
    m_times.clear();
    m_values.clear();
    m_count = 0;

    Util::scene()->problemInfo()->timeTotal.evaluate(true);
    setFunction(function, 0.0, Util::scene()->problemInfo()->timeTotal.number, count);
}

TimeFunction::~TimeFunction()
{
    m_times.clear();
    m_values.clear();
}

void TimeFunction::setFunction(const QString &function, double time_min, double time_max, double N, bool quiet)
{
    m_error = "";
    m_function = function;
    m_time_min = time_min;
    m_time_max = time_max;
    m_count = N;

    fillValues(quiet);
}

bool TimeFunction::check() const
{
    TimeFunction timeFunction(m_function, 1);
    return timeFunction.isValid();
}

void TimeFunction::showError()
{
    if (!m_error.isEmpty())
        QMessageBox::warning(QApplication::activeWindow(), QObject::tr("Time function error"), m_error);
}

bool TimeFunction::fillValues(bool quiet)
{
    m_times.clear();
    m_values.clear();

    if (m_function.isEmpty())
        m_function = "0.0";

    // speed improvement for const values
    if (!isTimeDep())
    {
        qDebug() << "bool TimeFunction::fillValues(bool quiet) - const";

        m_count = 1;
        ExpressionResult result = runPythonExpression(m_function);
        if (result.error.isEmpty())
        {
            m_times.append(0.0);
            m_values.append(result.value);

            return true;
        }
        else
        {
            m_error = result.error;

            return false;
        }
    }
    else
    {
        if (m_times.count() == 1)
            qDebug() << "bool TimeFunction::fillValues(bool quiet) - timedep - check";
        else
            qDebug() << "bool TimeFunction::fillValues(bool quiet) - timedep";

        // times
        double dt = (m_time_max - m_time_min) / (m_count + 1);
        for (int i = 0; i < m_count; i++)
            m_times.append(i * dt);

        // values
        QString error = fillTimeFunction(m_function, m_time_min, m_time_max, m_count, &m_values);

        if (error.isEmpty())
        {
            m_error = "";
            return true;
        }
        else
        {
            m_error = error;
            if (!quiet)
                showError();
            return false;
        }
    }
}

double TimeFunction::value(double time) const
{   
    if (!isTimeDep())
    {
        return m_values.at(0);
    }
    else
    {
        if (m_times.isEmpty())
            return 0.0;

        // first value
        if (time <= m_times.at(0))
            return m_values.at(0);

        // last value
        if (time >= m_times.last())
            return m_values.last();

        // linear approach
        // general case - non constant time start = 0
        int start = floor((time - m_time_min) / (m_time_max - m_time_min));
        for (int i = start; i < m_times.length() - 1; i++)
            if ((m_times.at(i) >= time) && (time <= m_times.at(i+1)))
                return m_values.at(i) + ((time - m_times.at(i)) / (m_times.at(i+1) - m_times.at(i))) * (m_values.at(i+1) - m_values.at(i));

        return m_values.last();
    }
}

// ************************************************************************************************************

TimeFunctionEdit::TimeFunctionEdit(QWidget *parent) : QWidget(parent)
{
    logMessage("TimeFunctionEdit::TimeFunctionEdit()");

    setToolTip(tr("Input time function. You can use 'time' variable."));
    createControls();
}

void TimeFunctionEdit::createControls()
{
    logMessage("TimeFunctionEdit::createControls()");

    txtFunction = new QLineEdit(this);
    btnEdit = new QPushButton(icon("three-dots"), "", this);
    connect(btnEdit, SIGNAL(clicked()), this, SLOT(doOpenDialog()));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->addWidget(txtFunction);
    layout->addWidget(btnEdit);

    setLayout(layout);
}

void TimeFunctionEdit::setTimeFunction(const TimeFunction &timeFunction)
{
    m_timeFunction = timeFunction;
    txtFunction->setText(m_timeFunction.function());
}

void TimeFunctionEdit::doOpenDialog()
{
    TimeFunctionDialog *dialog = new TimeFunctionDialog();
    dialog->setTimeFunction(m_timeFunction);

    if (dialog->exec() == QDialog::Accepted)
    {
        setTimeFunction(dialog->timeFunction());
    }
    delete dialog;
}

// ************************************************************************************************************

TimeFunctionDialog::TimeFunctionDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("TimeFunctionDialog::TimeFunctionDialog()");

    setWindowIcon(icon("timefunction"));
    setWindowTitle(tr("Time function"));

    createControls();
    doPlot();

    setMinimumSize(600, 400);

    QSettings settings;
    restoreGeometry(settings.value("TimeFunctionDialog/Geometry", saveGeometry()).toByteArray());
}

TimeFunctionDialog::~TimeFunctionDialog()
{
    QSettings settings;
    settings.setValue("TimeFunctionDialog/Geometry", saveGeometry());
}

void TimeFunctionDialog::setTimeFunction(const TimeFunction &timeFunction)
{
    m_timeFunction = timeFunction;
    txtFunction->setText(m_timeFunction.function());

    // plot
    doPlot();
}

void TimeFunctionDialog::createControls()
{
    logMessage("TimeFunctionDialog::createControls()");

    lblInfoError = new QLabel();

    QPalette palette = lblInfoError->palette();
    palette.setColor(QPalette::WindowText, Qt::red);
    lblInfoError->setPalette(palette);

    txtFunction = new QLineEdit(this);

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

    connect(picker, SIGNAL(moved(const QPoint &)), SLOT(doMoved(const QPoint &)));

    QGridLayout *controlsLayout = new QGridLayout();
    controlsLayout->addWidget(chart, 0, 0, 1, 2);
    controlsLayout->addWidget(new QLabel(tr("Function:")), 1, 0);
    controlsLayout->addWidget(txtFunction, 1, 1);
    controlsLayout->addWidget(lblInfoError, 2, 1);

    // dialog buttons
    btnOk = new QPushButton(tr("Ok"));
    btnOk->setDefault(true);
    connect(btnOk, SIGNAL(clicked()), this, SLOT(doAccept()));
    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doReject()));
    btnPlot = new QPushButton(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(doPlot()));

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

void TimeFunctionDialog::doMoved(const QPoint &pos)
{
    logMessage("ChartDialog::doMoved()");

    QString info;
    info.sprintf("x=%g, y=%g", chart->invTransform(QwtPlot::xBottom, pos.x()), chart->invTransform(QwtPlot::yLeft, pos.y()));
}

void TimeFunctionDialog::doPlot()
{
    logMessage("TimeFunctionDialog::doPlot()");

    m_timeFunction.setFunction(txtFunction->text(), m_timeFunction.timeMin(), m_timeFunction.timeMax(),
                               m_timeFunction.count(), true);
    if (!m_timeFunction.isValid())
    {
        lblInfoError->setText(m_timeFunction.getError());
        return;
    }
    else
    {
        lblInfoError->setText("");
    }

    chart->setData(m_timeFunction.times(), m_timeFunction.values());
}

void TimeFunctionDialog::doAccept()
{
    m_timeFunction.setFunction(txtFunction->text(), m_timeFunction.timeMin(), m_timeFunction.timeMax(),
                               m_timeFunction.count(), true);
    if (!m_timeFunction.isValid())
    {
        lblInfoError->setText(m_timeFunction.getError());
        txtFunction->setFocus();
        return;
    }

    accept();
}

void TimeFunctionDialog::doReject()
{
    reject();
}

