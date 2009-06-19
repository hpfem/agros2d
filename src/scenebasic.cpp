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

int SceneNode::showDialog(Scene *scene, QWidget *parent)
{
    DSceneNode *dialog = new DSceneNode(scene, this, parent);
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
        double z = (t.angle() - (nodeStart->point - c).angle())/180*M_PI;
        if ((z > 0) && (z < angle)) return l;

        double a = (point - nodeStart->point).magnitude();
        double b = (point - nodeEnd->point).magnitude();
        if (a<b)
            return a;
        else
            return b;
    }
}

int SceneEdge::showDialog(Scene *scene, QWidget *parent)
{
    DSceneEdge *dialog = new DSceneEdge(scene, this, parent);
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

int SceneLabel::showDialog(Scene *scene, QWidget *parent)
{
    DSceneLabel *dialog = new DSceneLabel(scene, this, parent);
    return dialog->exec();
}

// *************************************************************************************************************************************

DSceneBasic::DSceneBasic(Scene *scene, QWidget *parent) : QDialog(parent)
{
    this->m_scene = scene;
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
    save();

    accept();
}

void DSceneBasic::doReject()
{
    reject();
}

// *************************************************************************************************************************************

DSceneNode::DSceneNode(Scene *scene, SceneNode *node, QWidget *parent) : DSceneBasic(scene, parent)
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
    layout->addRow(tr("x (m):"), txtPointX);
    layout->addRow(tr("y (m):"), txtPointY);

    return layout;
}

void DSceneNode::load()
{
    SceneNode *sceneNode = dynamic_cast<SceneNode *>(m_object);

    txtPointX->setText(QString::number(sceneNode->point.x));
    txtPointY->setText(QString::number(sceneNode->point.y));
}

void DSceneNode::save()
{
    SceneNode *sceneNode = dynamic_cast<SceneNode *>(m_object);

    Point point(txtPointX->value(), txtPointY->value());
    sceneNode->point = point;
}

// *************************************************************************************************************************************

DSceneEdge::DSceneEdge(Scene *scene, SceneEdge *edge, QWidget *parent) : DSceneBasic(scene, parent)
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
    layout->addRow(tr("Angle (deg):"), txtAngle);

    fillComboBox();

    return layout;
}

void DSceneEdge::fillComboBox()
{
    // start and end nodes
    cmbNodeStart->clear();
    cmbNodeEnd->clear();
    for (int i = 0; i<m_scene->nodes.count(); i++)
    {
        cmbNodeStart->addItem(QString("[%1; %2]").arg(m_scene->nodes[i]->point.x, 0, 'f', 3).arg(m_scene->nodes[i]->point.y, 0, 'f', 3),
                              m_scene->nodes[i]->variant());
        cmbNodeEnd->addItem(QString("[%1; %2]").arg(m_scene->nodes[i]->point.x, 0, 'f', 3).arg(m_scene->nodes[i]->point.y, 0, 'f', 3),
                            m_scene->nodes[i]->variant());
    }

    // markers
    cmbMarker->clear();
    for (int i = 0; i<m_scene->edgeMarkers.count(); i++)
    {
        cmbMarker->addItem(m_scene->edgeMarkers[i]->name, m_scene->edgeMarkers[i]->variant());
    }
}

void DSceneEdge::load() {
    SceneEdge *sceneEdge = dynamic_cast<SceneEdge *>(m_object);

    cmbNodeStart->setCurrentIndex(cmbNodeStart->findData(sceneEdge->nodeStart->variant()));
    cmbNodeEnd->setCurrentIndex(cmbNodeEnd->findData(sceneEdge->nodeEnd->variant()));
    cmbMarker->setCurrentIndex(cmbMarker->findData(sceneEdge->marker->variant()));
    txtAngle->setText(QString::number(sceneEdge->angle));
}

void DSceneEdge::save() {
    SceneEdge *sceneEdge = dynamic_cast<SceneEdge *>(m_object);

    sceneEdge->nodeStart = dynamic_cast<SceneNode *>(cmbNodeStart->itemData(cmbNodeStart->currentIndex()).value<SceneBasic *>());
    sceneEdge->nodeEnd = dynamic_cast<SceneNode *>(cmbNodeEnd->itemData(cmbNodeEnd->currentIndex()).value<SceneBasic *>());
    sceneEdge->marker = cmbMarker->itemData(cmbMarker->currentIndex()).value<SceneEdgeMarker *>();
    sceneEdge->angle = txtAngle->value();
}

// *************************************************************************************************************************************

DSceneLabel::DSceneLabel(Scene *scene, SceneLabel *label, QWidget *parent) : DSceneBasic(scene, parent) {
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
    layout->addRow(tr("x (m):"), txtPointX);
    layout->addRow(tr("y (m):"), txtPointY);
    layout->addRow(tr("Material:"), cmbMarker);
    layout->addRow(tr("Triangle area (m):"), txtArea);

    fillComboBox();

    return layout;
}

void DSceneLabel::fillComboBox()
{
    // markers
    cmbMarker->clear();
    for (int i = 0; i<m_scene->labelMarkers.count(); i++)
    {
        cmbMarker->addItem(m_scene->labelMarkers[i]->name, m_scene->labelMarkers[i]->variant());
    }
}

void DSceneLabel::load()
{
    SceneLabel *sceneLabel = dynamic_cast<SceneLabel *>(m_object);

    txtPointX->setText(QString::number(sceneLabel->point.x));
    txtPointY->setText(QString::number(sceneLabel->point.y));
    cmbMarker->setCurrentIndex(cmbMarker->findData(sceneLabel->marker->variant()));
    txtArea->setText(QString::number(sceneLabel->area));
}

void DSceneLabel::save()
{
    SceneLabel *sceneLabel = dynamic_cast<SceneLabel *>(m_object);

    Point point(txtPointX->value(), txtPointY->value());
    sceneLabel->point = point;
    sceneLabel->marker = cmbMarker->itemData(cmbMarker->currentIndex()).value<SceneLabelMarker *>();
    sceneLabel->area = txtArea->value();
}
