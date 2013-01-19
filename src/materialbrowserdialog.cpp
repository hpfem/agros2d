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
#include "datatable.h"

#include "gui/chart.h"

/*
struct MaterialProperty
{
    QString id;
    QString name;
    QString shortname;
    QString unit;
    QString dependence_shortname;
    QString dependence_unit;
    QString source;
    QString key;
    QString value;
};
Q_DECLARE_METATYPE(MaterialProperty)

MaterialBrowserDialog::MaterialBrowserDialog(QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon(""));
    setWindowTitle(tr("Material browser"));

    createControls();

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
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_select);

    return exec();
}

void MaterialBrowserDialog::createControls()
{
    trvMaterial = new QTreeWidget(this);
    // trvMaterial->setContextMenuPolicy(Qt::CustomContextMenu);
    trvMaterial->setMouseTracking(true);
    trvMaterial->setColumnCount(1);
    trvMaterial->setColumnWidth(0, 150);
    trvMaterial->setIndentation(12);

    connect(trvMaterial, SIGNAL(itemPressed(QTreeWidgetItem *, int)), this, SLOT(doMaterialSelected(QTreeWidgetItem *, int)));
    connect(trvMaterial, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(doMaterialSelected(QTreeWidgetItem *, int)));
    connect(trvMaterial, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doMaterialSelected(QTreeWidgetItem *, QTreeWidgetItem *)));

    QStringList materialLabels;
    materialLabels << tr("Material");
    trvMaterial->setHeaderLabels(materialLabels);

    trvProperty = new QTreeWidget(this);
    // trvProperty->setContextMenuPolicy(Qt::CustomContextMenu);
    trvProperty->setMouseTracking(true);
    trvProperty->setColumnCount(1);
    trvProperty->setColumnWidth(0, 150);
    trvProperty->setIndentation(12);

    connect(trvProperty, SIGNAL(itemPressed(QTreeWidgetItem *, int)), this, SLOT(doPropertySelected(QTreeWidgetItem *, int)));
    connect(trvProperty, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(doPropertySelected(QTreeWidgetItem *, int)));
    connect(trvProperty, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doPropertySelected(QTreeWidgetItem *, QTreeWidgetItem *)));

    QStringList propertyLabels;
    propertyLabels << tr("Property");
    trvProperty->setHeaderLabels(propertyLabels);

    lblMaterial = new QLabel("-");
    lblProperty = new QLabel("-");
    lblShortname = new QLabel("-");
    lblDependenceShortname = new QLabel("-");
    lblSource = new QLabel("-");
    lblSource->setWordWrap(true);
    lblSource->setMaximumWidth(350);
    lblValue = new QLabel("0.0");

    chartValue = new Chart(this);

    QVBoxLayout *layoutChartValue = new QVBoxLayout();
    layoutChartValue->addWidget(chartValue);

    QGroupBox *grpChartValue = new QGroupBox(tr("Chart"));
    grpChartValue->setLayout(layoutChartValue);

    QGridLayout *layoutProperty = new QGridLayout();
    layoutProperty->addWidget(new QLabel(tr("Material:")), 0, 0);
    layoutProperty->addWidget(lblMaterial, 0, 1);
    layoutProperty->addWidget(new QLabel(tr("Property:")), 1, 0);
    layoutProperty->addWidget(lblProperty, 1, 1);
    layoutProperty->addWidget(new QLabel(tr("Variable:")), 2, 0);
    layoutProperty->addWidget(lblShortname, 2, 1);
    layoutProperty->addWidget(new QLabel(tr("Dependence:")), 3, 0);
    layoutProperty->addWidget(lblDependenceShortname, 3, 1);
    layoutProperty->addWidget(new QLabel(tr("Source:")), 4, 0);
    layoutProperty->addWidget(lblSource, 4, 1);
    layoutProperty->addWidget(new QLabel(tr("Value:")), 5, 0);
    layoutProperty->addWidget(lblValue, 5, 1);
    layoutProperty->setRowStretch(6, 1);
    layoutProperty->setColumnStretch(1, 1);

    QGroupBox *grpProperty = new QGroupBox(tr("Property"));
    grpProperty->setLayout(layoutProperty);

    QVBoxLayout *layoutPropertyChart = new QVBoxLayout();
    layoutPropertyChart->addWidget(grpProperty);
    layoutPropertyChart->addWidget(grpChartValue, 1);

    QHBoxLayout *layoutMaterials = new QHBoxLayout();
    layoutMaterials->addWidget(trvMaterial);
    layoutMaterials->addWidget(trvProperty);
    layoutMaterials->addLayout(layoutPropertyChart, 1);

    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutMaterials, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}


void MaterialBrowserDialog::readMaterials()
{
    QDir dir;
    dir.setPath(datadir() + "/resources/materials");

    // add all translations
    QStringList filters;
    filters << "*.xml";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    // remove extension
    QStringList list = dir.entryList();
    list.replaceInStrings(".xml", "");

    trvMaterial->clear();

    for (int i = 0; i < list.count(); i++)
    {
        QTreeWidgetItem *node = new QTreeWidgetItem(trvMaterial);
        node->setText(0, list.at(i));
    }
}

void MaterialBrowserDialog::doMaterialSelected(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    doMaterialSelected(current, Qt::UserRole);
}

void MaterialBrowserDialog::doMaterialSelected(QTreeWidgetItem *item, int role)
{
    if (item != NULL)
    {
        trvProperty->clear();
        QString fileName = datadir() + "/resources/materials/" + item->text(0) + ".xml";

        QDomDocument doc;
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {

            // qDebug << tr("File '%1' cannot be opened (%2).").
            //           arg(fileName).
            //           arg(file.errorString());

            return;
        }

        if (!doc.setContent(&file)) {
            file.close();
            // qDebug << tr("File '%1' is not valid material file.").arg(fileName);
            return;
        }
        file.close();

        // main document
        QDomElement eleDoc = doc.documentElement();

        QDomNode eleProperty = eleDoc.toElement().elementsByTagName("property").at(0);
        QDomNode n = eleProperty.firstChild();
        while(!n.isNull())
        {
            QDomElement element = n.toElement();

            MaterialProperty material;

            material.id = element.toElement().attribute("id");
            material.name = element.toElement().attribute("name");
            material.shortname = element.toElement().attribute("shortname");
            material.dependence_shortname = element.toElement().attribute("dependence_shortname");
            material.dependence_unit = element.toElement().attribute("dependence_unit");
            material.unit = element.toElement().attribute("unit");
            material.source = element.toElement().attribute("source");
            material.key = element.toElement().attribute("key");
            material.value = element.toElement().attribute("value");

            QVariant v;
            v.setValue(material);

            QTreeWidgetItem *node = new QTreeWidgetItem(trvProperty);
            node->setText(0, material.name);
            node->setData(0, Qt::UserRole, v);

            n = n.nextSibling();
        }

        if (trvProperty->topLevelItemCount() > 0.0)
        {
            trvProperty->topLevelItem(0)->setSelected(true);
            doPropertySelected(trvProperty->topLevelItem(0), trvProperty->topLevelItem(0));
        }
    }
}

void MaterialBrowserDialog::doPropertySelected(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    doPropertySelected(current, Qt::UserRole);
}

void MaterialBrowserDialog::doPropertySelected(QTreeWidgetItem *item, int role)
{
    m_x.clear();
    m_y.clear();

    if (item != NULL)
    {
        MaterialProperty material = item->data(0, Qt::UserRole).value<MaterialProperty>();

        lblMaterial->setText(trvMaterial->currentItem()->text(0));
        lblProperty->setText(material.name);
        lblShortname->setText(QString("%1 (%2)").arg(material.shortname).arg(material.unit));
        lblDependenceShortname->setText(QString("%1 (%2)").arg(material.dependence_shortname).arg(material.dependence_unit));
        lblSource->setText(material.source);

        if (!material.key.isEmpty())
        {
            // nonlinear material
            lblValue->setText(tr("nonlinear material"));
            chartValue->setVisible(true);

            // axes
            chartValue->setAxisTitle(QwtPlot::yLeft, lblShortname->text());
            chartValue->setAxisTitle(QwtPlot::xBottom, lblDependenceShortname->text());

            // data
            QStringList strx = material.key.split(",");
            QStringList stry = material.value.split(",");

            double *x = new double[strx.count()];
            double *y = new double[stry.count()];
            for (int i = 0; i < strx.count(); i++)
            {
                x[i] = strx.at(i).toDouble();
                y[i] = stry.at(i).toDouble();

                m_x.append(x[i]);
                m_y.append(y[i]);
            }

            chartValue->setData(x, y, strx.count());

            // clean up
            delete [] x;
            delete [] y;
        }
        else
        {
            // linear material
            lblValue->setText(material.value);
            chartValue->setVisible(false);
        }
    }
}

void MaterialBrowserDialog::doAccept()
{
    accept();
}
*/
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

