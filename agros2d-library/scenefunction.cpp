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

#include "scenefunction.h"

#include "gui.h"
#include "scripteditordialog.h"

SceneFunction::SceneFunction(const QString &name, const QString &function, Value start, Value end)
{
    logMessage("SceneFunction::SceneFunction()");

    this->name = name;
    this->function = function;
    this->start = start;
    this->end = end;

    m_countDefault = 200;
    m_count = 0;
    m_valuesX = new double[m_count];
    m_valuesY = new double[m_count];
}

SceneFunction::~SceneFunction()
{
    logMessage("SceneFunction::~SceneFunction()");

    if (m_valuesX) delete m_valuesX;
    if (m_valuesY) delete m_valuesY;
}

QString SceneFunction::script()
{
    logMessage("SceneFunction::script()");

    return QString("addfunction(\"%1\", \"%2\", %3, %4)").
            arg(name).
            arg(function).
            arg(start.text()).
            arg(end.text());
}

QVariant SceneFunction::variant()
{
    logMessage("SceneFunction::variant()");

    QVariant v;
    v.setValue(this);
    return v;
}

double SceneFunction::evaluate(double number, bool fromTable) throw (const QString &)
{
    logMessage("SceneFunction::evaluate()");

    if (fromTable)
    {
        if (number < start.number)
            return start.number;
        if (number > end.number)
            return end.number;

        double step = (end.number - start.number) / (double) m_count;
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

bool SceneFunction::evaluateValues() throw (const QString &)
{
    logMessage("SceneFunction::evaluateValues()");

    if (!start.evaluate(true)) return false;
    if (!end.evaluate(true)) return false;

    m_count = m_countDefault;
    double step = (end.number - start.number) / (double) m_count;
    m_valuesX = new double[m_count];
    m_valuesY = new double[m_count];

    // calculate values
    for (int i = 0; i<m_count; i++)
    {
        m_valuesX[i] = start.number + i*step;

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

int SceneFunction::showDialog(QWidget *parent)
{
    logMessage("SceneFunction::showDialog()");

    DSceneFunction *dialog = new DSceneFunction(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************

DSceneFunction::DSceneFunction(SceneFunction *sceneFunction, QWidget *parent) : QDialog(parent)
{
    logMessage("DSceneFunction::DSceneFunction()");

    m_sceneFunction = sceneFunction;

    setWindowIcon(icon("scene-function"));
    setWindowTitle(tr("Function"));

    createControls();
    load();
    doPlot();

    setMinimumSize(sizeHint());
}

DSceneFunction::~DSceneFunction()
{
    logMessage("DSceneFunction::~DSceneFunction()");

    delete txtName;
    delete txtFunction;
    delete txtStart;
    delete txtEnd;
}

void DSceneFunction::createControls()
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

    // name
    txtName = new QLineEdit("");
    txtFunction = new QLineEdit();

    // interval
    txtStart = new ValueLineEdit();
    txtStart->setMaximumWidth(100);
    connect(txtStart, SIGNAL(editingFinished()), this, SLOT(doPlot()));

    txtEnd = new ValueLineEdit();
    txtEnd->setMaximumWidth(100);
    connect(txtEnd, SIGNAL(editingFinished()), this, SLOT(doPlot()));

    lblError = new QLabel();
    lblError->setVisible(false);

    QPushButton *btnPlot = new QPushButton(this);
    btnPlot->setText(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(doPlot()));

    QPushButton *btnSaveImage = new QPushButton(this);
    btnSaveImage->setText(tr("Save image"));
    connect(btnSaveImage, SIGNAL(clicked()), SLOT(doSaveImage()));

    QGridLayout *controlsLayout = new QGridLayout();
    controlsLayout->addWidget(new QLabel(tr("Start:")), 0, 0);
    controlsLayout->addWidget(txtStart, 0, 1);
    controlsLayout->addWidget(new QLabel(tr("End:")), 1, 0);
    controlsLayout->addWidget(txtEnd, 1, 1);
    controlsLayout->addWidget(new QLabel(tr("Name:")), 0, 2);
    controlsLayout->addWidget(txtName, 0, 3);
    controlsLayout->addWidget(new QLabel(tr("Function:")), 1, 2);
    controlsLayout->addWidget(txtFunction, 1, 3);
    controlsLayout->addWidget(lblError, 0, 4, 2, 1);
    controlsLayout->addWidget(btnPlot, 0, 5);
    controlsLayout->addWidget(btnSaveImage, 1, 5);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(chart);
    layout->addLayout(controlsLayout);
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void DSceneFunction::doSaveImage()
{
    logMessage("DSceneFunction::doSaveImage()");

    chart->saveImage();
}

void DSceneFunction::doPlot()
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

void DSceneFunction::load()
{
    logMessage("DSceneFunction::load()");

    txtName->setText(m_sceneFunction->name);
    txtFunction->setText(m_sceneFunction->function);
    txtStart->setValue(m_sceneFunction->start);
    txtEnd->setValue(m_sceneFunction->end);
}

bool DSceneFunction::save()
{
    logMessage("DSceneFunction::save()");

    m_sceneFunction->name = txtName->text();
    m_sceneFunction->function = txtFunction->text();
    m_sceneFunction->start = txtStart->value();
    m_sceneFunction->end = txtEnd->value();

    return true;
}

void DSceneFunction::doAccept()
{
    logMessage("DSceneFunction::doAccept()");

    if (save())
    {       
        accept();
    }
}

void DSceneFunction::doReject()
{
    logMessage("DSceneFunction::doReject()");

    reject();
}
