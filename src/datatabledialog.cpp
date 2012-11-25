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

#include "pythonlab/pythonengine_agros.h"
#include "materialbrowserdialog.h"

#include "gui/chart.h"
#include "qwt_symbol.h"

DataTableDialog::DataTableDialog(QWidget *parent, const QString &labelX, const QString &labelY)
    : QDialog(parent), m_labelX(labelX), m_labelY(labelY)
{
    setWindowIcon(icon("scene-function"));
    setWindowTitle(tr("Data Table"));

    m_table = new DataTable();

    createControls();
    load();
    doPlot();

    setMinimumSize(600, 400);

    QSettings settings;
    restoreGeometry(settings.value("DataTableDialog/Geometry", saveGeometry()).toByteArray());
    chkInterpolation->setChecked(settings.value("DataTableDialog/Interpolation", true).toBool());
    chkDerivative->setChecked(settings.value("DataTableDialog/Derivative", false).toBool());
    chkMarkers->setChecked(settings.value("DataTableDialog/Markers", true).toBool());

    chartDerivative->setVisible(chkDerivative->isChecked());
}

DataTableDialog::~DataTableDialog()
{
    QSettings settings;
    settings.setValue("DataTableDialog/Geometry", saveGeometry());
    settings.setValue("DataTableDialog/Interpolation", chkInterpolation->isChecked());
    settings.setValue("DataTableDialog/Derivative", chkDerivative->isChecked());
    settings.setValue("DataTableDialog/Markers", chkMarkers->isChecked());

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
    textChanged();
    QTimer::singleShot(0, this, SLOT(doPlot()));
}

