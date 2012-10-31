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

#include "scene.h"
#include "sceneview_post2d.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/solutionstore.h"

#include "gui/chart.h"
#include "gui/common.h"

QList<Point> ChartLine::getPoints()
{
    if (numberOfPoints == 0)
        return QList<Point>();

    QList<Point> points;
    points.reserve(numberOfPoints);

    if (fabs(angle) < EPS_ZERO)
    {
        double dx = (end.x - start.x) / (numberOfPoints - 1);
        double dy = (end.y - start.y) / (numberOfPoints - 1);

        for (int i = 0; i < numberOfPoints; i++)
            if (reverse)
                points.insert(0, Point(start.x + i*dx, start.y + i*dy));
            else
                points.append(Point(start.x + i*dx, start.y + i*dy));
    }
    else
    {
        Point center = centerPoint(start, end, angle);
        double radius = (start - center).magnitude();
        double startAngle = atan2(center.y - start.y, center.x - start.x) / M_PI*180 - 180;
        double theta = angle / double(numberOfPoints - 1);

        for (int i = 0; i < numberOfPoints; i++)
        {
            double arc = (startAngle + i*theta)/180.0*M_PI;

            double x = radius * cos(arc);
            double y = radius * sin(arc);

            if (reverse)
                points.insert(0, Point(center.x + x, center.y + y));
            else
                points.append(Point(center.x + x, center.y + y));
        }
    }

    return points;
}

// **************************************************************************************************


ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent)
{
    m_chart = new Chart(this, true);

    QHBoxLayout *layoutMain = new QHBoxLayout();
    layoutMain->addWidget(m_chart);

    setLayout(layoutMain);
}

// **************************************************************************************************

ChartControlsWidget::ChartControlsWidget(SceneViewPost2D *sceneView,
                         Chart *chart,
                         QWidget *parent) : QWidget(parent), m_sceneViewPost2D(sceneView), m_chart(chart)
{
    connect(this, SIGNAL(setChartLine(ChartLine)), m_sceneViewPost2D, SLOT(setChartLine(ChartLine)));
    connect(Util::problem(), SIGNAL(solved()), this, SLOT(setControls()));

    createActions();
    createControls();

    QSettings settings;
    txtStartX->setValue(settings.value("ChartDialog/StartX", "0").toString());
    txtEndX->setValue(settings.value("ChartDialog/EndX", "0").toString());
    txtStartY->setValue(settings.value("ChartDialog/StartY", "0").toString());
    txtEndY->setValue(settings.value("ChartDialog/EndY", "0").toString());
    txtAngle->setValue(settings.value("ChartDialog/Angle", "0").toString());
    radAxisLength->setChecked(settings.value("ChartDialog/AxisLength", true).toBool());
    radAxisX->setChecked(settings.value("ChartDialog/AxisX", false).toBool());
    radAxisY->setChecked(settings.value("ChartDialog/AxisY", false).toBool());
    txtAxisPoints->setValue(settings.value("ChartDialog/AxisPoints", 200).toInt());
    chkAxisPointsReverse->setChecked(settings.value("ChartDialog/AxisPointsReverse", false).toBool());

    setControls();
}

ChartControlsWidget::~ChartControlsWidget()
{
    QSettings settings;
    settings.setValue("ChartDialog/StartX", txtStartX->value().text());
    settings.setValue("ChartDialog/EndX", txtEndX->value().text());
    settings.setValue("ChartDialog/StartY", txtStartY->value().text());
    settings.setValue("ChartDialog/EndY", txtEndY->value().text());
    settings.setValue("ChartDialog/Angle", txtAngle->value().text());
    settings.setValue("ChartDialog/AxisLength", radAxisLength->isChecked());
    settings.setValue("ChartDialog/AxisX", radAxisX->isChecked());
    settings.setValue("ChartDialog/AxisY", radAxisY->isChecked());
    settings.setValue("ChartDialog/AxisPoints", txtAxisPoints->value());
    settings.setValue("ChartDialog/AxisPointsReverse", chkAxisPointsReverse->isChecked());
}

