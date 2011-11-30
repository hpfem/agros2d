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

#include "dialog.h"

#include "scene.h"
#include "scenesolution.h"
#include "gui.h"

#include "module_agros.h"

SceneTabWidget::SceneTabWidget(Hermes::Module::DialogUI ui, QWidget *parent)
    : QWidget(parent), ui(ui)
{
}

void SceneTabWidget::createContent()
{
    // widget layout
    layout = new QVBoxLayout();
    setLayout(layout);

    // add custom widget
    addCustomWidget(layout);

    // equation
    equationImage = new QLabel();
    equationImage->setMinimumHeight(fontMetrics().height()*2.2);
    equationImage->setMinimumWidth(250);

    QGridLayout *layoutEquation = new QGridLayout();
    layoutEquation->addWidget(new QLabel(tr("Equation:")), 0, 0);
    layoutEquation->addWidget(equationImage, 0, 1, 1, 1, Qt::AlignRight);
    layout->addLayout(layoutEquation);

    for (std::map<std::string, Hermes::vector<Hermes::Module::DialogUI::Row> >::iterator it = ui.groups.begin(); it != ui.groups.end(); ++it)
    {
        // group layout
        QGridLayout *layoutGroup = new QGridLayout();

        // variables
        Hermes::vector<Hermes::Module::DialogUI::Row> variables = it->second;

        for (Hermes::vector<Hermes::Module::DialogUI::Row>::iterator
             itv = variables.begin(); itv < variables.end(); ++itv)
        {
            Hermes::Module::DialogUI::Row material = *itv;

            // id
            ids.append(QString::fromStdString(material.id));

            // label
            labels.append(new QLabel(QString("%1 (%2):").
                                     arg(QString::fromStdString(material.shortname_html)).
                                     arg(QString::fromStdString(material.unit_html))));
            labels.at(labels.count() - 1)->setToolTip((QString::fromStdString(material.name)));
            labels.at(labels.count() - 1)->setMinimumWidth(100);

            // text edit
            values.append(new ValueLineEdit(this, material.timedep, material.nonlin));
            values.at(values.count() - 1)->setValue(Value(QString::number(material.default_value)));

            int index = layoutGroup->rowCount();
            layoutGroup->addWidget(labels.at(labels.count() - 1), index, 0);
            layoutGroup->addWidget(values.at(values.count() - 1), index, 1);
        }

        // widget layout
        if (it->first == "")
        {
            layout->addLayout(layoutGroup);
        }
        else
        {
            QGroupBox *grpGroup = new QGroupBox(QString::fromStdString(it->first));
            grpGroup->setLayout(layoutGroup);
            layout->addWidget(grpGroup);
        }
    }

    refresh();
}

// *************************************************************************************************************************************

SceneTabWidgetMaterial::SceneTabWidgetMaterial(Hermes::Module::DialogUI ui, SceneMaterial *material, QWidget *parent)
    : SceneTabWidget(ui, parent), material(material)
{
}

void SceneTabWidgetMaterial::load()
{
    for (int j = 0; j < ids.count(); j++)
        values.at(j)->setValue(material->getValue(ids.at(j).toStdString()));
}

bool SceneTabWidgetMaterial::save()
{
    for (int j = 0; j < ids.count(); j++)
        if (values.at(j)->evaluate())
            material->addValue(ids.at(j).toStdString(), values.at(j)->value());
        else
            return false;

    return true;
}

void SceneTabWidgetMaterial::refresh()
{
    // read equation
    QString fileName = QString(":/images/equations/%1/%1_%2.png")
            .arg(QString::fromStdString(Util::scene()->fieldInfo("TODO")->module()->id))
            .arg(analysisTypeToStringKey(Util::scene()->fieldInfo("TODO")->analysisType));

    readPixmap(equationImage, fileName);
}

// *************************************************************************************************************************************

SceneTabWidgetBoundary::SceneTabWidgetBoundary(Hermes::Module::DialogUI ui, SceneBoundary *boundary, QWidget *parent)
    : SceneTabWidget(ui, parent), boundary(boundary)
{
}

void SceneTabWidgetBoundary::addCustomWidget(QVBoxLayout *layout)
{
    comboBox = new QComboBox(this);
    Util::scene()->fieldInfo("TODO")->module()->fillComboBoxBoundaryCondition(comboBox);
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged(int)));

    QFormLayout *layoutForm = new QFormLayout();
    layoutForm->addRow(tr("Type:"), comboBox);

    layout->addLayout(layoutForm);
}

void SceneTabWidgetBoundary::refresh()
{
    // set active marker
    doTypeChanged(comboBox->currentIndex());
}

