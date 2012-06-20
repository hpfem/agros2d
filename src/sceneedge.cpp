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
#include "scenenode.h"
#include "sceneedge.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"
#include "hermes2d/field.h"

SceneEdge::SceneEdge(SceneNode *nodeStart, SceneNode *nodeEnd, double angle)
    : MarkedSceneBasic(),
      m_nodeStart(nodeStart), m_nodeEnd(nodeEnd), m_angle(angle)
{
    foreach (FieldInfo* field, Util::problem()->fieldInfos())
    {
        this->addMarker(SceneBoundaryContainer::getNone(field));
    }
}

Point SceneEdge::center() const
{
    return centerPoint(m_nodeStart->point(), m_nodeEnd->point(), m_angle);
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

int SceneEdge::segments() const
{
    double division = 40.0;
    int segments = m_angle/division + 1;
    if (segments < Util::config()->angleSegmentsCount)
        segments = Util::config()->angleSegmentsCount; // minimum segments

    return segments;
}

double SceneEdge::length() const
{
    if (isStraight())
        return (m_nodeEnd->point() - m_nodeStart->point()).magnitude();
    else
        return radius() * m_angle / 180.0 * M_PI;
}

int SceneEdge::showDialog(QWidget *parent, bool isNew)
{
    SceneEdgeDialog *dialog = new SceneEdgeDialog(this, parent, isNew);
    return dialog->exec();
}

SceneEdgeCommandRemove* SceneEdge::getRemoveCommand()
{
    // TODO: undo
    return new SceneEdgeCommandRemove(m_nodeStart->point(), m_nodeEnd->point(), "TODO", m_angle);
}

//************************************************************************************************

void SceneEdgeContainer::removeConnectedToNode(SceneNode *node)
{
    foreach (SceneEdge *edge, data)
    {
        if ((edge->nodeStart() == node) || (edge->nodeEnd() == node))
        {
            // TODO: undo
            Util::scene()->undoStack()->push(new SceneEdgeCommandRemove(edge->nodeStart()->point(), edge->nodeEnd()->point(), "TODO",
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
    txtRefineTowardsEdge = new QSpinBox(this);
    txtRefineTowardsEdge->setMinimum(0);
    txtRefineTowardsEdge->setMaximum(10);

    chkRefineTowardsEdge = new QCheckBox();
    connect(chkRefineTowardsEdge, SIGNAL(stateChanged(int)), this, SLOT(doRefineTowardsEdge(int)));

    QHBoxLayout *layoutRefineTowardsEdge = new QHBoxLayout();
    layoutRefineTowardsEdge->addStretch(1);
    layoutRefineTowardsEdge->addWidget(chkRefineTowardsEdge);
    layoutRefineTowardsEdge->addWidget(txtRefineTowardsEdge);

    QFormLayout *layoutBoundaries = new QFormLayout();
    layoutBoundaries->addRow(tr("Boundary condition:"), layoutBoundary);
    layoutBoundaries->addRow(tr("Refine towards edge:"), layoutRefineTowardsEdge);

    setLayout(layoutBoundaries);
}

void SceneEdgeMarker::load()
{
    cmbBoundary->setCurrentIndex(cmbBoundary->findData(m_edge->marker(m_fieldInfo)->variant()));

    // TODO: load refine
    // chkRefineTowardsEdge->setChecked(m_edge->refineTowardsEdge > 0.0);
    txtRefineTowardsEdge->setEnabled(chkRefineTowardsEdge->isChecked());
    // txtRefineTowardsEdge->setValue(m_edge->refineTowardsEdge);
}

bool SceneEdgeMarker::save()
{
    m_edge->addMarker(cmbBoundary->itemData(cmbBoundary->currentIndex()).value<SceneBoundary *>());

    // TODO: save refine
    // m_edge->refineTowardsEdge = chkRefineTowardsEdge->isChecked() ? txtRefineTowardsEdge->value() : 0;

    return true;
}

void SceneEdgeMarker::fillComboBox()
{
    cmbBoundary->clear();

    // none marker
    cmbBoundary->addItem(Util::scene()->boundaries->getNone(m_fieldInfo)->getName(),
                         Util::scene()->boundaries->getNone(m_fieldInfo)->variant());

    // real markers
    foreach (SceneBoundary *boundary, Util::scene()->boundaries->filter(m_fieldInfo).items())
    {
        cmbBoundary->addItem(boundary->getName(),
                             boundary->variant());
    }
}

void SceneEdgeMarker::doBoundaryChanged(int index)
{
    btnBoundary->setEnabled(cmbBoundary->currentIndex() > 0);
    doRefineTowardsEdge(0);
}

void SceneEdgeMarker::doBoundaryClicked()
{
    SceneBoundary *marker = cmbBoundary->itemData(cmbBoundary->currentIndex()).value<SceneBoundary *>();
    if (marker->showDialog(this) == QDialog::Accepted)
    {
        cmbBoundary->setItemText(cmbBoundary->currentIndex(), marker->getName());
        Util::scene()->refresh();
    }
}

void SceneEdgeMarker::doRefineTowardsEdge(int state)
{
    chkRefineTowardsEdge->setEnabled(cmbBoundary->currentIndex() > 0);
    txtRefineTowardsEdge->setEnabled(chkRefineTowardsEdge->isChecked() && cmbBoundary->currentIndex() > 0);
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

    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
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
        // TODO: swap
        Util::scene()->undoStack()->push(new SceneEdgeCommandEdit(sceneEdge->nodeStart()->point(), sceneEdge->nodeEnd()->point(),
                                                                  sceneEdge->nodeStart()->point(), sceneEdge->nodeEnd()->point(),
                                                                  sceneEdge->angle(), txtAngle->number()));
    }

    sceneEdge->setAngle(txtAngle->number());

    foreach (SceneEdgeMarker *edgeMarker, m_edgeMarkers)
        edgeMarker->save();

    Util::scene()->checkEdge(sceneEdge);

    Util::scene()->refresh();
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

    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
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
        cmbBoundaries[fieldInfo]->addItem(Util::scene()->boundaries->getNone(fieldInfo)->getName(),
                                          Util::scene()->boundaries->getNone(fieldInfo)->variant());

        // real markers
        foreach (SceneBoundary *boundary, Util::scene()->boundaries->filter(fieldInfo).items())
            cmbBoundaries[fieldInfo]->addItem(boundary->getName(),
                                              boundary->variant());
    }

    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
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
        foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
        {
            if (cmbBoundaries[fieldInfo]->currentIndex() != -1)
                edge->addMarker(cmbBoundaries[fieldInfo]->itemData(cmbBoundaries[fieldInfo]->currentIndex()).value<SceneBoundary *>());

        }
    }

    Util::scene()->refresh();
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

SceneEdgeCommandAdd::SceneEdgeCommandAdd(const Point &pointStart, const Point &pointEnd, const QString &markerName,
                                         double angle, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_markerName = markerName;
    m_angle = angle;
}

void SceneEdgeCommandAdd::undo()
{
    Util::scene()->edges->remove(Util::scene()->getEdge(m_pointStart, m_pointEnd, m_angle));
}

void SceneEdgeCommandAdd::redo()
{
    //TODO
    SceneBoundary *boundary = Util::scene()->getBoundary(m_markerName);
    if (boundary == NULL) boundary = Util::scene()->boundaries->get("none"); //TODO - do it better
    //TODO

    Util::scene()->addEdge(new SceneEdge(Util::scene()->addNode(new SceneNode(m_pointStart)),
                                         Util::scene()->addNode(new SceneNode(m_pointEnd)),
                                         m_angle));
}

SceneEdgeCommandRemove::SceneEdgeCommandRemove(const Point &pointStart, const Point &pointEnd, const QString &markerName,
                                               double angle, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_markerName = markerName;
    m_angle = angle;
}

void SceneEdgeCommandRemove::undo()
{
    assert(0); //TODO
    //    SceneBoundary *boundary = Util::scene()->getBoundary(m_markerName);
    //    if (boundary == NULL) boundary = Util::scene()->boundaries->get("none"); //TODO - do it better
    //    Util::scene()->addEdge(new SceneEdge(Util::scene()->addNode(new SceneNode(m_pointStart)),
    //                                         Util::scene()->addNode(new SceneNode(m_pointEnd)),
    //                                         boundary,
    //                                         m_angle));
}

void SceneEdgeCommandRemove::redo()
{
    Util::scene()->edges->remove(Util::scene()->getEdge(m_pointStart, m_pointEnd, m_angle));
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
    SceneEdge *edge = Util::scene()->getEdge(m_pointStartNew, m_pointEndNew, m_angleNew);
    if (edge)
    {
        edge->setNodeStart(Util::scene()->getNode(m_pointStart));
        edge->setNodeEnd(Util::scene()->getNode(m_pointEnd));
        edge->setAngle(m_angle);
        Util::scene()->refresh();
    }
}

void SceneEdgeCommandEdit::redo()
{
    SceneEdge *edge = Util::scene()->getEdge(m_pointStart, m_pointEnd, m_angle);
    if (edge)
    {
        edge->setNodeStart(Util::scene()->getNode(m_pointStartNew));
        edge->setNodeEnd(Util::scene()->getNode(m_pointEndNew));
        edge->setAngle(m_angleNew);
        Util::scene()->refresh();
    }
}

bool SceneEdge::isOutsideArea() const
{
    return (m_nodeStart->isOutsideArea() || m_nodeEnd->isOutsideArea());
}

bool SceneEdge::isError() const
{
    return (this->isLyingNode() || this->isOutsideArea() || isCrossed());
}
