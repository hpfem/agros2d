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

#include "scenelabel.h"

#include "util.h"
#include "util/global.h"
#include "util/loops.h"
#include "gui/valuelineedit.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "meshgenerator.h"


#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

SceneLabel::SceneLabel(const Point &point, double area)
    : MarkedSceneBasic<SceneMaterial>(), m_point(point), m_area(area)
{
    foreach (FieldInfo* field, Agros2D::problem()->fieldInfos())
    {
        this->addMarker(SceneMaterialContainer::getNone(field));
    }
}

double SceneLabel::distance(const Point &point) const
{
    return (this->point() - point).magnitude();
}

bool SceneLabel::isHole()
{
    foreach (FieldInfo* field, Agros2D::problem()->fieldInfos())
        if(hasMarker(field) && !marker(field)->isNone())
            return false;

    return true;
}

int SceneLabel::showDialog(QWidget *parent, bool isNew)
{
    SceneLabelDialog *dialog = new SceneLabelDialog(this, parent, isNew);
    return dialog->exec();
}

SceneLabelCommandAdd* SceneLabel::getAddCommand()
{
    return new SceneLabelCommandAdd(m_point, markersKeys(), m_area);
}

SceneLabelCommandRemove* SceneLabel::getRemoveCommand()
{
    return new SceneLabelCommandRemove(m_point, markersKeys(), m_area);
}

SceneLabel *SceneLabel::findClosestLabel(const Point &point)
{
    try
    {
        QMapIterator<SceneLabel*, QList<LoopsInfo::Triangle> > i(Agros2D::scene()->loopsInfo()->polygonTriangles());
        while (i.hasNext())
        {
            i.next();

            foreach (LoopsInfo::Triangle triangle, i.value())
            {
                bool b1 = (point.x - triangle.b.x) * (triangle.a.y - triangle.b.y) - (triangle.a.x - triangle.b.x) * (point.y - triangle.b.y) < 0.0;
                bool b2 = (point.x - triangle.c.x) * (triangle.b.y - triangle.c.y) - (triangle.b.x - triangle.c.x) * (point.y - triangle.c.y) < 0.0;
                bool b3 = (point.x - triangle.a.x) * (triangle.c.y - triangle.a.y) - (triangle.c.x - triangle.a.x) * (point.y - triangle.a.y) < 0.0;

                if ((b1 == b2) && (b2 == b3))
                {
                    // in triangle
                    return i.key();
                }
            }
        }
    }
    catch (AgrosMeshException &ame)
    {
        // do nothing
    }

    if (Agros2D::scene()->loopsInfo()->isProcessPolygonError())
    {
        SceneLabel *labelClosest = NULL;

        double distance = numeric_limits<double>::max();
        foreach (SceneLabel *label, Agros2D::scene()->labels->items())
        {
            double labelDistance = label->distance(point);
            if (labelDistance < distance)
            {
                distance = labelDistance;
                labelClosest = label;
            }
        }

        return labelClosest;
    }

    return NULL;
}

//****************************************************************************************************

SceneLabel* SceneLabelContainer::get(SceneLabel *label) const
{
    foreach (SceneLabel *labelCheck, m_data)
    {
        if (labelCheck->point() == label->point())
        {
            return labelCheck;
        }
    }

    return NULL;
}

SceneLabel* SceneLabelContainer::get(const Point& point) const
{
    foreach (SceneLabel *labelCheck, m_data)
    {
        if (labelCheck->point() == point)
            return labelCheck;
    }

    return NULL;
}

