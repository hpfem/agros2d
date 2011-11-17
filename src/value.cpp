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
#include "datatable.h"
#include "datatabledialog.h"

Value::Value()
{
    this->m_isLinear = -1;
    this->m_isEvaluated = false;
    setText("0");
    this->table = new DataTable();
}

Value::Value(const QString &value, DataTable *table)
{
    this->m_isLinear = -1;
    this->m_isEvaluated = false;
    setText(value.isEmpty() ? "0" : value);
    this->table = table;
}

Value::Value(const QString &str, bool evaluateExpression)
{
    this->m_isLinear = -1;
    this->m_isEvaluated = false;

    fromString(str);

    if (evaluateExpression)
        evaluate(true);
}

Value::~Value()
{

}

double Value::number()
{
    if (!m_isEvaluated)
        evaluate(true);

    return m_number;
}

double Value::value(double key)
{
    // FIXME
    // if (m_isLinear == -1)
    m_isLinear = (Util::scene()->problemInfo()->linearityType == LinearityType_Linear) ? 1 : 0;

    if (m_isLinear || table->size() == 0)
        return number();
    else
    {
        // std::cout << table->value(key) << std::endl;
        return table->value_spline(key);
    }
}

Hermes::Ord Value::value(Hermes::Ord key)
{
    return Hermes::Ord(1);
}

double Value::derivative(double key)
{
    // FIXME
    // if (m_isLinear == -1)
    m_isLinear = (Util::scene()->problemInfo()->linearityType == LinearityType_Linear) ? 1 : 0;

    if (m_isLinear || table->size() == 0)
        return 0.0;
    else
        return table->derivative_spline(key);
}

Hermes::Ord Value::derivative(Hermes::Ord key)
{
    return Hermes::Ord(1);
}

QString Value::toString()
{
    if (table->size() == 0)
        return m_text;
    else
        return m_text + ";" + QString::fromStdString(table->to_string());
}

void Value::fromString(const QString &str)
{
    table = new DataTable();

    if (str.contains(";"))
    {
        // string and table
        QStringList lst = str.split(";");
        this->setText(lst.at(0));

        table->from_string((lst.at(1) + ";" + lst.at(2)).toStdString());
    }
    else
    {
        // just string
        this->setText(str);
    }
}

bool Value::evaluate(bool quiet)
{
    return evaluate(0.0, quiet);
}

bool Value::evaluate(double time, bool quiet)
{
    logMessage("Value::evaluate()");

    // eval time
    runPythonExpression(QString("time = %1").arg(time), false);

    // eval expression
    ExpressionResult expressionResult;
    expressionResult = runPythonExpression(m_text);
    if (expressionResult.error.isEmpty())
    {
        m_number = expressionResult.value;
    }
    else
    {
        if (!quiet)
            QMessageBox::warning(QApplication::activeWindow(), QObject::tr("Error"), expressionResult.error);
    }

    m_isEvaluated = true;
    return expressionResult.error.isEmpty();
}

bool Value::isTimeDep() const
{
    // FIXME - do it better
    return Util::scene()->problemInfo()->analysisType == AnalysisType_Transient
            && m_text.contains("time");
}

// ***********************************************************************************

ValueLineEdit::ValueLineEdit(QWidget *parent, bool hasTimeDep, bool hasNonlin) : QWidget(parent)
{
    logMessage("SLineEditValue::SLineEditValue()");

    m_minimum = -numeric_limits<double>::max();
    m_minimumSharp = -numeric_limits<double>::max();
    m_maximum = numeric_limits<double>::max();
    m_maximumSharp = numeric_limits<double>::max();

    m_hasTimeDep = hasTimeDep;
    m_hasNonlin = hasNonlin;
    m_table = new DataTable();

    // create controls
    txtLineEdit = new QLineEdit(this);
    txtLineEdit->setToolTip(tr("This textedit allows using variables."));
    txtLineEdit->setText("0");
    connect(txtLineEdit, SIGNAL(textChanged(QString)), this, SLOT(evaluate()));
    connect(txtLineEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));

    lblValue = new QLabel(this);
    lblInfo = new QLabel(tr("nonlinear material"));

#ifdef Q_WS_MAC
    btnDataTableDelete = new QToolButton();
    btnDataTableDelete->setIcon(icon("remove-item"));
    btnDataTableDelete->setMaximumHeight(txtLineEdit->height() - 4);
