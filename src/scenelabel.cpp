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
#include "hermes2d/field.h"
#include "hermes2d/problem.h"

SceneLabel::SceneLabel(const Point &point, double area)
    : MarkedSceneBasic()
{
    this->point = point;
    this->area = area;

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
    // TODO: undo
    return new SceneLabelCommandRemove(point, "TODO", area);
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

SceneLabelMarker::SceneLabelMarker(SceneLabel *label, FieldInfo *fieldInfo, QWidget *parent)
    : QGroupBox(parent), m_fieldInfo(fieldInfo), m_label(label)

{
    setTitle(fieldInfo->name());

    cmbMaterial = new QComboBox();
    connect(cmbMaterial, SIGNAL(currentIndexChanged(int)), this, SLOT(doMaterialChanged(int)));

    btnMaterial = new QPushButton(icon("three-dots"), "");
    btnMaterial->setMaximumSize(btnMaterial->sizeHint());
    connect(btnMaterial, SIGNAL(clicked()), this, SLOT(doMaterialClicked()));

    QHBoxLayout *layoutBoundary = new QHBoxLayout();
    layoutBoundary->addWidget(cmbMaterial, 1);
    layoutBoundary->addWidget(btnMaterial);

    txtPolynomialOrder = new QSpinBox(this);
    txtPolynomialOrder->setMinimum(0);
    txtPolynomialOrder->setMaximum(10);

    // order
    chkPolynomialOrder = new QCheckBox();
    connect(chkPolynomialOrder, SIGNAL(stateChanged(int)), this, SLOT(doPolynomialOrder(int)));

    QHBoxLayout *layoutPolynomialOrder = new QHBoxLayout();
    layoutPolynomialOrder->addWidget(chkPolynomialOrder);
    layoutPolynomialOrder->addWidget(txtPolynomialOrder);

    layoutPolynomialOrder->addWidget(new QLabel(tr("Global order is %1.").arg(fieldInfo->polynomialOrder())));

    QFormLayout *layoutBoundaries = new QFormLayout();
    layoutBoundaries->addRow(tr("Material:"), layoutBoundary);
    layoutBoundaries->addRow(tr("Polynomial order (-):"), layoutPolynomialOrder);

    setLayout(layoutBoundaries);
}

void SceneLabelMarker::load()
{
    cmbMaterial->setCurrentIndex(cmbMaterial->findData(m_label->getMarker(m_fieldInfo)->variant()));

    // txtPolynomialOrder->setValue(m_label->polynomialOrder);
    // chkPolynomialOrder->setChecked(m_label->polynomialOrder > 0);
    txtPolynomialOrder->setEnabled(chkPolynomialOrder->isChecked());
}

bool SceneLabelMarker::save()
{
    m_label->addMarker(cmbMaterial->itemData(cmbMaterial->currentIndex()).value<SceneMaterial *>());

    // TODO: sceneLabel->polynomialOrder = chkPolynomialOrder->isChecked() ? txtPolynomialOrder->value() : 0;

    return true;
}

void SceneLabelMarker::doPolynomialOrder(int state)
{
    chkPolynomialOrder->setEnabled(cmbMaterial->currentIndex() > 0);
    txtPolynomialOrder->setEnabled(chkPolynomialOrder->isChecked() && cmbMaterial->currentIndex() > 0);
}

void SceneLabelMarker::fillComboBox()
{
    cmbMaterial->clear();

    // none marker
    cmbMaterial->addItem(Util::scene()->materials->getNone(m_fieldInfo)->getName(),
                         Util::scene()->materials->getNone(m_fieldInfo)->variant());

    // real markers
    foreach (SceneMaterial *material, Util::scene()->materials->filter(m_fieldInfo).items())
    {
        cmbMaterial->addItem(material->getName(),
                             material->variant());
    }
}

void SceneLabelMarker::doMaterialChanged(int index)
{
    btnMaterial->setEnabled(cmbMaterial->currentIndex() > 0);
    doPolynomialOrder(0);
}

void SceneLabelMarker::doMaterialClicked()
{
    SceneMaterial *marker = cmbMaterial->itemData(cmbMaterial->currentIndex()).value<SceneMaterial *>();
    if (marker->showDialog(this) == QDialog::Accepted)
    {
        cmbMaterial->setItemText(cmbMaterial->currentIndex(), marker->getName());
        Util::scene()->refresh();
    }
}

SceneLabelDialog::SceneLabelDialog(SceneLabel *label, QWidget *parent, bool isNew) : SceneBasicDialog(parent, isNew)
{
    m_object = label;

    setWindowIcon(icon("scene-label"));
    setWindowTitle(tr("Label"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    // setMaximumSize(sizeHint());
}

QLayout* SceneLabelDialog::createContent()
{
    // markers
    QFormLayout *layout = new QFormLayout();

    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        SceneLabelMarker *sceneLabel = new SceneLabelMarker(dynamic_cast<SceneLabel *>(m_object), fieldInfo, this);
        layout->addRow(sceneLabel);

        m_labelMarkers.append(sceneLabel);
    }

    txtPointX = new ValueLineEdit();
    txtPointY = new ValueLineEdit();
    connect(txtPointX, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtPointY, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    txtArea = new ValueLineEdit();
    txtArea->setMinimum(0.0);
    connect(txtArea, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    // coordinates must be greater then or equal to 0 (axisymmetric case)
    if (Util::problem()->config()->coordinateType() == CoordinateType_Axisymmetric)
        txtPointX->setMinimum(0.0);

    // coordinates
    QFormLayout *layoutCoordinates = new QFormLayout();
    layoutCoordinates->addRow(Util::problem()->config()->labelX() + " (m):", txtPointX);
    layoutCoordinates->addRow(Util::problem()->config()->labelY() + " (m):", txtPointY);

    QGroupBox *grpCoordinates = new QGroupBox(tr("Coordinates"));
    grpCoordinates->setLayout(layoutCoordinates);

    // area
    chkArea = new QCheckBox();
    connect(chkArea, SIGNAL(stateChanged(int)), this, SLOT(doArea(int)));

    QHBoxLayout *layoutArea = new QHBoxLayout();
    layoutArea->addWidget(chkArea);
    layoutArea->addWidget(txtArea);

    // mesh
    QFormLayout *layoutMeshParameters = new QFormLayout();
    layoutMeshParameters->addRow(tr("Triangle area (m):"), layoutArea);

    QGroupBox *grpMeshParameters = new QGroupBox(tr("Mesh parameters"));
    grpMeshParameters->setLayout(layoutMeshParameters);

    layout->addRow(grpCoordinates);
    layout->addRow(grpMeshParameters);

    fillComboBox();

    return layout;
}

void SceneLabelDialog::fillComboBox()
{
    // markers
    foreach (SceneLabelMarker *labelMarker, m_labelMarkers)
        labelMarker->fillComboBox();
}

bool SceneLabelDialog::load()
{
    SceneLabel *sceneLabel = dynamic_cast<SceneLabel *>(m_object);

    txtPointX->setNumber(sceneLabel->point.x);
    txtPointY->setNumber(sceneLabel->point.y);
    txtArea->setNumber(sceneLabel->area);
    chkArea->setChecked(sceneLabel->area > 0.0);
    txtArea->setEnabled(chkArea->isChecked());

    foreach (SceneLabelMarker *labelMarker, m_labelMarkers)
        labelMarker->load();

    return true;
}

bool SceneLabelDialog::save()
{
    if (!txtPointX->evaluate(false)) return false;
    if (!txtPointY->evaluate(false)) return false;
    if (!txtArea->evaluate(false)) return false;

    SceneLabel *sceneLabel = dynamic_cast<SceneLabel *>(m_object);

    Point point(txtPointX->number(), txtPointY->number());

    // check if label doesn't exists
    if (Util::scene()->getLabel(point) && ((sceneLabel->point != point) || m_isNew))
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

    if (!m_isNew)
    {
        if (sceneLabel->point != point)
        {
            Util::scene()->undoStack()->push(new SceneLabelCommandEdit(sceneLabel->point, point));
        }
    }

    sceneLabel->point = point;
    sceneLabel->area = chkArea->isChecked() ? txtArea->number() : 0.0;

    foreach (SceneLabelMarker *labelMarker, m_labelMarkers)
        labelMarker->save();

    return true;
}

void SceneLabelDialog::doArea(int state)
{
    txtArea->setEnabled(chkArea->isChecked());
}

SceneLabelSelectDialog::SceneLabelSelectDialog(MarkedSceneBasicContainer<SceneMaterial, SceneLabel> labels, QWidget *parent)
    : QDialog(parent), m_labels(labels)
{
    setWindowIcon(icon("scene-label"));
    setWindowTitle(tr("Labels"));

    // markers
    QFormLayout *layoutMaterials = new QFormLayout();

    QGroupBox *grpMaterials = new QGroupBox(tr("Materials"));
    grpMaterials->setLayout(layoutMaterials);

    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        QComboBox *cmbMaterial = new QComboBox();
        cmbMaterials[fieldInfo] = cmbMaterial;

        layoutMaterials->addRow(fieldInfo->name(), cmbMaterial);
    }

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpMaterials);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    load();

    setMinimumSize(sizeHint());
}

void SceneLabelSelectDialog::load()
{
    // markers
    // markers
    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        cmbMaterials[fieldInfo]->clear();

        // none marker
        cmbMaterials[fieldInfo]->addItem(Util::scene()->materials->getNone(fieldInfo)->getName(),
                                         Util::scene()->materials->getNone(fieldInfo)->variant());

        // real markers
        foreach (SceneMaterial *material, Util::scene()->materials->filter(fieldInfo).items())
            cmbMaterials[fieldInfo]->addItem(material->getName(),
                                             material->variant());
    }

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

bool SceneLabelSelectDialog::save()
{
    foreach (SceneLabel* label, m_labels.items())
    {
        foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
        {
            if (cmbMaterials[fieldInfo]->currentIndex() != -1)
                label->addMarker(cmbMaterials[fieldInfo]->itemData(cmbMaterials[fieldInfo]->currentIndex()).value<SceneMaterial *>());

        }
    }

    return true;
}

void SceneLabelSelectDialog::doAccept()
{
    if (save())
        accept();
}

void SceneLabelSelectDialog::doReject()
{
    reject();
}

// undo framework *******************************************************************************************************************

SceneLabelCommandAdd::SceneLabelCommandAdd(const Point &point, const QString &markerName, double area, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_point = point;
    m_markerName = markerName;
    m_area = area;
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

SceneLabelCommandRemove::SceneLabelCommandRemove(const Point &point, const QString &markerName, double area, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_point = point;
    m_markerName = markerName;
    m_area = area;
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