void ChartControlsWidget::setControls()
{
    actChart->setEnabled(Util::problem()->isSolved());

    if (!Util::problem()->isSolved())
        return;

    fillComboBoxScalarVariable(Util::scene()->activeViewField(), cmbFieldVariable);
    doFieldVariable(cmbFieldVariable->currentIndex());
    fillComboBoxTimeStep(Util::scene()->activeViewField(), cmbTimeStep);

    // correct labels
    lblStartX->setText(Util::problem()->config()->labelX() + ":");
    lblStartY->setText(Util::problem()->config()->labelY() + ":");
    lblEndX->setText(Util::problem()->config()->labelX() + ":");
    lblEndY->setText(Util::problem()->config()->labelY() + ":");
    lblPointX->setText(Util::problem()->config()->labelX() + ":");
    lblPointY->setText(Util::problem()->config()->labelY() + ":");
    radAxisX->setText(Util::problem()->config()->labelX());
    radAxisY->setText(Util::problem()->config()->labelY());

    if (Util::scene()->activeViewField()->analysisType() == AnalysisType_Transient)
    {
        tabAnalysisType->setTabEnabled(tabAnalysisType->indexOf(widTime), true);
    }
    else
    {
        tabAnalysisType->setTabEnabled(tabAnalysisType->indexOf(widTime), false);
        tabAnalysisType->setCurrentWidget(widGeometry);
    }

    doChartLine();
}

void ChartControlsWidget::createControls()
{
    // controls
    QWidget *controls = new QWidget(this);

    QPushButton *btnPlot = new QPushButton(controls);
    btnPlot->setText(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(doPlot()));

    QPushButton *btnSaveImage = new QPushButton(controls);
    btnSaveImage->setText(tr("Save image"));
    connect(btnSaveImage, SIGNAL(clicked()), m_chart, SLOT(saveImage()));

    QPushButton *btnExportData = new QPushButton(controls);
    btnExportData->setText(tr("Export"));
    connect(btnExportData, SIGNAL(clicked()), SLOT(doExportData()));

    // geometry
    lblStartX = new QLabel("X:");
    lblStartY = new QLabel("Y:");
    lblEndX = new QLabel("X:");
    lblEndY = new QLabel("Y:");

    txtStartX = new ValueLineEdit();
    txtStartY = new ValueLineEdit();
    txtEndX = new ValueLineEdit();
    txtEndY = new ValueLineEdit();

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

    // angle
    txtAngle = new ValueLineEdit();
    connect(txtAngle, SIGNAL(editingFinished()), this, SLOT(doChartLine()));

    QHBoxLayout *layoutAngle = new QHBoxLayout();
    layoutAngle->addWidget(new QLabel("Angle"));
    layoutAngle->addWidget(txtAngle);

    QGroupBox *grpAngle = new QGroupBox(tr("Angle"), this);
    grpAngle->setLayout(layoutAngle);

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
    chkAxisPointsReverse = new QCheckBox(tr("Reverse"));
    connect(chkAxisPointsReverse, SIGNAL(clicked()), this, SLOT(doPlot()));

    // timestep
    cmbTimeStep = new QComboBox(this);
    connect(cmbTimeStep, SIGNAL(currentIndexChanged(int)), this, SLOT(doTimeStepChanged(int)));

    QGridLayout *layoutAxisPointsAndTimeStep = new QGridLayout();
    layoutAxisPointsAndTimeStep->addWidget(new QLabel(tr("Points:")), 0, 0);
    layoutAxisPointsAndTimeStep->addWidget(txtAxisPoints, 0, 1);
    layoutAxisPointsAndTimeStep->addWidget(chkAxisPointsReverse, 0, 2);
    layoutAxisPointsAndTimeStep->addWidget(new QLabel(tr("Time step:")), 1, 0);
    layoutAxisPointsAndTimeStep->addWidget(cmbTimeStep, 1, 1, 1, 2);

    QGroupBox *grpAxisPointsAndTimeStep = new QGroupBox(tr("Points and time step"), this);
    grpAxisPointsAndTimeStep->setLayout(layoutAxisPointsAndTimeStep);

    // time
    lblPointX = new QLabel("X:");
    lblPointY = new QLabel("Y:");
    txtPointX = new ValueLineEdit();
    txtPointY = new ValueLineEdit();

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

    QFormLayout *layoutVariable = new QFormLayout(this);
    layoutVariable->addRow(tr("Variable:"), cmbFieldVariable);
    layoutVariable->addRow(tr("Component:"), cmbFieldVariableComp);

    QWidget *grpVariable = new QWidget(this);
    grpVariable->setLayout(layoutVariable);

    // table
    trvTable = new QTableWidget(this);
    trvTable->setVisible(false);

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
    controlsGeometryLayout->addWidget(grpAngle);
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

    controlsLayout->addWidget(grpVariable);
    controlsLayout->addWidget(tabAnalysisType);
    controlsLayout->addStretch();
    controlsLayout->addWidget(widButton);

    // main layout
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(controls);

    setLayout(layout);
}

