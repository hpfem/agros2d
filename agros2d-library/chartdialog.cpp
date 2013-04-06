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

#include "util/global.h"

#include "scene.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "sceneview_geometry_chart.h"

#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/solutionstore.h"
#include "hermes2d/problem_config.h"
#include "pythonlab/pythonengine_agros.h"

#include "gui/common.h"
#include "gui/lineeditdouble.h"
#include "gui/physicalfield.h"

#include <QSvgRenderer>
#include "qcustomplot/qcustomplot.h"

QList<Point> ChartLine::getPoints()
{
    if (numberOfPoints == 0)
        return QList<Point>();

    QList<Point> points;
    points.reserve(numberOfPoints);

    double dx = (end.x - start.x) / (numberOfPoints - 1);
    double dy = (end.y - start.y) / (numberOfPoints - 1);

    for (int i = 0; i < numberOfPoints; i++)
        if (reverse)
            points.insert(0, Point(start.x + i*dx, start.y + i*dy));
        else
            points.append(Point(start.x + i*dx, start.y + i*dy));

    return points;
}

// **************************************************************************************************

ChartView::ChartView(QWidget *parent) : QWidget(parent)
{
    actSceneModeChart = new QAction(icon("chart"), tr("Chart"), this);
    actSceneModeChart->setShortcut(tr("Ctrl+6"));
    actSceneModeChart->setCheckable(true);

    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(setControls()));
    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(setControls()));

    m_chart = new QCustomPlot(this);
    m_chart->setInteractions(QCustomPlot::iRangeDrag | QCustomPlot::iRangeZoom);
    m_chart->setRangeDrag(Qt::Horizontal|Qt::Vertical);
    m_chart->setRangeZoom(Qt::Horizontal|Qt::Vertical);
    m_chart->addGraph();

    m_chart->graph(0)->setLineStyle(QCPGraph::lsLine);

    QHBoxLayout *layoutMain = new QHBoxLayout();
    layoutMain->addWidget(m_chart);

    setLayout(layoutMain);

    setControls();
}

void ChartView::setControls()
{
    actSceneModeChart->setEnabled(Agros2D::problem()->isSolved());
}

// **************************************************************************************************

ChartWidget::ChartWidget(ChartView *chart,
                         QWidget *parent) : QWidget(parent), m_chart(chart)
{
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(updateControls()));

    createControls();

    updateControls();
}

ChartWidget::~ChartWidget()
{
}

void ChartWidget::updateControls()
{
    if (!Agros2D::problem()->isSolved())
        return;

    fieldWidget->updateControls();

    // correct labels
    lblStartX->setText(Agros2D::problem()->config()->labelX() + ":");
    lblStartY->setText(Agros2D::problem()->config()->labelY() + ":");
    lblEndX->setText(Agros2D::problem()->config()->labelX() + ":");
    lblEndY->setText(Agros2D::problem()->config()->labelY() + ":");
    lblPointX->setText(Agros2D::problem()->config()->labelX() + ":");
    lblPointY->setText(Agros2D::problem()->config()->labelY() + ":");
    radHorizontalAxisX->setText(Agros2D::problem()->config()->labelX());
    radHorizontalAxisY->setText(Agros2D::problem()->config()->labelY());

    if (Agros2D::problem()->isTransient())
    {
        widTime->setEnabled(true);
    }
    else
    {
        widTime->setEnabled(false);
        widGeometry->setEnabled(true);
        tbxAnalysisType->setCurrentWidget(widGeometry);
    }

    txtStartX->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ChartStartX).toDouble());
    txtStartY->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ChartStartY).toDouble());
    txtEndX->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ChartEndX).toDouble());
    txtEndY->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ChartEndY).toDouble());
    txtTimeX->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ChartTimeX).toDouble());
    txtTimeY->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ChartTimeY).toDouble());
    radHorizontalAxisX->setChecked((ChartAxisType) Agros2D::problem()->setting()->value(ProblemSetting::View_ChartHorizontalAxis).toInt() == ChartAxis_X);
    radHorizontalAxisY->setChecked((ChartAxisType) Agros2D::problem()->setting()->value(ProblemSetting::View_ChartHorizontalAxis).toInt() == ChartAxis_Y);
    radHorizontalAxisLength->setChecked((ChartAxisType) Agros2D::problem()->setting()->value(ProblemSetting::View_ChartHorizontalAxis).toInt() == ChartAxis_Length);
    txtHorizontalAxisPoints->setValue(Agros2D::problem()->setting()->value(ProblemSetting::View_ChartHorizontalAxisPoints).toInt());
    chkHorizontalAxisReverse->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_ChartHorizontalAxisReverse).toBool());

    createChartLine();
}

