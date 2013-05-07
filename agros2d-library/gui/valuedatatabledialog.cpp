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

#include "qcustomplot/qcustomplot.h"

ValueDataTableDialog::ValueDataTableDialog(DataTable table, QWidget *parent, const QString &labelX, const QString &labelY, const QString &title)
    : QDialog(parent), m_labelX(labelX), m_labelY(labelY), m_table(table), m_title(title)
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
    chartValue = new QCustomPlot(this);
    chartValue->setInteractions(QCustomPlot::iRangeDrag | QCustomPlot::iRangeZoom);
    chartValue->setRangeDrag(Qt::Horizontal|Qt::Vertical);
    chartValue->setRangeZoom(Qt::Horizontal|Qt::Vertical);
    chartValue->xAxis->setLabel(m_labelX);
    chartValue->yAxis->setLabel(m_labelY);
    chartValue->addGraph();
    chartValue->addGraph();

    chartValue->graph(0)->setLineStyle(QCPGraph::lsLine);
    chartValue->graph(1)->setLineStyle(QCPGraph::lsNone);
    chartValue->graph(1)->setScatterStyle(QCP::ssDisc);
    chartValue->graph(1)->setScatterSize(7.0);
    chartValue->graph(1)->setPen(QPen(Qt::gray));

    chartDerivative = new QCustomPlot(this);
    chartDerivative->setInteractions(QCustomPlot::iRangeDrag | QCustomPlot::iRangeZoom);
    chartDerivative->setRangeDrag(Qt::Horizontal|Qt::Vertical);
    chartDerivative->setRangeZoom(Qt::Horizontal|Qt::Vertical);
    chartDerivative->xAxis->setLabel(m_labelX);
    chartDerivative->yAxis->setLabel(QString("d%1/d%2").arg(m_labelY).arg(m_labelX));
    chartDerivative->addGraph();

    chartDerivative->graph(0)->setLineStyle(QCPGraph::lsLine);

    QVBoxLayout *chartLayout = new QVBoxLayout();
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


    radFirstDerivative = new QRadioButton(tr("First"));
    radSecondDerivative = new QRadioButton(tr("Second"));
    QButtonGroup *derivativeGroup = new QButtonGroup();
    derivativeGroup->addButton(radFirstDerivative);
    derivativeGroup->addButton(radSecondDerivative);
    radFirstDerivative->setChecked(m_table.splineFirstDerivatives());
    radSecondDerivative->setChecked(!m_table.splineFirstDerivatives());
    connect(radFirstDerivative, SIGNAL(clicked()), this, SLOT(doSplineDerivativeChanged()));
    connect(radSecondDerivative, SIGNAL(clicked()), this, SLOT(doSplineDerivativeChanged()));

    radExtrapolateConstant = new QRadioButton(tr("Constant"));
    radExtrapolateLinear = new QRadioButton(tr("Linear function"));
    QButtonGroup *extrapolatinGroup = new QButtonGroup();
    extrapolatinGroup->addButton(radExtrapolateConstant);
    extrapolatinGroup->addButton(radExtrapolateLinear);
    radExtrapolateConstant->setChecked(m_table.extrapolateConstant());
    radExtrapolateLinear->setChecked(!m_table.extrapolateConstant());
    connect(radExtrapolateConstant, SIGNAL(clicked()), this, SLOT(doExtrapolateChanged()));
    connect(radExtrapolateLinear, SIGNAL(clicked()), this, SLOT(doExtrapolateChanged()));

    QVBoxLayout *layoutView = new QVBoxLayout();
    layoutView->addWidget(chkMarkers);
    layoutView->addWidget(chkDerivative);
    layoutView->addWidget(chkExtrapolation);
    layoutView->addStretch();

    QGroupBox *grpView = new QGroupBox(tr("View"));
    grpView->setLayout(layoutView);

    QGridLayout *layoutType = new QGridLayout();
    layoutType->addWidget(new QLabel(tr("Interpolation")), 0, 0);
    layoutType->addWidget(cmbType, 0, 1);

    QGridLayout *layoutInterpolation = new QGridLayout();
    layoutInterpolation->addWidget(new QLabel(tr("Derivative to be zero at endpoints")), 0, 0, 1, 2);
    layoutInterpolation->addWidget(radFirstDerivative, 1, 0, 1, 1);
    layoutInterpolation->addWidget(radSecondDerivative, 1, 1, 1, 1);
    layoutInterpolation->addWidget(new QLabel(tr("Extrapolate as")), 2, 0, 1, 2);
    layoutInterpolation->addWidget(radExtrapolateConstant, 3, 0, 1, 1);
    layoutInterpolation->addWidget(radExtrapolateLinear, 3, 1, 1, 1);

    grpInterpolation = new QGroupBox(tr("Spline properties"));
    grpInterpolation->setLayout(layoutInterpolation);
    grpInterpolation->setEnabled(m_table.type() == DataTableType_CubicSpline);

    QVBoxLayout *layoutSettings = new QVBoxLayout();
    layoutSettings->addLayout(layoutType);
    layoutSettings->addWidget(grpInterpolation);
    layoutSettings->addWidget(grpView);

    QGridLayout *tableLayout = new QGridLayout();
    tableLayout->addWidget(lblLabelX, 0, 0);
    tableLayout->addWidget(lblInfoX, 0, 1, 1, 1, Qt::AlignRight);
    tableLayout->addWidget(lstX, 1, 0, 1, 2);
    tableLayout->addWidget(lblLabelY, 0, 2);
    tableLayout->addWidget(lblInfoY, 0, 3, 1, 1, Qt::AlignRight);
    tableLayout->addWidget(lstY, 1, 2, 1, 2);
    tableLayout->addWidget(lblInfoError, 2, 0, 1, 4);

    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->addLayout(tableLayout, 1);
    leftLayout->addLayout(layoutSettings);

    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->addLayout(leftLayout);
    controlsLayout->addLayout(chartLayout, 1);

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
    parseTable();

    // points
    int count = m_table.size();

    Hermes::vector<double> pointsVector = m_table.pointsVector();
    Hermes::vector<double> valuesVector = m_table.valuesVector();

    QVector<double> pointsMarkersVector;
    QVector<double> valuesMarkersVector;

    for (int i = 0; i < pointsVector.size(); i++)
    {
        pointsMarkersVector.append(pointsVector[i]);
        valuesMarkersVector.append(valuesVector[i]);
    }

    // interpolation
    int countSpline = count * 1e3;
    double keyLength = m_table.maxKey() - m_table.minKey();
    double keyStart = m_table.minKey();
    if (chkExtrapolation->isChecked())
    {
        double overlap = 0.15;
        keyStart -= keyLength * overlap;
        keyLength *= (1 + 2 * overlap);
    }

    double dx = keyLength / (countSpline - 1);

    QVector<double> keysSpline;
    QVector<double> valuesSpline;
    QVector<double> derivativesSpline;

    // spline
    for (int i = 0; i < countSpline; i++)
    {
        keysSpline.append(keyStart + (i * dx));
        valuesSpline.append(m_table.value(keysSpline[i]));
        derivativesSpline.append(m_table.derivative(keysSpline[i]));
    }

    chartValue->graph(0)->setData(keysSpline, valuesSpline);
    if (chkMarkers->isChecked())
        chartValue->graph(1)->setData(pointsMarkersVector, valuesMarkersVector);
    else
        chartValue->graph(1)->clearData();
    chartValue->rescaleAxes();
    chartValue->replot();

    chartDerivative->graph(0)->setData(keysSpline, derivativesSpline);
    chartDerivative->rescaleAxes();
    chartDerivative->replot();
}

void ValueDataTableDialog::doShowDerivativeClicked()
{
    chartDerivative->setVisible(chkDerivative->isChecked());
}

void ValueDataTableDialog::doTypeChanged()
{
    m_table.setType(DataTableType(cmbType->currentIndex()));
    grpInterpolation->setEnabled(m_table.type() == DataTableType_CubicSpline);
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
