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

#include "hermes_custom.h"

#include "scene.h"
#include "gui.h"

// *************************************************************************************************************************************

SceneBoundaryCustomDialog::SceneBoundaryCustomDialog(SceneBoundary *boundary, QWidget *parent) : SceneBoundaryDialog(parent)
{
    m_boundary = boundary;
    
    createDialog();
    
    // tab order
    setTabOrder(txtName, cmbType);
    
    load();
    setSize();
}

void SceneBoundaryCustomDialog::createContent()
{
    lblEquation->setVisible(false);
    lblEquationImage->setVisible(false);
    
    cmbType = new QComboBox(this);
    Util::scene()->problemInfo()->module()->fillComboBoxBoundaryCondition(cmbType);
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));
    
    // set active marker
    doTypeChanged(cmbType->currentIndex());
    
    layout->addWidget(new QLabel(tr("BC type:")), 4, 0);
    layout->addWidget(cmbType, 4, 2);
    
    int i = 10;
    for(Hermes::vector<Hermes::Module::BoundaryType *>::iterator it_boundary_type = Util::scene()->problemInfo()->module()->boundary_types.begin(); it_boundary_type < Util::scene()->problemInfo()->module()->boundary_types.end(); ++it_boundary_type)
    {
        Hermes::Module::BoundaryType *boundary_type = ((Hermes::Module::BoundaryType *) *it_boundary_type);

        for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin(); it < boundary_type->variables.end(); ++it)
        {
            Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);
            
            // id
            ids.append(QString::fromStdString(variable->id));
            
            // label
            //lblLabel.append(new QLabel((QString::fromStdString(variable->shortname + " (" + variable->unit + "):"))));
            //lblLabel.at(lblLabel.count() - 1)->setToolTip((QString::fromStdString(variable->name)));
            lblLabel.at(lblLabel.count() - 1)->setMinimumWidth(100);
            
            // text edit
            txtVariable.append(new ValueLineEdit(this));
            //txtVariable.at(txtVariable.count() - 1)->setToolTip((QString::fromStdString(variable->name)));
            txtVariable.at(txtVariable.count() - 1)->setValue(Value(QString::number(variable->default_value)));
            
            layout->addWidget(lblLabel.at(txtVariable.count() - 1), i, 0);
            layout->addWidget(txtVariable.at(txtVariable.count() - 1), i, 2);
            
            i++;
        }
    }

    // set active marker
    doTypeChanged(cmbType->currentIndex());
}

void SceneBoundaryCustomDialog::load()
{
    SceneBoundaryDialog::load();
    
    // load type
    cmbType->setCurrentIndex(cmbType->findData(QString::fromStdString(m_boundary->type)));

    // load variables
    for (int i = 0; i < ids.count(); i++)
        txtVariable.at(i)->setValue(m_boundary->get_value(ids.at(i).toStdString()));
}

bool SceneBoundaryCustomDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;
    
    // save type
    m_boundary->type = cmbType->itemData(cmbType->currentIndex()).toString().toStdString();
    
    // save variables
    for (int i = 0; i < ids.count(); i++)
        if (txtVariable.at(i)->evaluate())
            m_boundary->values[ids.at(i).toStdString()] = txtVariable.at(i)->value();
        else
            return false;

    return true;
}

void SceneBoundaryCustomDialog::doTypeChanged(int index)
{
    setMinimumSize(sizeHint());

    // disable variables
    for (int i = 0; i < ids.count(); i++)
        txtVariable.at(i)->setEnabled(false);

    Hermes::Module::BoundaryType *boundary_type = Util::scene()->problemInfo()->module()->get_boundary_type(cmbType->itemData(index).toString().toStdString());
    for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin(); it < boundary_type->variables.end(); ++it)
    {
        Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);

        int i = ids.indexOf(QString::fromStdString(variable->id));

        if (i >= 0)
            txtVariable.at(i)->setEnabled(true);
    }
}

// *************************************************************************************************************************************

SceneMaterialCustomDialog::SceneMaterialCustomDialog(SceneMaterial *material, QWidget *parent) : SceneMaterialDialog(parent)
{
    m_material = material;
    
    createDialog();
    
    load();
    setSize();
}

void SceneMaterialCustomDialog::createContent()
{
    lblEquation->setVisible(false);
    lblEquationImage->setVisible(false);
    
    int i = 10;
    Hermes::vector<Hermes::Module::MaterialTypeVariable *> materials = Util::scene()->problemInfo()->module()->material_type_variables;
    for (Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = materials.begin(); it < materials.end(); ++it)
    {
        Hermes::Module::MaterialTypeVariable *variable = ((Hermes::Module::MaterialTypeVariable *) *it);
        
        // id
        ids.append(QString::fromStdString(variable->id));
        
        // label
        //lblLabel.append(new QLabel((QString::fromStdString(variable->shortname + " (" + variable->unit + "):"))));
        //lblLabel.at(lblLabel.count() - 1)->setToolTip((QString::fromStdString(variable->name)));
        lblLabel.at(lblLabel.count() - 1)->setMinimumWidth(100);
        
        // text edit
        txtVariable.append(new ValueLineEdit(this));
        txtVariable.at(txtVariable.count() - 1)->setValue(Value(QString::number(variable->default_value)));
        
        layout->addWidget(lblLabel.at(txtVariable.count() - 1), i, 0);
        layout->addWidget(txtVariable.at(txtVariable.count() - 1), i, 2);
        
        i++;
    }
}

void SceneMaterialCustomDialog::load()
{
    SceneMaterialDialog::load();
    
    // load variables
    for (int i = 0; i < ids.count(); i++)
        txtVariable.at(i)->setValue(m_material->get_value(ids.at(i).toStdString()));
}

bool SceneMaterialCustomDialog::save() {
    if (!SceneMaterialDialog::save()) return false;;
    
    // save variables
    for (int i = 0; i < ids.count(); i++)
        if (txtVariable.at(i)->evaluate())
            m_material->values[ids.at(i).toStdString()] = txtVariable.at(i)->value();
        else
            return false;
    
    return true;
}