#else
    btnDataTableDelete = new QPushButton(icon("remove-item"), "");
    btnDataTableDelete->setMaximumSize(btnDataTableDelete->sizeHint());
#endif
    connect(btnDataTableDelete, SIGNAL(clicked()), this, SLOT(doOpenDataTableDelete()));

#ifdef Q_WS_MAC
    btnDataTableDialog = new QToolButton();
    btnDataTableDialog->setIcon(icon("three-dots"));
    btnDataTableDialog->setMaximumHeight(txtLineEdit->height() - 4);
#else
    btnDataTableDialog = new QPushButton(icon("three-dots"), "");
    btnDataTableDialog->setMaximumSize(btnDataTableDialog->sizeHint());
#endif
    connect(btnDataTableDialog, SIGNAL(clicked()), this, SLOT(doOpenDataTableDialog()));

    // timedep value
#ifdef Q_WS_MAC
    btnEditTimeDep = new QToolButton();
    btnEditTimeDep->setIcon(icon("three-dots"));
    btnEditTimeDep->setMaximumHeight(txtLineEdit->height() - 4);
#else
    btnEditTimeDep = new QPushButton(icon("three-dots"), "");
#endif
    connect(btnEditTimeDep, SIGNAL(clicked()), this, SLOT(doOpenValueTimeDialog()));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->addWidget(txtLineEdit, 1);
    layout->addWidget(lblInfo, 1);
    layout->addWidget(lblValue, 0, Qt::AlignRight);
    layout->addWidget(btnEditTimeDep, 0, Qt::AlignRight);
    layout->addWidget(btnDataTableDelete, 0, Qt::AlignRight);
    layout->addWidget(btnDataTableDialog, 0, Qt::AlignRight);

    setLayout(layout);

    setLayoutValue();
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

    txtLineEdit->setText(value.text());

    delete m_table;
    m_table = value.table->copy();

    setLayoutValue();
    evaluate();
}

Value ValueLineEdit::value()
{
    logMessage("SLineEditValue::value()");

    return Value(txtLineEdit->text(), m_table->copy());
}

