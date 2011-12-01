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
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/dialog.h"
#include "scene.h"
#include "scenebasic.h"
#include "gui.h"

SceneBoundary::SceneBoundary(std::string field, std::string name, std::string type,
                             std::map<std::string, Value> values)
    : Boundary(field, name, type, values)
{
    logMessage("SceneBoundary::SceneBoundary()");
}

int SceneBoundary::showDialog(QWidget *parent)
{
    SceneBoundaryDialog *dialog = new SceneBoundaryCustomDialog(this, parent);
    if (dialog)
        return dialog->exec();
    else
        QMessageBox::information(QApplication::activeWindow(), QObject::tr(""), QObject::tr("Boundary dialog doesn't exists."));
}

QString SceneBoundary::script()
{
    // TODO
    // value
//    QString val = "{";
//    Hermes::Module::BoundaryType *boundary_type = Util::scene()->problemInfo()->module()->get_boundary_type(type);
//    for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin(); it < boundary_type->variables.end(); ++it)
//    {
//        Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);
//        val += "\"" + QString::fromStdString(variable->shortname) + "\" : " + values[variable->id].text() + ", ";
//    }

//    if (val.length() > 1)
//        val = val.left(val.length() - 2);
//    val += "}";

//    QString str;

//    str += QString("addboundary(\"%1\", \"%2\", %3)").
//            arg(QString::fromStdString(name)).
//            arg(QString::fromStdString(type)).
//            arg(val);

//    return str;
}

QString SceneBoundary::html()
{
//    TODO
//    QString out;
//    out += "<h4>" + QString::fromStdString(Util::scene()->problemInfo()->module()->name) + "</h4>";
//    out += "<table>";
    
//    Hermes::Module::BoundaryType *boundary_type = Util::scene()->problemInfo()->module()->get_boundary_type(type);
//    if (boundary_type)
//        for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin(); it < boundary_type->variables.end(); ++it)
//        {
//            Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);
            
//            out += "<tr>";
////            out += QString("<td>%1 (%2):</td>").
////                    arg(QString::fromStdString(variable->name)).
////                    arg(QString::fromStdString(variable->unit));
//            out += QString("<td>%1</td>").
//                    arg(values[variable->id].text());
//            out += "</tr>";
//        }
    
//    out += "</table>";
    
//    return out;
}

QVariant SceneBoundary::variant()
{
    logMessage("SceneBoundary::variant()");
    
    QVariant v;
    v.setValue(this);
    return v;
}

SceneBoundaryNone::SceneBoundaryNone() : SceneBoundary("", "none")
{
    logMessage("SceneBoundary::SceneBoundaryNone()");
}

// *************************************************************************************************************************************

SceneMaterial::SceneMaterial(std::string field, std::string name,
                             std::map<std::string, Value> values) : Material(field, name, values)
{
    logMessage("SceneMaterial::SceneMaterial()");
}

int SceneMaterial::showDialog(QWidget *parent)
{
    SceneMaterialDialog *dialog = new SceneMaterialCustomDialog(this, parent);
    if (dialog)
        return dialog->exec();
    else
        QMessageBox::information(QApplication::activeWindow(), QObject::tr(""), QObject::tr("Material dialog doesn't exists."));

}

QString SceneMaterial::script()
{
    assert(0); //TODO
//    // value
//    QString val = "{";
//    Hermes::vector<Hermes::Module::MaterialTypeVariable *> materials = Util::scene()->problemInfo()->module()->material_type_variables;
//    for (Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = materials.begin(); it < materials.end(); ++it)
//    {
//        Hermes::Module::MaterialTypeVariable *variable = ((Hermes::Module::MaterialTypeVariable *) *it);
//        val += "\"" + QString::fromStdString(variable->shortname) + "\" : " + values[variable->id].text() + ", ";
//    }

//    if (val.length() > 1)
//        val = val.left(val.length() - 2);
//    val += "}";

//    QString str;

//    str += QString("addmaterial(\"%1\", %2)").
//            arg(QString::fromStdString(name)).
//            arg(val);

//    return str;
}

