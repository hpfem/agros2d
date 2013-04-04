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

#include "valuedatatabledialog.h"

#include "pythonlab/pythonengine_agros.h"
#include "materialbrowserdialog.h"

#include "gui/chart.h"
#include "qwt_symbol.h"

ValueDataTableDialog::ValueDataTableDialog(DataTable table, QWidget *parent, const QString &labelX, const QString &labelY)
    : QDialog(parent), m_labelX(labelX), m_labelY(labelY), m_table(table)
{
    setWindowIcon(icon("scene-function"));
    setWindowTitle(tr("Data Table"));


    createControls();
    load();

    processDataTable();
    doPlot();

    setMinimumSize(600, 400);

    QSettings settings;
    restoreGeometry(settings.value("DataTableDialog/Geometry", saveGeometry()).toByteArray());
    chkDerivative->setChecked(settings.value("DataTableDialog/Derivative", false).toBool());
    chkMarkers->setChecked(settings.value("DataTableDialog/Markers", true).toBool());
    chkExtrapolation->setChecked(settings.value("DataTableDialog/Extrapolation", true).toBool());

    chartDerivative->setVisible(chkDerivative->isChecked());
}

ValueDataTableDialog::~ValueDataTableDialog()
{
    QSettings settings;
    settings.setValue("DataTableDialog/Geometry", saveGeometry());
    settings.setValue("DataTableDialog/Derivative", chkDerivative->isChecked());
    settings.setValue("DataTableDialog/Markers", chkMarkers->isChecked());
    settings.setValue("DataTableDialog/Extrapolation", chkExtrapolation->isChecked());
}

void ValueDataTableDialog::processDataTable()
{
    for (int i = 0; i < m_table.pointsVector().size(); i++)
    {
        lstX->appendPlainText(QString::number(m_table.pointsVector().at(i)));
        lstY->appendPlainText(QString::number(m_table.valuesVector().at(i)));
    }

    // plot
    textChanged();
    QTimer::singleShot(0, this, SLOT(doPlot()));
}

bool ValueDataTableDialog::parseTable(bool addToTable)
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
        m_table.clear();
        m_table.setValues(keys, values, count);
    }

    delete [] keys;
    delete [] values;

    btnPlot->setEnabled(procesOK);
    btnOk->setEnabled(procesOK);

    return procesOK;
}

void ValueDataTableDialog::createControls()
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
    chartValue = new Chart(this);
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

    chartDerivative = new Chart(this);
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

    chkExtrapolation = new QCheckBox(tr("Show extrapolation"));
    connect(chkExtrapolation, SIGNAL(clicked()), this, SLOT(doPlot()));

    chkDerivative = new QCheckBox(tr("Derivative chart"));
    connect(chkDerivative, SIGNAL(clicked()), this, SLOT(doShowDerivativeClicked()));
    doShowDerivativeClicked();

    cmbType = new QComboBox();
    cmbType->addItem(dataTableTypeString(DataTableType_CubicSpline), DataTableType_CubicSpline);
    cmbType->addItem(dataTableTypeString(DataTableType_PiecewiseLinear), DataTableType_PiecewiseLinear);
    cmbType->addItem(dataTableTypeString(DataTableType_Constant), DataTableType_Constant);
    cmbType->setCurrentIndex(m_table.type());
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged()));


    radFirstDerivative = new QRadioButton("First");
    radSecondDerivative = new QRadioButton("Second");
    QButtonGroup *derivativeGroup = new QButtonGroup();
    derivativeGroup->addButton(radFirstDerivative);
    derivativeGroup->addButton(radSecondDerivative);
    radFirstDerivative->setChecked(m_table.splineFirstDerivatives());
    radSecondDerivative->setChecked(!m_table.splineFirstDerivatives());
    connect(radFirstDerivative, SIGNAL(clicked()), this, SLOT(doSplineDerivativeChanged()));
    connect(radSecondDerivative, SIGNAL(clicked()), this, SLOT(doSplineDerivativeChanged()));

    radExtrapolateConstant = new QRadioButton("Constant");
    radExtrapolateLinear = new QRadioButton("Linear function");
    QButtonGroup *extrapolatinGroup = new QButtonGroup();
    extrapolatinGroup->addButton(radExtrapolateConstant);
    extrapolatinGroup->addButton(radExtrapolateLinear);
    radExtrapolateConstant->setChecked(m_table.extrapolateConstant());
    radExtrapolateLinear->setChecked(!m_table.extrapolateConstant());
    connect(radExtrapolateConstant, SIGNAL(clicked()), this, SLOT(doExtrapolateChanged()));
    connect(radExtrapolateLinear, SIGNAL(clicked()), this, SLOT(doExtrapolateChanged()));

    QGridLayout *layoutSettings = new QGridLayout();
    layoutSettings->addWidget(chkMarkers, 3, 0, 1, 2);
    layoutSettings->addWidget(chkDerivative, 4, 0, 1, 2);
    layoutSettings->addWidget(chkExtrapolation, 5, 0, 1, 2);
    layoutSettings->addWidget(new QLabel(tr("Interpolation")), 6, 0, 1, 1);
    layoutSettings->addWidget(cmbType, 6, 1, 1, 1);
    layoutSettings->addWidget(new QLabel(tr("Derivative to be zero at endpoints:")), 7, 0, 1, 2);
    layoutSettings->addWidget(radFirstDerivative, 8, 0, 1, 1);
    layoutSettings->addWidget(radSecondDerivative, 8, 1, 1, 1);
    layoutSettings->addWidget(new QLabel(tr("Extrapolate as:")), 9, 0, 1, 2);
    layoutSettings->addWidget(radExtrapolateConstant, 10, 0, 1, 1);
    layoutSettings->addWidget(radExtrapolateLinear, 10, 1, 1, 1);

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