void ChartWidget::createControls()
{
    fieldWidget = new PhysicalFieldWidget(this);
    connect(fieldWidget, SIGNAL(fieldChanged()), this, SLOT(doField()));

    // variable
    cmbFieldVariable = new QComboBox();
    connect(cmbFieldVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(doFieldVariable(int)));

    // component
    cmbFieldVariableComp = new QComboBox();

    QFormLayout *layoutVariable = new QFormLayout();
    layoutVariable->addRow(tr("Variable:"), cmbFieldVariable);
    layoutVariable->addRow(tr("Component:"), cmbFieldVariableComp);

    QGroupBox *grpVariable = new QGroupBox(tr("Variable"));
    grpVariable->setLayout(layoutVariable);

    // viewer
    geometryViewer = new SceneViewPreprocessorChart(this);
    geometryViewer->setMinimumHeight(150);
    // geometryViewer->setMaximumHeight(150);

    QVBoxLayout *layoutChart = new QVBoxLayout();
    layoutChart->addWidget(geometryViewer);

    QGroupBox *grpChart = new QGroupBox(tr("Line preview"));
    grpChart->setLayout(layoutChart);

    // controls
    btnOK = new QPushButton();
    btnOK->setDefault(false);
    btnOK->setText(tr("Apply"));
    connect(btnOK, SIGNAL(clicked()), this, SLOT(doApply()));

    btnSaveImage = new QPushButton();
    btnSaveImage->setDefault(false);
    btnSaveImage->setEnabled(false);
    btnSaveImage->setText(tr("Save image"));
    connect(btnSaveImage, SIGNAL(clicked()), this, SLOT(doSaveImage()));

    btnExportData = new QPushButton();
    btnExportData->setDefault(false);
    btnExportData->setEnabled(false);
    btnExportData->setText(tr("Export"));
    connect(btnExportData, SIGNAL(clicked()), this, SLOT(doExportData()));

    // geometry
    lblStartX = new QLabel("X:");
    lblStartY = new QLabel("Y:");
    lblEndX = new QLabel("X:");
    lblEndY = new QLabel("Y:");

    txtStartX = new LineEditDouble(0.0, true);
    txtStartY = new LineEditDouble(0.0, true);
    connect(txtStartX, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));
    connect(txtStartY, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));

    txtEndX = new LineEditDouble(0.0, true);
    txtEndY = new LineEditDouble(0.0, true);
    connect(txtEndX, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));
    connect(txtEndY, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));

    // start
    QGridLayout *layoutStart = new QGridLayout();
    layoutStart->addWidget(lblStartX, 0, 0);
    layoutStart->addWidget(txtStartX, 0, 1);
    layoutStart->addWidget(lblStartY, 0, 2);
    layoutStart->addWidget(txtStartY, 0, 3);

    QGroupBox *grpStart = new QGroupBox(tr("Start"));
    grpStart->setLayout(layoutStart);

    // end
    QGridLayout *layoutEnd = new QGridLayout();
    layoutEnd->addWidget(lblEndX, 0, 0);
    layoutEnd->addWidget(txtEndX, 0, 1);
    layoutEnd->addWidget(lblEndY, 0, 2);
    layoutEnd->addWidget(txtEndY, 0, 3);

    QGroupBox *grpEnd = new QGroupBox(tr("End"));
    grpEnd->setLayout(layoutEnd);

    // x - axis
    radHorizontalAxisLength = new QRadioButton(tr("Length"));
    radHorizontalAxisLength->setChecked(true);
    radHorizontalAxisX = new QRadioButton("X");
    radHorizontalAxisY = new QRadioButton("Y");

    QButtonGroup *axisGroup = new QButtonGroup();
    axisGroup->addButton(radHorizontalAxisLength);
    axisGroup->addButton(radHorizontalAxisX);
    axisGroup->addButton(radHorizontalAxisY);

    /*
    connect(radAxisLength, SIGNAL(clicked()), this, SLOT(doPlot()));
    connect(radAxisX, SIGNAL(clicked()), this, SLOT(doPlot()));
    connect(radAxisY, SIGNAL(clicked()), this, SLOT(doPlot()));
    */

    // axis
    QHBoxLayout *layoutAxis = new QHBoxLayout();
    layoutAxis->addWidget(radHorizontalAxisLength);
    layoutAxis->addWidget(radHorizontalAxisX);
    layoutAxis->addWidget(radHorizontalAxisY);

    QGroupBox *grpAxis = new QGroupBox(tr("Horizontal axis"));
    grpAxis->setLayout(layoutAxis);

    // axis points and time step
    txtHorizontalAxisPoints = new QSpinBox(this);
    txtHorizontalAxisPoints->setMinimum(2);
    txtHorizontalAxisPoints->setMaximum(500);
    txtHorizontalAxisPoints->setValue(200);
    chkHorizontalAxisReverse = new QCheckBox(tr("Reverse"));
    //connect(chkAxisPointsReverse, SIGNAL(clicked()), this, SLOT(doPlot()));

    // timestep
    QGridLayout *layoutAxisPointsAndTimeStep = new QGridLayout();
    layoutAxisPointsAndTimeStep->setColumnStretch(1, 1);
    layoutAxisPointsAndTimeStep->addWidget(new QLabel(tr("Points:")), 0, 0);
    layoutAxisPointsAndTimeStep->addWidget(txtHorizontalAxisPoints, 0, 1);
    layoutAxisPointsAndTimeStep->addWidget(chkHorizontalAxisReverse, 1, 0, 1, 2);

    QGroupBox *grpAxisPointsAndTimeStep = new QGroupBox(tr("Points and time step"), this);
    grpAxisPointsAndTimeStep->setLayout(layoutAxisPointsAndTimeStep);

    // time
    lblPointX = new QLabel("X:");
    lblPointY = new QLabel("Y:");

    txtTimeX = new LineEditDouble(0.0, true);
    txtTimeY = new LineEditDouble(0.0, true);
    connect(txtTimeX, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));
    connect(txtTimeY, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));

    QGridLayout *layoutTime = new QGridLayout();
    layoutTime->addWidget(lblPointX, 0, 0);
    layoutTime->addWidget(txtTimeX, 0, 1);
    layoutTime->addWidget(lblPointY, 1, 0);
    layoutTime->addWidget(txtTimeY, 1, 1);

    QGroupBox *grpTime = new QGroupBox(tr("Point"));
    grpTime->setLayout(layoutTime);

    // button bar
    QHBoxLayout *layoutButton = new QHBoxLayout();
    layoutButton->setContentsMargins(2, 0, 0, 0);
    layoutButton->addStretch();
    layoutButton->addWidget(btnSaveImage);
    layoutButton->addWidget(btnExportData);
    layoutButton->addWidget(btnOK);

    QWidget *widButton = new QWidget();
    widButton->setLayout(layoutButton);

    // controls geometry
    widGeometry = new QWidget();
    QVBoxLayout *controlsGeometryLayout = new QVBoxLayout();
    widGeometry->setLayout(controlsGeometryLayout);
    controlsGeometryLayout->addWidget(grpStart);
    controlsGeometryLayout->addWidget(grpEnd);
    controlsGeometryLayout->addWidget(grpAxis);
    controlsGeometryLayout->addWidget(grpAxisPointsAndTimeStep);
    controlsGeometryLayout->addStretch();

    // controls time
    widTime = new QWidget();
    QVBoxLayout *controlsTimeLayout = new QVBoxLayout();
    widTime->setLayout(controlsTimeLayout);
    controlsTimeLayout->addWidget(grpTime);
    controlsTimeLayout->addStretch();

    tbxAnalysisType = new QTabWidget();
    tbxAnalysisType->addTab(widGeometry, icon(""), tr("Geometry"));
    tbxAnalysisType->addTab(widTime, icon(""), tr("Time"));

    // controls
    QVBoxLayout *controlsLayout = new QVBoxLayout();
    controlsLayout->setContentsMargins(0, 0, 0, 0);
    controlsLayout->addWidget(fieldWidget);
    controlsLayout->addWidget(grpVariable);
    controlsLayout->addWidget(tbxAnalysisType, 0.5);
    controlsLayout->addWidget(grpChart, 0.5);
    // controlsLayout->addStretch(1);

    QWidget *widget = new QWidget(this);
    widget->setLayout(controlsLayout);

    QScrollArea *widgetArea = new QScrollArea();
    widgetArea->setFrameShape(QFrame::NoFrame);
    widgetArea->setWidgetResizable(true);
    widgetArea->setWidget(widget);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(2, 2, 2, 3);
    layout->addWidget(widgetArea);
    layout->addWidget(widButton);

    setLayout(layout);
}