#include "scene.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "moduledialog.h"

#include "gui/lineeditdouble.h"
#include "gui/latexviewer.h"
#include "gui/common.h"

#include "hermes2d/module.h"

#include "ctemplate/template.h"

#include "../resources_source/classes/material_xml.h";

MaterialBrowserDialog::MaterialBrowserDialog(QWidget *parent) : QDialog(parent),
    m_select(false)
{
    setWindowTitle(tr("Material Browser"));
    setModal(true);

    // problem information
    webView = new QWebView(this);

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

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widget, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    readMaterials();

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

MaterialBrowserDialog::~MaterialBrowserDialog()
{
}

void MaterialBrowserDialog::doAccept()
{
    accept();
}

void MaterialBrowserDialog::doReject()
{
    reject();
}

int MaterialBrowserDialog::showDialog(bool select)
{
    m_select = select;
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_select);

    return exec();
}

void MaterialBrowserDialog::doItemSelected(QTreeWidgetItem *item, int column)
{
    m_selectedFilename = item->data(0, Qt::UserRole).toString();
    if (!m_selectedFilename.isEmpty())
    {
        materialInfo(m_selectedFilename);
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    else
    {
        webView->setHtml("");
    }
}

void MaterialBrowserDialog::doItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (trvMaterial->currentItem())
    {
        m_selectedFilename = trvMaterial->currentItem()->data(0, Qt::UserRole).toString();
        accept();
    }
}