void ChartControlsWidget::createActions()
{
    actChart = new QAction(icon("chart"), tr("Chart"), this);
    actChart->setCheckable(true);
}

QList<double> ChartControlsWidget::getHorizontalAxisValues(ChartLine *chartLine)
{
    QList<Point> points = chartLine->getPoints();
    QList<double> xval;

    if (radAxisLength->isChecked())
    {
        for (int i = 0; i < points.length(); i++)
        {
            if (i == 0)
                xval.append(0.0);
            else
            {
                if (fabs(chartLine->angle) < EPS_ZERO)
                    xval.append(xval.at(i-1) + sqrt(Hermes::sqr(points.at(i).x - points.at(i-1).x) + Hermes::sqr(points.at(i).y - points.at(i-1).y)));
                else
                {
                    Point center = centerPoint(points.at(i-1), points.at(i), chartLine->angle/(points.length() - 1));
                    double radius = (points.at(i-1) - center).magnitude();
                    double angle = atan2(points.at(i).y - center.y, points.at(i).x - center.x)
                                   - atan2(points.at(i-1).y - center.y, points.at(i-1).x - center.x);
                    xval.append(xval[i-1] + radius * angle);
                }
            }
        }
    }
    else if (radAxisX->isChecked())
    {
        foreach (Point point, points)
            xval.append(point.x);
    }
    else if (radAxisY->isChecked())
    {
        foreach (Point point, points)
            xval.append(point.y);
    }

    return xval;
}

void ChartControlsWidget::plotGeometry()
{
    // variable
    Module::LocalVariable *physicFieldVariable = Util::scene()->activeViewField()->module()->localVariable(cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString());
    if (!physicFieldVariable) return;

    // variable component
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt();
    if (physicFieldVariableComp == PhysicFieldVariableComp_Undefined) return;

    // number of points
    int count = txtAxisPoints->value();

    // chart
    m_chart->setAxisTitle(QwtPlot::yLeft, QString("%1 (%2)").
                        arg(physicFieldVariable->name()).
                        arg(physicFieldVariable->unit()));

    QString text;
    if (radAxisLength->isChecked()) text = tr("Length (m)");
    if (radAxisX->isChecked()) text = Util::problem()->config()->labelX() + " (m)";
    if (radAxisY->isChecked()) text = Util::problem()->config()->labelY() + " (m)";
    m_chart->setAxisTitle(QwtPlot::xBottom, text);

    // table
    QStringList head = headers();

    trvTable->clear();
    trvTable->setRowCount(count);
    trvTable->setColumnCount(head.count());
    trvTable->setHorizontalHeaderLabels(head);

    // values
    ChartLine *chartLine = new ChartLine(Point(txtStartX->value().number(), txtStartY->value().number()),
                                         Point(txtEndX->value().number(), txtEndY->value().number()),
                                         txtAngle->value().number(),
                                         count);
    doChartLine();

    QList<Point> points = chartLine->getPoints();
    QList<double> xval = getHorizontalAxisValues(chartLine);
    QList<double> yval;

    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        foreach (Module::LocalVariable *variable, fieldInfo->module()->localPointVariables())
        {
            if (physicFieldVariable->id() != variable->id()) continue;

            foreach (Point point, points)
            {
                LocalValue *localValue = Util::plugins()[fieldInfo->fieldId()]->localValue(fieldInfo, point);
                QMap<Module::LocalVariable *, PointValue> values = localValue->values();

                if (variable->isScalar())
                    yval.append(values[variable].scalar);
                else
                {
                    if (physicFieldVariableComp == PhysicFieldVariableComp_X)
                        yval.append(values[variable].vector.x);
                    else if (physicFieldVariableComp == PhysicFieldVariableComp_Y)
                        yval.append(values[variable].vector.y);
                    else
                        yval.append(values[variable].vector.magnitude());
                }

                // fill table row
                fillTableRow(localValue, 0.0, points.indexOf(point));
            }
        }
    }

    // reverse x axis
    if (chkAxisPointsReverse->isChecked())
    {
        for (int i = 0; i < points.length() / 2; i++)
        {
            double tmp = yval[i];
            yval[i] = yval[points.length() - i - 1];
            yval[points.length() - i - 1] = tmp;
        }
    }

    m_chart->setData(xval, yval);
}

