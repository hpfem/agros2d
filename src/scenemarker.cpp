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

SceneEdgeMarker::SceneEdgeMarker(const QString &name, PhysicFieldBC type)
{
    logMessage("SceneEdgeMarker::SceneEdgeMarker()");

    this->name = name;
    this->type = type;
}

QString SceneEdgeMarker::html()
{
    logMessage("SceneEdgeMarker::html()");

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

QVariant SceneEdgeMarker::variant()
{
    logMessage("SceneEdgeMarker::variant()");

    QVariant v;
    v.setValue(this);
    return v;
}

SceneEdgeMarkerNone::SceneEdgeMarkerNone() : SceneEdgeMarker("none", PhysicFieldBC_None)
{
    logMessage("SceneEdgeMarker::SceneEdgeMarkerNone()");

}

// *************************************************************************************************************************************

SceneLabelMarker::SceneLabelMarker(const QString &name)
{
    logMessage("SceneLabelMarker::SceneLabelMarker()");

    this->name = name;
}

QString SceneLabelMarker::html()
{
    logMessage("SceneLabelMarker::html()");

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

QVariant SceneLabelMarker::variant()
{
    logMessage("SceneLabelMarker::variant()");

    QVariant v;
    v.setValue(this);
    return v;
}

SceneLabelMarkerNone::SceneLabelMarkerNone() : SceneLabelMarker("none")
{
    logMessage("SceneLabelMarker::SceneLabelMarkerNone()");

}


// *************************************************************************************************************************************

DSceneEdgeMarker::DSceneEdgeMarker(QWidget *parent) : QDialog(parent)
{
    logMessage("DSceneEdgeMarker::DSceneEdgeMarker()");

    layout = new QGridLayout();
    txtName = new QLineEdit(this);
    lblEquation = new QLabel(tr("Equation:"));
    lblEquationImage = new QLabel(this);
}

void DSceneEdgeMarker::createDialog()
{
    logMessage("DSceneEdgeMarker::createDialog()");

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

void DSceneEdgeMarker::load()
{
    logMessage("DSceneEdgeMarker::load()");

    txtName->setText(m_edgeMarker->name);
}

bool DSceneEdgeMarker::save()
{
    logMessage("DSceneEdgeMarker::save()");

    // find name duplicities
    foreach (SceneEdgeMarker *edgeMarker, Util::scene()->edgeMarkers)
    {
        if (edgeMarker->name == txtName->text())
        {
            if (m_edgeMarker == edgeMarker)
                continue;

            QMessageBox::warning(this, tr("Boundary marker"), tr("Boundary marker name already exists."));
            return false;
        }
    }
    m_edgeMarker->name = txtName->text();
    return true;
}

void DSceneEdgeMarker::setSize()
{
    logMessage("DSceneEdgeMarker::setSize()");

    setWindowIcon(icon("scene-edgemarker"));
    setWindowTitle(tr("Boundary condition"));

    setMinimumSize(sizeHint());
}

void DSceneEdgeMarker::doAccept()
{
    logMessage("DSceneEdgeMarker::doAccept()");

    if (save())
        accept();    
}

void DSceneEdgeMarker::doReject()
{
    logMessage("DSceneEdgeMarker::doReject()");

    reject();
}

void DSceneEdgeMarker::evaluated(bool isError)
{
    logMessage("DSceneEdgeMarker::evaluated()");

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!isError);
}

void DSceneEdgeMarker::readEquation(QLabel *lblEquation, PhysicFieldBC type)
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

DSceneLabelMarker::DSceneLabelMarker(QWidget *parent) : QDialog(parent)
{
    logMessage("DSceneLabelMarker::DSceneLabelMarker()");

    layout = new QGridLayout();
    txtName = new QLineEdit(this);
    lblEquation = new QLabel(tr("Equation:"));
    lblEquationImage = new QLabel(this);
}

void DSceneLabelMarker::createDialog()
{
    logMessage("DSceneLabelMarker::createDialog()");

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

void DSceneLabelMarker::setSize()
{
    logMessage("DSceneLabelMarker::setSize()");

    setWindowIcon(icon("scene-labelmarker"));
    setWindowTitle(tr("Material"));

    setMinimumSize(sizeHint());
}

void DSceneLabelMarker::load()
{
    logMessage("DSceneLabelMarker::load()");

    txtName->setText(m_labelMarker->name);
}

bool DSceneLabelMarker::save()
{
    logMessage("DSceneLabelMarker::save()");

    // find name duplicities
    foreach (SceneLabelMarker *labelMarker, Util::scene()->labelMarkers)
    {
        if (labelMarker->name == txtName->text())
        {
            if (m_labelMarker == labelMarker)
                continue;

            QMessageBox::warning(this, tr("Material marker"), tr("Material marker name already exists."));
            return false;
        }
    }
    m_labelMarker->name = txtName->text();
    return true;
}

void DSceneLabelMarker::doAccept()
{
    logMessage("DSceneLabelMarker::doAccept()");

    if (save())
        accept();
}

void DSceneLabelMarker::doReject()
{
    logMessage("DSceneLabelMarker::doReject()");

    reject();
}

void DSceneLabelMarker::evaluated(bool isError)
{
    logMessage("DSceneLabelMarker::evaluated()");

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!isError);
}

// ***********************************************************************************************************

EdgeMarkerDialog::EdgeMarkerDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("EdgeMarkerDialog::EdgeMarkerDialog()");

    setWindowTitle(tr("Edge marker"));
    setWindowIcon(icon("scene-edge"));
    setModal(true);

    // fill combo
    cmbMarker = new QComboBox(this);
    for (int i = 0; i<Util::scene()->edgeMarkers.count(); i++)
    {
        cmbMarker->addItem(Util::scene()->edgeMarkers[i]->name, Util::scene()->edgeMarkers[i]->variant());
    }

    // select marker
    cmbMarker->setCurrentIndex(-1);
    SceneEdgeMarker *marker = NULL;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edges[i]->isSelected)
        {
            if (!marker)
            {
                marker = Util::scene()->edges[i]->marker;
            }
            if (marker != Util::scene()->edges[i]->marker)
            {
                marker = NULL;
                break;
            }
        }
    }
    if (marker)
        cmbMarker->setCurrentIndex(cmbMarker->findData(marker->variant()));

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QHBoxLayout *layoutMarker = new QHBoxLayout();
    layoutMarker->addWidget(new QLabel(tr("Edge marker:")));
    layoutMarker->addWidget(cmbMarker);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutMarker);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMaximumSize(sizeHint());
}

