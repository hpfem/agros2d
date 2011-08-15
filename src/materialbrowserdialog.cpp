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

MaterialBrowserDialog::MaterialBrowserDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("MaterialBrowserDialog::MaterialBrowserDialog()");

    setWindowIcon(icon(""));
    setWindowTitle(tr("Material browser"));

    createControls();

    readMaterials();
}

void MaterialBrowserDialog::createControls()
{
    logMessage("MaterialBrowserDialog::createControls()");

    trvMaterial = new QTreeWidget(this);
    // trvMaterial->setContextMenuPolicy(Qt::CustomContextMenu);
    trvMaterial->setMouseTracking(true);
    trvMaterial->setColumnCount(1);
    trvMaterial->setColumnWidth(0, 150);
    trvMaterial->setIndentation(12);

    QStringList materialLabels;
    materialLabels << tr("Material");
    trvMaterial->setHeaderLabels(materialLabels);

    trvProperty = new QTreeWidget(this);
    // trvProperty->setContextMenuPolicy(Qt::CustomContextMenu);
    trvProperty->setMouseTracking(true);
    trvProperty->setColumnCount(1);
    trvProperty->setColumnWidth(0, 150);
    trvProperty->setIndentation(12);

    QStringList propertyLabels;
    propertyLabels << tr("Property");
    trvProperty->setHeaderLabels(propertyLabels);

    QWidget *info = new QWidget(this);
    info->setMinimumWidth(300);

    QHBoxLayout *layoutMaterials = new QHBoxLayout();
    layoutMaterials->addWidget(trvMaterial);
    layoutMaterials->addWidget(trvProperty);
    layoutMaterials->addWidget(info);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

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

    QStringList list = dir.entryList();

    trvMaterial->clear();

    for (int i = 0; i < list.count(); i++)
    {
        QTreeWidgetItem *node = new QTreeWidgetItem(trvMaterial);
        node->setText(0, list.at(i));
    }
}
