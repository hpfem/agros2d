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

#include "sceneedge.h"

#include "util.h"
#include "util/global.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"


#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/field.h"

SceneEdge::SceneEdge(SceneNode *nodeStart, SceneNode *nodeEnd, double angle)
    : MarkedSceneBasic<SceneBoundary>(),
      m_nodeStart(nodeStart), m_nodeEnd(nodeEnd), m_angle(angle)
{
    foreach (FieldInfo* field, Agros2D::problem()->fieldInfos())
    {
        this->addMarker(SceneBoundaryContainer::getNone(field));
    }

    // cache center;
    computeCenter();
}

void SceneEdge::swapDirection()
{
    SceneNode *tmp = m_nodeStart;

    m_nodeStart = m_nodeEnd;
    m_nodeEnd = tmp;

    // cache center;
    computeCenter();
}

Point SceneEdge::center() const
{
    return m_centerCache;
}

double SceneEdge::radius() const
{
    return (center() - m_nodeStart->point()).magnitude();
}

double SceneEdge::distance(const Point &point) const
{
    if (isStraight())
    {
        double t = ((point.x-m_nodeStart->point().x)*(m_nodeEnd->point().x-m_nodeStart->point().x) + (point.y-m_nodeStart->point().y)*(m_nodeEnd->point().y-m_nodeStart->point().y)) /
                ((m_nodeEnd->point().x-m_nodeStart->point().x)*(m_nodeEnd->point().x-m_nodeStart->point().x) + (m_nodeEnd->point().y-m_nodeStart->point().y)*(m_nodeEnd->point().y-m_nodeStart->point().y));

        if (t > 1.0) t = 1.0;
        if (t < 0.0) t = 0.0;

        double x = m_nodeStart->point().x + t*(m_nodeEnd->point().x-m_nodeStart->point().x);
        double y = m_nodeStart->point().y + t*(m_nodeEnd->point().y-m_nodeStart->point().y);

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

        double z = (t.angle() - (m_nodeStart->point() - c).angle())/M_PI*180.0;
        if (z < 0) z = z + 360.0; // interval (0, 360)
        if ((z > 0) && (z < m_angle)) return l;

        double a = (point - m_nodeStart->point()).magnitude();
        double b = (point - m_nodeEnd->point()).magnitude();

        return qMin(a, b);
    }
}

QList<SceneEdge *> SceneEdge::crossedEdges() const
{
    QList<SceneEdge *> edges;

    foreach (SceneEdge *edgeCheck, Agros2D::scene()->edges->items())
    {
        if (edgeCheck != this)
        {
            QList<Point> intersects;

            // TODO: improve - add check of crossings of two arcs
            if (isStraight())
                intersects = intersection(m_nodeStart->point(), m_nodeEnd->point(),
                                          edgeCheck->center(), radius(), angle(),
                                          edgeCheck->nodeStart()->point(), edgeCheck->nodeEnd()->point(),
                                          edgeCheck->center(), edgeCheck->radius(), edgeCheck->angle());

            else
                intersects = intersection(edgeCheck->nodeStart()->point(), edgeCheck->nodeEnd()->point(),
                                          edgeCheck->center(), edgeCheck->radius(), edgeCheck->angle(),
                                          m_nodeStart->point(), m_nodeEnd->point(),
                                          center(), radius(), angle());

            if (intersects.count() > 0)
                edges.append(edgeCheck);
        }
    }

    return edges;
}

QList<SceneNode *> SceneEdge::lyingNodes() const
{
    QList<SceneNode *> nodes;

    foreach (SceneNode *node, Agros2D::scene()->nodes->items())
    {
        if ((this->nodeStart() == node) || (this->nodeEnd() == node))
            continue;

        if (this->distance(node->point()) < EPS_ZERO)
            nodes.append(node);
    }

    return nodes;
}