QString SceneMaterial::html()
{
    assert(0); //TODO
//    logMessage("SceneMaterial::html()");
    
//    QString out;
//    out += "<h4>" + QString::fromStdString(Util::scene()->problemInfo()->module()->name) + "</h4>";
//    out += "<table>";
    
//    for (Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = Util::scene()->problemInfo()->module()->material_type_variables.begin();
//         it < Util::scene()->problemInfo()->module()->material_type_variables.end(); ++it )
//    {
//        Hermes::Module::MaterialTypeVariable *material = ((Hermes::Module::MaterialTypeVariable *) *it);
        
//        out += "<tr>";
////        out += QString("<td>%1 (%2)</td>").
////                arg(QString::fromStdString(material->name)).
////                arg(QString::fromStdString(material->unit));
//        // FIXME - add value
//        out += QString("<td>%1</td>").
//                arg(QString::fromStdString("FIXME"));
//        out += "</tr>";
//    }
//    out += "</table>";
    
//    return out;
}

QVariant SceneMaterial::variant()
{
    logMessage("SceneMaterial::variant()");
    
    QVariant v;
    v.setValue(this);
    return v;
}

SceneMaterialNone::SceneMaterialNone() : SceneMaterial("", "none")
{
    logMessage("SceneMaterial::SceneMaterialNone()");
}


// *************************************************************************************************************************************

SceneBoundaryDialog::SceneBoundaryDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("SceneBoundaryDialog::SceneBoundaryDialog()");
    
    setWindowIcon(icon("scene-edgemarker"));
    setWindowTitle(tr("Boundary condition"));
    
    layout = new QGridLayout();
    txtName = new QLineEdit(this);
    lblEquation = new QLabel(tr("Equation:"));
    lblEquationImage = new QLabel(this);
}

void SceneBoundaryDialog::createDialog()
{
    logMessage("SceneBoundaryDialog::createDialog()");
    
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
    logMessage("SceneBoundaryDialog::load()");
    
    txtName->setText(QString::fromStdString(m_boundary->getName()));
}

bool SceneBoundaryDialog::save()
{
    logMessage("SceneBoundaryDialog::save()");
    
    // find name duplicities
    foreach (SceneBoundary *boundary, Util::scene()->boundaries->items())
    {
        if (QString::fromStdString(boundary->getName()) == txtName->text())
        {
            if (m_boundary == boundary)
                continue;
            
            QMessageBox::warning(this, tr("Boundary marker"), tr("Boundary marker name already exists."));
            return false;
        }
    }
    m_boundary->setName(txtName->text().toStdString());
    return true;
}

void SceneBoundaryDialog::setSize()
{
    logMessage("SceneBoundaryDialog::setSize()");
    
    setMinimumSize(sizeHint());
}

void SceneBoundaryDialog::doAccept()
{
    logMessage("SceneBoundaryDialog::doAccept()");
    
    if (save())
        accept();    
}

void SceneBoundaryDialog::doReject()
{
    logMessage("SceneBoundaryDialog::doReject()");
    
    reject();
}

void SceneBoundaryDialog::evaluated(bool isError)
{
    logMessage("SceneBoundaryDialog::evaluated()");
    
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!isError);
}

void SceneBoundaryDialog::readEquation(QLabel *lblEquation, const QString &type)
{
    assert(0); //TODO
//    QString fileName = QString(":/images/equations/%1/%2_%3.png")
//            .arg(QString::fromStdString(Util::scene()->problemInfo()->module()->name))
//            .arg(type)
//            .arg(analysisTypeToStringKey(Util::scene()->problemInfo()->analysisType));
    
//    if (QFile::exists(fileName))
//        // analysis dependand
//        readPixmap(lblEquation, fileName);
//    else
//        // general form
//        readPixmap(lblEquation, QString(":/images/equations/%1/%2.png")
//                   .arg(QString::fromStdString(Util::scene()->problemInfo()->module()->id))
//                   .arg(type));
}

// *************************************************************************************************************************************

SceneMaterialDialog::SceneMaterialDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("DSceneMaterial::DSceneMaterial()");
    
    setWindowIcon(icon("scene-labelmarker"));
    setWindowTitle(tr("Material"));
    
    layout = new QGridLayout();
    txtName = new QLineEdit(this);
    lblEquation = new QLabel(tr("Equation:"));
    lblEquationImage = new QLabel(this);
}

void SceneMaterialDialog::createDialog()
{
    assert(0); //TODO
//    logMessage("DSceneMaterial::createDialog()");
    
//    // dialog buttons
//    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
//    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
//    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));
    
//    // name
//    layout->addWidget(new QLabel(tr("Name:")), 0, 0, 1, 2);
//    layout->addWidget(txtName, 0, 2);
    
//    // equation
//    layout->addWidget(lblEquation, 1, 0, 1, 2);
//    layout->addWidget(lblEquationImage, 1, 2);
//    readPixmap(lblEquationImage,
//               QString(":/images/equations/%1/%1_%2.png")
//               .arg(QString::fromStdString(Util::scene()->problemInfo()->module()->id))
//               .arg(analysisTypeToStringKey(Util::scene()->problemInfo()->analysisType)));
    
//    // content
//    createContent();
    
//    layout->addWidget(buttonBox, 100, 0, 1, 3);
//    layout->setRowStretch(99, 1);
    
//    txtName->setFocus();
    
//    setLayout(layout);
}