QVector<double> ChartWidget::horizontalAxisValues(ChartLine *chartLine)
{
    QList<Point> points = chartLine->getPoints();
    QVector<double> xval;

    if (radHorizontalAxisLength->isChecked())
    {
        for (int i = 0; i < points.length(); i++)
        {
            if (i == 0)
                xval.append(0.0);
            else
                xval.append(xval.at(i-1) + sqrt(Hermes::sqr(points.at(i).x - points.at(i-1).x) + Hermes::sqr(points.at(i).y - points.at(i-1).y)));
        }
    }
    else if (radHorizontalAxisX->isChecked())
    {
        foreach (Point point, points)
            xval.append(point.x);
    }
    else if (radHorizontalAxisY->isChecked())
    {
        foreach (Point point, points)
            xval.append(point.y);
    }

    return xval;
}

void ChartWidget::plotGeometry()
{
    // variable
    Module::LocalVariable physicFieldVariable = fieldWidget->selectedField()->localVariable(cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString());

    // variable component
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt();
    if (physicFieldVariableComp == PhysicFieldVariableComp_Undefined) return;

    // number of points
    int count = txtHorizontalAxisPoints->value();

    // chart
    m_chart->chart()->xAxis->setLabel(QString("%1 (%2)").
                                      arg(physicFieldVariable.name()).
                                      arg(physicFieldVariable.unit()));

    QString text;
    if (radHorizontalAxisLength->isChecked()) text = tr("Length (m)");
    if (radHorizontalAxisX->isChecked()) text = Agros2D::problem()->config()->labelX() + " (m)";
    if (radHorizontalAxisY->isChecked()) text = Agros2D::problem()->config()->labelY() + " (m)";
    m_chart->chart()->yAxis->setLabel(text);

    // table
    QStringList head = headers();

    // values
    ChartLine chartLine(Point(txtStartX->value(), txtStartY->value()),
                        Point(txtEndX->value(), txtEndY->value()),
                        count);
    createChartLine();

    QList<Point> points = chartLine.getPoints();
    QVector<double> xval = horizontalAxisValues(&chartLine);
    QVector<double> yval;

    foreach (Module::LocalVariable variable, fieldWidget->selectedField()->localPointVariables())
    {
        if (physicFieldVariable.id() != variable.id()) continue;

        foreach (Point point, points)
        {
            LocalValue *localValue = fieldWidget->selectedField()->plugin()->localValue(fieldWidget->selectedField(),
                                                                                        fieldWidget->selectedTimeStep(),
                                                                                        fieldWidget->selectedAdaptivityStep(),
                                                                                        fieldWidget->selectedAdaptivitySolutionType(),
                                                                                        point);
            QMap<QString, PointValue> values = localValue->values();

            if (variable.isScalar())
            {
                yval.append(values[variable.id()].scalar);
            }
            else
            {
                if (physicFieldVariableComp == PhysicFieldVariableComp_X)
                    yval.append(values[variable.id()].vector.x);
                else if (physicFieldVariableComp == PhysicFieldVariableComp_Y)
                    yval.append(values[variable.id()].vector.y);
                else
                    yval.append(values[variable.id()].vector.magnitude());
            }

            delete localValue;
        }
    }

    assert(xval.count() == yval.count());

    // reverse x axis
    if (chkHorizontalAxisReverse->isChecked())
    {
        for (int i = 0; i < points.length() / 2; i++)
        {
            double tmp = yval[i];
            yval[i] = yval[points.length() - i - 1];
            yval[points.length() - i - 1] = tmp;
        }
    }

    m_chart->chart()->graph(0)->setData(xval, yval);
    m_chart->chart()->rescaleAxes();
    m_chart->chart()->replot();
}

