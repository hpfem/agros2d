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

    view->load(QUrl::fromLocalFile(tempProblemDir() + "/report/index.html"));
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
    destination.replace("[Problem.Name]", Util::scene()->problemInfo()->name, Qt::CaseSensitive);
    destination.replace("[Problem.Date]", Util::scene()->problemInfo()->date.toString("dd.MM.yyyy"), Qt::CaseSensitive);
    destination.replace("[Problem.FileName]", QFileInfo(Util::scene()->problemInfo()->fileName).completeBaseName(), Qt::CaseSensitive);
    destination.replace("[Problem.ProblemType]", problemTypeString(Util::scene()->problemInfo()->problemType), Qt::CaseSensitive);
    destination.replace("[Problem.PhysicField]", physicFieldString(Util::scene()->problemInfo()->physicField()), Qt::CaseSensitive);
    destination.replace("[Problem.NumberOfRefinements]", QString::number(Util::scene()->problemInfo()->numberOfRefinements), Qt::CaseSensitive);
    destination.replace("[Problem.PolynomialOrder]", QString::number(Util::scene()->problemInfo()->polynomialOrder), Qt::CaseSensitive);
    destination.replace("[Problem.AdaptivityType]", adaptivityTypeString(Util::scene()->problemInfo()->adaptivityType), Qt::CaseSensitive);
    destination.replace("[Problem.AdaptivitySteps]", QString::number(Util::scene()->problemInfo()->adaptivitySteps), Qt::CaseSensitive);
    destination.replace("[Problem.AdaptivityTolerance]", QString::number(Util::scene()->problemInfo()->adaptivityTolerance), Qt::CaseSensitive);

    destination.replace("[Problem.Frequency]", QString::number(Util::scene()->problemInfo()->frequency), Qt::CaseSensitive);
    destination.replace("[Problem.AnalysisType]", analysisTypeString(Util::scene()->problemInfo()->analysisType), Qt::CaseSensitive);
    destination.replace("[Problem.TimeStep]", QString::number(Util::scene()->problemInfo()->timeStep), Qt::CaseSensitive);
    destination.replace("[Problem.TimeTotal]", QString::number(Util::scene()->problemInfo()->timeTotal), Qt::CaseSensitive);
    destination.replace("[Problem.InititalCondition]", QString::number(Util::scene()->problemInfo()->initialCondition), Qt::CaseSensitive);

    // script
    destination.replace("[Script]", createEcmaFromModel(), Qt::CaseSensitive);

    // physical properties
    destination.replace("[Materials]", htmlMaterials(), Qt::CaseSensitive);
    destination.replace("[Boundaries]", htmlBoundaries(), Qt::CaseSensitive);

    // geometry
    destination.replace("[Geometry.Nodes]", htmlGeometryNodes(), Qt::CaseSensitive);
    destination.replace("[Geometry.Edges]", htmlGeometryEdges(), Qt::CaseSensitive);
    destination.replace("[Geometry.Labels]", htmlGeometryLabels(), Qt::CaseSensitive);

    return QString(destination);
}

QString ReportDialog::htmlMaterials()
{
    QString out;

    out  = "\n";
    for (int i = 1; i < Util::scene()->labelMarkers.count(); i++)
    {
        SceneLabelMarker *marker = Util::scene()->labelMarkers[i];
        out += "<h5>" + marker->name + "</h5>";

        out += "<table>";
        QMap<QString, QString> data = marker->data();
        for (int j = 0; j < data.keys().length(); j++)
        {
            out += "<tr>";
            out += "<td>" + data.keys()[j] + "</td>";
            out += "<td>" + data.values()[j] + "</td>";
            out += "</tr>";
        }
        out += "</table>";
        out += "\n";
    }    

    return QString(out);
}

QString ReportDialog::htmlBoundaries()
{
    QString out;

    out  = "\n";
    for (int i = 1; i < Util::scene()->edgeMarkers.count(); i++)
    {
        SceneEdgeMarker *marker = Util::scene()->edgeMarkers[i];
        out += "<h5>" + marker->name + "</h5>";

        out += "<table>";
        QMap<QString, QString> data = marker->data();
        for (int j = 0; j < data.keys().length(); j++)
        {
            out += "<tr>";
            out += "<td>" + data.keys()[j] + "</td>";
            out += "<td>" + data.values()[j] + "</td>";
            out += "</tr>";
        }
        out += "</table>";
        out += "\n";
    }

    return QString(out);
}

QString ReportDialog::htmlGeometryNodes()
{
    QString out;

    out  = "\n";
    out += "<table>";
    out += "<tr>";
    out += "<th>" + Util::scene()->problemInfo()->labelX() + " (m)</th>";
    out += "<th>" + Util::scene()->problemInfo()->labelY() + " (m)</th>";
    out += "</tr>";
    for (int i = 0; i < Util::scene()->nodes.count(); i++)
    {
        out += "<tr>";
        out += "<td>" + QString::number(Util::scene()->nodes[i]->point.x, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->nodes[i]->point.y, 'e', 3) + "</td>";
        out += "</tr>";
    }
    out += "</table>";
    out += "\n";

    return QString(out);
}

QString ReportDialog::htmlGeometryEdges()
{
    QString out;

    out  = "\n";
    out += "<table>";
    out += "<tr>";
    out += "<th>Start node</th>";
    out += "<th>End node</th>";
    out += "<th>Angle (deg.)</th>";
    out += "<th>Marker</th>";
    out += "</tr>";
    for (int i = 0; i < Util::scene()->edges.count(); i++)
    {
        out += "<tr>";
        out += "<td>[" + QString::number(Util::scene()->edges[i]->nodeStart->point.x, 'e', 3) + "; " + QString::number(Util::scene()->edges[i]->nodeStart->point.y, 'e', 3) + "]</td>";
        out += "<td>[" + QString::number(Util::scene()->edges[i]->nodeEnd->point.x, 'e', 3) + "; " + QString::number(Util::scene()->edges[i]->nodeEnd->point.y, 'e', 3) + "]</td>";
        out += "<td>" + QString::number(Util::scene()->edges[i]->angle, 'f', 2) + "</td>";
        out += "<td>" + Util::scene()->edges[i]->marker->name + "</td>";
        out += "</tr>";
    }
    out += "</table>";
    out += "\n";

    return QString(out);
}

QString ReportDialog::htmlGeometryLabels()
{
    QString out;

    out  = "\n";
    out += "<table>";
    out += "<tr>";
    out += "<th>" + Util::scene()->problemInfo()->labelX() + " (m)</th>";
    out += "<th>" + Util::scene()->problemInfo()->labelY() + " (m)</th>";
    out += "<th>Array (m)</th>";
    out += "<th>Marker</th>";    out += "</tr>";
    for (int i = 0; i < Util::scene()->labels.count(); i++)
    {
        out += "<tr>";
        out += "<td>" + QString::number(Util::scene()->labels[i]->point.x, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->labels[i]->point.y, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->labels[i]->area, 'e', 3) + "</td>";
        out += "<td>" + Util::scene()->labels[i]->marker->name + "</td>";
        out += "</tr>";
    }
    out += "</table>";
    out += "\n";

    return QString(out);
}