RectPoint SceneLabelContainer::boundingBox() const
{
    Point min( numeric_limits<double>::max(),  numeric_limits<double>::max());
    Point max(-numeric_limits<double>::max(), -numeric_limits<double>::max());

    foreach (SceneLabel *label, m_data)
    {
        min.x = qMin(min.x, label->point().x);
        max.x = qMax(max.x, label->point().x);
        min.y = qMin(min.y, label->point().y);
        max.y = qMax(max.y, label->point().y);
    }

    return RectPoint(min, max);
}

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

    txtAreaRefinement = new QSpinBox(this);
    txtAreaRefinement->setMinimum(0);
    txtAreaRefinement->setMaximum(10);

    chkAreaRefinement = new QCheckBox();
    connect(chkAreaRefinement, SIGNAL(stateChanged(int)), this, SLOT(doAreaRefinement(int)));

    QHBoxLayout *layoutAreaRefinement = new QHBoxLayout();
    layoutAreaRefinement->addWidget(chkAreaRefinement);
    layoutAreaRefinement->addWidget(txtAreaRefinement);
    layoutAreaRefinement->addWidget(new QLabel(tr("Global refinement is %1.").arg(fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt())));
    layoutAreaRefinement->addStretch();

    // order
    txtPolynomialOrder = new QSpinBox(this);
    txtPolynomialOrder->setMinimum(1);
    txtPolynomialOrder->setMaximum(10);

    chkPolynomialOrder = new QCheckBox();
    connect(chkPolynomialOrder, SIGNAL(stateChanged(int)), this, SLOT(doPolynomialOrder(int)));

    QHBoxLayout *layoutPolynomialOrder = new QHBoxLayout();
    layoutPolynomialOrder->addWidget(chkPolynomialOrder);
    layoutPolynomialOrder->addWidget(txtPolynomialOrder);
    layoutPolynomialOrder->addWidget(new QLabel(tr("Global order is %1.").arg(fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt())));
    layoutPolynomialOrder->addStretch();

    QFormLayout *layoutBoundaries = new QFormLayout();
    layoutBoundaries->addRow(tr("Material:"), layoutBoundary);
    layoutBoundaries->addRow(tr("Area refinement (-):"), layoutAreaRefinement);
    layoutBoundaries->addRow(tr("Polynomial order (-):"), layoutPolynomialOrder);

    setLayout(layoutBoundaries);
}

void SceneLabelMarker::load()
{
    if (m_label->hasMarker(m_fieldInfo))
        cmbMaterial->setCurrentIndex(cmbMaterial->findData(m_label->marker(m_fieldInfo)->variant()));

    // refine area
    int refinement = m_fieldInfo->labelRefinement(m_label);
    chkAreaRefinement->setChecked(refinement > 0);
    txtAreaRefinement->setEnabled(chkAreaRefinement->isChecked());
    txtAreaRefinement->setValue(refinement);

    // polynomial order
    int order = m_fieldInfo->labelPolynomialOrder(m_label);
    chkPolynomialOrder->setChecked(order != m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt());
    txtPolynomialOrder->setEnabled(chkPolynomialOrder->isChecked());
    txtPolynomialOrder->setValue(order);
}

bool SceneLabelMarker::save()
{
    m_label->addMarker(cmbMaterial->itemData(cmbMaterial->currentIndex()).value<SceneMaterial *>());

    // refine area
    if (chkAreaRefinement->isChecked())
        m_fieldInfo->setLabelRefinement(m_label, txtAreaRefinement->text().toInt());
    else
        m_fieldInfo->removeLabelRefinement(m_label);

    // polynomial order
    if (chkPolynomialOrder->isChecked())
        m_fieldInfo->setLabelPolynomialOrder(m_label, txtPolynomialOrder->text().toInt());
    else
        m_fieldInfo->removeLabelPolynomialOrder(m_label);

    return true;
}

void SceneLabelMarker::doAreaRefinement(int state)
{
    txtAreaRefinement->setEnabled(chkAreaRefinement->isChecked());
}

void SceneLabelMarker::doPolynomialOrder(int state)
{
    txtPolynomialOrder->setEnabled(chkPolynomialOrder->isChecked());
}

void SceneLabelMarker::fillComboBox()
{
    cmbMaterial->clear();

    // none marker
    cmbMaterial->addItem(Agros2D::scene()->materials->getNone(m_fieldInfo)->name(),
                         Agros2D::scene()->materials->getNone(m_fieldInfo)->variant());

    // real markers
    foreach (SceneMaterial *material, Agros2D::scene()->materials->filter(m_fieldInfo).items())
    {
        cmbMaterial->addItem(material->name(),
                             material->variant());
    }
}

void SceneLabelMarker::doMaterialChanged(int index)
{
    btnMaterial->setEnabled(cmbMaterial->currentIndex() > 0);
    chkAreaRefinement->setEnabled(cmbMaterial->currentIndex() > 0);
    chkPolynomialOrder->setEnabled(cmbMaterial->currentIndex() > 0);
}

