#include "reportdialog.h"

#include "scene.h"
#include "scripteditordialog.h"

ReportDialog::ReportDialog(SceneView *sceneView, QWidget *parent) : QDialog(parent)
{
    m_sceneView = sceneView;

    setWindowIcon(icon("browser"));
    setWindowTitle(tr("Report"));
    setWindowFlags(Qt::Window);
    setMinimumSize(400, 400);

    createControls();
}

ReportDialog::~ReportDialog()
{
    delete btnClose;
    delete btnOpenInExternalBrowser;
    delete btnPrint;
}

void ReportDialog::createControls()
{
    view = new QWebView(this);

    // dialog buttons
    btnOpenInExternalBrowser = new QPushButton(tr("Open in external viewer"));
    connect(btnOpenInExternalBrowser, SIGNAL(clicked()), this, SLOT(doOpenInExternalBrowser()));

    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    btnPrint = new QPushButton(tr("Print"));
    connect(btnPrint, SIGNAL(clicked()), this, SLOT(doPrint()));

    QHBoxLayout *layoutButtonFile = new QHBoxLayout();
    layoutButtonFile->addStretch();
    layoutButtonFile->addWidget(btnPrint);
    layoutButtonFile->addWidget(btnOpenInExternalBrowser);
    layoutButtonFile->addWidget(btnClose);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(view);
    layout->addStretch();
    layout->addLayout(layoutButtonFile);

    setLayout(layout);
}

void ReportDialog::doClose()
{
    hide();
}

void ReportDialog::doOpenInExternalBrowser()
{
    QDesktopServices::openUrl(tempProblemDir() + "/report/index.html");
}

void ReportDialog::doPrint()
{
    QPrintDialog printDialog(this);
    if (printDialog.exec() == QDialog::Accepted)
    {
        view->print(printDialog.printer());
    }
}

void ReportDialog::showDialog()
{
    QDir(tempProblemDir()).mkdir("report");

    QFile::copy(QString("%1/doc/report/template/template.html").arg(datadir()), tempProblemDir() + "/report/template.html");
    QFile::copy(QString("%1/doc/report/template/default.css").arg(datadir()), tempProblemDir() + "/report/default.css");

    generateFigures();
    generateIndex();

    view->load(QUrl::fromLocalFile(tempProblemDir() + "/report/index.html"));
    view->show();

    show();
    activateWindow();
    raise();
}

void ReportDialog::generateFigures()
{
    m_sceneView->saveImagesForReport(tempProblemDir() + "/report", 600, 400);
}

