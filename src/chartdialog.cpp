#include "chartdialog.h"

ChartDialog::ChartDialog(Scene *scene, QWidget *parent) : QDialog(parent)
{
    QSettings settings;
    restoreGeometry(settings.value("ChartDialog/Geometry", saveGeometry()).toByteArray());
    
    m_scene = scene;
    
    setWindowIcon(icon("chart"));
    setWindowTitle(tr("Chart"));

    createControls();   
    setMinimumSize(sizeHint());
}

ChartDialog::~ChartDialog()
{
    QSettings settings;
    settings.setValue("ChartDialog/Geometry", saveGeometry());
    
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
    fillComboBoxVariable(cmbFieldVariable, m_scene->projectInfo().physicField);    
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
    
    txtStartX = new SLineEdit("0.01", false);
    txtStartY = new SLineEdit("0.02", false);
    txtEndX = new SLineEdit("0.05", false);
    txtEndY = new SLineEdit("0.02", false);

    connect(txtStartX, SIGNAL(editingFinished()), this, SLOT(doChartLine()));
    connect(txtStartY, SIGNAL(editingFinished()), this, SLOT(doChartLine()));
    connect(txtEndX, SIGNAL(editingFinished()), this, SLOT(doChartLine()));
    connect(txtEndY, SIGNAL(editingFinished()), this, SLOT(doChartLine()));

    // start
    QFormLayout *layoutStart = new QFormLayout();
    layoutStart->addRow(m_scene->projectInfo().labelX() + ":", txtStartX);
    layoutStart->addRow(m_scene->projectInfo().labelY() + ":", txtStartY);
    
    QGroupBox *grpStart = new QGroupBox(tr("Start"), this);
    grpStart->setLayout(layoutStart);
    
    // end
    QFormLayout *layoutEnd = new QFormLayout();
    layoutEnd->addRow(m_scene->projectInfo().labelX() + ":", txtEndX);
    layoutEnd->addRow(m_scene->projectInfo().labelY() + ":", txtEndY);
    
    QGroupBox *grpEnd = new QGroupBox(tr("End"), this);
    grpEnd->setLayout(layoutEnd);
    
    // x - axis
    radAxisLength = new QRadioButton(tr("Length"), this);
    radAxisLength->setChecked(true);
    radAxisX = new QRadioButton(m_scene->projectInfo().labelX(), this);
    radAxisY = new QRadioButton(m_scene->projectInfo().labelY(), this);
    
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
    QStringList headers = localPointValueHeaderFactory(m_scene->projectInfo().physicField);
    trvTable->setColumnCount(headers.count());
    trvTable->setHorizontalHeaderLabels(headers);
    
    // chart
    if (radAxisLength->isChecked()) text.setText(tr("Length (m)"));
    if (radAxisX->isChecked()) text.setText(m_scene->projectInfo().labelX() + " (m):");
    if (radAxisY->isChecked()) text.setText(m_scene->projectInfo().labelY() + " (m):");
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
        LocalPointValue *localPointValue = localPointValueFactory(point, m_scene);
        
        // x value
        if (radAxisLength->isChecked()) xval[i] = sqrt(sqr(i*diff.x) + sqr(i*diff.y));
        if (radAxisX->isChecked()) xval[i] = start.x + i*diff.x;
        if (radAxisY->isChecked()) xval[i] = start.y + i*diff.y;
        
        // y value
        yval[i] = localPointValue->variableValue(physicFieldVariable, physicFieldVariableComp);
        // yval[i] = -m_scene->sceneSolution()->sln()->get_pt_value(point.x, point.y, FN_DX_0);
        // yval[i] = 10/(point.x * (log(0.03/0.01)/10 + log(0.05/0.03)/3) * ((point.x<0.03) ? 10 : 3));
        
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
        cmbFieldVariableComp->addItem(m_scene->projectInfo().labelX(), PHYSICFIELDVARIABLECOMP_X);
        cmbFieldVariableComp->addItem(m_scene->projectInfo().labelX(), PHYSICFIELDVARIABLECOMP_Y);
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
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export image to file"), "data", "PNG files (*.png)");
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().toLower() != "png") fileName += ".png";
        
        QImage image(1024, 768, QImage::Format_ARGB32);
        chart->print(image);
        image.save(fileName, "PNG");
    }
}

void ChartDialog::doExportData()
{
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export data to file"), "data", "CSV files (*.csv);;Matlab/Octave script (*.m)", &selectedFilter);
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

// *********************************************************************************************************************

Chart::Chart(QWidget *parent) : QwtPlot(parent)
{
    //  chart style
    setAutoReplot(false);
    setMargin(5);
    setTitle("");
    setCanvasBackground(QColor(Qt::white));
    
    // legend
    /*
    QwtLegend *legend = new QwtLegend;
    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
    insertLegend(legend, QwtPlot::BottomLegend);
    */
    
    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajPen(QPen(Qt::darkGray, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->enableX(true);
    grid->enableY(true);
    grid->enableXMin(true);
    grid->enableYMin(true);
    grid->attach(this);
    
    // axes
    setAxisTitle(QwtPlot::xBottom, " ");
    setAxisFont(QwtPlot::xBottom, QFont("Helvetica", 9, QFont::Normal));
    setAxisTitle(QwtPlot::yLeft, " ");
    setAxisFont(QwtPlot::yLeft, QFont("Helvetica", 9, QFont::Normal));
    
    // curve styles
    QwtSymbol sym;
    
    sym.setStyle(QwtSymbol::Cross);
    sym.setPen(QColor(Qt::black));
    sym.setSize(5);
    
    // curve
    m_curve = new QwtPlotCurve();
    m_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_curve->setPen(QPen(Qt::blue));
    m_curve->setCurveAttribute(QwtPlotCurve::Inverted);
    m_curve->setYAxis(QwtPlot::yLeft);
    m_curve->attach(this);
}

Chart::~Chart()
{
    delete m_curve;
}

void Chart::setData(double *xval, double *yval, int count)
{
    const bool doReplot = autoReplot();
    setAutoReplot(false);
    
    m_curve->setData(xval, yval, count);    

    setAutoReplot(doReplot);
    
    replot();
}