void SceneLabelMarker::doMaterialClicked()
{
    SceneMaterial *marker = cmbMaterial->itemData(cmbMaterial->currentIndex()).value<SceneMaterial *>();
    if (marker->showDialog(this) == QDialog::Accepted)
    {
        cmbMaterial->setItemText(cmbMaterial->currentIndex(), marker->name());
        Agros2D::scene()->invalidate();
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

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
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
    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Axisymmetric)
        txtPointX->setMinimum(0.0);

    // coordinates
    QFormLayout *layoutCoordinates = new QFormLayout();
    layoutCoordinates->addRow(Agros2D::problem()->config()->labelX() + " (m):", txtPointX);
    layoutCoordinates->addRow(Agros2D::problem()->config()->labelY() + " (m):", txtPointY);

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
    layoutMeshParameters->addRow(tr("Element area (Triangle) (m<sup>2</sup>):"), layoutArea);

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

    txtPointX->setNumber(sceneLabel->point().x);
    txtPointY->setNumber(sceneLabel->point().y);
    txtArea->setNumber(sceneLabel->area());
    chkArea->setChecked(sceneLabel->area() > 0.0);
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
    if (Agros2D::scene()->getLabel(point) && ((sceneLabel->point() != point) || m_isNew))
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
        if (sceneLabel->point() != point)
        {
            Agros2D::scene()->undoStack()->push(new SceneLabelCommandEdit(sceneLabel->point(), point));
        }
    }

    sceneLabel->setPoint(point);
    sceneLabel->setArea(chkArea->isChecked() ? txtArea->number() : 0.0);

    foreach (SceneLabelMarker *labelMarker, m_labelMarkers)
        labelMarker->save();

    Agros2D::scene()->invalidate();
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

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
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
    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        cmbMaterials[fieldInfo]->clear();

        // none marker
        cmbMaterials[fieldInfo]->addItem(Agros2D::scene()->materials->getNone(fieldInfo)->name(),
                                         Agros2D::scene()->materials->getNone(fieldInfo)->variant());

        // real markers
        foreach (SceneMaterial *material, Agros2D::scene()->materials->filter(fieldInfo).items())
            cmbMaterials[fieldInfo]->addItem(material->name(),
                                             material->variant());
    }

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        SceneMaterial* material = NULL;
        bool match = true;
        foreach(SceneLabel* label, m_labels.items())
        {
            if(material)
                match = match && (material == label->marker(fieldInfo));
            else
                material = label->marker(fieldInfo);
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
        foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
        {
            if (cmbMaterials[fieldInfo]->currentIndex() != -1)
                label->addMarker(cmbMaterials[fieldInfo]->itemData(cmbMaterials[fieldInfo]->currentIndex()).value<SceneMaterial *>());

        }
    }

    Agros2D::scene()->invalidate();
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

SceneLabelCommandAdd::SceneLabelCommandAdd(const Point &point, const QMap<QString, QString> &markers, double area, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_point = point;
    m_markers = markers;
    m_area = area;
}

void SceneLabelCommandAdd::undo()
{
    Agros2D::scene()->labels->remove(Agros2D::scene()->getLabel(m_point));
    Agros2D::scene()->invalidate();
}

void SceneLabelCommandAdd::redo()
{
    // new edge
    SceneLabel *label = new SceneLabel(m_point, m_area);

    foreach (QString fieldId, m_markers.keys())
    {
        if (Agros2D::problem()->hasField(fieldId))
        {
            SceneMaterial *material = Agros2D::scene()->materials->filter(Agros2D::problem()->fieldInfo(fieldId)).get(m_markers[fieldId]);

            if (!material)
                material = Agros2D::scene()->materials->getNone(Agros2D::problem()->fieldInfo(fieldId));

            // add marker
            label->addMarker(material);
        }
    }

    // add edge to the list
    Agros2D::scene()->addLabel(label);;
    Agros2D::scene()->invalidate();
}

SceneLabelCommandRemove::SceneLabelCommandRemove(const Point &point, const QMap<QString, QString> &markers, double area, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_point = point;
    m_markers = markers;
    m_area = area;
}

void SceneLabelCommandRemove::undo()
{
    // new edge
    SceneLabel *label = new SceneLabel(m_point, m_area);

    foreach (QString fieldId, m_markers.keys())
    {
        if (Agros2D::problem()->hasField(fieldId))
        {
            SceneMaterial *material = Agros2D::scene()->materials->filter(Agros2D::problem()->fieldInfo(fieldId)).get(m_markers[fieldId]);

            if (!material)
                material = Agros2D::scene()->materials->getNone(Agros2D::problem()->fieldInfo(fieldId));

            // add marker
            label->addMarker(material);
        }
    }

    // add edge to the list
    Agros2D::scene()->addLabel(label);
    Agros2D::scene()->invalidate();
}

void SceneLabelCommandRemove::redo()
{
    Agros2D::scene()->labels->remove(Agros2D::scene()->getLabel(m_point));
    Agros2D::scene()->invalidate();
}

SceneLabelCommandEdit::SceneLabelCommandEdit(const Point &point, const Point &pointNew, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_point = point;
    m_pointNew = pointNew;
}

void SceneLabelCommandEdit::undo()
{
    SceneLabel *label = Agros2D::scene()->getLabel(m_pointNew);
    if (label)
    {
        label->setPoint(m_point);
        Agros2D::scene()->invalidate();
    }
}

