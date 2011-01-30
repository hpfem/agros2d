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

#include "datafunction.h"

#include "gui.h"
#include "scripteditordialog.h"

DataFunction::DataFunction(const QString &function, double start, double end)
{
    logMessage("SceneFunction::SceneFunction()");

    this->function = function;
    this->start = start;
    this->end = end;

    m_countDefault = 200;
    m_count = 0;
    m_valuesX = new double[m_count];
    m_valuesY = new double[m_count];
}

DataFunction::~DataFunction()
{
    logMessage("SceneFunction::~SceneFunction()");

    if (m_valuesX) delete m_valuesX;
    if (m_valuesY) delete m_valuesY;
}

QVariant DataFunction::variant()
{
    logMessage("SceneFunction::variant()");

    QVariant v;
    v.setValue(this);
    return v;
}

double DataFunction::evaluate(double number, bool fromTable) throw (const QString &)
{
    logMessage("SceneFunction::evaluate()");

    if (fromTable)
    {
        if (number < start)
            return start;
        if (number > end)
            return end;

        double step = (end - start) / (double) m_count;
        for (int i = 0; i<m_count; i++)
            if (number >= i*step && number < (i+1)*step)
                return ((i+1)*step - i*step)/step + i*step;
    }
    else
    {
        runPythonExpression(QString("val = %1").arg(number));
        ExpressionResult expressionResult = runPythonExpression(QString("%1").arg(function));
        if (expressionResult.error.isEmpty())
        {
            return expressionResult.value;
        }
        else
        {
            throw expressionResult.error;
        }
    }
}

bool DataFunction::evaluateValues() throw (const QString &)
{
    logMessage("SceneFunction::evaluateValues()");

    m_count = m_countDefault;
    double step = (end - start) / (double) m_count;
    m_valuesX = new double[m_count];
    m_valuesY = new double[m_count];

    // calculate values
    for (int i = 0; i<m_count; i++)
    {
        m_valuesX[i] = start + i*step;

        // evaluate
        try
        {
            m_valuesY[i] = evaluate(m_valuesX[i]);
        }
        catch (const QString &e)
        {
            delete[] m_valuesX;
            delete[] m_valuesY;
            m_count = 0;
            m_valuesX = new double[0];
            m_valuesY = new double[0];

            throw e;

            break;
        }
    }

    return (m_count > 0);
}

int DataFunction::showDialog(QWidget *parent)
{
    logMessage("SceneFunction::showDialog()");

    DDataFunction *dialog = new DDataFunction(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************

DDataFunction::DDataFunction(DataFunction *sceneFunction, QWidget *parent) : QDialog(parent)
{
    logMessage("DSceneFunction::DSceneFunction()");

    m_sceneFunction = sceneFunction;

    setWindowIcon(icon("scene-function"));
    setWindowTitle(tr("Function"));

    createControls();
    load();
    doPlot();

    setMinimumSize(600, 400);
}

void DDataFunction::createControls()
{
    logMessage("DSceneFunction::createControls()");

    // chart
    chart = new Chart(this);
    // axis labels
    QwtText text("");
    text.setFont(QFont("Helvetica", 10, QFont::Normal));
    text.setText("x");
    chart->setAxisTitle(QwtPlot::xBottom, text);
    text.setText("y");
    chart->setAxisTitle(QwtPlot::yLeft, text);

    txtFunction = new QLineEdit();

    // interval
    txtStart = new SLineEditDouble();
    txtStart->setMaximumWidth(100);
    connect(txtStart, SIGNAL(editingFinished()), this, SLOT(doPlot()));

    txtEnd = new SLineEditDouble();
    txtEnd->setMaximumWidth(100);
    connect(txtEnd, SIGNAL(editingFinished()), this, SLOT(doPlot()));

    lblError = new QLabel();
    lblError->setVisible(false);

    QGridLayout *controlsLayout = new QGridLayout();
    controlsLayout->addWidget(chart, 0, 0, 1, 3);
    controlsLayout->addWidget(new QLabel(tr("Start:")), 1, 0);
    controlsLayout->addWidget(txtStart, 1, 1);
    controlsLayout->addWidget(new QLabel(tr("End:")), 2, 0);
    controlsLayout->addWidget(txtEnd, 2, 1);
    controlsLayout->addWidget(new QLabel(tr("Function:")), 3, 0);
    controlsLayout->addWidget(txtFunction, 3, 1, 1, 2);
    controlsLayout->addWidget(lblError, 4, 0, 1, 3);

    // dialog buttons
    btnOk = new QPushButton(tr("Ok"));
    btnOk->setDefault(true);
    connect(btnOk, SIGNAL(clicked()), this, SLOT(doAccept()));
    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doReject()));
    btnPlot = new QPushButton(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(doPlot()));
    btnSaveImage = new QPushButton(tr("Save image"));
    connect(btnSaveImage, SIGNAL(clicked()), SLOT(doSaveImage()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnPlot);
    layoutButtons->addWidget(btnSaveImage);
    layoutButtons->addWidget(btnOk);
    layoutButtons->addWidget(btnClose);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(controlsLayout);
    layout->addLayout(layoutButtons);

    setLayout(layout);
}

void DDataFunction::doSaveImage()
{
    logMessage("DSceneFunction::doSaveImage()");

    chart->saveImage();
}

void DDataFunction::doPlot()
{
    logMessage("DSceneFunction::doPlot()");

    // evaluate
    try
    {
        lblError->setVisible(false);
        m_sceneFunction->function = txtFunction->text();
        m_sceneFunction->evaluateValues();
    }
    catch (const QString &e)
    {
        lblError->setVisible(true);
        lblError->setText(e);
        QPalette palette = lblError->palette();
        palette.setColor(QPalette::WindowText, Qt::red);
        lblError->setPalette(palette);
        lblError->setVisible(true);
    }

    chart->setData(m_sceneFunction->valuesX(), m_sceneFunction->valuesY(), m_sceneFunction->count());
}

void DDataFunction::load()
{
    logMessage("DSceneFunction::load()");

    txtFunction->setText(m_sceneFunction->function);
    txtStart->setValue(m_sceneFunction->start);
    txtEnd->setValue(m_sceneFunction->end);
}

bool DDataFunction::save()
{
    logMessage("DSceneFunction::save()");

    m_sceneFunction->function = txtFunction->text();
    m_sceneFunction->start = txtStart->value();
    m_sceneFunction->end = txtEnd->value();

    return true;
}

void DDataFunction::doAccept()
{
    logMessage("DSceneFunction::doAccept()");

    if (save())
    {       
        accept();
    }
}

void DDataFunction::doReject()
{
    logMessage("DSceneFunction::doReject()");

    reject();
}

// *************************************************************************************************************

DataTableDialog::DataTableDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("DataTableDialog::DataTableDialog()");

    setWindowIcon(icon("scene-function"));
    setWindowTitle(tr("Data Table"));

    m_table = new DataTable();

    createControls();
    load();
    doPlot();

    setMinimumSize(600, 400);
}

