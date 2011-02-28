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

#include "datatabledialog.h"

#include "gui.h"
#include "scripteditordialog.h"

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

    QSettings settings;
    restoreGeometry(settings.value("DataTableDialog/Geometry", saveGeometry()).toByteArray());
    chkInterpolation->setChecked(settings.value("DataTableDialog/Interpolation").toBool());
}

DataTableDialog::~DataTableDialog()
{
    QSettings settings;
    settings.setValue("DataTableDialog/Geometry", saveGeometry());
    settings.setValue("DataTableDialog/Interpolation", chkInterpolation->isChecked());

    if (m_table)
        delete m_table;
}

void DataTableDialog::setTable(const DataTable *table)
{
    delete m_table;

    m_table = table->copy();

    DataTableRow *data = m_table->data();
    while (data)
    {
        DataTableRow *tmp = data;

        lstX->appendPlainText(QString::number(tmp->key));
        lstY->appendPlainText(QString::number(tmp->value));

        // next row
        data = data->next;
    }

    // plot
    doPlot();
    doTextChanged();
}

bool DataTableDialog::parseTable(bool addToTable)
{
    lblInfoError->setText("");

    bool procesOK = true;

    QStringList x = lstX->toPlainText().trimmed().split("\n");
    QStringList y = lstY->toPlainText().trimmed().split("\n");

    // check size
    if (x.size() != y.size())
    {
        btnPlot->setEnabled(false);
        btnOk->setEnabled(false);

        lblInfoError->setText(tr("Size doesn't match (%1 != %2).")
                              .arg(x.size())
                              .arg(y.size()));
        return false;
    }

    int count = x.size();
    double *keys = new double[count];
    double *values = new double[count];

    for (int i = 0; i < count; i++)
    {
        bool ok;

        keys[i] = x[i].toDouble(&ok);
        if (!ok)
        {
            lblInfoError->setText(tr("X: cannot parse number (line %1).")
                                  .arg(i));
            procesOK = false;
            break;
        }

        values[i] = y[i].toDouble(&ok);
        if (!ok)
        {
            lblInfoError->setText(tr("Y: cannot parse number (line %1).")
                                  .arg(i));
            procesOK = false;
            break;
        }
    }

    if (addToTable)
    {
        m_table->clear();
        m_table->add(keys, values, count);
    }

    delete [] keys;
    delete [] values;

    btnPlot->setEnabled(procesOK);
    btnOk->setEnabled(procesOK);

    return procesOK;
}

