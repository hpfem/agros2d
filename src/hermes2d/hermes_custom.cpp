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
}

void SceneBoundaryCustomDialog::load()
{
    SceneBoundaryDialog::load();

    cmbType->setCurrentIndex(cmbType->findData(QString::fromStdString(m_boundary->type)));
}

bool SceneBoundaryCustomDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;

    m_boundary->type = cmbType->itemData(cmbType->currentIndex()).toString().toStdString();

    return true;
}

void SceneBoundaryCustomDialog::doTypeChanged(int index)
{
    setMinimumSize(sizeHint());
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
}

void SceneMaterialCustomDialog::load()
{
    SceneMaterialDialog::load();
}

bool SceneMaterialCustomDialog::save() {
    if (!SceneMaterialDialog::save()) return false;;

    return true;
}
