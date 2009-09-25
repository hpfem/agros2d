#include "scenebasic.h"

SceneBasic::SceneBasic()
{
    isSelected = false;
    isHighlighted = false;
}

SceneBasic::~SceneBasic()
{
}

QVariant SceneBasic::variant()
{
    QVariant v;
    v.setValue(this);
    return v;
}

// *************************************************************************************************************************************

SceneNode::SceneNode(const Point &point) : SceneBasic()
{
    this->point = point;
}

SceneNode::~SceneNode()
{
}

double SceneNode::distance(const Point &point)
{
    return (this->point - point).magnitude();
}

int SceneNode::showDialog(QWidget *parent)
{
    DSceneNode *dialog = new DSceneNode(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneEdge::SceneEdge(SceneNode *nodeStart, SceneNode *nodeEnd, SceneEdgeMarker *marker, double angle) : SceneBasic()
{
    this->nodeStart = nodeStart;
    this->nodeEnd = nodeEnd;
    this->marker = marker;
    this->angle = angle;
}

SceneEdge::~SceneEdge()
{
}

Point SceneEdge::center()
{
    double distance = (nodeEnd->point - nodeStart->point).magnitude();
    Point t = (nodeEnd->point - nodeStart->point) / distance;
    double R = distance / (2.0*sin(angle/180.0*M_PI / 2.0));

    Point p = Point(distance/2.0, sqrt(sqr(R) - sqr(distance)/4.0));
    Point center = nodeStart->point + Point(p.x*t.x - p.y*t.y, p.x*t.y + p.y*t.x);

    return Point(center.x, center.y);
}

double SceneEdge::radius()
{
    return (center() - nodeStart->point).magnitude();
}

double SceneEdge::distance(const Point &point)
{
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
        double distance = (point-c).magnitude();

        // point and radius are similar        
        if (distance < EPS_ZERO) return radius();

        Point t = (point - c) / distance;
        double l = ((point - c) - t * R).magnitude();
        double z = fabs(t.angle() - (nodeStart->point - c).angle())/180*M_PI;
        if ((z > 0) && (z < angle)) return l;

        double a = (point - nodeStart->point).magnitude();
        double b = (point - nodeEnd->point).magnitude();
        if (a<b)
            return a;
        else
            return b;
    }
}

int SceneEdge::showDialog(QWidget *parent)
{
    DSceneEdge *dialog = new DSceneEdge(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

SceneLabel::SceneLabel(const Point &point, SceneLabelMarker *marker, double area) : SceneBasic()
{
    this->point = point;
    this->marker = marker;
    this->area = area;
}

SceneLabel::~SceneLabel()
{
}

double SceneLabel::distance(const Point &point)
{
    return (this->point - point).magnitude();
}

int SceneLabel::showDialog(QWidget *parent)
{
    DSceneLabel *dialog = new DSceneLabel(this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

DSceneBasic::DSceneBasic(QWidget *parent) : QDialog(parent)
{
    layout = new QVBoxLayout();
}

DSceneBasic::~DSceneBasic()
{
    delete layout;
}

void DSceneBasic::createControls()
{
    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    layout->addLayout(createContent());
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void DSceneBasic::doAccept()
{
    if (save())
        accept();
}

void DSceneBasic::doReject()
{
    reject();
}

// *************************************************************************************************************************************

DSceneNode::DSceneNode(SceneNode *node, QWidget *parent) : DSceneBasic(parent)
{
    m_object = node;

    setWindowIcon(icon("scene-node"));
    setWindowTitle(tr("Node"));

    createControls();

    load();

    resize(sizeHint());
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}    

DSceneNode::~DSceneNode()
{
    delete txtPointX;
    delete txtPointY;
}

QLayout* DSceneNode::createContent()
{
    txtPointX = new SLineEdit("0", false);
    txtPointY = new SLineEdit("0", false);

    QFormLayout *layout = new QFormLayout();
    layout->addRow(Util::scene()->problemInfo().labelX() + " (m):", txtPointX);
    layout->addRow(Util::scene()->problemInfo().labelY() + " (m):", txtPointY);

    return layout;
}

bool DSceneNode::load()
{
    SceneNode *sceneNode = dynamic_cast<SceneNode *>(m_object);

    txtPointX->setText(QString::number(sceneNode->point.x));
    txtPointY->setText(QString::number(sceneNode->point.y));

    return true;
}

bool DSceneNode::save()
{
    SceneNode *sceneNode = dynamic_cast<SceneNode *>(m_object);

    Point point(txtPointX->value(), txtPointY->value());

    if (sceneNode->point != point)
    {
        Util::scene()->undoStack()->push(new SceneNodeCommandEdit(sceneNode->point, point));
        sceneNode->point = point;
    }

    return true;
}

// *************************************************************************************************************************************

DSceneEdge::DSceneEdge(SceneEdge *edge, QWidget *parent) : DSceneBasic(parent)
{
    m_object = edge;

    setWindowIcon(icon("scene-edge"));
    setWindowTitle(tr("Edge"));

    createControls();

    load();

    resize(sizeHint());
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

DSceneEdge::~DSceneEdge()
{
    delete cmbNodeStart;
    delete cmbNodeEnd;
    delete cmbMarker;
    delete txtAngle;
}

QLayout* DSceneEdge::createContent()
{
    cmbNodeStart = new QComboBox();
    cmbNodeEnd = new QComboBox();
    cmbMarker = new QComboBox();
    txtAngle = new SLineEdit("0");

    QFormLayout *layout = new QFormLayout();
    layout->addRow(tr("Start point:"), cmbNodeStart);
    layout->addRow(tr("End point:"), cmbNodeEnd);
    layout->addRow(tr("Boundary condition:"), cmbMarker);
    layout->addRow(tr("Angle (deg.):"), txtAngle);

    fillComboBox();

    return layout;
}

void DSceneEdge::fillComboBox()
{
    // start and end nodes
    cmbNodeStart->clear();
    cmbNodeEnd->clear();
    for (int i = 0; i<Util::scene()->nodes.count(); i++)
    {
        cmbNodeStart->addItem(QString("[%1; %2]").arg(Util::scene()->nodes[i]->point.x, 0, 'e', 2).arg(Util::scene()->nodes[i]->point.y, 0, 'e', 2),
                              Util::scene()->nodes[i]->variant());
        cmbNodeEnd->addItem(QString("[%1; %2]").arg(Util::scene()->nodes[i]->point.x, 0, 'e', 2).arg(Util::scene()->nodes[i]->point.y, 0, 'e', 2),
                            Util::scene()->nodes[i]->variant());
    }

    // markers
    cmbMarker->clear();
    for (int i = 0; i<Util::scene()->edgeMarkers.count(); i++)
    {
        cmbMarker->addItem(Util::scene()->edgeMarkers[i]->name, Util::scene()->edgeMarkers[i]->variant());
    }
}

bool DSceneEdge::load() {
    SceneEdge *sceneEdge = dynamic_cast<SceneEdge *>(m_object);

    cmbNodeStart->setCurrentIndex(cmbNodeStart->findData(sceneEdge->nodeStart->variant()));
    cmbNodeEnd->setCurrentIndex(cmbNodeEnd->findData(sceneEdge->nodeEnd->variant()));
    cmbMarker->setCurrentIndex(cmbMarker->findData(sceneEdge->marker->variant()));
    txtAngle->setText(QString::number(sceneEdge->angle));

    return true;
}

bool DSceneEdge::save() {
    SceneEdge *sceneEdge = dynamic_cast<SceneEdge *>(m_object);

    SceneNode *nodeStart = dynamic_cast<SceneNode *>(cmbNodeStart->itemData(cmbNodeStart->currentIndex()).value<SceneBasic *>());
    SceneNode *nodeEnd = dynamic_cast<SceneNode *>(cmbNodeEnd->itemData(cmbNodeEnd->currentIndex()).value<SceneBasic *>());

    if (nodeStart == nodeEnd)
    {
        QMessageBox::warning(this, "Nodes", "Start and end node are same.");
        return false;
    }

    if ((sceneEdge->nodeStart != nodeStart) || (sceneEdge->nodeEnd != nodeEnd))
    {
        Util::scene()->undoStack()->push(new SceneEdgeCommandEdit(sceneEdge->nodeStart->point, sceneEdge->nodeEnd->point, nodeStart->point, nodeEnd->point));

        sceneEdge->nodeStart = nodeStart;
        sceneEdge->nodeEnd = nodeEnd;
    }   
    sceneEdge->marker = cmbMarker->itemData(cmbMarker->currentIndex()).value<SceneEdgeMarker *>();
    sceneEdge->angle = txtAngle->value();

    return true;
}

// *************************************************************************************************************************************

DSceneLabel::DSceneLabel(SceneLabel *label, QWidget *parent) : DSceneBasic(parent) {
    m_object = label;

    setWindowIcon(icon("scene-label"));
    setWindowTitle(tr("Label"));

    createControls();

    load();

    resize(sizeHint());
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

DSceneLabel::~DSceneLabel()
{
    delete txtPointX;
    delete txtPointY;
}

QLayout* DSceneLabel::createContent() {
    txtPointX = new SLineEdit("0", false);
    txtPointX->setValidator(new QDoubleValidator(txtPointX));
    txtPointY = new SLineEdit("0", false);
    txtPointY->setValidator(new QDoubleValidator(txtPointY));
    cmbMarker = new QComboBox();
    txtArea = new SLineEdit("0");
    txtArea->setValidator(new QDoubleValidator(txtArea));

    QFormLayout *layout = new QFormLayout();
    layout->addRow(Util::scene()->problemInfo().labelX() + " (m):", txtPointX);
    layout->addRow(Util::scene()->problemInfo().labelY() + " (m):", txtPointY);
    layout->addRow(tr("Material:"), cmbMarker);
    layout->addRow(tr("Triangle area (m):"), txtArea);

    fillComboBox();

    return layout;
}

void DSceneLabel::fillComboBox()
{
    // markers
    cmbMarker->clear();
    for (int i = 0; i<Util::scene()->labelMarkers.count(); i++)
    {
        cmbMarker->addItem(Util::scene()->labelMarkers[i]->name, Util::scene()->labelMarkers[i]->variant());
    }
}

bool DSceneLabel::load()
{
    SceneLabel *sceneLabel = dynamic_cast<SceneLabel *>(m_object);

    txtPointX->setText(QString::number(sceneLabel->point.x));
    txtPointY->setText(QString::number(sceneLabel->point.y));
    cmbMarker->setCurrentIndex(cmbMarker->findData(sceneLabel->marker->variant()));
    txtArea->setText(QString::number(sceneLabel->area));

    return true;
}

bool DSceneLabel::save()
{
    SceneLabel *sceneLabel = dynamic_cast<SceneLabel *>(m_object);

    Point point(txtPointX->value(), txtPointY->value());

    if (sceneLabel->point != point)
    {
        Util::scene()->undoStack()->push(new SceneLabelCommandEdit(sceneLabel->point, point));
        sceneLabel->point = point;
    }
    sceneLabel->marker = cmbMarker->itemData(cmbMarker->currentIndex()).value<SceneLabelMarker *>();
    sceneLabel->area = txtArea->value();

    return true;
}

// undo framework *******************************************************************************************************************

// Node

SceneNodeCommandAdd::SceneNodeCommandAdd(const Point &point, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_point = point;
}

void SceneNodeCommandAdd::undo()
{
    SceneNode *node = Util::scene()->getNode(m_point);
    if (node)
    {
        Util::scene()->removeNode(node);
    }
}

void SceneNodeCommandAdd::redo()
{
    Util::scene()->addNode(new SceneNode(m_point));
}

SceneNodeCommandRemove::SceneNodeCommandRemove(const Point &point, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_point = point;
}

void SceneNodeCommandRemove::undo()
{
    Util::scene()->addNode(new SceneNode(m_point));
}

void SceneNodeCommandRemove::redo()
{
    SceneNode *node = Util::scene()->getNode(m_point);
    if (node)
    {
        Util::scene()->removeNode(node);
    }
}

SceneNodeCommandEdit::SceneNodeCommandEdit(const Point &point, const Point &pointNew, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_point = point;
    m_pointNew = pointNew;
}

void SceneNodeCommandEdit::undo()
{
    SceneNode *node = Util::scene()->getNode(m_pointNew);
    if (node)
    {
        node->point = m_point;
        Util::scene()->refresh();
    }
}

void SceneNodeCommandEdit::redo()
{
    SceneNode *node = Util::scene()->getNode(m_point);
    if (node)
    {
        node->point = m_pointNew;
        Util::scene()->refresh();
    }
}

// Label

SceneLabelCommandAdd::SceneLabelCommandAdd(const Point &point, const QString &markerName, double area, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_point = point;
    m_markerName = markerName;
    m_area = area;
}

void SceneLabelCommandAdd::undo()
{
    Util::scene()->removeLabel(Util::scene()->getLabel(m_point));
}

void SceneLabelCommandAdd::redo()
{
    SceneLabelMarker *labelMarker = Util::scene()->getLabelMarker(m_markerName);
    if (labelMarker == NULL) labelMarker = Util::scene()->labelMarkers[0];
    Util::scene()->addLabel(new SceneLabel(m_point, labelMarker, m_area));
}

SceneLabelCommandRemove::SceneLabelCommandRemove(const Point &point, const QString &markerName, double area, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_point = point;
    m_markerName = markerName;
    m_area = area;
}

void SceneLabelCommandRemove::undo()
{
    SceneLabelMarker *labelMarker = Util::scene()->getLabelMarker(m_markerName);
    if (labelMarker == NULL) labelMarker = Util::scene()->labelMarkers[0];
    Util::scene()->addLabel(new SceneLabel(m_point, labelMarker, m_area));
}

void SceneLabelCommandRemove::redo()
{
    Util::scene()->removeLabel(Util::scene()->getLabel(m_point));
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

// Edge

SceneEdgeCommandAdd::SceneEdgeCommandAdd(const Point &pointStart, const Point &pointEnd, const QString &markerName, double angle, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_markerName = markerName;
    m_angle = angle;
}

void SceneEdgeCommandAdd::undo()
{
    Util::scene()->removeEdge(Util::scene()->getEdge(m_pointStart, m_pointEnd));
}

void SceneEdgeCommandAdd::redo()
{
    SceneEdgeMarker *edgeMarker = Util::scene()->getEdgeMarker(m_markerName);
    if (edgeMarker == NULL) edgeMarker = Util::scene()->edgeMarkers[0];
    Util::scene()->addEdge(new SceneEdge(Util::scene()->addNode(new SceneNode(m_pointStart)), Util::scene()->addNode(new SceneNode(m_pointEnd)), edgeMarker, m_angle));
}

SceneEdgeCommandRemove::SceneEdgeCommandRemove(const Point &pointStart, const Point &pointEnd, const QString &markerName, double angle, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_markerName = markerName;
    m_angle = angle;
}

void SceneEdgeCommandRemove::undo()
{
    SceneEdgeMarker *edgeMarker = Util::scene()->getEdgeMarker(m_markerName);
    if (edgeMarker == NULL) edgeMarker = Util::scene()->edgeMarkers[0];
    Util::scene()->addEdge(new SceneEdge(Util::scene()->addNode(new SceneNode(m_pointStart)), Util::scene()->addNode(new SceneNode(m_pointEnd)), edgeMarker, m_angle));
}

void SceneEdgeCommandRemove::redo()
{
    Util::scene()->removeEdge(Util::scene()->getEdge(m_pointStart, m_pointEnd));
}

SceneEdgeCommandEdit::SceneEdgeCommandEdit(const Point &pointStart, const Point &pointEnd, const Point &pointStartNew, const Point &pointEndNew, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_pointStartNew = pointStartNew;
    m_pointEndNew = pointEndNew;
}

void SceneEdgeCommandEdit::undo()
{
    SceneEdge *edge = Util::scene()->getEdge(m_pointStartNew, m_pointEndNew);
    if (edge)
    {
        edge->nodeStart = Util::scene()->getNode(m_pointStart);
        edge->nodeEnd = Util::scene()->getNode(m_pointEnd);
        Util::scene()->refresh();
    }
}

void SceneEdgeCommandEdit::redo()
{
    SceneEdge *edge = Util::scene()->getEdge(m_pointStart, m_pointEnd);
    if (edge)
    {
        edge->nodeStart = Util::scene()->getNode(m_pointStartNew);
        edge->nodeEnd = Util::scene()->getNode(m_pointEndNew);
        Util::scene()->refresh();
    }
}
