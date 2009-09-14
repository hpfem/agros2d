#include "reportdialog.h"

#include "scene.h"
#include "scripteditordialog.h"

ReportDialog::ReportDialog(QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon("script-startup"));
    setWindowTitle(tr("Report"));
    setWindowFlags(Qt::Window);
    setMinimumSize(400, 400);

    createControls();
}

ReportDialog::~ReportDialog()
{

}

void ReportDialog::createControls()
{
    view = new QWebView(this);

    // dialog buttons
    QPushButton *btnOpen = new QPushButton(tr("Open in external viewer"));
    connect(btnOpen, SIGNAL(clicked()), this, SLOT(doOpen()));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    buttonBox->addButton(btnOpen, QDialogButtonBox::ApplyRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(view);
    // layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void ReportDialog::doAccept()
{
    accept();
}

void ReportDialog::doOpen()
{
    QDesktopServices::openUrl(tempProblemDir() + "/report/index.html");
}

void ReportDialog::showDialog()
{
    QDir(tempProblemDir()).mkdir("report");

    QFile::copy(QString("%1/doc/report/template/template.html").arg(datadir()), tempProblemDir() + "/report/template.html");
    QFile::copy(QString("%1/doc/report/template/default.css").arg(datadir()), tempProblemDir() + "/report/default.css");

    generateIndex();

    view->load(QUrl("file://" + tempProblemDir() + "/report/index.html"));
    view->show();

    show();
}

void ReportDialog::generateIndex()
{
    QString fileNameTemplate = tempProblemDir() + "/report/template.html";
    QString fileNameIndex = tempProblemDir() + "/report/index.html";

    QString content;

    // load template.html
    QFile fileTemplate(fileNameTemplate);
    if (fileTemplate.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&fileTemplate);
        content = stream.readAll();

        fileTemplate.close();
    }
    QFile::remove(fileNameTemplate);

    // save index.html
    QFile fileIndex(fileNameIndex);
    if (fileIndex.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&fileIndex);

        stream << replaceTemplates(content);

        fileIndex.flush();
        fileIndex.close();
    }
}

QString ReportDialog::replaceTemplates(const QString &source)
{
    QString destination = source;

    // common
    destination.replace("[Date]", QDateTime(QDateTime::currentDateTime()).toString("dd.MM.yyyy hh:mm"), Qt::CaseSensitive);

    // problem info
    destination.replace("[Problem.Name]", Util::scene()->problemInfo().name, Qt::CaseSensitive);
    destination.replace("[Problem.Date]", Util::scene()->problemInfo().date.toString("dd.MM.yyyy"), Qt::CaseSensitive);
    destination.replace("[Problem.FileName]", QFileInfo(Util::scene()->problemInfo().fileName).completeBaseName(), Qt::CaseSensitive);
    destination.replace("[Problem.ProblemType]", problemTypeString(Util::scene()->problemInfo().problemType), Qt::CaseSensitive);
    destination.replace("[Problem.PhysicField]", physicFieldString(Util::scene()->problemInfo().physicField), Qt::CaseSensitive);
    destination.replace("[Problem.NumberOfRefinements]", QString::number(Util::scene()->problemInfo().numberOfRefinements), Qt::CaseSensitive);
    destination.replace("[Problem.PolynomialOrder]", QString::number(Util::scene()->problemInfo().polynomialOrder), Qt::CaseSensitive);
    destination.replace("[Problem.AdaptivityType]", adaptivityTypeString(Util::scene()->problemInfo().adaptivityType), Qt::CaseSensitive);
    destination.replace("[Problem.AdaptivitySteps]", QString::number(Util::scene()->problemInfo().adaptivitySteps), Qt::CaseSensitive);
    destination.replace("[Problem.AdaptivityTolerance]", QString::number(Util::scene()->problemInfo().adaptivityTolerance), Qt::CaseSensitive);
    destination.replace("[Problem.Frequency]", QString::number(Util::scene()->problemInfo().frequency), Qt::CaseSensitive);

    // script
    destination.replace("[Script]", createEcmaFromModel(), Qt::CaseSensitive);

    return destination;
}