void ChartControlsWidget::plotTime()
{
    // variable
    Module::LocalVariable *physicFieldVariable = Util::scene()->activeViewField()->module()->localVariable(cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString());
    if (!physicFieldVariable)
        return;

    // variable comp
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt();
    if (physicFieldVariableComp == PhysicFieldVariableComp_Undefined) return;

    // time levels
    QList<double> timeLevels = Util::solutionStore()->timeLevels(Util::scene()->activeViewField());

    // chart
    m_chart->setAxisTitle(QwtPlot::yLeft, QString("%1 (%2)").
                        arg(physicFieldVariable->name()).
                        arg(physicFieldVariable->unit()));

    m_chart->setAxisTitle(QwtPlot::xBottom, tr("time (s)"));

    // table
    QStringList head = headers();

    trvTable->clear();
    trvTable->setRowCount(timeLevels.count());
    trvTable->setColumnCount(head.count());
    trvTable->setHorizontalHeaderLabels(head);

    // values
    int timeStep = Util::scene()->activeTimeStep();

    QList<double> xval;
    QList<double> yval;

    doChartLine();

    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        foreach (Module::LocalVariable *variable, fieldInfo->module()->localPointVariables())
        {
            if (physicFieldVariable->id() != variable->id()) continue;

            for (int i = 0; i < timeLevels.count(); i++)
            {
                // change time level
                Util::scene()->setActiveTimeStep(i);

                xval.append(timeLevels.at(i));

                Point point(txtPointX->value().number(), txtPointY->value().number());
                LocalValue *localValue = Util::plugins()[fieldInfo->fieldId()]->localValue(fieldInfo, point);
                QMap<Module::LocalVariable *, PointValue> values = localValue->values();

                if (variable->isScalar())
                    yval.append(values[variable].scalar);
                else
                {
                    if (physicFieldVariableComp == PhysicFieldVariableComp_X)
                        yval.append(values[variable].vector.x);
                    else if (physicFieldVariableComp == PhysicFieldVariableComp_Y)
                        yval.append(values[variable].vector.y);
                    else
                        yval.append(values[variable].vector.magnitude());
                }

                // fill table row
                fillTableRow(localValue, timeLevels.at(i), i);
            }
        }
    }

    m_chart->setData(xval, yval);

    // restore previous timestep
    Util::scene()->setActiveTimeStep(timeStep);
    m_sceneViewPost2D->postHermes()->refresh();
}

void ChartControlsWidget::fillTableRow(LocalValue *localValue, double time, int row)
{
    int count = trvTable->rowCount();

    // coordinates
    trvTable->setItem(chkAxisPointsReverse->isChecked() ? count - 1 - row : row, 0,
                      new QTableWidgetItem(QString("%1").arg(localValue->point().x, 0, 'e', 3)));
    trvTable->setItem(chkAxisPointsReverse->isChecked() ? count - 1 - row : row, 1,
                      new QTableWidgetItem(QString("%1").arg(localValue->point().y, 0, 'e', 3)));
    // time
    trvTable->setItem(chkAxisPointsReverse->isChecked() ? count - 1 - row : row, 2,
                      new QTableWidgetItem(QString("%1").arg(time, 0, 'e', 3)));

    // values
    int n = 3;

    QMap<Module::LocalVariable *, PointValue> values = localValue->values();
    foreach (Module::LocalVariable *variable, values.keys())
    {
        if (variable->isScalar())
        {
            // scalar variable
            trvTable->setItem(chkAxisPointsReverse->isChecked() ? count - 1 - row : row, n,
                              new QTableWidgetItem(QString("%1").arg(values[variable].scalar, 0, 'e', 3)));
            n++;
        }
        else
        {
            // vector variable
            trvTable->setItem(chkAxisPointsReverse->isChecked() ? count - 1 - row : row, n,
                              new QTableWidgetItem(QString("%1").arg(values[variable].vector.x, 0, 'e', 3)));
            n++;
            trvTable->setItem(chkAxisPointsReverse->isChecked() ? count - 1 - row : row, n,
                              new QTableWidgetItem(QString("%1").arg(values[variable].vector.y, 0, 'e', 3)));
            n++;
            trvTable->setItem(chkAxisPointsReverse->isChecked() ? count - 1 - row : row, n,
                              new QTableWidgetItem(QString("%1").arg(values[variable].vector.magnitude(), 0, 'e', 3)));
            n++;
        }
    }
}

