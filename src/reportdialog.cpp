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
#include "gui.h"
#include "scene.h"
#include "scenebasic.h"
#include "scenesolution.h"
#include "sceneview.h"
#include "scripteditordialog.h"
#include "hermes2d/module.h"
#include "ctemplate/template.h"

ReportDialog::ReportDialog(SceneView *sceneView, QWidget *parent) : QDialog(parent)
{
    logMessage("ReportDialog::ReportDialog()");

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
    logMessage("ReportDialog::~ReportDialog()");

    QSettings settings;
    settings.setValue("ReportDialog/Geometry", saveGeometry());
}

void ReportDialog::createControls()
{
    logMessage("ReportDialog::createControls()");

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
    chkFigureContourView = new QCheckBox(tr("Contour view"));
    chkFigureVectorView = new QCheckBox(tr("Vector view"));

    txtFigureWidth = new ValueLineEdit();
    txtFigureWidth->setValue(Value("600"));
    txtFigureWidth->setMinimum(200);
    txtFigureHeight = new ValueLineEdit();
    txtFigureHeight->setValue(Value("400"));
    txtFigureHeight->setMinimum(200);
    chkFigureShowGrid = new QCheckBox(tr("Show grid"));
    chkFigureShowRulers = new QCheckBox(tr("Show rulers"));
    chkFigureShowAxes = new QCheckBox(tr("Show axes"));
    chkFigureShowLabel = new QCheckBox(tr("Show label"));

    txtTemplate = new QLineEdit();
    connect(txtTemplate, SIGNAL(textChanged(QString)), this, SLOT(checkPaths()));
    txtStyleSheet = new QLineEdit();
    connect(txtStyleSheet, SIGNAL(textChanged(QString)), this, SLOT(checkPaths()));

    btnDefault = new QPushButton(tr("Default"));
    connect(btnDefault, SIGNAL(clicked()), this, SLOT(defaultValues()));
    btnShowReport = new QPushButton(tr("Show report"));
    connect(btnShowReport, SIGNAL(clicked()), this, SLOT(doShowReport()));
    btnClose = new QPushButton(tr("Close"));
    btnClose->setDefault(true);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QVBoxLayout *layoutSections = new QVBoxLayout();
    layoutSections->addWidget(chkDescription);
    layoutSections->addWidget(chkProblemInformation);
    layoutSections->addWidget(chkStartupScript);
    layoutSections->addWidget(chkPhysicalProperties);
    layoutSections->addWidget(chkGeometry);
    layoutSections->addWidget(chkMeshAndSolution);
    layoutSections->addWidget(chkScript);

    QVBoxLayout *layoutFigure = new QVBoxLayout();
    layoutFigure->addWidget(chkFigureGeometry);
    layoutFigure->addWidget(chkFigureMesh);
    layoutFigure->addWidget(chkFigureOrder);
    layoutFigure->addWidget(chkFigureScalarView);
    layoutFigure->addWidget(chkFigureContourView);
    layoutFigure->addWidget(chkFigureVectorView);

    QGridLayout *layoutFigureProperties = new QGridLayout();
    layoutFigureProperties->addWidget(new QLabel(tr("Width")), 0, 0);
    layoutFigureProperties->addWidget(txtFigureWidth, 0, 1);
    layoutFigureProperties->addWidget(new QLabel(tr("Height")), 1, 0);
    layoutFigureProperties->addWidget(txtFigureHeight, 1, 1, 1, 2);
    layoutFigureProperties->addWidget(chkFigureShowGrid, 2, 0, 1, 2);
    layoutFigureProperties->addWidget(chkFigureShowRulers, 3, 0, 1, 2);
    layoutFigureProperties->addWidget(chkFigureShowAxes, 4, 0, 1, 2);
    layoutFigureProperties->addWidget(chkFigureShowLabel, 5, 0, 1, 2);

    QHBoxLayout *layoutFigures = new QHBoxLayout();
    layoutFigures->addLayout(layoutFigure);
    layoutFigures->addLayout(layoutFigureProperties);
    layoutFigures->addStretch();

    QGridLayout *layoutTemplate = new QGridLayout();
    layoutTemplate->addWidget(new QLabel(tr("Template")), 0, 0);
    layoutTemplate->addWidget(txtTemplate, 0, 1);
    layoutTemplate->addWidget(new QLabel(tr("Style sheet")), 1, 0);
    layoutTemplate->addWidget(txtStyleSheet, 1, 1);

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnShowReport);
    layoutButtons->addWidget(btnDefault);
    layoutButtons->addWidget(btnClose);

    QGroupBox *grpBasicProperties = new QGroupBox(tr("Sections"));
    grpBasicProperties->setLayout(layoutSections);

    QGroupBox *grpFigure = new QGroupBox(tr("Figures properties"));
    grpFigure->setLayout(layoutFigures);

    QGroupBox *grpAdditionalProperties = new QGroupBox(tr("Additional properties"));
    grpAdditionalProperties->setLayout(layoutTemplate);

    QGridLayout *layout = new QGridLayout();
    layout->addWidget(grpBasicProperties, 0, 0);
    layout->addWidget(grpFigure, 0, 1);
    layout->addWidget(grpAdditionalProperties, 1, 0, 1, 2);
    layout->addLayout(layoutButtons, 2, 0, 2, 2);

    setLayout(layout);
}