bool DataTableDialog::parseTable(bool addToTable)
{
    lblInfoError->setText("");

    if (lstX->toPlainText().trimmed().isEmpty() && lstY->toPlainText().trimmed().isEmpty())
    {
        chartValue->setEnabled(false);
        chartDerivative->setEnabled(false);

        return false;
    }

    bool procesOK = true;

    QStringList x = lstX->toPlainText().trimmed().split("\n");
    QStringList y = lstY->toPlainText().trimmed().split("\n");

    // check size
    if (x.size() != y.size())
    {
        btnPlot->setEnabled(false);
        btnOk->setEnabled(false);

        lblInfoError->setText((x.size() > y.size()) ? tr("Size doesn't match (%1 > %2).").arg(x.size()).arg(y.size()) :
                                                      tr("Size doesn't match (%1 < %2).").arg(x.size()).arg(y.size()));
        return false;
    }

    int count = x.size();
    double *keys = new double[count];
    double *values = new double[count];

    chartValue->setEnabled(true);
    chartDerivative->setEnabled(true);

    for (int i = 0; i < count; i++)
    {
        bool ok;

        keys[i] = x[i].toDouble(&ok);
        if (!ok)
        {
            lblInfoError->setText(tr("%1: cannot parse number (line %2).")
                                  .arg(m_labelX)
                                  .arg(i));
            procesOK = false;
            break;
        }

        values[i] = y[i].toDouble(&ok);
        if (!ok)
        {
            lblInfoError->setText(tr("%1: cannot parse number (line %2).")
                                  .arg(m_labelY)
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
    lblLabelX = new QLabel(m_labelX);
    lblLabelY = new QLabel(m_labelY);
    lblInfoX = new QLabel();
    lblInfoY = new QLabel();
    lblInfoError = new QLabel();

    QPalette palette = lblInfoError->palette();
    palette.setColor(QPalette::WindowText, Qt::red);
    lblInfoError->setPalette(palette);

    // chart
    chartValue = new Chart(this, true);
    // axis labels
    chartValue->setAxisTitle(QwtPlot::xBottom, m_labelX);
    chartValue->setAxisTitle(QwtPlot::yLeft, m_labelY);

    chartValueCurveMarkers = new QwtPlotCurve();
    chartValueCurveMarkers->setRenderHint(QwtPlotItem::RenderAntialiased);
    chartValueCurveMarkers->setStyle(QwtPlotCurve::NoCurve);
    chartValueCurveMarkers->setCurveAttribute(QwtPlotCurve::Inverted);
    chartValueCurveMarkers->setYAxis(QwtPlot::yLeft);
    chartValueCurveMarkers->setSymbol(new QwtSymbol(QwtSymbol::Diamond, QColor(Qt::black), QColor(Qt::black), QSize(5, 5)));
    chartValueCurveMarkers->attach(chartValue);

    chartDerivative = new Chart(this, true);
    // axis labels
    chartDerivative->setAxisTitle(QwtPlot::xBottom, m_labelX);
    chartDerivative->setAxisTitle(QwtPlot::yLeft, QString("d%1/d%2").arg(m_labelY).arg(m_labelX));

    QGridLayout *chartLayout = new QGridLayout();
    chartLayout->addWidget(chartValue);
    chartLayout->addWidget(chartDerivative);

    // interval
    lstX = new QPlainTextEdit();
    lstX->setMaximumWidth(100);
    lstX->setMinimumWidth(100);
    connect(lstX, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(lstX, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLineX()));
    lstY = new QPlainTextEdit();
    lstY->setMaximumWidth(100);
    lstY->setMinimumWidth(100);
    connect(lstY, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(lstY, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLineY()));

    chkMarkers = new QCheckBox(tr("Show markers"));
    connect(chkMarkers, SIGNAL(clicked()), this, SLOT(doPlot()));

    chkInterpolation = new QCheckBox(tr("Spline interpolation"));
    connect(chkInterpolation, SIGNAL(clicked()), this, SLOT(doPlot()));

    chkDerivative = new QCheckBox(tr("Derivative chart"));
    connect(chkDerivative, SIGNAL(clicked()), this, SLOT(doShowDerivativeClicked()));
    doShowDerivativeClicked();

    QGridLayout *layoutSettings = new QGridLayout();
    layoutSettings->addWidget(chkMarkers, 3, 0, 1, 2);
    layoutSettings->addWidget(chkInterpolation, 4, 0, 1, 2);
    layoutSettings->addWidget(chkDerivative, 5, 0, 1, 2);

    QGridLayout *controlsLayout = new QGridLayout();
    controlsLayout->addWidget(lblLabelX, 0, 0);
    controlsLayout->addWidget(lblInfoX, 0, 1, 1, 1, Qt::AlignRight);
    controlsLayout->addWidget(lstX, 1, 0, 1, 2);
    controlsLayout->addWidget(lblLabelY, 0, 2);
    controlsLayout->addWidget(lblInfoY, 0, 3, 1, 1, Qt::AlignRight);
    controlsLayout->addWidget(lstY, 1, 2, 1, 2);
    controlsLayout->addWidget(lblInfoError, 2, 0, 1, 4);
    controlsLayout->addLayout(layoutSettings, 3, 0, 1, 5);
    controlsLayout->addLayout(chartLayout, 1, 5, 4, 1);

    // dialog buttons
    btnOk = new QPushButton(tr("Ok"));
    btnOk->setDefault(true);
    connect(btnOk, SIGNAL(clicked()), this, SLOT(doAccept()));
    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doReject()));
    btnPlot = new QPushButton(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(doPlot()));
    QPushButton *btnMaterialBrowser = new QPushButton(tr("Material browser"));
    connect(btnMaterialBrowser, SIGNAL(clicked()), this, SLOT(doMaterialBrowser()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnMaterialBrowser);
    layoutButtons->addWidget(btnPlot);
    layoutButtons->addWidget(btnOk);
    layoutButtons->addWidget(btnClose);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(controlsLayout);
    layout->addLayout(layoutButtons);

    setLayout(layout);
}

void DataTableDialog::textChanged()
{
    lblInfoX->setText(tr("%1").arg(lstX->toPlainText().trimmed().split("\n").size()));
    lblInfoY->setText(tr("%1").arg(lstY->toPlainText().trimmed().split("\n").size()));

    // try parse
    parseTable(false);
}

void DataTableDialog::gotoLine(QPlainTextEdit *lst, int lineNumber)
{
    if (lineNumber >= lst->document()->lineCount())
        lineNumber = lst->document()->lineCount() - 1;

    const QTextBlock &block = lst->document()->findBlockByNumber(lineNumber);
    QTextCursor cursor(block);

    int linedif = lineNumber - lst->textCursor().blockNumber();
    if (linedif < 0)
        cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor, 0);
    else
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, 0);

    lst->blockSignals(true);
    lst->setTextCursor(cursor);
    highlightCurrentLine(lst);
    lst->blockSignals(false);
    lst->ensureCursorVisible();
}