QStringList ChartControlsWidget::headers()
{
    QStringList head;
    head << "x" << "y" << "t";

    foreach (Module::LocalVariable *variable, Util::scene()->activeViewField()->module()->localPointVariables())
    {
        if (variable->isScalar())
        {
            // scalar variable
            head.append(variable->shortname());
        }
        else
        {
            // vector variable
            head.append(variable->shortname() + Util::problem()->config()->labelX().toLower());
            head.append(variable->shortname() + Util::problem()->config()->labelY().toLower());
            head.append(variable->shortname());
        }
    }

    return head;
}

void ChartControlsWidget::doPlot()
{
    if (!Util::problem()->isSolved()) return;

    if (tabAnalysisType->currentWidget() == widGeometry)
    {
        if (!txtStartX->evaluate()) return;
        if (!txtStartY->evaluate()) return;
        if (!txtEndX->evaluate()) return;
        if (!txtEndY->evaluate()) return;
        if (!txtAngle->evaluate()) return;

        plotGeometry();
    }

    if (tabAnalysisType->currentWidget() == widTime)
    {
        if (!txtPointX->evaluate()) return;
        if (!txtPointY->evaluate()) return;

        plotTime();
    }
}

void ChartControlsWidget::doFieldVariable(int index)
{
    if (!Util::scene()->activeViewField())
        return;

    Module::LocalVariable *physicFieldVariable = Util::scene()->activeViewField()->module()->localVariable(cmbFieldVariable->itemData(index).toString());
    if (!physicFieldVariable)
        return;

    cmbFieldVariableComp->clear();
    if (physicFieldVariable->isScalar())
    {
        cmbFieldVariableComp->addItem(tr("Scalar"), PhysicFieldVariableComp_Scalar);
    }
    else
    {
        cmbFieldVariableComp->addItem(tr("Magnitude"), PhysicFieldVariableComp_Magnitude);
        cmbFieldVariableComp->addItem(Util::problem()->config()->labelX(), PhysicFieldVariableComp_X);
        cmbFieldVariableComp->addItem(Util::problem()->config()->labelY(), PhysicFieldVariableComp_Y);
    }

    if (cmbFieldVariableComp->currentIndex() == -1)
        cmbFieldVariableComp->setCurrentIndex(0);

    doPlot();
}

void ChartControlsWidget::doFieldVariableComp(int index)
{
    doPlot();
}

void ChartControlsWidget::doExportData()
{
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
                {
                    if (trvTable->item(i, j))
                        out << trvTable->item(i, j)->text().isEmpty()  << ";";
                    else
                        out << ""  << ";";
                }
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

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastDataDir", fileInfo.absolutePath());

        file.close();
    }
}

void ChartControlsWidget::doChartLine()
{
    if (isVisible())
    {
        if (tabAnalysisType->currentWidget() == widGeometry)
        {
            if (!txtStartX->evaluate()) return;
            if (!txtStartY->evaluate()) return;
            if (!txtEndX->evaluate()) return;
            if (!txtEndY->evaluate()) return;
            if (!txtAngle->evaluate()) return;

            emit setChartLine(ChartLine(Point(txtStartX->value().number(), txtStartY->value().number()),
                                        Point(txtEndX->value().number(), txtEndY->value().number()),
                                        txtAngle->value().number(),
                                        txtAxisPoints->value(),
                                        chkAxisPointsReverse->isChecked()));
        }
        if (tabAnalysisType->currentWidget() == widTime)
        {
            if (!txtPointX->evaluate()) return;
            if (!txtPointY->evaluate()) return;

            emit setChartLine(ChartLine(Point(txtPointX->value().number(), txtPointY->value().number()),
                                        Point(txtPointX->value().number(), txtPointY->value().number()),
                                        0.0,
                                        0));
        }
    }
    else
    {
        emit setChartLine(ChartLine());
    }
}

void ChartControlsWidget::doTimeStepChanged(int index)
{
    if (cmbTimeStep->currentIndex() != -1)
    {
        Util::scene()->setActiveTimeStep(cmbTimeStep->currentIndex());
        doPlot();
    }
}