int SceneEdge::segments() const
{
    double division = 40.0;
    int segments = m_angle/division + 1;
    if (segments < Agros2D::problem()->configView()->angleSegmentsCount)
        segments = Agros2D::problem()->configView()->angleSegmentsCount; // minimum segments

    return segments;
}

double SceneEdge::length() const
{
    if (isStraight())
        return (m_nodeEnd->point() - m_nodeStart->point()).magnitude();
    else
        return radius() * m_angle / 180.0 * M_PI;
}

bool SceneEdge::isOutsideArea() const
{
    return (m_nodeStart->isOutsideArea() || m_nodeEnd->isOutsideArea());
}

bool SceneEdge::isError() const
{
    return (this->isLyingNode() || this->isOutsideArea() || isCrossed());
}

int SceneEdge::showDialog(QWidget *parent, bool isNew)
{
    SceneEdgeDialog *dialog = new SceneEdgeDialog(this, parent, isNew);
    return dialog->exec();
}

SceneEdgeCommandAdd* SceneEdge::getAddCommand()
{
    return new SceneEdgeCommandAdd(m_nodeStart->point(), m_nodeEnd->point(), markersKeys(), m_angle);
}

SceneEdgeCommandRemove* SceneEdge::getRemoveCommand()
{
    return new SceneEdgeCommandRemove(m_nodeStart->point(), m_nodeEnd->point(), markersKeys(), m_angle);
}

SceneEdge *SceneEdge::findClosestEdge(const Point &point)
{
    SceneEdge *edgeClosest = NULL;

    double distance = numeric_limits<double>::max();
    foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
    {
        double edgeDistance = edge->distance(point);
        if (edge->distance(point) < distance)
        {
            distance = edgeDistance;
            edgeClosest = edge;
        }
    }

    return edgeClosest;
}

void SceneEdge::computeCenter()
{
    if (fabs(m_angle) > EPS_ZERO)
        m_centerCache = centerPoint(m_nodeStart->point(), m_nodeEnd->point(), m_angle);
    else
        m_centerCache = Point();
}

//************************************************************************************************

void SceneEdgeContainer::removeConnectedToNode(SceneNode *node)
{
    foreach (SceneEdge *edge, data)
    {
        if ((edge->nodeStart() == node) || (edge->nodeEnd() == node))
        {
            Agros2D::scene()->undoStack()->push(new SceneEdgeCommandRemove(edge->nodeStart()->point(),
                                                                           edge->nodeEnd()->point(),
                                                                           edge->markersKeys(),
                                                                           edge->angle()));
            remove(edge);
        }
    }

}

SceneEdge* SceneEdgeContainer::get(SceneEdge* edge) const
{
    foreach (SceneEdge *edgeCheck, data)
    {
        if (((((edgeCheck->nodeStart() == edge->nodeStart()) && (edgeCheck->nodeEnd() == edge->nodeEnd())) &&
              (fabs(edgeCheck->angle() - edge->angle()) < EPS_ZERO)) ||
             (((edgeCheck->nodeStart() == edge->nodeEnd()) && (edgeCheck->nodeEnd() == edge->nodeStart()))) &&
             (fabs(edgeCheck->angle() + edge->angle()) < EPS_ZERO)))
        {
            return edgeCheck;
        }
    }

    return NULL;
}

SceneEdge* SceneEdgeContainer::get(const Point &pointStart, const Point &pointEnd, double angle) const
{
    foreach (SceneEdge *edgeCheck, data)
    {
        if (((edgeCheck->nodeStart()->point() == pointStart) && (edgeCheck->nodeEnd()->point() == pointEnd)) && (edgeCheck->angle() == angle))
            return edgeCheck;
    }

    return NULL;
}

RectPoint SceneEdgeContainer::boundingBox() const
{
    return SceneEdgeContainer::boundingBox(data);
}

