
// ***********************************************************************************************************************************************

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

#include "materialbrowserdialog.h"

#include "util/constants.h"
#include "util/global.h"
#include "gui/lineeditdouble.h"

#include "scene.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "moduledialog.h"

#include "gui/lineeditdouble.h"
#include "gui/common.h"

#include "hermes2d/module.h"

#include "ctemplate/template.h"
#include "qcustomplot/qcustomplot.h"

#include "pythonlab/pythonengine_agros.h"

MaterialEditDialog::MaterialEditDialog(const QString &fileName, QWidget *parent) : QDialog(parent),
    m_fileName(fileName)
{
    setWindowIcon(icon(""));
    setWindowTitle(tr("Material editor"));

    createControls();

    readMaterial();

    QSettings settings;
    restoreGeometry(settings.value("MaterialEditDialog/Geometry", saveGeometry()).toByteArray());
}

MaterialEditDialog::~MaterialEditDialog()
{
    QSettings settings;
    settings.setValue("MaterialEditDialog/Geometry", saveGeometry());
}

int MaterialEditDialog::showDialog()
{
    return exec();
}

void MaterialEditDialog::createControls()
{    
    lstProperties = new QListWidget(this);
    lstProperties->setMouseTracking(true);
    lstProperties->setMaximumWidth(200);

    connect(lstProperties, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(doPropertyChanged(QListWidgetItem *, QListWidgetItem *)));

    txtName = new QLineEdit();
    txtDescription = new QLineEdit();

    QGridLayout *layoutNameAndDescription = new QGridLayout();
    layoutNameAndDescription->addWidget(new QLabel(tr("Name:")), 0, 0);
    layoutNameAndDescription->addWidget(txtName, 0, 1);
    layoutNameAndDescription->addWidget(new QLabel(tr("Description:")), 1, 0);
    layoutNameAndDescription->addWidget(txtDescription, 2, 0, 1, 3);

    QPushButton *btnAddProperty = new QPushButton(tr("Add..."));
    btnAddProperty->setDefault(false);

    QAction *actAddCustom = new QAction(tr("Custom property"), this);
    connect(actAddCustom, SIGNAL(triggered()), this, SLOT(addProperty()));

    // TODO: more general
    QAction *actAddThermalConductivity = new QAction(tr("Thermal conductivity"), this);
    connect(actAddThermalConductivity, SIGNAL(triggered()), this, SLOT(addPropertyThermalConductivity()));
    QAction *actAddSpecificHeat = new QAction(tr("Specific heat"), this);
    connect(actAddSpecificHeat, SIGNAL(triggered()), this, SLOT(addPropertySpecificHeat()));
    QAction *actAddDensity = new QAction(tr("Density"), this);
    connect(actAddDensity, SIGNAL(triggered()), this, SLOT(addPropertyDensity()));
    QAction *actAddMagneticPermeability = new QAction(tr("Magnetic permeability"), this);
    connect(actAddMagneticPermeability, SIGNAL(triggered()), this, SLOT(addPropertyMagneticPermeability()));

    QMenu *menu = new QMenu();
    menu->addAction(actAddCustom);
    menu->addSeparator();
    menu->addAction(actAddThermalConductivity);
    menu->addAction(actAddSpecificHeat);
    menu->addAction(actAddDensity);
    menu->addSeparator();
    menu->addAction(actAddMagneticPermeability);

    btnAddProperty->setMenu(menu);

    btnDeleteProperty = new QPushButton(tr("Delete"));
    btnDeleteProperty->setDefault(false);
    connect(btnDeleteProperty, SIGNAL(clicked()), this, SLOT(deleteProperty()));

    QGridLayout *layoutList = new QGridLayout();
    layoutList->addWidget(lstProperties, 0, 0, 1, 2);
    layoutList->addWidget(btnAddProperty, 1, 0);
    layoutList->addWidget(btnDeleteProperty, 1, 1);

    propertyGUI = createPropertyGUI();

    QHBoxLayout *layoutNonlinearProperties = new QHBoxLayout();
    layoutNonlinearProperties->addLayout(layoutList);
    layoutNonlinearProperties->addWidget(propertyGUI, 1);

    // table
    txtPropertyTableKeys = new QPlainTextEdit();
    txtPropertyTableValues = new QPlainTextEdit();

    QGridLayout *layoutTable = new QGridLayout();
    layoutTable->addWidget(new QLabel(tr("Keys:")), 0, 0);
    layoutTable->addWidget(txtPropertyTableKeys, 1, 0);
    layoutTable->addWidget(new QLabel(tr("Values:")), 0, 1);
    layoutTable->addWidget(txtPropertyTableValues, 1, 1);

    widNonlinearTable = new QGroupBox(tr("Table"));
    widNonlinearTable->setLayout(layoutTable);

    // function
    txtPropertyFunction = new ScriptEditor(currentPythonEngine(), this);
    txtPropertyFunctionFrom = new LineEditDouble(0.0);
    txtPropertyFunctionTo = new LineEditDouble(0.0);

    QGridLayout *layoutFunction = new QGridLayout();
    layoutFunction->addWidget(txtPropertyFunction, 0, 0, 1, 4);
    layoutFunction->addWidget(new QLabel(tr("From:")), 1, 0);
    layoutFunction->addWidget(txtPropertyFunctionFrom, 1, 1);
    layoutFunction->addWidget(new QLabel(tr("To:")), 1, 2);
    layoutFunction->addWidget(txtPropertyFunctionTo, 1, 3);
    layoutFunction->setRowStretch(0, 1);

    widNonlinearFunction = new QGroupBox(tr("Nonlinear function"));
    widNonlinearFunction->setLayout(layoutFunction);

    chartNonlinear = new QCustomPlot();
    chartNonlinear->setInteractions(QCustomPlot::iRangeDrag | QCustomPlot::iRangeZoom);
    chartNonlinear->setRangeDrag(Qt::Horizontal|Qt::Vertical);
    chartNonlinear->setRangeZoom(Qt::Horizontal|Qt::Vertical);
    chartNonlinear->setMinimumHeight(120);
    chartNonlinear->addGraph();
    chartNonlinear->graph(0)->setLineStyle(QCPGraph::lsLine);

    QHBoxLayout *layoutChartNonlinear = new QHBoxLayout();
    layoutChartNonlinear->addWidget(chartNonlinear);

    widChartNonlinear = new QGroupBox(tr("Chart"));
    widChartNonlinear->setLayout(layoutChartNonlinear);

    layoutNonlinearType = new QStackedLayout();
    layoutNonlinearType->addWidget(widNonlinearFunction);
    layoutNonlinearType->addWidget(widNonlinearTable);

    QHBoxLayout *layoutNonlinearChart = new QHBoxLayout();
    layoutNonlinearChart->addLayout(layoutNonlinearType, 2);
    layoutNonlinearChart->addWidget(widChartNonlinear, 1);

    QVBoxLayout *layoutNonlinear = new QVBoxLayout();
    layoutNonlinear->addLayout(layoutNonlinearProperties);
    layoutNonlinear->addLayout(layoutNonlinearChart, 1);

    // dialog buttons
    QPushButton *btnPlot = new QPushButton(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(drawChart()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->addButton(btnPlot, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutNameAndDescription);
    layout->addLayout(layoutNonlinear, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

QWidget *MaterialEditDialog::createPropertyGUI()
{
    txtPropertyName = new QLineEdit();
    txtPropertyShortname = new QLineEdit();
    connect(txtPropertyShortname, SIGNAL(textChanged(QString)), this, SLOT(setFunctionLabel()));
    txtPropertyUnit = new QLineEdit();
    connect(txtPropertyUnit, SIGNAL(textChanged(QString)), this, SLOT(setFunctionLabel()));
    txtPropertySource = new QLineEdit();
    txtPropertyIndependentVariableShortname = new QLineEdit();
    connect(txtPropertyIndependentVariableShortname, SIGNAL(textChanged(QString)), this, SLOT(setFunctionLabel()));
    txtPropertyIndependentVariableUnit = new QLineEdit();
    connect(txtPropertyIndependentVariableUnit, SIGNAL(textChanged(QString)), this, SLOT(setFunctionLabel()));
    cmbPropertyNonlinearityType = new QComboBox();
    cmbPropertyNonlinearityType->addItem(tr("Function"), MaterialEditDialog::Function);
    cmbPropertyNonlinearityType->addItem(tr("Table"), MaterialEditDialog::Table);
    connect(cmbPropertyNonlinearityType, SIGNAL(activated(int)), this, SLOT(doNonlinearDependenceChanged(int)));
    lblPropertyFunction = new QLabel();

    // constant
    txtPropertyConstant = new LineEditDouble(0.0);

    // table and function tab
    QGridLayout *layoutProperty = new QGridLayout();
    layoutProperty->addWidget(new QLabel(tr("Name:")), 0, 0);
    layoutProperty->addWidget(txtPropertyName, 0, 1, 1, 3);
    layoutProperty->addWidget(new QLabel(tr("Source:")), 1, 0);
    layoutProperty->addWidget(txtPropertySource, 1, 1, 1, 3);
    layoutProperty->addWidget(new QLabel(tr("Shortname:")), 2, 0);
    layoutProperty->addWidget(txtPropertyShortname, 2, 1);
    layoutProperty->addWidget(new QLabel(tr("Unit:")), 3, 0);
    layoutProperty->addWidget(txtPropertyUnit, 3, 1);
    layoutProperty->addWidget(new QLabel(tr("Constant value:")), 4, 0);
    layoutProperty->addWidget(txtPropertyConstant, 4, 1);
    layoutProperty->addWidget(new QLabel(tr("Kind of nonlinearity:")), 5, 0);
    layoutProperty->addWidget(cmbPropertyNonlinearityType, 5, 1);
    layoutProperty->addWidget(new QLabel(tr("Ind. var. shortname:")), 2, 2);
    layoutProperty->addWidget(txtPropertyIndependentVariableShortname, 2, 3);
    layoutProperty->addWidget(new QLabel(tr("Ind. var. unit:")), 3, 2);
    layoutProperty->addWidget(txtPropertyIndependentVariableUnit, 3, 3);
    layoutProperty->addWidget(new QLabel(tr("Function:")), 5, 2);
    layoutProperty->addWidget(lblPropertyFunction, 5, 3);
    layoutProperty->setRowStretch(11, 1);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layoutProperty);

    return widget;
}

void MaterialEditDialog::readMaterial()
{
    if (QFile::exists(m_fileName))
    {
        std::auto_ptr<XMLMaterial::material> material_xsd;
        material_xsd = XMLMaterial::material_(compatibleFilename(m_fileName).toStdString(), xml_schema::flags::dont_validate);
        XMLMaterial::material *material = material_xsd.get();

        txtName->setText(QString::fromStdString(material->general().name()));
        if (material->general().description().present())
            txtDescription->setText(QString::fromStdString(material->general().description().get()));

        m_properties.clear();
        lstProperties->clear();

        // properties
        for (unsigned int i = 0; i < material->properties().property().size(); i++)
        {
            XMLMaterial::property prop = material->properties().property().at(i);
            m_properties.append(prop);

            // item
            QListWidgetItem *item = new QListWidgetItem(lstProperties);
            item->setText(QString::fromStdString(prop.name()));
            item->setData(Qt::UserRole, lstProperties->count() - 1);

            lstProperties->addItem(item);
        }

        propertyGUI->setEnabled(lstProperties->count() > 0);
        btnDeleteProperty->setEnabled(lstProperties->count() > 0);

        if (lstProperties->count() > 0)
        {
            lstProperties->setCurrentRow(0);
            readProperty(m_properties.at(0));
        }
        else
        {
            readProperty();

        }
    }
}

bool MaterialEditDialog::writeMaterial()
{
    if (lstProperties->currentItem())
        m_properties.replace(lstProperties->currentRow(), writeProperty());

    QFileInfo fileInfo(m_fileName);
    if (fileInfo.baseName() != txtName->text())
    {
        bool move = true;

        // rename file
        QString newFilename = fileInfo.absolutePath() + "/" + txtName->text() + ".mat";
        if (QFile::exists(newFilename))
            move = (QMessageBox::question(this, tr("File exists"), tr("Material '%1' already exists. Do you wish replace this file?").arg(newFilename)) == 0);

        if (move)
        {
            QFile::remove(m_fileName);
            m_fileName = newFilename;
        }
    }

    try
    {
        // general
        XMLMaterial::general general(txtName->text().toStdString());
        general.description().set(txtDescription->text().toStdString());

        // properties
        XMLMaterial::properties properties;
        foreach(XMLMaterial::property prop, m_properties)
        {
            // add property
            properties.property().push_back(prop);
        }

        // material
        XMLMaterial::material material(general, properties);

        std::string mesh_schema_location("");

        // TODO: set path more general
        mesh_schema_location.append(QString("%1/material_xml.xsd").arg(QFileInfo(datadir() + XSDROOT).absoluteFilePath()).toStdString());
        ::xml_schema::namespace_info namespace_info_mesh("XMLMaterial", mesh_schema_location);

        ::xml_schema::namespace_infomap namespace_info_map;
        namespace_info_map.insert(std::pair<std::basic_string<char>, xml_schema::namespace_info>("material", namespace_info_mesh));

        std::ofstream out(compatibleFilename(m_fileName).toStdString().c_str());
        XMLMaterial::material_(out, material, namespace_info_map);

        return true;
    }
    catch (const xml_schema::exception& e)
    {
        std::cerr << e << std::endl;

        return false;
    }
}

void MaterialEditDialog::addProperty(const QString &name, const QString &shortname, const QString &unit, const QString &nonlinearityKind,
                                     const QString &indepedentShortname, const QString &indepedentUnit)
{
    bool ok = false;
    QString propName = name;

    if (propName.isEmpty())
        propName = QInputDialog::getText(this, tr("Add property"),
                                         tr("Name:"), QLineEdit::Normal, "", &ok);
    else
        ok = true;

    if (ok && !propName.isEmpty())
    {
        XMLMaterial::property prop(propName.toStdString(),
                                   shortname.toStdString(),
                                   unit.toStdString(),
                                   "",
                                   nonlinearityKind.toStdString(),
                                   indepedentShortname.toStdString(),
                                   indepedentUnit.toStdString());

        XMLMaterial::nonlinearity nonlinearity;
        nonlinearity.function().set(XMLMaterial::function("def agros2d_material(t):\n"
                                                          "    return 0.0",
                                                          0, 100));
        prop.nonlinearity().set(nonlinearity);
        m_properties.append(prop);

        // item
        QListWidgetItem *item = new QListWidgetItem(lstProperties);
        item->setText(QString::fromStdString(prop.name()));

        lstProperties->addItem(item);
        lstProperties->setCurrentItem(item);
        propertyGUI->setEnabled(lstProperties->count() > 0);
        btnDeleteProperty->setEnabled(lstProperties->count() > 0);

        readProperty(prop);
    }
}

void MaterialEditDialog::deleteProperty()
{
    if (lstProperties->currentItem())
    {
        if (QMessageBox::question(this, tr("Delete property"),
                                  tr("Property '%1' will be pernamently deleted. Are you sure?").arg(txtPropertyName->text()),
                                  tr("&Yes"), tr("&No")) == 0)
        {
            int row = lstProperties->row(lstProperties->currentItem());
            lstProperties->takeItem(row);
            m_properties.removeAt(row);

            propertyGUI->setEnabled(lstProperties->count() > 0);
            btnDeleteProperty->setEnabled(lstProperties->count() > 0);

            if (lstProperties->count() > 0)
            {
                lstProperties->setCurrentRow(0);
                readProperty(m_properties.at(0));
            }
            else
            {
                readProperty();
            }
        }
    }
}

void MaterialEditDialog::readProperty(XMLMaterial::property prop)
{
    txtPropertyName->setText(QString::fromStdString(prop.name()));
    txtPropertyShortname->setText(QString::fromStdString(prop.shortname()));
    txtPropertyUnit->setText(QString::fromStdString(prop.unit()));
    txtPropertySource->setText(QString::fromStdString(prop.source()));
    cmbPropertyNonlinearityType->setCurrentIndex(cmbPropertyNonlinearityType->findData(nonlinearityTypeFromStringKey(QString::fromStdString(prop.nonlinearity_kind()))));
    txtPropertyIndependentVariableShortname->setText(QString::fromStdString(prop.independent_shortname()));
    txtPropertyIndependentVariableUnit->setText(QString::fromStdString(prop.independent_unit()));

    // clear
    txtPropertyConstant->setValue(0.0);
    txtPropertyTableKeys->clear();
    txtPropertyTableValues->clear();
    txtPropertyFunction->clear();
    txtPropertyFunctionFrom->setValue(0.0);
    txtPropertyFunctionTo->setValue(0.0);

    // constant
    if (prop.constant().present())
        txtPropertyConstant->setValue(prop.constant().get().value());

    // table
    if (prop.nonlinearity().present())
    {
        if (prop.nonlinearity().get().table().present())
        {
            XMLMaterial::table table = prop.nonlinearity().get().table().get();

            txtPropertyTableKeys->setPlainText(QString::fromStdString(table.keys()));
            txtPropertyTableValues->setPlainText(QString::fromStdString(table.values()));
        }
    }

    // function
    if (prop.nonlinearity().present())
    {
        if (prop.nonlinearity().get().function().present())
        {
            XMLMaterial::function function = prop.nonlinearity().get().function().get();

            txtPropertyFunction->setPlainText(QString::fromStdString(function.body()));
            txtPropertyFunctionFrom->setValue(function.interval_from());
            txtPropertyFunctionTo->setValue(function.interval_to());
        }
    }

    doNonlinearDependenceChanged(cmbPropertyNonlinearityType->currentIndex());

    // draw chart
    drawChart();
}

void MaterialEditDialog::setFunctionLabel()
{
    lblPropertyFunction->setText(QString("%1 (%2) = function(%3 (%4))").
                                 arg(txtPropertyShortname->text()).
                                 arg(txtPropertyUnit->text()).
                                 arg(txtPropertyIndependentVariableShortname->text()).
                                 arg(txtPropertyIndependentVariableUnit->text()));
}

void MaterialEditDialog::drawChart()
{
    QVector<double> keys;
    QVector<double> values;

    if (((NonlinearityType) cmbPropertyNonlinearityType->itemData(cmbPropertyNonlinearityType->currentIndex()).toInt()) == MaterialEditDialog::Function)
    {
        currentPythonEngineAgros()->materialValues(txtPropertyFunction->toPlainText(),
                                                   txtPropertyFunctionFrom->value(),
                                                   txtPropertyFunctionTo->value(),
                                                   &keys, &values, 500);
    }
    else
    {
        QStringList keysString = txtPropertyTableKeys->toPlainText().split("\n");
        QStringList valuesString = txtPropertyTableValues->toPlainText().split("\n");

        for (int j = 0; j < keysString.size(); j++)
        {
            if ((!keysString.at(j).isEmpty()) && (j < valuesString.count()) && (!valuesString.at(j).isEmpty()))
            {
                keys.append(keysString.at(j).toDouble());
                values.append(valuesString.at(j).toDouble());
            }
        }
    }

    // chartNonlinear->xAxis->setLabel(txtPropertyDependenceUnit->text());
    // chartNonlinear->yAxis->setLabel(txtPropertyUnit->text());
    chartNonlinear->graph(0)->setData(keys, values);
    chartNonlinear->rescaleAxes();
    chartNonlinear->replot();
}

XMLMaterial::property MaterialEditDialog::writeProperty()
{
    // property
    XMLMaterial::property prop(txtPropertyName->text().toStdString(),
                               txtPropertyShortname->text().toStdString(),
                               txtPropertyUnit->text().toStdString(),
                               txtPropertySource->text().toStdString(),
                               nonlinearityTypeToStringKey((NonlinearityType) cmbPropertyNonlinearityType->itemData(cmbPropertyNonlinearityType->currentIndex()).toInt()).toStdString(),
                               txtPropertyIndependentVariableShortname->text().toStdString(),
                               txtPropertyIndependentVariableUnit->text().toStdString());

    // constant
    prop.constant().set(XMLMaterial::constant(txtPropertyConstant->value()));

    // dependence
    XMLMaterial::nonlinearity nonlinearity;

    // table
    if (((NonlinearityType) cmbPropertyNonlinearityType->itemData(cmbPropertyNonlinearityType->currentIndex()).toInt()) == MaterialEditDialog::Function)
    {
        // function
        nonlinearity.function().set(XMLMaterial::function(txtPropertyFunction->toPlainText().toStdString(),
                                                          txtPropertyFunctionFrom->value(),
                                                          txtPropertyFunctionTo->value()));
    }
    else if (((NonlinearityType) cmbPropertyNonlinearityType->itemData(cmbPropertyNonlinearityType->currentIndex()).toInt()) == MaterialEditDialog::Table)
    {
        nonlinearity.table().set(XMLMaterial::table(txtPropertyTableKeys->toPlainText().toStdString(),
                                                    txtPropertyTableValues->toPlainText().toStdString()));
    }

    prop.nonlinearity().set(nonlinearity);

    return prop;
}

void MaterialEditDialog::doNonlinearDependenceChanged(int index)
{
    if (((NonlinearityType) cmbPropertyNonlinearityType->itemData(cmbPropertyNonlinearityType->currentIndex()).toInt()) == MaterialEditDialog::Function)
        layoutNonlinearType->setCurrentWidget(widNonlinearFunction);
    else if (((NonlinearityType) cmbPropertyNonlinearityType->itemData(cmbPropertyNonlinearityType->currentIndex()).toInt()) == MaterialEditDialog::Table)
        layoutNonlinearType->setCurrentWidget(widNonlinearTable);
}

void MaterialEditDialog::doPropertyChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (previous)
        m_properties.replace(lstProperties->row(previous), writeProperty());

    if (current)
        readProperty(m_properties.at(lstProperties->row(current)));
}

void MaterialEditDialog::doAccept()
{
    if (writeMaterial())
        accept();
}

// ***********************************************************************************************************

MaterialBrowserDialog::MaterialBrowserDialog(QWidget *parent) : QDialog(parent),
    m_select(false)
{
    setWindowTitle(tr("Material Browser"));
    setModal(true);

    // problem information
    webView = new QWebView(this);
    webView->page()->setNetworkAccessManager(networkAccessManager());
    webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    connect(webView->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", htmlFontFamily().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(htmlFontSize()).toStdString()));

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/style_common.css").toStdString(), ctemplate::DO_NOT_STRIP, &stylesheet, &style);
    m_cascadeStyleSheet = QString::fromStdString(style);

    trvMaterial = new QTreeWidget(this);
    trvMaterial->setMouseTracking(true);
    trvMaterial->setColumnCount(1);
    trvMaterial->setIndentation(15);
    trvMaterial->setIconSize(QSize(24, 24));
    trvMaterial->setHeaderHidden(true);
    trvMaterial->setMinimumWidth(230);

    connect(trvMaterial, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(trvMaterial, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    btnNew = new QPushButton();
    btnNew->setText(tr("New"));
    btnNew->setDefault(false);
    connect(btnNew, SIGNAL(clicked()), this, SLOT(doNew()));

    btnEdit = new QPushButton();
    btnEdit->setText(tr("Edit"));
    btnEdit->setDefault(false);
    btnEdit->setEnabled(false);
    connect(btnEdit, SIGNAL(clicked()), this, SLOT(doEdit()));

    btnDelete = new QPushButton();
    btnDelete->setText(tr("Delete"));
    btnDelete->setDefault(false);
    btnDelete->setEnabled(false);
    connect(btnDelete, SIGNAL(clicked()), this, SLOT(doDelete()));

    QGridLayout *layoutProperties = new QGridLayout();
    layoutProperties->addWidget(trvMaterial, 0, 0, 1, 3);
    layoutProperties->addWidget(btnNew, 2, 0);
    layoutProperties->addWidget(btnEdit, 2, 1);
    layoutProperties->addWidget(btnDelete, 2, 2);

    QHBoxLayout *layoutSurface = new QHBoxLayout();
    layoutSurface->addLayout(layoutProperties);
    layoutSurface->addWidget(webView, 1);

    QWidget *widget = new QWidget();
    widget->setLayout(layoutSurface);

    QPushButton *btnClose = new QPushButton();
    btnClose->setText(tr("Close"));
    btnClose->setDefault(true);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch(1);
    layoutButtons->addWidget(btnClose);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widget, 1);
    layout->addStretch();
    layout->addLayout(layoutButtons);

    setLayout(layout);

    readMaterials();

    QSettings settings;
    restoreGeometry(settings.value("MaterialBrowserDialog/Geometry", saveGeometry()).toByteArray());
}

MaterialBrowserDialog::~MaterialBrowserDialog()
{    
    QSettings settings;
    settings.setValue("MaterialBrowserDialog/Geometry", saveGeometry());
}

int MaterialBrowserDialog::showDialog(bool select)
{
    m_select = select;
    return exec();
}

void MaterialBrowserDialog::doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    webView->setHtml("");
    btnEdit->setEnabled(false);
    btnDelete->setEnabled(false);

    if (current)
    {
        m_selectedFilename = current->data(0, Qt::UserRole).toString();
        if (!m_selectedFilename.isEmpty())
        {
            materialInfo(m_selectedFilename);
            if (QFileInfo(m_selectedFilename).isWritable())
            {
                btnEdit->setEnabled(true);
                btnDelete->setEnabled(true);
            }
        }
    }
}

void MaterialBrowserDialog::doItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (trvMaterial->currentItem())
    {
        doEdit();
    }
}

void MaterialBrowserDialog::readMaterials()
{
    // clear listview
    trvMaterial->clear();

    // read materials
    QDir dirSystem(QString("%1/resources/materials").arg(datadir()));
    dirSystem.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);
    readMaterials(dirSystem, trvMaterial->invisibleRootItem());

    // user materials
    QFont fnt = trvMaterial->font();
    fnt.setBold(true);

    QTreeWidgetItem *customMaterialsItem = new QTreeWidgetItem(trvMaterial->invisibleRootItem());
    customMaterialsItem->setText(0, tr("Custom materials"));
    customMaterialsItem->setFont(0, fnt);
    customMaterialsItem->setExpanded(true);

    QDir dirUser(QString("%1/materials").arg(userDataDir()));
    if (!dirUser.exists())
        QDir(userDataDir()).mkpath(dirUser.absolutePath());
    dirUser.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);
    readMaterials(dirUser, customMaterialsItem);

    if (!m_selectedFilename.isEmpty())
    {
        // qDebug() << QFileInfo(m_selectedFilename).baseName();
        QList<QTreeWidgetItem *> items = trvMaterial->findItems(QFileInfo(m_selectedFilename).baseName(), Qt::MatchExactly);
        // qDebug() << items.count();
        if (items.count() >= 1)
        {
            trvMaterial->setCurrentItem(items.at(items.count() - 1));
            materialInfo(m_selectedFilename);
        }
    }
}

