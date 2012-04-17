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

#include "util.h"
#include "scene.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"

SceneLabel::SceneLabel(const Point &point, double area, int polynomialOrder)
    : MarkedSceneBasic()
{
    this->point = point;
    this->area = area;
    this->polynomialOrder = polynomialOrder;

    foreach (FieldInfo* field, Util::problem()->fieldInfos())
    {
        this->addMarker(SceneMaterialContainer::getNone(field));
    }
}

double SceneLabel::distance(const Point &point) const
{
    return (this->point - point).magnitude();
}

int SceneLabel::showDialog(QWidget *parent, bool isNew)
{
    SceneLabelDialog *dialog = new SceneLabelDialog(this, parent, isNew);
    return dialog->exec();
}

SceneLabelCommandRemove* SceneLabel::getRemoveCommand()
{
    return new SceneLabelCommandRemove(point, "TODO", area, polynomialOrder);
}


//****************************************************************************************************

SceneLabel* SceneLabelContainer::get(SceneLabel *label) const
{
    foreach (SceneLabel *labelCheck, data)
    {
        if (labelCheck->point == label->point)
        {
            return labelCheck;
        }
    }

    return NULL;
}

SceneLabel* SceneLabelContainer::get(const Point& point) const
{
    foreach (SceneLabel *labelCheck, data)
    {
        if (labelCheck->point == point)
            return labelCheck;
    }

    return NULL;
}
/*
SceneLabel* SceneLabelContainer::atNotNoneHack(int i, FieldInfo* fieldInfo)
{
    int count = 0;
    foreach(SceneLabel* label, items())
    {
        if (label->getMarker(fieldInfo)->isNone())
            continue;
        if (count == i)
            return label;
        count++;
    }
}
*/

// *************************************************************************************************************************************