RectPoint SceneEdgeContainer::boundingBox(QList<SceneEdge *> edges)
{
    Point min( numeric_limits<double>::max(),  numeric_limits<double>::max());
    Point max(-numeric_limits<double>::max(), -numeric_limits<double>::max());

    foreach (SceneEdge *edge, edges)
    {
        // start and end node
        min.x = qMin(min.x, qMin(edge->nodeStart()->point().x, edge->nodeEnd()->point().x));
        max.x = qMax(max.x, qMax(edge->nodeStart()->point().x, edge->nodeEnd()->point().x));
        min.y = qMin(min.y, qMin(edge->nodeStart()->point().y, edge->nodeEnd()->point().y));
        max.y = qMax(max.y, qMax(edge->nodeStart()->point().y, edge->nodeEnd()->point().y));

        if (!edge->isStraight())
        {
            int segments = 4;
            double theta = deg2rad(edge->angle()) / double(segments);
            Point center = edge->center();
            double radius = edge->radius();

            double startAngle = atan2(center.y - edge->nodeStart()->point().y,
                                      center.x - edge->nodeStart()->point().x) - M_PI;

            for (int i = 1; i < segments; i++)
            {
                double arc = startAngle + i*theta;

                double x = center.x + radius * cos(arc);
                double y = center.y + radius * sin(arc);

                min.x = qMin(min.x, x);
                max.x = qMax(max.x, x);
                min.y = qMin(min.y, y);
                max.y = qMax(max.y, y);
            }
        }
    }

    return RectPoint(min, max);
}

// *************************************************************************************************************************************

SceneEdgeMarker::SceneEdgeMarker(SceneEdge *edge, FieldInfo *fieldInfo, QWidget *parent)
    : QGroupBox(parent), m_fieldInfo(fieldInfo), m_edge(edge)

{
    setTitle(fieldInfo->name());

    cmbBoundary = new QComboBox();
    connect(cmbBoundary, SIGNAL(currentIndexChanged(int)), this, SLOT(doBoundaryChanged(int)));

    btnBoundary = new QPushButton(icon("three-dots"), "");
    btnBoundary->setMaximumSize(btnBoundary->sizeHint());
    connect(btnBoundary, SIGNAL(clicked()), this, SLOT(doBoundaryClicked()));

    QHBoxLayout *layoutBoundary = new QHBoxLayout();
    layoutBoundary->addWidget(cmbBoundary, 1);
    layoutBoundary->addWidget(btnBoundary);

    // refine towards edge
    chkRefineTowardsEdge = new QCheckBox();
    connect(chkRefineTowardsEdge, SIGNAL(stateChanged(int)), this, SLOT(doRefineTowardsEdge(int)));

    txtRefineTowardsEdge = new QSpinBox(this);
    txtRefineTowardsEdge->setMinimum(0);
    txtRefineTowardsEdge->setMaximum(10);

    QHBoxLayout *layoutRefineTowardsEdge = new QHBoxLayout();
    layoutRefineTowardsEdge->addWidget(chkRefineTowardsEdge);
    layoutRefineTowardsEdge->addWidget(txtRefineTowardsEdge);
    layoutRefineTowardsEdge->addStretch();

    QFormLayout *layoutBoundaries = new QFormLayout();
    layoutBoundaries->addRow(tr("Boundary condition:"), layoutBoundary);
    layoutBoundaries->addRow(tr("Refine towards edge:"), layoutRefineTowardsEdge);

    setLayout(layoutBoundaries);
}

void SceneEdgeMarker::load()
{
    if (m_edge->hasMarker(m_fieldInfo))
        cmbBoundary->setCurrentIndex(cmbBoundary->findData(m_edge->marker(m_fieldInfo)->variant()));

    // edge refinement
    int refinement = m_fieldInfo->edgeRefinement(m_edge);
    chkRefineTowardsEdge->setChecked(refinement > 0);
    txtRefineTowardsEdge->setEnabled(chkRefineTowardsEdge->isChecked());
    txtRefineTowardsEdge->setValue(refinement);
}