EdgeMarkerDialog::~EdgeMarkerDialog()
{
    logMessage("EdgeMarkerDialog::~EdgeMarkerDialog()");

    delete cmbMarker;
}

void EdgeMarkerDialog::doAccept()
{
    logMessage("EdgeMarkerDialog::doAccept()");

    if (marker())
    {
        for (int i = 0; i<Util::scene()->edges.count(); i++)
        {
            if (Util::scene()->edges[i]->isSelected)
                Util::scene()->edges[i]->marker = marker();
        }
    }
    accept();
}

// *************************************************************************************************************************************

LabelMarkerDialog::LabelMarkerDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("LabelMarkerDialog::LabelMarkerDialog()");

    setWindowTitle(tr("Label marker"));
    setWindowIcon(icon("scene-label"));
    setModal(true);

    // fill combo
    cmbMarker = new QComboBox(this);
    for (int i = 0; i<Util::scene()->labelMarkers.count(); i++)
    {
        cmbMarker->addItem(Util::scene()->labelMarkers[i]->name, Util::scene()->labelMarkers[i]->variant());
    }

    // select marker
    cmbMarker->setCurrentIndex(-1);
    SceneLabelMarker *marker = NULL;
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labels[i]->isSelected)
        {
            if (!marker)
            {
                marker = Util::scene()->labels[i]->marker;
            }
            if (marker != Util::scene()->labels[i]->marker)
            {
                marker = NULL;
                break;
            }
        }
    }
    if (marker)
        cmbMarker->setCurrentIndex(cmbMarker->findData(marker->variant()));

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QHBoxLayout *layoutMarker = new QHBoxLayout();
    layoutMarker->addWidget(new QLabel(tr("Label marker:")));
    layoutMarker->addWidget(cmbMarker);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutMarker);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMaximumSize(sizeHint());
}

LabelMarkerDialog::~LabelMarkerDialog()
{
    logMessage("LabelMarkerDialog::~LabelMarkerDialog()");

    delete cmbMarker;
}

void LabelMarkerDialog::doAccept()
{
    logMessage("LabelMarkerDialog::doAccept()");

    if (marker())
    {
        for (int i = 0; i<Util::scene()->labels.count(); i++)
        {
            if (Util::scene()->labels[i]->isSelected)
                Util::scene()->labels[i]->marker = marker();
        }
    }
    accept();
}
