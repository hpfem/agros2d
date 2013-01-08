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
            /*
            qDebug << tr("File '%1' cannot be opened (%2).").
                      arg(fileName).
                      arg(file.errorString());
                      */
            return;
        }

        if (!doc.setContent(&file)) {
            file.close();
            /*
            qDebug << tr("File '%1' is not valid material file.").arg(fileName);
            */
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