void DataTableDialog::createControls()
{
    logMessage("DataTableDialog::createControls()");

    lblInfoX = new QLabel();
    lblInfoY = new QLabel();
    lblInfoError = new QLabel();

    QPalette palette = lblInfoError->palette();
    palette.setColor(QPalette::WindowText, Qt::red);
    lblInfoError->setPalette(palette);

    // chart
    QwtText text("");

    chartValue = new Chart(this);
    // axis labels
    text.setFont(QFont("Helvetica", 10, QFont::Normal));
    text.setText("x");
    chartValue->setAxisTitle(QwtPlot::xBottom, text);
    text.setText("y");
    chartValue->setAxisTitle(QwtPlot::yLeft, text);

    chartValueCurveDots = new QwtPlotCurve();
    chartValueCurveDots->setRenderHint(QwtPlotItem::RenderAntialiased);
    chartValueCurveDots->setStyle(QwtPlotCurve::NoCurve);
    chartValueCurveDots->setCurveAttribute(QwtPlotCurve::Inverted);
    chartValueCurveDots->setYAxis(QwtPlot::yLeft);
    chartValueCurveDots->setSymbol(QwtSymbol(QwtSymbol::Diamond, Qt::red, QPen(Qt::blue, 1), QSize(10,10)));
    chartValueCurveDots->attach(chartValue);

    chartDerivative = new Chart(this);
    // axis labels
    text.setFont(QFont("Helvetica", 10, QFont::Normal));
    text.setText("x");
    chartDerivative->setAxisTitle(QwtPlot::xBottom, text);
    text.setText("dy/dx");
    chartDerivative->setAxisTitle(QwtPlot::yLeft, text);

    chartDerivativeCurveDots = new QwtPlotCurve();
    chartDerivativeCurveDots->setRenderHint(QwtPlotItem::RenderAntialiased);
    chartDerivativeCurveDots->setStyle(QwtPlotCurve::NoCurve);
    chartDerivativeCurveDots->setCurveAttribute(QwtPlotCurve::Inverted);
    chartDerivativeCurveDots->setYAxis(QwtPlot::yLeft);
    chartDerivativeCurveDots->setSymbol(QwtSymbol(QwtSymbol::Diamond, Qt::red, QPen(Qt::blue, 1), QSize(10,10)));
    chartDerivativeCurveDots->attach(chartDerivative);

    QGridLayout *chartLayout = new QGridLayout();
    chartLayout->addWidget(chartValue);
    chartLayout->addWidget(chartDerivative);

    // interval
    lstX = new QPlainTextEdit();
    lstX->setMaximumWidth(100);
    lstX->setMinimumWidth(100);
    connect(lstX, SIGNAL(textChanged()), this, SLOT(doTextChanged()));
    lstY = new QPlainTextEdit();
    lstY->setMaximumWidth(100);
    lstY->setMinimumWidth(100);
    connect(lstY, SIGNAL(textChanged()), this, SLOT(doTextChanged()));

    chkInterpolation = new QCheckBox(tr("Spline interpolation"));

    QGridLayout *layoutSettings = new QGridLayout();
    layoutSettings->addWidget(chkInterpolation, 3, 0, 1, 2);

    QGridLayout *controlsLayout = new QGridLayout();
    controlsLayout->addWidget(lblInfoX, 0, 0);
    controlsLayout->addWidget(lstX, 1, 0);
    controlsLayout->addWidget(lblInfoY, 0, 1);
    controlsLayout->addWidget(lstY, 1, 1);
    controlsLayout->addLayout(layoutSettings, 2, 0, 1, 2);
    controlsLayout->addLayout(chartLayout, 1, 2, 3, 1);
    controlsLayout->addWidget(lblInfoError, 3, 0, 1, 3);

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

void DataTableDialog::doTextChanged()
{
    lblInfoX->setText(tr("X: %1").arg(lstX->toPlainText().trimmed().split("\n").size()));
    lblInfoY->setText(tr("Y: %1").arg(lstY->toPlainText().trimmed().split("\n").size()));

    // try parse
    parseTable(false);
}

void DataTableDialog::doPlot()
{
    logMessage("DSceneFunction::doPlot()");

    parseTable();

    // points
    int count = m_table->size();

    double *keys = new double[count];
    double *values = new double[count];
    double *derivatives = new double[count];

    m_table->get(keys, values, derivatives);
    chartValueCurveDots->setData(keys, values, count);
    chartDerivativeCurveDots->setData(keys, derivatives, count);

    delete [] keys;
    delete [] values;
    delete [] derivatives;

    // interpolation
    int countSpline = count*1e3;
    double dx = (m_table->max_key() - m_table->min_key()) / (countSpline);

    double *keysSpline = new double[countSpline];
    double *valuesSpline = new double[countSpline];
    double *derivativesSpline = new double[countSpline];

    if (chkInterpolation->isChecked())
    {
        // spline
        for (int i = 0; i < countSpline; i++)
        {
            keysSpline[i] = m_table->min_key() + (i * dx);
            // values[i] = m_table->value(keys[i]);
            valuesSpline[i] = m_table->value_spline(keysSpline[i]);
            // derivatives[i] = m_table->derivative(keys[i]);
            derivativesSpline[i] = m_table->derivative_spline(keysSpline[i]);
        }
    }
    else
    {
        // lines
        for (int i = 0; i < countSpline; i++)
        {
            keysSpline[i] = m_table->min_key() + (i * dx);
            valuesSpline[i] = m_table->value(keysSpline[i]);
            derivativesSpline[i] = m_table->derivative(keysSpline[i]);
        }
    }

    chartValue->setData(keysSpline, valuesSpline, countSpline);
    chartDerivative->setData(keysSpline, derivativesSpline, countSpline);

    delete [] keysSpline;
    delete [] valuesSpline;
    delete [] derivativesSpline;
}

void DataTableDialog::load()
{
    logMessage("DSceneFunction::load()");
}

bool DataTableDialog::save()
{
    logMessage("DSceneFunction::save()");

    return parseTable();
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
