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
#include "sceneview_post2d.h"

#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/solutionstore.h"
#include "hermes2d/problem_config.h"

#include "gui/chart.h"
#include "gui/common.h"

#include <QSvgRenderer>

static QString generateSvgGeometryWithLine(ChartLine line)
{
    RectPoint boundingBox = SceneEdgeContainer::boundingBox(Agros2D::scene()->edges->items());

    double w = 150;
    double h = 150;
    double stroke_width = max(boundingBox.width(), boundingBox.height()) / qMax(w, h) / 2.0;

    // svg
    QString str;
    str += QString("<svg width=\"%1px\" height=\"%2px\" viewBox=\"%3 %4 %5 %6\" preserveAspectRatio=\"xMinYMin meet\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n").
            arg(w).
            arg(h).
            arg(boundingBox.start.x).
            arg(0).
            arg(boundingBox.width()).
            arg(boundingBox.height());

    str += QString("<g stroke=\"black\" stroke-width=\"%1\" fill=\"none\">\n").arg(stroke_width);

    foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
    {
        if (edge->angle() > 0.0)
        {
            Point center = edge->center();
            double radius = edge->radius();
            double startAngle = atan2(center.y - edge->nodeStart()->point().y,
                                      center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

            int segments = edge->angle() / 5.0;
            if (segments < 2) segments = 2;
            double theta = edge->angle() / double(segments - 1);

            for (int i = 0; i < segments-1; i++)
            {
                double arc1 = (startAngle + i*theta)/180.0*M_PI;
                double arc2 = (startAngle + (i+1)*theta)/180.0*M_PI;

                double x1 = radius * cos(arc1);
                double y1 = radius * sin(arc1);
                double x2 = radius * cos(arc2);
                double y2 = radius * sin(arc2);

                str += QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" />\n").
                        arg(center.x + x1).
                        arg(boundingBox.end.y - (center.y + y1)).
                        arg(center.x + x2).
                        arg(boundingBox.end.y - (center.y + y2));
            }
        }
        else
        {
            str += QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" />\n").
                    arg(edge->nodeStart()->point().x).
                    arg(boundingBox.end.y - edge->nodeStart()->point().y).
                    arg(edge->nodeEnd()->point().x).
                    arg(boundingBox.end.y - edge->nodeEnd()->point().y);
        }
    }
    str += "</g>\n";

    // line
    str += QString("<g stroke=\"red\" stroke-width=\"%1\" fill=\"none\">%2</g>\n").arg(stroke_width*3).arg(QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" />").
                                                                                                             arg(line.start.x).
                                                                                                             arg(boundingBox.end.y - line.start.y).
                                                                                                             arg(line.end.x).
                                                                                                             arg(boundingBox.end.y - line.end.y));

    str += "</svg>\n";

    return str;
}

SvgWidget::SvgWidget(QWidget* parent) : QSvgWidget(parent)
{
}

void SvgWidget::load(const QString &fileName)
{
    renderer.load(fileName);
    update();
}

void SvgWidget::paintEvent(QPaintEvent* qp)
{
    QSize siz = renderer.defaultSize();

    // maintain te aspect ratio
    // double aspect = renderer.viewBoxF().width() / renderer.viewBoxF().height();

    // qDebug() << aspect;

    QPainter painter(this);
    // painter.scale(1.0, 1 / aspect);
    renderer.render(&painter);
}

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

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent)
{
    actSceneModeChart = new QAction(icon("chart"), tr("Chart"), this);
    actSceneModeChart->setShortcut(tr("Ctrl+6"));
    actSceneModeChart->setStatusTip(tr("Chart"));
    actSceneModeChart->setCheckable(true);

    connect(Agros2D::scene(), SIGNAL(cleared()), this, SLOT(setControls()));
    connect(Agros2D::scene(), SIGNAL(invalidated()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(setControls()));

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
    connect(Agros2D::problem(), SIGNAL(solved()), this, SLOT(setControls()));

    createControls();

    // TODO: move to config (problem part)
    QSettings settings;
    txtStartX->setValue(settings.value("ChartDialog/StartX", "0").toString());
    txtEndX->setValue(settings.value("ChartDialog/EndX", "0").toString());
    txtStartY->setValue(settings.value("ChartDialog/StartY", "0").toString());
    txtEndY->setValue(settings.value("ChartDialog/EndY", "0").toString());
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
        widTime->setEnabled(true);
    }
    else
    {
        widTime->setEnabled(false);
        widGeometry->setEnabled(true);
        tbxAnalysisType->setCurrentWidget(widGeometry);
    }

    createChartLine();
}

