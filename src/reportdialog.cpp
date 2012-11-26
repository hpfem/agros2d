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

#include <ctemplate/template.h>

#include "reportdialog.h"
#include "scene.h"
#include "scenebasic.h"
#include "sceneview_common.h"
#include "scenemarkerdialog.h"
#include "pythonlab/pythonengine_agros.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"

#include "util/constants.h"

ReportDialog::ReportDialog(SceneViewCommon *sceneView, QWidget *parent) : QDialog(parent)
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
    txtFigureWidth->setValue(Value("600"));
    txtFigureHeight->setValue(Value("400"));
    chkFigureShowGrid->setChecked(Util::config()->showGrid);
    chkFigureShowRulers->setChecked(Util::config()->showRulers);
    chkFigureShowAxes->setChecked(Util::config()->showAxes);

    txtTemplate->setText(QString("%1/report/default.tpl").arg(datadir() + TEMPLATEROOT));
    txtStyleSheet->setText(QString("%1/report/default.css").arg(datadir() + TEMPLATEROOT));

    checkPaths();
}

void ReportDialog::setControls()
{
//    assert(0); //TODO
//    chkDescription->setDisabled(Util::problem()->config()->description.isEmpty());
//    chkDescription->setChecked(!Util::problem()->config()->description.isEmpty());
//    chkStartupScript->setDisabled(Util::problem()->config()->scriptStartup.isEmpty());
//    chkStartupScript->setChecked(!Util::problem()->config()->scriptStartup.isEmpty());

//    chkMeshAndSolution->setDisabled(!Util::scene()->sceneSolution()->isSolved());
//    chkMeshAndSolution->setChecked(Util::scene()->sceneSolution()->isSolved());
//    chkFigureMesh->setDisabled(!Util::scene()->sceneSolution()->isSolved());
//    chkFigureMesh->setChecked(Util::scene()->sceneSolution()->isSolved());
//    chkFigureOrder->setDisabled(!Util::scene()->sceneSolution()->isSolved());
//    chkFigureOrder->setChecked(Util::scene()->sceneSolution()->isSolved());
//    chkFigureScalarView->setDisabled(!Util::scene()->sceneSolution()->isSolved());
//    chkFigureScalarView->setChecked(Util::scene()->sceneSolution()->isSolved());
//    chkFigureContourView->setDisabled(!Util::scene()->sceneSolution()->isSolved());
//    chkFigureContourView->setChecked(Util::scene()->sceneSolution()->isSolved());
//    chkFigureVectorView->setDisabled(!Util::scene()->sceneSolution()->isSolved());
//    chkFigureVectorView->setChecked(Util::scene()->sceneSolution()->isSolved());
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

    chkFigureShowGrid->setEnabled(chkGeometry->isChecked() || chkMeshAndSolution->isChecked());
    chkFigureShowRulers->setEnabled(chkGeometry->isChecked() || chkMeshAndSolution->isChecked());
    chkFigureShowAxes->setEnabled(chkGeometry->isChecked() || chkMeshAndSolution->isChecked());
}