void ValueDataTableDialog::textChanged()
{
    lblInfoX->setText(QString("%1").arg(lstX->toPlainText().trimmed().split("\n").size()));
    lblInfoY->setText(QString("%1").arg(lstY->toPlainText().trimmed().split("\n").size()));

    // try parse
    parseTable(false);
}

void ValueDataTableDialog::gotoLine(QPlainTextEdit *lst, int lineNumber)
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

void ValueDataTableDialog::highlightCurrentLine(QPlainTextEdit *lst)
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

void ValueDataTableDialog::highlightCurrentLineX()
{
    highlightCurrentLine(lstX);
    gotoLine(lstY, lstX->textCursor().blockNumber());
}

void ValueDataTableDialog::highlightCurrentLineY()
{
    highlightCurrentLine(lstY);
    gotoLine(lstX, lstY->textCursor().blockNumber());
}

void ValueDataTableDialog::doPlot()
{
    chartValueCurveMarkers->setVisible(chkMarkers->isChecked());

    parseTable();

    // points
    int count = m_table.size();

    Hermes::vector<double> pointsVector = m_table.pointsVector();
    Hermes::vector<double> valuesVector = m_table.valuesVector();

    double *keys = new double[pointsVector.size()];
    double *values = new double[valuesVector.size()];
    for (int i = 0; i < pointsVector.size(); i++)
    {
        keys[i] = pointsVector[i];
        values[i] = valuesVector[i];
    }

    chartValueCurveMarkers->setSamples(keys, values, count);

    delete [] keys;
    delete [] values;

    // interpolation
    int countSpline = count*1e3;
    double keyLength = m_table.maxKey() - m_table.minKey();
    double keyStart = m_table.minKey();
    if(chkExtrapolation->isChecked())
    {
        double overlap = 0.15;
        keyStart -= keyLength * overlap;
        keyLength *= (1 + 2*overlap);
    }

    double dx = keyLength / (countSpline);

    double *keysSpline = new double[countSpline];
    double *valuesSpline = new double[countSpline];
    double *derivativesSpline = new double[countSpline];

    // spline
    for (int i = 0; i < countSpline; i++)
    {
        keysSpline[i] = keyStart + (i * dx);
        valuesSpline[i] = m_table.value(keysSpline[i]);
        derivativesSpline[i] = m_table.derivative(keysSpline[i]);
    }

    chartValue->setData(keysSpline, valuesSpline, countSpline);
    chartDerivative->setData(keysSpline, derivativesSpline, countSpline);

    delete [] keysSpline;
    delete [] valuesSpline;
    delete [] derivativesSpline;
}

void ValueDataTableDialog::doShowDerivativeClicked()
{
    chartDerivative->setVisible(chkDerivative->isChecked());
}

void ValueDataTableDialog::doTypeChanged()
{
    m_table.setType(DataTableType(cmbType->currentIndex()));
    doPlot();
}

void ValueDataTableDialog::doSplineDerivativeChanged()
{
    assert(radFirstDerivative->isChecked() != radSecondDerivative->isChecked());
    m_table.setSplineFirstDerivatives(radFirstDerivative->isChecked());
    doPlot();
}

void ValueDataTableDialog::doExtrapolateChanged()
{
    assert(radExtrapolateConstant->isChecked() != radExtrapolateLinear->isChecked());
    m_table.setExtrapolateConstant(radExtrapolateConstant->isChecked());
    doPlot();
}

void ValueDataTableDialog::doMaterialBrowser()
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

void ValueDataTableDialog::load()
{

}

bool ValueDataTableDialog::save()
{
    return parseTable();
}

void ValueDataTableDialog::doAccept()
{
    if (save())
    {
        accept();
    }
}

void ValueDataTableDialog::doReject()
{
    reject();
}

DataTable ValueDataTableDialog::table()
{
    return m_table;
}
