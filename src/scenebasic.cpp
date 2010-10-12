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
#include "scene.h"

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
double SceneNode::distance(const Point &point)
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

SceneEdge::SceneEdge(SceneNode *nodeStart, SceneNode *nodeEnd, SceneEdgeMarker *marker, double angle) : SceneBasic()
{
    logMessage("SceneEdge::SceneEdge()");

    this->nodeStart = nodeStart;
    this->nodeEnd = nodeEnd;
    this->marker = marker;
    this->angle = angle;
}

Point SceneEdge::center()
{
    logMessage("SceneEdge::center()");

    double distance = (nodeEnd->point - nodeStart->point).magnitude();
    Point t = (nodeEnd->point - nodeStart->point) / distance;
    double R = distance / (2.0*sin(angle/180.0*M_PI / 2.0));

    Point p = Point(distance/2.0, sqrt(sqr(R) - sqr(distance)/4.0 > 0.0 ? sqr(R) - sqr(distance)/4.0 : 0.0));
    Point center = nodeStart->point + Point(p.x*t.x - p.y*t.y, p.x*t.y + p.y*t.x);

    return Point(center.x, center.y);
}

double SceneEdge::radius()
{
    logMessage("SceneEdge::radius()");

    return (center() - nodeStart->point).magnitude();
}

