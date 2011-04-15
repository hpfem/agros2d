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

#include "scenemarker.h"

#include "scene.h"
#include "gui.h"

SceneBoundary::SceneBoundary(const QString &name, PhysicFieldBC type)
{
    logMessage("SceneBoundary::SceneBoundary()");

    this->name = name;
    this->type = type;
}

QString SceneBoundary::html()
{
    logMessage("SceneBoundary::html()");

    QString out;
    out += "<h4>" + physicFieldString(Util::scene()->problemInfo()->physicField()) + "</h4>";
    out += "<table>";
    QMap<QString, QString> data = this->data();
    for (int j = 0; j < data.keys().length(); j++)
    {
        out += "<tr>";
        out += "<td>" + data.keys()[j] + ":</td>";
        out += "<td>" + data.values()[j] + "</td>";
        out += "</tr>";
    }
    out += "</table>";

    return out;
}

QVariant SceneBoundary::variant()
{
    logMessage("SceneBoundary::variant()");

    QVariant v;
    v.setValue(this);
    return v;
}

SceneBoundaryNone::SceneBoundaryNone() : SceneBoundary("none", PhysicFieldBC_None)
{
    logMessage("SceneBoundary::SceneBoundaryNone()");

}

// *************************************************************************************************************************************

SceneMaterial::SceneMaterial(const QString &name)
{
    logMessage("SceneMaterial::SceneMaterial()");

    this->name = name;
}

QString SceneMaterial::html()
{
    logMessage("SceneMaterial::html()");

    QString out;
    out += "<h4>" + physicFieldString(Util::scene()->problemInfo()->physicField()) + "</h4>";
    out += "<table>";
    QMap<QString, QString> data = this->data();
    for (int j = 0; j < data.keys().length(); j++)
    {
        out += "<tr>";
        out += "<td>" + data.keys()[j] + ":</td>";
        out += "<td>" + data.values()[j] + "</td>";
        out += "</tr>";
    }
    out += "</table>";

    return out;
}

QVariant SceneMaterial::variant()
{
    logMessage("SceneMaterial::variant()");

    QVariant v;
    v.setValue(this);
    return v;
}

SceneMaterialNone::SceneMaterialNone() : SceneMaterial("none")
{
    logMessage("SceneMaterial::SceneMaterialNone()");

}


// *************************************************************************************************************************************

SceneBoundaryDialog::SceneBoundaryDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("DSceneBoundary::DSceneBoundary()");

    layout = new QGridLayout();
    txtName = new QLineEdit(this);
    lblEquation = new QLabel(tr("Equation:"));
    lblEquationImage = new QLabel(this);
}

void SceneBoundaryDialog::createDialog()
{
    logMessage("DSceneBoundary::createDialog()");

    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    layout->addWidget(new QLabel(tr("Name:")), 0, 0);
    layout->addWidget(txtName, 0, 2);
    layout->addWidget(lblEquation, 5, 0);
    layout->addWidget(lblEquationImage, 5, 2);

    // content
    createContent();

    layout->addWidget(buttonBox, 100, 0, 1, 3);
    layout->setRowStretch(99, 1);

    txtName->setFocus();

    setLayout(layout);
}

void SceneBoundaryDialog::load()
{
    logMessage("DSceneBoundary::load()");

    txtName->setText(m_boundary->name);
}

bool SceneBoundaryDialog::save()
{
    logMessage("DSceneBoundary::save()");

    // find name duplicities
    foreach (SceneBoundary *boundary, Util::scene()->boundaries)
    {
        if (boundary->name == txtName->text())
        {
            if (m_boundary == boundary)
                continue;

            QMessageBox::warning(this, tr("Boundary marker"), tr("Boundary marker name already exists."));
            return false;
        }
    }
    m_boundary->name = txtName->text();
    return true;
}

void SceneBoundaryDialog::setSize()
{
    logMessage("DSceneBoundary::setSize()");

    setWindowIcon(icon("scene-edgemarker"));
    setWindowTitle(tr("Boundary condition"));

    setMinimumSize(sizeHint());
}

void SceneBoundaryDialog::doAccept()
{
    logMessage("DSceneBoundary::doAccept()");

    if (save())
        accept();    
}

void SceneBoundaryDialog::doReject()
{
    logMessage("DSceneBoundary::doReject()");

    reject();
}

void SceneBoundaryDialog::evaluated(bool isError)
{
    logMessage("DSceneBoundary::evaluated()");

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!isError);
}

void SceneBoundaryDialog::readEquation(QLabel *lblEquation, PhysicFieldBC type)
{
    QString fileName = QString(":/images/equations/%1/%2_%3.png")
            .arg(physicFieldToStringKey(Util::scene()->problemInfo()->physicField()))
            .arg(physicFieldBCToStringKey(type))
            .arg(analysisTypeToStringKey(Util::scene()->problemInfo()->analysisType));

    if (QFile::exists(fileName))
        // analysis dependand
        readPixmap(lblEquation, fileName);
    else
        // general form
        readPixmap(lblEquation, QString(":/images/equations/%1/%2.png")
                   .arg(physicFieldToStringKey(Util::scene()->problemInfo()->physicField()))
                   .arg(physicFieldBCToStringKey(type)));
}

// *************************************************************************************************************************************

SceneMaterialDialog::SceneMaterialDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("DSceneMaterial::DSceneMaterial()");

    layout = new QGridLayout();
    txtName = new QLineEdit(this);
    lblEquation = new QLabel(tr("Equation:"));
    lblEquationImage = new QLabel(this);
}