void ChartWidget::plotTime()
{
    // variable
    Module::LocalVariable physicFieldVariable = fieldWidget->selectedField()->localVariable(cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString());

    // variable comp
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt();
    if (physicFieldVariableComp == PhysicFieldVariableComp_Undefined) return;

    // time levels
    QList<double> timeLevels = Agros2D::solutionStore()->timeLevels(fieldWidget->selectedField());

    // chart
    m_chart->chart()->xAxis->setLabel(QString("%1 (%2)").
                                      arg(physicFieldVariable.name()).
                                      arg(physicFieldVariable.unit()));

    m_chart->chart()->yAxis->setLabel(tr("time (s)"));

    // table
    QVector<double> xval;
    QVector<double> yval;

    createChartLine();

    foreach (Module::LocalVariable variable, fieldWidget->selectedField()->localPointVariables())
    {
        if (physicFieldVariable.id() != variable.id()) continue;

        for (int i = 0; i < timeLevels.count(); i++)
        {
            // change time level
            xval.append(timeLevels.at(i));

            Point point(txtTimeX->value(), txtTimeY->value());
            int timeLevelIndex = Agros2D::solutionStore()->nthCalculatedTimeStep(fieldWidget->selectedField(), i);
            LocalValue *localValue = fieldWidget->selectedField()->plugin()->localValue(fieldWidget->selectedField(),
                                                                                        timeLevelIndex,
                                                                                        Agros2D::solutionStore()->lastAdaptiveStep(fieldWidget->selectedField(), SolutionMode_Normal, timeLevelIndex),
                                                                                        SolutionMode_Normal,
                                                                                        point);
            QMap<QString, PointValue> values = localValue->values();

            if (variable.isScalar())
                yval.append(values[variable.id()].scalar);
            else
            {
                if (physicFieldVariableComp == PhysicFieldVariableComp_X)
                    yval.append(values[variable.id()].vector.x);
                else if (physicFieldVariableComp == PhysicFieldVariableComp_Y)
                    yval.append(values[variable.id()].vector.y);
                else
                    yval.append(values[variable.id()].vector.magnitude());
            }

            delete localValue;
        }
    }

    m_chart->chart()->graph(0)->setData(xval, yval);
    m_chart->chart()->rescaleAxes();
    m_chart->chart()->replot();
}

