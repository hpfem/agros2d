#include "gui.h"
#include "scene.h"

void addTreeWidgetItemValue(QTreeWidgetItem *parent, const QString &name, const QString &text, const QString &unit)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, name);
    item->setText(1, text);
    item->setTextAlignment(1, Qt::AlignRight);
    item->setText(2, unit + " ");
    item->setTextAlignment(2, Qt::AlignLeft);
}

void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
{
    // store variable
    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toInt();

    // clear combo
    cmbFieldVariable->clear();
    Util::scene()->problemInfo()->hermes()->fillComboBoxScalarVariable(cmbFieldVariable);

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
}

void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
{
    // store variable
    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toInt();

    // clear combo
    cmbFieldVariable->clear();
    Util::scene()->problemInfo()->hermes()->fillComboBoxVectorVariable(cmbFieldVariable);

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
}

void fillComboBoxTimeStep(QComboBox *cmbFieldVariable)
{
    cmbFieldVariable->blockSignals(true);

    // store variable
    int timeStep = cmbFieldVariable->currentIndex();
    if (timeStep == -1) timeStep = 0;

    // clear combo
    cmbFieldVariable->clear();
    for (int i = 0; i < Util::scene()->sceneSolution()->timeStepCount(); i++)
    {
        cmbFieldVariable->addItem(QString::number(Util::scene()->sceneSolution()->solutionArrayList()->value(i)->time, 'e', 2), i);
    }

    cmbFieldVariable->setCurrentIndex(timeStep);
    cmbFieldVariable->blockSignals(false);
}

// ***********************************************************************************************************

SLineEditValue::SLineEditValue(QWidget *parent) : QWidget(parent)
{
    // create controls
    txtLineEdit = new QLineEdit(this);
    txtLineEdit->setToolTip(tr("This textedit allows using variables."));
    txtLineEdit->setText("0");
    connect(txtLineEdit, SIGNAL(editingFinished()), this, SLOT(evaluate()));

    lblValue = new QLabel(this);    

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->addWidget(txtLineEdit, 1);
    layout->addWidget(lblValue, 0, Qt::AlignRight);

    setLayout(layout);    
}

void SLineEditValue::setValue(Value value)
{
   txtLineEdit->setText(value.text);
   evaluate();
}

Value SLineEditValue::value()
{
    return Value(txtLineEdit->text());
}

bool SLineEditValue::evaluate()
{
    Value val = value();
    if (val.evaluate())
    {
        m_number = val.number;
        lblValue->setText(QString("%1").arg(m_number, 0, 'g', 2));
        return true;
    }
    else
    {
        setFocus();
        return false;
    }
}

void SLineEditValue::focusInEvent(QFocusEvent *event)
{
    txtLineEdit->setFocus(event->reason());
}

double SLineEditValue::number()
{
    if (evaluate())
        return m_number;
}

// ****************************************************************************************************************


Chart::Chart(QWidget *parent) : QwtPlot(parent)
{
    //  chart style
    setAutoReplot(false);
    setMargin(5);
    setTitle("");
    setCanvasBackground(QColor(Qt::white));
    setMinimumSize(700, 450);

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

void Chart::saveImage(const QString &fileName)
{
    QString fileNameTemp;
    if (fileName.isEmpty())
    {
        fileNameTemp = QFileDialog::getSaveFileName(this, tr("Export image to file"), "data", tr("PNG files (*.png)"));
    }
    else
    {
        fileNameTemp = fileName;
    }

    if (!fileNameTemp.isEmpty())
    {
        QFileInfo fileInfo(fileNameTemp);
        if (fileInfo.suffix().toLower() != "png") fileNameTemp += ".png";

        QImage image(1024, 768, QImage::Format_ARGB32);
        print(image);
        image.save(fileNameTemp, "PNG");
    }
}

void Chart::setData(double *xval, double *yval, int count)
{
    const bool doReplot = autoReplot();
    setAutoReplot(false);

    m_curve->setData(xval, yval, count);

    setAutoReplot(doReplot);

    replot();
}

// ***********************************************************************************************************

CommandDialog::CommandDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Command"));
    setWindowIcon(icon("system-run"));   

    cmbCommand = new QComboBox(this);
    cmbCommand->setEditable(true);
    cmbCommand->setMinimumWidth(350);

    // completer
    QSettings settings;
    QStringList list;
    list = settings.value("CommandDialog/RecentCommands").value<QStringList>();
    /*
    QStringListModel *model = new QStringListModel();
    model->setStringList(list);

    completer = new QCompleter(model, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    cmbCommand->setCompleter(completer);
    */
    cmbCommand->addItem("", "");
    cmbCommand->addItems(list);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QHBoxLayout *layoutCommand = new QHBoxLayout();
    layoutCommand->addWidget(new QLabel(tr("Enter command:")));
    layoutCommand->addWidget(cmbCommand);
    layoutCommand->addStretch();

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutCommand);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMaximumSize(sizeHint());
}

CommandDialog::~CommandDialog()
{
    delete cmbCommand;
    // delete completer;
}

void CommandDialog::doAccept()
{
    if (!command().isEmpty())
    {
        QSettings settings;
        QStringList list;
        list = settings.value("CommandDialog/RecentCommands").value<QStringList>();
        list.insert(0, command());

        // remove last item (over 30), empty strings and duplicates
        list.removeAll("");
        list.removeDuplicates();
        while (list.count() > 30)
            list.removeAt(0);

        settings.setValue("CommandDialog/RecentCommands", list);
    }

    accept();
}