void MaterialBrowserDialog::readMaterials(QDir dir, QTreeWidgetItem *parentItem)
{
    QFileInfoList listExamples = dir.entryInfoList();
    for (int i = 0; i < listExamples.size(); ++i)
    {
        QFileInfo fileInfo = listExamples.at(i);
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if (fileInfo.isDir())
        {
            QFont fnt = trvMaterial->font();
            fnt.setBold(true);

            QTreeWidgetItem *dirItem = new QTreeWidgetItem(parentItem);
            dirItem->setText(0, fileInfo.fileName());
            dirItem->setFont(0, fnt);
            dirItem->setExpanded(true);

            // recursive read
            readMaterials(fileInfo.absoluteFilePath(), dirItem);
        }
        else if (fileInfo.suffix() == "mat")
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(parentItem);
            item->setText(0, fileInfo.baseName());
            item->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());
        }
    }
}

void MaterialBrowserDialog::materialInfo(const QString &fileName)
{
    // template
    std::string info;
    ctemplate::TemplateDictionary materialInfo("info");

    if (QFile::exists(fileName))
    {
        try
        {
            std::auto_ptr<XMLMaterial::material> material_xsd;
            material_xsd = XMLMaterial::material_(compatibleFilename(fileName).toStdString(), xml_schema::flags::dont_validate);
            XMLMaterial::material *material = material_xsd.get();

            materialInfo.SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
            materialInfo.SetValue("PANELS_DIRECTORY", QUrl::fromLocalFile(QString("%1%2").arg(QDir(datadir()).absolutePath()).arg(TEMPLATEROOT + "/panels")).toString().toStdString());

            materialInfo.SetValue("NAME", material->general().name());
            if (material->general().description().present())
                materialInfo.SetValue("DESCRIPTION", material->general().description().get());

            // properties
            for (unsigned int i = 0; i < material->properties().property().size(); i++)
            {
                XMLMaterial::property prop = material->properties().property().at(i);

                ctemplate::TemplateDictionary *propSection = materialInfo.AddSectionDictionary("PROPERTIES_SECTION");

                propSection->SetValue("PROPERTY_LABEL", prop.name());
                propSection->SetValue("PROPERTY_SOURCE", prop.source());

                propSection->SetValue("PROPERTY_SHORTNAME_LABEL", tr("Variable:").toStdString());
                propSection->SetValue("PROPERTY_SHORTNAME", prop.shortname());
                propSection->SetValue("PROPERTY_UNIT_LABEL", tr("Unit:").toStdString());
                propSection->SetValue("PROPERTY_UNIT", prop.unit());

                propSection->SetValue("PROPERTY_INDEPENDENT_SHORTNAME_LABEL", tr("Independent variable:").toStdString());
                propSection->SetValue("PROPERTY_INDEPENDENT_SHORTNAME", prop.independent_shortname());
                propSection->SetValue("PROPERTY_INDEPENDENT_UNIT_LABEL", tr("Independent unit:").toStdString());
                propSection->SetValue("PROPERTY_INDEPENDENT_UNIT", prop.independent_unit());

                if (m_select)
                    propSection->ShowSection("PROPERTY_SELECTABLE");

                if (prop.constant().present())
                {
                    XMLMaterial::constant constant = prop.constant().get();
                    propSection->SetValue("PROPERTY_CONSTANT_LABEL", tr("Constant:").toStdString());
                    propSection->SetValue("PROPERTY_CONSTANT", QString::number(constant.value()).toStdString());
                }

                if (prop.nonlinearity().present())
                {
                    QVector<double> keys;
                    QVector<double> values;

                    if (prop.nonlinearity().get().table().present())
                    {
                        XMLMaterial::table table = prop.nonlinearity().get().table().get();

                        QStringList keysString = QString::fromStdString(table.keys()).split("\n");
                        QStringList valuesString = QString::fromStdString(table.values()).split("\n");

                        for (int j = 0; j < keysString.size(); j++)
                        {
                            if ((!keysString.at(j).isEmpty()) && (j < valuesString.count()) && (!valuesString.at(j).isEmpty()))
                            {
                                keys.append(keysString.at(j).toDouble());
                                values.append(valuesString.at(j).toDouble());
                            }
                        }
                    }

                    if (prop.nonlinearity().get().function().present())
                    {
                        XMLMaterial::function function = prop.nonlinearity().get().function().get();

                        currentPythonEngineAgros()->materialValues(QString::fromStdString(function.body()),
                                                                   function.interval_from(),
                                                                   function.interval_to(),
                                                                   &keys, &values);
                    }

                    if (keys.size() > 0)
                    {
                        QString keysString;
                        QString valuesString;

                        QString chart = "[";
                        for (int i = 0; i < keys.size(); i++)
                        {
                            chart += QString("[%1, %2], ").arg(keys[i]).arg(values[i]);
                            keysString += QString("%1").arg(keys[i]) + ((i < keys.size() - 1) ? "," : "");
                            valuesString += QString("%1").arg(values[i]) + ((i < keys.size() - 1) ? "," : "");
                        }
                        chart += "]";

                        QString id = QUuid::createUuid().toString().remove("{").remove("}");

                        propSection->SetValue("PROPERTY_CHART", QString("chart_%1").arg(id).toStdString());
                        propSection->SetValue("PROPERTY_CHART_SCRIPT", QString("<script type=\"text/javascript\">$(function () { $.plot($(\"#chart_%1\"), [ { data: %2, color: \"rgb(61, 61, 251)\", lines: { show: true }, points: { show: false } } ], { grid: { hoverable : false }, xaxes: [ { axisLabel: '%3 (%4)' } ], yaxes: [ { axisLabel: '%5 (%6)' } ] });});</script>")
                                              .arg(id)
                                              .arg(chart)
                                              .arg(QString::fromStdString(prop.independent_shortname()))
                                              .arg(QString::fromStdString(prop.independent_unit()))
                                              .arg(QString::fromStdString(prop.shortname()))
                                              .arg(QString::fromStdString(prop.unit())).toStdString());

                        propSection->SetValue("PROPERTY_INDEPENDENT_X", keysString.toStdString());
                        propSection->SetValue("PROPERTY_INDEPENDENT_Y", valuesString.toStdString());
                    }
                }
            }
        }
        catch (const xml_schema::exception& e)
        {
            std::cerr << e << std::endl;
        }
    }

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/material.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &materialInfo, &info);

    // setHtml(...) doesn't work
    // webView->setHtml(QString::fromStdString(info));

    // load(...) works
    writeStringContent(tempProblemDir() + "/material.html", QString::fromStdString(info));
    webView->load(QUrl::fromLocalFile(tempProblemDir() + "/material.html"));
}

