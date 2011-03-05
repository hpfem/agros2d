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

#include "chartdialog.h"

#include "gui.h"
#include "scene.h"
#include "localvalueview.h"

ChartDialog::ChartDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("ChartDialog::ChartDialog()");

    setWindowIcon(icon("chart"));
    setWindowFlags(Qt::Window);
    setWindowTitle(tr("Chart"));

    createControls();

    resize(sizeHint());
    QSettings settings;
    restoreGeometry(settings.value("ChartDialog/Geometry", saveGeometry()).toByteArray());
    txtStartX->setValue(settings.value("ChartDialog/StartX", "0").toString());
    txtEndX->setValue(settings.value("ChartDialog/EndX", "0").toString());
    txtStartY->setValue(settings.value("ChartDialog/StartY", "0").toString());
    txtEndY->setValue(settings.value("ChartDialog/EndY", "0").toString());
    radAxisLength->setChecked(settings.value("ChartDialog/AxisLength", true).toBool());
    radAxisX->setChecked(settings.value("ChartDialog/AxisX", false).toBool());
    radAxisY->setChecked(settings.value("ChartDialog/AxisY", false).toBool());
    txtAxisPoints->setValue(settings.value("ChartDialog/AxisPoints", 200).toInt());
}

ChartDialog::~ChartDialog()
{
    logMessage("ChartDialog::~ChartDialog()");

    QSettings settings;
    settings.setValue("ChartDialog/Geometry", saveGeometry());
    settings.setValue("ChartDialog/StartX", txtStartX->value().text);
    settings.setValue("ChartDialog/EndX", txtEndX->value().text);
    settings.setValue("ChartDialog/StartY", txtStartY->value().text);
    settings.setValue("ChartDialog/EndY", txtEndY->value().text);
    settings.setValue("ChartDialog/AxisLength", radAxisLength->isChecked());
    settings.setValue("ChartDialog/AxisX", radAxisX->isChecked());
    settings.setValue("ChartDialog/AxisY", radAxisY->isChecked());
    settings.setValue("ChartDialog/AxisPoints", txtAxisPoints->value());
}

void ChartDialog::showDialog()
{
    logMessage("ChartDialog::showDialog()");

    fillComboBoxScalarVariable(cmbFieldVariable);
    fillComboBoxTimeStep(cmbTimeStep);

    // correct labels
    lblStartX->setText(Util::scene()->problemInfo()->labelX() + ":");
    lblStartY->setText(Util::scene()->problemInfo()->labelY() + ":");
    lblEndX->setText(Util::scene()->problemInfo()->labelX() + ":");
    lblEndY->setText(Util::scene()->problemInfo()->labelY() + ":");
    lblPointX->setText(Util::scene()->problemInfo()->labelX() + ":");
    lblPointY->setText(Util::scene()->problemInfo()->labelY() + ":");
    radAxisX->setText(Util::scene()->problemInfo()->labelX());
    radAxisY->setText(Util::scene()->problemInfo()->labelY());

    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        tabAnalysisType->setTabEnabled(tabAnalysisType->indexOf(widTime), true);
    }
    else
    {
        tabAnalysisType->setTabEnabled(tabAnalysisType->indexOf(widTime), false);
        tabAnalysisType->setCurrentWidget(widGeometry);
    }

    show();
    activateWindow();
    raise();
    doChartLine();
}

void ChartDialog::hideEvent(QHideEvent *event)
{
    logMessage("ChartDialog::hideEvent()");

    doChartLine();
}

