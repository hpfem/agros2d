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

#include "moduledialog.h"

#include "gui.h"

#include "scene.h"
#include "sceneview_common.h"
#include "pythonlabagros.h"
#include "hermes2d/module.h"

ModuleItemDialog::ModuleItemDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowIcon(icon(""));
    setWindowTitle(tr("Item editor"));

    txtId = new QLineEdit();
    txtName = new QLineEdit();
    txtShortname = new QLineEdit();
    txtShortnameHtml = new QLineEdit();
    txtShortnameLatex = new QLineEdit();
    txtUnit = new QLineEdit();
    txtUnitHtml = new QLineEdit();
    txtUnitLatex = new QLineEdit();

    QGridLayout *layoutGeneral = new QGridLayout();

    layoutGeneral->addWidget(new QLabel(tr("ID:")), 0, 0);
    layoutGeneral->addWidget(txtId, 0, 1, 1, 3);
    layoutGeneral->addWidget(new QLabel(tr("Name:")), 1, 0);
    layoutGeneral->addWidget(txtName, 1, 1, 1, 3);
    layoutGeneral->addWidget(new QLabel(tr("Shortname:")), 2, 0);
    layoutGeneral->addWidget(txtShortname, 2, 1);
    layoutGeneral->addWidget(new QLabel(tr("Shortname (HTML):")), 3, 0);
    layoutGeneral->addWidget(txtShortnameHtml, 3, 1);
    layoutGeneral->addWidget(new QLabel(tr("Shortname (LaTeX):")), 4, 0);
    layoutGeneral->addWidget(txtShortnameLatex, 4, 1);
    layoutGeneral->addWidget(new QLabel(tr("Unit:")), 2, 2);
    layoutGeneral->addWidget(txtUnit, 2, 3);
    layoutGeneral->addWidget(new QLabel(tr("Unit (HTML):")), 3, 2);
    layoutGeneral->addWidget(txtUnitHtml, 3, 3);
    layoutGeneral->addWidget(new QLabel(tr("Unit (LaTeX):")), 4, 2);
    layoutGeneral->addWidget(txtUnitLatex, 4, 3);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpGeneral);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMinimumSize(sizeHint());
}

// ***********************************************************************************************************************

ModuleItemLocalValueDialog::ModuleItemLocalValueDialog(XMLModule::localvariable *lv, QWidget *parent)
    : ModuleItemDialog(parent), m_lv(lv)
{
    txtId->setText(QString::fromStdString(lv->id()));
    txtName->setText(QString::fromStdString(lv->name()));
    txtShortname->setText(QString::fromStdString(lv->shortname()));
    if (lv->shortname_html().present())
        txtShortnameHtml->setText(QString::fromStdString(lv->shortname_html().get()));
    if (lv->shortname_latex().present())
        txtShortnameLatex->setText(QString::fromStdString(lv->shortname_latex().get()));
    txtUnit->setText(QString::fromStdString(lv->unit()));
    if (lv->unit_html().present())
        txtUnitHtml->setText(QString::fromStdString(lv->unit_html().get()));
    if (lv->unit_latex().present())
        txtUnitLatex->setText(QString::fromStdString(lv->unit_latex().get()));
}

void ModuleItemLocalValueDialog::doAccept()
{
    m_lv->id(txtId->text().toStdString());
    m_lv->name(txtName->text().toStdString());
    m_lv->shortname(txtShortname->text().toStdString());
    m_lv->shortname_html(txtShortnameHtml->text().toStdString());
    m_lv->shortname_latex(txtShortnameLatex->text().toStdString());
    m_lv->unit(txtUnit->text().toStdString());
    m_lv->unit_html(txtUnitHtml->text().toStdString());
    m_lv->unit_latex(txtUnitLatex->text().toStdString());

    accept();
}

// ***********************************************************************************************************************