void ReportDialog::checkPaths()
{
    logMessage("ReportDialog::checkPaths()");

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
    logMessage("ReportDialog::defaultValues()");

    chkProblemInformation->setChecked(true);
    chkPhysicalProperties->setChecked(true);
    chkGeometry->setChecked(true);
    chkScript->setChecked(true);

    chkFigureGeometry->setChecked(true);
    txtFigureWidth->setValue(Value("600"));
    txtFigureHeight->setValue(Value("400"));
    chkFigureShowGrid->setChecked(Util::config()->showGrid);
    chkFigureShowRulers->setChecked(Util::config()->showRulers);
    chkFigureShowAxes->setChecked(Util::config()->showAxes);
    chkFigureShowLabel->setChecked(Util::config()->showLabel);

    txtTemplate->setText(QString("%1/resources/report/default.tpl").arg(datadir()));
    txtStyleSheet->setText(QString("%1/resources/report/default.css").arg(datadir()));

    checkPaths();
}

void ReportDialog::setControls()
{
    logMessage("ReportDialog::setControls()");

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
    chkFigureContourView->setDisabled(!Util::scene()->sceneSolution()->isSolved());
    chkFigureContourView->setChecked(Util::scene()->sceneSolution()->isSolved());
    chkFigureVectorView->setDisabled(!Util::scene()->sceneSolution()->isSolved());
    chkFigureVectorView->setChecked(Util::scene()->sceneSolution()->isSolved());
}

void ReportDialog::resetControls()
{
    logMessage("ReportDialog::resetControls()");

    chkFigureGeometry->setChecked(chkGeometry->isChecked());
    chkFigureGeometry->setEnabled(chkGeometry->isChecked());
    chkFigureMesh->setChecked(chkMeshAndSolution->isChecked());
    chkFigureMesh->setEnabled(chkMeshAndSolution->isChecked());
    chkFigureOrder->setChecked(chkMeshAndSolution->isChecked());
    chkFigureOrder->setEnabled(chkMeshAndSolution->isChecked());
    chkFigureScalarView->setChecked(chkMeshAndSolution->isChecked());
    chkFigureScalarView->setEnabled(chkMeshAndSolution->isChecked());

    chkFigureShowGrid->setEnabled(chkGeometry->isChecked() || chkMeshAndSolution->isChecked());
    chkFigureShowRulers->setEnabled(chkGeometry->isChecked() || chkMeshAndSolution->isChecked());
    chkFigureShowAxes->setEnabled(chkGeometry->isChecked() || chkMeshAndSolution->isChecked());
    chkFigureShowLabel->setEnabled(chkGeometry->isChecked() || chkMeshAndSolution->isChecked());
}