void ChartDialog::createControls()
{
    logMessage("ChartDialog::createControls()");

    chart = new Chart(this);

    // controls
    QWidget *controls = new QWidget(this);

    QPushButton *btnPlot = new QPushButton(controls);
    btnPlot->setText(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(doPlot()));

    QPushButton *btnSaveImage = new QPushButton(controls);
    btnSaveImage->setText(tr("Save image"));
    connect(btnSaveImage, SIGNAL(clicked()), SLOT(doSaveImage()));

    QPushButton *btnExportData = new QPushButton(controls);
    btnExportData->setText(tr("Export"));
    connect(btnExportData, SIGNAL(clicked()), SLOT(doExportData()));

    // geometry
    lblStartX = new QLabel("X:");
    lblStartY = new QLabel("Y:");
    lblEndX = new QLabel("X:");
    lblEndY = new QLabel("Y:");

    txtStartX = new SLineEditValue();
    txtStartY = new SLineEditValue();
    txtEndX = new SLineEditValue();
    txtEndY = new SLineEditValue();

    connect(txtStartX, SIGNAL(editingFinished()), this, SLOT(doChartLine()));
    connect(txtStartY, SIGNAL(editingFinished()), this, SLOT(doChartLine()));
    connect(txtEndX, SIGNAL(editingFinished()), this, SLOT(doChartLine()));
    connect(txtEndY, SIGNAL(editingFinished()), this, SLOT(doChartLine()));

    // start
    QGridLayout *layoutStart = new QGridLayout();
    layoutStart->addWidget(lblStartX, 0, 0);
    layoutStart->addWidget(txtStartX, 0, 1);
    layoutStart->addWidget(lblStartY, 1, 0);
    layoutStart->addWidget(txtStartY, 1, 1);

    QGroupBox *grpStart = new QGroupBox(tr("Start"), this);
    grpStart->setLayout(layoutStart);

    // end
    QGridLayout *layoutEnd = new QGridLayout();
    layoutEnd->addWidget(lblEndX, 0, 0);
    layoutEnd->addWidget(txtEndX, 0, 1);
    layoutEnd->addWidget(lblEndY, 1, 0);
    layoutEnd->addWidget(txtEndY, 1, 1);

    QGroupBox *grpEnd = new QGroupBox(tr("End"), this);
    grpEnd->setLayout(layoutEnd);

    // x - axis
    radAxisLength = new QRadioButton(tr("Length"), this);
    radAxisLength->setChecked(true);
    radAxisX = new QRadioButton("X", this);
    radAxisY = new QRadioButton("Y", this);

    QButtonGroup *axisGroup = new QButtonGroup(this);
    axisGroup->addButton(radAxisLength);
    axisGroup->addButton(radAxisX);
    axisGroup->addButton(radAxisY);
    connect(radAxisLength, SIGNAL(clicked()), this, SLOT(doPlot()));
    connect(radAxisX, SIGNAL(clicked()), this, SLOT(doPlot()));
    connect(radAxisY, SIGNAL(clicked()), this, SLOT(doPlot()));

    // axis
    QHBoxLayout *layoutAxis = new QHBoxLayout(this);
    layoutAxis->addWidget(radAxisLength);
    layoutAxis->addWidget(radAxisX);
    layoutAxis->addWidget(radAxisY);

    QGroupBox *grpAxis = new QGroupBox(tr("Horizontal axis"), this);
    grpAxis->setLayout(layoutAxis);

    // axis points and time step
    txtAxisPoints = new QSpinBox(this);
    txtAxisPoints->setMinimum(2);
    txtAxisPoints->setMaximum(500);
    txtAxisPoints->setValue(200);

    // timestep
    cmbTimeStep = new QComboBox(this);
    connect(cmbTimeStep, SIGNAL(currentIndexChanged(int)), this, SLOT(doTimeStepChanged(int)));

    QFormLayout *layoutAxisPointsAndTimeStep = new QFormLayout();
    layoutAxisPointsAndTimeStep->addRow(tr("Points:"), txtAxisPoints);
    layoutAxisPointsAndTimeStep->addRow(tr("Time step:"), cmbTimeStep);

    QGroupBox *grpAxisPointsAndTimeStep = new QGroupBox(tr("Points and time step"), this);
    grpAxisPointsAndTimeStep->setLayout(layoutAxisPointsAndTimeStep);

    // time
    lblPointX = new QLabel("X:");
    lblPointY = new QLabel("Y:");
    txtPointX = new SLineEditValue();
    txtPointY = new SLineEditValue();

    QGridLayout *layoutTime = new QGridLayout();
    layoutTime->addWidget(lblPointX, 0, 0);
    layoutTime->addWidget(txtPointX, 0, 1);
    layoutTime->addWidget(lblPointY, 1, 0);
    layoutTime->addWidget(txtPointY, 1, 1);

    QGroupBox *grpTime = new QGroupBox(tr("Point"), this);
    grpTime->setLayout(layoutTime);

    // plot
    // variable
    cmbFieldVariable = new QComboBox(this);
    connect(cmbFieldVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(doFieldVariable(int)));
    // component
    cmbFieldVariableComp = new QComboBox(this);
    connect(cmbFieldVariableComp, SIGNAL(currentIndexChanged(int)), this, SLOT(doFieldVariableComp(int)));
    doFieldVariable(cmbFieldVariable->currentIndex());

    QFormLayout *layoutVariable = new QFormLayout(this);
    layoutVariable->addRow(tr("Variable:"), cmbFieldVariable);
    layoutVariable->addRow(tr("Component:"), cmbFieldVariableComp);

    QWidget *grpVariable = new QWidget(this);
    grpVariable->setLayout(layoutVariable);

    // table
    trvTable = new QTableWidget(this);

    // button bar
    QHBoxLayout *layoutButton = new QHBoxLayout();
    layoutButton->addStretch();
    layoutButton->addWidget(btnPlot);
    layoutButton->addWidget(btnSaveImage);
    layoutButton->addWidget(btnExportData);

    QWidget *widButton = new QWidget(this);
    widButton->setLayout(layoutButton);

    // controls geometry
    widGeometry = new QWidget(this);
    QVBoxLayout *controlsGeometryLayout = new QVBoxLayout();
    widGeometry->setLayout(controlsGeometryLayout);
    controlsGeometryLayout->addWidget(grpStart);
    controlsGeometryLayout->addWidget(grpEnd);
    controlsGeometryLayout->addWidget(grpAxis);
    controlsGeometryLayout->addWidget(grpAxisPointsAndTimeStep);
    controlsGeometryLayout->addStretch();

    // controls time
    widTime = new QWidget(this);
    QVBoxLayout *controlsTimeLayout = new QVBoxLayout();
    widTime->setLayout(controlsTimeLayout);
    controlsTimeLayout->addWidget(grpTime);
    controlsTimeLayout->addStretch();

    tabAnalysisType = new QTabWidget(this);
    tabAnalysisType->addTab(widGeometry, icon(""), tr("Geometry"));
    tabAnalysisType->addTab(widTime, icon(""), tr("Time"));

    // controls
    QVBoxLayout *controlsLayout = new QVBoxLayout();
    controls->setLayout(controlsLayout);
    controls->setMinimumWidth(280);

    controlsLayout->addWidget(tabAnalysisType);
    controlsLayout->addWidget(grpVariable);
    controlsLayout->addStretch();
    controlsLayout->addWidget(widButton);

    // tab data widget
    tabOutput = new QTabWidget(this);
    tabOutput->addTab(chart, icon(""), tr("Chart"));
    tabOutput->addTab(trvTable, icon(""), tr("Table"));

    // main layout
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(controls);
    layout->addWidget(tabOutput, 1);

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

    setLayout(layout);
}