void ChartControlsWidget::createControls()
{
    // variable
    cmbFieldVariable = new QComboBox();

    // component
    cmbFieldVariableComp = new QComboBox();

    QFormLayout *layoutVariable = new QFormLayout();
    layoutVariable->addRow(tr("Variable:"), cmbFieldVariable);
    layoutVariable->addRow(tr("Component:"), cmbFieldVariableComp);

    m_variableWidget = new QWidget();
    m_variableWidget->setLayout(layoutVariable);

    // viewer
    viewerSVG = new SvgWidget();
    viewerSVG->setMinimumWidth(150);
    viewerSVG->setMaximumWidth(150);
    viewerSVG->setMinimumHeight(250);
    viewerSVG->setMaximumHeight(250);

    // controls
    btnPlot = new QPushButton();
    btnPlot->setDefault(false);
    btnPlot->setText(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(doPlot()));

    btnSaveImage = new QPushButton();
    btnSaveImage->setDefault(false);
    btnSaveImage->setEnabled(false);
    btnSaveImage->setText(tr("Save image"));
    connect(btnSaveImage, SIGNAL(clicked()), m_chart, SLOT(saveImage()));

    btnExportData = new QPushButton();
    btnExportData->setDefault(false);
    btnExportData->setEnabled(false);
    btnExportData->setText(tr("Export"));
    connect(btnExportData, SIGNAL(clicked()), SLOT(doExportData()));

    // geometry
    lblStartX = new QLabel("X:");
    lblStartY = new QLabel("Y:");
    lblEndX = new QLabel("X:");
    lblEndY = new QLabel("Y:");

    txtStartX = new ValueLineEdit();
    txtStartY = new ValueLineEdit();
    connect(txtStartX, SIGNAL(evaluated(bool)), this, SLOT(doEvaluate(bool)));
    connect(txtStartY, SIGNAL(evaluated(bool)), this, SLOT(doEvaluate(bool)));

    txtEndX = new ValueLineEdit();
    txtEndY = new ValueLineEdit();
    connect(txtEndX, SIGNAL(evaluated(bool)), this, SLOT(doEvaluate(bool)));
    connect(txtEndY, SIGNAL(evaluated(bool)), this, SLOT(doEvaluate(bool)));

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

    // x - axis
    radAxisLength = new QRadioButton(tr("Length"));
    radAxisLength->setChecked(true);
    radAxisX = new QRadioButton("X");
    radAxisY = new QRadioButton("Y");

    QButtonGroup *axisGroup = new QButtonGroup();
    axisGroup->addButton(radAxisLength);
    axisGroup->addButton(radAxisX);
    axisGroup->addButton(radAxisY);

    /*
    connect(radAxisLength, SIGNAL(clicked()), this, SLOT(doPlot()));
    connect(radAxisX, SIGNAL(clicked()), this, SLOT(doPlot()));
    connect(radAxisY, SIGNAL(clicked()), this, SLOT(doPlot()));
    */

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
    //connect(chkAxisPointsReverse, SIGNAL(clicked()), this, SLOT(doPlot()));

    // timestep
    QGridLayout *layoutAxisPointsAndTimeStep = new QGridLayout();
    layoutAxisPointsAndTimeStep->addWidget(new QLabel(tr("Points:")), 0, 0);
    layoutAxisPointsAndTimeStep->addWidget(txtAxisPoints, 0, 1);
    layoutAxisPointsAndTimeStep->addWidget(chkAxisPointsReverse, 1, 0, 1, 2);

    QGroupBox *grpAxisPointsAndTimeStep = new QGroupBox(tr("Points and time step"), this);
    grpAxisPointsAndTimeStep->setLayout(layoutAxisPointsAndTimeStep);

    // time
    lblPointX = new QLabel("X:");
    lblPointY = new QLabel("Y:");

    txtPointX = new ValueLineEdit();
    txtPointY = new ValueLineEdit();
    connect(txtPointX, SIGNAL(evaluated(bool)), this, SLOT(doEvaluate(bool)));
    connect(txtPointY, SIGNAL(evaluated(bool)), this, SLOT(doEvaluate(bool)));

    QGridLayout *layoutTime = new QGridLayout();
    layoutTime->addWidget(lblPointX, 0, 0);
    layoutTime->addWidget(txtPointX, 0, 1);
    layoutTime->addWidget(lblPointY, 1, 0);
    layoutTime->addWidget(txtPointY, 1, 1);

    QGroupBox *grpTime = new QGroupBox(tr("Point"));
    grpTime->setLayout(layoutTime);

    // button bar
    QVBoxLayout *layoutButton = new QVBoxLayout();
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

    QVBoxLayout *controlsAndFigureLayout = new QVBoxLayout();
    controlsAndFigureLayout->addWidget(viewerSVG);
    // controlsAndFigureLayout->addStretch(1);
    controlsAndFigureLayout->addWidget(widButton);

    QHBoxLayout *viewLayout = new QHBoxLayout();
    viewLayout->addWidget(tbxAnalysisType);
    viewLayout->addLayout(controlsAndFigureLayout);

    // controls
    QVBoxLayout *controlsLayout = new QVBoxLayout();
    controlsLayout->setMargin(0);
    controlsLayout->addLayout(viewLayout);
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
                xval.append(xval.at(i-1) + sqrt(Hermes::sqr(points.at(i).x - points.at(i-1).x) + Hermes::sqr(points.at(i).y - points.at(i-1).y)));
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
    Module::LocalVariable physicFieldVariable = Agros2D::scene()->activeViewField()->localVariable(cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString());

    // variable component
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt();
    if (physicFieldVariableComp == PhysicFieldVariableComp_Undefined) return;

    // number of points
    int count = txtAxisPoints->value();

    // chart
    m_chart->setAxisTitle(QwtPlot::yLeft, QString("%1 (%2)").
                          arg(physicFieldVariable.name()).
                          arg(physicFieldVariable.unit()));

    QString text;
    if (radAxisLength->isChecked()) text = tr("Length (m)");
    if (radAxisX->isChecked()) text = Agros2D::problem()->config()->labelX() + " (m)";
    if (radAxisY->isChecked()) text = Agros2D::problem()->config()->labelY() + " (m)";
    m_chart->setAxisTitle(QwtPlot::xBottom, text);

    // table
    QStringList head = headers();

    // values
    ChartLine chartLine(Point(txtStartX->value().number(), txtStartY->value().number()),
                        Point(txtEndX->value().number(), txtEndY->value().number()),
                        count);
    createChartLine();

    QList<Point> points = chartLine.getPoints();
    QList<double> xval = getHorizontalAxisValues(&chartLine);
    QList<double> yval;

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        foreach (Module::LocalVariable variable, fieldInfo->localPointVariables())
        {
            if (physicFieldVariable.id() != variable.id()) continue;

            foreach (Point point, points)
            {
                LocalValue *localValue = fieldInfo->plugin()->localValue(fieldInfo, point);
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
    Module::LocalVariable physicFieldVariable = Agros2D::scene()->activeViewField()->localVariable(cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString());

    // variable comp
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt();
    if (physicFieldVariableComp == PhysicFieldVariableComp_Undefined) return;

    // time levels
    QList<double> timeLevels = Agros2D::solutionStore()->timeLevels(Agros2D::scene()->activeViewField());

    // chart
    m_chart->setAxisTitle(QwtPlot::yLeft, QString("%1 (%2)").
                          arg(physicFieldVariable.name()).
                          arg(physicFieldVariable.unit()));

    m_chart->setAxisTitle(QwtPlot::xBottom, tr("time (s)"));

    // table
    QStringList head = headers();

    // values
    int timeStep = Agros2D::scene()->activeTimeStep();

    QList<double> xval;
    QList<double> yval;

    createChartLine();

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        foreach (Module::LocalVariable variable, fieldInfo->localPointVariables())
        {
            if (physicFieldVariable.id() != variable.id()) continue;

            for (int i = 0; i < timeLevels.count(); i++)
            {
                // change time level
                Agros2D::scene()->setActiveTimeStep(i);

                xval.append(timeLevels.at(i));

                Point point(txtPointX->value().number(), txtPointY->value().number());
                LocalValue *localValue = fieldInfo->plugin()->localValue(fieldInfo, point);
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

    foreach (Module::LocalVariable variable, Agros2D::scene()->activeViewField()->localPointVariables())
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

void ChartControlsWidget::doPlot()
{
    if (!Agros2D::problem()->isSolved()) return;

    if (tbxAnalysisType->currentWidget() == widGeometry)
    {
        if (!txtStartX->evaluate()) return;
        if (!txtStartY->evaluate()) return;
        if (!txtEndX->evaluate()) return;
        if (!txtEndY->evaluate()) return;

        plotGeometry();
    }

    if (tbxAnalysisType->currentWidget() == widTime)
    {
        if (!txtPointX->evaluate()) return;
        if (!txtPointY->evaluate()) return;

        plotTime();
    }

    btnSaveImage->setEnabled(m_chart->curve()->dataSize() > 0);
    btnExportData->setEnabled(m_chart->curve()->dataSize() > 0);
}

void ChartControlsWidget::doFieldVariable(int index)
{
    if (!Agros2D::scene()->activeViewField())
        return;

    Module::LocalVariable physicFieldVariable = Agros2D::scene()->activeViewField()->localVariable(cmbFieldVariable->itemData(index).toString());

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

void ChartControlsWidget::doExportData()
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
        ChartLine *chartLine = new ChartLine(Point(txtStartX->value().number(), txtStartY->value().number()),
                                             Point(txtEndX->value().number(), txtEndY->value().number()),
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

        delete chartLine;
    }
    else if (tbxAnalysisType->currentWidget() == widTime)
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

QMap<QString, double> ChartControlsWidget::getData(Point point, int timeStep)
{
    FieldInfo *fieldInfo = Agros2D::scene()->activeViewField();
    QMap<QString, double> table;

    if (Agros2D::scene()->activeTimeStep() != timeStep)
        Agros2D::scene()->setActiveTimeStep(timeStep);

    foreach (Module::LocalVariable variable, fieldInfo->localPointVariables())
    {
        LocalValue *localValue = fieldInfo->plugin()->localValue(fieldInfo, point);
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
    table.insert("t", Agros2D::solutionStore()->timeLevel(fieldInfo, timeStep));

    return table;
}

void ChartControlsWidget::doEvaluate(bool isError)
{
    btnPlot->setEnabled(!isError);

    if (!isError)
        createChartLine();
}

void ChartControlsWidget::createChartLine()
{
    ChartLine line;

    if (isVisible())
    {
        if (tbxAnalysisType->currentWidget() == widGeometry)
        {
            if (!txtStartX->evaluate()) return;
            if (!txtStartY->evaluate()) return;
            if (!txtEndX->evaluate()) return;
            if (!txtEndY->evaluate()) return;

            line = ChartLine(Point(txtStartX->value().number(), txtStartY->value().number()),
                             Point(txtEndX->value().number(), txtEndY->value().number()),
                             txtAxisPoints->value(),
                             chkAxisPointsReverse->isChecked());
        }
        if (tbxAnalysisType->currentWidget() == widTime)
        {
            if (!txtPointX->evaluate()) return;
            if (!txtPointY->evaluate()) return;

            line = ChartLine(Point(txtPointX->value().number(), txtPointY->value().number()),
                             Point(txtPointX->value().number(), txtPointY->value().number()),
                             0.0,
                             0);
        }
    }

    QString figure = generateSvgGeometryWithLine(line);
    QString fileName = QString("%1/figure.svg").arg(tempProblemDir());
    writeStringContent(fileName, figure);

    viewerSVG->load(fileName);
    // double ratio = ((double) m_viewerSVG->renderer()->viewBox().width() / (double) m_viewerSVG->renderer()->viewBox().height());
    // double h = m_viewerSVG->width() / ratio;
    // m_viewerSVG->resize(m_viewerSVG->width(), h);
}
