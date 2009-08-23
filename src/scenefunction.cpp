#include "scenefunction.h"

SceneFunction::SceneFunction(const QString &name, const QString &function)
{
    this->name = name;
    this->function = function;
   
    m_engine = scriptEngine();
}

SceneFunction::~SceneFunction()
{
    delete m_engine;
}

QString SceneFunction::script()
{
    return QString("addFunction(\"%1\", \"%2\");").
            arg(name).
            arg(function);
}

QVariant SceneFunction::variant()
{
    QVariant v;
    v.setValue(this);
    return v;
}

double SceneFunction::evaluate(double number) throw (const QString &)
{
    // evaluate
    QString text = QString("var x = %1; \n %2").arg(number).arg(function);
    QScriptValue scriptValue = m_engine->evaluate(text);
    if (scriptValue.isNumber())
    {
        return scriptValue.toNumber();
    }
    else
    {
        throw QString(QObject::tr("Expression '%1' cannot be evaluated.").arg(text));
    }
}

int SceneFunction::showDialog(QWidget *parent)
{
    DSceneFunction *dialog = new DSceneFunction(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************

DSceneFunction::DSceneFunction(SceneFunction *sceneFunction, QWidget *parent) : QDialog(parent)
{
    m_sceneFunction = sceneFunction;

    setWindowIcon(icon("scene-function"));
    setWindowTitle(tr("Function"));

    createControls();
    load();
    doPlot();

    setMinimumSize(sizeHint());
}

DSceneFunction::~DSceneFunction()
{
    delete txtName;
    delete txtFunction;
    delete txtStart;
    delete txtEnd;
}

void DSceneFunction::createControls()
{
    // chart
    chart = new Chart(this);
    // axis labels
    QwtText text("");
    text.setFont(QFont("Helvetica", 10, QFont::Normal));
    text.setText("x");
    chart->setAxisTitle(QwtPlot::xBottom, text);
    text.setText("y");
    chart->setAxisTitle(QwtPlot::yLeft, text);

    // name
    txtName = new QLineEdit("");
    txtFunction = new SLineEdit("1", false, this);

    // interval
    txtStart = new SLineEdit("0", true);
    txtStart->setMaximumWidth(100);
    connect(txtStart, SIGNAL(editingFinished()), this, SLOT(doPlot()));

    txtEnd = new SLineEdit("10", true);
    txtEnd->setMaximumWidth(100);
    connect(txtEnd, SIGNAL(editingFinished()), this, SLOT(doPlot()));

    QPushButton *btnPlot = new QPushButton(this);
    btnPlot->setText(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(doPlot()));

    QPushButton *btnSaveImage = new QPushButton(this);
    btnSaveImage->setText(tr("Save image"));
    connect(btnSaveImage, SIGNAL(clicked()), SLOT(doSaveImage()));

    QGridLayout *controlsLayout = new QGridLayout();
    controlsLayout->addWidget(new QLabel(tr("Start:")), 0, 0);
    controlsLayout->addWidget(txtStart, 0, 1);
    controlsLayout->addWidget(new QLabel(tr("End:")), 1, 0);
    controlsLayout->addWidget(txtEnd, 1, 1);
    controlsLayout->addWidget(new QLabel(tr("Name:")), 0, 2);
    controlsLayout->addWidget(txtName, 0, 3);
    controlsLayout->addWidget(new QLabel(tr("Function:")), 1, 2);
    controlsLayout->addWidget(txtFunction, 1, 3);
    controlsLayout->addWidget(btnPlot, 0, 4);
    controlsLayout->addWidget(btnSaveImage, 1, 4);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(chart);
    layout->addLayout(controlsLayout);
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void DSceneFunction::doSaveImage()
{
    chart->saveImage();
}

void DSceneFunction::doPlot()
{
    int count = 200;
    double step = (txtEnd->value() - txtStart->value()) / (double) count;
    double *xval = new double[count];
    double *yval = new double[count];

    SceneFunction function("plot", txtFunction->text());

    // calculate values
    for (int i = 0; i<count; i++)
    {
        xval[i] = txtStart->value() + i*step;

        // evaluate
        try
        {
            yval[i] = function.evaluate(xval[i]);
        }
        catch (const QString &e)
        {
            delete[] xval;
            delete[] yval;
            xval = new double[0];
            yval = new double[0];
            count = 0;

            QMessageBox::warning(QApplication::activeWindow(), QObject::tr("Error"), QObject::tr("Expression '%1' cannot be evaluated.").arg(txtFunction->text()));

            break;
        }
    }

    chart->setData(xval, yval, count);

    delete[] xval;
    delete[] yval;
}

void DSceneFunction::load()
{
    txtName->setText(m_sceneFunction->name);
    txtFunction->setText(m_sceneFunction->function);
}

bool DSceneFunction::save()
{
    m_sceneFunction->name = txtName->text();
    m_sceneFunction->function = txtFunction->text();

    return true;
}

void DSceneFunction::doAccept()
{
    if (save())
        accept();
}

void DSceneFunction::doReject()
{
    reject();
}