void MaterialBrowserDialog::linkClicked(const QUrl &url)
{
    QString search = "/property?";
    if (url.toString().contains(search))
    {
        m_selected_x.clear();
        m_selected_y.clear();

#if QT_VERSION < 0x050000
        QStringList keysString = url.queryItemValue("x").split(",");
        QStringList valuesString = url.queryItemValue("y").split(",");
#else
        QStringList keysString = QUrlQuery(url).queryItemValue("x").split(",");
        QStringList valuesString = QUrlQuery(url).queryItemValue("y").split(",");
#endif

        for (int j = 0; j < keysString.size(); j++)
        {
            m_selected_x.append(keysString.at(j).toDouble());
            m_selected_y.append(valuesString.at(j).toDouble());
        }

        accept();
    }
}

void MaterialBrowserDialog::doNew()
{
    try
    {
        bool ok = false;
        QString name = QInputDialog::getText(this, tr("Add custom material"),
                                             tr("Name:"), QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty())
        {
            QString fileName = QString("%1/materials/%2.mat").arg(userDataDir()).arg(name);
            if (QFile::exists(fileName))
            {
                QMessageBox::warning(this, tr("Material"), tr("Material already exists."));
                return;
            }

            // general
            XMLMaterial::general general(name.toStdString());

            // properties
            XMLMaterial::properties properties;

            // material
            XMLMaterial::material material(general, properties);

            std::string mesh_schema_location("");

            // TODO: set path more general
            mesh_schema_location.append(QString("%1/material_xml.xsd").arg(QFileInfo(datadir() + XSDROOT).absoluteFilePath()).toStdString());
            ::xml_schema::namespace_info namespace_info_mesh("XMLMaterial", mesh_schema_location);

            ::xml_schema::namespace_infomap namespace_info_map;
            namespace_info_map.insert(std::pair<std::basic_string<char>, xml_schema::namespace_info>("material", namespace_info_mesh));

            std::ofstream out(compatibleFilename(fileName).toStdString().c_str());
            XMLMaterial::material_(out, material, namespace_info_map);

            // select item and edit
            m_selectedFilename = fileName;
            readMaterials();

            doEdit();
        }
    }
    catch (const xml_schema::exception& e)
    {
        std::cerr << e << std::endl;
    }
}

void MaterialBrowserDialog::doEdit()
{
    if (!btnEdit->isEnabled())
        return;

    MaterialEditDialog dialog(m_selectedFilename, this);
    if (dialog.showDialog() == QDialog::Accepted)
    {
        m_selectedFilename = dialog.fileName();
        trvMaterial->currentItem()->setText(0, QFileInfo(m_selectedFilename).baseName());
        materialInfo(m_selectedFilename);
    }
}

void MaterialBrowserDialog::doDelete()
{
    QFileInfo fileInfo(m_selectedFilename);
    if (fileInfo.exists() && fileInfo.isWritable())
    {
        if (QMessageBox::question(this, tr("Delete material"),
                                  tr("Material '%1' will be pernamently deleted. Are you sure?").arg(fileInfo.baseName()),
                                  tr("&Yes"), tr("&No")) == 0)
        {
            QFile::remove(m_selectedFilename);
            m_selectedFilename = "";

            readMaterials();
        }
    }
}

