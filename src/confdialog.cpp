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

#include "confdialog.h"

#include "gui.h"

#include "scene.h"
#include "sceneview_common.h"
#include "pythonlabagros.h"
#include "hermes2d/module.h"

ConfigDialog::ConfigDialog(QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon("options"));
    setWindowTitle(tr("Options"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

void ConfigDialog::load()
{
    // gui style
    cmbGUIStyle->setCurrentIndex(cmbGUIStyle->findText(Util::config()->guiStyle));
    if (cmbGUIStyle->currentIndex() == -1 && cmbGUIStyle->count() > 0) cmbGUIStyle->setCurrentIndex(0);

    // language
    cmbLanguage->setCurrentIndex(cmbLanguage->findText(Util::config()->language));
    if (cmbLanguage->currentIndex() == -1 && cmbLanguage->count() > 0) cmbLanguage->setCurrentIndex(0);

    // default physic field
    cmbDefaultPhysicField->setCurrentIndex(cmbDefaultPhysicField->findData(Util::config()->defaultPhysicField));

    // collaboration server
    txtCollaborationServerURL->setText(Util::config()->collaborationServerURL);

    // check version
    chkCheckVersion->setChecked(Util::config()->checkVersion);

    // show result in line edit value widget
    chkLineEditValueShowResult->setChecked(Util::config()->lineEditValueShowResult);

    // geometry
    txtMeshAngleSegmentsCount->setValue(Util::config()->angleSegmentsCount);
    chkMeshCurvilinearElements->setChecked(Util::config()->curvilinearElements);

    // number of threads
    chkNumOfThreads->setChecked(Util::config()->numberOfThreads > 0);
    txtNumOfThreads->setValue(Util::config()->numberOfThreads);
    connect(chkNumOfThreads, SIGNAL(stateChanged(int)), this, SLOT(numOfThreadsChecked(int)));
    numOfThreadsChecked(chkNumOfThreads->isChecked());

    // delete files
    chkDeleteTriangleMeshFiles->setChecked(Util::config()->deleteTriangleMeshFiles);
    chkDeleteHermes2DMeshFile->setChecked(Util::config()->deleteHermes2DMeshFile);

    // save problem with solution
    if (Util::config()->showExperimentalFeatures)
        chkSaveWithSolution->setChecked(Util::config()->saveProblemWithSolution);

    // adaptivity
    txtMaxDOFs->setValue(Util::config()->maxDofs);
    //chkIsoOnly->setChecked(Util::config()->isoOnly);
    txtConvExp->setValue(Util::config()->convExp);
    txtThreshold->setValue(Util::config()->threshold);
    cmbStrategy->setCurrentIndex(cmbStrategy->findData(Util::config()->strategy));
    cmbMeshRegularity->setCurrentIndex(cmbMeshRegularity->findData(Util::config()->meshRegularity));
    cmbProjNormType->setCurrentIndex(cmbProjNormType->findData(Util::config()->projNormType));

    // command argument
    txtArgumentTriangle->setText(Util::config()->commandTriangle);
    txtArgumentFFmpeg->setText(Util::config()->commandFFmpeg);

    // experimental features
    chkExperimentalFeatures->setChecked(Util::config()->showExperimentalFeatures);

    // global script
    txtGlobalScript->setPlainText(Util::config()->globalScript);
}

void ConfigDialog::save()
{
    // gui style
    Util::config()->guiStyle = cmbGUIStyle->currentText();
    setGUIStyle(cmbGUIStyle->currentText());

    // language
    if (Util::config()->language != cmbLanguage->currentText())
        QMessageBox::warning(QApplication::activeWindow(),
                                 tr("Language change"),
                                 tr("Interface language has been changed. You must restart the application."));
    Util::config()->language = cmbLanguage->currentText();

    // default physic field
    Util::config()->defaultPhysicField = cmbDefaultPhysicField->itemData(cmbDefaultPhysicField->currentIndex()).toString();

    // collaboration server
    QString collaborationServerUrl = txtCollaborationServerURL->text();
    if (!collaborationServerUrl.startsWith("http://"))
        collaborationServerUrl = QString("http://%1").arg(collaborationServerUrl);

    if (!collaborationServerUrl.endsWith("/"))
        collaborationServerUrl = QString("%1/").arg(collaborationServerUrl);

    Util::config()->collaborationServerURL = collaborationServerUrl;

    // check version
    Util::config()->checkVersion = chkCheckVersion->isChecked();

    // show result in line edit value widget
    Util::config()->lineEditValueShowResult = chkLineEditValueShowResult->isChecked();

    // mesh
    Util::config()->angleSegmentsCount = txtMeshAngleSegmentsCount->value();
    Util::config()->curvilinearElements = chkMeshCurvilinearElements->isChecked();

    // number of threads
    Util::config()->numberOfThreads = chkNumOfThreads->isChecked() ? txtNumOfThreads->value() : 0;

    // delete files
    Util::config()->deleteTriangleMeshFiles = chkDeleteTriangleMeshFiles->isChecked();
    Util::config()->deleteHermes2DMeshFile = chkDeleteHermes2DMeshFile->isChecked();

    // save problem with solution
    if (Util::config()->showExperimentalFeatures)
        Util::config()->saveProblemWithSolution = chkSaveWithSolution->isChecked();

    // adaptivity
    Util::config()->maxDofs = txtMaxDOFs->value();
    //Util::config()->isoOnly = chkIsoOnly->isChecked();
    Util::config()->convExp = txtConvExp->value();
    Util::config()->threshold = txtThreshold->value();
    Util::config()->strategy = cmbStrategy->itemData(cmbStrategy->currentIndex()).toInt();
    Util::config()->meshRegularity = cmbMeshRegularity->itemData(cmbMeshRegularity->currentIndex()).toInt();
    Util::config()->projNormType = (Hermes::Hermes2D::ProjNormType) cmbProjNormType->itemData(cmbProjNormType->currentIndex()).toInt();

    // command argument
    Util::config()->commandTriangle = txtArgumentTriangle->text();
    Util::config()->commandFFmpeg = txtArgumentFFmpeg->text();

    // experimental features
    Util::config()->showExperimentalFeatures = chkExperimentalFeatures->isChecked();

    // global script
    Util::config()->globalScript = txtGlobalScript->toPlainText();

    // save
    Util::config()->save();
}

void ConfigDialog::createControls()
{
    lstView = new QListWidget(this);
    pages = new QStackedWidget(this);

    panMain = createMainWidget();
    panSolver = createSolverWidget();
    panGlobalScriptWidget = createGlobalScriptWidget();

    // List View
    lstView->setCurrentRow(0);
    lstView->setViewMode(QListView::IconMode);
    lstView->setResizeMode(QListView::Adjust);
    lstView->setMovement(QListView::Static);
    lstView->setFlow(QListView::TopToBottom);
    lstView->setIconSize(QSize(60, 60));
    lstView->setMinimumWidth(135);
    lstView->setMaximumWidth(135);
    lstView->setMinimumHeight((45+fontMetrics().height()*4)*5);
    connect(lstView, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
               this, SLOT(doCurrentItemChanged(QListWidgetItem *, QListWidgetItem *)));

    QSize sizeItem(131, 85);

    // listView items
    QListWidgetItem *itemMain = new QListWidgetItem(icon("options-main"), tr("Main"), lstView);
    itemMain->setTextAlignment(Qt::AlignHCenter);
    itemMain->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemMain->setSizeHint(sizeItem);

    QListWidgetItem *itemSolver = new QListWidgetItem(icon("options-solver"), tr("Solver"), lstView);
    itemSolver->setTextAlignment(Qt::AlignHCenter);
    itemSolver->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemSolver->setSizeHint(sizeItem);

    QListWidgetItem *itemGlobalScript = new QListWidgetItem(icon("options-python"), tr("Python"), lstView);
    itemGlobalScript->setTextAlignment(Qt::AlignHCenter);
    itemGlobalScript->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemGlobalScript->setSizeHint(sizeItem);

    pages->addWidget(panMain);
    pages->addWidget(panSolver);
    pages->addWidget(panGlobalScriptWidget);

    QHBoxLayout *layoutHorizontal = new QHBoxLayout();
    layoutHorizontal->addWidget(lstView);
    layoutHorizontal->addWidget(pages);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutHorizontal);
    // layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

QWidget *ConfigDialog::createMainWidget()
{
    QWidget *mainWidget = new QWidget(this);

    // general
    cmbGUIStyle = new QComboBox(mainWidget);
    cmbGUIStyle->addItems(QStyleFactory::keys());
    cmbGUIStyle->addItem("Manhattan");

    cmbLanguage = new QComboBox(mainWidget);
    cmbLanguage->addItems(availableLanguages());

    cmbDefaultPhysicField = new QComboBox();
    fillComboBoxPhysicField(cmbDefaultPhysicField);

    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->addWidget(new QLabel(tr("UI:")), 0, 0);
    layoutGeneral->addWidget(cmbGUIStyle, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Language:")), 1, 0);
    layoutGeneral->addWidget(cmbLanguage, 1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Default physic field:")), 2, 0);
    layoutGeneral->addWidget(cmbDefaultPhysicField, 2, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // collaboration
    txtCollaborationServerURL = new SLineEditDouble();

    QVBoxLayout *layoutCollaboration = new QVBoxLayout();
    layoutCollaboration->addWidget(new QLabel(tr("Collaboration server URL:")));
    layoutCollaboration->addWidget(txtCollaborationServerURL);

    QGroupBox *grpCollaboration = new QGroupBox(tr("Collaboration"));
    grpCollaboration->setLayout(layoutCollaboration);

    // other
    chkLineEditValueShowResult = new QCheckBox(tr("Show value result in line edit input"));
    chkCheckVersion = new QCheckBox(tr("Check new version during startup"));
    chkExperimentalFeatures = new QCheckBox(tr("Enable experimental features"));
    chkExperimentalFeatures->setToolTip(tr("Warning: Agros2D should be unstable!"));

    QVBoxLayout *layoutOther = new QVBoxLayout();
    layoutOther->addWidget(chkLineEditValueShowResult);
    layoutOther->addWidget(chkCheckVersion);
    layoutOther->addWidget(chkExperimentalFeatures);

    QGroupBox *grpOther = new QGroupBox(tr("Other"));
    grpOther->setLayout(layoutOther);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpGeneral);
    layout->addWidget(grpCollaboration);
    layout->addWidget(grpOther);
    layout->addStretch();

    mainWidget->setLayout(layout);

    return mainWidget;
}

QWidget *ConfigDialog::createSolverWidget()
{
    // general
    chkDeleteTriangleMeshFiles = new QCheckBox(tr("Delete files with initial mesh (Triangle)"));
    chkDeleteHermes2DMeshFile = new QCheckBox(tr("Delete files with solution mesh (Hermes2D)"));
    if (Util::config()->showExperimentalFeatures)
        chkSaveWithSolution = new QCheckBox(tr("Save problem with solution"));

    chkNumOfThreads = new QCheckBox(tr("Enable threads settings"));
    txtNumOfThreads = new QSpinBox(this);
    txtNumOfThreads->setMinimum(0);
    txtNumOfThreads->setMaximum(omp_get_max_threads());

    QGridLayout *layoutSolver = new QGridLayout();
    layoutSolver->addWidget(chkDeleteTriangleMeshFiles, 0, 0, 1, 2);
    layoutSolver->addWidget(chkDeleteHermes2DMeshFile, 1, 0, 1, 2);
    layoutSolver->addWidget(chkNumOfThreads, 2, 0, 1, 2);
    layoutSolver->addWidget(new QLabel(tr("Number of threads:")), 3, 0);
    layoutSolver->addWidget(txtNumOfThreads, 3, 2);
    if (Util::config()->showExperimentalFeatures)
        layoutSolver->addWidget(chkSaveWithSolution);

    QGroupBox *grpSolver = new QGroupBox(tr("Solver"));
    grpSolver->setLayout(layoutSolver);

    txtMeshAngleSegmentsCount = new QSpinBox(this);
    txtMeshAngleSegmentsCount->setMinimum(2);
    txtMeshAngleSegmentsCount->setMaximum(20);
    chkMeshCurvilinearElements = new QCheckBox(tr("Curvilinear elements"));

    QGridLayout *layoutMesh = new QGridLayout();
    layoutMesh->addWidget(new QLabel(tr("Angle segments count:")), 0, 0);
    layoutMesh->addWidget(txtMeshAngleSegmentsCount, 0, 1);
    layoutMesh->addWidget(chkMeshCurvilinearElements, 1, 0, 1, 2);

    QGroupBox *grpMesh = new QGroupBox(tr("Mesh"));
    grpMesh->setLayout(layoutMesh);

    QVBoxLayout *layoutGeneral = new QVBoxLayout();
    layoutGeneral->addWidget(grpSolver);
    layoutGeneral->addWidget(grpMesh);
    layoutGeneral->addStretch();

    QWidget *solverGeneralWidget = new QWidget(this);
    solverGeneralWidget->setLayout(layoutGeneral);

    // adaptivity
    lblMaxDofs = new QLabel(tr("Maximum number of DOFs:"));
    txtMaxDOFs = new QSpinBox(this);
    txtMaxDOFs->setMinimum(1e2);
    txtMaxDOFs->setMaximum(1e9);
    txtMaxDOFs->setSingleStep(1e2);
    /*
    chkIsoOnly = new QCheckBox(tr("Isotropic refinement"));
    lblIsoOnly = new QLabel(tr("<table>"
                               "<tr><td><b>true</b></td><td>isotropic refinement</td></tr>"
                               "<tr><td><b>false</b></td><td>anisotropic refinement</td></tr>"
                               "</table>"));
    */
    txtConvExp = new SLineEditDouble();
    lblConvExp = new QLabel(tr("<b></b>default value is 1.0, this parameter influences<br/>the selection of candidates in hp-adaptivity"));
    txtThreshold = new SLineEditDouble();
    lblThreshold = new QLabel(tr("<b></b>quantitative parameter of the adapt(...) function<br/>with different meanings for various adaptive strategies"));
    cmbStrategy = new QComboBox();
    cmbStrategy->addItem(tr("0"), 0);
    cmbStrategy->addItem(tr("1"), 1);
    cmbStrategy->addItem(tr("2"), 2);
    lblStrategy = new QLabel(tr("<table>"
                                 "<tr><td><b>0</b></td><td>refine elements until sqrt(<b>threshold</b>)<br/>times total error is processed.<br/>If more elements have similar errors,<br/>refine all to keep the mesh symmetric</td></tr>"
                                 "<tr><td><b>1</b></td><td>refine all elements<br/>whose error is larger than <b>threshold</b><br/>times maximum element error</td></tr>"
                                 "<tr><td><b>2</b></td><td>refine all elements<br/>whose error is larger than <b>threshold</b></td></tr>"
                                 "</table>"));
    cmbMeshRegularity = new QComboBox();
    cmbMeshRegularity->addItem(tr("arbitrary level hang. nodes"), -1);
    cmbMeshRegularity->addItem(tr("at most one-level hang. nodes"), 1);
    cmbMeshRegularity->addItem(tr("at most two-level hang. nodes"), 2);
    cmbMeshRegularity->addItem(tr("at most three-level hang. nodes"), 3);
    cmbMeshRegularity->addItem(tr("at most four-level hang. nodes"), 4);
    cmbMeshRegularity->addItem(tr("at most five-level hang. nodes"), 5);

    cmbProjNormType = new QComboBox();
    cmbProjNormType->addItem(errorNormString(Hermes::Hermes2D::HERMES_H1_NORM), Hermes::Hermes2D::HERMES_H1_NORM);
    cmbProjNormType->addItem(errorNormString(Hermes::Hermes2D::HERMES_L2_NORM), Hermes::Hermes2D::HERMES_L2_NORM);
    cmbProjNormType->addItem(errorNormString(Hermes::Hermes2D::HERMES_H1_SEMINORM), Hermes::Hermes2D::HERMES_H1_SEMINORM);

    // default
    QPushButton *btnAdaptivityDefault = new QPushButton(tr("Default"));
    connect(btnAdaptivityDefault, SIGNAL(clicked()), this, SLOT(doAdaptivityDefault()));

    QGridLayout *layoutAdaptivitySettings = new QGridLayout();
    layoutAdaptivitySettings->addWidget(lblMaxDofs, 0, 0);
    layoutAdaptivitySettings->addWidget(txtMaxDOFs, 0, 1, 1, 2);
    layoutAdaptivitySettings->addWidget(new QLabel(tr("Conv. exp.:")), 2, 0);
    layoutAdaptivitySettings->addWidget(txtConvExp, 2, 1);
    layoutAdaptivitySettings->addWidget(lblConvExp, 3, 0, 1, 2);
    layoutAdaptivitySettings->addWidget(new QLabel(tr("Strategy:")), 4, 0);
    layoutAdaptivitySettings->addWidget(cmbStrategy, 4, 1);
    layoutAdaptivitySettings->addWidget(lblStrategy, 5, 0, 1, 2);
    layoutAdaptivitySettings->addWidget(new QLabel(tr("Threshold:")), 6, 0);
    layoutAdaptivitySettings->addWidget(txtThreshold, 6, 1);
    layoutAdaptivitySettings->addWidget(lblThreshold, 7, 0, 1, 2);
    layoutAdaptivitySettings->addWidget(new QLabel(tr("Mesh regularity:")), 8, 0);
    layoutAdaptivitySettings->addWidget(cmbMeshRegularity, 8, 1);
    layoutAdaptivitySettings->addWidget(new QLabel(tr("Norm:")), 9, 0);
    layoutAdaptivitySettings->addWidget(cmbProjNormType, 9, 1);

    QVBoxLayout *layoutAdaptivity = new QVBoxLayout();
    layoutAdaptivity->addLayout(layoutAdaptivitySettings);
    layoutAdaptivity->addStretch();
    layoutAdaptivity->addWidget(btnAdaptivityDefault, 0, Qt::AlignLeft);

    QWidget *solverAdaptivityWidget = new QWidget(this);
    solverAdaptivityWidget->setLayout(layoutAdaptivity);

    // commands
    txtArgumentTriangle = new QLineEdit("");
    txtArgumentFFmpeg = new QLineEdit("");

    // default
    QPushButton *btnCommandsDefault = new QPushButton(tr("Default"));
    connect(btnCommandsDefault, SIGNAL(clicked()), this, SLOT(doCommandsDefault()));

    QVBoxLayout *layoutCommands = new QVBoxLayout();
    layoutCommands->addWidget(new QLabel(tr("Triangle")));
    layoutCommands->addWidget(txtArgumentTriangle);
    layoutCommands->addWidget(new QLabel(tr("FFmpeg")));
    layoutCommands->addWidget(txtArgumentFFmpeg);
    layoutCommands->addStretch();
    layoutCommands->addWidget(btnCommandsDefault, 0, Qt::AlignLeft);

    QWidget *solverCommandsWidget = new QWidget(this);
    solverCommandsWidget->setLayout(layoutCommands);

    QTabWidget *solverWidget = new QTabWidget(this);
    solverWidget->addTab(solverGeneralWidget, icon(""), tr("General"));
    solverWidget->addTab(solverAdaptivityWidget, icon(""), tr("Adaptivity"));
    solverWidget->addTab(solverCommandsWidget, icon(""), tr("Commands"));

    return solverWidget;
}

QWidget *ConfigDialog::createGlobalScriptWidget()
{
    QWidget *viewWidget = new QWidget(this);

    txtGlobalScript = new ScriptEditor(currentPythonEngine(), this);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(txtGlobalScript);

    viewWidget->setLayout(layout);

    return viewWidget;
}

void ConfigDialog::fillComboBoxPhysicField(QComboBox *cmbPhysicField)
{
    // block signals
    cmbPhysicField->blockSignals(true);

    cmbPhysicField->clear();
    QMapIterator<QString, QString> it(availableModules());
    while (it.hasNext())
    {
        it.next();
        cmbPhysicField->addItem(it.value(), it.key());
    }

    // unblock signals
    cmbPhysicField->blockSignals(false);

    // default physic field
    cmbPhysicField->setCurrentIndex(cmbPhysicField->findData(Util::config()->defaultPhysicField));
    if (cmbPhysicField->currentIndex() == -1)
        cmbPhysicField->setCurrentIndex(0);
}

void ConfigDialog::doCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    pages->setCurrentIndex(lstView->row(current));
}

void ConfigDialog::doAccept()
{
    save();
    accept();
}

void ConfigDialog::doReject()
{
    reject();
}

void ConfigDialog::doAdaptivityDefault()
{
    txtMaxDOFs->setValue(MAX_DOFS);
    //chkIsoOnly->setChecked(ADAPTIVITY_ISOONLY);
    txtConvExp->setValue(ADAPTIVITY_CONVEXP);
    txtThreshold->setValue(ADAPTIVITY_THRESHOLD);
    cmbStrategy->setCurrentIndex(cmbStrategy->findData(ADAPTIVITY_STRATEGY));
    cmbMeshRegularity->setCurrentIndex(cmbMeshRegularity->findData(ADAPTIVITY_MESHREGULARITY));
    cmbProjNormType->setCurrentIndex(cmbProjNormType->findData(ADAPTIVITY_PROJNORMTYPE));
}

void ConfigDialog::doCommandsDefault()
{
    txtArgumentTriangle->setText(COMMANDS_TRIANGLE);
    txtArgumentFFmpeg->setText(COMMANDS_FFMPEG);
}

void ConfigDialog::numOfThreadsChecked(int state)
{
    txtNumOfThreads->setEnabled(chkNumOfThreads->isChecked());
}