void ReportDialog::showDialog()
{
    logMessage("ReportDialog::showDialog()");

    setControls();
    show();
    activateWindow();
    raise();
}

void ReportDialog::doClose()
{
    logMessage("ReportDialog::doClose()");

    hide();
}

void ReportDialog::doShowReport()
{
    logMessage("ReportDialog::doShowReport()");

    QDir(tempProblemDir()).mkdir("report");

    QFile::remove(QString("%1/report/template.html").arg(tempProblemDir()));
    QFile::remove(QString("%1/report/style.css").arg(tempProblemDir()));

    bool fileStyleOK = QFile::copy(QString(txtStyleSheet->text()),
                                      QString("%1/report/style.css").arg(tempProblemDir()));
    if (!fileStyleOK)
        QMessageBox::critical(QApplication::activeWindow(), tr("Error"), tr("Template style could not be copied."));
    else
    {
        generateFigures();
        generateIndex();

        QDesktopServices::openUrl(QUrl::fromLocalFile(tempProblemDir() + "/report/report.html"));
    }
}

void ReportDialog::generateIndex()
{
    logMessage("ReportDialog::generateIndex()");

    QString fileNameTemplate = txtTemplate->text();
    QString fileNameIndex = tempProblemDir() + "/report/report.html";

    QFile fileIndex(fileNameIndex);
    if (fileIndex.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&fileIndex);
        stream.setCodec("UTF-8");
        stream << replaceTemplates(fileNameTemplate);

        fileIndex.flush();
        fileIndex.close();
    }
}

void ReportDialog::generateFigures()
{
    logMessage("ReportDialog::generateFigures()");

    bool showRulers = chkFigureShowRulers->isChecked();
    bool showGrid = chkFigureShowGrid->isChecked();
    bool showAxes = chkFigureShowRulers->isChecked();
    bool showLabel = chkFigureShowGrid->isChecked();
    m_sceneView->saveImagesForReport(tempProblemDir() + "/report",
                                     showGrid,
                                     showRulers,
                                     showAxes,
                                     showLabel,
                                     txtFigureWidth->value().number(),
                                     txtFigureHeight->value().number());
}

