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
    setControls();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());

    QSettings settings;
    restoreGeometry(settings.value("ReportDialog/Geometry", saveGeometry()).toByteArray());
}

ReportDialog::~ReportDialog()
{
    QSettings settings;
    settings.setValue("ReportDialog/Geometry", saveGeometry());
}

void ReportDialog::createControls()
{
    chkDescription = new QCheckBox(tr("Description"));
    chkProblemInformation = new QCheckBox(tr("Problem information"));
    chkStartupScript = new QCheckBox(tr("Startup script"));
    chkPhysicalProperties = new QCheckBox(tr("Physical properties"));
    chkGeometry = new QCheckBox(tr("Geometry"));
    connect(chkGeometry, SIGNAL(clicked()), this, SLOT(resetControls()));
    chkMeshAndSolution = new QCheckBox(tr("Mesh and solution"));
    connect(chkMeshAndSolution, SIGNAL(clicked()), this, SLOT(resetControls()));
    chkScript = new QCheckBox(tr("Script"));

    chkFigureGeometry = new QCheckBox(tr("Geometry"));
    chkFigureMesh = new QCheckBox(tr("Mesh"));
    chkFigureOrder = new QCheckBox(tr("Order"));
    chkFigureScalarView = new QCheckBox(tr("Scalar view"));
    chkShowGrid = new QCheckBox(tr("Show grid in figures"));
    chkShowRulers = new QCheckBox(tr("Show rulers in figures"));

    txtTemplate = new QLineEdit();
    connect(txtTemplate, SIGNAL(textChanged(QString)), this, SLOT(checkPaths()));
    txtStyleSheet = new QLineEdit();
    connect(txtStyleSheet, SIGNAL(textChanged(QString)), this, SLOT(checkPaths()));

    txtFigureWidth = new SLineEditValue();
    txtFigureWidth->setValue(Value("600"));
    txtFigureWidth->setMinimum(200);
    txtFigureHeight = new SLineEditValue();
    txtFigureHeight->setValue(Value("400"));
    txtFigureHeight->setMinimum(200);

    btnDefault = new QPushButton(tr("Default"));
    connect(btnDefault, SIGNAL(clicked()), this, SLOT(defaultValues()));

    btnShowReport = new QPushButton(tr("Show report"));
    connect(btnShowReport, SIGNAL(clicked()), this, SLOT(doShowReport()));

    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QVBoxLayout *layoutSections = new QVBoxLayout();
    layoutSections->addWidget(new QLabel(tr("Sections")));
    layoutSections->addWidget(chkDescription);
    layoutSections->addWidget(chkProblemInformation);
    layoutSections->addWidget(chkStartupScript);
    layoutSections->addWidget(chkPhysicalProperties);
    layoutSections->addWidget(chkGeometry);
    layoutSections->addWidget(chkMeshAndSolution);
    layoutSections->addWidget(chkScript);

    QVBoxLayout *layoutFigure = new QVBoxLayout();
    layoutFigure->addWidget(new QLabel(tr("Figures")));
    layoutFigure->addWidget(chkFigureGeometry);
    layoutFigure->addWidget(chkFigureMesh);
    layoutFigure->addWidget(chkFigureOrder);
    layoutFigure->addWidget(chkFigureScalarView);
    layoutFigure->addStretch();

    QGridLayout *layoutBasicProperties = new QGridLayout();
    layoutBasicProperties->addLayout(layoutSections, 0, 0);
    layoutBasicProperties->addLayout(layoutFigure, 0, 1);

    QGridLayout *layoutTemplate = new QGridLayout();
    layoutTemplate->addWidget(new QLabel(tr("Template")), 0, 0);
    layoutTemplate->addWidget(txtTemplate, 0, 1);
    layoutTemplate->addWidget(new QLabel(tr("Style sheet")), 1, 0);
    layoutTemplate->addWidget(txtStyleSheet, 1, 1);

    QGridLayout *layoutFigureSize = new QGridLayout();
    layoutFigureSize->addWidget(new QLabel(tr("Width")), 0, 0);
    layoutFigureSize->addWidget(txtFigureWidth, 0, 1);
    layoutFigureSize->addWidget(new QLabel(tr("Height")), 1, 0);
    layoutFigureSize->addWidget(txtFigureHeight, 1, 1);
    layoutFigureSize->addWidget(chkShowGrid, 2, 0, 1, 2);
    layoutFigureSize->addWidget(chkShowRulers, 3, 0, 1, 2);

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnShowReport);
    layoutButtons->addWidget(btnDefault);
    layoutButtons->addWidget(btnClose);

    QGroupBox *grpBasicProperties = new QGroupBox(tr("Basic properties"));
    grpBasicProperties->setLayout(layoutBasicProperties);

    QGroupBox *grpAdditionalProperties = new QGroupBox(tr("Additional properties"));
    grpAdditionalProperties->setLayout(layoutTemplate);

    QGroupBox *grpFigure = new QGroupBox(tr("Figures properties"));
    grpFigure->setLayout(layoutFigureSize);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpBasicProperties);
    layout->addWidget(grpFigure);
    layout->addWidget(grpAdditionalProperties);
    layout->addStretch();
    layout->addLayout(layoutButtons);

    setLayout(layout);
}