void DataTableDialog::highlightCurrentLine(QPlainTextEdit *lst)
{
    lst->blockSignals(true);

    QList<QTextEdit::ExtraSelection> selections;

    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::yellow).lighter(180);

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = lst->textCursor();
    selection.cursor.clearSelection();
    selections.append(selection);

    lst->setExtraSelections(selections);
    lst->blockSignals(false);
}

void DataTableDialog::highlightCurrentLineX()
{
    highlightCurrentLine(lstX);
    gotoLine(lstY, lstX->textCursor().blockNumber());
}

void DataTableDialog::highlightCurrentLineY()
{
    highlightCurrentLine(lstY);
    gotoLine(lstX, lstY->textCursor().blockNumber());
}

void DataTableDialog::doPlot()
{
    chartValueCurveMarkers->setVisible(chkMarkers->isChecked());

    parseTable();

    // points
    int count = m_table->size();

    double *keys = new double[count];
    double *values = new double[count];
    double *derivatives = new double[count];

    m_table->get(keys, values, derivatives);
    chartValueCurveMarkers->setSamples(keys, values, count);

    delete [] keys;
    delete [] values;
    delete [] derivatives;

    // interpolation
    int countSpline = count*1e3;
    double dx = (m_table->maxKey() - m_table->minKey()) / (countSpline);

    double *keysSpline = new double[countSpline];
    double *valuesSpline = new double[countSpline];
    double *derivativesSpline = new double[countSpline];

    if (chkInterpolation->isChecked())
    {
        // spline
        for (int i = 0; i < countSpline; i++)
        {
            keysSpline[i] = m_table->minKey() + (i * dx);
            // values[i] = m_table->value(keys[i]);
            valuesSpline[i] = m_table->valueSpline(keysSpline[i]);
            // derivatives[i] = m_table->derivative(keys[i]);
            derivativesSpline[i] = m_table->derivativeSpline(keysSpline[i]);
        }
    }
    else
    {
        // lines
        for (int i = 0; i < countSpline; i++)
        {
            keysSpline[i] = m_table->minKey() + (i * dx);
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

void DataTableDialog::doShowDerivativeClicked()
{
    chartDerivative->setVisible(chkDerivative->isChecked());
}

void DataTableDialog::doMaterialBrowser()
{
    MaterialBrowserDialog materialBrowserDialog(this);
    if (materialBrowserDialog.showDialog(true) == QDialog::Accepted)
    {
        lstX->clear();
        lstY->clear();
        for (int i = 0; i < materialBrowserDialog.x().size(); i++)
        {
            lstX->appendPlainText(QString::number(materialBrowserDialog.x().at(i)));
            lstY->appendPlainText(QString::number(materialBrowserDialog.y().at(i)));
        }

        QTimer::singleShot(0, this, SLOT(doPlot()));
    }
}

void DataTableDialog::load()
{

}

bool DataTableDialog::save()
{
    return parseTable();
}

void DataTableDialog::doAccept()
{
    if (save())
    {
        accept();
    }
}

void DataTableDialog::doReject()
{
    reject();
}

DataTable *DataTableDialog::table()
{
    return m_table->copy();
}