void SceneMaterialDialog::createDialog()
{
    logMessage("DSceneMaterial::createDialog()");

    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    // name
    layout->addWidget(new QLabel(tr("Name:")), 0, 0, 1, 2);
    layout->addWidget(txtName, 0, 2);

    // equation
    layout->addWidget(lblEquation, 1, 0, 1, 2);
    layout->addWidget(lblEquationImage, 1, 2);
    readPixmap(lblEquationImage,
               QString(":/images/equations/%1/%1_%2.png")
               .arg(physicFieldToStringKey(Util::scene()->problemInfo()->physicField()))
               .arg(analysisTypeToStringKey(Util::scene()->problemInfo()->analysisType)));

    // content
    createContent();

    layout->addWidget(buttonBox, 100, 0, 1, 3);
    layout->setRowStretch(99, 1);

    txtName->setFocus();

    setLayout(layout);
}

void SceneMaterialDialog::setSize()
{
    logMessage("DSceneMaterial::setSize()");

    setWindowIcon(icon("scene-labelmarker"));
    setWindowTitle(tr("Material"));

    setMinimumSize(sizeHint());
}

void SceneMaterialDialog::load()
{
    logMessage("DSceneMaterial::load()");

    txtName->setText(m_material->name);
}

bool SceneMaterialDialog::save()
{
    logMessage("DSceneMaterial::save()");

    // find name duplicities
    foreach (SceneMaterial *material, Util::scene()->materials)
    {
        if (material->name == txtName->text())
        {
            if (m_material == material)
                continue;

            QMessageBox::warning(this, tr("Material marker"), tr("Material marker name already exists."));
            return false;
        }
    }
    m_material->name = txtName->text();
    return true;
}

void SceneMaterialDialog::doAccept()
{
    logMessage("DSceneMaterial::doAccept()");

    if (save())
        accept();
}

void SceneMaterialDialog::doReject()
{
    logMessage("DSceneMaterial::doReject()");

    reject();
}

void SceneMaterialDialog::evaluated(bool isError)
{
    logMessage("DSceneMaterial::evaluated()");

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!isError);
}

// ***********************************************************************************************************

SceneBoundarySelectDialog::SceneBoundarySelectDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("SceneBoundarySelectDialog::SceneBoundarySelectDialog()");

    setWindowTitle(tr("Edge marker"));
    setWindowIcon(icon("scene-edge"));
    setModal(true);

    // fill combo
    cmbBoundary = new QComboBox(this);
    for (int i = 0; i<Util::scene()->boundaries.count(); i++)
    {
        cmbBoundary->addItem(Util::scene()->boundaries[i]->name, Util::scene()->boundaries[i]->variant());
    }

    // select marker
    cmbBoundary->setCurrentIndex(-1);
    SceneBoundary *boundary = NULL;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edges[i]->isSelected)
        {
            if (!boundary)
            {
                boundary = Util::scene()->edges[i]->boundary;
            }
            if (boundary != Util::scene()->edges[i]->boundary)
            {
                boundary = NULL;
                break;
            }
        }
    }
    if (boundary)
        cmbBoundary->setCurrentIndex(cmbBoundary->findData(boundary->variant()));

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QHBoxLayout *layoutBoundary = new QHBoxLayout();
    layoutBoundary->addWidget(new QLabel(tr("Boundary:")));
    layoutBoundary->addWidget(cmbBoundary);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutBoundary);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMaximumSize(sizeHint());
}

void SceneBoundarySelectDialog::doAccept()
{
    logMessage("SceneBoundarySelectDialog::doAccept()");

    if (boundary())
    {
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            if (Util::scene()->edges[i]->isSelected)
                Util::scene()->edges[i]->boundary = boundary();
        }
    }
    accept();
}

// *************************************************************************************************************************************

SceneMaterialSelectDialog::SceneMaterialSelectDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("SceneMaterialSelectDialog::SceneMaterialSelectDialog()");

    setWindowTitle(tr("Label marker"));
    setWindowIcon(icon("scene-label"));
    setModal(true);

    // fill combo
    cmbMaterial = new QComboBox(this);
    for (int i = 0; i<Util::scene()->materials.count(); i++)
    {
        cmbMaterial->addItem(Util::scene()->materials[i]->name, Util::scene()->materials[i]->variant());
    }

    // select marker
    cmbMaterial->setCurrentIndex(-1);
    SceneMaterial *marker = NULL;
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labels[i]->isSelected)
        {
            if (!marker)
            {
                marker = Util::scene()->labels[i]->material;
            }
            if (marker != Util::scene()->labels[i]->material)
            {
                marker = NULL;
                break;
            }
        }
    }
    if (marker)
        cmbMaterial->setCurrentIndex(cmbMaterial->findData(marker->variant()));

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QHBoxLayout *layoutMaterial = new QHBoxLayout();
    layoutMaterial->addWidget(new QLabel(tr("Material:")));
    layoutMaterial->addWidget(cmbMaterial);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutMaterial);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMaximumSize(sizeHint());
}

void SceneMaterialSelectDialog::doAccept()
{
    logMessage("SceneMaterialSelectDialog::doAccept()");

    if (marker())
    {
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            if (Util::scene()->labels[i]->isSelected)
                Util::scene()->labels[i]->material = marker();
        }
    }
    accept();
}