void SceneLabelCommandEdit::redo()
{
    SceneLabel *label = Agros2D::scene()->getLabel(m_point);
    if (label)
    {
        label->setPoint(m_pointNew);
        Agros2D::scene()->invalidate();
    }
}

SceneLabelCommandRemoveMulti::SceneLabelCommandRemoveMulti(QList<Point> points, QList<QMap<QString, QString> > markers, QList<double> areas, QUndoCommand *parent) : QUndoCommand(parent)
{
    assert(points.size() == markers.size());
    assert(points.size() == areas.size());
    m_points = points;
    m_areas = areas;
    m_markers = markers;
}

void SceneLabelCommandRemoveMulti::undo()
{
    Agros2D::scene()->stopInvalidating(true);

    for (int i = 0; i < m_points.size(); i++)
    {
        SceneLabel *label = new SceneLabel(m_points[i], m_areas[i]);

        foreach (QString fieldId, m_markers[i].keys())
        {
            if (Agros2D::problem()->hasField(fieldId))
            {
                SceneMaterial *material = Agros2D::scene()->materials->filter(Agros2D::problem()->fieldInfo(fieldId)).get(m_markers[i][fieldId]);

                if (!material)
                    material = Agros2D::scene()->materials->getNone(Agros2D::problem()->fieldInfo(fieldId));

                // add marker
                label->addMarker(material);
            }
        }

        // add label to the list
        Agros2D::scene()->addLabel(label);
    }

    Agros2D::scene()->stopInvalidating(false);
    Agros2D::scene()->invalidate();
}

void SceneLabelCommandRemoveMulti::redo()
{
    Agros2D::scene()->stopInvalidating(true);
    for(int i = 0; i < m_points.size(); i++)
    {
        Agros2D::scene()->labels->remove(Agros2D::scene()->getLabel(m_points[i]));
    }

    Agros2D::scene()->stopInvalidating(false);
    Agros2D::scene()->invalidate();
}

SceneLabelCommandMoveMulti::SceneLabelCommandMoveMulti(QList<Point> points, QList<Point> pointsNew, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_points = points;
    m_pointsNew = pointsNew;
}

void SceneLabelCommandMoveMulti::moveAll(QList<Point> moveFrom, QList<Point> moveTo)
{
    assert(moveFrom.size() == moveTo.size());
    QList<SceneLabel*> labels;
    for(int i = 0; i < moveFrom.size(); i++)
    {
        Point point = moveFrom[i];
        SceneLabel *label = Agros2D::scene()->getLabel(point);
        labels.push_back(label);
    }

    for(int i = 0; i < moveFrom.size(); i++)
    {
        Point pointNew = moveTo[i];
        SceneLabel *label = labels[i];
        if (label)
        {
            label->setPoint(pointNew);
        }
    }
}

void SceneLabelCommandMoveMulti::undo()
{
    moveAll(m_pointsNew, m_points);
    Agros2D::scene()->invalidate();
}

void SceneLabelCommandMoveMulti::redo()
{
    moveAll(m_points, m_pointsNew);
    Agros2D::scene()->invalidate();
}

SceneLabelCommandAddMulti::SceneLabelCommandAddMulti(QList<Point> points, QList<QMap<QString, QString> > markers, QList<double> areas, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_points = points;
    m_areas = areas;
    m_markers = markers;
}

void SceneLabelCommandAddMulti::undo()
{
    Agros2D::scene()->stopInvalidating(true);
    foreach(Point point, m_points)
    {
        SceneLabel *label = Agros2D::scene()->getLabel(point);
        if (label)
        {
            Agros2D::scene()->labels->remove(label);
        }
    }

    Agros2D::scene()->stopInvalidating(false);
    Agros2D::scene()->invalidate();
}

void SceneLabelCommandAddMulti::redo()
{
    Agros2D::scene()->stopInvalidating(true);

    for(int i = 0; i < m_points.size(); i++)
    {
        SceneLabel *label = new SceneLabel(m_points[i], m_areas[i]);

        // if markers are not empty, the operation was performed with "withMarkers = True"
        if(!m_markers.empty())
        {
            foreach (QString fieldId, m_markers[i].keys())
            {
                if (Agros2D::problem()->hasField(fieldId))
                {
                    SceneMaterial *material = Agros2D::scene()->materials->filter(Agros2D::problem()->fieldInfo(fieldId)).get(m_markers[i][fieldId]);

                    if (!material)
                        material = Agros2D::scene()->materials->getNone(Agros2D::problem()->fieldInfo(fieldId));

                    // add marker
                    label->addMarker(material);
                }
            }
        }

        Agros2D::scene()->addLabel(label);
    }

    Agros2D::scene()->stopInvalidating(false);
    Agros2D::scene()->invalidate();
}