QStringList ChartWidget::headers()
{
    QStringList head;
    head << "x" << "y" << "t";

    foreach (Module::LocalVariable variable, fieldWidget->selectedField()->localPointVariables())
    {
        if (variable.isScalar())
        {
            // scalar variable
            head.append(variable.shortname());
        }
        else
        {
            // vector variable
            head.append(variable.shortname() + Agros2D::problem()->config()->labelX().toLower());
            head.append(variable.shortname() + Agros2D::problem()->config()->labelY().toLower());
            head.append(variable.shortname());
        }
    }

    return head;
}

void ChartWidget::doApply()
{
    if (!Agros2D::problem()->isSolved())
        return;

    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ChartStartX, txtStartX->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ChartStartY, txtStartY->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ChartEndX, txtEndX->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ChartEndY, txtEndY->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ChartTimeX, txtTimeX->value());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ChartTimeY, txtTimeY->value());
    if (radHorizontalAxisX->isChecked())
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ChartHorizontalAxis, ChartAxis_X);
    else if (radHorizontalAxisY->isChecked())
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ChartHorizontalAxis, ChartAxis_Y);
    else if (radHorizontalAxisLength->isChecked())
        Agros2D::problem()->setting()->setValue(ProblemSetting::View_ChartHorizontalAxis, ChartAxis_Length);
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ChartHorizontalAxisReverse, chkHorizontalAxisReverse->isChecked());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_ChartHorizontalAxisPoints, txtHorizontalAxisPoints->value());

    if (tbxAnalysisType->currentWidget() == widGeometry)
    {
        plotGeometry();
    }

    if (tbxAnalysisType->currentWidget() == widTime)
    {
        plotTime();
    }

    btnSaveImage->setEnabled(m_chart->chart()->graph()->data()->size() > 0);
    btnExportData->setEnabled(m_chart->chart()->graph()->data()->size() > 0);
}