void ChartDialog::plotGeometry()
{
    logMessage("ChartDialog::plotGeometry()");

    if (!txtStartX->evaluate()) return;
    if (!txtStartY->evaluate()) return;
    if (!txtEndX->evaluate()) return;
    if (!txtEndY->evaluate()) return;

    doChartLine();

    // variable
    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toInt();
    if (physicFieldVariable == PhysicFieldVariable_Undefined) return;
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt();
    if (physicFieldVariableComp == PhysicFieldVariableComp_Undefined) return;

    int count = txtAxisPoints->value();
    double *xval = new double[count];
    double *yval = new double[count];

    // chart->setTitle(physicFieldVariableString(physicFieldVariable) + " - " + physicFieldVariableCompString(physicFieldVariableComp));
    QwtText text("");
    text.setFont(QFont("Helvetica", 10, QFont::Normal));
    text.setText(QString("%1 (%2)").
                 arg(physicFieldVariableString(physicFieldVariable)).
                 arg(physicFieldVariableUnitsString(physicFieldVariable)));
    chart->setAxisTitle(QwtPlot::yLeft, text);

    // table
    trvTable->clear();
    trvTable->setRowCount(count);
    QStringList headers = Util::scene()->problemInfo()->hermes()->localPointValueHeader();
    trvTable->setColumnCount(headers.count());
    trvTable->setHorizontalHeaderLabels(headers);

    // chart
    if (radAxisLength->isChecked()) text.setText(tr("Length (m)"));
    if (radAxisX->isChecked()) text.setText(Util::scene()->problemInfo()->labelX() + " (m):");
    if (radAxisY->isChecked()) text.setText(Util::scene()->problemInfo()->labelY() + " (m):");
    chart->setAxisTitle(QwtPlot::xBottom, text);

    // line
    Point start(txtStartX->value().number, txtStartY->value().number);
    Point end(txtEndX->value().number, txtEndY->value().number);

    Point diff((end.x - start.x)/(count-1), (end.y - start.y)/(count-1));

    // calculate values
    QStringList row;
    for (int i = 0; i<count; i++)
    {
        Point point(start.x + i*diff.x, start.y + i*diff.y);
        LocalPointValue *localPointValue = Util::scene()->problemInfo()->hermes()->localPointValue(point);

        // x value
        if (radAxisLength->isChecked()) xval[i] = sqrt(sqr(i*diff.x) + sqr(i*diff.y));
        if (radAxisX->isChecked()) xval[i] = start.x + i*diff.x;
        if (radAxisY->isChecked()) xval[i] = start.y + i*diff.y;

        // y value
        yval[i] = localPointValue->variableValue(physicFieldVariable, physicFieldVariableComp);

        // table
        row.clear();
        row << localPointValue->variables();

        for (int j = 0; j<row.count(); j++)
            trvTable->setItem(i, j, new QTableWidgetItem(row.at(j)));

        delete localPointValue;
    }

    chart->setData(xval, yval, count);

    delete[] xval;
    delete[] yval;
}

