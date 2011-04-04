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

#include "value.h"

#include "gui.h"
#include "scripteditordialog.h"
#include "scene.h"

bool Value::evaluate(bool quiet)
{
    evaluate(0.0, quiet);
}

bool Value::evaluate(double time, bool quiet)
{
    logMessage("Value::evaluate()");

    // eval time
    runPythonExpression(QString("time = %1").arg(time));

    // eval expression
    ExpressionResult expressionResult;
    expressionResult = runPythonExpression(text);
    if (expressionResult.error.isEmpty())
    {
        number = expressionResult.value;
    }
    else
    {
        if (!quiet)
            QMessageBox::warning(QApplication::activeWindow(), QObject::tr("Error"), expressionResult.error);
    }
    return expressionResult.error.isEmpty();
}

bool Value::isTimeDep() const
{
    // FIXME - do it better
    return Util::scene()->problemInfo()->analysisType == AnalysisType_Transient
            && text.contains("time");
}

// ***********************************************************************************

ValueLineEdit::ValueLineEdit(QWidget *parent, bool hasTimeDep) : QWidget(parent)
{
    logMessage("SLineEditValue::SLineEditValue()");

    m_minimum = -CONST_DOUBLE;
    m_minimumSharp = -CONST_DOUBLE;
    m_maximum =  CONST_DOUBLE;
    m_maximumSharp =  CONST_DOUBLE;

    m_hasTimeDep = hasTimeDep;

    // create controls
    txtLineEdit = new QLineEdit(this);
    txtLineEdit->setToolTip(tr("This textedit allows using variables."));
    txtLineEdit->setText("0");
    connect(txtLineEdit, SIGNAL(textChanged(QString)), this, SLOT(evaluate()));
    connect(txtLineEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));

    lblValue = new QLabel(this);

    // timedep value
    btnEdit = new QPushButton(icon("three-dots"), "", this);
    connect(btnEdit, SIGNAL(clicked()), this, SLOT(doOpenValueTimeDialog()));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->addWidget(txtLineEdit, 1);
    layout->addWidget(lblValue, 0, Qt::AlignRight);
    layout->addWidget(btnEdit, 0, Qt::AlignRight);

    setLayout(layout);

    evaluate();
}

void ValueLineEdit::setNumber(double value)
{
    logMessage("SLineEditValue::setNumber()");

    txtLineEdit->setText(QString::number(value));
    evaluate();
}

double ValueLineEdit::number()
{
    logMessage("SLineEditValue::number()");

    if (evaluate())
        return m_number;
    else
        return 0.0;
}

void ValueLineEdit::setValue(Value value)
{
    logMessage("SLineEditValue::setValue()");

    txtLineEdit->setText(value.text);
    evaluate();
}

Value ValueLineEdit::value()
{
    logMessage("SLineEditValue::value()");

    return Value(txtLineEdit->text());
}

bool ValueLineEdit::evaluate(bool quiet)
{
    logMessage("SLineEditValue::evaluate()");

    bool isOk = false;

    Value val = value();
    // btnEdit->setVisible(m_hasTimeDep && val.isTimeDep());
    btnEdit->setVisible(m_hasTimeDep && Util::scene()->problemInfo()->analysisType == AnalysisType_Transient);

    if (val.evaluate(quiet))
    {
        if (val.number <= m_minimumSharp)
        {
            setLabel(QString("<= %1").arg(m_minimumSharp), QColor(Qt::blue), true);
        }
        else if (val.number >= m_maximumSharp)
        {
            setLabel(QString(">= %1").arg(m_maximumSharp), QColor(Qt::blue), true);
        }
        else if (val.number < m_minimum)
        {
            setLabel(QString("< %1").arg(m_minimum), QColor(Qt::blue), true);
        }
        else if (val.number > m_maximum)
        {
            setLabel(QString("> %1").arg(m_maximum), QColor(Qt::blue), true);
        }
        else
        {
            m_number = val.number;
            setLabel(QString("%1").arg(m_number, 0, 'g', 3), QApplication::palette().color(QPalette::WindowText), Util::config()->lineEditValueShowResult);
            isOk = true;
        }
    }
    else
    {
        setLabel(tr("error"), QColor(Qt::red), true);
        setFocus();
    }

    if (isOk)
    {
        emit evaluated(false);
        return true;
    }
    else
    {
        emit evaluated(true);
        return false;
    }
}

void ValueLineEdit::setLabel(const QString &text, QColor color, bool isVisible)
{
    logMessage("SLineEditValue::setLabel()");

    lblValue->setText(text);
    QPalette palette = lblValue->palette();
    palette.setColor(QPalette::WindowText, color);
    lblValue->setPalette(palette);
    lblValue->setVisible(isVisible);
}

void ValueLineEdit::focusInEvent(QFocusEvent *event)
{
    logMessage("SLineEditValue::focusInEvent()");

    txtLineEdit->setFocus(event->reason());
}

void ValueLineEdit::doOpenValueTimeDialog()
{
    ValueTimeDialog *dialog = new ValueTimeDialog();
    dialog->setValue(Value(txtLineEdit->text()));

    if (dialog->exec() == QDialog::Accepted)
    {
        txtLineEdit->setText(dialog->value().text);
        evaluate();
    }
    delete dialog;
}

// ****************************************************************************************************************

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