void ReportDialog::showDialog()
{
    setControls();
    activateWindow();
    raise();
    exec();
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
    assert(0); //TODO
//    std::string report;
//    ctemplate::TemplateDictionary dict("report");

//    // stylesheet
//    dict.SetValue("STYLESHEET", "./style.css");

//    // problem name
//    dict.SetValue("NAME", Util::problem()->config()->name.toStdString());

//    // description
//    if (chkDescription->isChecked())
//    {
//        dict.SetValue("DESCRIPTION_LABEL", tr("Description").toStdString());
//        dict.SetValue("DESCRIPTION", Util::problem()->config()->description.toStdString());

//        dict.ShowSection("DESCRIPTION_SECTION");
//    }

//    // problem information
//    if (chkProblemInformation->isChecked())
//    {
//        dict.SetValue("PROBLEM_LABEL", tr("Problem Information").toStdString());
//        dict.SetValue("PROBLEM_NAME_LABEL", tr("Name:").toStdString());
//        dict.SetValue("PROBLEM_NAME", Util::problem()->config()->name.toStdString());
//        dict.SetValue("PROBLEM_DATE_LABEL", tr("Date:").toStdString());
//        dict.SetValue("PROBLEM_DATE", Util::problem()->config()->date.toString("dd.MM.yyyy").toStdString());
//        dict.SetValue("PROBLEM_FILENAME_LABEL", tr("File name:").toStdString());
//        dict.SetValue("PROBLEM_FILENAME", QFileInfo(Util::problem()->config()->fileName).fileName().toStdString());
//        dict.SetValue("PROBLEM_PROBLEMTYPE_LABEL", tr("Problem type:").toStdString());
//        dict.SetValue("PROBLEM_PROBLEMTYPE", problemTypeString(Util::problem()->config()->problemType).toStdString());
//        dict.SetValue("PROBLEM_PHYSICFIELD_LABEL", tr("Physic field:").toStdString());
//        dict.SetValue("PROBLEM_PHYSICFIELD", Util::problem()->config()->module()->name);
//        dict.SetValue("PROBLEM_ANALYSISTYPE_LABEL", tr("Analysis type:").toStdString());
//        dict.SetValue("PROBLEM_ANALYSISTYPE", analysisTypeString(Util::problem()->config()->analysisType).toStdString());
//        dict.SetValue("PROBLEM_NUMBEROFREFINEMENTS_LABEL", tr("Number of refinements:").toStdString());
//        dict.SetValue("PROBLEM_POLYNOMIALORDER_LABEL", tr("Polynomial order:").toStdString());
//        dict.SetIntValue("PROBLEM_POLYNOMIALORDER", Util::problem()->config()->polynomialOrder);
//        dict.SetIntValue("PROBLEM_NUMBEROFREFINEMENTS", Util::problem()->config()->numberOfRefinements);
//        dict.SetValue("PROBLEM_NEWTONTOLERANCE_LABEL", tr("Newton solver tolerance:").toStdString());
//        dict.SetFormattedValue ("PROBLEM_NEWTONTOLERANCE", "%f", Util::problem()->config()->nonlinearTolerance);
//        dict.SetValue("PROBLEM_NEWTONSTEPS_LABEL", tr("Newton solver steps:").toStdString());
//        dict.SetIntValue("PROBLEM_NEWTONSTEPS", Util::problem()->config()->nonlinearSteps);

//        // adaptivity
//        if (Util::scene ()->problemInfo ()->adaptivityType != AdaptivityType_None)
//        {
//            dict.SetValue("PROBLEM_ADAPTIVITYTYPE_LABEL", tr("Adaptivity type:").toStdString());
//            dict.SetValue("PROBLEM_ADAPTIVITYTYPE", adaptivityTypeString(Util::problem()->config()->adaptivityType).toStdString());
//            dict.SetValue("PROBLEM_ADAPTIVITYSTEPS_LABEL", tr("Adaptivity steps:").toStdString());
//            dict.SetIntValue("PROBLEM_ADAPTIVITYSTEPS", Util::problem()->config()->adaptivitySteps);
//            dict.SetValue("PROBLEM_ADAPTIVITYTOLERANCE_LABEL", tr("Adaptivity tolerance:").toStdString());
//            dict.SetFormattedValue("PROBLEM_ADAPTIVITYTOLERANCE", "%f", Util::problem()->config()->adaptivityTolerance);
//            dict.SetValue("PROBLEM_MAXDOFS_LABEL", tr("Maximum DOFs:").toStdString());

//            dict.ShowSection("PROBLEM_ADAPTIVITY_SECTION");
//        }

//        // analysis type
//        if (Util::problem()->config()->analysisType() == AnalysisType_Harmonic)
//        {
//            dict.SetValue("PROBLEM_FREQUENCY_LABEL", tr("Frequency:").toStdString());
//            dict.SetFormattedValue("PROBLEM_FREQUENCY", "%f", Util::problem()->config()->frequency);

//            dict.ShowSection("PROBLEM_HARMONIC_SECTION");
//        }
//        if (Util::problem()->config()->analysisType() == AnalysisType_Transient)
//        {
//            dict.SetValue("PROBLEM_TIMESTEP_LABEL", tr("Time step:").toStdString());
//            dict.SetIntValue("PROBLEM_TIMESTEP", Util::problem()->config()->timeStep.number());
//            dict.SetValue("PROBLEM_TIMETOTAL_LABEL", tr("Total time:").toStdString());
//            dict.SetIntValue("PROBLEM_TIMETOTAL", Util::problem()->config()->timeTotal.number());
//            dict.SetValue("PROBLEM_INITITALCONDITION_LABEL", tr("Initial condition:").toStdString());
//            dict.SetFormattedValue("PROBLEM_INITITALCONDITION", "%f", Util::problem()->config()->initialCondition.number());

//            dict.ShowSection("PROBLEM_TRANSIENT_SECTION");
//        }

//        dict.ShowSection("PROBLEM_SECTION");
//    }

//    // startup script
//    if (chkStartupScript->isChecked())
//    {
//        dict.SetValue("STARTUPSCRIPT_LABEL", tr("Startup Script").toStdString());
//        dict.SetValue("STARTUPSCRIPT", Util::problem()->config()->scriptStartup.toStdString());

//        dict.ShowSection("STARTUPSCRIPT_SECTION");
//    }

//    // physical properties
//    if (chkPhysicalProperties->isChecked())
//    {
//        dict.SetValue("PHYSICALPROPERTIES_LABEL", tr("Physical Properties").toStdString());

//        // materials
//        dict.SetValue("MATERIALS_LABEL", tr("Materials").toStdString());
//        for (int i = 1; i < Util::scene()->materials.count(); i++)
//        {
//            SceneMaterial *marker = Util::scene()->materials[i];
//            ctemplate::TemplateDictionary *material_dict = dict.AddSectionDictionary("MATERIAL_SECTION");
//            material_dict->SetValue("MATERIAL_NAME", marker->name);

//            Hermes::vector<Module::MaterialTypeVariable *> variables = Util::problem()->config()->module()->material_type_variables;
//            for (Hermes::vector<Module::MaterialTypeVariable *>::iterator it = variables.begin(); it < variables.end(); ++it)
//            {
//                Module::MaterialTypeVariable *variable = ((Module::MaterialTypeVariable *) *it);
//                ctemplate::TemplateDictionary *material_key_dict = material_dict->AddSectionDictionary("MATERIAL_KEY_SECTION");

//                //material_key_dict->SetValue("MATERIAL_KEY", variable->name);
//                material_key_dict->SetValue("MATERIAL_KEY_VALUE", marker->get_value(variable->id).toString().toStdString());
//                //material_key_dict->SetValue("MATERIAL_KEY_UNIT", variable->unit);
//            }
//        }

//        // boundaries
//        dict.SetValue("BOUNDARIES_LABEL", tr("Boundaris conditions").toStdString());
//        for (int i = 1; i < Util::scene()->boundaries.count(); i++)
//        {
//            SceneBoundary *marker = Util::scene()->boundaries[i];
//            ctemplate::TemplateDictionary *boundary_dict = dict.AddSectionDictionary("BOUNDARY_SECTION");
//            boundary_dict->SetValue("BOUNDARY_NAME", marker->name);

//            Module::BoundaryType *boundary_type = Util::problem()->config()->module()->get_boundary_type(marker->type);
//            if (boundary_type)
//            {
//                for (Hermes::vector<Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin(); it < boundary_type->variables.end(); ++it)
//                {
//                    Module::BoundaryTypeVariable *variable = ((Module::BoundaryTypeVariable *) *it);
//                    ctemplate::TemplateDictionary *boundary_key_dict = boundary_dict->AddSectionDictionary("BOUNDARY_KEY_SECTION");

//                    //boundary_key_dict->SetValue("BOUNDARY_KEY", variable->name);
//                    boundary_key_dict->SetValue("BOUNDARY_KEY_VALUE", marker->get_value(variable->id).toString().toStdString());
//                    //boundary_key_dict->SetValue("BOUNDARY_KEY_UNIT", variable->unit);
//                }
//            }
//        }
//        dict.ShowSection("PHYSICALPROPERTIES_SECTION");
//    }

//    // geometry
//    if (chkGeometry->isChecked())
//    {
//        dict.SetValue("GEOMETRY_XLABEL", Util::problem()->config()->labelX().toStdString());
//        dict.SetValue("GEOMETRY_YLABEL", Util::problem()->config()->labelY().toStdString());

//        dict.SetValue("GEOMETRY_STARTNODE", tr("Start node").toStdString());
//        dict.SetValue("GEOMETRY_ENDNODE", tr("End node").toStdString());
//        dict.SetValue("GEOMETRY_ANGLE", tr("Angel").toStdString());
//        dict.SetValue("GEOMETRY_MARKER", tr("Marker").toStdString());

//        dict.SetValue("GEOMETRY_ARRAY", tr("Array").toStdString());
//        dict.SetValue("GEOMETRY_POLYNOMIALORDER", tr("Polynomial order").toStdString());

//        // nodes
//        dict.SetValue("GEOMETRY_LABEL", tr("Geometry").toStdString());
//        dict.SetValue("GEOMETRY_NODES_LABEL", tr("Nodes").toStdString());
//        for (int i = 0; i < Util::scene()->nodes.count(); i++)
//        {
//            ctemplate::TemplateDictionary *node_dict = dict.AddSectionDictionary("GEOMETRY_NODE_SECTION");
//            node_dict->SetFormattedValue("GEOMETRY_NODEX", "%f", Util::scene()->nodes[i]->point.x);
//            node_dict->SetFormattedValue("GEOMETRY_NODEY", "%f", Util::scene()->nodes[i]->point.y);
//        }

//        // edges
//        dict.SetValue("GEOMETRY_EDGES_LABEL", tr("Edges").toStdString());
//        for (int i = 0; i < Util::scene()->edges.count(); i++)
//        {
//            ctemplate::TemplateDictionary *edge_dict = dict.AddSectionDictionary("GEOMETRY_EDGE_SECTION");
//            edge_dict->SetFormattedValue("GEOMETRY_EDGE_STARTNODEX", "%f", Util::scene()->edges[i]->nodeStart->point.x);
//            edge_dict->SetFormattedValue("GEOMETRY_EDGE_STARTNODEY", "%f", Util::scene()->edges[i]->nodeStart->point.y);
//            edge_dict->SetFormattedValue("GEOMETRY_EDGE_ENDNODEX", "%f", Util::scene()->edges[i]->nodeEnd->point.x);
//            edge_dict->SetFormattedValue("GEOMETRY_EDGE_ENDNODEY", "%f", Util::scene()->edges[i]->nodeEnd->point.y);
//            edge_dict->SetFormattedValue("GEOMETRY_EDGE_ANGLE", "%f", Util::scene()->edges[i]->angle);
//            edge_dict->SetValue ("GEOMETRY_EDGE_MARKER", Util::scene()->edges[i]->boundary->name);
//        }

//        // labels
//        dict.SetValue("GEOMETRY_LABELS_LABEL", tr("Labels").toStdString());
//        for (int i = 0; i < Util::scene()->labels.count(); i++)
//        {
//            ctemplate::TemplateDictionary *label_dict = dict.AddSectionDictionary("GEOMETRY_LABEL_SECTION");
//            label_dict->SetFormattedValue("GEOMETRY_LABELX", "%f", Util::scene()->labels[i]->point.x);
//            label_dict->SetFormattedValue("GEOMETRY_LABELY", "%f", Util::scene()->labels[i]->point.y);
//            label_dict->SetFormattedValue("GEOMETRY_LABEL_ARRAY", "%f", Util::scene()->labels[i]->area);
//            label_dict->SetValue ("GEOMETRY_LABEL_MARKER", Util::scene()->labels[i]->material->name);
//            label_dict->SetIntValue ("GEOMETRY_LABEL_POLYNOMIALORDER", Util::scene()->labels[i]->polynomialOrder);
//        }

//        dict.ShowSection("GEOMETRY_SECTION");
//    }

//    // solver
//    if (chkMeshAndSolution->isChecked())
//    {
//        QTime time;
//        time = milisecondsToTime(Util::scene()->sceneSolution()->timeElapsed());

//        dict.SetValue("MESHANDSOLVER_LABEL", tr("Mesh and Solution").toStdString());
//        dict.SetValue("SOLVER_LABEL", tr("Solver information").toStdString());
//        dict.SetValue("SOLVER_NODES_LABEL", tr("Nodes:").toStdString());
//        dict.SetIntValue("SOLVER_NODES", Util::scene()->sceneSolution()->meshInitial()->get_num_nodes());
//        dict.SetValue("SOLVER_ELEMENTS_LABEL", tr("Elements:").toStdString());
//        dict.SetIntValue("SOLVER_ELEMENTS", Util::scene()->sceneSolution()->meshInitial()->get_num_active_elements());
//        dict.SetValue("SOLVER_DOFS_LABEL", tr("DOFs:").toStdString());

//        if (Util::scene()->sceneSolution()->space())
//            dict.SetIntValue("SOLVER_DOFS", Util::scene()->sceneSolution()->space()->get_num_dofs());

//        dict.SetValue("SOLVER_TIMEELAPSED_LABEL", tr("Elapsed time:").toStdString());
//        dict.SetValue("SOLVER_TIMEELAPSED", time.toString("mm:ss.zzz").toStdString());

//        // adaptivity
//        if (Util::problem()->config()->adaptivityType != AdaptivityType_None)
//        {
//            dict.SetValue("SOLVER_ADAPTIVEERROR_LABEL", tr("Adaptive error:").toStdString());
//            dict.SetFormattedValue("SOLVER_ADAPTIVEERROR", "%f", Util::scene()->sceneSolution()->adaptiveError());
//            dict.SetValue("SOLVER_ADAPTIVESTEPS_LABEL", tr("Adaptive steps:").toStdString());
//            dict.SetIntValue("SOLVER_ADAPTIVESTEPS", Util::scene()->sceneSolution()->adaptiveSteps());

//            QFile::remove(QString("%1/report/adaptivity_convergence.png").arg(tempProblemDir()));
//            bool copyChart = QFile::copy(QString("%1/adaptivity_conv.png").arg(tempProblemDir()), QString("%1/report/adaptivity_convergence.png").arg(tempProblemDir()));

//            if (copyChart)
//            {
//                dict.SetValue("CONVERGENCE_CHARTS_LABEL", tr("Convergence charts").toStdString());
//                dict.SetValue("FIGURE_ADAPTIVITY", tempProblemDir().toStdString()+ "/report/adaptivity_convergence.png");
//                dict.SetValue("FIGURE_ADAPTIVITY_DESCRIPTION", tr("Figure: ").toStdString() + tr("Adaptivity convergence chart").toStdString());

//                dict.ShowSection("CONVERGENCE_CHARTS_SECTION");
//            }


//            dict.ShowSection("SOLVER_ADAPTIVITY_SECTION");
//        }

//        dict.ShowSection("MESHANDSOLVER_SECTION");
//    }

//    // script
//    if (chkScript->isChecked())
//    {
//        dict.SetValue("SCRIPT_LABEL", tr("Script").toStdString());
//        dict.SetValue("SCRIPT", createPythonFromModel().toStdString());

//        dict.ShowSection("SCRIPT_SECTION");
//    }

//    // figures
//    dict.SetValue("FIGURE_LABEL", tr("Figures").toStdString());
//    if (chkFigureGeometry->isChecked())
//    {
//        dict.SetValue("FIGURE_GEOMETRY", tempProblemDir().toStdString()+ "/report/geometry.png");
//        dict.SetValue("FIGURE_GEOMETRY_DESCRIPTION", tr("Figure: ").toStdString() + tr("Geometry").toStdString());
//    }

//    if (chkFigureMesh->isChecked())
//    {
//        dict.SetValue("FIGURE_MESH", tempProblemDir().toStdString()+ "/report/mesh.png");
//        dict.SetValue("FIGURE_MESH_DESCRIPTION", tr("Figure: ").toStdString() + tr("Initial and solution mesh").toStdString());
//        dict.ShowSection("FIGURE_SECTION");
//    }

//    if (chkFigureOrder->isChecked())
//    {
//        dict.SetValue("FIGURE_ORDER", tempProblemDir().toStdString()+ "/report/order.png");
//        dict.SetValue("FIGURE_ORDER_DESCRIPTION", tr("Figure: ").toStdString() + tr("Polynomial order").toStdString());
//        dict.ShowSection("FIGURE_SECTION");
//    }

//    if (chkFigureScalarView->isChecked())
//    {
//        dict.SetValue("FIGURE_SCALARVIEW", tempProblemDir().toStdString()+ "/report/scalarview.png");
//        dict.SetValue("FIGURE_SCALARVIEW_DESCRIPTION", tr("Figure: ").toStdString()
//                      + QString::fromStdString(Util::problem()->config()->module()->view_default_scalar_variable->name).toStdString());
//        dict.ShowSection("FIGURE_SECTION");
//    }

//    if (chkFigureContourView->isChecked())
//    {
//        dict.SetValue("FIGURE_CONTOURVIEW", tempProblemDir().toStdString()+ "/report/contourview.png");
//        dict.SetValue("FIGURE_CONTOURVIEW_DESCRIPTION", tr("Figure: ").toStdString()
//                      + QString::fromStdString(Util::problem()->config()->module()->view_default_scalar_variable->name).toStdString() + tr(" (contours)").toStdString());
//        dict.ShowSection("FIGURE_SECTION");
//    }

//    if (chkFigureVectorView->isChecked())
//    {
//        dict.SetValue("FIGURE_VECTORVIEW", tempProblemDir().toStdString()+ "/report/vectorview.png");
//        dict.SetValue("FIGURE_VECTORVIEW_DESCRIPTION", tr("Figure: ").toStdString()
//                      + QString::fromStdString(Util::problem()->config()->module()->view_default_vector_variable->name).toStdString() + tr(" (vectors)").toStdString());
//        dict.ShowSection("FIGURE_SECTION");
//    }

//    // footer
//    dict.SetValue("FOOTER", tr("Computed by Agros2D (<a href=\"http://agros2d.org\">http://agros2d.org</a>)").toStdString ());

//    // units
//    dict.SetValue("PERCENT", tr("%").toStdString());
//    dict.SetValue("HERTZ", tr("Hz").toStdString());
//    dict.SetValue("SECOND", tr("s").toStdString());
//    dict.SetValue("METERS", tr("m").toStdString());
//    dict.SetValue("DEGREE", tr("deg.").toStdString());

//    // expand template
//    ctemplate::ExpandTemplate(fileNameTemplate.toStdString(), ctemplate::DO_NOT_STRIP, &dict, &report);
    return  QString::fromStdString("");
}