bool SceneEdgeMarker::save()
{
    m_edge->addMarker(cmbBoundary->itemData(cmbBoundary->currentIndex()).value<SceneBoundary *>());

    // edge refinement
    if (chkRefineTowardsEdge->isChecked())
        m_fieldInfo->setEdgeRefinement(m_edge, txtRefineTowardsEdge->text().toInt());
    else
        m_fieldInfo->removeEdgeRefinement(m_edge);

    return true;
}

void SceneEdgeMarker::fillComboBox()
{
    cmbBoundary->clear();

    // none marker
    cmbBoundary->addItem(Agros2D::scene()->boundaries->getNone(m_fieldInfo)->name(),
                         Agros2D::scene()->boundaries->getNone(m_fieldInfo)->variant());

    // real markers
    foreach (SceneBoundary *boundary, Agros2D::scene()->boundaries->filter(m_fieldInfo).items())
    {
        cmbBoundary->addItem(boundary->name(),
                             boundary->variant());
    }
}

void SceneEdgeMarker::doBoundaryChanged(int index)
{
    btnBoundary->setEnabled(cmbBoundary->currentIndex() > 0);
}

void SceneEdgeMarker::doBoundaryClicked()
{
    SceneBoundary *marker = cmbBoundary->itemData(cmbBoundary->currentIndex()).value<SceneBoundary *>();
    if (marker->showDialog(this) == QDialog::Accepted)
    {
        cmbBoundary->setItemText(cmbBoundary->currentIndex(), marker->name());
        Agros2D::scene()->invalidate();
    }
}

void SceneEdgeMarker::doRefineTowardsEdge(int state)
{
    txtRefineTowardsEdge->setEnabled(chkRefineTowardsEdge->isChecked());
}

SceneEdgeDialog::SceneEdgeDialog(SceneEdge *edge, QWidget *parent, bool isNew) : SceneBasicDialog(parent, isNew)
{
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
    // layout
    QFormLayout *layout = new QFormLayout();

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        SceneEdgeMarker *sceneEdge = new SceneEdgeMarker(dynamic_cast<SceneEdge *>(m_object), fieldInfo, this);
        layout->addRow(sceneEdge);

        m_edgeMarkers.append(sceneEdge);
    }

    txtAngle = new ValueLineEdit();
    txtAngle->setMinimum(0.0);
    txtAngle->setMaximum(180.0);
    connect(txtAngle, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    // connect(txtAngle, SIGNAL(evaluated(bool)), this, SLOT(doAngleChanged()));
    lblLength = new QLabel();

    // coordinates
    QFormLayout *layoutEdgeParameters = new QFormLayout();
    layoutEdgeParameters->addRow(tr("Angle (deg.):"), txtAngle);
    layoutEdgeParameters->addRow(tr("Length:"), lblLength);

    QGroupBox *grpEdgeParameters = new QGroupBox(tr("Edge parameters"));
    grpEdgeParameters->setLayout(layoutEdgeParameters);

    layout->addRow(grpEdgeParameters);

    fillComboBox();

    return layout;
}

void SceneEdgeDialog::fillComboBox()
{
    // markers
    foreach (SceneEdgeMarker *edgeMarker, m_edgeMarkers)
        edgeMarker->fillComboBox();
}

bool SceneEdgeDialog::load()
{
    SceneEdge *sceneEdge = dynamic_cast<SceneEdge *>(m_object);

    txtAngle->setNumber(sceneEdge->angle());

    foreach (SceneEdgeMarker *edgeMarker, m_edgeMarkers)
        edgeMarker->load();

    doAngleChanged();

    return true;
}

bool SceneEdgeDialog::save()
{
    if (!txtAngle->evaluate(false)) return false;

    SceneEdge *sceneEdge = dynamic_cast<SceneEdge *>(m_object);

    if (!m_isNew)
    {
        Agros2D::scene()->undoStack()->push(new SceneEdgeCommandEdit(sceneEdge->nodeStart()->point(), sceneEdge->nodeEnd()->point(),
                                                                     sceneEdge->nodeStart()->point(), sceneEdge->nodeEnd()->point(),
                                                                     sceneEdge->angle(), txtAngle->number()));
    }

    sceneEdge->setAngle(txtAngle->number());

    foreach (SceneEdgeMarker *edgeMarker, m_edgeMarkers)
        edgeMarker->save();

    Agros2D::scene()->invalidate();
    return true;
}

