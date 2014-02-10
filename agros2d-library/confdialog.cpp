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
#include "gui/common.h"

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
    cmbGUIStyle->setCurrentIndex(cmbGUIStyle->findText(Agros2D::configComputer()->value(Config::Config_GUIStyle).toString()));
    if (cmbGUIStyle->currentIndex() == -1 && cmbGUIStyle->count() > 0) cmbGUIStyle->setCurrentIndex(0);

    // language
    cmbLanguage->setCurrentIndex(cmbLanguage->findText(Agros2D::configComputer()->value(Config::Config_Locale).toString()));
    if (cmbLanguage->currentIndex() == -1 && cmbLanguage->count() > 0)
        cmbLanguage->setCurrentIndex(0);

    // show result in line edit value widget
    chkLineEditValueShowResult->setChecked(Agros2D::configComputer()->value(Config::Config_ShowResults).toBool());

    // development
    chkDiscreteSaveMatrixRHS->setChecked(Agros2D::configComputer()->value(Config::Config_LinearSystemSave).toBool());
    cmbDumpFormat->setCurrentIndex(cmbDumpFormat->findData((Hermes::Algebra::MatrixExportFormat) Agros2D::configComputer()->value(Config::Config_LinearSystemFormat).toInt(), Qt::UserRole));

    // number of threads
    txtNumOfThreads->setValue(Agros2D::configComputer()->value(Config::Config_NumberOfThreads).toInt());

    // cache size
    txtCacheSize->setValue(Agros2D::configComputer()->value(Config::Config_CacheSize).toInt());

    // std log
    chkLogStdOut->setChecked(Agros2D::configComputer()->value(Config::Config_LogStdOut).toBool());

    // workspace
    chkShowGrid->setChecked(Agros2D::configComputer()->value(Config::Config_ShowGrid).toBool());
    chkShowAxes->setChecked(Agros2D::configComputer()->value(Config::Config_ShowAxes).toBool());
    chkShowRulers->setChecked(Agros2D::configComputer()->value(Config::Config_ShowRulers).toBool());

    cmbRulersFont->setCurrentIndex(cmbRulersFont->findData(Agros2D::configComputer()->value(Config::Config_RulersFontFamily).toString()));
    txtRulersFontSizes->setValue(Agros2D::configComputer()->value(Config::Config_RulersFontPointSize).toInt());
    cmbPostFont->setCurrentIndex(cmbPostFont->findData(Agros2D::configComputer()->value(Config::Config_PostFontFamily).toString()));
    txtPostFontSizes->setValue(Agros2D::configComputer()->value(Config::Config_PostFontPointSize).toInt());
}

void ConfigComputerDialog::save()
{
    // gui style
    Agros2D::configComputer()->setValue(Config::Config_GUIStyle, cmbGUIStyle->currentText());
    setGUIStyle(cmbGUIStyle->currentText());

    // language
    if (Agros2D::configComputer()->value(Config::Config_Locale).toString() != cmbLanguage->currentText())
        QMessageBox::warning(QApplication::activeWindow(),
                             tr("Language change"),
                             tr("Interface language has been changed. You must restart the application."));
    Agros2D::configComputer()->setValue(Config::Config_Locale, cmbLanguage->currentText());

    // show result in line edit value widget
    Agros2D::configComputer()->setValue(Config::Config_ShowResults, chkLineEditValueShowResult->isChecked());

    // development
    Agros2D::configComputer()->setValue(Config::Config_LinearSystemSave, chkDiscreteSaveMatrixRHS->isChecked());
    Agros2D::configComputer()->setValue(Config::Config_LinearSystemFormat, (Hermes::Algebra::MatrixExportFormat) cmbDumpFormat->itemData(cmbDumpFormat->currentIndex(), Qt::UserRole).toInt());

    // number of threads
    Agros2D::configComputer()->setValue(Config::Config_NumberOfThreads, txtNumOfThreads->value());

    // cache size
    Agros2D::configComputer()->setValue(Config::Config_CacheSize, txtCacheSize->value());

    // std log
    Agros2D::configComputer()->setValue(Config::Config_LogStdOut, chkLogStdOut->isChecked());

    // workspace
    Agros2D::configComputer()->setValue(Config::Config_ShowGrid, chkShowGrid->isChecked());
    Agros2D::configComputer()->setValue(Config::Config_ShowRulers, chkShowRulers->isChecked());
    Agros2D::configComputer()->setValue(Config::Config_ShowAxes, chkShowAxes->isChecked());

    Agros2D::configComputer()->setValue(Config::Config_RulersFontFamily, cmbRulersFont->itemData(cmbRulersFont->currentIndex()).toString());
    Agros2D::configComputer()->setValue(Config::Config_RulersFontPointSize, txtRulersFontSizes->value());
    Agros2D::configComputer()->setValue(Config::Config_PostFontFamily, cmbPostFont->itemData(cmbPostFont->currentIndex()).toString());
    Agros2D::configComputer()->setValue(Config::Config_PostFontPointSize, txtPostFontSizes->value());

    // save
    Agros2D::configComputer()->save();
}