void ChartDialog::plotTime()
{
    logMessage("ChartDialog::plotTime()");

    if (!txtPointX->evaluate()) return;
    if (!txtPointY->evaluate()) return;

    doChartLine();

    // variable
    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toInt();
    if (physicFieldVariable == PhysicFieldVariable_Undefined) return;
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt();
    if (physicFieldVariableComp == PhysicFieldVariableComp_Undefined) return;

    // store timestep
    int timeStep = Util::scene()->sceneSolution()->timeStep();

    int count = Util::scene()->sceneSolution()->timeStepCount();
    double *xval = new double[count];
    double *yval = new double[count];

    // chart->setTitle(physicFieldVariableString(physicFieldVariable) + " - " + physicFieldVariableCompString(physicFieldVariableComp));
    QwtText text("");
    text.setFont(QFont("Helvetica", 10, QFont::Normal));
    text.setText(QString("%1 (%2)").
                 arg(physicFieldVariableString(physicFieldVariable)).
                 arg(physicFieldVariableUnitsString(physicFieldVariable)));
    chart->setAxisTitle(QwtPlot::yLeft, text);

    // table
    trvTable->clear();
    trvTable->setRowCount(count);
    QStringList headers = Util::scene()->problemInfo()->hermes()->localPointValueHeader();
    trvTable->setColumnCount(headers.count());
    trvTable->setHorizontalHeaderLabels(headers);

    // chart
    text.setText(tr("Time (s)"));
    chart->setAxisTitle(QwtPlot::xBottom, text);

    // calculate values
    QStringList row;
    for (int i = 0; i<Util::scene()->sceneSolution()->timeStepCount(); i++)
    {
        // change time level
        Util::scene()->sceneSolution()->setTimeStep(i);

        Point point(txtPointX->value().number, txtPointY->value().number);
        LocalPointValue *localPointValue = Util::scene()->problemInfo()->hermes()->localPointValue(point);

        // x value
        xval[i] = Util::scene()->sceneSolution()->time();

        // y value
        yval[i] = localPointValue->variableValue(physicFieldVariable, physicFieldVariableComp);

        // table
        row.clear();
        row << localPointValue->variables();

        for (int j = 0; j<row.count(); j++)
            trvTable->setItem(i, j, new QTableWidgetItem(row.at(j)));

        delete localPointValue;
    }

    chart->setData(xval, yval, count);

    delete[] xval;
    delete[] yval;

    // restore previous timestep
    Util::scene()->sceneSolution()->setTimeStep(timeStep);
}

void ChartDialog::doPlot()
{
    logMessage("ChartDialog::doPlot()");

    if (!Util::scene()->sceneSolution()->isSolved()) return;

    if (tabAnalysisType->currentWidget() == widGeometry)
        plotGeometry();

    if (tabAnalysisType->currentWidget() == widTime)
        plotTime();
}

void ChartDialog::doFieldVariable(int index)
{
    logMessage("ChartDialog::doFieldVariable()");

    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbFieldVariable->itemData(index).toInt();

    cmbFieldVariableComp->clear();
    if (isPhysicFieldVariableScalar(physicFieldVariable))
    {
        cmbFieldVariableComp->addItem(tr("Scalar"), PhysicFieldVariableComp_Scalar);
    }
    else
    {
        cmbFieldVariableComp->addItem(tr("Magnitude"), PhysicFieldVariableComp_Magnitude);
        cmbFieldVariableComp->addItem(Util::scene()->problemInfo()->labelX(), PhysicFieldVariableComp_X);
        cmbFieldVariableComp->addItem(Util::scene()->problemInfo()->labelY(), PhysicFieldVariableComp_Y);
    }

    if (cmbFieldVariableComp->currentIndex() == -1)
        cmbFieldVariableComp->setCurrentIndex(0);

    if (isVisible()) doPlot();
}