SceneLabelDialog::SceneLabelDialog(SceneLabel *label, QWidget *parent, bool isNew) : SceneBasicDialog(parent, isNew)
{
    m_object = label;
    m_singleLabel = true;

    setWindowIcon(icon("scene-label"));
    setWindowTitle(tr("Label"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    // setMaximumSize(sizeHint());
}

SceneLabelDialog::SceneLabelDialog(MarkedSceneBasicContainer<SceneMaterial, SceneLabel> labels, QWidget *parent) : SceneBasicDialog(parent, false)
{
    m_labels = labels;
    m_object = NULL;
    m_singleLabel = false;

    setWindowIcon(icon("scene-label"));
    setWindowTitle(tr("Labels"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    // setMaximumSize(sizeHint());

}

QLayout* SceneLabelDialog::createContent()
{
    // markers
    QFormLayout *layoutMaterials = new QFormLayout();

    QGroupBox *grpMaterials = new QGroupBox(tr("Materials"));
    grpMaterials->setLayout(layoutMaterials);

    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        QComboBox *cmbMaterial = new QComboBox();
        connect(cmbMaterial, SIGNAL(currentIndexChanged(int)), this, SLOT(doMaterialChanged(int)));
        cmbMaterials[fieldInfo] = cmbMaterial;

        QPushButton *btnMaterial = new QPushButton(icon("three-dots"), "");
        btnMaterial->setMaximumSize(btnMaterial->sizeHint());
        connect(btnMaterial, SIGNAL(clicked()), this, SLOT(doMaterialClicked()));
        btnMaterials[fieldInfo] = btnMaterial;

        QHBoxLayout *layoutMaterial = new QHBoxLayout();
        layoutMaterial->addWidget(cmbMaterial);
        layoutMaterial->addWidget(btnMaterial);

        layoutMaterials->addRow(QString::fromStdString(fieldInfo->module()->name),
                                layoutMaterial);
    }

    QFormLayout *layout = new QFormLayout();
    layout->addRow(grpMaterials);

    if (m_singleLabel)
    {
        txtPointX = new ValueLineEdit();
        txtPointY = new ValueLineEdit();
        connect(txtPointX, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
        connect(txtPointY, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

        txtArea = new ValueLineEdit();
        txtArea->setMinimum(0.0);
        connect(txtArea, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
        txtPolynomialOrder = new QSpinBox(this);
        txtPolynomialOrder->setMinimum(0);
        txtPolynomialOrder->setMaximum(10);

        // coordinates must be greater then or equal to 0 (axisymmetric case)
        if (Util::problem()->config()->coordinateType() == CoordinateType_Axisymmetric)
            txtPointX->setMinimum(0.0);

        // coordinates
        QFormLayout *layoutCoordinates = new QFormLayout();
        layoutCoordinates->addRow(Util::problem()->config()->labelX() + " (m):", txtPointX);
        layoutCoordinates->addRow(Util::problem()->config()->labelY() + " (m):", txtPointY);

        QGroupBox *grpCoordinates = new QGroupBox(tr("Coordinates"));
        grpCoordinates->setLayout(layoutCoordinates);

        // order
        chkPolynomialOrder = new QCheckBox();
        connect(chkPolynomialOrder, SIGNAL(stateChanged(int)), this, SLOT(doPolynomialOrder(int)));

        QHBoxLayout *layoutPolynomialOrder = new QHBoxLayout();
        layoutPolynomialOrder->addWidget(chkPolynomialOrder);
        layoutPolynomialOrder->addWidget(txtPolynomialOrder);

        //TODO
        //layoutPolynomialOrder->addWidget(new QLabel(tr("Global order is %1.").arg(Util::problem()->config()->polynomialOrder)));
        layoutPolynomialOrder->addWidget(new QLabel(tr("Global order TODO is %1.").arg(1)));

        // area
        chkArea = new QCheckBox();
        connect(chkArea, SIGNAL(stateChanged(int)), this, SLOT(doArea(int)));

        QHBoxLayout *layoutArea = new QHBoxLayout();
        layoutArea->addWidget(chkArea);
        layoutArea->addWidget(txtArea);

        // mesh
        QFormLayout *layoutMeshParameters = new QFormLayout();
        layoutMeshParameters->addRow(tr("Triangle area (m):"), layoutArea);
        layoutMeshParameters->addRow(tr("Polynomial order (-):"), layoutPolynomialOrder);

        QGroupBox *grpMeshParameters = new QGroupBox(tr("Mesh parameters"));
        grpMeshParameters->setLayout(layoutMeshParameters);

        layout->addRow(grpCoordinates);
        layout->addRow(grpMeshParameters);
    }

    fillComboBox();

    return layout;
}

void SceneLabelDialog::fillComboBox()
{
    // markers
    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        cmbMaterials[fieldInfo]->clear();

        // none marker
        cmbMaterials[fieldInfo]->addItem(QString::fromStdString(Util::scene()->materials->getNone(fieldInfo)->getName()),
                                  Util::scene()->materials->getNone(fieldInfo)->variant());

        // real markers
        foreach (SceneMaterial *material, Util::scene()->materials->filter(fieldInfo).items())
        {
            cmbMaterials[fieldInfo]->addItem(QString::fromStdString(material->getName()),
                                     material->variant());
        }
    }
}

bool SceneLabelDialog::load()
{
    if(m_singleLabel)
    {
        SceneLabel *sceneLabel = dynamic_cast<SceneLabel *>(m_object);

        txtPointX->setNumber(sceneLabel->point.x);
        txtPointY->setNumber(sceneLabel->point.y);
        txtArea->setNumber(sceneLabel->area);
        chkArea->setChecked(sceneLabel->area > 0.0);
        txtArea->setEnabled(chkArea->isChecked());
        txtPolynomialOrder->setValue(sceneLabel->polynomialOrder);
        chkPolynomialOrder->setChecked(sceneLabel->polynomialOrder > 0);
        txtPolynomialOrder->setEnabled(chkPolynomialOrder->isChecked());

        foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
        {
            cmbMaterials[fieldInfo]->setCurrentIndex(cmbMaterials[fieldInfo]->findData(sceneLabel->getMarker(fieldInfo)->variant()));
        }
    }
    else
    {
        foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
        {
            SceneMaterial* material = NULL;
            bool match = true;
            foreach(SceneLabel* label, m_labels.items())
            {
                if(material)
                    match = match && (material == label->getMarker(fieldInfo));
                else
                    material = label->getMarker(fieldInfo);
            }
            if(match)
                cmbMaterials[fieldInfo]->setCurrentIndex(cmbMaterials[fieldInfo]->findData(material->variant()));
            else
                cmbMaterials[fieldInfo]->setCurrentIndex(-1);
        }

    }
    return true;
}

bool SceneLabelDialog::save()
{
    if (m_singleLabel)
    {
        if (!txtPointX->evaluate(false)) return false;
        if (!txtPointY->evaluate(false)) return false;
        if (!txtArea->evaluate(false)) return false;

        SceneLabel *sceneLabel = dynamic_cast<SceneLabel *>(m_object);

        Point point(txtPointX->number(), txtPointY->number());

        // check if label doesn't exists
        if (Util::scene()->getLabel(point) && ((sceneLabel->point != point) || isNew))
        {
            QMessageBox::warning(this, "Label", "Label already exists.");
            return false;
        }

        // area
        if (txtArea->value().number() < 0)
        {
            QMessageBox::warning(this, "Label", "Area must be positive or zero.");
            txtArea->setFocus();
            return false;
        }


        if (!isNew)
        {
            if (sceneLabel->point != point)
            {
                Util::scene()->undoStack()->push(new SceneLabelCommandEdit(sceneLabel->point, point));
            }
        }

        sceneLabel->point = point;
        sceneLabel->area = chkArea->isChecked() ? txtArea->number() : 0.0;
        sceneLabel->polynomialOrder = chkPolynomialOrder->isChecked() ? txtPolynomialOrder->value() : 0;

        foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
        {
            sceneLabel->addMarker(cmbMaterials[fieldInfo]->itemData(cmbMaterials[fieldInfo]->currentIndex()).value<SceneMaterial *>());
        }
    }
    else
    {
        foreach (SceneLabel* label, m_labels.items())
        {
            foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
            {
                if (cmbMaterials[fieldInfo]->currentIndex() != -1)
                    label->addMarker(cmbMaterials[fieldInfo]->itemData(cmbMaterials[fieldInfo]->currentIndex()).value<SceneMaterial *>());

            }
        }

    }

    return true;
}

void SceneLabelDialog::doMaterialChanged(int index)
{
    // for (int i = 0; i < cmbMaterials.length(); i++)
    //     btnMaterials[i]->setEnabled(cmbMaterials[i]->currentIndex() > 0);
}

void SceneLabelDialog::doMaterialClicked()
{
    //TODO
    assert(0);
    //    SceneMaterial *marker = cmbMaterial->itemData(cmbMaterial->currentIndex()).value<SceneMaterial *>();
    //    if (marker->showDialog(this) == QDialog::Accepted)
    //    {
    //        cmbMaterial->setItemText(cmbMaterial->currentIndex(), QString::fromStdString(marker->getName()));
    //        Util::scene()->refresh();
    //    }
}

void SceneLabelDialog::doPolynomialOrder(int state)
{
    txtPolynomialOrder->setEnabled(chkPolynomialOrder->isChecked());
}

void SceneLabelDialog::doArea(int state)
{
    txtArea->setEnabled(chkArea->isChecked());
}


// undo framework *******************************************************************************************************************

SceneLabelCommandAdd::SceneLabelCommandAdd(const Point &point, const QString &markerName, double area, int polynomialOrder, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_point = point;
    m_markerName = markerName;
    m_area = area;
    m_polynomialOrder = polynomialOrder;
}

void SceneLabelCommandAdd::undo()
{
    Util::scene()->labels->remove(Util::scene()->getLabel(m_point));
}

void SceneLabelCommandAdd::redo()
{
    //assert(0); //TODO
    //    SceneMaterial *material = Util::scene()->getMaterial(m_markerName);
    //    if (material == NULL) material = Util::scene()->materials->get("none"); //TODO - do it better
    //    Util::scene()->addLabel(new SceneLabel(m_point, material, m_area, m_polynomialOrder));
}

SceneLabelCommandRemove::SceneLabelCommandRemove(const Point &point, const QString &markerName, double area, int polynomialOrder, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_point = point;
    m_markerName = markerName;
    m_area = area;
    m_polynomialOrder = polynomialOrder;
}

void SceneLabelCommandRemove::undo()
{
   // assert(0);//TODO
    //    SceneMaterial *material = Util::scene()->getMaterial(m_markerName);
    //    if (material == NULL) material = Util::scene()->materials->get("none"); //TODO - do it better
    //    Util::scene()->addLabel(new SceneLabel(m_point, material, m_area, m_polynomialOrder));
}

void SceneLabelCommandRemove::redo()
{
    Util::scene()->labels->remove(Util::scene()->getLabel(m_point));
}

SceneLabelCommandEdit::SceneLabelCommandEdit(const Point &point, const Point &pointNew, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_point = point;
    m_pointNew = pointNew;
}

void SceneLabelCommandEdit::undo()
{
    SceneLabel *label = Util::scene()->getLabel(m_pointNew);
    if (label)
    {
        label->point = m_point;
        Util::scene()->refresh();
    }
}

void SceneLabelCommandEdit::redo()
{
    SceneLabel *label = Util::scene()->getLabel(m_point);
    if (label)
    {
        label->point = m_pointNew;
        Util::scene()->refresh();
    }
}
