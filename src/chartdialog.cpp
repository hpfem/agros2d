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
#include "scenesolution.h"
#include "hermes2d/localpoint.h"
#include "hermes2d/module.h"


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
    txtAngle->setValue(settings.value("ChartDialog/Angle", "0").toString());
    radAxisLength->setChecked(settings.value("ChartDialog/AxisLength", true).toBool());
    radAxisX->setChecked(settings.value("ChartDialog/AxisX", false).toBool());
    radAxisY->setChecked(settings.value("ChartDialog/AxisY", false).toBool());
    txtAxisPoints->setValue(settings.value("ChartDialog/AxisPoints", 200).toInt());
    chkAxisPointsReverse->setChecked(settings.value("ChartDialog/AxisPointsReverse", false).toBool());
}

ChartDialog::~ChartDialog()
{
    logMessage("ChartDialog::~ChartDialog()");

    QSettings settings;
    settings.setValue("ChartDialog/Geometry", saveGeometry());
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
    if (!txtAngle->evaluate()) return;

    doChartLine();

    // variable
    Hermes::Module::LocalVariable *physicFieldVariable = Util::scene()->problemInfo()->module()->get_variable(cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString().toStdString());
    if (!physicFieldVariable)
        return;

    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt();
    if (physicFieldVariableComp == PhysicFieldVariableComp_Undefined) return;

    int count = txtAxisPoints->value();
    double *xval = new double[count];
    double *yval = new double[count];

    // chart->setTitle(physicFieldVariableString(physicFieldVariable) + " - " + physicFieldVariableCompString(physicFieldVariableComp));
    QwtText text("");
    text.setFont(QFont("Helvetica", 10, QFont::Normal));
    text.setText(QString("%1 (%2)").
                 arg(QString::fromStdString(physicFieldVariable->name)).
                 arg(QString::fromStdString(physicFieldVariable->unit)));
    chart->setAxisTitle(QwtPlot::yLeft, text);

    // headers
    QStringList head = headers();

    // table
    trvTable->clear();
    trvTable->setRowCount(count);    
    trvTable->setColumnCount(head.count());
    trvTable->setHorizontalHeaderLabels(head);

    // chart
    if (radAxisLength->isChecked()) text.setText(tr("Length (m)"));
    if (radAxisX->isChecked()) text.setText(Util::scene()->problemInfo()->labelX() + " (m):");
    if (radAxisY->isChecked()) text.setText(Util::scene()->problemInfo()->labelY() + " (m):");
    chart->setAxisTitle(QwtPlot::xBottom, text);

    // line
    ChartLine chartLine(Point(txtStartX->value().number(), txtStartY->value().number()),
                        Point(txtEndX->value().number(), txtEndY->value().number()),
                        txtAngle->value().number(),
                        count);

    QList<Point> points = chartLine.getPoints();

    // calculate values
    for (int i = 0; i < points.length(); i++)
    {
        LocalPointValue localPointValue(points.at(i));

        // x value
        if (radAxisLength->isChecked())
        {
            if (i == 0)
                xval[i] = 0.0;
            else
                if (fabs(chartLine.angle) < EPS_ZERO)
                {
                    xval[i] = xval[i-1] + sqrt(Hermes::sqr(points.at(i).x - points.at(i-1).x) + Hermes::sqr(points.at(i).y - points.at(i-1).y));
                }
                else
                {
                    Point center = centerPoint(points.at(i-1), points.at(i), chartLine.angle/(points.length() - 1));
                    double radius = (points.at(i-1) - center).magnitude();
                    double angle = atan2(points.at(i).y - center.y, points.at(i).x - center.x) -
                            atan2(points.at(i-1).y - center.y, points.at(i-1).x - center.x);

                    xval[i] = xval[i-1] + radius * angle;
                }
        }
        if (radAxisX->isChecked()) xval[i] = points.at(i).x;
        if (radAxisY->isChecked()) xval[i] = points.at(i).y;

        addValue(&localPointValue, yval, i, points.length(),
                 physicFieldVariableComp, physicFieldVariable);
    }

    // reverse vertical axis
    if (chkAxisPointsReverse->isChecked())
    {
        for (int i = 0; i < points.length() / 2; i++)
        {
            double tmp = yval[i];
            yval[i] = yval[points.length() - i - 1];
            yval[points.length() - i - 1] = tmp;
        }
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
    Hermes::Module::LocalVariable *physicFieldVariable = Util::scene()->problemInfo()->module()->get_variable(cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString().toStdString());
    if (!physicFieldVariable)
        return;

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
                 arg(QString::fromStdString(physicFieldVariable->name)).
                 arg(QString::fromStdString(physicFieldVariable->unit)));
    chart->setAxisTitle(QwtPlot::yLeft, text);

    // headers
    QStringList head = headers();

    // table
    trvTable->clear();
    trvTable->setRowCount(count);    
    trvTable->setColumnCount(head.count());
    trvTable->setHorizontalHeaderLabels(head);

    // chart
    text.setText(tr("Time (s)"));
    chart->setAxisTitle(QwtPlot::xBottom, text);

    // calculate values
    QStringList row;
    for (int i = 0; i<Util::scene()->sceneSolution()->timeStepCount(); i++)
    {
        // change time level
        Util::scene()->sceneSolution()->setTimeStep(i, false);

        Point point(txtPointX->value().number(), txtPointY->value().number());
        LocalPointValue localPointValue(point);

        addValue(&localPointValue, yval, i, Util::scene()->sceneSolution()->timeStepCount(),
                 physicFieldVariableComp, physicFieldVariable);

        for (int j = 0; j<row.count(); j++)
            trvTable->setItem(i, j, new QTableWidgetItem(row.at(j)));
    }

    chart->setData(xval, yval, count);

    delete[] xval;
    delete[] yval;

    // restore previous timestep
    Util::scene()->sceneSolution()->setTimeStep(timeStep);
}