void ChartWidget::doField()
{
    fillComboBoxScalarVariable(fieldWidget->selectedField(), cmbFieldVariable);
    doFieldVariable(cmbFieldVariable->currentIndex());
}

void ChartWidget::doFieldVariable(int index)
{
    if (!fieldWidget->selectedField())
        return;

    Module::LocalVariable physicFieldVariable = fieldWidget->selectedField()->localVariable(cmbFieldVariable->itemData(index).toString());

    cmbFieldVariableComp->clear();
    if (physicFieldVariable.isScalar())
    {
        cmbFieldVariableComp->addItem(tr("Scalar"), PhysicFieldVariableComp_Scalar);
    }
    else
    {
        cmbFieldVariableComp->addItem(tr("Magnitude"), PhysicFieldVariableComp_Magnitude);
        cmbFieldVariableComp->addItem(Agros2D::problem()->config()->labelX(), PhysicFieldVariableComp_X);
        cmbFieldVariableComp->addItem(Agros2D::problem()->config()->labelY(), PhysicFieldVariableComp_Y);
    }

    if (cmbFieldVariableComp->currentIndex() == -1)
        cmbFieldVariableComp->setCurrentIndex(0);
}

void ChartWidget::doExportData()
{
    QSettings settings;
    QString dir = settings.value("General/LastDataDir").toString();

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export data to file"), dir, tr("CSV files (*.csv)"), &selectedFilter);
    if (fileName.isEmpty())
    {
        cerr << "Incorrect file name." << endl;
        return;
    }

    QFileInfo fileInfo(fileName);

    // open file for write
    if (fileInfo.suffix().isEmpty())
        fileName = fileName + ".csv";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        cerr << "Could not create " + fileName.toStdString() + " file." << endl;
        return;
    }
    QTextStream out(&file);

    QMap<QString, QList<double> > table;
    if (tbxAnalysisType->currentWidget() == widGeometry)
    {
        ChartLine *chartLine = new ChartLine(Point(txtStartX->value(), txtStartY->value()),
                                             Point(txtEndX->value(), txtEndY->value()),
                                             txtHorizontalAxisPoints->value());

        foreach (Point point, chartLine->getPoints())
        {
            QMap<QString, double> data = getData(point,
                                                 fieldWidget->selectedTimeStep(),
                                                 fieldWidget->selectedAdaptivityStep(),
                                                 fieldWidget->selectedAdaptivitySolutionType());
            foreach (QString key, data.keys())
            {
                QList<double> *values = &table.operator [](key);
                values->append(data.value(key));
            }
        }

        delete chartLine;
    }
    else if (tbxAnalysisType->currentWidget() == widTime)
    {
        /*
        Point point(txtTimeX->value(), txtTimeY->value());
        foreach (double timeLevel, Agros2D::solutionStore()->timeLevels(fieldWidget->selectedField()))
        {
            int timeStep = Agros2D::solutionStore()->timeLevelIndex(fieldWidget->selectedField(), timeLevel);
            QMap<QString, double> data = getData(point, timeStep);
            foreach (QString key, data.keys())
            {
                QList<double> *values = &table.operator [](key);
                values->append(data.value(key));
            }
        }
        */
    }

    // csv
    // headers
    foreach(QString key, table.keys())
        out << key << ";";
    out << "\n";

    // values
    for (int i = 0; i < table.values().first().size(); i++)
    {
        foreach(QString key, table.keys())
            out << QString::number(table.value(key).at(i)) << ";";
        out << endl;
    }

    if (fileInfo.absoluteDir() != tempProblemDir())
        settings.setValue("General/LastDataDir", fileInfo.absolutePath());

    file.close();
}

