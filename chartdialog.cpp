#include "chartdialog.h"

ChartDialog::ChartDialog(Scene *scene, QWidget *parent) : QDialog(parent)
{
    this->m_scene = scene;

    m_N = 500;

    setMinimumSize(500, 400);
    setWindowIcon(getIcon("chart"));
    setWindowTitle(tr("Chart"));
    setWindowModality(Qt::NonModal);

    createControls();

    resize(800, 600);
}

ChartDialog::~ChartDialog()
{
    delete controls;
    delete chart;
}

void ChartDialog::createControls()
{
    chart = new Chart(this);

    // controls
    controls = new QGroupBox("Controls", this);
    QGridLayout *controlsLayout = new QGridLayout(controls);
    controls->setLayout(controlsLayout);

    QPushButton *btnPlot = new QPushButton(controls);
    btnPlot->setIcon(getIcon("chart"));
    btnPlot->setText(tr("Plot chart"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(doPlot()));

    QPushButton *btnPrint = new QPushButton(controls);
    btnPrint->setIcon(getIcon("chart"));
    btnPrint->setText(tr("Print chart"));
    connect(btnPrint, SIGNAL(clicked()), SLOT(doPrint()));

    QPushButton *btnZoom = new QPushButton(controls);
    
    btnZoom->setIcon(getIcon("chart"));
    btnZoom->setText(tr("Zoom chart"));
    connect(btnZoom, SIGNAL(toggled(bool)), SLOT(doEnableZoomMode(bool)));

    controlsLayout->addWidget(btnPlot, 0, 0);
    controlsLayout->addWidget(btnPrint, 1, 0);
    controlsLayout->addWidget(btnZoom, 2, 0);
    controlsLayout->setRowStretch(1, 100);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(controls);
    mainLayout->addWidget(chart);

    panner = new QwtPlotPanner(chart->canvas());
    panner->setMouseButton(Qt::MidButton);

    picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                               QwtPicker::PointSelection | QwtPicker::DragSelection,
                               QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
                               chart->canvas());
    picker->setRubberBandPen(QColor(Qt::green));
    picker->setRubberBand(QwtPicker::CrossRubberBand);
    picker->setTrackerMode(QwtPicker::ActiveOnly);
    picker->setTrackerPen(QColor(Qt::black));

    zoomer = new QwtPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, chart->canvas());
    zoomer->setRubberBand(QwtPicker::RectRubberBand);
    zoomer->setRubberBandPen(QColor(Qt::green));
    zoomer->setTrackerMode(QwtPicker::AlwaysOff);
    zoomer->setTrackerPen(QColor(Qt::black));
    zoomer->setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);

    connect(picker, SIGNAL(moved(const QPoint &)), SLOT(doMoved(const QPoint &)));
    connect(picker, SIGNAL(selected(const QwtPolygon &)), SLOT(doSelected(const QwtPolygon &)));
    connect(zoomer, SIGNAL(selected(const QwtPolygon &)), SLOT(doZoomed(const QwtPolygon &)));

    doEnableZoomMode(false);

    setLayout(mainLayout);
}

void ChartDialog::doPlot()
{
    chart->setData();
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

void ChartDialog::showInfo(QString text)
{
    if ( text == QString::null )
    {
        if (picker->rubberBand())
            text = "Cursor Pos: Press left mouse button in plot region";
        else
            text = "Zoom: Press mouse button and drag";
    }

    cout << text.toStdString() << endl;
}

void ChartDialog::doMoved(const QPoint &pos)
{
    QString info;
    info.sprintf("x=%g, y=%g", chart->invTransform(QwtPlot::xBottom, pos.x()), chart->invTransform(QwtPlot::yLeft, pos.y()));
    showInfo(info);
}

void ChartDialog::doSelected(const QwtPolygon &)
{
    showInfo();
}

void ChartDialog::doZoomed(const QwtPolygon &)
{
    doEnableZoomMode(false);    
}

void ChartDialog::doEnableZoomMode(bool on)
{
    panner->setEnabled(on);

    zoomer->setEnabled(on);
    zoomer->zoom(0);

    picker->setEnabled(!on);

    showInfo();
}

// *********************************************************************************************************************

Chart::Chart(QWidget *parent) : QwtPlot(parent)
{
    //  chart style
    setAutoReplot(false);
    setMargin(5);
    setTitle("Chart");
    setCanvasBackground(QColor(Qt::white));

    // legend
    QwtLegend *legend = new QwtLegend;
    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
    insertLegend(legend, QwtPlot::BottomLegend);

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajPen(QPen(Qt::darkGray, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(this);

    // axes
    setAxisTitle(QwtPlot::xBottom, "x (m)");
    setAxisTitle(QwtPlot::yLeft, "y (J)");

    setAxisMaxMajor(QwtPlot::xBottom, 6);
    setAxisMaxMinor(QwtPlot::xBottom, 10);

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

void Chart::setData()
{
    int count = 27;
    double xval[count];
    double yval[count];

    const bool doReplot = autoReplot();
    setAutoReplot(false);

    // calculate values
    for(int i = 0; i<count; i++)
    {   xval[i] = double(i) * 10.0 / double(count - 1);
        yval[i] = sin(xval[i]) * cos(2.0 * xval[i]);
    }

    m_curve->setData(xval, yval, count);

    setAutoReplot(doReplot);

    replot();
}