void SceneTabWidgetBoundary::doTypeChanged(int index)
{
    setMinimumSize(sizeHint());

    // disable variables
    for (int i = 0; i < ids.count(); i++)
        values.at(i)->setEnabled(false);

    Hermes::Module::BoundaryType *boundary_type = Util::scene()->fieldInfo("TODO")->module()->get_boundary_type(comboBox->itemData(index).toString().toStdString());
    for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin(); it < boundary_type->variables.end(); ++it)
    {
        Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);

        int i = ids.indexOf(QString::fromStdString(variable->id));

        if (i >= 0)
            values.at(i)->setEnabled(true);
    }

    // read equation
    QString fileName = QString(":/images/equations/%1/%2.png")
            .arg(QString::fromStdString(Util::scene()->fieldInfo("TODO")->module()->id))
            .arg(comboBox->itemData(index).toString());

    readPixmap(equationImage, fileName);

    // sizehint
    setMinimumSize(sizeHint());
}

void SceneTabWidgetBoundary::load()
{
    // load type
    comboBox->setCurrentIndex(comboBox->findData(QString::fromStdString(boundary->getType())));

    // load variables
    for (int i = 0; i < ids.count(); i++)
        values.at(i)->setValue(boundary->getValue(ids.at(i).toStdString()));
}

bool SceneTabWidgetBoundary::save()
{
    // save type
    boundary->setType(comboBox->itemData(comboBox->currentIndex()).toString().toStdString());

    // save variables
    for (int i = 0; i < ids.count(); i++)
        if (values.at(i)->evaluate())
            boundary->setValue(ids.at(i).toStdString(), values.at(i)->value());
        else
            return false;

    return true;
}

// *************************************************************************************************************************************

SceneBoundaryCustomDialog::SceneBoundaryCustomDialog(SceneBoundary *boundary, QWidget *parent)
    : SceneBoundaryDialog(parent)
{
    m_boundary = boundary;

    createDialog();

    load();
    setSize();
}

void SceneBoundaryCustomDialog::createContent()
{
    lblEquation->setVisible(false);
    lblEquationImage->setVisible(false);

    tabModules = new QTabWidget(this);
    SceneTabWidgetBoundary *wid = new SceneTabWidgetBoundary(Util::scene()->fieldInfo("TODO")->module()->boundary_ui, m_boundary, this);
    wid->createContent();
    wid->setMinimumSize(sizeHint());

    tabModules->addTab(wid, icon(""), QString::fromStdString(Util::scene()->fieldInfo("TODO")->module()->name));

    layout->addWidget(tabModules, 10, 0, 1, 3);
}

void SceneBoundaryCustomDialog::load()
{
    SceneBoundaryDialog::load();

    // load variables
    for (int i = 0; i < tabModules->count(); i++)
    {
        SceneTabWidget *wid = dynamic_cast<SceneTabWidget *>(tabModules->widget(i));
        wid->load();
    }
}

bool SceneBoundaryCustomDialog::save() {
    if (!SceneBoundaryDialog::save()) return false;

    // save variables
    for (int i = 0; i < tabModules->count(); i++)
    {
        SceneTabWidget *wid = dynamic_cast<SceneTabWidget *>(tabModules->widget(i));
        if (!wid->save())
            return false;
    }
}

// *************************************************************************************************************************************

SceneMaterialCustomDialog::SceneMaterialCustomDialog(SceneMaterial *material, QWidget *parent)
    : SceneMaterialDialog(parent)
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

    tabModules = new QTabWidget(this);
    SceneTabWidgetMaterial *wid = new SceneTabWidgetMaterial(Util::scene()->fieldInfo("TODO")->module()->material_ui, m_material, this);
    wid->createContent();
    wid->setMinimumSize(sizeHint());

    tabModules->addTab(wid, icon(""), QString::fromStdString(Util::scene()->fieldInfo("TODO")->module()->name));

    layout->addWidget(tabModules, 10, 0, 1, 3);
}

void SceneMaterialCustomDialog::load()
{
    SceneMaterialDialog::load();
    
    // load variables
    for (int i = 0; i < tabModules->count(); i++)
    {
        SceneTabWidget *wid = dynamic_cast<SceneTabWidget *>(tabModules->widget(i));
        wid->load();
    }
}

bool SceneMaterialCustomDialog::save()
{
    if (!SceneMaterialDialog::save()) return false;;
    
    // save variables
    for (int i = 0; i < tabModules->count(); i++)
    {
        SceneTabWidget *wid = dynamic_cast<SceneTabWidget *>(tabModules->widget(i));
        if (!wid->save())
            return false;
    }
    
    return true;
}

