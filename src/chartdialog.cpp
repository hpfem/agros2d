#include "chartdialog.h"

ChartDialog::ChartDialog(QWidget *parent) : QDialog(parent)
{
    QSettings settings;
    restoreGeometry(settings.value("ChartDialog/Geometry", saveGeometry()).toByteArray());
    
    setWindowIcon(icon("chart"));
    setWindowFlags(Qt::Window);
    setWindowTitle(tr("Chart"));

    createControls();

    resize(900, 600);

    setMinimumSize(sizeHint());
}

ChartDialog::~ChartDialog()
{
    QSettings settings;
    settings.setValue("ChartDialog/Geometry", saveGeometry());

    // geometry
    delete lblStartX;
    delete lblStartY;
    delete lblEndX;
    delete lblEndY;

    delete txtStartX;
    delete txtStartY;
    delete txtEndX;
    delete txtEndY;
    
    delete radAxisLength;
    delete radAxisX;
    delete radAxisY;

    delete cmbTimeStep;

    // time
    delete lblPointX;
    delete lblPointY;
    delete txtPointX;
    delete txtPointY;

    delete txtAxisPoints;

    delete cmbFieldVariable;
    delete cmbFieldVariableComp;

    delete picker;
    delete chart;    

    delete widGeometry;
    delete widTime;

    delete tabAnalysisType;
    delete tabOutput;
}

void ChartDialog::showDialog()
{
    fillComboBoxScalarVariable(cmbFieldVariable);
    fillComboBoxTimeStep(cmbTimeStep);

    // correct labels
    lblStartX->setText(Util::scene()->problemInfo().labelX() + ":");
    lblStartY->setText(Util::scene()->problemInfo().labelY() + ":");
    lblEndX->setText(Util::scene()->problemInfo().labelX() + ":");
    lblEndY->setText(Util::scene()->problemInfo().labelY() + ":");
    lblPointX->setText(Util::scene()->problemInfo().labelX() + ":");
    lblPointY->setText(Util::scene()->problemInfo().labelY() + ":");
    radAxisX->setText(Util::scene()->problemInfo().labelX());
    radAxisY->setText(Util::scene()->problemInfo().labelY());

    if (Util::scene()->problemInfo().analysisType == ANALYSISTYPE_TRANSIENT)
    {
        tabAnalysisType->setTabEnabled(tabAnalysisType->indexOf(widTime), true);
    }
    else
    {
        tabAnalysisType->setTabEnabled(tabAnalysisType->indexOf(widTime), false);
        tabAnalysisType->setCurrentWidget(widGeometry);
    }

    show();
    doChartLine();
}

void ChartDialog::hideEvent(QHideEvent *event)
{
    doChartLine();
}