bool ValueLineEdit::evaluate(bool quiet)
{
    logMessage("SLineEditValue::evaluate()");

    bool isOk = false;

    if (!m_hasNonlin || m_table->size() == 0)
    {
        Value val = value();
        btnEditTimeDep->setVisible(m_hasTimeDep && Util::scene()->problemInfo()->analysisType == AnalysisType_Transient);

        if (val.evaluate(quiet))
        {
            if (val.number() <= m_minimumSharp)
            {
                setLabel(QString("<= %1").arg(m_minimumSharp), QColor(Qt::blue), true);
            }
            else if (val.number() >= m_maximumSharp)
            {
                setLabel(QString(">= %1").arg(m_maximumSharp), QColor(Qt::blue), true);
            }
            else if (val.number() < m_minimum)
            {
                setLabel(QString("< %1").arg(m_minimum), QColor(Qt::blue), true);
            }
            else if (val.number() > m_maximum)
            {
                setLabel(QString("> %1").arg(m_maximum), QColor(Qt::blue), true);
            }
            else
            {
                m_number = val.number();
                setLabel(QString("%1").arg(m_number, 0, 'g', 3), QApplication::palette().color(QPalette::WindowText), Util::config()->lineEditValueShowResult);
                isOk = true;
            }
        }
        else
        {
            setLabel(tr("error"), QColor(Qt::red), true);
            setFocus();
        }
    }
    else
    {
        // table
        isOk = true;
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

void ValueLineEdit::setLayoutValue()
{
    txtLineEdit->setVisible(false);
    lblValue->setVisible(false);
    lblInfo->setVisible(false);
    btnDataTableDelete->setVisible(false);
    btnDataTableDialog->setVisible(false);

    if ((!m_hasNonlin) || (m_hasNonlin && m_table->size() == 0))
    {
        txtLineEdit->setVisible(true);
        lblValue->setVisible(true);
    }
    if (m_hasNonlin && m_table->size() > 0)
    {
        lblInfo->setVisible(true);
        btnDataTableDelete->setVisible(true);
    }
    if (m_hasNonlin)
        btnDataTableDialog->setVisible(true);
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

QSize ValueLineEdit::sizeHint() const
{
    return QSize(100, 10);
}

void ValueLineEdit::focusInEvent(QFocusEvent *event)
{
    logMessage("SLineEditValue::focusInEvent()");

    txtLineEdit->setFocus(event->reason());
}

void ValueLineEdit::doOpenValueTimeDialog()
{
    ValueTimeDialog dialog;
    dialog.setValue(Value(txtLineEdit->text()));

    if (dialog.exec() == QDialog::Accepted)
    {
        txtLineEdit->setText(dialog.value().text());
        evaluate();
    }
}

void ValueLineEdit::doOpenDataTableDelete()
{
    m_table->clear();

    setLayoutValue();
    evaluate();
}

void ValueLineEdit::doOpenDataTableDialog()
{
    DataTableDialog dataTableDialog(this);
    dataTableDialog.setTable(m_table);
    if (dataTableDialog.exec() == QDialog::Accepted)
    {
        m_table = dataTableDialog.table();
    }

    setLayoutValue();
    evaluate();
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

void ValueTimeDialog::setValue(Value value)
{
    txtLineEdit->setText(value.text());

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

    txtTimeTotal = new ValueLineEdit();
    txtTimeTotal->setValue(Util::scene()->problemInfo()->timeTotal);

    cmbPresets = new QComboBox();
    cmbPresets->addItem(tr("select a preset..."));
    cmbPresets->addItem(tr("constant"), "1.0");
    cmbPresets->addItem(tr("step"), QString("1.0*(time<%1)"));
    cmbPresets->addItem(tr("sine wave"), QString("1.0*sin(2*pi*1.0/%1*time)"));
    cmbPresets->addItem(tr("exp. step"), "1.0*(exp(-10/%1*time) - exp(-20/%1*time))");
    connect(cmbPresets, SIGNAL(currentIndexChanged(int)), this, SLOT(presetsChanged(int)));

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
    controlsLayout->addWidget(chart, 0, 0, 1, 4);
    controlsLayout->addWidget(new QLabel(tr("Function:")), 1, 0);
    controlsLayout->addWidget(txtLineEdit, 1, 1);
    controlsLayout->setColumnStretch(1, 1);
    controlsLayout->addWidget(cmbPresets, 1, 2, 1, 2);
    controlsLayout->addWidget(new QLabel(tr("Total time:")), 2, 2);
    controlsLayout->addWidget(txtTimeTotal, 2, 3);
    controlsLayout->addWidget(new QLabel(tr("Error:")), 2, 0);
    controlsLayout->addWidget(lblInfoError, 2, 1, 3, 1, Qt::AlignTop);
    controlsLayout->addWidget(new QLabel(""), 2, 1);
    controlsLayout->addWidget(new QLabel(""), 3, 1);

    // dialog buttons
    btnOk = new QPushButton(tr("Ok"));
    btnOk->setDefault(true);
    connect(btnOk, SIGNAL(clicked()), this, SLOT(doAccept()));
    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doReject()));
    // btnPlot = new QPushButton(tr("Plot"));
    // connect(btnPlot, SIGNAL(clicked()), this, SLOT(plotFunction()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    // layoutButtons->addWidget(btnPlot);
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

void ValueTimeDialog::presetsChanged(int index)
{
    if (cmbPresets->currentIndex() > 0)
    {
        if (txtTimeTotal->value().evaluate())
        {
            QString preset = cmbPresets->itemData(cmbPresets->currentIndex()).toString().arg(txtTimeTotal->value().number() / 2.0);

            txtLineEdit->setText(preset);
            cmbPresets->setCurrentIndex(0);

            plotFunction();
        }
    }
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
    if (expressionResult.error.isEmpty())
        plotFunction();
    else
        txtLineEdit->setFocus();
}

void ValueTimeDialog::plotFunction()
{
    logMessage("ValueTimeDialog::plotFunction()");

    // plot solution
    int count = 200;

    double *xval = new double[count];
    double *yval = new double[count];

    double totalTime = txtTimeTotal->value().number();

    // time step
    double dt = totalTime / (count + 1);

    Value val(txtLineEdit->text());
    for (int i = 0; i < count; i++)
    {
        xval[i] = i*dt;

        if (!val.evaluate(xval[i], true))
            break;
        yval[i] = val.number();
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