QStringList ChartDialog::headers()
{
    QStringList head;

    head << "x" << "y" << "t";

    for (Hermes::vector<Hermes::Module::LocalVariable *>::iterator it = Util::scene()->problemInfo()->module()->local_point.begin();
         it != Util::scene()->problemInfo()->module()->local_point.end(); ++it)
        if (((Hermes::Module::LocalVariable *) *it)->is_scalar)
        {
            // scalar variable
            head.append(QString::fromStdString(((Hermes::Module::LocalVariable *) *it)->shortname));
        }
        else
        {
            // vector variable
            head.append(QString::fromStdString(((Hermes::Module::LocalVariable *) *it)->shortname) + Util::scene()->problemInfo()->labelX().toLower());
            head.append(QString::fromStdString(((Hermes::Module::LocalVariable *) *it)->shortname) + Util::scene()->problemInfo()->labelY().toLower());
            head.append(QString::fromStdString(((Hermes::Module::LocalVariable *) *it)->shortname));
        }

    return head;
}

void ChartDialog::addValue(LocalPointValue *localPointValue, double *yval, int i, int N,
                           PhysicFieldVariableComp physicFieldVariableComp,
                           Hermes::Module::LocalVariable *physicFieldVariable)
{
    // coordinates
    trvTable->setItem(chkAxisPointsReverse->isChecked() ? N - 1 - i : i, 0,
                      new QTableWidgetItem(QString("%1").arg(localPointValue->point.x, 0, 'e', 3)));
    trvTable->setItem(chkAxisPointsReverse->isChecked() ? N - 1 - i : i, 1,
                      new QTableWidgetItem(QString("%1").arg(localPointValue->point.y, 0, 'e', 3)));
    // time
    trvTable->setItem(chkAxisPointsReverse->isChecked() ? N - 1 - i : i, 2,
                      new QTableWidgetItem(QString("%1").arg(Util::scene()->sceneSolution()->time(), 0, 'e', 3)));

    // counter
    int n = 3;

    // local variables
    // FIXME - wrong order!!!
    for (std::map<Hermes::Module::LocalVariable *, PointValue>::iterator it = localPointValue->values.begin(); it != localPointValue->values.end(); ++it)
    {
        // chart
        if (it->first->id == physicFieldVariable->id)
        {
            if (physicFieldVariable->is_scalar)
            {
                // scalar variable
                yval[i] = it->second.scalar;
            }
            else
            {
                // vector variable
                if (physicFieldVariableComp == PhysicFieldVariableComp_X)
                    yval[i] = it->second.vector.x;
                else if (physicFieldVariableComp == PhysicFieldVariableComp_Y)
                    yval[i] = it->second.vector.y;
                else
                    yval[i] = it->second.vector.magnitude();
            }
        }

        // table
        if (it->first->is_scalar)
        {
            // scalar variable
            trvTable->setItem(chkAxisPointsReverse->isChecked() ? N - 1 - i : i, n,
                              new QTableWidgetItem(QString("%1").arg(it->second.scalar, 0, 'e', 3)));
            n++;
        }
        else
        {
            // vector variable
            trvTable->setItem(chkAxisPointsReverse->isChecked() ? N - 1 - i : i, n,
                              new QTableWidgetItem(QString("%1").arg(it->second.vector.x, 0, 'e', 3)));
            n++;
            trvTable->setItem(chkAxisPointsReverse->isChecked() ? N - 1 - i : i, n,
                              new QTableWidgetItem(QString("%1").arg(it->second.vector.y, 0, 'e', 3)));
            n++;
            trvTable->setItem(chkAxisPointsReverse->isChecked() ? N - 1 - i : i, n,
                              new QTableWidgetItem(QString("%1").arg(it->second.vector.magnitude(), 0, 'e', 3)));
            n++;
        }
    }
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

    Hermes::Module::LocalVariable *physicFieldVariable = Util::scene()->problemInfo()->module()->get_variable(cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString().toStdString());
    if (!physicFieldVariable)
        return;

    cmbFieldVariableComp->clear();
    if (physicFieldVariable->is_scalar)
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

        if (fileInfo.absoluteDir() != tempProblemDir())
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

void ChartDialog::doTimeStepChanged(int index)
{
    logMessage("ChartDialog::doTimeStepChanged()");

    if (cmbTimeStep->currentIndex() != -1)
    {
        Util::scene()->sceneSolution()->setTimeStep(cmbTimeStep->currentIndex());
        doPlot();
    }
}