ModuleVolumeIntegralValueDialog::ModuleVolumeIntegralValueDialog(XMLModule::volumeintegral *vol, QWidget *parent)
    : ModuleItemDialog(parent), m_vol(vol)
{
    txtId->setText(QString::fromStdString(vol->id()));
    txtName->setText(QString::fromStdString(vol->name()));
    txtShortname->setText(QString::fromStdString(vol->shortname()));
    if (vol->shortname_html().present())
        txtShortnameHtml->setText(QString::fromStdString(vol->shortname_html().get()));
    if (vol->shortname_latex().present())
        txtShortnameLatex->setText(QString::fromStdString(vol->shortname_latex().get()));
    txtUnit->setText(QString::fromStdString(vol->unit()));
    if (vol->unit_html().present())
        txtUnitHtml->setText(QString::fromStdString(vol->unit_html().get()));
    if (vol->unit_latex().present())
        txtUnitLatex->setText(QString::fromStdString(vol->unit_latex().get()));
}

void ModuleVolumeIntegralValueDialog::doAccept()
{
    m_vol->id(txtId->text().toStdString());
    m_vol->name(txtName->text().toStdString());
    m_vol->shortname(txtShortname->text().toStdString());
    m_vol->shortname_html(txtShortnameHtml->text().toStdString());
    m_vol->shortname_latex(txtShortnameLatex->text().toStdString());
    m_vol->unit(txtUnit->text().toStdString());
    m_vol->unit_html(txtUnitHtml->text().toStdString());
    m_vol->unit_latex(txtUnitLatex->text().toStdString());

    accept();
}

// ***********************************************************************************************************************

ModuleSurfaceIntegralValueDialog::ModuleSurfaceIntegralValueDialog(XMLModule::surfaceintegral *sur, QWidget *parent)
    : ModuleItemDialog(parent), m_sur(sur)
{
    txtId->setText(QString::fromStdString(sur->id()));
    txtName->setText(QString::fromStdString(sur->name()));
    txtShortname->setText(QString::fromStdString(sur->shortname()));
    if (sur->shortname_html().present())
        txtShortnameHtml->setText(QString::fromStdString(sur->shortname_html().get()));
    if (sur->shortname_latex().present())
        txtShortnameLatex->setText(QString::fromStdString(sur->shortname_latex().get()));
    txtUnit->setText(QString::fromStdString(sur->unit()));
    if (sur->unit_html().present())
        txtUnitHtml->setText(QString::fromStdString(sur->unit_html().get()));
    if (sur->unit_latex().present())
        txtUnitLatex->setText(QString::fromStdString(sur->unit_latex().get()));
}

void ModuleSurfaceIntegralValueDialog::doAccept()
{
    m_sur->id(txtId->text().toStdString());
    m_sur->name(txtName->text().toStdString());
    m_sur->shortname(txtShortname->text().toStdString());
    m_sur->shortname_html(txtShortnameHtml->text().toStdString());
    m_sur->shortname_latex(txtShortnameLatex->text().toStdString());
    m_sur->unit(txtUnit->text().toStdString());
    m_sur->unit_html(txtUnitHtml->text().toStdString());
    m_sur->unit_latex(txtUnitLatex->text().toStdString());

    accept();
}

// ***********************************************************************************************************************

ModuleDialog::ModuleDialog(const QString &fieldId, QWidget *parent)
    : QDialog(parent), m_fieldId(fieldId)
{
    setWindowIcon(icon(""));
    setWindowTitle(tr("Module editor"));

    createControls();

    m_module_xsd = XMLModule::module_((datadir().toStdString() + MODULEROOT.toStdString() + "/" + m_fieldId.toStdString() + ".xml").c_str());

    load();

    setMinimumSize(QSize(800, 600));
}

