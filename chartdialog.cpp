#include "chartdialog.h"

ChartDialog::ChartDialog(Scene *scene, QWidget *parent) : QDialog(parent)
{
    QSettings settings;
    restoreGeometry(settings.value("ChartDialog/Geometry", saveGeometry()).toByteArray());

    m_scene = scene;

    setMinimumSize(800, 600);
    setWindowIcon(icon("chart"));
    setWindowTitle(tr("Chart"));
    setWindowModality(Qt::NonModal);

    createControls();   
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

void ChartDialog::createControls()
{
    chart = new Chart(m_scene, this);

    // controls
    QWidget *controls = new QWidget(this);
    QVBoxLayout *controlsLayout = new QVBoxLayout();
    controls->setLayout(controlsLayout);
    controls->setMinimumWidth(200);
    controls->setMaximumWidth(200);

    QPushButton *btnPlot = new QPushButton(controls);
    btnPlot->setText(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(doPlot()));

    QPushButton *btnPrint = new QPushButton(controls);
    btnPrint->setText(tr("Print"));
    connect(btnPrint, SIGNAL(clicked()), SLOT(doPrint()));

    txtStartX = new SLineEdit("0.01", false);
    txtStartY = new SLineEdit("0.02", false);
    txtEndX = new SLineEdit("0.05", false);
    txtEndY = new SLineEdit("0.02", false);

    // start
    QFormLayout *layoutStart = new QFormLayout();
    layoutStart->addRow(tr("X:"), txtStartX);
    layoutStart->addRow(tr("Y:"), txtStartY);

    QGroupBox *grpStart = new QGroupBox(tr("Start"), this);
    grpStart->setLayout(layoutStart);

    QFormLayout *layoutEnd = new QFormLayout();
    layoutEnd->addRow(tr("X:"), txtEndX);
    layoutEnd->addRow(tr("Y:"), txtEndY);

    // to
    QGroupBox *grpEnd = new QGroupBox(tr("End"), this);
    grpEnd->setLayout(layoutEnd);

    // x - axis
    radAxisLength = new QRadioButton(tr("Length"), this);
    radAxisLength->setChecked(true);
    radAxisX = new QRadioButton(tr("X"), this);
    radAxisY = new QRadioButton(tr("Y"), this);

    QButtonGroup *axisGroup = new QButtonGroup(this);
    axisGroup->addButton(radAxisLength);
    axisGroup->addButton(radAxisX);
    axisGroup->addButton(radAxisY);

    // axis points
    txtAxisPoints = new QSpinBox(this);
    txtAxisPoints->setMinimum(2);
    txtAxisPoints->setMaximum(200);
    txtAxisPoints->setValue(50);

    QFormLayout *layoutAxisPoints = new QFormLayout();
    layoutAxisPoints->addRow(tr("Points:"), txtAxisPoints);

    QVBoxLayout *layoutAxis = new QVBoxLayout(this);
    layoutAxis->addWidget(radAxisLength);
    layoutAxis->addWidget(radAxisX);
    layoutAxis->addWidget(radAxisY);
    layoutAxis->addLayout(layoutAxisPoints);

    QGroupBox *grpAxis = new QGroupBox(tr("Horizontal axis"), this);
    grpAxis->setLayout(layoutAxis);


    // plot   
    // variable
    cmbFieldVariable = new QComboBox(this);
    fillComboBoxVariable(cmbFieldVariable, m_scene->projectInfo().physicField);
    connect(cmbFieldVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(doFieldVariable(int)));
    // component
    cmbFieldVariableComp = new QComboBox(this);
    doFieldVariable(cmbFieldVariable->currentIndex());

    QVBoxLayout *layoutVariable = new QVBoxLayout(this);
    layoutVariable->addWidget(cmbFieldVariable);
    layoutVariable->addWidget(cmbFieldVariableComp);

    QGroupBox *grpVariable = new QGroupBox(tr("Variable"), this);
    grpVariable->setLayout(layoutVariable);

    // button bar
    QHBoxLayout *layoutButton = new QHBoxLayout();
    layoutButton->addWidget(btnPlot);
    layoutButton->addWidget(btnPrint);

    QWidget *widButton = new QWidget(this);
    widButton->setLayout(layoutButton);

    controlsLayout->addWidget(grpStart);
    controlsLayout->addWidget(grpEnd);
    controlsLayout->addWidget(grpAxis);
    controlsLayout->addWidget(grpVariable);
    controlsLayout->addStretch();
    controlsLayout->addWidget(widButton);

    // main layout
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(controls);
    layout->addWidget(chart);

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
    int count = txtAxisPoints->value();
    double *xval = new double[count];
    double *yval = new double[count];

    // variable
    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toInt();
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt();

    chart->setTitle(physicFieldVariableString(physicFieldVariable) + " - " + physicFieldVariableCompString(physicFieldVariableComp));
    chart->setAxisTitle(QwtPlot::yLeft, physicFieldVariableString(physicFieldVariable) + " (" + physicFieldVariableUnits(physicFieldVariable) + ")");

    // chart
    if (radAxisLength->isChecked()) chart->setAxisTitle(QwtPlot::xBottom, tr("Length (m)"));
    if (radAxisX->isChecked()) chart->setAxisTitle(QwtPlot::xBottom, tr("X (m)"));
    if (radAxisY->isChecked()) chart->setAxisTitle(QwtPlot::xBottom, tr("Y (m)"));

    // line
    Point start(txtStartX->value(), txtStartY->value());
    Point end(txtEndX->value(), txtEndY->value());

    Point diff((end.x - start.x)/(count-1), (end.y - start.y)/(count-1));

    // calculate values
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

        delete localPointValue;
    }

    chart->setData(xval, yval, count);
    // chart->setAxisScale(0, sqrt(sqr(start.x) + sqr(start.y)), sqrt(sqr(end.x) + sqr(end.y)));
    // chart->setAxisScale(1, 150, 300);

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
        cmbFieldVariableComp->addItem(tr("X"), PHYSICFIELDVARIABLECOMP_X);
        cmbFieldVariableComp->addItem(tr("Y"), PHYSICFIELDVARIABLECOMP_Y);
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

void ChartDialog::doMoved(const QPoint &pos)
{
    QString info;
    info.sprintf("x=%g, y=%g", chart->invTransform(QwtPlot::xBottom, pos.x()), chart->invTransform(QwtPlot::yLeft, pos.y()));
}

// *********************************************************************************************************************

Chart::Chart(Scene *scene, QWidget *parent) : QwtPlot(parent)
{
    m_scene = scene;

    //  chart style
    setAutoReplot(false);
    setMargin(5);
    setTitle("Chart");
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
    grid->attach(this);

    // axes
    setAxisTitle(QwtPlot::xBottom, " ");
    setAxisFont(QwtPlot::xBottom, QFont("Helvetica", 10, QFont::Normal));
    setAxisTitle(QwtPlot::yLeft, " ");
    setAxisFont(QwtPlot::yLeft, QFont("Helvetica", 10, QFont::Normal));

    // curve styles
    QwtSymbol sym;

    sym.setStyle(QwtSymbol::Cross);
    sym.setPen(QColor(Qt::black));
    sym.setSize(5);

    // curve
    m_curve = new QwtPlotCurve("Amplitude");
    m_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_curve->setPen(QPen(Qt::blue));
    m_curve->setCurveAttribute(QwtPlotCurve::Fitted);
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
