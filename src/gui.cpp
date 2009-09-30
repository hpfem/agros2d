#include "gui.h"

void fillComboBoxVariable(QComboBox *cmbFieldVariable, PhysicField physicField)
{
    // store variable
    PhysicFieldVariable physicFieldVariable = (PhysicFieldVariable) cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toInt();

    // clear combo
    cmbFieldVariable->clear();

    switch (physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        {
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL), PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD), PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT), PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY), PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY), PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY);
        }
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        {
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL), PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD), PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY), PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY);
        }
        break;
    case PHYSICFIELD_HARMONIC_MAGNETIC:
        {
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL_REAL), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL_REAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL_IMAG), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL_IMAG);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY_REAL), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY_REAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY_IMAG), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY_IMAG);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD_REAL), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD_REAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD_IMAG), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD_IMAG);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_REAL), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_REAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_IMAG), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_IMAG);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_POWER_LOSSES), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_POWER_LOSSES);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_ENERGY_DENSITY), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_ENERGY_DENSITY);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_PERMEABILITY), PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_PERMEABILITY);
        }
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        {
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HEAT_TEMPERATURE), PHYSICFIELDVARIABLE_HEAT_TEMPERATURE);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT), PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HEAT_FLUX), PHYSICFIELDVARIABLE_HEAT_FLUX);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY), PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY);
        }
        break;
    case PHYSICFIELD_CURRENT:
        {
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_POTENTIAL), PHYSICFIELDVARIABLE_CURRENT_POTENTIAL);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD), PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY), PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_LOSSES), PHYSICFIELDVARIABLE_CURRENT_LOSSES);
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY), PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY);
        }
        break;
    case PHYSICFIELD_ELASTICITY:
        {
            cmbFieldVariable->addItem(physicFieldVariableString(PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS), PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS);
        }
        break;
    default:
        std::cerr << "Physical field '" + physicFieldStringKey(physicField).toStdString() + "' is not implemented. fillComboBoxVariable(QComboBox *cmbFieldVariable, PhysicField physicField)" << endl;
        throw;
        break;
    }

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
}

// ***********************************************************************************************************

SLineEdit::SLineEdit(QWidget *parent) : QLineEdit(parent)
{   
    SLineEdit::SLineEdit("", true, true, parent);

    m_engine = scriptEngine();
}

SLineEdit::SLineEdit(const QString &contents, bool hasValidator, bool hasScriptEngine, QWidget *parent) : QLineEdit(contents, parent)
{
    if (hasValidator)
        this->setValidator(new QDoubleValidator(this));

    m_engine = scriptEngine();
}

SLineEdit::~SLineEdit()
{
    if (m_engine)
        delete m_engine;
}

double SLineEdit::value()
{
    if (m_engine)
    {
        QScriptValue scriptValue = m_engine->evaluate(text());
        if (scriptValue.isNumber())
            return scriptValue.toNumber();
    }
    else
    {
        return text().toDouble();
    }
}

void SLineEdit::setValue(double value)
{
    setText(QString::number(value));
}

// ***********************************************************************************************************

SLineEditValue::SLineEditValue(QWidget *parent) : QLineEdit(parent)
{
    setToolTip(tr("This textedit allows using variables."));
    setText("0");
}

Value SLineEditValue::value()
{
    return Value(text());
}

bool SLineEditValue::evaluate()
{
    Value val = value();
    if (val.evaluate(Util::scene()->problemInfo().scriptStartup))
    {
        m_number = val.number;
        return true;
    }
    else
    {
        setFocus();
        return false;
    }
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
        QStringList list;
        for (int i = 0; i < cmbCommand->count(); i++)
        {
            list.insert(0, cmbCommand->itemText(i));
        }

        // remove last item (over 30), empty strings and duplicates
        list.removeAll("");
        while (list.count() > 30)
            list.removeAt(0);
        list.removeDuplicates();

        QSettings settings;
        settings.setValue("CommandDialog/RecentCommands", list);
    }

    accept();
}

// ***********************************************************************************************************

EdgeMarkerDialog::EdgeMarkerDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Edge marker"));
    setWindowIcon(icon("scene-edge"));
    setModal(true);   

    // fill combo
    cmbMarker = new QComboBox(this);
    for (int i = 0; i<Util::scene()->edgeMarkers.count(); i++)
    {
        cmbMarker->addItem(Util::scene()->edgeMarkers[i]->name, Util::scene()->edgeMarkers[i]->variant());
    }

    // select marker
    cmbMarker->setCurrentIndex(-1);
    SceneEdgeMarker *marker = NULL;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edges[i]->isSelected)
        {
            if (!marker)
            {
                marker = Util::scene()->edges[i]->marker;
            }
            if (marker != Util::scene()->edges[i]->marker)
            {
                marker = NULL;
                break;
            }
        }
    }
    if (marker)
        cmbMarker->setCurrentIndex(cmbMarker->findData(marker->variant()));

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QHBoxLayout *layoutMarker = new QHBoxLayout();
    layoutMarker->addWidget(new QLabel(tr("Edge marker:")));
    layoutMarker->addWidget(cmbMarker);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutMarker);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMaximumSize(sizeHint());
}

EdgeMarkerDialog::~EdgeMarkerDialog()
{
    delete cmbMarker;
}

void EdgeMarkerDialog::doAccept()
{
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edges[i]->isSelected)
            Util::scene()->edges[i]->marker = marker();
    }

    accept();
}

// ******************************************************************************************************************

LabelMarkerDialog::LabelMarkerDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Edge marker"));
    setWindowIcon(icon("scene-label"));
    setModal(true);

    // fill combo
    cmbMarker = new QComboBox(this);
    for (int i = 0; i<Util::scene()->labelMarkers.count(); i++)
    {
        cmbMarker->addItem(Util::scene()->labelMarkers[i]->name, Util::scene()->labelMarkers[i]->variant());
    }

    // select marker
    cmbMarker->setCurrentIndex(-1);
    SceneLabelMarker *marker = NULL;
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labels[i]->isSelected)
        {
            if (!marker)
            {
                marker = Util::scene()->labels[i]->marker;
            }
            if (marker != Util::scene()->labels[i]->marker)
            {
                marker = NULL;
                break;
            }
        }
    }
    if (marker)
        cmbMarker->setCurrentIndex(cmbMarker->findData(marker->variant()));

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QHBoxLayout *layoutMarker = new QHBoxLayout();
    layoutMarker->addWidget(new QLabel(tr("Label marker:")));
    layoutMarker->addWidget(cmbMarker);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutMarker);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMaximumSize(sizeHint());
}

LabelMarkerDialog::~LabelMarkerDialog()
{
    delete cmbMarker;
}

void LabelMarkerDialog::doAccept()
{
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labels[i]->isSelected)
            Util::scene()->labels[i]->marker = marker();
    }

    accept();
}
