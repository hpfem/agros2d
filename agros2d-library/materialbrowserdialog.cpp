
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
#include "gui/latexviewer.h"
#include "gui/common.h"

#include "hermes2d/module.h"

#include "ctemplate/template.h"

#include "pythonlab/pythonengine_agros.h"

#include "../resources_source/classes/material_xml.h";


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
    tabProperties = new QTabWidget(this);
    connect(tabProperties, SIGNAL(tabCloseRequested(int)), this, SLOT(closeProperty(int)));

    txtName = new QLineEdit();
    txtDescription = new QLineEdit();

    QGridLayout *layoutProperty = new QGridLayout();
    layoutProperty->addWidget(new QLabel(tr("Name:")), 0, 0);
    layoutProperty->addWidget(txtName, 0, 1);
    layoutProperty->addWidget(new QLabel(tr("Description:")), 1, 0);
    layoutProperty->addWidget(txtDescription, 1, 1);
    layoutProperty->setColumnStretch(1, 1);

    // dialog buttons
    QPushButton *btnAddProperty = new QPushButton(tr("Add property"));
    btnAddProperty->setDefault(false);
    connect(btnAddProperty, SIGNAL(clicked()), this, SLOT(addProperty()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->addButton(btnAddProperty, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutProperty, 1);
    layout->addWidget(tabProperties);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

MaterialEditDialog::Property MaterialEditDialog::addPropertyUI(const QString &name)
{
    Property propUI;

    propUI.txtName = new QLineEdit();
    propUI.txtShortname = new QLineEdit();
    propUI.txtUnit = new QLineEdit();
    propUI.txtSource = new QLineEdit();
    propUI.txtDependenceShortname = new QLineEdit();
    propUI.txtDependenceUnit = new QLineEdit();

    // constant
    propUI.txtConstant = new LineEditDouble(0.0);

    QGridLayout *layoutConstant = new QGridLayout();
    layoutConstant->addWidget(new QLabel(tr("Value:")), 0, 0);
    layoutConstant->addWidget(propUI.txtConstant, 0, 1);

    QGroupBox *grpConstant = new QGroupBox(tr("Constant"));
    grpConstant->setLayout(layoutConstant);

    // table
    propUI.txtTableKeys = new QTextEdit();
    propUI.txtTableValues = new QTextEdit();

    QGridLayout *layoutTable = new QGridLayout();
    layoutTable->addWidget(new QLabel(tr("Keys:")), 0, 0);
    layoutTable->addWidget(propUI.txtTableKeys, 0, 1);
    layoutTable->addWidget(new QLabel(tr("Values:")), 1, 0);
    layoutTable->addWidget(propUI.txtTableValues, 1, 1);
    layoutTable->setRowStretch(10, 1);

    QWidget *widTable = new QWidget();
    widTable->setLayout(layoutTable);

    // function
    propUI.txtFunction = new QTextEdit();
    propUI.txtFunctionFrom = new LineEditDouble(0.0);
    propUI.txtFunctionTo = new LineEditDouble(0.0);

    QGridLayout *layoutFunction = new QGridLayout();
    layoutFunction->addWidget(new QLabel(tr("Function:")), 0, 0);
    layoutFunction->addWidget(propUI.txtFunction, 0, 1);
    layoutFunction->addWidget(new QLabel(tr("From:")), 1, 0);
    layoutFunction->addWidget(propUI.txtFunctionFrom, 1, 1);
    layoutFunction->addWidget(new QLabel(tr("To:")), 2, 0);
    layoutFunction->addWidget(propUI.txtFunctionTo, 2, 1);
    layoutFunction->setRowStretch(0, 1);

    QWidget *widFunction = new QWidget();
    widFunction->setLayout(layoutFunction);

    // table and function tab
    propUI.tabTableAndFunction = new QTabWidget();
    propUI.tabTableAndFunction->addTab(widFunction, tr("Function"));
    propUI.tabTableAndFunction->addTab(widTable, tr("Table"));

    QGridLayout *layoutProperty = new QGridLayout();
    layoutProperty->addWidget(new QLabel(tr("Name:")), 0, 0);
    layoutProperty->addWidget(propUI.txtName, 0, 1, 1, 3);
    layoutProperty->addWidget(new QLabel(tr("Source:")), 1, 0);
    layoutProperty->addWidget(propUI.txtSource, 1, 1, 1, 3);
    layoutProperty->addWidget(new QLabel(tr("Shortname:")), 2, 0);
    layoutProperty->addWidget(propUI.txtShortname, 2, 1);
    layoutProperty->addWidget(new QLabel(tr("Unit:")), 3, 0);
    layoutProperty->addWidget(propUI.txtUnit, 3, 1);
    layoutProperty->addWidget(new QLabel(tr("Dependence shortname:")), 2, 2);
    layoutProperty->addWidget(propUI.txtDependenceShortname, 2, 3);
    layoutProperty->addWidget(new QLabel(tr("Dependence unit:")), 3, 2);
    layoutProperty->addWidget(propUI.txtDependenceUnit, 3, 3);

    layoutProperty->addWidget(grpConstant, 10, 0, 1, 4);
    layoutProperty->addWidget(propUI.tabTableAndFunction, 11, 0, 1, 4);
    layoutProperty->setRowStretch(11, 1);

    // add property UI to the list
    propertiesUI.append(propUI);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layoutProperty);

    tabProperties->addTab(widget, name);
    tabProperties->setTabsClosable(true);

    return propUI;
}

void MaterialEditDialog::readMaterial()
{
    if (QFile::exists(m_fileName))
    {
        std::auto_ptr<XMLMaterial::material> material_xsd;
        material_xsd = XMLMaterial::material_(m_fileName.toLatin1().data(), xml_schema::flags::dont_validate);
        XMLMaterial::material *material = material_xsd.get();

        txtName->setText(QString::fromStdString(material->general().name()));
        if (material->general().description().present())
            txtDescription->setText(QString::fromStdString(material->general().description().get()));

        tabProperties->clear();

        // properties
        for (unsigned int i = 0; i < material->properties().property().size(); i++)
        {
            XMLMaterial::property prop = material->properties().property().at(i);

            Property propUI = addPropertyUI(QString::fromStdString(prop.name()));

            propUI.txtName->setText(QString::fromStdString(prop.name()));
            propUI.txtShortname->setText(QString::fromStdString(prop.shortname()));
            propUI.txtUnit->setText(QString::fromStdString(prop.unit()));
            propUI.txtDependenceShortname->setText(QString::fromStdString(prop.dependence_shortname()));
            propUI.txtDependenceUnit->setText(QString::fromStdString(prop.dependence_unit()));
            propUI.txtSource->setText(QString::fromStdString(prop.source()));

            // constant
            if (prop.constant().present())
                propUI.txtConstant->setValue(prop.constant().get().value());

            // table
            if (prop.dependence().present())
            {
                if (prop.dependence().get().table().present())
                {
                    XMLMaterial::table table = prop.dependence().get().table().get();

                    propUI.txtTableKeys->setText(QString::fromStdString(table.keys()));
                    propUI.txtTableValues->setText(QString::fromStdString(table.values()));
                }
            }

            // function
            if (prop.dependence().present())
            {
                if (prop.dependence().get().function().present())
                {
                    XMLMaterial::function function = prop.dependence().get().function().get();

                    propUI.txtFunction->setText(QString::fromStdString(function.body()));
                    propUI.txtFunctionFrom->setValue(function.interval_from());
                    propUI.txtFunctionTo->setValue(function.interval_to());
                }
            }

            if (propUI.txtFunction->toPlainText().trimmed().isEmpty())
                propUI.tabTableAndFunction->setCurrentIndex(1);
            else
                propUI.tabTableAndFunction->setCurrentIndex(0);
        }
    }
}

bool MaterialEditDialog::writeMaterial()
{
    try
    {
        // general
        XMLMaterial::general general(txtName->text().toStdString());
        general.description().set(txtDescription->text().toStdString());

        // properties
        XMLMaterial::properties properties;
        foreach (Property propUI, propertiesUI)
        {
            // property
            XMLMaterial::property prop(propUI.txtName->text().toStdString(),
                                       propUI.txtShortname->text().toStdString(),
                                       propUI.txtUnit->text().toStdString(),
                                       propUI.txtDependenceShortname->text().toStdString(),
                                       propUI.txtDependenceUnit->text().toStdString(),
                                       propUI.txtSource->text().toStdString());

            // constant
            prop.constant().set(XMLMaterial::constant(propUI.txtConstant->value()));

            // dependence
            XMLMaterial::dependence dependence;

            // table
            if (propUI.tabTableAndFunction->currentIndex() == 0)
            {
                // function
                dependence.function().set(XMLMaterial::function(propUI.txtFunction->toPlainText().toStdString(),
                                                                propUI.txtFunctionFrom->value(),
                                                                propUI.txtFunctionTo->value()));
            }
            else
            {
                dependence.table().set(XMLMaterial::table(propUI.txtTableKeys->toPlainText().toStdString(),
                                                          propUI.txtTableValues->toPlainText().toStdString()));
            }

            prop.dependence().set(dependence);

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

void MaterialEditDialog::addProperty()
{
    addPropertyUI(tr("New property"));
    writeMaterial();
}

void MaterialEditDialog::closeProperty(int index)
{
    if (index != -1)
    {
        if (QMessageBox::question(this, tr("Delete property"),
                                  tr("Property '%1' will be pernamently deleted. Are you sure?").arg(tabProperties->tabText(index)),
                                  tr("&Yes"), tr("&No")) == 0)
        {
            tabProperties->removeTab(index);
            propertiesUI.removeAt(index);
        }
    }
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

    connect(trvMaterial, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
            this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(trvMaterial, SIGNAL(itemActivated(QTreeWidgetItem *, int)),
            this, SLOT(doItemSelected(QTreeWidgetItem *, int)));
    connect(trvMaterial, SIGNAL(itemPressed(QTreeWidgetItem *, int)),
            this, SLOT(doItemSelected(QTreeWidgetItem *, int)));

    QHBoxLayout *layoutSurface = new QHBoxLayout();
    layoutSurface->addWidget(trvMaterial);
    layoutSurface->addWidget(webView, 1);

    QWidget *widget = new QWidget();
    widget->setLayout(layoutSurface);

    QPushButton *btnClose = new QPushButton();
    btnClose->setText(tr("Close"));
    btnClose->setDefault(true);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

    btnEdit = new QPushButton();
    btnEdit->setText(tr("Edit"));
    btnEdit->setDefault(false);
    btnEdit->setEnabled(false);
    btnEdit->setVisible(false);
    connect(btnEdit, SIGNAL(clicked()), this, SLOT(doEdit()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch(1);
    layoutButtons->addWidget(btnEdit);
    layoutButtons->addWidget(btnClose);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widget, 1);
    layout->addStretch();
    layout->addLayout(layoutButtons);

    setLayout(layout);

    readMaterials();

    QSettings settings;
    settings.setValue("MaterialBrowserDialog/Geometry", saveGeometry());
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

void MaterialBrowserDialog::doItemSelected(QTreeWidgetItem *item, int column)
{
    m_selectedFilename = item->data(0, Qt::UserRole).toString();
    if (!m_selectedFilename.isEmpty())
    {
        materialInfo(m_selectedFilename);
        btnEdit->setEnabled(true);
    }
    else
    {
        webView->setHtml("");
        btnEdit->setEnabled(false);
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
    QTreeWidgetItem *currentItem = trvMaterial->currentItem();

    // clear listview
    trvMaterial->clear();

    QDir dir(QString("%1/resources/materials").arg(datadir()));
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);

    readMaterials(dir, trvMaterial->invisibleRootItem());

    if (currentItem)
    {
        trvMaterial->setCurrentItem(currentItem);
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
            QTreeWidgetItem *exampleItem = new QTreeWidgetItem(parentItem);
            exampleItem->setText(0, fileInfo.baseName());
            exampleItem->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());
        }
    }
}

void MaterialBrowserDialog::materialInfo(const QString &fileName)
{
    if (QFile::exists(fileName))
    {
        std::auto_ptr<XMLMaterial::material> material_xsd;
        material_xsd = XMLMaterial::material_(fileName.toLatin1().data(), xml_schema::flags::dont_validate);
        XMLMaterial::material *material = material_xsd.get();

        // template
        std::string info;
        ctemplate::TemplateDictionary materialInfo("info");

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

            propSection->SetValue("PROPERTY_DEPENDENCE_SHORTNAME_LABEL", tr("Dependence variable:").toStdString());
            propSection->SetValue("PROPERTY_DEPENDENCE_SHORTNAME", prop.dependence_shortname());
            propSection->SetValue("PROPERTY_DEPENDENCE_UNIT_LABEL", tr("Dependence unit:").toStdString());
            propSection->SetValue("PROPERTY_DEPENDENCE_UNIT", prop.dependence_unit());

            if (m_select)
                propSection->ShowSection("PROPERTY_SELECTABLE");

            if (prop.constant().present())
            {
                XMLMaterial::constant constant = prop.constant().get();
                propSection->SetValue("PROPERTY_CONSTANT_LABEL", tr("Constant:").toStdString());
                propSection->SetValue("PROPERTY_CONSTANT", QString::number(constant.value()).toStdString());
            }

            if (prop.dependence().present())
            {
                QList<double> keys;
                QList<double> values;

                if (prop.dependence().get().table().present())
                {
                    XMLMaterial::table table = prop.dependence().get().table().get();

                    QStringList keysString = QString::fromStdString(table.keys()).split(",");
                    QStringList valuesString = QString::fromStdString(table.values()).split(",");
                    assert(keys.size() == values.size());

                    for (int j = 0; j < keysString.size(); j++)
                    {
                        if ((!keysString.at(j).isEmpty()) && (!valuesString.at(j).isEmpty()))
                        {
                            keys.append(keysString.at(j).toDouble());
                            values.append(valuesString.at(j).toDouble());
                        }
                    }
                }

                if (prop.dependence().get().function().present())
                {
                    XMLMaterial::function function = prop.dependence().get().function().get();

                    functionValues(QString::fromStdString(function.body()),
                                   function.interval_from(),
                                   function.interval_to(),
                                   40,
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
                                          .arg(QString::fromStdString(prop.dependence_shortname()))
                                          .arg(QString::fromStdString(prop.dependence_unit()))
                                          .arg(QString::fromStdString(prop.shortname()))
                                          .arg(QString::fromStdString(prop.unit())).toStdString());

                    propSection->SetValue("PROPERTY_DEPENDENCE_X", keysString.toStdString());
                    propSection->SetValue("PROPERTY_DEPENDENCE_Y", valuesString.toStdString());
                }
            }

        }

        ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/material.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &materialInfo, &info);

        // setHtml(...) doesn't work
        // webView->setHtml(QString::fromStdString(info));

        // load(...) works
        writeStringContent(tempProblemDir() + "/material.html", QString::fromStdString(info));
        webView->load(QUrl::fromLocalFile(tempProblemDir() + "/material.html"));
    }
}

void MaterialBrowserDialog::functionValues(const QString &function, double from, double to, int count, QList<double> *keys, QList<double> *values)
{
    if (function.isEmpty())
        return;

    double step = ((to - from) - 2 * EPS_ZERO) / (count - 1);

    ExpressionResult expressionResult = currentPythonEngineAgros()->runExpression(function, false);
    if (!expressionResult.error.isEmpty())
        qDebug() << "Function: " << expressionResult.error;

    for (int i = 0; i < count; i++)
    {
        double key = from + i * step + EPS_ZERO;

        ExpressionResult expressionResult = currentPythonEngineAgros()->runExpression(QString("agros2d_material(%1)").arg(key), true);
        if (expressionResult.error.isEmpty())
        {
            keys->append(key);
            values->append(expressionResult.value);
        }
        else
        {
            qDebug() << "Function eval: " << expressionResult.error;
        }
    }
    currentPythonEngineAgros()->runExpression("del agros2d_material", false);
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void MaterialBrowserDialog::linkClicked(const QUrlQuery &url)
#else
void MaterialBrowserDialog::linkClicked(const QUrl &url)
#endif
{
    QString search = "/property?";
    if (url.toString().contains(search))
    {
        m_selected_x.clear();
        m_selected_y.clear();

        QStringList keysString = url.queryItemValue("x").split(",");
        QStringList valuesString = url.queryItemValue("y").split(",");

        for (int j = 0; j < keysString.size(); j++)
        {
            m_selected_x.append(keysString.at(j).toDouble());
            m_selected_y.append(valuesString.at(j).toDouble());
        }

        accept();
    }
}

void MaterialBrowserDialog::doEdit()
{
    MaterialEditDialog dialog(m_selectedFilename, this);
    if (dialog.showDialog() == QDialog::Accepted)
    {
        materialInfo(m_selectedFilename);
    }
}
