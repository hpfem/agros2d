// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "reportdialog.h"
#include "scene.h"
#include "scripteditordialog.h"

ReportDialog::ReportDialog(SceneView *sceneView, QWidget *parent) : QDialog(parent)
{
    m_sceneView = sceneView;

    setWindowIcon(icon("browser"));
    setWindowTitle(tr("Report"));
    setWindowFlags(Qt::Window);

    createControls();
    defaultValues();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
    QSettings settings;
    restoreGeometry(settings.value("ReportDialog/Geometry", saveGeometry()).toByteArray());
}

ReportDialog::~ReportDialog()
{
    QSettings settings;
    settings.setValue("ReportDialog/Geometry", saveGeometry());

    delete btnShowReport;
    //delete btnPrint;
    delete btnClose;
}

void ReportDialog::createControls()
{
    chkDescription = new QCheckBox(tr("Description"));
    chkProblemInformation = new QCheckBox(tr("Problem information"));
    chkStartupScript = new QCheckBox(tr("Startup script"));
    chkPhysicalProperties = new QCheckBox(tr("Physical properties"));
    chkGeometry = new QCheckBox(tr("Geometry"));
    chkMeshAndSolution = new QCheckBox(tr("Mesh and solution"));
    chkScript = new QCheckBox(tr("Script"));

    QVBoxLayout *layoutBasicProperties = new QVBoxLayout();
    layoutBasicProperties->addWidget(new QLabel(tr("Basic properties")));
    layoutBasicProperties->addWidget(chkDescription);
    layoutBasicProperties->addWidget(chkProblemInformation);
    layoutBasicProperties->addWidget(chkStartupScript);
    layoutBasicProperties->addWidget(chkPhysicalProperties);
    layoutBasicProperties->addWidget(chkGeometry);
    layoutBasicProperties->addWidget(chkMeshAndSolution);
    layoutBasicProperties->addWidget(chkScript);

    btnShowReport = new QPushButton(tr("Show report"));
    connect(btnShowReport, SIGNAL(clicked()), this, SLOT(doShowReport()));

    //btnPrint = new QPushButton(tr("Print"));
    //connect(btnPrint, SIGNAL(clicked()), this, SLOT(doPrint()));

    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnShowReport);
    //layoutButtonFile->addWidget(btnPrint);
    layoutButtons->addWidget(btnClose);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutBasicProperties);
    layout->addLayout(layoutButtons);

    setLayout(layout);
}

void ReportDialog::defaultValues()
{
    chkProblemInformation->setChecked(true);
    chkPhysicalProperties->setChecked(true);
    chkGeometry->setChecked(true);
    chkScript->setChecked(true);
}

void ReportDialog::load()
{
    if (Util::scene()->problemInfo()->description.isEmpty())
    {
        chkDescription->setDisabled(true);
        chkDescription->setChecked(false);
    }
    else
    {
        chkDescription->setDisabled(false);
        chkDescription->setChecked(true);
    }

    if (Util::scene()->problemInfo()->scriptStartup.isEmpty())
    {
        chkStartupScript->setDisabled(true);
        chkStartupScript->setChecked(false);
    }
    else
    {
        chkStartupScript->setDisabled(false);
        chkStartupScript->setChecked(true);
    }

    if (Util::scene()->sceneSolution()->isSolved())
    {
        chkMeshAndSolution->setDisabled(false);
        chkMeshAndSolution->setChecked(true);
    }
    else
    {
        chkMeshAndSolution->setDisabled(true);
        chkMeshAndSolution->setChecked(false);
    }
}

void ReportDialog::doClose()
{
    hide();
}

void ReportDialog::doShowReport()
{
    generateFigures();
    generateIndex();

    QDesktopServices::openUrl(tempProblemDir() + "/report/index.html");
}

//void ReportDialog::doPrint()
//{
//    QPrintDialog printDialog(this);
//    if (printDialog.exec() == QDialog::Accepted)
//    {
//        view->print(printDialog.printer());
//    }
//}