void ChartDialog::doFieldVariableComp(int index)
{
    logMessage("ChartDialog::doFieldVariableComp()");

    if (isVisible()) doPlot();
}

void ChartDialog::doSaveImage()
{
    logMessage("ChartDialog::doSaveImage()");

    chart->saveImage();
}

void ChartDialog::doExportData()
{
    logMessage("ChartDialog::doExportData()");

    QSettings settings;
    QString dir = settings.value("General/LastDataDir").toString();

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export data to file"), dir, tr("CSV files (*.csv);;Matlab/Octave script (*.m)"), &selectedFilter);
    if (!fileName.isEmpty())
    {
        QString ext = (selectedFilter.contains("CSV")) ? ".csv" : ".m";
        QFileInfo fileInfo(fileName);

        // open file for write
        if (fileInfo.suffix().isEmpty())
            fileName = fileName + ext;

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            cerr << "Could not create " + fileName.toStdString() + " file." << endl;
            return;
        }
        QTextStream out(&file);

        // export
        // csv
        if (fileInfo.suffix().toLower() == "csv")
        {
            // header
            for (int j = 0; j < trvTable->columnCount(); j++)
                out << trvTable->horizontalHeaderItem(j)->text() << ";";
            out << endl;

            // items
            for (int i = 0; i < trvTable->rowCount(); i++)
            {
                for (int j = 0; j < trvTable->columnCount(); j++)
                    out << trvTable->item(i, j)->text()  << ";";
                out << endl;
            }
        }

        // m-file
        if (fileInfo.suffix().toLower() == "m")
        {
            // items
            for (int j = 0; j < trvTable->columnCount(); j++)
            {
                out << trvTable->horizontalHeaderItem(j)->text().replace(" ", "_") << " = [";
                for (int i = 0; i < trvTable->rowCount(); i++)
                    out << trvTable->item(i, j)->text().replace(",", ".") << ((i <= trvTable->rowCount()-2) ? ", " : "");
                out << "];" << endl;
            }

            // example
            out << endl << endl;
            out << "% example" << endl;
            out << "% plot(sqrt(X.^2 + Y.^2), " << trvTable->horizontalHeaderItem(2)->text().replace(" ", "_") << ");" << endl;
            out << "% grid on;" << endl;
            out << "% xlabel('length (m)');" << endl;
            out << "% ylabel('" << trvTable->horizontalHeaderItem(2)->text() << "');" << endl;
        }

        settings.setValue("General/LastDataDir", fileInfo.absolutePath());

        file.close();
    }
}

void ChartDialog::doMoved(const QPoint &pos)
{
    logMessage("ChartDialog::doMoved()");

    QString info;
    info.sprintf("x=%g, y=%g", chart->invTransform(QwtPlot::xBottom, pos.x()), chart->invTransform(QwtPlot::yLeft, pos.y()));
}

void ChartDialog::doChartLine()
{
    logMessage("ChartDialog::doChartLine()");

    if (isVisible())
    {
        if (tabAnalysisType->currentWidget() == widGeometry)
        {
            if (!txtStartX->evaluate()) return;
            if (!txtStartY->evaluate()) return;
            if (!txtEndX->evaluate()) return;
            if (!txtEndY->evaluate()) return;

            emit setChartLine(Point(txtStartX->value().number, txtStartY->value().number), Point(txtEndX->value().number, txtEndY->value().number));
        }
        if (tabAnalysisType->currentWidget() == widTime)
        {
            if (!txtPointX->evaluate()) return;
            if (!txtPointY->evaluate()) return;

            emit setChartLine(Point(txtPointX->value().number, txtPointY->value().number), Point(txtPointX->value().number, txtPointY->value().number));
        }
    }
    else
    {
        emit setChartLine(Point(), Point());
    }
}

void ChartDialog::doTimeStepChanged(int index)
{
    logMessage("ChartDialog::doTimeStepChanged()");

    if (cmbTimeStep->currentIndex() != -1)
    {
        Util::scene()->sceneSolution()->setTimeStep(cmbTimeStep->currentIndex());
        doPlot();
    }
}