double SceneEdge::distance(const Point &point)
{
    logMessage("SceneEdge::distance()");

    if (angle == 0)
    {
        double t = ((point.x-nodeStart->point.x)*(nodeEnd->point.x-nodeStart->point.x) + (point.y-nodeStart->point.y)*(nodeEnd->point.y-nodeStart->point.y)) /
                   ((nodeEnd->point.x-nodeStart->point.x)*(nodeEnd->point.x-nodeStart->point.x) + (nodeEnd->point.y-nodeStart->point.y)*(nodeEnd->point.y-nodeStart->point.y));

        if (t > 1.0) t = 1.0;
        if (t < 0.0) t = 0.0;

        double x = nodeStart->point.x + t*(nodeEnd->point.x-nodeStart->point.x);
        double y = nodeStart->point.y + t*(nodeEnd->point.y-nodeStart->point.y);

        return sqrt(sqr(point.x-x) + sqr(point.y-y));
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

int SceneEdge::showDialog(QWidget *parent, bool isNew)
{
    logMessage("SceneEdge::showDialog()");

    DSceneEdge *dialog = new DSceneEdge(this, parent, isNew);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabel::SceneLabel(const Point &point, SceneLabelMarker *marker, double area, int polynomialOrder) : SceneBasic()
{
    logMessage("SceneLabel::SceneLabel()");

    this->point = point;
    this->marker = marker;
    this->area = area;
    this->polynomialOrder = polynomialOrder;
}

double SceneLabel::distance(const Point &point)
{
    logMessage("SceneLabel::distance()");

    return (this->point - point).magnitude();
}

int SceneLabel::showDialog(QWidget *parent, bool isNew)
{
    logMessage("SceneLabel::showDialog()");

    DSceneLabel *dialog = new DSceneLabel(this, parent, isNew);
    return dialog->exec();
}

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
    setMaximumSize(sizeHint());
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

    txtPointX = new SLineEditValue();
    txtPointY = new SLineEditValue();
    connect(txtPointX, SIGNAL(editingFinished()), this, SLOT(doEditingFinished()));
    connect(txtPointY, SIGNAL(editingFinished()), this, SLOT(doEditingFinished()));
    connect(txtPointX, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtPointY, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    lblDistance = new QLabel();
    lblAngle = new QLabel();

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
        QMessageBox::warning(this, "Node", "Node already exists.");
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

    lblDistance->setText(QString("%1 m").arg(sqrt(sqr(txtPointX->number()) + sqr(txtPointY->number()))));
    lblAngle->setText(QString("%1 deg.").arg(
            (sqrt(sqr(txtPointX->number()) + sqr(txtPointY->number())) > EPS_ZERO)
            ? atan2(txtPointY->number(), txtPointX->number()) / M_PI * 180.0 : 0.0));
}

// *************************************************************************************************************************************

DSceneEdge::DSceneEdge(SceneEdge *edge, QWidget *parent, bool isNew) : DSceneBasic(parent, isNew)
{
    logMessage("DSceneEdge::DSceneEdge()");

    m_object = edge;

    setWindowIcon(icon("scene-edge"));
    setWindowTitle(tr("Edge"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

DSceneEdge::~DSceneEdge()
{
    logMessage("DSceneEdge::~DSceneEdge()");

    delete cmbNodeStart;
    delete cmbNodeEnd;
    delete cmbMarker;
    delete btnMarker;
    delete txtAngle;
}

QLayout* DSceneEdge::createContent()
{
    logMessage("DSceneEdge::createContent()");

    cmbNodeStart = new QComboBox();
    cmbNodeEnd = new QComboBox();
    connect(cmbNodeStart, SIGNAL(currentIndexChanged(int)), this, SLOT(doNodeChanged()));
    connect(cmbNodeEnd, SIGNAL(currentIndexChanged(int)), this, SLOT(doNodeChanged()));
    cmbMarker = new QComboBox();
    connect(cmbMarker, SIGNAL(currentIndexChanged(int)), this, SLOT(doMarkerChanged(int)));
    btnMarker = new QPushButton("...");
    btnMarker->setMaximumWidth(25);
    connect(btnMarker, SIGNAL(clicked()), this, SLOT(doMarkerClicked()));
    txtAngle = new SLineEditValue();
    txtAngle->setMinimum(0.0);
    txtAngle->setMaximum(180.0);
    connect(txtAngle, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    lblLength = new QLabel();

    QHBoxLayout *layoutMarker = new QHBoxLayout();
    layoutMarker->addWidget(cmbMarker);
    layoutMarker->addWidget(btnMarker);

    QFormLayout *layout = new QFormLayout();
    layout->addRow(tr("Start point:"), cmbNodeStart);
    layout->addRow(tr("End point:"), cmbNodeEnd);
    layout->addRow(tr("Boundary condition:"), layoutMarker);
    layout->addRow(tr("Angle (deg.):"), txtAngle);
    layout->addRow(tr("Length:"), lblLength);

    fillComboBox();

    return layout;
}

void DSceneEdge::fillComboBox()
{
    logMessage("DSceneEdge::fillComboBox()");

    // start and end nodes
    cmbNodeStart->clear();
    cmbNodeEnd->clear();
    for (int i = 0; i<Util::scene()->nodes.count(); i++)
    {
        cmbNodeStart->addItem(QString("%1 - [%2; %3]").
                              arg(i).
                              arg(Util::scene()->nodes[i]->point.x, 0, 'e', 2).
                              arg(Util::scene()->nodes[i]->point.y, 0, 'e', 2),
                              Util::scene()->nodes[i]->variant());
        cmbNodeEnd->addItem(QString("%1 - [%2; %3]").
                            arg(i).
                            arg(Util::scene()->nodes[i]->point.x, 0, 'e', 2).
                            arg(Util::scene()->nodes[i]->point.y, 0, 'e', 2),
                            Util::scene()->nodes[i]->variant());
    }

    // markers
    cmbMarker->clear();
    for (int i = 0; i<Util::scene()->edgeMarkers.count(); i++)
    {
        cmbMarker->addItem(Util::scene()->edgeMarkers[i]->name, Util::scene()->edgeMarkers[i]->variant());
    }
}

bool DSceneEdge::load()
{
    logMessage("DSceneEdge::load()");

    SceneEdge *sceneEdge = dynamic_cast<SceneEdge *>(m_object);

    cmbNodeStart->setCurrentIndex(cmbNodeStart->findData(sceneEdge->nodeStart->variant()));
    cmbNodeEnd->setCurrentIndex(cmbNodeEnd->findData(sceneEdge->nodeEnd->variant()));
    cmbMarker->setCurrentIndex(cmbMarker->findData(sceneEdge->marker->variant()));
    txtAngle->setNumber(sceneEdge->angle);

    doNodeChanged();

    return true;
}

bool DSceneEdge::save()
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
            Util::scene()->undoStack()->push(new SceneEdgeCommandEdit(sceneEdge->nodeStart->point, sceneEdge->nodeEnd->point, nodeStart->point, nodeEnd->point, sceneEdge->angle, txtAngle->number()));
        }
    }

    sceneEdge->nodeStart = nodeStart;
    sceneEdge->nodeEnd = nodeEnd;
    sceneEdge->marker = cmbMarker->itemData(cmbMarker->currentIndex()).value<SceneEdgeMarker *>();
    sceneEdge->angle = txtAngle->number();

    return true;
}

void DSceneEdge::doMarkerChanged(int index)
{
    logMessage("DSceneEdge::doMarkerChanged()");

    btnMarker->setEnabled(cmbMarker->currentIndex() > 0);
}

void DSceneEdge::doMarkerClicked()
{
    logMessage("DSceneEdge::doMarkerClicked()");

    SceneEdgeMarker *marker = cmbMarker->itemData(cmbMarker->currentIndex()).value<SceneEdgeMarker *>();
    if (marker->showDialog(this) == QDialog::Accepted)
    {
        cmbMarker->setItemText(cmbMarker->currentIndex(), marker->name);
        Util::scene()->refresh();
    }
}

void DSceneEdge::doNodeChanged()
{
    logMessage("DSceneEdge::doNodeChanged()");

    SceneNode *nodeStart = dynamic_cast<SceneNode *>(cmbNodeStart->itemData(cmbNodeStart->currentIndex()).value<SceneBasic *>());
    SceneNode *nodeEnd = dynamic_cast<SceneNode *>(cmbNodeEnd->itemData(cmbNodeEnd->currentIndex()).value<SceneBasic *>());

    if (nodeStart && nodeEnd)
    {
        if (txtAngle->number() < EPS_ZERO)
        {
            // line
            lblLength->setText(QString("%1 m").arg(sqrt(sqr(nodeEnd->point.x - nodeStart->point.x) + sqr(nodeEnd->point.y - nodeStart->point.y))));
        }
        else
        {
            // arc
            SceneEdge edge(nodeStart, nodeEnd, Util::scene()->edgeMarkers[0], txtAngle->number());
            lblLength->setText(QString("%1 m").arg(edge.radius() * edge.angle / 180.0 * M_PI));
        }
    }
}

// *************************************************************************************************************************************

DSceneLabel::DSceneLabel(SceneLabel *label, QWidget *parent, bool isNew) : DSceneBasic(parent, isNew)
{
    logMessage("DSceneLabel::DSceneLabel()");

    m_object = label;

    setWindowIcon(icon("scene-label"));
    setWindowTitle(tr("Label"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

DSceneLabel::~DSceneLabel()
{
    logMessage("DSceneLabel::~DSceneLabel()");

    delete txtPointX;
    delete txtPointY;
    delete cmbMarker;
    delete btnMarker;
    delete txtArea;
}

QLayout* DSceneLabel::createContent()
{
    logMessage("DSceneLabel::createContent()");

    txtPointX = new SLineEditValue();
    txtPointY = new SLineEditValue();
    connect(txtPointX, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtPointY, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    cmbMarker = new QComboBox();
    connect(cmbMarker, SIGNAL(currentIndexChanged(int)), this, SLOT(doMarkerChanged(int)));
    btnMarker = new QPushButton("...");
    btnMarker->setMaximumSize(btnMarker->sizeHint());
    connect(btnMarker, SIGNAL(clicked()), this, SLOT(doMarkerClicked()));
    txtArea = new SLineEditValue();
    connect(txtArea, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    txtPolynomialOrder = new QSpinBox(this);
    txtPolynomialOrder->setMinimum(0);
    txtPolynomialOrder->setMaximum(10);

    // marker
    QHBoxLayout *layoutMarker = new QHBoxLayout();
    layoutMarker->addWidget(cmbMarker);
    layoutMarker->addWidget(btnMarker);

    // order
    chkPolynomialOrder = new QCheckBox();
    connect(chkPolynomialOrder, SIGNAL(stateChanged(int)), this, SLOT(doPolynomialOrder(int)));

    QHBoxLayout *layoutPolynomialOrder = new QHBoxLayout();
    layoutPolynomialOrder->addWidget(chkPolynomialOrder);
    layoutPolynomialOrder->addWidget(txtPolynomialOrder);
    layoutPolynomialOrder->addWidget(new QLabel(tr("Global order is %1.").arg(Util::scene()->problemInfo()->polynomialOrder)));

    // area
    chkArea = new QCheckBox();
    connect(chkArea, SIGNAL(stateChanged(int)), this, SLOT(doArea(int)));

    QHBoxLayout *layoutArea = new QHBoxLayout();
    layoutArea->addWidget(chkArea);
    layoutArea->addWidget(txtArea);

    QFormLayout *layoutMeshParameters = new QFormLayout();
    layoutMeshParameters->addRow(tr("Triangle area (m):"), layoutArea);
    layoutMeshParameters->addRow(tr("Polynomial order (-):"), layoutPolynomialOrder);

    QGroupBox *grpMeshParameters = new QGroupBox(tr("Mesh parameters"));
    grpMeshParameters->setLayout(layoutMeshParameters);

    QFormLayout *layout = new QFormLayout();
    layout->addRow(Util::scene()->problemInfo()->labelX() + " (m):", txtPointX);
    layout->addRow(Util::scene()->problemInfo()->labelY() + " (m):", txtPointY);
    layout->addRow(tr("Material:"), layoutMarker);
    layout->addRow(grpMeshParameters);

    fillComboBox();

    return layout;
}

void DSceneLabel::fillComboBox()
{
    logMessage("DSceneLabel::fillComboBox()");

    // markers
    cmbMarker->clear();
    for (int i = 0; i<Util::scene()->labelMarkers.count(); i++)
    {
        cmbMarker->addItem(Util::scene()->labelMarkers[i]->name, Util::scene()->labelMarkers[i]->variant());
    }
}

bool DSceneLabel::load()
{
    logMessage("DSceneLabel::load()");

    SceneLabel *sceneLabel = dynamic_cast<SceneLabel *>(m_object);

    txtPointX->setNumber(sceneLabel->point.x);
    txtPointY->setNumber(sceneLabel->point.y);
    cmbMarker->setCurrentIndex(cmbMarker->findData(sceneLabel->marker->variant()));
    txtArea->setNumber(sceneLabel->area);
    chkArea->setChecked(sceneLabel->area != 0.0);
    txtArea->setEnabled(chkArea->isChecked());
    txtPolynomialOrder->setValue(sceneLabel->polynomialOrder);
    chkPolynomialOrder->setChecked(sceneLabel->polynomialOrder > 0);
    txtPolynomialOrder->setEnabled(chkPolynomialOrder->isChecked());

    return true;
}

bool DSceneLabel::save()
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
    if (txtArea->value().number < 0)
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
    sceneLabel->marker = cmbMarker->itemData(cmbMarker->currentIndex()).value<SceneLabelMarker *>();
    sceneLabel->area = chkArea->isChecked() ? txtArea->number() : 0.0;
    sceneLabel->polynomialOrder = chkPolynomialOrder->isChecked() ? txtPolynomialOrder->value() : 0;

    return true;
}

void DSceneLabel::doMarkerChanged(int index)
{
    logMessage("DSceneLabel::doMarkerChanged()");

    btnMarker->setEnabled(cmbMarker->currentIndex() > 0);
}

void DSceneLabel::doMarkerClicked()
{
    logMessage("DSceneLabel::doMarkerClicked()");

    SceneLabelMarker *marker = cmbMarker->itemData(cmbMarker->currentIndex()).value<SceneLabelMarker *>();
    if (marker->showDialog(this) == QDialog::Accepted)
    {
        cmbMarker->setItemText(cmbMarker->currentIndex(), marker->name);
        Util::scene()->refresh();
    }
}

void DSceneLabel::doPolynomialOrder(int state)
{
    logMessage("DSceneLabel::()doPolynomialOrder");

    txtPolynomialOrder->setEnabled(chkPolynomialOrder->isChecked());
}

void DSceneLabel::doArea(int state)
{
    logMessage("DSceneLabel::()doArea");

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
        Util::scene()->removeNode(node);
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
        Util::scene()->removeNode(node);
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

    Util::scene()->removeLabel(Util::scene()->getLabel(m_point));
}

void SceneLabelCommandAdd::redo()
{
    logMessage("SceneLabelCommandAdd::redo()");

    SceneLabelMarker *labelMarker = Util::scene()->getLabelMarker(m_markerName);
    if (labelMarker == NULL) labelMarker = Util::scene()->labelMarkers[0];
    Util::scene()->addLabel(new SceneLabel(m_point, labelMarker, m_area, m_polynomialOrder));
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

    SceneLabelMarker *labelMarker = Util::scene()->getLabelMarker(m_markerName);
    if (labelMarker == NULL) labelMarker = Util::scene()->labelMarkers[0];
    Util::scene()->addLabel(new SceneLabel(m_point, labelMarker, m_area, m_polynomialOrder));
}

void SceneLabelCommandRemove::redo()
{
    logMessage("SceneLabelCommandRemove::redo()");

    Util::scene()->removeLabel(Util::scene()->getLabel(m_point));
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

SceneEdgeCommandAdd::SceneEdgeCommandAdd(const Point &pointStart, const Point &pointEnd, const QString &markerName, double angle, QUndoCommand *parent) : QUndoCommand(parent)
{
    logMessage("SceneEdgeCommandAdd::SceneEdgeCommandAdd()");

    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_markerName = markerName;
    m_angle = angle;
}

void SceneEdgeCommandAdd::undo()
{
    logMessage("SceneEdgeCommandAdd::undo()");

    Util::scene()->removeEdge(Util::scene()->getEdge(m_pointStart, m_pointEnd, m_angle));
}

void SceneEdgeCommandAdd::redo()
{
    logMessage("SceneEdgeCommandAdd::redo()");

    SceneEdgeMarker *edgeMarker = Util::scene()->getEdgeMarker(m_markerName);
    if (edgeMarker == NULL) edgeMarker = Util::scene()->edgeMarkers[0];
    Util::scene()->addEdge(new SceneEdge(Util::scene()->addNode(new SceneNode(m_pointStart)),
                                         Util::scene()->addNode(new SceneNode(m_pointEnd)),
                                         edgeMarker,
                                         m_angle));
}

SceneEdgeCommandRemove::SceneEdgeCommandRemove(const Point &pointStart, const Point &pointEnd, const QString &markerName, double angle, QUndoCommand *parent) : QUndoCommand(parent)
{
    logMessage("SceneEdgeCommandRemove::SceneEdgeCommandRemove()");

    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_markerName = markerName;
    m_angle = angle;
}

void SceneEdgeCommandRemove::undo()
{
    logMessage("SceneEdgeCommandRemove::undo()");

    SceneEdgeMarker *edgeMarker = Util::scene()->getEdgeMarker(m_markerName);
    if (edgeMarker == NULL) edgeMarker = Util::scene()->edgeMarkers[0];
    Util::scene()->addEdge(new SceneEdge(Util::scene()->addNode(new SceneNode(m_pointStart)),
                                         Util::scene()->addNode(new SceneNode(m_pointEnd)),
                                         edgeMarker,
                                         m_angle));
}

void SceneEdgeCommandRemove::redo()
{
    logMessage("SceneEdgeCommandRemove::redo()");

    Util::scene()->removeEdge(Util::scene()->getEdge(m_pointStart, m_pointEnd, m_angle));
}

SceneEdgeCommandEdit::SceneEdgeCommandEdit(const Point &pointStart, const Point &pointEnd, const Point &pointStartNew, const Point &pointEndNew, double angle, double angleNew, QUndoCommand *parent) : QUndoCommand(parent)
{
    logMessage("SceneEdgeCommandEdit::SceneEdgeCommandEdit()");

    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_pointStartNew = pointStartNew;
    m_pointEndNew = pointEndNew;
    m_angle = angle;
    m_angleNew = angleNew;
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
        Util::scene()->refresh();
    }
}