void SceneEdgeDialog::doAngleChanged()
{
    SceneEdge *sceneEdge = dynamic_cast<SceneEdge *>(m_object);

    if (txtAngle->number() < EPS_ZERO)
    {
        // line
        lblLength->setText(QString("%1 m").arg(sqrt(Hermes::sqr(sceneEdge->nodeEnd()->point().x - sceneEdge->nodeStart()->point().x) + Hermes::sqr(sceneEdge->nodeEnd()->point().y - sceneEdge->nodeStart()->point().y))));
    }
    else
    {
        // arc
        SceneEdge edge(sceneEdge->nodeStart(), sceneEdge->nodeEnd(), txtAngle->number());
        lblLength->setText(QString("%1 m").arg(edge.radius() * edge.angle() / 180.0 * M_PI));
    }
}

SceneEdgeSelectDialog::SceneEdgeSelectDialog(MarkedSceneBasicContainer<SceneBoundary, SceneEdge> edges, QWidget *parent)
    : QDialog(parent), m_edges(edges)
{
    setWindowIcon(icon("scene-edge"));
    setWindowTitle(tr("Edges"));

    // markers
    QFormLayout *layoutBoundaries = new QFormLayout();

    QGroupBox *grpBoundaries = new QGroupBox(tr("Boundary conditions"));
    grpBoundaries->setLayout(layoutBoundaries);

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        QComboBox *cmbBoundary = new QComboBox();
        cmbBoundaries[fieldInfo] = cmbBoundary;

        layoutBoundaries->addRow(fieldInfo->name(), cmbBoundary);
    }

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpBoundaries);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    load();

    setMinimumSize(sizeHint());
}

void SceneEdgeSelectDialog::load()
{
    // markers
    foreach (FieldInfo *fieldInfo, cmbBoundaries.keys())
    {
        cmbBoundaries[fieldInfo]->clear();

        // none marker
        cmbBoundaries[fieldInfo]->addItem(Agros2D::scene()->boundaries->getNone(fieldInfo)->name(),
                                          Agros2D::scene()->boundaries->getNone(fieldInfo)->variant());

        // real markers
        foreach (SceneBoundary *boundary, Agros2D::scene()->boundaries->filter(fieldInfo).items())
            cmbBoundaries[fieldInfo]->addItem(boundary->name(),
                                              boundary->variant());
    }

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        SceneBoundary* boundary = NULL;
        bool match = true;
        foreach (SceneEdge *edge, m_edges.items())
        {
            if (boundary)
                match = match && (boundary == edge->marker(fieldInfo));
            else
                boundary = edge->marker(fieldInfo);
        }
        if (match)
            cmbBoundaries[fieldInfo]->setCurrentIndex(cmbBoundaries[fieldInfo]->findData(boundary->variant()));
        else
            cmbBoundaries[fieldInfo]->setCurrentIndex(-1);
    }
}

bool SceneEdgeSelectDialog::save()
{
    foreach (SceneEdge* edge, m_edges.items())
    {
        foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
        {
            if (cmbBoundaries[fieldInfo]->currentIndex() != -1)
                edge->addMarker(cmbBoundaries[fieldInfo]->itemData(cmbBoundaries[fieldInfo]->currentIndex()).value<SceneBoundary *>());

        }
    }

    Agros2D::scene()->invalidate();
    return true;
}

void SceneEdgeSelectDialog::doAccept()
{
    if (save())
        accept();
}

void SceneEdgeSelectDialog::doReject()
{
    reject();
}

// undo framework *******************************************************************************************************************
// **********************************************************************************************************************************

SceneEdgeCommandAdd::SceneEdgeCommandAdd(const Point &pointStart, const Point &pointEnd, const QMap<QString, QString> &markers,
                                         double angle, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_markers = markers;
    m_angle = angle;
}