QString ReportDialog::replaceTemplates(const QString &fileNameTemplate)
{
    logMessage("ReportDialog::replaceTemplates()");

    std::string report;
    ctemplate::TemplateDictionary dict("report");

    // stylesheet
    dict.SetValue("STYLESHEET", "./style.css");

    // problem name
    dict.SetValue("NAME", Util::scene()->problemInfo()->name.toStdString());

    // description
    if (chkDescription->isChecked())
    {
        dict.SetValue("DESCRIPTION_LABEL", tr("Description").toStdString());
        dict.SetValue("DESCRIPTION", Util::scene()->problemInfo()->description.toStdString());
    }

    // problem information
    if (chkProblemInformation->isChecked())
    {
        dict.SetValue("PROBLEM_LABEL", tr("Problem Information").toStdString());
        dict.SetValue("PROBLEM_NAME_LABEL", tr("Name:").toStdString());
        dict.SetValue("PROBLEM_NAME", Util::scene()->problemInfo()->name.toStdString());
        dict.SetValue("PROBLEM_DATE_LABEL", tr("Date:").toStdString());
        dict.SetValue("PROBLEM_DATE", Util::scene()->problemInfo()->date.toString("dd.MM.yyyy").toStdString());
        dict.SetValue("PROBLEM_FILENAME_LABEL", tr("File name:").toStdString());
        dict.SetValue("PROBLEM_FILENAME", QFileInfo(Util::scene()->problemInfo()->fileName).fileName().toStdString());
        dict.SetValue("PROBLEM_PROBLEMTYPE_LABEL", tr("Problem type:").toStdString());
        dict.SetValue("PROBLEM_PROBLEMTYPE", problemTypeString(Util::scene()->problemInfo()->problemType).toStdString());
        dict.SetValue("PROBLEM_PHYSICFIELD_LABEL", tr("Physic field:").toStdString());
        dict.SetValue("PROBLEM_PHYSICFIELD", Util::scene()->problemInfo()->module()->name);
        dict.SetValue("PROBLEM_ANALYSISTYPE_LABEL", tr("Analysis type:").toStdString());
        dict.SetValue("PROBLEM_ANALYSISTYPE", analysisTypeString(Util::scene()->problemInfo()->analysisType).toStdString());
        dict.SetValue("PROBLEM_NUMBEROFREFINEMENTS_LABEL", tr("Number of refinements:").toStdString());
        dict.SetIntValue("PROBLEM_NUMBEROFREFINEMENTS", Util::scene()->problemInfo()->numberOfRefinements);
        dict.SetValue("PROBLEM_POLYNOMIALORDER_LABEL", tr("Polynomial order:").toStdString());
        dict.SetIntValue("PROBLEM_POLYNOMIALORDER", Util::scene()->problemInfo()->polynomialOrder);

        dict.SetValue("PROBLEM_ADAPTIVITYTYPE_LABEL", tr("Adaptivity type:").toStdString());
        dict.SetValue("PROBLEM_ADAPTIVITYTYPE", adaptivityTypeString(Util::scene()->problemInfo()->adaptivityType).toStdString());
        dict.SetValue("PROBLEM_ADAPTIVITYSTEPS_LABEL", tr("Adaptivity steps:").toStdString());
        dict.SetIntValue("PROBLEM_ADAPTIVITYSTEPS", Util::scene()->problemInfo()->adaptivitySteps);
        dict.SetValue("PROBLEM_ADAPTIVITYTOLERANCE_LABEL", tr("Adaptivity tolerance:").toStdString());
        dict.SetFormattedValue("PROBLEM_ADAPTIVITYTOLERANCE", "%.2f", Util::scene()->problemInfo()->adaptivityTolerance);
        dict.SetValue("PROBLEM_MAXDOFS_LABEL", tr("Maximum DOFs:").toStdString());
        dict.SetIntValue("PROBLEM_MAXDOFS", Util::scene()->problemInfo()->adaptivityMaxDOFs);

        dict.SetValue("PROBLEM_FREQUENCY_LABEL", tr("Frequency:").toStdString());
        dict.SetFormattedValue("PROBLEM_FREQUENCY", "%.2f", Util::scene()->problemInfo()->frequency);
        dict.SetValue("PROBLEM_TIMESTEP_LABEL", tr("Time step:").toStdString());
        dict.SetIntValue("PROBLEM_TIMESTEP", Util::scene()->problemInfo()->timeStep.number());
        dict.SetValue("PROBLEM_TIMETOTAL_LABEL", tr("Total time:").toStdString());
        dict.SetIntValue("PROBLEM_TIMETOTAL", Util::scene()->problemInfo()->timeTotal.number());
        dict.SetValue("PROBLEM_INITITALCONDITION_LABEL", tr("Initial condition:").toStdString());
        dict.SetFormattedValue("PROBLEM_INITITALCONDITION", "%.2f", Util::scene()->problemInfo()->initialCondition.number());
    }

    // startup script
    if (chkStartupScript->isChecked())
    {
        dict.SetValue("STARTUPSCRIPT_LABEL", tr("Startup Script").toStdString());
        dict.SetValue("STARTUPSCRIPT", Util::scene()->problemInfo()->scriptStartup.toStdString());
    }

    // physical properties
    if (chkPhysicalProperties->isChecked())
    {
        dict.SetValue("PHYSICALPROPERTIES_LABEL", tr("Physical Properties").toStdString());
        dict.SetValue("MATERIALS_LABEL", tr("Materials").toStdString());
        //dict.SetValue("MATERIALS", );
        dict.SetValue("BOUNDARIES_LABEL", tr("Boundaris conditions").toStdString());
        //dict.SetValue("BOUNDARIES", );
    }

    // geometry
    if (chkGeometry->isChecked())
    {
        dict.SetValue("GEOMETRY_LABEL", tr("Geometry").toStdString());
        dict.SetValue("GEOMETRY_NODES_LABEL", tr("Nodes").toStdString());
        dict.SetValue("GEOMETRY_NODES", htmlGeometryNodes().toStdString());
        dict.SetValue("GEOMETRY_EDGES_LABEL", tr("Edges").toStdString());
        dict.SetValue("GEOMETRY_EDGES", htmlGeometryEdges().toStdString());
        dict.SetValue("GEOMETRY_LABELS_LABEL", tr("Labels").toStdString());
        dict.SetValue("GEOMETRY_LABELS", htmlGeometryLabels().toStdString());
    }

    // solver
    if (chkMeshAndSolution->isChecked())
    {
        QTime time;
        time = milisecondsToTime(Util::scene()->sceneSolution()->timeElapsed());

        dict.SetValue("MESHANDSOLVER_LABEL", tr("Mesh and Solution").toStdString());
        dict.SetValue("SOLVER_LABEL", tr("Solver information").toStdString());
        dict.SetValue("SOLVER_NODES_LABEL", tr("Nodes:").toStdString());
        dict.SetIntValue("SOLVER_NODES", Util::scene()->sceneSolution()->meshInitial()->get_num_nodes());
        dict.SetValue("SOLVER_ELEMENTS_LABEL", tr("Elements:").toStdString());
        dict.SetIntValue("SOLVER_ELEMENTS", Util::scene()->sceneSolution()->meshInitial()->get_num_active_elements());
        dict.SetValue("SOLVER_DOFS_LABEL", tr("DOFs:").toStdString());

        if (Util::scene()->sceneSolution()->sln()->get_space())
            dict.SetIntValue("SOLVER_DOFS", Util::scene()->sceneSolution()->sln()->get_space()->get_num_dofs());

        dict.SetValue("SOLVER_TIMEELAPSED_LABEL", tr("Elapsed time:").toStdString());
        dict.SetValue("SOLVER_TIMEELAPSED", time.toString("mm:ss.zzz").toStdString());

        if (Util::scene()->problemInfo()->adaptivityType != AdaptivityType_None)
        {
            dict.SetValue("SOLVER_ADAPTIVEERROR_LABEL", tr("Adaptive error:").toStdString());
            dict.SetFormattedValue("SOLVER_ADAPTIVEERROR", "%.2f", Util::scene()->sceneSolution()->adaptiveError());
            dict.SetValue("SOLVER_DOFS_LABEL", tr("DOFs:").toStdString());
            dict.SetFormattedValue("SOLVER_DOFS", "%.2f", Util::scene()->sceneSolution()->adaptiveSteps());
        }
    }

    // script
    if (chkScript->isChecked())
    {
        dict.SetValue("SCRIPT_LABEL", tr("Script").toStdString());
        dict.SetValue("SCRIPT", createPythonFromModel().toStdString());
    }

    // figures
    dict.SetValue("FIGURE_LABEL", tr("Figures").toStdString());
    if (chkFigureGeometry->isChecked())
        dict.SetValue("FIGURE_GEOMETRY", htmlFigure("geometry.png", tr("Geometry")).toStdString());
    if (chkFigureMesh->isChecked())
        dict.SetValue("FIGURE_MESH", htmlFigure("mesh.png", tr("Mesh")).toStdString());
    if (chkFigureOrder->isChecked())
        dict.SetValue("FIGURE_ORDER", htmlFigure("order.png", tr("Polynomial order")).toStdString());
    if (chkFigureScalarView->isChecked())
        dict.SetValue("FIGURE_SCALARVIEW", htmlFigure("scalarview.png", tr("ScalarView: ")
                                                      + QString::fromStdString(Util::scene()->problemInfo()->module()->view_default_scalar_variable->name)).toStdString());
    if (chkFigureContourView->isChecked())
        dict.SetValue("FIGURE_CONTOURVIEW", htmlFigure("contourview.png", tr("ContourView: ")
                                                       + QString::fromStdString(Util::scene()->problemInfo()->module()->view_default_scalar_variable->name)).toStdString());
    if (chkFigureVectorView->isChecked())
        dict.SetValue("FIGURE_VECTORVIEW", htmlFigure("vectorview.png", tr("VectorView: ")
                                                      + QString::fromStdString(Util::scene()->problemInfo()->module()->view_default_vector_variable->name)).toStdString());

    // expand template
    ctemplate::ExpandTemplate(fileNameTemplate.toStdString(), ctemplate::DO_NOT_STRIP, &dict, &report);
    return  QString::fromStdString(report);
}