void ReportDialog::generateIndex()
{
    QString fileNameTemplate = tempProblemDir() + "/report/template.html";
    QString fileNameIndex = tempProblemDir() + "/report/index.html";

    QString content;

    // load template.html
    content = readFileContent(fileNameTemplate);
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
    destination.replace("[Problem.AdaptivitySteps]", (Util::scene()->problemInfo()->adaptivityType != ADAPTIVITYTYPE_NONE) ? QString::number(Util::scene()->problemInfo()->adaptivitySteps) : "", Qt::CaseSensitive);
    destination.replace("[Problem.AdaptivityTolerance]", (Util::scene()->problemInfo()->adaptivityType != ADAPTIVITYTYPE_NONE) ? QString::number(Util::scene()->problemInfo()->adaptivityTolerance) : "", Qt::CaseSensitive);

    destination.replace("[Problem.Frequency]", (Util::scene()->problemInfo()->hermes()->hasFrequency()) ? QString::number(Util::scene()->problemInfo()->frequency) : "", Qt::CaseSensitive);
    destination.replace("[Problem.AnalysisType]", analysisTypeString(Util::scene()->problemInfo()->analysisType), Qt::CaseSensitive);
    destination.replace("[Problem.TimeStep]", (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_TRANSIENT) ? QString::number(Util::scene()->problemInfo()->timeStep) : "", Qt::CaseSensitive);
    destination.replace("[Problem.TimeTotal]", (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_TRANSIENT) ? QString::number(Util::scene()->problemInfo()->timeTotal) : "", Qt::CaseSensitive);
    destination.replace("[Problem.InititalCondition]", (Util::scene()->problemInfo()->analysisType == ANALYSISTYPE_TRANSIENT) ? QString::number(Util::scene()->problemInfo()->initialCondition) : "", Qt::CaseSensitive);

    // script
    destination.replace("[Script]", createPythonFromModel(), Qt::CaseSensitive);

    // description
    destination.replace("[Description]", Util::scene()->problemInfo()->description, Qt::CaseSensitive);

    // physical properties
    destination.replace("[Materials]", htmlMaterials(), Qt::CaseSensitive);
    destination.replace("[Boundaries]", htmlBoundaries(), Qt::CaseSensitive);

    // geometry
    destination.replace("[Geometry.Nodes]", htmlGeometryNodes(), Qt::CaseSensitive);
    destination.replace("[Geometry.Edges]", htmlGeometryEdges(), Qt::CaseSensitive);
    destination.replace("[Geometry.Labels]", htmlGeometryLabels(), Qt::CaseSensitive);

    // solver
    destination.replace("[Solver.Nodes]", (Util::scene()->sceneSolution()->isMeshed()) ? QString::number(Util::scene()->sceneSolution()->mesh()->get_num_nodes()) : "", Qt::CaseSensitive);
    destination.replace("[Solver.Elements]", (Util::scene()->sceneSolution()->isMeshed()) ? QString::number(Util::scene()->sceneSolution()->mesh()->get_num_active_elements()) : "", Qt::CaseSensitive);
    destination.replace("[Solver.DOFs]", (Util::scene()->sceneSolution()->isSolved()) ? QString::number(Util::scene()->sceneSolution()->sln()->get_num_dofs()) : "", Qt::CaseSensitive);
    QTime time;
    if (Util::scene()->sceneSolution()->isSolved())
        time = milliSecondsToTime(Util::scene()->sceneSolution()->timeElapsed());
    destination.replace("[Solver.TimeElapsed]", (Util::scene()->sceneSolution()->isSolved()) ? time.toString("mm:ss.zzz") + " s" : "", Qt::CaseSensitive);
    destination.replace("[Solver.AdaptiveError]", (Util::scene()->sceneSolution()->isSolved() && Util::scene()->problemInfo()->adaptivityType != ADAPTIVITYTYPE_NONE) ? QString::number(Util::scene()->sceneSolution()->adaptiveError(), 'f', 3) : "", Qt::CaseSensitive);
    destination.replace("[Solver.AdaptiveSteps]", (Util::scene()->sceneSolution()->isSolved() && Util::scene()->problemInfo()->adaptivityType != ADAPTIVITYTYPE_NONE) ? QString::number(Util::scene()->sceneSolution()->adaptiveSteps()) : "", Qt::CaseSensitive);

    // figures    
    destination.replace("[Figure.Geometry]", htmlFigure("geometry.png", "Geometry"), Qt::CaseSensitive);
    destination.replace("[Figure.Mesh]", htmlFigure("mesh.png", "Mesh"), Qt::CaseSensitive);
    destination.replace("[Figure.ScalarView]", htmlFigure("scalarview.png", "ScalarView: " + physicFieldVariableString(Util::scene()->problemInfo()->hermes()->scalarPhysicFieldVariable())), Qt::CaseSensitive);
    destination.replace("[Figure.Order]", htmlFigure("order.png", "Polynomial order"), Qt::CaseSensitive);

    return destination;
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

    return out;
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

    return out;
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

    return out;
}

QString ReportDialog::htmlGeometryEdges()
{
    QString out;

    out  = "\n";
    out += "<table>";
    out += "<tr>";
    out += "<th colspan=\"2\">Start node</th>";
    out += "<th colspan=\"2\">End node</th>";
    out += "<th>Angle (deg.)</th>";
    out += "<th>Marker</th>";
    out += "</tr>";
    for (int i = 0; i < Util::scene()->edges.count(); i++)
    {
        out += "<tr>";
        out += "<td>" + QString::number(Util::scene()->edges[i]->nodeStart->point.x, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->edges[i]->nodeStart->point.y, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->edges[i]->nodeEnd->point.x, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->edges[i]->nodeEnd->point.y, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->edges[i]->angle, 'f', 2) + "</td>";
        out += "<td>" + Util::scene()->edges[i]->marker->name + "</td>";
        out += "</tr>";
    }
    out += "</table>";
    out += "\n";

    return out;
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

    return out;
}

QString ReportDialog::htmlFigure(const QString &fileName, const QString &caption)
{
    QString out;

    if (QFile::exists(tempProblemDir() + "/report/" + fileName))
    {
        out += "\n";
        out += QString("<div id=\"figure\"><img src=\"%1\" tag=\"Geometry\" /><div>Figure: %2</div></div>").
                arg(fileName).
                arg(caption);
        out += "\n";
    }

    return out;
}
