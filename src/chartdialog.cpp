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
#include "sceneview_post2d.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/solutionstore.h"
#include "hermes2d/problem_config.h"

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
    actSceneModeChart = new QAction(icon("chart"), tr("Chart"), this);
    actSceneModeChart->setShortcut(Qt::Key_F8);
    actSceneModeChart->setStatusTip(tr("Chart"));
    actSceneModeChart->setCheckable(true);

    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(setControls()));
    // connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(setControls()));

    m_chart = new Chart(this);

    QHBoxLayout *layoutMain = new QHBoxLayout();
    layoutMain->addWidget(m_chart);

    setLayout(layoutMain);

    setControls();
}

void ChartWidget::setControls()
{
    actSceneModeChart->setEnabled(Agros2D::problem()->isSolved());
}

// **************************************************************************************************

ChartControlsWidget::ChartControlsWidget(SceneViewPost2D *sceneView,
                         ChartBasic *chart,
                         QWidget *parent) : QWidget(parent), m_sceneViewPost2D(sceneView), m_chart(chart)
{
    connect(this, SIGNAL(setChartLine(ChartLine)), m_sceneViewPost2D, SLOT(setChartLine(ChartLine)));
    // connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(setControls()));

    createControls();

    // TODO: move to config (problem part)
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
    // TODO: move to config (problem part)
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
    if (!Agros2D::problem()->isSolved())
        return;

    fillComboBoxScalarVariable(Agros2D::scene()->activeViewField(), cmbFieldVariable);
    doFieldVariable(cmbFieldVariable->currentIndex());

    // correct labels
    lblStartX->setText(Agros2D::problem()->config()->labelX() + ":");
    lblStartY->setText(Agros2D::problem()->config()->labelY() + ":");
    lblEndX->setText(Agros2D::problem()->config()->labelX() + ":");
    lblEndY->setText(Agros2D::problem()->config()->labelY() + ":");
    lblPointX->setText(Agros2D::problem()->config()->labelX() + ":");
    lblPointY->setText(Agros2D::problem()->config()->labelY() + ":");
    radAxisX->setText(Agros2D::problem()->config()->labelX());
    radAxisY->setText(Agros2D::problem()->config()->labelY());

    if (Agros2D::scene()->activeViewField()->analysisType() == AnalysisType_Transient)
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
    // variable
    cmbFieldVariable = new QComboBox();
    connect(cmbFieldVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(doFieldVariable(int)));
    // component
    cmbFieldVariableComp = new QComboBox();
    connect(cmbFieldVariableComp, SIGNAL(currentIndexChanged(int)), this, SLOT(doFieldVariableComp(int)));

    QFormLayout *layoutVariable = new QFormLayout();
    layoutVariable->addRow(tr("Variable:"), cmbFieldVariable);
    layoutVariable->addRow(tr("Component:"), cmbFieldVariableComp);

    m_variableWidget = new QWidget();
    m_variableWidget->setLayout(layoutVariable);

    // controls
    QPushButton *btnPlot = new QPushButton();
    btnPlot->setText(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(doPlot()));

    QPushButton *btnSaveImage = new QPushButton();
    btnSaveImage->setText(tr("Save image"));
    connect(btnSaveImage, SIGNAL(clicked()), m_chart, SLOT(saveImage()));

    QPushButton *btnExportData = new QPushButton();
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

    QGroupBox *grpStart = new QGroupBox(tr("Start"));
    grpStart->setLayout(layoutStart);

    // end
    QGridLayout *layoutEnd = new QGridLayout();
    layoutEnd->addWidget(lblEndX, 0, 0);
    layoutEnd->addWidget(txtEndX, 0, 1);
    layoutEnd->addWidget(lblEndY, 1, 0);
    layoutEnd->addWidget(txtEndY, 1, 1);

    QGroupBox *grpEnd = new QGroupBox(tr("End"));
    grpEnd->setLayout(layoutEnd);

    // angle
    txtAngle = new ValueLineEdit();
    connect(txtAngle, SIGNAL(editingFinished()), this, SLOT(doChartLine()));

    QHBoxLayout *layoutAngle = new QHBoxLayout();
    layoutAngle->addWidget(new QLabel("Angle"));
    layoutAngle->addWidget(txtAngle);

    QGroupBox *grpAngle = new QGroupBox(tr("Angle"));
    grpAngle->setLayout(layoutAngle);

    // x - axis
    radAxisLength = new QRadioButton(tr("Length"));
    radAxisLength->setChecked(true);
    radAxisX = new QRadioButton("X");
    radAxisY = new QRadioButton("Y");

    QButtonGroup *axisGroup = new QButtonGroup();
    axisGroup->addButton(radAxisLength);
    axisGroup->addButton(radAxisX);
    axisGroup->addButton(radAxisY);
    connect(radAxisLength, SIGNAL(clicked()), this, SLOT(doPlot()));
    connect(radAxisX, SIGNAL(clicked()), this, SLOT(doPlot()));
    connect(radAxisY, SIGNAL(clicked()), this, SLOT(doPlot()));

    // axis
    QHBoxLayout *layoutAxis = new QHBoxLayout();
    layoutAxis->addWidget(radAxisLength);
    layoutAxis->addWidget(radAxisX);
    layoutAxis->addWidget(radAxisY);

    QGroupBox *grpAxis = new QGroupBox(tr("Horizontal axis"));
    grpAxis->setLayout(layoutAxis);

    // axis points and time step
    txtAxisPoints = new QSpinBox(this);
    txtAxisPoints->setMinimum(2);
    txtAxisPoints->setMaximum(500);
    txtAxisPoints->setValue(200);
    chkAxisPointsReverse = new QCheckBox(tr("Reverse"));
    connect(chkAxisPointsReverse, SIGNAL(clicked()), this, SLOT(doPlot()));

    // timestep
    QGridLayout *layoutAxisPointsAndTimeStep = new QGridLayout();
    layoutAxisPointsAndTimeStep->addWidget(new QLabel(tr("Points:")), 0, 0);
    layoutAxisPointsAndTimeStep->addWidget(txtAxisPoints, 0, 1);
    layoutAxisPointsAndTimeStep->addWidget(chkAxisPointsReverse, 0, 2);

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

    QGroupBox *grpTime = new QGroupBox(tr("Point"));
    grpTime->setLayout(layoutTime);

    // button bar
    QHBoxLayout *layoutButton = new QHBoxLayout();
    layoutButton->addStretch();
    layoutButton->addWidget(btnPlot);
    layoutButton->addWidget(btnSaveImage);
    layoutButton->addWidget(btnExportData);

    QWidget *widButton = new QWidget();
    widButton->setLayout(layoutButton);

    // controls geometry
    widGeometry = new QWidget();
    QVBoxLayout *controlsGeometryLayout = new QVBoxLayout();
    widGeometry->setLayout(controlsGeometryLayout);
    controlsGeometryLayout->addWidget(grpStart);
    controlsGeometryLayout->addWidget(grpEnd);
    controlsGeometryLayout->addWidget(grpAngle);
    controlsGeometryLayout->addWidget(grpAxis);
    controlsGeometryLayout->addWidget(grpAxisPointsAndTimeStep);
    controlsGeometryLayout->addStretch();

    // controls time
    widTime = new QWidget();
    QVBoxLayout *controlsTimeLayout = new QVBoxLayout();
    widTime->setLayout(controlsTimeLayout);
    controlsTimeLayout->addWidget(grpTime);
    controlsTimeLayout->addStretch();

    tabAnalysisType = new QTabWidget();
    tabAnalysisType->addTab(widGeometry, icon(""), tr("Geometry"));
    tabAnalysisType->addTab(widTime, icon(""), tr("Time"));

    // controls
    QVBoxLayout *controlsLayout = new QVBoxLayout();
    controlsLayout->setMargin(0);
    controlsLayout->addWidget(tabAnalysisType);    
    controlsLayout->addWidget(widButton);
    controlsLayout->addStretch(1);

    setLayout(controlsLayout);
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
    Module::LocalVariable *physicFieldVariable = Agros2D::scene()->activeViewField()->module()->localVariable(cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString());
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
    if (radAxisX->isChecked()) text = Agros2D::problem()->config()->labelX() + " (m)";
    if (radAxisY->isChecked()) text = Agros2D::problem()->config()->labelY() + " (m)";
    m_chart->setAxisTitle(QwtPlot::xBottom, text);

    // table
    QStringList head = headers();

    // values
    ChartLine *chartLine = new ChartLine(Point(txtStartX->value().number(), txtStartY->value().number()),
                                         Point(txtEndX->value().number(), txtEndY->value().number()),
                                         txtAngle->value().number(),
                                         count);
    doChartLine();

    QList<Point> points = chartLine->getPoints();
    QList<double> xval = getHorizontalAxisValues(chartLine);
    QList<double> yval;

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        foreach (Module::LocalVariable *variable, fieldInfo->module()->localPointVariables())
        {
            if (physicFieldVariable->id() != variable->id()) continue;

            foreach (Point point, points)
            {
                LocalValue *localValue = Agros2D::plugin(fieldInfo->fieldId())->localValue(fieldInfo, point);
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
    Module::LocalVariable *physicFieldVariable = Agros2D::scene()->activeViewField()->module()->localVariable(cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString());
    if (!physicFieldVariable)
        return;

    // variable comp
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt();
    if (physicFieldVariableComp == PhysicFieldVariableComp_Undefined) return;

    // time levels
    QList<double> timeLevels = Agros2D::solutionStore()->timeLevels(Agros2D::scene()->activeViewField());

    // chart
    m_chart->setAxisTitle(QwtPlot::yLeft, QString("%1 (%2)").
                        arg(physicFieldVariable->name()).
                        arg(physicFieldVariable->unit()));

    m_chart->setAxisTitle(QwtPlot::xBottom, tr("time (s)"));

    // table
    QStringList head = headers();

    // values
    int timeStep = Agros2D::scene()->activeTimeStep();

    QList<double> xval;
    QList<double> yval;

    doChartLine();

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        foreach (Module::LocalVariable *variable, fieldInfo->module()->localPointVariables())
        {
            if (physicFieldVariable->id() != variable->id()) continue;

            for (int i = 0; i < timeLevels.count(); i++)
            {
                // change time level
                Agros2D::scene()->setActiveTimeStep(i);

                xval.append(timeLevels.at(i));

                Point point(txtPointX->value().number(), txtPointY->value().number());
                LocalValue *localValue = Agros2D::plugin(fieldInfo->fieldId())->localValue(fieldInfo, point);
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
            }
        }
    }

    m_chart->setData(xval, yval);

    // restore previous timestep
    Agros2D::scene()->setActiveTimeStep(timeStep);
    m_sceneViewPost2D->postHermes()->refresh();
}

QStringList ChartControlsWidget::headers()
{
    QStringList head;
    head << "x" << "y" << "t";

    foreach (Module::LocalVariable *variable, Agros2D::scene()->activeViewField()->module()->localPointVariables())
    {
        if (variable->isScalar())
        {
            // scalar variable
            head.append(variable->shortname());
        }
        else
        {
            // vector variable
            head.append(variable->shortname() + Agros2D::problem()->config()->labelX().toLower());
            head.append(variable->shortname() + Agros2D::problem()->config()->labelY().toLower());
            head.append(variable->shortname());
        }
    }

    return head;
}

void ChartControlsWidget::doPlot()
{
    if (!Agros2D::problem()->isSolved()) return;

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
    if (!Agros2D::scene()->activeViewField())
        return;

    Module::LocalVariable *physicFieldVariable = Agros2D::scene()->activeViewField()->module()->localVariable(cmbFieldVariable->itemData(index).toString());
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
        cmbFieldVariableComp->addItem(Agros2D::problem()->config()->labelX(), PhysicFieldVariableComp_X);
        cmbFieldVariableComp->addItem(Agros2D::problem()->config()->labelY(), PhysicFieldVariableComp_Y);
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
    if (fileName.isEmpty())
    {
        cerr << "Incorrect file name." << endl;
        return;
    }

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

    QMap<QString, QList<double> > table;
    if (tabAnalysisType->currentWidget() == widGeometry)
    {
        ChartLine *chartLine = new ChartLine(Point(txtStartX->value().number(), txtStartY->value().number()),
                                             Point(txtEndX->value().number(), txtEndY->value().number()),
                                             txtAngle->value().number(),
                                             txtAxisPoints->value());

        foreach (Point point, chartLine->getPoints())
        {
            QMap<QString, double> data = getData(point, Agros2D::scene()->activeTimeStep());
            foreach (QString key, data.keys())
            {
                QList<double> *values = &table.operator [](key);
                values->append(data.value(key));
            }
        }
    }
    else if (tabAnalysisType->currentWidget() == widTime)
    {
        Point point(txtPointX->value().number(), txtPointY->value().number());
        foreach (double timeLevel, Agros2D::solutionStore()->timeLevels(Agros2D::scene()->activeViewField()))
        {
            int timeStep = Agros2D::solutionStore()->timeLevelIndex(Agros2D::scene()->activeViewField(), timeLevel);
            QMap<QString, double> data = getData(point, timeStep);
            foreach (QString key, data.keys())
            {
                QList<double> *values = &table.operator [](key);
                values->append(data.value(key));
            }
        }
    }

    // csv
    if (fileInfo.suffix().toLower() == "csv")
    {
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
    }

    // m-file
    if (fileInfo.suffix().toLower() == "m")
    {
        // matrix
        foreach(QString key, table.keys())
        {
            out << key << " = [ ";
            foreach(double value, table.value(key))
                out << QString::number(value).replace(",", ".") << " ";
            out << "]" << endl;
        }

        /*
        // example
        QString x = table.keys().at(0).at(0);
        QString y = table.keys().at(1).at(0);

        out << endl << "% example" << endl;
        out << QString("% plot(%1, %2)").arg(x).arg(y) << endl;
        out << "% grid on;" << endl;
        out << QString("% xlabel('%1');").arg(x) << endl;
        out << QString("% ylabel('%1');").arg(y) << endl;
        */
    }

    if (fileInfo.absoluteDir() != tempProblemDir())
        settings.setValue("General/LastDataDir", fileInfo.absolutePath());

    file.close();
}

QMap<QString, double> ChartControlsWidget::getData(Point point, int timeStep)
{
    FieldInfo *fieldInfo = Agros2D::scene()->activeViewField();
    QMap<QString, double> table;

    if (Agros2D::scene()->activeTimeStep() != timeStep)
        Agros2D::scene()->setActiveTimeStep(timeStep);

    foreach (Module::LocalVariable *variable, fieldInfo->module()->localPointVariables())
    {
        LocalValue *localValue = Agros2D::plugin(fieldInfo->fieldId())->localValue(fieldInfo, point);
        QMap<Module::LocalVariable *, PointValue> values = localValue->values();

        if (variable->isScalar())
            table.insert(variable->shortname(), values[variable].scalar);
        else
        {
            table.insert(QString(variable->shortname()), values[variable].vector.magnitude());
            table.insert(QString(variable->shortname() + "x"), values[variable].vector.x);
            table.insert(QString(variable->shortname() + "y"), values[variable].vector.y);
        }
    }

    table.insert(Agros2D::problem()->config()->labelX(), point.x);
    table.insert(Agros2D::problem()->config()->labelY(), point.y);
    table.insert("t", Agros2D::solutionStore()->timeLevel(fieldInfo, timeStep));

    return table;
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