QString ReportDialog::htmlMaterials()
{
    logMessage("ReportDialog::htmlMaterials()");

    QString out;

    // FIXME - more general
    /*
    out  = "\n";
    for (int i = 1; i < Util::scene()->materials.count(); i++)
    {
        SceneMaterial *marker = Util::scene()->materials[i];
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
    */
    return out;
}

QString ReportDialog::htmlBoundaries()
{
    logMessage("ReportDialog::htmlBoundaries()");

    QString out;
    // FIXME - more general
    /*
    out  = "\n";
    for (int i = 1; i < Util::scene()->boundaries.count(); i++)
    {
        SceneBoundary *marker = Util::scene()->boundaries[i];
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
    */
    return out;
}

QString ReportDialog::htmlGeometryNodes()
{
    logMessage("ReportDialog::htmlGeometryNodes()");

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
    logMessage("ReportDialog::htmlGeometryEdges()");

    QString out;

    out  = "\n";
    out += "<table><tr>";
    out += "<th colspan=\"2\">" + tr("Start node") + "</th>";
    out += "<th colspan=\"2\">" + tr("End node") + "</th>";
    out += "<th>" + tr("Angle (deg.)") + "</th>";
    out += "<th>" + tr("Marker") + "</th></tr>";
    for (int i = 0; i < Util::scene()->edges.count(); i++)
    {
        out += "<tr><td>" + QString::number(Util::scene()->edges[i]->nodeStart->point.x, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->edges[i]->nodeStart->point.y, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->edges[i]->nodeEnd->point.x, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->edges[i]->nodeEnd->point.y, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->edges[i]->angle, 'f', 2) + "</td>";
        out += "<td>" + QString::fromStdString(Util::scene()->edges[i]->boundary->name) + "</td></tr>";
    }
    out += "</table>";
    out += "\n";

    return out;
}

