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

#include "util/constants.h"
#include "util/global.h"
#include "gui/lineeditdouble.h"
#include "gui/systemoutput.h"

#include "scene.h"
#include "sceneview_common.h"
#include "pythonlab/pythonengine_agros.h"
#include "moduledialog.h"

#include "hermes2d/module.h"
#include "hermes2d/coupling.h"


ConfigComputerDialog::ConfigComputerDialog(QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon("options"));
    setWindowTitle(tr("Options"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

void ConfigComputerDialog::load()
{
    // gui style
    cmbGUIStyle->setCurrentIndex(cmbGUIStyle->findText(Agros2D::configComputer()->guiStyle));
    if (cmbGUIStyle->currentIndex() == -1 && cmbGUIStyle->count() > 0) cmbGUIStyle->setCurrentIndex(0);

    // language
    cmbLanguage->setCurrentIndex(cmbLanguage->findText(Agros2D::configComputer()->language));
    if (cmbLanguage->currentIndex() == -1 && cmbLanguage->count() > 0) cmbLanguage->setCurrentIndex(0);

    // collaboration server
    // txtCollaborationServerURL->setText(Agros2D::configComputer()->collaborationServerURL);

    // check version
    chkCheckVersion->setChecked(Agros2D::configComputer()->checkVersion);

    // show result in line edit value widget
    chkLineEditValueShowResult->setChecked(Agros2D::configComputer()->lineEditValueShowResult);

    // development
    chkDiscreteSaveMatrixRHS->setChecked(Agros2D::configComputer()->saveMatrixRHS);
    chkExperimentalFeatures->setChecked(Agros2D::configComputer()->experimentalFeatures);

    // number of threads
    txtNumOfThreads->setValue(Agros2D::configComputer()->numberOfThreads);

    // delete files
    chkDeleteTriangleMeshFiles->setChecked(Agros2D::configComputer()->deleteMeshFiles);
    chkDeleteHermes2DMeshFile->setChecked(Agros2D::configComputer()->deleteHermesMeshFile);

    // save problem with solution
    chkSaveWithSolution->setChecked(Agros2D::configComputer()->saveProblemWithSolution);

    // cache size
    txtCacheSize->setValue(Agros2D::configComputer()->cacheSize);

    // std log
    chkLogStdOut->setChecked(Agros2D::configComputer()->showLogStdOut);

    // global script
    txtGlobalScript->setPlainText(Agros2D::configComputer()->globalScript);
}

void ConfigComputerDialog::save()
{
    // gui style
    Agros2D::configComputer()->guiStyle = cmbGUIStyle->currentText();
    setGUIStyle(cmbGUIStyle->currentText());

    // language
    if (Agros2D::configComputer()->language != cmbLanguage->currentText())
        QMessageBox::warning(QApplication::activeWindow(),
                             tr("Language change"),
                             tr("Interface language has been changed. You must restart the application."));
    Agros2D::configComputer()->language = cmbLanguage->currentText();

    // collaboration server
    // QString collaborationServerUrl = txtCollaborationServerURL->text();
    // if (!collaborationServerUrl.startsWith("http://"))
    //     collaborationServerUrl = QString("http://%1").arg(collaborationServerUrl);

    // if (!collaborationServerUrl.endsWith("/"))
    //     collaborationServerUrl = QString("%1/").arg(collaborationServerUrl);

    // Agros2D::configComputer()->collaborationServerURL = collaborationServerUrl;

    // check version
    Agros2D::configComputer()->checkVersion = chkCheckVersion->isChecked();

    // show result in line edit value widget
    Agros2D::configComputer()->lineEditValueShowResult = chkLineEditValueShowResult->isChecked();

    // development
    Agros2D::configComputer()->saveMatrixRHS = chkDiscreteSaveMatrixRHS->isChecked();
    Agros2D::configComputer()->experimentalFeatures = chkExperimentalFeatures->isChecked();

    // number of threads
    Agros2D::configComputer()->numberOfThreads = txtNumOfThreads->value();

    // delete files
    Agros2D::configComputer()->deleteMeshFiles = chkDeleteTriangleMeshFiles->isChecked();
    Agros2D::configComputer()->deleteHermesMeshFile = chkDeleteHermes2DMeshFile->isChecked();

    // save problem with solution
    Agros2D::configComputer()->saveProblemWithSolution = chkSaveWithSolution->isChecked();

    // cache size
    Agros2D::configComputer()->cacheSize = txtCacheSize->value();

    // std log
    Agros2D::configComputer()->showLogStdOut = chkLogStdOut->isChecked();

    // global script
    Agros2D::configComputer()->globalScript = txtGlobalScript->toPlainText();

    // save
    Agros2D::configComputer()->save();
}

void ConfigComputerDialog::createControls()
{
    lstView = new QListWidget(this);
    pages = new QStackedWidget(this);

    panMain = createMainWidget();
    panSolver = createSolverWidget();
    panPlugin = createPluginWidget();
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

    QListWidgetItem *itemPlugin = new QListWidgetItem(icon("options-plugin"), tr("Plugins"), lstView);
    itemPlugin->setTextAlignment(Qt::AlignHCenter);
    itemPlugin->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemPlugin->setSizeHint(sizeItem);

    QListWidgetItem *itemGlobalScript = new QListWidgetItem(icon("options-python"), tr("Python"), lstView);
    itemGlobalScript->setTextAlignment(Qt::AlignHCenter);
    itemGlobalScript->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemGlobalScript->setSizeHint(sizeItem);

    pages->addWidget(panMain);
    pages->addWidget(panSolver);
    pages->addWidget(panPlugin);
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

QWidget *ConfigComputerDialog::createMainWidget()
{
    QWidget *mainWidget = new QWidget(this);

    // general
    cmbGUIStyle = new QComboBox(mainWidget);
    cmbGUIStyle->addItems(QStyleFactory::keys());

    cmbLanguage = new QComboBox(mainWidget);
    cmbLanguage->addItems(availableLanguages());

    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->addWidget(new QLabel(tr("UI:")), 0, 0);
    layoutGeneral->addWidget(cmbGUIStyle, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Language:")), 1, 0);
    layoutGeneral->addWidget(cmbLanguage, 1, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // collaboration
    // txtCollaborationServerURL = new LineEditDouble();

    // QVBoxLayout *layoutCollaboration = new QVBoxLayout();
    // layoutCollaboration->addWidget(new QLabel(tr("Collaboration server URL:")));
    // layoutCollaboration->addWidget(txtCollaborationServerURL);

    // QGroupBox *grpCollaboration = new QGroupBox(tr("Collaboration"));
    // grpCollaboration->setLayout(layoutCollaboration);

    // other
    chkLineEditValueShowResult = new QCheckBox(tr("Show value result in line edit input"));
    chkCheckVersion = new QCheckBox(tr("Check new version during startup"));

    chkLogStdOut = new QCheckBox(tr("Print application log to standard output."));

    QVBoxLayout *layoutOther = new QVBoxLayout();
    layoutOther->addWidget(chkLineEditValueShowResult);
    layoutOther->addWidget(chkCheckVersion);
    layoutOther->addWidget(chkLogStdOut);

    QGroupBox *grpOther = new QGroupBox(tr("Other"));
    grpOther->setLayout(layoutOther);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpGeneral);
    // layout->addWidget(grpCollaboration);
    layout->addWidget(grpOther);
    layout->addStretch();

    mainWidget->setLayout(layout);

    return mainWidget;
}

QWidget *ConfigComputerDialog::createSolverWidget()
{
    // general
    chkDeleteTriangleMeshFiles = new QCheckBox(tr("Delete files with initial mesh (Triangle)"));
    chkDeleteHermes2DMeshFile = new QCheckBox(tr("Delete files with solution mesh (Hermes2D)"));
    chkSaveWithSolution = new QCheckBox(tr("Save problem with solution"));
    txtCacheSize = new QSpinBox(this);
    txtCacheSize->setMinimum(2);
    txtCacheSize->setMaximum(50);

    txtNumOfThreads = new QSpinBox(this);
    txtNumOfThreads->setMinimum(1);
    txtNumOfThreads->setMaximum(omp_get_max_threads());

    QGridLayout *layoutSolver = new QGridLayout();
    layoutSolver->addWidget(chkDeleteTriangleMeshFiles, 0, 0, 1, 2);
    layoutSolver->addWidget(chkDeleteHermes2DMeshFile, 1, 0, 1, 2);
    layoutSolver->addWidget(new QLabel(tr("Number of threads:")), 2, 0);
    layoutSolver->addWidget(txtNumOfThreads, 2, 1);
    layoutSolver->addWidget(new QLabel(tr("Number of cache slots:")), 3, 0);
    layoutSolver->addWidget(txtCacheSize, 3, 1);
    layoutSolver->addWidget(chkSaveWithSolution);

    QGroupBox *grpSolver = new QGroupBox(tr("Solver"));
    grpSolver->setLayout(layoutSolver);

    chkDiscreteSaveMatrixRHS = new QCheckBox(tr("Save matrix and RHS"));
    chkExperimentalFeatures = new QCheckBox(tr("Allow experimental features"));

    QGridLayout *layoutDevelopment = new QGridLayout();
    layoutDevelopment->addWidget(chkDiscreteSaveMatrixRHS, 0, 0, 1, 2);
    layoutDevelopment->addWidget(chkExperimentalFeatures, 1, 0, 1, 2);

    QGroupBox *grpDevelopment = new QGroupBox(tr("Development"));
    grpDevelopment->setLayout(layoutDevelopment);

    QVBoxLayout *layoutGeneral = new QVBoxLayout();
    layoutGeneral->addWidget(grpSolver);
    layoutGeneral->addWidget(grpDevelopment);
    layoutGeneral->addStretch();

    QWidget *solverGeneralWidget = new QWidget(this);
    solverGeneralWidget->setLayout(layoutGeneral);

    return solverGeneralWidget;
}

QWidget *ConfigComputerDialog::createPluginWidget()
{
    QWidget *pluginWidget = new QWidget(this);

    btnBuildModule = new QPushButton(tr("Build plugin"));
    btnBuildModule->setEnabled(false);
    connect(btnBuildModule, SIGNAL(clicked()), this, SLOT(buildModule()));

    QHBoxLayout *layoutButtonsModule = new QHBoxLayout();
    layoutButtonsModule->addStretch();
    layoutButtonsModule->addWidget(btnBuildModule);

    treeModules = new QTreeWidget(this);
    treeModules->setMouseTracking(true);
    treeModules->setColumnCount(2);
    treeModules->setColumnWidth(0, 250);
    treeModules->setIndentation(20);
    QStringList headModules;
    headModules << tr("Name") << tr("Availability");
    treeModules->setHeaderLabels(headModules);

    // connect(treeModules, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(moduleDoubleClicked(QTreeWidgetItem *, int)));
    connect(treeModules, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(moduleClicked(QTreeWidgetItem *, int)));
    connect(treeModules, SIGNAL(itemPressed(QTreeWidgetItem *, int)), this, SLOT(moduleClicked(QTreeWidgetItem *, int)));

    // module
    QVBoxLayout *layoutModules = new QVBoxLayout();
    layoutModules->addWidget(treeModules);
    layoutModules->addLayout(layoutButtonsModule);

    QGroupBox *grpModules = new QGroupBox(tr("Modules"));
    grpModules->setLayout(layoutModules);

    btnBuildCoupling = new QPushButton(tr("Build plugin"));
    btnBuildCoupling->setEnabled(false);
    connect(btnBuildCoupling, SIGNAL(clicked()), this, SLOT(buildCoupling()));

    QHBoxLayout *layoutButtonsCoupling = new QHBoxLayout();
    layoutButtonsCoupling->addStretch();
    layoutButtonsCoupling->addWidget(btnBuildCoupling);

    treeCouplings = new QTreeWidget(this);
    treeCouplings->setMouseTracking(true);
    treeCouplings->setColumnCount(2);
    treeCouplings->setColumnWidth(0, 250);
    treeCouplings->setIndentation(5);
    QStringList headCouplings;
    headCouplings << tr("Name") << tr("Availability");
    treeCouplings->setHeaderLabels(headCouplings);

    connect(treeCouplings, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(couplingDoubleClicked(QTreeWidgetItem *, int)));
    connect(treeCouplings, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(couplingClicked(QTreeWidgetItem *, int)));
    connect(treeCouplings, SIGNAL(itemPressed(QTreeWidgetItem *, int)), this, SLOT(couplingClicked(QTreeWidgetItem *, int)));

    // coupling
    QVBoxLayout *layoutCouplings = new QVBoxLayout();
    layoutCouplings->addWidget(treeCouplings);
    layoutCouplings->addLayout(layoutButtonsCoupling);

    QGroupBox *grpCouplings = new QGroupBox(tr("Couplings"));
    grpCouplings ->setLayout(layoutCouplings);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpModules, 3);
    layout->addWidget(grpCouplings, 2);
    layout->addStretch();

    pluginWidget->setLayout(layout);

    readModulesAndCouplings();

    return pluginWidget;
}

void ConfigComputerDialog::moduleClicked(QTreeWidgetItem *item, int role)
{
    btnBuildModule->setEnabled(item);
}

void ConfigComputerDialog::moduleDoubleClicked(QTreeWidgetItem *item, int role)
{
    QString module = item->data(0, Qt::UserRole).toString();
    if (!module.isEmpty())
    {
        ModuleDialog moduleDialog(module, this);
        moduleDialog.exec();

        readModulesAndCouplings();
    }
}

void ConfigComputerDialog::couplingClicked(QTreeWidgetItem *item, int role)
{
    btnBuildCoupling->setEnabled(item);
}

void ConfigComputerDialog::couplingDoubleClicked(QTreeWidgetItem *item, int role)
{
    QString module = item->data(0, Qt::UserRole).toString();
    if (!module.isEmpty())
    {
        // TODO: implement
    }
}

void ConfigComputerDialog::buildModule()
{
    if (treeModules->selectedItems().count() == 1)
    {
        QString module = treeModules->selectedItems()[0]->data(0, Qt::UserRole).toString();
        if (!module.isEmpty())
        {
            buildModuleOrCoupling(module);
        }
    }
}

void ConfigComputerDialog::buildCoupling()
{
    if (treeCouplings->selectedItems().count() == 1)
    {
        QString coupling = treeCouplings->selectedItems()[0]->data(0, Qt::UserRole).toString();
        if (!coupling.isEmpty())
        {
            buildModuleOrCoupling(coupling);
        }
    }
}

void ConfigComputerDialog::buildModuleOrCoupling(const QString &id)
{
    SystemOutputWidget *output = new SystemOutputWidget();
    output->execute(QString(COMMANDS_BUILD_PLUGIN).arg(id));

    readModulesAndCouplings();
}

void ConfigComputerDialog::readModulesAndCouplings()
{
    treeModules->clear();
    QMapIterator<QString, QString> itModules(Module::availableModules());
    while (itModules.hasNext())
    {
        itModules.next();

        QTreeWidgetItem *item = new QTreeWidgetItem(treeModules);
        item->setText(0, itModules.value());
        item->setData(0, Qt::UserRole, itModules.key());

        try
        {
            foreach (QString plugin, QStringList(itModules.key()))
                Agros2D::loadPlugin(plugin);

            item->setText(1, tr("available"));
        }
        catch (AgrosPluginException e)
        {
            item->setText(1, tr("missing"));
            item->setForeground(1, QBrush(Qt::red));
        }
    }

    treeCouplings->clear();
    QMapIterator<QString, QString> itCouplings(availableCouplings());
    while (itCouplings.hasNext())
    {
        itCouplings.next();

        QTreeWidgetItem *item = new QTreeWidgetItem(treeCouplings);
        item->setText(0, itCouplings.value());
        item->setData(0, Qt::UserRole, itCouplings.key());

        try
        {
            foreach (QString plugin, QStringList(itCouplings.key()))
                Agros2D::loadPlugin(plugin);

            item->setText(1, tr("available"));
        }
        catch (AgrosPluginException e)
        {
            item->setText(1, tr("missing"));
            item->setForeground(1, QBrush(Qt::red));
        }
    }
}

QWidget *ConfigComputerDialog::createGlobalScriptWidget()
{
    QWidget *viewWidget = new QWidget(this);

    txtGlobalScript = new ScriptEditor(currentPythonEngine(), this);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(txtGlobalScript);

    viewWidget->setLayout(layout);

    return viewWidget;
}

void ConfigComputerDialog::fillComboBoxPhysicField(QComboBox *cmbPhysicField)
{
    // block signals
    cmbPhysicField->blockSignals(true);

    cmbPhysicField->clear();
    QMapIterator<QString, QString> it(Module::availableModules());
    while (it.hasNext())
    {
        it.next();
        cmbPhysicField->addItem(it.value(), it.key());
    }

    // unblock signals
    cmbPhysicField->blockSignals(false);

    // physic field
    if (cmbPhysicField->currentIndex() == -1)
        cmbPhysicField->setCurrentIndex(0);
}

void ConfigComputerDialog::doCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    pages->setCurrentIndex(lstView->row(current));
}

void ConfigComputerDialog::doAccept()
{
    save();
    accept();
}

void ConfigComputerDialog::doReject()
{
    reject();
}