DataTableDialog::~DataTableDialog()
{
    if (m_table)
        delete m_table;
}

void DataTableDialog::setTable(const DataTable *table)
{
    delete m_table;

    m_table = table->copy();

    int count = m_table->size();
    double *keys = new double[count];
    double *values = new double[count];
    double *derivatives = new double[count];

    m_table->get(keys, values, derivatives);
    for (int i = 0; i < count; i++)
    {
        lstX->appendPlainText(QString::number(keys[i]));
        lstY->appendPlainText(QString::number(values[i]));
    }

    delete [] keys;
    delete [] values;
    delete [] derivatives;

    // plot
    doPlot();
}

void DataTableDialog::parseTable()
{
    QStringList x = lstX->toPlainText().split("\n");
    QStringList y = lstY->toPlainText().split("\n");

    // check size
    if (x.size() != y.size())
    {
        QMessageBox::critical(this, tr("Size"), tr("Size doesn't match"));
    }

    int count = x.size();
    double *keys = new double[count];
    double *values = new double[count];

    for (int i = 0; i < count; i++)
    {
        keys[i] = x[i].toDouble();
        values[i] = y[i].toDouble();
    }

    m_table->clear();
    m_table->add(keys, values, count);

    delete [] keys;
    delete [] values;
}

void DataTableDialog::createControls()
{
    logMessage("DataTableDialog::createControls()");

    // chart
    chart = new Chart(this);
    // axis labels
    QwtText text("");
    text.setFont(QFont("Helvetica", 10, QFont::Normal));
    text.setText("x");
    chart->setAxisTitle(QwtPlot::xBottom, text);
    text.setText("y");
    chart->setAxisTitle(QwtPlot::yLeft, text);

    // interval
    lstX = new QPlainTextEdit();
    lstX->setMaximumWidth(100);
    lstX->setMinimumWidth(100);
    lstY = new QPlainTextEdit();
    lstY->setMaximumWidth(100);
    lstY->setMinimumWidth(100);

    QGridLayout *controlsLayout = new QGridLayout();
    controlsLayout->addWidget(new QLabel(tr("X:")), 0, 0);
    controlsLayout->addWidget(lstX, 1, 0);
    controlsLayout->addWidget(new QLabel(tr("Y:")), 0, 1);
    controlsLayout->addWidget(lstY, 1, 1);
    controlsLayout->addWidget(chart, 0, 2, 2, 1);

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

void DataTableDialog::doPlot()
{
    logMessage("DSceneFunction::doPlot()");

    parseTable();

    int count = m_table->size();
    double *keys = new double[count];
    double *values = new double[count];
    double *derivatives = new double[count];

    m_table->get(keys, values, derivatives);
    chart->setData(keys, values, count);

    delete [] keys;
    delete [] values;
    delete [] derivatives;
}

void DataTableDialog::load()
{
    logMessage("DSceneFunction::load()");
}

bool DataTableDialog::save()
{
    logMessage("DSceneFunction::save()");

    return true;
}

void DataTableDialog::doAccept()
{
    logMessage("DSceneFunction::doAccept()");

    if (save())
    {
        accept();
    }
}

void DataTableDialog::doReject()
{
    logMessage("DSceneFunction::doReject()");

    reject();
}

DataTable *DataTableDialog::table()
{
    return m_table->copy();
}