void SceneMaterialDialog::setSize()
{
    logMessage("DSceneMaterial::setSize()");
    
    setMinimumSize(sizeHint());
}

void SceneMaterialDialog::load()
{
    logMessage("DSceneMaterial::load()");
    
    txtName->setText(QString::fromStdString(m_material->getName()));
}

bool SceneMaterialDialog::save()
{
    logMessage("DSceneMaterial::save()");
    
    // find name duplicities
    foreach (SceneMaterial *material, Util::scene()->materials->items())
    {
        if (material->getName() == txtName->text().toStdString())
        {
            if (m_material == material)
                continue;
            
            QMessageBox::warning(this, tr("Material marker"), tr("Material marker name already exists."));
            return false;
        }
    }
    m_material->setName(txtName->text().toStdString());
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
    assert(0); //TODO
//    logMessage("SceneBoundarySelectDialog::SceneBoundarySelectDialog()");
    
//    setWindowTitle(tr("Boundary condition"));
//    setWindowIcon(icon("scene-edge"));
//    setModal(true);
    
//    // fill combo
//    cmbBoundary = new QComboBox(this);
//    for (int i = 0; i<Util::scene()->boundaries.count(); i++)
//    {
//        cmbBoundary->addItem(QString::fromStdString(Util::scene()->boundaries[i]->name), Util::scene()->boundaries[i]->variant());
//    }
    
//    // select marker
//    cmbBoundary->setCurrentIndex(-1);
//    SceneBoundary *boundary = NULL;
//    for (int i = 0; i<Util::scene()->edges.count(); i++)
//    {
//        if (Util::scene()->edges[i]->isSelected)
//        {
//            if (!boundary)
//            {
//                boundary = Util::scene()->edges[i]->boundary;
//            }
//            if (boundary != Util::scene()->edges[i]->boundary)
//            {
//                boundary = NULL;
//                break;
//            }
//        }
//    }
//    if (boundary)
//        cmbBoundary->setCurrentIndex(cmbBoundary->findData(boundary->variant()));
    
//    // dialog buttons
//    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
//    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
//    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
//    QHBoxLayout *layoutBoundary = new QHBoxLayout();
//    layoutBoundary->addWidget(new QLabel(tr("Boundary:")));
//    layoutBoundary->addWidget(cmbBoundary);
    
//    QVBoxLayout *layout = new QVBoxLayout();
//    layout->addLayout(layoutBoundary);
//    layout->addStretch();
//    layout->addWidget(buttonBox);
    
//    setLayout(layout);
    
//    setMaximumSize(sizeHint());
}

void SceneBoundarySelectDialog::doAccept()
{
    logMessage("SceneBoundarySelectDialog::doAccept()");
    
    if (boundary())
    {
        for (int i = 0; i<Util::scene()->edges->length(); i++)
        {
            if (Util::scene()->edges->at(i)->isSelected)
                Util::scene()->edges->at(i)->marker = boundary();
        }
    }
    accept();
}

// *************************************************************************************************************************************

SceneMaterialSelectDialog::SceneMaterialSelectDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("SceneMaterialSelectDialog::SceneMaterialSelectDialog()");
    
    setWindowTitle(tr("Material"));
    setWindowIcon(icon("scene-label"));
    setModal(true);
    
    // fill combo
    cmbMaterial = new QComboBox(this);
    foreach (SceneMaterial *material, Util::scene()->materials->items())
    {
        cmbMaterial->addItem(QString::fromStdString(material->getName()),
                             material->variant());
    }
    
    // select marker
    cmbMaterial->setCurrentIndex(-1);
    SceneMaterial *marker = NULL;
    for (int i = 0; i<Util::scene()->labels->length(); i++)
    {
        if (Util::scene()->labels->at(i)->isSelected)
        {
            if (!marker)
            {
                marker = Util::scene()->labels->at(i)->marker;
            }
            if (marker != Util::scene()->labels->at(i)->marker)
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
        for (int i = 0; i<Util::scene()->labels->length(); i++)
        {
            if (Util::scene()->labels->at(i)->isSelected)
                Util::scene()->labels->at(i)->marker = marker();
        }
    }
    accept();
}