void ModuleDialog::load()
{
    XMLModule::module *module = m_module_xsd.get();

    // main
    txtId->setText(QString::fromStdString(module->general().id()));
    txtName->setText(QString::fromStdString(module->general().name()));
    txtDescription->setPlainText(QString::fromStdString(module->general().description()));

    // local values
    treeLocalVariables->clear();
    for (int i = 0; i < module->postprocessor().localvariables().localvariable().size(); i++)
    {
        XMLModule::localvariable lv = module->postprocessor().localvariables().localvariable().at(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(treeLocalVariables);

        item->setData(0, Qt::UserRole, QString::fromStdString(lv.id()));
        item->setText(0, QString::fromStdString(lv.name()));
        item->setText(1, QString::fromStdString(lv.shortname()));
        item->setText(2, QString::fromStdString(lv.type()));

        QString analyses;
        for (int j = 0; j < lv.expression().size(); j++)
        {
            XMLModule::expression expr = lv.expression().at(j);
            analyses += QString("%1, ").arg(QString::fromStdString(expr.analysistype()));
        }
        if (analyses.length() > 0)
            analyses = analyses.left(analyses.length() - 2);
        item->setText(3, analyses);
    }

    // volume integrals
    treeVolumeIntegrals->clear();
    for (int i = 0; i < module->postprocessor().volumeintegrals().volumeintegral().size(); i++)
    {
        XMLModule::volumeintegral vol = module->postprocessor().volumeintegrals().volumeintegral().at(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(treeVolumeIntegrals);

        item->setData(0, Qt::UserRole, QString::fromStdString(vol.id()));
        item->setText(0, QString::fromStdString(vol.name()));
        item->setText(1, QString::fromStdString(vol.shortname()));

        QString analyses;
        for (int j = 0; j < vol.expression().size(); j++)
        {
            XMLModule::expression expr = vol.expression().at(j);
            analyses += QString("%1, ").arg(QString::fromStdString(expr.analysistype()));
        }
        if (analyses.length() > 0)
            analyses = analyses.left(analyses.length() - 2);
        item->setText(2, analyses);
    }

    // surface integrals
    treeSurfaceIntegrals->clear();
    for (int i = 0; i < module->postprocessor().surfaceintegrals().surfaceintegral().size(); i++)
    {
        XMLModule::surfaceintegral sur = module->postprocessor().surfaceintegrals().surfaceintegral().at(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(treeSurfaceIntegrals);

        item->setData(0, Qt::UserRole, QString::fromStdString(sur.id()));
        item->setText(0, QString::fromStdString(sur.name()));
        item->setText(1, QString::fromStdString(sur.shortname()));

        QString analyses;
        for (int j = 0; j < sur.expression().size(); j++)
        {
            XMLModule::expression expr = sur.expression().at(j);
            analyses += QString("%1, ").arg(QString::fromStdString(expr.analysistype()));
        }
        if (analyses.length() > 0)
            analyses = analyses.left(analyses.length() - 2);
        item->setText(2, analyses);
    }
}

void ModuleDialog::save()
{
    try
    {
        std::string mesh_schema_location("");
        mesh_schema_location.append("../xsd/module_xml.xsd");
        ::xml_schema::namespace_info namespace_info_mesh("XMLModule", mesh_schema_location);

        ::xml_schema::namespace_infomap namespace_info_map;
        namespace_info_map.insert(std::pair<std::basic_string<char>, xml_schema::namespace_info>("module", namespace_info_mesh));

        XMLModule::module *module = m_module_xsd.get();
        std::ofstream out((datadir().toStdString() + MODULEROOT.toStdString() + "/" + m_fieldId.toStdString() + ".xml").c_str());
        XMLModule::module_(out, *module, namespace_info_map);
    }
    catch (const xml_schema::exception& e)
    {
        std::cerr << e << std::endl;
    }
}

void ModuleDialog::createControls()
{
    lstView = new QListWidget(this);
    pages = new QStackedWidget(this);

    panMain = createMainWidget();
    panWeakforms = createWeakforms();
    panPreprocessor = createPreprocessor();
    panPostprocessor = createPostprocessor();

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

    QListWidgetItem *itemWeakforms = new QListWidgetItem(icon("options-main"), tr("Weakforms"), lstView);
    itemWeakforms->setTextAlignment(Qt::AlignHCenter);
    itemWeakforms->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemWeakforms->setSizeHint(sizeItem);

    QListWidgetItem *itemPreprocessor = new QListWidgetItem(icon("options-main"), tr("Preprocessor"), lstView);
    itemPreprocessor->setTextAlignment(Qt::AlignHCenter);
    itemPreprocessor->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemPreprocessor->setSizeHint(sizeItem);

    QListWidgetItem *itemPostprocessor = new QListWidgetItem(icon("options-main"), tr("Postprocessor"), lstView);
    itemPostprocessor->setTextAlignment(Qt::AlignHCenter);
    itemPostprocessor->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemPostprocessor->setSizeHint(sizeItem);

    pages->addWidget(panMain);
    pages->addWidget(panWeakforms);
    pages->addWidget(panPreprocessor);
    pages->addWidget(panPostprocessor);

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

QWidget *ModuleDialog::createMainWidget()
{
    QWidget *mainWidget = new QWidget(this);

    txtId = new QLineEdit();
    txtName = new QLineEdit();
    txtDescription = new QPlainTextEdit();

    QGridLayout *layoutGeneral = new QGridLayout();

    layoutGeneral->addWidget(new QLabel(tr("ID:")), 0, 0);
    layoutGeneral->addWidget(txtId, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Name:")), 1, 0);
    layoutGeneral->addWidget(txtName, 1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Description:")), 2, 0);
    layoutGeneral->addWidget(txtDescription, 3, 0, 1, 2);
    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpGeneral);
    layout->addStretch();

    mainWidget->setLayout(layout);

    return mainWidget;
}

QWidget *ModuleDialog::createWeakforms()
{
    QVBoxLayout *layoutVolume = new QVBoxLayout();
    layoutVolume->addStretch();

    QWidget *weakformVolume = new QWidget(this);
    weakformVolume->setLayout(layoutVolume);

    QVBoxLayout *layoutSurface = new QVBoxLayout();
    layoutSurface->addWidget(new QLabel(tr("Surface")));
    layoutSurface->addStretch();

    QWidget *weakformSurface = new QWidget(this);
    weakformSurface->setLayout(layoutSurface);

    QTabWidget *weakformWidget = new QTabWidget(this);
    weakformWidget->addTab(weakformVolume, icon(""), tr("Volume"));
    weakformWidget->addTab(weakformSurface, icon(""), tr("Surface"));

    return weakformWidget;
}

QWidget *ModuleDialog::createPreprocessor()
{
    QWidget *viewWidget = new QWidget(this);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    // layout->addWidget(txtGlobalScript);

    viewWidget->setLayout(layout);

    return viewWidget;
}

QWidget *ModuleDialog::createPostprocessor()
{
    // local variables
    treeLocalVariables = new QTreeWidget(this);
    treeLocalVariables->setMouseTracking(true);
    treeLocalVariables->setColumnCount(4);
    treeLocalVariables->setColumnWidth(0, 200);
    treeLocalVariables->setIndentation(5);
    QStringList headLocalVariables;
    headLocalVariables << tr("Name") << tr("Shortcut") << tr("Type") << tr("Analyses");
    treeLocalVariables->setHeaderLabels(headLocalVariables);

    connect(treeLocalVariables, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(localItemDoubleClicked(QTreeWidgetItem *, int)));

    QVBoxLayout *layoutLocalVariables = new QVBoxLayout();
    layoutLocalVariables->addWidget(treeLocalVariables);
    // layoutLocalVariables->addStretch();

    QWidget *localVariables = new QWidget(this);
    localVariables->setLayout(layoutLocalVariables);

    // volume integrals
    treeVolumeIntegrals = new QTreeWidget(this);
    treeVolumeIntegrals->setMouseTracking(true);
    treeVolumeIntegrals->setColumnCount(3);
    treeVolumeIntegrals->setColumnWidth(0, 200);
    treeVolumeIntegrals->setIndentation(5);
    QStringList headVolume;
    headVolume << tr("Name") << tr("Shortcut") << tr("Analyses");
    treeVolumeIntegrals->setHeaderLabels(headVolume);

    connect(treeVolumeIntegrals, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(volumeIntegralDoubleClicked(QTreeWidgetItem *, int)));

    QVBoxLayout *layoutVolume = new QVBoxLayout();
    layoutVolume->addWidget(treeVolumeIntegrals);
    // layoutVolume->addStretch();

    QWidget *volumeIntegrals = new QWidget(this);
    volumeIntegrals->setLayout(layoutVolume);

    // surface integrals
    treeSurfaceIntegrals = new QTreeWidget(this);
    treeSurfaceIntegrals->setMouseTracking(true);
    treeSurfaceIntegrals->setColumnCount(3);
    treeSurfaceIntegrals->setColumnWidth(0, 200);
    treeSurfaceIntegrals->setIndentation(5);
    QStringList headSurface;
    headSurface << tr("Name") << tr("Shortcut") << tr("Analyses");
    treeSurfaceIntegrals->setHeaderLabels(headSurface);

    connect(treeSurfaceIntegrals, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(surfaceIntegralDoubleClicked(QTreeWidgetItem *, int)));

    QVBoxLayout *layoutSurface = new QVBoxLayout();
    layoutSurface->addWidget(treeSurfaceIntegrals);
    // layoutSurface->addStretch();

    QWidget *surfaceIntegrals = new QWidget(this);
    surfaceIntegrals->setLayout(layoutSurface);

    QTabWidget *postprocessorWidget = new QTabWidget(this);
    postprocessorWidget->addTab(localVariables, icon(""), tr("Local variables"));
    postprocessorWidget->addTab(volumeIntegrals, icon(""), tr("Volume integrals"));
    postprocessorWidget->addTab(surfaceIntegrals, icon(""), tr("Surface integrals"));

    return postprocessorWidget;
}

void ModuleDialog::doCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    pages->setCurrentIndex(lstView->row(current));
}

void ModuleDialog::doAccept()
{
    save();
    accept();
}

void ModuleDialog::doReject()
{
    reject();
}

void ModuleDialog::localItemDoubleClicked(QTreeWidgetItem *item, int role)
{
    XMLModule::module *module = m_module_xsd.get();
    for (int i = 0; i < module->postprocessor().localvariables().localvariable().size(); i++)
    {
        XMLModule::localvariable *lv = &module->postprocessor().localvariables().localvariable().at(i);
        if (item->data(0, Qt::UserRole).toString().toStdString() == lv->id())
        {
            ModuleItemLocalValueDialog dialog(lv, this);
            if (dialog.exec())
            {
                item->setText(0, QString::fromStdString(lv->name()));
                item->setText(1, QString::fromStdString(lv->shortname()));
                item->setText(2, QString::fromStdString(lv->type()));
            }
        }
    }
}

void ModuleDialog::volumeIntegralDoubleClicked(QTreeWidgetItem *item, int role)
{
    XMLModule::module *module = m_module_xsd.get();
    for (int i = 0; i < module->postprocessor().volumeintegrals().volumeintegral().size(); i++)
    {
        XMLModule::volumeintegral *vol = &module->postprocessor().volumeintegrals().volumeintegral().at(i);
        if (item->data(0, Qt::UserRole).toString().toStdString() == vol->id())
        {
            ModuleVolumeIntegralValueDialog dialog(vol, this);
            if (dialog.exec())
            {
                item->setText(0, QString::fromStdString(vol->name()));
                item->setText(1, QString::fromStdString(vol->shortname()));
            }
        }
    }
}

void ModuleDialog::surfaceIntegralDoubleClicked(QTreeWidgetItem *item, int role)
{
    XMLModule::module *module = m_module_xsd.get();
    for (int i = 0; i < module->postprocessor().surfaceintegrals().surfaceintegral().size(); i++)
    {
        XMLModule::surfaceintegral *sur = &module->postprocessor().surfaceintegrals().surfaceintegral().at(i);
        if (item->data(0, Qt::UserRole).toString().toStdString() == sur->id())
        {
            ModuleSurfaceIntegralValueDialog dialog(sur, this);
            if (dialog.exec())
            {
                item->setText(0, QString::fromStdString(sur->name()));
                item->setText(1, QString::fromStdString(sur->shortname()));
            }
        }
    }
}