void ReportDialog::showDialog()
{
    QDir(tempProblemDir()).mkdir("report");

    QFile::remove(QString("%1/report/template.html").arg(tempProblemDir()));
    QFile::remove(QString("%1/report/default.css").arg(tempProblemDir()));
    bool fileTemplateOK = QFile::copy(QString("%1/doc/report/template/template.html").arg(datadir()),
                                      QString("%1/report/template.html").arg(tempProblemDir()));
    bool fileStyleOK = QFile::copy(QString("%1/doc/report/template/default.css").arg(datadir()),
                                      QString("%1/report/default.css").arg(tempProblemDir()));
    if (!fileTemplateOK || !fileStyleOK)
        QMessageBox::warning(QApplication::activeWindow(), tr("Error"), tr("Report template could not be copied."));

    load();

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
    if (fileIndex.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&fileIndex);
        stream.setCodec("UTF-8");

        stream << replaceTemplates(content);

        fileIndex.flush();
        fileIndex.close();
    }
}

QString ReportDialog::replaceTemplates(const QString &source)
{
    QString destination = source;

    // problem name
    destination.replace("[Report.Label]", "<h1>" + Util::scene()->problemInfo()->name + "</h1>", Qt::CaseSensitive);

    // description
    if (chkDescription->isChecked())
    {
        destination.replace("[Description.Label]", "<h2>Description</h2>", Qt::CaseSensitive);
        destination.replace("[Description]", "<p>" + Util::scene()->problemInfo()->description + "</p>", Qt::CaseSensitive);
    }
    else
    {
        destination.remove("[Description.Label]", Qt::CaseSensitive);
        destination.remove("[Description]", Qt::CaseSensitive);
    }

    // problem information
    if (chkProblemInformation->isChecked())
    {
        destination.replace("[ProblemInformation.Label]", "<h2>Problem Information</h2>", Qt::CaseSensitive);
        destination.replace("[ProblemInformation.Name]", "<table><tr><td>Name:</td><td>" + Util::scene()->problemInfo()->name + "</td></tr>", Qt::CaseSensitive);
        destination.replace("[ProblemInformation.Date]", "<tr><td>Date:</td><td>" + Util::scene()->problemInfo()->date.toString("dd.MM.yyyy") + "</td></tr>", Qt::CaseSensitive);
        destination.replace("[ProblemInformation.FileName]", "<tr><td>Filename:</td><td>" + QFileInfo(Util::scene()->problemInfo()->fileName).completeBaseName() + "</td></tr>", Qt::CaseSensitive);
        destination.replace("[ProblemInformation.ProblemType]", "<tr><td>Problem type:</td><td>" + problemTypeString(Util::scene()->problemInfo()->problemType) + "</td></tr>", Qt::CaseSensitive);
        destination.replace("[ProblemInformation.PhysicField]", "<tr><td>Physic field:</td><td>" + physicFieldString(Util::scene()->problemInfo()->physicField()) + "</td></tr>", Qt::CaseSensitive);
        destination.replace("[ProblemInformation.AnalysisType]", "<tr><td>Analysis type:</td><td>" + analysisTypeString(Util::scene()->problemInfo()->analysisType) + "</td></tr>", Qt::CaseSensitive);
        destination.replace("[ProblemInformation.NumberOfRefinements]", "<tr><td>Number of refinements:</td><td>" + QString::number(Util::scene()->problemInfo()->numberOfRefinements) + "</td></tr>", Qt::CaseSensitive);
        destination.replace("[ProblemInformation.PolynomialOrder]", "<tr><td>Polynomial order:</td><td>" + QString::number(Util::scene()->problemInfo()->polynomialOrder) + "</td></tr></table>", Qt::CaseSensitive);

        if (Util::scene()->problemInfo()->adaptivityType != AdaptivityType_None && chkProblemInformation->isChecked())
        {
            destination.replace("[ProblemInformation.AdaptivityType]", "<table><tr><td>Adaptivity type:</td><td>" + adaptivityTypeString(Util::scene()->problemInfo()->adaptivityType) + "</td></tr>", Qt::CaseSensitive);
            destination.replace("[ProblemInformation.AdaptivitySteps]", "<tr><td>Adaptivity steps:</td><td>" + QString::number(Util::scene()->problemInfo()->adaptivitySteps) + "</td></tr>", Qt::CaseSensitive);
            destination.replace("[ProblemInformation.AdaptivityTolerance]", "<tr><td>Adaptivity tolerance:</td><td>" + QString::number(Util::scene()->problemInfo()->adaptivityTolerance) + "</td></tr></table>", Qt::CaseSensitive);
        }
        else
        {
            // remove empty tags
            QString tag [3] = {"[ProblemInformation.AdaptivityType]", "[ProblemInformation.AdaptivitySteps]",
                               "[ProblemInformation.AdaptivityTolerance]"};

            for (int i = 0; i < 3; i++)
            {
                destination.remove(tag[i].toUtf8(), Qt::CaseSensitive);
            }
        }

        if ((Util::scene()->problemInfo()->hermes()->hasHarmonic() || Util::scene()->problemInfo()->analysisType == AnalysisType_Transient) && chkProblemInformation->isChecked())
        {
            destination.replace("[ProblemInformation.Frequency]", "<table><tr><td>Adaptivity type:</td><td>" + QString::number(Util::scene()->problemInfo()->frequency) + "</td></tr>", Qt::CaseSensitive);
            destination.replace("[ProblemInformation.TimeStep]", "<tr><td>Adaptivity type:</td><td>" + QString::number(Util::scene()->problemInfo()->timeStep.number) + "</td></tr>", Qt::CaseSensitive);
            destination.replace("[ProblemInformation.TimeTotal]", "<tr><td>Adaptivity type:</td><td>" + QString::number(Util::scene()->problemInfo()->timeTotal.number) + "</td></tr>", Qt::CaseSensitive);
            destination.replace("[ProblemInformation.InititalCondition]", "<tr><td>Adaptivity type:</td><td>" + QString::number(Util::scene()->problemInfo()->initialCondition.number) + "</td></tr></table>", Qt::CaseSensitive);
        }
        else
        {
            // remove empty tags
            QString tag [4] = {"[ProblemInformation.Frequency]", "[ProblemInformation.TimeStep]",
                               "[ProblemInformation.TimeTotal]", "[ProblemInformation.InititalCondition]",};

            for (int i = 0; i < 4; i++)
            {
                destination.remove(tag[i].toUtf8(), Qt::CaseSensitive);
            }
        }
    }
    else
    {
        // remove empty tags
        QString tag [9] = {"[ProblemInformation.Label]", "[ProblemInformation.Name]",
                           "[ProblemInformation.Date]", "[ProblemInformation.Filename]",
                           "[ProblemInformation.ProblemType]", "[ProblemInformation.PhysicField]",
                           "[ProblemInformation.AnalysisType]", "[ProblemInformation.NumberOfRefinements]",
                           "[ProblemInformation.PolynomialOrder]"};

        for (int i = 0; i < 9; i++)
        {
            destination.remove(tag[i].toUtf8(), Qt::CaseSensitive);
        }
    }

    // startup script
    if (chkStartupScript->isChecked())
    {
        destination.replace("[StartupScript.Label]", "<h2>Startup Script</h2>", Qt::CaseSensitive);
        destination.replace("[StartupScript]", "<pre>" + Util::scene()->problemInfo()->scriptStartup + "</pre>", Qt::CaseSensitive);
    }
    else
    {
        destination.remove("[StartupScript.Label]", Qt::CaseSensitive);
        destination.remove("[StartupScript]", Qt::CaseSensitive);
    }

    // physical properties
    if (chkPhysicalProperties->isChecked())
    {
        destination.replace("[PhysicalProperties.Label]", "<h2>Physical Properties</h2>", Qt::CaseSensitive);
        destination.replace("[Materials.Label]", "<h3>Materials</h3>", Qt::CaseSensitive);
        destination.replace("[Materials]", htmlMaterials(), Qt::CaseSensitive);
        destination.replace("[Boundaries.Label]", "<h3>Boundaris conditions</h3>", Qt::CaseSensitive);
        destination.replace("[Boundaries]", htmlBoundaries(), Qt::CaseSensitive);
    }
    else
    {
        // remove empty tags
        QString tag [5] = {"[PhysicalProperties.Label]", "[Materials.Label]",
                           "[Materials]", "[Boundaries.Label]", "[Boundaries]"};

        for (int i = 0; i < 5; i++)
        {
            destination.remove(tag[i].toUtf8(), Qt::CaseSensitive);
        }
    }

    // geometry
    if (chkGeometry->isChecked())
    {
        destination.replace("[Geometry.Label]", "<h2>Geometry</h2>", Qt::CaseSensitive);
        destination.replace("[Geometry.Nodes.Label]", "<h3>Nodes</h3>", Qt::CaseSensitive);
        destination.replace("[Geometry.Nodes]", htmlGeometryNodes(), Qt::CaseSensitive);
        destination.replace("[Geometry.Edges.Label]", "<h3>Edges</h3>", Qt::CaseSensitive);
        destination.replace("[Geometry.Edges]", htmlGeometryEdges(), Qt::CaseSensitive);
        destination.replace("[Geometry.Labels.Label]", "<h3>Labels</h3>", Qt::CaseSensitive);
        destination.replace("[Geometry.Labels]", htmlGeometryLabels(), Qt::CaseSensitive);
    }
    else
    {
        // remove empty tags
        QString tag [7] = {"[Geometry.Label]", "[Geometry.Nodes.Label]",
                           "[Geometry.Nodes]", "[Geometry.Edges.Label]",
                           "[Geometry.Edges]", "[Geometry.Labels.Label]", "[Geometry.Labels]"};

        for (int i = 0; i < 7; i++)
        {
            destination.remove(tag[i].toUtf8(), Qt::CaseSensitive);
        }
    }

    // solver
    if (chkMeshAndSolution->isChecked())
    {
        QTime time;
        time = milisecondsToTime(Util::scene()->sceneSolution()->timeElapsed());

        destination.replace("[MeshAndSolver.Label]", "<h2>Mesh and Solution</h2>", Qt::CaseSensitive);
        destination.replace("[Solver.Label]", "<h3>Solver information</h3>", Qt::CaseSensitive);
        destination.replace("[Solver.Nodes]", "<table><tr><td>Nodes:</td><td>" + QString::number(Util::scene()->sceneSolution()->meshInitial()->get_num_nodes()) + "</td></tr>", Qt::CaseSensitive);
        destination.replace("[Solver.Elements]", "<tr><td>Elements:</td><td>" + QString::number(Util::scene()->sceneSolution()->meshInitial()->get_num_active_elements()) + "</td></tr>", Qt::CaseSensitive);
        destination.replace("[Solver.DOFs]", "<tr><td>DOFs:</td><td>" + QString::number(Util::scene()->sceneSolution()->sln()->get_num_dofs()) + "</td></tr>", Qt::CaseSensitive);
        destination.replace("[Solver.TimeElapsed]", "<tr><td>Elapsed time:</td><td>" + time.toString("mm:ss.zzz") + " s</td></tr></table>", Qt::CaseSensitive);

        if (Util::scene()->problemInfo()->adaptivityType != AdaptivityType_None)
        {
            destination.replace("[Solver.AdaptiveError]", "<table><tr><td>Adaptive error:</td><td>" + QString::number(Util::scene()->sceneSolution()->adaptiveError(), 'f', 3)  + "</td></tr>", Qt::CaseSensitive);
            destination.replace("[Solver.AdaptiveSteps]", "<tr><td>Adaptive steps:</td><td>" + QString::number(Util::scene()->sceneSolution()->adaptiveSteps()) + "</td></tr></table>", Qt::CaseSensitive);
        }
        else
        {


            // remove empty tags
            QString tag [2] = {"[Solver.AdaptiveError]", "[Solver.AdaptiveSteps]"};

            for (int i = 0; i < 2; i++)
            {
                destination.remove(tag[i].toUtf8(), Qt::CaseSensitive);
            }
        }
    }
    else
    {
        // remove empty tags
        QString tag [8] = {"[MeshAndSolver.Label]", "[Solver.Label]",
                           "[Solver.Nodes]", "[Solver.Elements]",
                           "[Solver.DOFs]", "[Solver.TimeElapsed]",
                           "[Solver.AdaptiveError]", "[Solver.AdaptiveSteps]"};

        for (int i = 0; i < 8; i++)
        {
            destination.remove(tag[i].toUtf8(), Qt::CaseSensitive);
        }
    }

    // script
    if (chkScript->isChecked())
    {
        destination.replace("[Script.Label]", "<h3>Script</h3>", Qt::CaseSensitive);
        destination.replace("[Script]", "<pre>" + createPythonFromModel() + "</pre>", Qt::CaseSensitive);
    }
    else
    {
        destination.remove("[Script.Label]", Qt::CaseSensitive);
        destination.remove("[Script]", Qt::CaseSensitive);
    }

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
        out += "<h4>" + marker->name + "</h4>";

        out += "<table>";
        QMap<QString, QString> data = marker->data();
        for (int j = 0; j < data.keys().length(); j++)
        {
            out += "<tr><td>" + data.keys()[j] + "</td>";
            out += "<td>" + data.values()[j] + "</td></tr>";
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
        out += "<h4>" + marker->name + "</h4>";

        out += "<table>";
        QMap<QString, QString> data = marker->data();
        for (int j = 0; j < data.keys().length(); j++)
        {
            out += "<tr><td>" + data.keys()[j] + "</td>";
            out += "<td>" + data.values()[j] + "</td></tr>";
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
    out += "<table><tr>";
    out += "<th>" + Util::scene()->problemInfo()->labelX() + " (m)</th>";
    out += "<th>" + Util::scene()->problemInfo()->labelY() + " (m)</th></tr>";
    for (int i = 0; i < Util::scene()->nodes.count(); i++)
    {
        out += "<tr><td>" + QString::number(Util::scene()->nodes[i]->point.x, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->nodes[i]->point.y, 'e', 3) + "</tr>";
    }
    out += "</table>";
    out += "\n";

    return out;
}

QString ReportDialog::htmlGeometryEdges()
{
    QString out;

    out  = "\n";
    out += "<table><tr>";
    out += "<th colspan=\"2\">Start node</th>";
    out += "<th colspan=\"2\">End node</th>";
    out += "<th>Angle (deg.)</th>";
    out += "<th>Marker</th></tr>";
    for (int i = 0; i < Util::scene()->edges.count(); i++)
    {
        out += "<tr><td>" + QString::number(Util::scene()->edges[i]->nodeStart->point.x, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->edges[i]->nodeStart->point.y, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->edges[i]->nodeEnd->point.x, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->edges[i]->nodeEnd->point.y, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->edges[i]->angle, 'f', 2) + "</td>";
        out += "<td>" + Util::scene()->edges[i]->marker->name + "</td></tr>";
    }
    out += "</table>";
    out += "\n";

    return out;
}

QString ReportDialog::htmlGeometryLabels()
{
    QString out;

    out  = "\n";
    out += "<table><tr>";
    out += "<th>" + Util::scene()->problemInfo()->labelX() + " (m)</th>";
    out += "<th>" + Util::scene()->problemInfo()->labelY() + " (m)</th>";
    out += "<th>Array (m)</th>";
    out += "<th>Marker</th></tr>";
    for (int i = 0; i < Util::scene()->labels.count(); i++)
    {
        out += "<tr><td>" + QString::number(Util::scene()->labels[i]->point.x, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->labels[i]->point.y, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->labels[i]->area, 'e', 3) + "</td>";
        out += "<td>" + Util::scene()->labels[i]->marker->name + "</td></tr>";
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