bool ReportDialog::checkPaths()
{
    bool templateExists = QFile::exists(QString(txtTemplate->text()));
    bool styleSheetExists = QFile::exists(QString(txtStyleSheet->text()));

    QPalette paletteTemplate = txtTemplate->palette();
    if (templateExists)
        paletteTemplate.setColor(QPalette::Text, QPalette().color(QPalette::Text));
    else
        paletteTemplate.setColor(QPalette::Text, QColor(Qt::red));
    txtTemplate->setPalette(paletteTemplate);

    QPalette paletteStyleSheet = txtStyleSheet->palette();
    if (styleSheetExists)
        paletteStyleSheet.setColor(QPalette::Text, QPalette().color(QPalette::Text));
    else
        paletteStyleSheet.setColor(QPalette::Text, QColor(Qt::red));
    txtStyleSheet->setPalette(paletteStyleSheet);

    btnShowReport->setEnabled(templateExists && styleSheetExists);
}

void ReportDialog::defaultValues()
{
    chkProblemInformation->setChecked(true);
    chkPhysicalProperties->setChecked(true);
    chkGeometry->setChecked(true);
    chkScript->setChecked(true);

    chkFigureGeometry->setChecked(true);
    chkShowGrid->setChecked(true);
    chkShowRulers->setChecked(true);

    txtFigureWidth->setValue(Value("600"));
    txtFigureHeight->setValue(Value("400"));

    txtTemplate->setText(QString("%1/doc/report/default.html").arg(datadir()));
    txtStyleSheet->setText(QString("%1/doc/report/default.css").arg(datadir()));

    checkPaths();
}

void ReportDialog::setControls()
{
    chkDescription->setDisabled(Util::scene()->problemInfo()->description.isEmpty());
    chkDescription->setChecked(!Util::scene()->problemInfo()->description.isEmpty());
    chkStartupScript->setDisabled(Util::scene()->problemInfo()->scriptStartup.isEmpty());
    chkStartupScript->setChecked(!Util::scene()->problemInfo()->scriptStartup.isEmpty());

    chkMeshAndSolution->setDisabled(!Util::scene()->sceneSolution()->isSolved());
    chkMeshAndSolution->setChecked(Util::scene()->sceneSolution()->isSolved());
    chkFigureMesh->setDisabled(!Util::scene()->sceneSolution()->isSolved());
    chkFigureMesh->setChecked(Util::scene()->sceneSolution()->isSolved());
    chkFigureOrder->setDisabled(!Util::scene()->sceneSolution()->isSolved());
    chkFigureOrder->setChecked(Util::scene()->sceneSolution()->isSolved());
    chkFigureScalarView->setDisabled(!Util::scene()->sceneSolution()->isSolved());
    chkFigureScalarView->setChecked(Util::scene()->sceneSolution()->isSolved());
}

void ReportDialog::resetControls()
{
    chkFigureGeometry->setChecked(chkGeometry->isChecked());
    chkFigureGeometry->setEnabled(chkGeometry->isChecked());
    chkFigureMesh->setChecked(chkMeshAndSolution->isChecked());
    chkFigureMesh->setEnabled(chkMeshAndSolution->isChecked());
    chkFigureOrder->setChecked(chkMeshAndSolution->isChecked());
    chkFigureOrder->setEnabled(chkMeshAndSolution->isChecked());
    chkFigureScalarView->setChecked(chkMeshAndSolution->isChecked());
    chkFigureScalarView->setEnabled(chkMeshAndSolution->isChecked());

    chkShowGrid->setChecked(chkGeometry->isChecked() || chkMeshAndSolution->isChecked());
    chkShowGrid->setEnabled(chkGeometry->isChecked() || chkMeshAndSolution->isChecked());
    chkShowRulers->setChecked(chkGeometry->isChecked() || chkMeshAndSolution->isChecked());
    chkShowRulers->setEnabled(chkGeometry->isChecked() || chkMeshAndSolution->isChecked());
}

void ReportDialog::showDialog()
{
    setControls();
    show();
    activateWindow();
    raise();
}

void ReportDialog::doClose()
{
    hide();
}

