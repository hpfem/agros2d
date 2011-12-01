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

#include "scenebasic.h"

#include "gui.h"

#include "scene.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

SceneBasic::SceneBasic()
{
    logMessage("SceneBasic::SceneBasic()");

    isSelected = false;
    isHighlighted = false;
}

QVariant SceneBasic::variant()
{
    logMessage("SceneBasic::variant()");

    QVariant v;
    v.setValue(this);
    return v;
}

// *************************************************************************************************************************************

SceneNode::SceneNode(const Point &point) : SceneBasic()
{
    logMessage("SceneNode::SceneNode()");

    this->point = point;
}
double SceneNode::distance(const Point &point) const
{
    logMessage("SceneNode::distance()");

    return (this->point - point).magnitude();
}

int SceneNode::showDialog(QWidget *parent, bool isNew)
{
    logMessage("SceneNode::showDialog()");

    DSceneNode *dialog = new DSceneNode(this, parent, isNew);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneEdge::SceneEdge(SceneNode *nodeStart, SceneNode *nodeEnd, SceneBoundary *marker, double angle, int refineTowardsEdge)
    : MarkedSceneBasic()
{
    logMessage("SceneEdge::SceneEdge()");

    this->nodeStart = nodeStart;
    this->nodeEnd = nodeEnd;
    this->marker = marker;
    this->angle = angle;
    this->refineTowardsEdge = refineTowardsEdge;
}

Point SceneEdge::center() const
{
    return centerPoint(nodeStart->point, nodeEnd->point, angle);
}

double SceneEdge::radius() const
{
    logMessage("SceneEdge::radius()");

    return (center() - nodeStart->point).magnitude();
}

double SceneEdge::distance(const Point &point) const
{
    logMessage("SceneEdge::distance()");

    if (isStraight())
    {
        double t = ((point.x-nodeStart->point.x)*(nodeEnd->point.x-nodeStart->point.x) + (point.y-nodeStart->point.y)*(nodeEnd->point.y-nodeStart->point.y)) /
                   ((nodeEnd->point.x-nodeStart->point.x)*(nodeEnd->point.x-nodeStart->point.x) + (nodeEnd->point.y-nodeStart->point.y)*(nodeEnd->point.y-nodeStart->point.y));

        if (t > 1.0) t = 1.0;
        if (t < 0.0) t = 0.0;

        double x = nodeStart->point.x + t*(nodeEnd->point.x-nodeStart->point.x);
        double y = nodeStart->point.y + t*(nodeEnd->point.y-nodeStart->point.y);

        return sqrt(Hermes::sqr(point.x-x) + Hermes::sqr(point.y-y));
    }
    else
    {
        Point c = center();
        double R = radius();
        double distance = (point - c).magnitude();

        // point and radius are similar
        if (distance < EPS_ZERO) return R;

        Point t = (point - c) / distance;
        double l = ((point - c) - t * R).magnitude();
        double z = (t.angle() - (nodeStart->point - c).angle())/M_PI*180.0;
        if (z < 0) z = z + 360.0; // interval (0, 360)
        if ((z > 0) && (z < angle)) return l;

        double a = (point - nodeStart->point).magnitude();
        double b = (point - nodeEnd->point).magnitude();

        return qMin(a, b);
    }
}

int SceneEdge::segments() const
{
    double division = 40.0;
    int segments = angle/division + 1;
    if (segments < Util::config()->angleSegmentsCount)
        segments = Util::config()->angleSegmentsCount; // minimum segments

    return segments;
}

double SceneEdge::length() const
{
    if (isStraight())
        return (nodeEnd->point - nodeStart->point).magnitude();
    else
        return radius() * angle / 180.0 * M_PI;
}

int SceneEdge::showDialog(QWidget *parent, bool isNew)
{
    logMessage("SceneEdge::showDialog()");

    SceneEdgeDialog *dialog = new SceneEdgeDialog(this, parent, isNew);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabel::SceneLabel(const Point &point, SceneMaterial *marker, double area, int polynomialOrder)
    : MarkedSceneBasic()
{
    logMessage("SceneLabel::SceneLabel()");

    this->point = point;
    this->marker = marker;
    this->area = area;
    this->polynomialOrder = polynomialOrder;
}

double SceneLabel::distance(const Point &point) const
{
    logMessage("SceneLabel::distance()");

    return (this->point - point).magnitude();
}

int SceneLabel::showDialog(QWidget *parent, bool isNew)
{
    logMessage("SceneLabel::showDialog()");

    SceneLabelDialog *dialog = new SceneLabelDialog(this, parent, isNew);
    return dialog->exec();
}


template <typename BasicType>
QList<BasicType*> SceneBasicContainer<BasicType>::selected()
{
    QList<BasicType*> list;
    foreach (BasicType* item, list)
    {
        if (item->isSelected)
            list.push_back(item);
    }

    return list;
}

template <typename BasicType>
QList<BasicType*> SceneBasicContainer<BasicType>::highlited()
{
    QList<BasicType*> list;
    foreach (BasicType* item, list)
    {
        if (item->isHighlited)
            list.push_back(item);
    }

    return list;
}

template <typename BasicType>
bool SceneBasicContainer<BasicType>::add(BasicType *item)
{
    //TODO add check
    data.append(item);

    return true;
}

template bool SceneBasicContainer<SceneNode>::add(SceneNode *item);
template bool SceneBasicContainer<SceneEdge>::add(SceneEdge *item);
template bool SceneBasicContainer<SceneLabel>::add(SceneLabel *item);

template <typename BasicType>
bool SceneBasicContainer<BasicType>::remove(BasicType *item)
{
    return data.removeOne(item);
}

template bool SceneBasicContainer<SceneNode>::remove(SceneNode *item);
template bool SceneBasicContainer<SceneEdge>::remove(SceneEdge *item);
template bool SceneBasicContainer<SceneLabel>::remove(SceneLabel *item);

template <typename BasicType>
BasicType *SceneBasicContainer<BasicType>::at(int i)
{
    return data.at(i);
}

template SceneNode *SceneBasicContainer<SceneNode>::at(int i);
template SceneEdge *SceneBasicContainer<SceneEdge>::at(int i);
template SceneLabel *SceneBasicContainer<SceneLabel>::at(int i);

template <typename BasicType>
void SceneBasicContainer<BasicType>::clear()
{
    foreach (BasicType* item, data)
        delete item;

    data.clear();
}

template void SceneBasicContainer<SceneNode>::clear();
template void SceneBasicContainer<SceneEdge>::clear();
template void SceneBasicContainer<SceneLabel>::clear();

// *************************************************************************************************************************************
// *************************************************************************************************************************************

DSceneBasic::DSceneBasic(QWidget *parent, bool isNew) : QDialog(parent)
{
    logMessage("DSceneBasic::DSceneBasic()");

    this->isNew = isNew;
    layout = new QVBoxLayout();
}

DSceneBasic::~DSceneBasic()
{
    logMessage("DSceneBasic::~DSceneBasic()");

    delete layout;
}

void DSceneBasic::createControls()
{
    logMessage("DSceneBasic::createControls()");

    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    layout->addLayout(createContent());
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void DSceneBasic::doAccept()
{
    logMessage("DSceneBasic::doAccept()");

    if (save())
        accept();
}

void DSceneBasic::doReject()
{
    logMessage("DSceneBasic::doReject()");

    reject();
}

void DSceneBasic::evaluated(bool isError)
{
    logMessage("DSceneBasic::evaluated()");

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!isError);
}

// *************************************************************************************************************************************

DSceneNode::DSceneNode(SceneNode *node, QWidget *parent, bool isNew) : DSceneBasic(parent, isNew)
{
    logMessage("DSceneNode::DSceneNode()");

    m_object = node;

    setWindowIcon(icon("scene-node"));
    setWindowTitle(tr("Node"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    // setMaximumSize(sizeHint());
}

DSceneNode::~DSceneNode()
{
    logMessage("DSceneNode::~DSceneNode()");

    delete txtPointX;
    delete txtPointY;
}

QLayout* DSceneNode::createContent()
{
    logMessage("DSceneNode::createContent()");

    txtPointX = new ValueLineEdit();
    txtPointY = new ValueLineEdit();
    connect(txtPointX, SIGNAL(editingFinished()), this, SLOT(doEditingFinished()));
    connect(txtPointY, SIGNAL(editingFinished()), this, SLOT(doEditingFinished()));
    connect(txtPointX, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtPointY, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    lblDistance = new QLabel();
    lblAngle = new QLabel();

    // coordinates must be greater then or equal to 0 (axisymmetric case)
    if (Util::scene()->problemInfo()->coordinateType == CoordinateType_Axisymmetric)
        txtPointX->setMinimum(0.0);

    QFormLayout *layout = new QFormLayout();
    layout->addRow(Util::scene()->problemInfo()->labelX() + " (m):", txtPointX);
    layout->addRow(Util::scene()->problemInfo()->labelY() + " (m):", txtPointY);
    layout->addRow(tr("Distance:"), lblDistance);
    layout->addRow(tr("Angle:"), lblAngle);

    return layout;
}

bool DSceneNode::load()
{
    logMessage("DSceneNode::load()");

    SceneNode *sceneNode = dynamic_cast<SceneNode *>(m_object);

    txtPointX->setNumber(sceneNode->point.x);
    txtPointY->setNumber(sceneNode->point.y);

    doEditingFinished();

    return true;
}

bool DSceneNode::save()
{
    logMessage("DSceneNode::save()");

    if (!txtPointX->evaluate(false)) return false;
    if (!txtPointY->evaluate(false)) return false;

    SceneNode *sceneNode = dynamic_cast<SceneNode *>(m_object);

    Point point(txtPointX->number(), txtPointY->number());

    // check if node doesn't exists
    if (Util::scene()->getNode(point) && ((sceneNode->point != point) || isNew))
    {
        QMessageBox::warning(this, tr("Node"), tr("Node already exists."));
        return false;
    }

    if (!isNew)
    {
        if (sceneNode->point != point)
        {
            Util::scene()->undoStack()->push(new SceneNodeCommandEdit(sceneNode->point, point));
        }
    }

    sceneNode->point = point;

    return true;
}

void DSceneNode::doEditingFinished()
{
    logMessage("DSceneNode::doEditingFinished()");

    lblDistance->setText(QString("%1 m").arg(sqrt(Hermes::sqr(txtPointX->number()) + Hermes::sqr(txtPointY->number()))));
    lblAngle->setText(QString("%1 deg.").arg(
            (sqrt(Hermes::sqr(txtPointX->number()) + Hermes::sqr(txtPointY->number())) > EPS_ZERO)
            ? atan2(txtPointY->number(), txtPointX->number()) / M_PI * 180.0 : 0.0));
}

// *************************************************************************************************************************************

SceneEdgeDialog::SceneEdgeDialog(SceneEdge *edge, QWidget *parent, bool isNew) : DSceneBasic(parent, isNew)
{
    logMessage("DSceneEdge::DSceneEdge()");

    m_object = edge;

    setWindowIcon(icon("scene-edge"));
    setWindowTitle(tr("Edge"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    // setMaximumSize(sizeHint());
}

QLayout* SceneEdgeDialog::createContent()
{
    logMessage("DSceneEdge::createContent()");

    cmbNodeStart = new QComboBox();
    cmbNodeEnd = new QComboBox();
    connect(cmbNodeStart, SIGNAL(currentIndexChanged(int)), this, SLOT(doNodeChanged()));
    connect(cmbNodeEnd, SIGNAL(currentIndexChanged(int)), this, SLOT(doNodeChanged()));
    cmbBoundary = new QComboBox();
    connect(cmbBoundary, SIGNAL(currentIndexChanged(int)), this, SLOT(doBoundaryChanged(int)));
    btnBoundary = new QPushButton(icon("three-dots"), "");
    btnBoundary->setMaximumSize(btnBoundary->sizeHint());
    connect(btnBoundary, SIGNAL(clicked()), this, SLOT(doBoundaryClicked()));
    txtAngle = new ValueLineEdit();
    txtAngle->setMinimum(0.0);
    txtAngle->setMaximum(180.0);
    connect(txtAngle, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    txtRefineTowardsEdge = new QSpinBox(this);
    txtRefineTowardsEdge->setMinimum(0);
    txtRefineTowardsEdge->setMaximum(10);
    lblLength = new QLabel();

    // coordinates
    QFormLayout *layoutCoordinates = new QFormLayout();
    layoutCoordinates->addRow(tr("Start point:"), cmbNodeStart);
    layoutCoordinates->addRow(tr("End point:"), cmbNodeEnd);
    layoutCoordinates->addRow(tr("Angle (deg.):"), txtAngle);

    QGroupBox *grpCoordinates = new QGroupBox(tr("Coordinates"));
    grpCoordinates->setLayout(layoutCoordinates);

    // marker
    QHBoxLayout *layoutBoundary = new QHBoxLayout();
    layoutBoundary->addWidget(cmbBoundary);
    layoutBoundary->addWidget(btnBoundary);

    // refine towards edge
    chkRefineTowardsEdge = new QCheckBox();
    connect(chkRefineTowardsEdge, SIGNAL(stateChanged(int)), this, SLOT(doRefineTowardsEdge(int)));

    QHBoxLayout *layoutRefineTowardsEdge = new QHBoxLayout();
    layoutRefineTowardsEdge->addStretch(1);
    layoutRefineTowardsEdge->addWidget(chkRefineTowardsEdge);
    layoutRefineTowardsEdge->addWidget(txtRefineTowardsEdge);

    // mesh
    QFormLayout *layoutMeshParameters = new QFormLayout();
    layoutMeshParameters->addRow(tr("Refine towards edge:"), layoutRefineTowardsEdge);

    QGroupBox *grpMeshParameters = new QGroupBox(tr("Mesh parameters"));
    grpMeshParameters->setLayout(layoutMeshParameters);

    // layout
    QFormLayout *layout = new QFormLayout();
    layout->addRow(tr("Boundary condition:"), layoutBoundary);
    layout->addRow(grpCoordinates);
    layout->addRow(grpMeshParameters);
    layout->addRow(tr("Length:"), lblLength);

    fillComboBox();

    return layout;
}

void SceneEdgeDialog::fillComboBox()
{
    logMessage("DSceneEdge::fillComboBox()");

    // start and end nodes
    cmbNodeStart->clear();
    cmbNodeEnd->clear();
    for (int i = 0; i<Util::scene()->nodes->length(); i++)
    {
        cmbNodeStart->addItem(QString("%1 - [%2; %3]").
                              arg(i).
                              arg(Util::scene()->nodes->at(i)->point.x, 0, 'e', 2).
                              arg(Util::scene()->nodes->at(i)->point.y, 0, 'e', 2),
                              Util::scene()->nodes->at(i)->variant());
        cmbNodeEnd->addItem(QString("%1 - [%2; %3]").
                            arg(i).
                            arg(Util::scene()->nodes->at(i)->point.x, 0, 'e', 2).
                            arg(Util::scene()->nodes->at(i)->point.y, 0, 'e', 2),
                            Util::scene()->nodes->at(i)->variant());
    }

    // markers
    cmbBoundary->clear();
    foreach (SceneBoundary *boundary, Util::scene()->boundaries->items())
        cmbBoundary->addItem(QString::fromStdString(boundary->getName()),
                             boundary->variant());
}

bool SceneEdgeDialog::load()
{
    logMessage("DSceneEdge::load()");

    SceneEdge *sceneEdge = dynamic_cast<SceneEdge *>(m_object);

    cmbNodeStart->setCurrentIndex(cmbNodeStart->findData(sceneEdge->nodeStart->variant()));
    cmbNodeEnd->setCurrentIndex(cmbNodeEnd->findData(sceneEdge->nodeEnd->variant()));
    cmbBoundary->setCurrentIndex(cmbBoundary->findData(sceneEdge->marker->variant()));
    txtAngle->setNumber(sceneEdge->angle);
    chkRefineTowardsEdge->setChecked(sceneEdge->refineTowardsEdge > 0.0);
    txtRefineTowardsEdge->setEnabled(chkRefineTowardsEdge->isChecked());
    txtRefineTowardsEdge->setValue(sceneEdge->refineTowardsEdge);

    doNodeChanged();

    return true;
}

bool SceneEdgeDialog::save()
{
    logMessage("DSceneEdge::save()");

    if (!txtAngle->evaluate(false)) return false;

    SceneEdge *sceneEdge = dynamic_cast<SceneEdge *>(m_object);

    SceneNode *nodeStart = dynamic_cast<SceneNode *>(cmbNodeStart->itemData(cmbNodeStart->currentIndex()).value<SceneBasic *>());
    SceneNode *nodeEnd = dynamic_cast<SceneNode *>(cmbNodeEnd->itemData(cmbNodeEnd->currentIndex()).value<SceneBasic *>());

    // check if edge doesn't exists
    SceneEdge *edgeCheck = Util::scene()->getEdge(nodeStart->point, nodeEnd->point, txtAngle->number());
    if ((edgeCheck) && ((sceneEdge != edgeCheck) || isNew))
    {
        QMessageBox::warning(this, "Edge", "Edge already exists.");
        return false;
    }

    if (nodeStart == nodeEnd)
    {
        QMessageBox::warning(this, "Edge", "Start and end node are same.");
        return false;
    }

    if (!isNew)
    {
        if ((sceneEdge->nodeStart != nodeStart) || (sceneEdge->nodeEnd != nodeEnd) || (sceneEdge->angle != txtAngle->number()))
        {
            Util::scene()->undoStack()->push(new SceneEdgeCommandEdit(sceneEdge->nodeStart->point, sceneEdge->nodeEnd->point, nodeStart->point, nodeEnd->point,
                                                                      sceneEdge->angle, txtAngle->number(),
                                                                      sceneEdge->refineTowardsEdge, txtRefineTowardsEdge->value()));
        }
    }

    sceneEdge->nodeStart = nodeStart;
    sceneEdge->nodeEnd = nodeEnd;
    sceneEdge->marker = cmbBoundary->itemData(cmbBoundary->currentIndex()).value<SceneBoundary *>();
    sceneEdge->angle = txtAngle->number();
    sceneEdge->refineTowardsEdge = chkRefineTowardsEdge->isChecked() ? txtRefineTowardsEdge->value() : 0;

    return true;
}

void SceneEdgeDialog::doBoundaryChanged(int index)
{
    logMessage("DSceneEdge::doBoundaryChanged()");

    btnBoundary->setEnabled(cmbBoundary->currentIndex() > 0);
}

void SceneEdgeDialog::doBoundaryClicked()
{
    logMessage("DSceneEdge::doBoundaryClicked()");

    SceneBoundary *marker = cmbBoundary->itemData(cmbBoundary->currentIndex()).value<SceneBoundary *>();
    if (marker->showDialog(this) == QDialog::Accepted)
    {
        cmbBoundary->setItemText(cmbBoundary->currentIndex(), QString::fromStdString(marker->getName()));
        Util::scene()->refresh();
    }
}

void SceneEdgeDialog::doNodeChanged()
{
    logMessage("DSceneEdge::doNodeChanged()");

    SceneNode *nodeStart = dynamic_cast<SceneNode *>(cmbNodeStart->itemData(cmbNodeStart->currentIndex()).value<SceneBasic *>());
    SceneNode *nodeEnd = dynamic_cast<SceneNode *>(cmbNodeEnd->itemData(cmbNodeEnd->currentIndex()).value<SceneBasic *>());

    if (nodeStart && nodeEnd)
    {
        if (txtAngle->number() < EPS_ZERO)
        {
            // line
            lblLength->setText(QString("%1 m").arg(sqrt(Hermes::sqr(nodeEnd->point.x - nodeStart->point.x) + Hermes::sqr(nodeEnd->point.y - nodeStart->point.y))));
        }
        else
        {
            // arc
            SceneEdge edge(nodeStart, nodeEnd, Util::scene()->boundaries->get("none"), txtAngle->number(), 0); //TODO - do it better
            lblLength->setText(QString("%1 m").arg(edge.radius() * edge.angle / 180.0 * M_PI));
        }
    }
}

void SceneEdgeDialog::doRefineTowardsEdge(int state)
{
    logMessage("DSceneLabel::doRefineTowardsEdge()");

    txtRefineTowardsEdge->setEnabled(chkRefineTowardsEdge->isChecked());
}

// *************************************************************************************************************************************

SceneLabelDialog::SceneLabelDialog(SceneLabel *label, QWidget *parent, bool isNew) : DSceneBasic(parent, isNew)
{
    logMessage("DSceneLabel::DSceneLabel()");

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
    assert(0); //TODO
//    logMessage("DSceneLabel::createContent()");

//    txtPointX = new ValueLineEdit();
//    txtPointY = new ValueLineEdit();
//    connect(txtPointX, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
//    connect(txtPointY, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
//    cmbMaterial = new QComboBox();
//    connect(cmbMaterial, SIGNAL(currentIndexChanged(int)), this, SLOT(doMaterialChanged(int)));
//    btnMaterial = new QPushButton(icon("three-dots"), "");
//    btnMaterial->setMaximumSize(btnMaterial->sizeHint());
//    connect(btnMaterial, SIGNAL(clicked()), this, SLOT(doMaterialClicked()));
//    txtArea = new ValueLineEdit();
//    txtArea->setMinimum(0.0);
//    connect(txtArea, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
//    txtPolynomialOrder = new QSpinBox(this);
//    txtPolynomialOrder->setMinimum(0);
//    txtPolynomialOrder->setMaximum(10);

//    // coordinates must be greater then or equal to 0 (axisymmetric case)
//    if (Util::scene()->problemInfo()->problemType == ProblemType_Axisymmetric)
//        txtPointX->setMinimum(0.0);

//    // coordinates
//    QFormLayout *layoutCoordinates = new QFormLayout();
//    layoutCoordinates->addRow(Util::scene()->problemInfo()->labelX() + " (m):", txtPointX);
//    layoutCoordinates->addRow(Util::scene()->problemInfo()->labelY() + " (m):", txtPointY);

//    QGroupBox *grpCoordinates = new QGroupBox(tr("Coordinates"));
//    grpCoordinates->setLayout(layoutCoordinates);

//    // marker
//    QHBoxLayout *layoutMaterial = new QHBoxLayout();
//    layoutMaterial->addWidget(cmbMaterial);
//    layoutMaterial->addWidget(btnMaterial);

//    // order
//    chkPolynomialOrder = new QCheckBox();
//    connect(chkPolynomialOrder, SIGNAL(stateChanged(int)), this, SLOT(doPolynomialOrder(int)));

//    QHBoxLayout *layoutPolynomialOrder = new QHBoxLayout();
//    layoutPolynomialOrder->addWidget(chkPolynomialOrder);
//    layoutPolynomialOrder->addWidget(txtPolynomialOrder);
//    layoutPolynomialOrder->addWidget(new QLabel(tr("Global order is %1.").arg(Util::scene()->problemInfo()->polynomialOrder)));

//    // area
//    chkArea = new QCheckBox();
//    connect(chkArea, SIGNAL(stateChanged(int)), this, SLOT(doArea(int)));

//    QHBoxLayout *layoutArea = new QHBoxLayout();
//    layoutArea->addWidget(chkArea);
//    layoutArea->addWidget(txtArea);

//    // mesh
//    QFormLayout *layoutMeshParameters = new QFormLayout();
//    layoutMeshParameters->addRow(tr("Triangle area (m):"), layoutArea);
//    layoutMeshParameters->addRow(tr("Polynomial order (-):"), layoutPolynomialOrder);

//    QGroupBox *grpMeshParameters = new QGroupBox(tr("Mesh parameters"));
//    grpMeshParameters->setLayout(layoutMeshParameters);

//    QFormLayout *layout = new QFormLayout();
//    layout->addRow(tr("Material:"), layoutMaterial);
//    layout->addRow(grpCoordinates);
//    layout->addRow(grpMeshParameters);

//    fillComboBox();

//    return layout;
}

void SceneLabelDialog::fillComboBox()
{
    logMessage("DSceneLabel::fillComboBox()");

    // markers
    cmbMaterial->clear();
    foreach (SceneMaterial *material, Util::scene()->materials->items())
        cmbMaterial->addItem(QString::fromStdString(material->getName()),
                             material->variant());    
}

bool SceneLabelDialog::load()
{
    logMessage("DSceneLabel::load()");

    SceneLabel *sceneLabel = dynamic_cast<SceneLabel *>(m_object);

    txtPointX->setNumber(sceneLabel->point.x);
    txtPointY->setNumber(sceneLabel->point.y);
    cmbMaterial->setCurrentIndex(cmbMaterial->findData(sceneLabel->marker->variant()));
    txtArea->setNumber(sceneLabel->area);
    chkArea->setChecked(sceneLabel->area > 0.0);
    txtArea->setEnabled(chkArea->isChecked());
    txtPolynomialOrder->setValue(sceneLabel->polynomialOrder);
    chkPolynomialOrder->setChecked(sceneLabel->polynomialOrder > 0);
    txtPolynomialOrder->setEnabled(chkPolynomialOrder->isChecked());

    return true;
}

bool SceneLabelDialog::save()
{
    logMessage("DSceneLabel::save()");

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
    sceneLabel->marker = cmbMaterial->itemData(cmbMaterial->currentIndex()).value<SceneMaterial *>();
    sceneLabel->area = chkArea->isChecked() ? txtArea->number() : 0.0;
    sceneLabel->polynomialOrder = chkPolynomialOrder->isChecked() ? txtPolynomialOrder->value() : 0;

    return true;
}

void SceneLabelDialog::doMaterialChanged(int index)
{
    logMessage("DSceneLabel::doMaterialChanged()");

    btnMaterial->setEnabled(cmbMaterial->currentIndex() > 0);
}

void SceneLabelDialog::doMaterialClicked()
{
    logMessage("DSceneLabel::doMaterialClicked()");

    SceneMaterial *marker = cmbMaterial->itemData(cmbMaterial->currentIndex()).value<SceneMaterial *>();
    if (marker->showDialog(this) == QDialog::Accepted)
    {
        cmbMaterial->setItemText(cmbMaterial->currentIndex(), QString::fromStdString(marker->getName()));
        Util::scene()->refresh();
    }
}

void SceneLabelDialog::doPolynomialOrder(int state)
{
    logMessage("DSceneLabel::doPolynomialOrder()");

    txtPolynomialOrder->setEnabled(chkPolynomialOrder->isChecked());
}

void SceneLabelDialog::doArea(int state)
{
    logMessage("DSceneLabel::doArea()");

    txtArea->setEnabled(chkArea->isChecked());
}

// undo framework *******************************************************************************************************************

// Node

SceneNodeCommandAdd::SceneNodeCommandAdd(const Point &point, QUndoCommand *parent) : QUndoCommand(parent)
{
    logMessage("SceneNodeCommandAdd::SceneNodeCommandAdd()");

    m_point = point;
}

void SceneNodeCommandAdd::undo()
{
    logMessage("SceneNodeCommandAdd::undo()");

    SceneNode *node = Util::scene()->getNode(m_point);
    if (node)
    {
        Util::scene()->nodes->remove(node);
    }
}

void SceneNodeCommandAdd::redo()
{
    logMessage("SceneNodeCommandAdd::redo()");

    Util::scene()->addNode(new SceneNode(m_point));
}

SceneNodeCommandRemove::SceneNodeCommandRemove(const Point &point, QUndoCommand *parent) : QUndoCommand(parent)
{
    logMessage("SceneNodeCommandRemove::SceneNodeCommandRemove()");

    m_point = point;
}

void SceneNodeCommandRemove::undo()
{
    logMessage("SceneNodeCommandRemove::undo()");

    Util::scene()->addNode(new SceneNode(m_point));
}

void SceneNodeCommandRemove::redo()
{
    logMessage("SceneNodeCommandRemove::redo()");

    SceneNode *node = Util::scene()->getNode(m_point);
    if (node)
    {
        Util::scene()->nodes->remove(node);
    }
}

SceneNodeCommandEdit::SceneNodeCommandEdit(const Point &point, const Point &pointNew, QUndoCommand *parent) : QUndoCommand(parent)
{
    logMessage("SceneNodeCommandEdit::SceneNodeCommandEdit()");

    m_point = point;
    m_pointNew = pointNew;
}

void SceneNodeCommandEdit::undo()
{
    logMessage("SceneNodeCommandEdit::undo()");

    SceneNode *node = Util::scene()->getNode(m_pointNew);
    if (node)
    {
        node->point = m_point;
        Util::scene()->refresh();
    }
}

void SceneNodeCommandEdit::redo()
{
    logMessage("SceneNodeCommandEdit:redo:()");

    SceneNode *node = Util::scene()->getNode(m_point);
    if (node)
    {
        node->point = m_pointNew;
        Util::scene()->refresh();
    }
}

// Label

SceneLabelCommandAdd::SceneLabelCommandAdd(const Point &point, const QString &markerName, double area, int polynomialOrder, QUndoCommand *parent) : QUndoCommand(parent)
{
    logMessage("SceneLabelCommandAdd::SceneLabelCommandAdd()");

    m_point = point;
    m_markerName = markerName;
    m_area = area;
    m_polynomialOrder = polynomialOrder;
}

void SceneLabelCommandAdd::undo()
{
    logMessage("SceneLabelCommandAdd::undo()");

    Util::scene()->labels->remove(Util::scene()->getLabel(m_point));
}

void SceneLabelCommandAdd::redo()
{
    logMessage("SceneLabelCommandAdd::redo()");

    SceneMaterial *material = Util::scene()->getMaterial(m_markerName);
    if (material == NULL) material = Util::scene()->materials->get("none"); //TODO - do it better
    Util::scene()->addLabel(new SceneLabel(m_point, material, m_area, m_polynomialOrder));
}

SceneLabelCommandRemove::SceneLabelCommandRemove(const Point &point, const QString &markerName, double area, int polynomialOrder, QUndoCommand *parent) : QUndoCommand(parent)
{
    logMessage("SceneLabelCommandRemove::SceneLabelCommandRemove()");

    m_point = point;
    m_markerName = markerName;
    m_area = area;
    m_polynomialOrder = polynomialOrder;
}

void SceneLabelCommandRemove::undo()
{
    logMessage("SceneLabelCommandRemove::undo()");

    SceneMaterial *material = Util::scene()->getMaterial(m_markerName);
    if (material == NULL) material = Util::scene()->materials->get("none"); //TODO - do it better
    Util::scene()->addLabel(new SceneLabel(m_point, material, m_area, m_polynomialOrder));
}

void SceneLabelCommandRemove::redo()
{
    logMessage("SceneLabelCommandRemove::redo()");

    Util::scene()->labels->remove(Util::scene()->getLabel(m_point));
}

SceneLabelCommandEdit::SceneLabelCommandEdit(const Point &point, const Point &pointNew, QUndoCommand *parent) : QUndoCommand(parent)
{
    logMessage("SceneLabelCommandEdit::SceneLabelCommandEdit()");

    m_point = point;
    m_pointNew = pointNew;
}

void SceneLabelCommandEdit::undo()
{
    logMessage("SceneLabelCommandEdit::undo()");

    SceneLabel *label = Util::scene()->getLabel(m_pointNew);
    if (label)
    {
        label->point = m_point;
        Util::scene()->refresh();
    }
}

void SceneLabelCommandEdit::redo()
{
    logMessage("SceneLabelCommandEdit::redo()");

    SceneLabel *label = Util::scene()->getLabel(m_point);
    if (label)
    {
        label->point = m_pointNew;
        Util::scene()->refresh();
    }
}

// Edge

SceneEdgeCommandAdd::SceneEdgeCommandAdd(const Point &pointStart, const Point &pointEnd, const QString &markerName,
                                         double angle, int refineTowardsEdge, QUndoCommand *parent) : QUndoCommand(parent)
{
    logMessage("SceneEdgeCommandAdd::SceneEdgeCommandAdd()");

    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_markerName = markerName;
    m_angle = angle;
    m_refineTowardsEdge = refineTowardsEdge;
}

void SceneEdgeCommandAdd::undo()
{
    logMessage("SceneEdgeCommandAdd::undo()");

    Util::scene()->edges->remove(Util::scene()->getEdge(m_pointStart, m_pointEnd, m_angle));
}

void SceneEdgeCommandAdd::redo()
{
    logMessage("SceneEdgeCommandAdd::redo()");

    SceneBoundary *boundary = Util::scene()->getBoundary(m_markerName);
    if (boundary == NULL) boundary = Util::scene()->boundaries->get("none"); //TODO - do it better
    Util::scene()->addEdge(new SceneEdge(Util::scene()->addNode(new SceneNode(m_pointStart)),
                                         Util::scene()->addNode(new SceneNode(m_pointEnd)),
                                         boundary,
                                         m_angle,
                                         m_refineTowardsEdge));
}

SceneEdgeCommandRemove::SceneEdgeCommandRemove(const Point &pointStart, const Point &pointEnd, const QString &markerName,
                                               double angle, int refineTowardsEdge, QUndoCommand *parent) : QUndoCommand(parent)
{
    logMessage("SceneEdgeCommandRemove::SceneEdgeCommandRemove()");

    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_markerName = markerName;
    m_angle = angle;
    m_refineTowardsEdge = refineTowardsEdge;
}

void SceneEdgeCommandRemove::undo()
{
    logMessage("SceneEdgeCommandRemove::undo()");

    SceneBoundary *boundary = Util::scene()->getBoundary(m_markerName);
    if (boundary == NULL) boundary = Util::scene()->boundaries->get("none"); //TODO - do it better
    Util::scene()->addEdge(new SceneEdge(Util::scene()->addNode(new SceneNode(m_pointStart)),
                                         Util::scene()->addNode(new SceneNode(m_pointEnd)),
                                         boundary,
                                         m_angle,
                                         m_refineTowardsEdge));
}

void SceneEdgeCommandRemove::redo()
{
    logMessage("SceneEdgeCommandRemove::redo()");

    Util::scene()->edges->remove(Util::scene()->getEdge(m_pointStart, m_pointEnd, m_angle));
}

SceneEdgeCommandEdit::SceneEdgeCommandEdit(const Point &pointStart, const Point &pointEnd, const Point &pointStartNew, const Point &pointEndNew,
                                           double angle, double angleNew, int refineTowardsEdge, int refineTowardsEdgeNew, QUndoCommand *parent) : QUndoCommand(parent)
{
    logMessage("SceneEdgeCommandEdit::SceneEdgeCommandEdit()");

    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_pointStartNew = pointStartNew;
    m_pointEndNew = pointEndNew;
    m_angle = angle;
    m_angleNew = angleNew;
    m_refineTowardsEdge = refineTowardsEdge;
    m_refineTowardsEdgeNew = refineTowardsEdgeNew;
}

void SceneEdgeCommandEdit::undo()
{
    logMessage("SceneEdgeCommandEdit::undo()");

    SceneEdge *edge = Util::scene()->getEdge(m_pointStartNew, m_pointEndNew, m_angleNew);
    if (edge)
    {
        edge->nodeStart = Util::scene()->getNode(m_pointStart);
        edge->nodeEnd = Util::scene()->getNode(m_pointEnd);
        edge->angle = m_angle;
        edge->refineTowardsEdge = m_refineTowardsEdge;
        Util::scene()->refresh();
    }
}

void SceneEdgeCommandEdit::redo()
{
    logMessage("SceneEdgeCommandEdit::redo()");

    SceneEdge *edge = Util::scene()->getEdge(m_pointStart, m_pointEnd, m_angle);
    if (edge)
    {
        edge->nodeStart = Util::scene()->getNode(m_pointStartNew);
        edge->nodeEnd = Util::scene()->getNode(m_pointEndNew);
        edge->angle = m_angleNew;
        edge->refineTowardsEdge = m_refineTowardsEdge;
        Util::scene()->refresh();
    }
}