void MaterialBrowserDialog::readMaterials()
{
    try
    {
        // general
        XMLMaterial::general general("Aluminum");
        general.description().set("desc.");

        // property
        XMLMaterial::property prop1("thermal_conductivity", "Thermal conductivity 1", "lambda", "W/m.K", "T", "deg.", "C.Y. Ho, R.W. Powell and P.E. Liley, J. Phys. Chem. Ref. Data, v1, p279 (1972)");
        prop1.constant().set(XMLMaterial::constant(100));
        prop1.function().set(XMLMaterial::function("def A514_UNS_K11872_spec_heat_0(t) :"
                                                   "# ASM Handbook, vol 1, 10th edition, ASM International (1990)"
                                                   "# bal. Fe, 0.15-0.21 C, 0.7 Cr, 0.95 Mn, 0.5 Mo, 0.7 Si, 0.08 Zr,"
                                                   "# 0.0025 B max (wt%)"
                                                   "# same data as 1030 steel; UNS K11872"
                                                   "# data is in units of J/(kg-K)"
                                                   "# t must be in degrees Kelvin for these equations"
                                                   "if t &gt;= 293.0 and t &lt;= 848.0 :"
                                                   "    return -1.078824e-08*t*t*t*t +2.414973e-05*t*t*t -1.834293e-02*t*t +6.018500e+00*t -2.157306e+02"
                                                   "else :"
                                                   "    return 1.000000e+100", 0, 848));

        XMLMaterial::property prop2("thermal_conductivity", "Thermal conductivity", "lambda", "W/m.K", "T", "deg.", "C.Y. Ho, R.W. Powell and P.E. Liley, J. Phys. Chem. Ref. Data, v1, p279 (1972)");
        prop2.constant().set(XMLMaterial::constant(238));
        prop2.table().set(XMLMaterial::table("-173.16,-156.16,-139.16,-122.16,-105.16,-88.16,-71.16,-54.16,-37.16,-20.16,-3.16,13.84,30.84,47.84,64.84,81.84,98.84,115.84,132.84,149.84,166.84,183.84,200.84,217.84,234.84,251.84,268.84,285.84,302.84,319.84,336.84,353.84,370.84,387.84,404.84,421.84,438.84,455.84,472.84,489.84,506.84,523.84,540.84,557.84,574.84,591.84,608.84,625.84,642.84,659.84",
                                             "300.847,272.734599091,255.320111188,245.451888257,240.509215237,238.402310044,237.572323566,236.991339667,236.162375186,235.119379934,234.4272367,235.181761244,237.537361254,238.699849161,239.472587025,239.914252479,240.077201716,240.007810588,239.746815718,239.329655603,238.786811725,238.14414965,237.423260141,236.641800264,235.813834488,234.950175801,234.058726808,233.144820843,232.211563073,231.260171604,230.29031859,229.300471336,228.288233408,227.250685736,226.184727723,225.08741835,223.956317284,222.789825983,221.587528802,220.350534102,219.081815354,217.786552245,216.472471787,215.150189424,213.833550132,212.539969534,211.290775,210.111546758,209.032458997,208.088620975"));

        // properties
        XMLMaterial::properties properties;
        properties.property().push_back(prop1);
        properties.property().push_back(prop2);

        // material
        XMLMaterial::material material(general, properties);

        // general
        // material.general().name().set("x");

        std::string mesh_schema_location("");

        // TODO: set path more general
        mesh_schema_location.append(QString("%1/material_xml.xsd").arg(QFileInfo(datadir() + XSDROOT).absoluteFilePath()).toStdString());
        ::xml_schema::namespace_info namespace_info_mesh("XMLMaterial", mesh_schema_location);

        ::xml_schema::namespace_infomap namespace_info_map;
        namespace_info_map.insert(std::pair<std::basic_string<char>, xml_schema::namespace_info>("material", namespace_info_mesh));

        std::ofstream out("/home/karban/Prvek.mat");
        XMLMaterial::material_(out, material, namespace_info_map);
    }
    catch (const xml_schema::exception& e)
    {
        std::cerr << e << std::endl;
    }

    // clear listview
    trvMaterial->clear();

    QDir dir(QString("%1/resources/materials").arg(datadir()));
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);

    readMaterials(dir, trvMaterial->invisibleRootItem());
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
            // if (icons.count() == 1)
            //     exampleItem->setIcon(0, icons.at(0));
            // else
            //    exampleItem->setIcon(0, icon("fields/empty"));
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
        material_xsd = XMLMaterial::material_(fileName.toStdString().c_str());
        XMLMaterial::material *material = material_xsd.get();

        // stylesheet
        std::string style;
        ctemplate::TemplateDictionary stylesheet("style");
        stylesheet.SetValue("FONTFAMILY", QApplication::font().family().toStdString());
        stylesheet.SetValue("FONTSIZE", (QString("%1").arg(QApplication::font().pointSize() + 1).toStdString()));

        ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/problem_style.tpl", ctemplate::DO_NOT_STRIP, &stylesheet, &style);

        // template
        std::string info;
        ctemplate::TemplateDictionary materialInfo("info");

        materialInfo.SetValue("STYLESHEET", style);
        materialInfo.SetValue("JS_DIRECTORY", QString("%1%2").arg(QDir(datadir()).absolutePath()).arg(TEMPLATEROOT + "/panels").toStdString());

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

            if (prop.constant().present())
            {
                XMLMaterial::constant constant = prop.constant().get();
                propSection->SetValue("PROPERTY_CONSTANT_LABEL", tr("Constant:").toStdString());
                propSection->SetValue("PROPERTY_CONSTANT", QString::number(constant.value()).toStdString());
            }

            if (prop.table().present())
            {
                XMLMaterial::table table = prop.table().get();

                QStringList keys = QString::fromStdString(table.keys()).split(",");
                QStringList values = QString::fromStdString(table.values()).split(",");
                assert(keys.size() == values.size());

                QString chart = "[";
                for (int i = 0; i < keys.size(); i++)
                    chart += QString("[%1, %2], ").arg(keys[i]).arg(values[i]);
                chart += "]";

                propSection->SetValue("PROPERTY_CHART", QString("chart_%1").arg(QString::fromStdString(prop.id())).toStdString());
                propSection->SetValue("PROPERTY_CHART_SCRIPT", QString("<script type=\"text/javascript\">$(function () { $.plot($(\"#chart_%1\"), [ { data: %2, color: \"rgb(61, 61, 251)\", lines: { show: true }, points: { show: true } } ], { grid: { hoverable : true } });});</script>")
                                       .arg(QString::fromStdString(prop.id())).arg(chart).toStdString());
            }
        }

        ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/material.tpl", ctemplate::DO_NOT_STRIP, &materialInfo, &info);

        // setHtml(...) doesn't work
        // webView->setHtml(QString::fromStdString(info));

        // load(...) works
        writeStringContent(tempProblemDir() + "/material.html", QString::fromStdString(info));
        webView->load(tempProblemDir() + "/material.html");
    }
}