void ReportDialog::doShowReport()
{
    QDir(tempProblemDir()).mkdir("report");

    QFile::remove(QString("%1/report/template.html").arg(tempProblemDir()));
    QFile::remove(QString("%1/report/style.css").arg(tempProblemDir()));
    bool fileTemplateOK = QFile::copy(QString(txtTemplate->text()),
                                      QString("%1/report/template.html").arg(tempProblemDir()));
    bool fileStyleOK = QFile::copy(QString(txtStyleSheet->text()),
                                      QString("%1/report/style.css").arg(tempProblemDir()));
    if (!fileTemplateOK)
        QMessageBox::critical(QApplication::activeWindow(), tr("Error"), tr("Report template could not be copied."));
    else if (!fileStyleOK)
        QMessageBox::critical(QApplication::activeWindow(), tr("Error"), tr("Template style could not be copied."));
    else
    {
        generateFigures();
        generateIndex();

        QDesktopServices::openUrl(tempProblemDir() + "/report/report.html");
    }
}

void ReportDialog::generateIndex()
{
    QString fileNameTemplate = tempProblemDir() + "/report/template.html";
    QString fileNameIndex = tempProblemDir() + "/report/report.html";

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

void ReportDialog::generateFigures()
{
    bool showRulers = chkShowRulers->isChecked();
    bool showGrid = chkShowGrid->isChecked();
    m_sceneView->saveImagesForReport(tempProblemDir() + "/report",
                                     showRulers,
                                     showGrid,
                                     txtFigureWidth->value().number,
                                     txtFigureHeight->value().number);
}

QString ReportDialog::replaceTemplates(const QString &source)
{
    QString destination = source;

    // stylesheet
    destination.replace("[StyleSheet]", "./style.css", Qt::CaseSensitive);

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
        destination.replace("[ProblemInformation.FileName]", "<tr><td>Filename:</td><td>" + QFileInfo(Util::scene()->problemInfo()->fileName).fileName() + "</td></tr>", Qt::CaseSensitive);
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
        QString tag [16] = {"[ProblemInformation.Label]", "[ProblemInformation.Name]",
                           "[ProblemInformation.Date]", "[ProblemInformation.FileName]",
                           "[ProblemInformation.ProblemType]", "[ProblemInformation.PhysicField]",
                           "[ProblemInformation.AnalysisType]", "[ProblemInformation.NumberOfRefinements]",
                           "[ProblemInformation.PolynomialOrder]", "[ProblemInformation.AdaptivityType]",
                           "[ProblemInformation.AdaptivitySteps]", "[ProblemInformation.AdaptivityTolerance]",
                           "[ProblemInformation.Frequency]", "[ProblemInformation.TimeStep]",
                           "[ProblemInformation.TimeTotal]", "[ProblemInformation.InititalCondition]"};

        for (int i = 0; i < 16; i++)
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
        destination.replace("[Script.Label]", "<h2>Script</h2>", Qt::CaseSensitive);
        destination.replace("[Script]", "<pre>" + createPythonFromModel() + "</pre>", Qt::CaseSensitive);
    }
    else
    {
        destination.remove("[Script.Label]", Qt::CaseSensitive);
        destination.remove("[Script]", Qt::CaseSensitive);
    }

    // footer
    destination.replace("[Report.Footer]", "<p id=\"footer\">Computed by Agros2D (<a href=\"http://hpfem.org/agros2d\">http://hpfem.org/agros2d</a>)</p>");

    // figures
    if (chkFigureGeometry->isChecked())
        destination.replace("[Figure.Geometry]", htmlFigure("geometry.png", "Geometry"), Qt::CaseSensitive);
    else
        destination.remove("[Figure.Geometry]", Qt::CaseSensitive);

    if (chkFigureMesh->isChecked())
        destination.replace("[Figure.Mesh]", htmlFigure("mesh.png", "Mesh"), Qt::CaseSensitive);
    else
        destination.remove("[Figure.Mesh]", Qt::CaseSensitive);

    if (chkFigureOrder->isChecked())
            destination.replace("[Figure.Order]", htmlFigure("order.png", "Polynomial order"), Qt::CaseSensitive);
    else
        destination.remove("[Figure.Order]", Qt::CaseSensitive);

    if (chkFigureScalarView->isChecked())
        destination.replace("[Figure.ScalarView]", htmlFigure("scalarview.png", "ScalarView: " + physicFieldVariableString(Util::scene()->problemInfo()->hermes()->scalarPhysicFieldVariable())), Qt::CaseSensitive);
    else
        destination.remove("[Figure.ScalarView]", Qt::CaseSensitive);

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
        out += QString("<img src=\"%1\"><p>Figure: %2</p>").
                arg(fileName).
                arg(caption);
        out += "\n";
    }

    return out;
}