void ChartWidget::doSaveImage()
{
    QSettings settings;
    QString dir = settings.value("General/LastDataDir").toString();

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save image"), dir, tr("PNG files (*.png)"), &selectedFilter);
    if (fileName.isEmpty())
    {
        cerr << "Incorrect file name." << endl;
        return;
    }

    QFileInfo fileInfo(fileName);

    // open file for write
    if (fileInfo.suffix().isEmpty())
        fileName = fileName + ".png";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        cerr << "Could not create " + fileName.toStdString() + " file." << endl;
        return;
    }

    m_chart->chart()->savePng(fileName, 1024, 768);
}

QMap<QString, double> ChartWidget::getData(Point point, int timeStep, int adaptivityStep, SolutionMode solutionType)
{
    QMap<QString, double> table;

    foreach (Module::LocalVariable variable, fieldWidget->selectedField()->localPointVariables())
    {
        LocalValue *localValue = fieldWidget->selectedField()->plugin()->localValue(fieldWidget->selectedField(),
                                                                                    timeStep,
                                                                                    adaptivityStep,
                                                                                    solutionType,
                                                                                    point);
        QMap<QString, PointValue> values = localValue->values();

        if (variable.isScalar())
        {
            table.insert(variable.shortname(), values[variable.id()].scalar);
        }
        else
        {
            table.insert(QString(variable.shortname()), values[variable.id()].vector.magnitude());
            table.insert(QString(variable.shortname() + "x"), values[variable.id()].vector.x);
            table.insert(QString(variable.shortname() + "y"), values[variable.id()].vector.y);
        }

        delete localValue;
    }

    table.insert(Agros2D::problem()->config()->labelX(), point.x);
    table.insert(Agros2D::problem()->config()->labelY(), point.y);
    table.insert("t", Agros2D::solutionStore()->timeLevel(fieldWidget->selectedField(), timeStep));

    return table;
}

void ChartWidget::createChartLine()
{
    ChartLine line;

    if (tbxAnalysisType->currentWidget() == widGeometry)
    {
        line = ChartLine(Point(txtStartX->value(), txtStartY->value()),
                         Point(txtEndX->value(), txtEndY->value()),
                         txtHorizontalAxisPoints->value(),
                         chkHorizontalAxisReverse->isChecked());
    }
    if (tbxAnalysisType->currentWidget() == widTime)
    {
        line = ChartLine(Point(txtTimeX->value(), txtTimeY->value()),
                         Point(txtTimeX->value(), txtTimeY->value()),
                         0.0,
                         0);
    }

    geometryViewer->setChartLine(line);
    geometryViewer->doZoomBestFit();
}