QString ReportDialog::htmlGeometryLabels()
{
    logMessage("ReportDialog::htmlGeometryLabels()");

    QString out;

    out  = "\n";
    out += "<table><tr>";
    out += "<th>" + Util::scene()->problemInfo()->labelX() + " (m)</th>";
    out += "<th>" + Util::scene()->problemInfo()->labelY() + " (m)</th>";
    out += "<th>" + tr("Array (m)") + "</th>";
    out += "<th>" + tr("Marker") + "</th></tr>";
    for (int i = 0; i < Util::scene()->labels.count(); i++)
    {
        out += "<tr><td>" + QString::number(Util::scene()->labels[i]->point.x, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->labels[i]->point.y, 'e', 3) + "</td>";
        out += "<td>" + QString::number(Util::scene()->labels[i]->area, 'e', 3) + "</td>";
        out += "<td>" + QString::fromStdString(Util::scene()->labels[i]->material->name) + "</td></tr>";
    }
    out += "</table>";
    out += "\n";

    return out;
}

QString ReportDialog::htmlFigure(const QString &fileName, const QString &caption)
{
    logMessage("ReportDialog::htmlFigure()");

    QString out;

    if (QFile::exists(tempProblemDir() + "/report/" + fileName))
    {
        out += "\n";
        out += QString("<img src=\"%1\"><p>" + tr("Figure: %2") + "</p>").
                arg(fileName).
                arg(caption);
        out += "\n";
    }

    return out;
}