void SceneEdgeCommandAdd::undo()
{
    Agros2D::scene()->edges->remove(Agros2D::scene()->getEdge(m_pointStart, m_pointEnd, m_angle));
    Agros2D::scene()->invalidate();
}

void SceneEdgeCommandAdd::redo()
{
    // new edge
    SceneEdge *edge = new SceneEdge(Agros2D::scene()->addNode(new SceneNode(m_pointStart)),
                                    Agros2D::scene()->addNode(new SceneNode(m_pointEnd)),
                                    m_angle);

    foreach (QString fieldId, m_markers.keys())
    {
        if (Agros2D::problem()->hasField(fieldId))
        {
            SceneBoundary *boundary = Agros2D::scene()->boundaries->filter(Agros2D::problem()->fieldInfo(fieldId)).get(m_markers[fieldId]);

            if (!boundary)
                boundary = Agros2D::scene()->boundaries->getNone(Agros2D::problem()->fieldInfo(fieldId));

            // add marker
            edge->addMarker(boundary);
        }
    }

    // add edge to the list
    Agros2D::scene()->addEdge(edge);
    Agros2D::scene()->invalidate();
}

SceneEdgeCommandRemove::SceneEdgeCommandRemove(const Point &pointStart, const Point &pointEnd, const QMap<QString, QString> &markers,
                                               double angle, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_markers = markers;
    m_angle = angle;
}

void SceneEdgeCommandRemove::undo()
{
    // new edge
    SceneEdge *edge = new SceneEdge(Agros2D::scene()->addNode(new SceneNode(m_pointStart)),
                                    Agros2D::scene()->addNode(new SceneNode(m_pointEnd)),
                                    m_angle);

    foreach (QString fieldId, m_markers.keys())
    {
        if (Agros2D::problem()->hasField(fieldId))
        {
            SceneBoundary *boundary = Agros2D::scene()->boundaries->filter(Agros2D::problem()->fieldInfo(fieldId)).get(m_markers[fieldId]);

            if (!boundary)
                boundary = Agros2D::scene()->boundaries->getNone(Agros2D::problem()->fieldInfo(fieldId));

            // add marker
            edge->addMarker(boundary);
        }
    }

    // add edge to the list
    Agros2D::scene()->addEdge(edge);
    Agros2D::scene()->invalidate();
}

void SceneEdgeCommandRemove::redo()
{
    Agros2D::scene()->edges->remove(Agros2D::scene()->getEdge(m_pointStart, m_pointEnd, m_angle));
    Agros2D::scene()->invalidate();
}

SceneEdgeCommandEdit::SceneEdgeCommandEdit(const Point &pointStart, const Point &pointEnd, const Point &pointStartNew, const Point &pointEndNew,
                                           double angle, double angleNew, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_pointStartNew = pointStartNew;
    m_pointEndNew = pointEndNew;
    m_angle = angle;
    m_angleNew = angleNew;
}

void SceneEdgeCommandEdit::undo()
{
    SceneEdge *edge = Agros2D::scene()->getEdge(m_pointStartNew, m_pointEndNew, m_angleNew);
    if (edge)
    {
        edge->setNodeStart(Agros2D::scene()->getNode(m_pointStart));
        edge->setNodeEnd(Agros2D::scene()->getNode(m_pointEnd));
        edge->setAngle(m_angle);
        Agros2D::scene()->invalidate();
    }
}

void SceneEdgeCommandEdit::redo()
{
    SceneEdge *edge = Agros2D::scene()->getEdge(m_pointStart, m_pointEnd, m_angle);
    if (edge)
    {
        edge->setNodeStart(Agros2D::scene()->getNode(m_pointStartNew));
        edge->setNodeEnd(Agros2D::scene()->getNode(m_pointEndNew));
        edge->setAngle(m_angleNew);
        Agros2D::scene()->invalidate();
    }
}

