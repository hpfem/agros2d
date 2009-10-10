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
    
    delete cmbFieldVariable;
    delete cmbFieldVariableComp;

    delete picker;
    delete chart;    
}

void ChartDialog::showDialog()
{
    fillComboBoxScalarVariable(cmbFieldVariable);

    // correct labels
    lblStartX->setText(Util::scene()->problemInfo().labelX() + ":");
    lblStartY->setText(Util::scene()->problemInfo().labelY() + ":");
    lblEndX->setText(Util::scene()->problemInfo().labelX() + ":");
    lblEndY->setText(Util::scene()->problemInfo().labelY() + ":");
    radAxisX->setText(Util::scene()->problemInfo().labelX());
    radAxisY->setText(Util::scene()->problemInfo().labelY());

    show();
    doChartLine();
}

void ChartDialog::createControls()
{
    chart = new Chart(this);
    
    // controls
    QWidget *controls = new QWidget(this);
    QVBoxLayout *controlsLayout = new QVBoxLayout();
    controls->setLayout(controlsLayout);
    controls->setMinimumWidth(200);
    controls->setMaximumWidth(200);
    
    QPushButton *btnPlot = new QPushButton(controls);
    btnPlot->setText(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(doPlot()));
    
    QPushButton *btnSaveImage = new QPushButton(controls);
    btnSaveImage->setText(tr("Save image"));
    connect(btnSaveImage, SIGNAL(clicked()), SLOT(doSaveImage()));
    
    QPushButton *btnExportData = new QPushButton(controls);
    btnExportData->setText(tr("Export"));
    connect(btnExportData, SIGNAL(clicked()), SLOT(doExportData()));

    // QPushButton *btnPrint = new QPushButton(controls);
    // btnPrint->setText(tr("Print"));
    // connect(btnPrint, SIGNAL(clicked()), SLOT(doPrint()));
    
    lblStartX = new QLabel("X");
    lblStartY = new QLabel("Y");
    lblEndX = new QLabel("X");
    lblEndY = new QLabel("Y");

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
    
    controlsLayout->addWidget(grpStart);
    controlsLayout->addWidget(grpEnd);
    controlsLayout->addWidget(grpAxis);
    controlsLayout->addWidget(grpAxisPoints);
    controlsLayout->addWidget(grpVariable);
    controlsLayout->addStretch();
    controlsLayout->addWidget(widButton);
    
    // tab widget
    tabWidget = new QTabWidget(this);
    tabWidget->addTab(chart, icon(""), tr("Chart"));
    tabWidget->addTab(trvTable, icon(""), tr("Table"));
    
    // main layout
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(controls);
    layout->addWidget(tabWidget);
    
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

void ChartDialog::doPlot()
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

void ChartDialog::doPrint()
{
    QPrinter printer(QPrinter::HighResolution);
    
    QString docName = chart->title().text();
    if ( !docName.isEmpty() )
    {
        docName.replace (QRegExp (QString::fromLatin1 ("\n")), tr (" -- "));
        printer.setDocName (docName);
    }
    
    printer.setCreator("Chart");
    printer.setOrientation(QPrinter::Landscape);
    
    QPrintDialog dialog(&printer);
    if ( dialog.exec() )
    {
        QwtPlotPrintFilter filter;
        if ( printer.colorMode() == QPrinter::GrayScale )
        {
            int options = QwtPlotPrintFilter::PrintAll;
            options &= ~QwtPlotPrintFilter::PrintBackground;
            options |= QwtPlotPrintFilter::PrintFrameWithScales;
            filter.setOptions(options);
        }
        chart->print(printer, filter);
    }
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
        emit setChartLine(Point(txtStartX->value(), txtStartY->value()), Point(txtEndX->value(), txtEndY->value()));
    else
        emit setChartLine(Point(), Point());
}

void ChartDialog::hideEvent(QHideEvent *event)
{
    doChartLine();
}