void ConfigComputerDialog::createControls()
{
    lstView = new QListWidget(this);
    pages = new QStackedWidget(this);

    panMain = createMainWidget();
    panSolver = createSolverWidget();

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

    pages->addWidget(panMain);
    pages->addWidget(panSolver);

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

    // other
    chkLineEditValueShowResult = new QCheckBox(tr("Show value result in line edit input"));

    chkLogStdOut = new QCheckBox(tr("Print application log to standard output."));

    QVBoxLayout *layoutOther = new QVBoxLayout();
    layoutOther->addWidget(chkLineEditValueShowResult);
    layoutOther->addWidget(chkLogStdOut);

    QGroupBox *grpOther = new QGroupBox(tr("Other"));
    grpOther->setLayout(layoutOther);

    // workspace
    chkShowGrid = new QCheckBox(tr("Show grid"));
    chkShowRulers = new QCheckBox(tr("Show rulers"));
    chkShowAxes = new QCheckBox(tr("Show axes"));

    QGridLayout *layoutGrid = new QGridLayout();
    layoutGrid->addWidget(chkShowGrid, 0, 0);
    layoutGrid->addWidget(chkShowAxes, 1, 0);
    layoutGrid->addWidget(chkShowRulers, 2, 0);

    QGroupBox *grpGrid = new QGroupBox(tr("Grid"));
    grpGrid->setLayout(layoutGrid);

    cmbRulersFont = new QComboBox();
    fillComboBoxFonts(cmbRulersFont);
    txtRulersFontSizes = new QSpinBox();
    txtRulersFontSizes->setMinimum(6);
    txtRulersFontSizes->setMaximum(40);

    cmbPostFont = new QComboBox();
    fillComboBoxFonts(cmbPostFont);
    txtPostFontSizes = new QSpinBox();
    txtPostFontSizes->setMinimum(6);
    txtPostFontSizes->setMaximum(40);

    QGridLayout *layoutFont = new QGridLayout();
    layoutFont->setColumnStretch(1, 1);
    layoutFont->addWidget(new QLabel(tr("Rulers:")), 0, 0);
    layoutFont->addWidget(cmbRulersFont, 0, 1);
    layoutFont->addWidget(txtRulersFontSizes, 0, 2);
    layoutFont->addWidget(new QLabel(tr("Postprocessor:")), 2, 0);
    layoutFont->addWidget(cmbPostFont, 2, 1);
    layoutFont->addWidget(txtPostFontSizes, 2, 2);

    QGroupBox *grpFont = new QGroupBox(tr("Fonts"));
    grpFont->setLayout(layoutFont);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpGeneral);
    layout->addWidget(grpOther);
    layout->addWidget(grpGrid);
    layout->addWidget(grpFont);
    layout->addStretch();

    mainWidget->setLayout(layout);

    return mainWidget;
}

QWidget *ConfigComputerDialog::createSolverWidget()
{
    // general
    txtCacheSize = new QSpinBox(this);
    txtCacheSize->setMinimum(2);
    txtCacheSize->setMaximum(50);

    txtNumOfThreads = new QSpinBox(this);
    txtNumOfThreads->setMinimum(1);
    txtNumOfThreads->setMaximum(omp_get_max_threads());

    QGridLayout *layoutSolver = new QGridLayout();
    layoutSolver->addWidget(new QLabel(tr("Number of threads:")), 0, 0);
    layoutSolver->addWidget(txtNumOfThreads, 0, 1);
    layoutSolver->addWidget(new QLabel(tr("Number of cache slots:")), 1, 0);
    layoutSolver->addWidget(txtCacheSize, 1, 1);

    QGroupBox *grpSolver = new QGroupBox(tr("Solver"));
    grpSolver->setLayout(layoutSolver);

    chkDiscreteSaveMatrixRHS = new QCheckBox(tr("Save matrix and RHS"));
    cmbDumpFormat = new QComboBox(this);
    cmbDumpFormat->addItem(dumpFormatString(EXPORT_FORMAT_PLAIN_ASCII), EXPORT_FORMAT_PLAIN_ASCII);
    cmbDumpFormat->addItem(dumpFormatString(EXPORT_FORMAT_MATLAB_MATIO), EXPORT_FORMAT_MATLAB_MATIO);
    // cmbDumpFormat->addItem(dumpFormatString(DF_MATRIX_MARKET), DF_MATRIX_MARKET);

    QGridLayout *layoutDevelopment = new QGridLayout();
    layoutDevelopment->addWidget(chkDiscreteSaveMatrixRHS, 0, 0, 1, 2);
    layoutDevelopment->addWidget(new QLabel(tr("Matrix format")), 1, 0);
    layoutDevelopment->addWidget(cmbDumpFormat, 1, 1);

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