void ChartDialog::createControls()
{
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
    // timestep
    cmbTimeStep = new QComboBox(this);
    connect(cmbTimeStep, SIGNAL(currentIndexChanged(int)), this, SLOT(doTimeStepChanged(int)));

    QVBoxLayout *layoutTimeStep = new QVBoxLayout();
    layoutTimeStep->addWidget(cmbTimeStep);

    QGroupBox *grpTimeStep = new QGroupBox(tr("Time step"), this);
    grpTimeStep->setLayout(layoutTimeStep);

    // geometry
    lblStartX = new QLabel("X:");
    lblStartY = new QLabel("Y:");
    lblEndX = new QLabel("X:");
    lblEndY = new QLabel("Y:");

    txtStartX = new SLineEdit("0", false);
    txtStartY = new SLineEdit("0", false);
    txtEndX = new SLineEdit("0.0035", false);
    txtEndY = new SLineEdit("0", false);

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
    
    // axis
    QVBoxLayout *layoutAxis = new QVBoxLayout(this);
    layoutAxis->addWidget(radAxisLength);
    layoutAxis->addWidget(radAxisX);
    layoutAxis->addWidget(radAxisY);
    
    QGroupBox *grpAxis = new QGroupBox(tr("Horizontal axis"), this);
    grpAxis->setLayout(layoutAxis);
    
    // axis points
    txtAxisPoints = new QSpinBox(this);
    txtAxisPoints->setMinimum(2);
    txtAxisPoints->setMaximum(500);
    txtAxisPoints->setValue(200);

    QVBoxLayout *layoutAxisPoints = new QVBoxLayout();
    layoutAxisPoints->addWidget(txtAxisPoints);

    QGroupBox *grpAxisPoints = new QGroupBox(tr("Points:"), this);
    grpAxisPoints->setLayout(layoutAxisPoints);

    // time
    lblPointX = new QLabel("X:");
    lblPointY = new QLabel("Y:");
    txtPointX = new SLineEdit("0", false);
    txtPointY = new SLineEdit("0", false);

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
    doFieldVariable(cmbFieldVariable->currentIndex());
    
    QVBoxLayout *layoutVariable = new QVBoxLayout(this);
    layoutVariable->addWidget(cmbFieldVariable);
    layoutVariable->addWidget(cmbFieldVariableComp);
    
    QGroupBox *grpVariable = new QGroupBox(tr("Variable"), this);
    grpVariable->setLayout(layoutVariable);
    
    // table
    trvTable = new QTableWidget(this);
    
    // button bar
    QVBoxLayout *layoutButton = new QVBoxLayout();
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
    controlsGeometryLayout->addWidget(grpAxisPoints);
    controlsGeometryLayout->addWidget(grpTimeStep);
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
    controls->setMinimumWidth(200);
    controls->setMaximumWidth(200);

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
    layout->addWidget(tabOutput);
    
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
    doChartLine();

    int count = txtAxisPoints->value();
    double *xval = new double[count];
    double *yval = new double[count];
    
    // variable
    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toInt();
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt();
    
    // chart->setTitle(physicFieldVariableString(physicFieldVariable) + " - " + physicFieldVariableCompString(physicFieldVariableComp));
    QwtText text("");
    text.setFont(QFont("Helvetica", 10, QFont::Normal));
    text.setText(physicFieldVariableString(physicFieldVariable) + " (" + physicFieldVariableUnits(physicFieldVariable) + ")");
    chart->setAxisTitle(QwtPlot::yLeft, text);
    
    // table
    trvTable->clear();
    trvTable->setRowCount(count);
    QStringList headers = Util::scene()->problemInfo().hermes->localPointValueHeader();
    trvTable->setColumnCount(headers.count());
    trvTable->setHorizontalHeaderLabels(headers);
    
    // chart
    if (radAxisLength->isChecked()) text.setText(tr("Length (m)"));
    if (radAxisX->isChecked()) text.setText(Util::scene()->problemInfo().labelX() + " (m):");
    if (radAxisY->isChecked()) text.setText(Util::scene()->problemInfo().labelY() + " (m):");
    chart->setAxisTitle(QwtPlot::xBottom, text);

    // line
    Point start(txtStartX->value(), txtStartY->value());
    Point end(txtEndX->value(), txtEndY->value());
    
    Point diff((end.x - start.x)/(count-1), (end.y - start.y)/(count-1));
    
    // calculate values
    QStringList row;
    for (int i = 0; i<count; i++)
    {
        Point point(start.x + i*diff.x, start.y + i*diff.y);
        LocalPointValue *localPointValue = Util::scene()->problemInfo().hermes->localPointValue(point);
        
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
    doChartLine();

    // store timestep
    int timeStep = Util::scene()->sceneSolution()->timeStep();

    int count = Util::scene()->sceneSolution()->timeStepCount();
    double *xval = new double[count];
    double *yval = new double[count];

    // variable
    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toInt();
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt();

    // chart->setTitle(physicFieldVariableString(physicFieldVariable) + " - " + physicFieldVariableCompString(physicFieldVariableComp));
    QwtText text("");
    text.setFont(QFont("Helvetica", 10, QFont::Normal));
    text.setText(physicFieldVariableString(physicFieldVariable) + " (" + physicFieldVariableUnits(physicFieldVariable) + ")");
    chart->setAxisTitle(QwtPlot::yLeft, text);

    // table
    trvTable->clear();
    trvTable->setRowCount(count);
    QStringList headers = Util::scene()->problemInfo().hermes->localPointValueHeader();
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
        Util::scene()->sceneSolution()->setSolutionArray(i);

        Point point(txtPointX->value(), txtPointY->value());
        LocalPointValue *localPointValue = Util::scene()->problemInfo().hermes->localPointValue(point);

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
    Util::scene()->sceneSolution()->setSolutionArray(timeStep);
}

void ChartDialog::doPlot()
{
    if (tabAnalysisType->currentWidget() == widGeometry)
        plotGeometry();

    if (tabAnalysisType->currentWidget() == widTime)
        plotTime();
}

void ChartDialog::doFieldVariable(int index)
{
    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbFieldVariable->itemData(index).toInt();
    
    cmbFieldVariableComp->clear();
    if (isPhysicFieldVariableScalar(physicFieldVariable))
    {
        cmbFieldVariableComp->addItem(tr("Scalar"), PHYSICFIELDVARIABLECOMP_SCALAR);
    }
    else
    {
        cmbFieldVariableComp->addItem(tr("Magnitude"), PHYSICFIELDVARIABLECOMP_MAGNITUDE);
        cmbFieldVariableComp->addItem(Util::scene()->problemInfo().labelX(), PHYSICFIELDVARIABLECOMP_X);
        cmbFieldVariableComp->addItem(Util::scene()->problemInfo().labelY(), PHYSICFIELDVARIABLECOMP_Y);
    }
    
    if (cmbFieldVariableComp->currentIndex() == -1)
        cmbFieldVariableComp->setCurrentIndex(0);
}

void ChartDialog::doSaveImage()
{
    chart->saveImage();
}

void ChartDialog::doExportData()
{
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export data to file"), "data", tr("CSV files (*.csv);;Matlab/Octave script (*.m)"), &selectedFilter);
    if (!fileName.isEmpty())
    {
        // open file for write
        QFile file(fileName + selectedFilter);
        if (!file.open(QIODevice::WriteOnly))
        {
            cerr << "Could not create " + fileName.toStdString() + " file." << endl;
            return;
        }
        QTextStream out(&file);

        // export
        // csv
        QFileInfo fileInfo(fileName);
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

        file.close();
    }
}

void ChartDialog::doMoved(const QPoint &pos)
{
    QString info;
    info.sprintf("x=%g, y=%g", chart->invTransform(QwtPlot::xBottom, pos.x()), chart->invTransform(QwtPlot::yLeft, pos.y()));
}

void ChartDialog::doChartLine()
{
    if (isVisible())
    {
        if (tabAnalysisType->currentWidget() == widGeometry)
            emit setChartLine(Point(txtStartX->value(), txtStartY->value()), Point(txtEndX->value(), txtEndY->value()));
   
        if (tabAnalysisType->currentWidget() == widTime)
            emit setChartLine(Point(txtPointX->value(), txtPointY->value()), Point(txtPointX->value(), txtPointY->value()));
    }
    else
    {
        emit setChartLine(Point(), Point());
    }
}

void ChartDialog::doTimeStepChanged(int index)
{
    if (cmbTimeStep->currentIndex() != -1)
    {
        Util::scene()->sceneSolution()->setSolutionArray(cmbTimeStep->currentIndex());
        doPlot();
    }
}
