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

SceneEdge::SceneEdge(SceneNode *nodeStart, SceneNode *nodeEnd, double angle, int refineTowardsEdge)
    : MarkedSceneBasic()
{
    this->nodeStart = nodeStart;
    this->nodeEnd = nodeEnd;
    this->angle = angle;
    this->refineTowardsEdge = refineTowardsEdge;

    foreach (FieldInfo* field, Util::problem()->fieldInfos())
    {
        this->addMarker(SceneBoundaryContainer::getNone(field));
    }
}

Point SceneEdge::center() const
{
    return centerPoint(nodeStart->point, nodeEnd->point, angle);
}

double SceneEdge::radius() const
{
    return (center() - nodeStart->point).magnitude();
}

double SceneEdge::distance(const Point &point) const
{
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
    SceneEdgeDialog *dialog = new SceneEdgeDialog(this, parent, isNew);
    return dialog->exec();
}

SceneEdgeCommandRemove* SceneEdge::getRemoveCommand()
{
    return new SceneEdgeCommandRemove(nodeStart->point, nodeEnd->point, "TODO", angle, refineTowardsEdge);
}

//************************************************************************************************

void SceneEdgeContainer::removeConnectedToNode(SceneNode *node)
{
    foreach (SceneEdge *edge, data)
    {
        if ((edge->nodeStart == node) || (edge->nodeEnd == node))
        {
            Util::scene()->undoStack()->push(new SceneEdgeCommandRemove(edge->nodeStart->point, edge->nodeEnd->point, "TODO",
                                                                        edge->angle, edge->refineTowardsEdge));
            remove(edge);
        }
    }

}

SceneEdge* SceneEdgeContainer::get(SceneEdge* edge) const
{
    foreach (SceneEdge *edgeCheck, data)
    {
        if (((((edgeCheck->nodeStart == edge->nodeStart) && (edgeCheck->nodeEnd == edge->nodeEnd)) &&
              (fabs(edgeCheck->angle - edge->angle) < EPS_ZERO)) ||
             (((edgeCheck->nodeStart == edge->nodeEnd) && (edgeCheck->nodeEnd == edge->nodeStart))) &&
             (fabs(edgeCheck->angle + edge->angle) < EPS_ZERO)))
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
        if (((edgeCheck->nodeStart->point == pointStart) && (edgeCheck->nodeEnd->point == pointEnd)) && (edgeCheck->angle == angle))
            return edgeCheck;
    }

    return NULL;
}


// *************************************************************************************************************************************

SceneEdgeDialog::SceneEdgeDialog(SceneEdge *edge, QWidget *parent, bool isNew) : SceneBasicDialog(parent, isNew)
{
    m_object = edge;
    m_singleEdge = true;

    setWindowIcon(icon("scene-edge"));
    setWindowTitle(tr("Edge"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    // setMaximumSize(sizeHint());
}

SceneEdgeDialog::SceneEdgeDialog(MarkedSceneBasicContainer<SceneBoundary, SceneEdge> edges, QWidget *parent) : SceneBasicDialog(parent, false)
{
    m_edges = edges;
    m_object = NULL;
    m_singleEdge = false;

    setWindowIcon(icon("scene-edge"));
    setWindowTitle(tr("Edges"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    // setMaximumSize(sizeHint());

}

QLayout* SceneEdgeDialog::createContent()
{
    // markers
    QFormLayout *layoutBoundaries = new QFormLayout();

    QGroupBox *grpBoundaries = new QGroupBox(tr("Boundary conditions"));
    grpBoundaries->setLayout(layoutBoundaries);

    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
    {
        QComboBox *cmbBoundary = new QComboBox();
        connect(cmbBoundary, SIGNAL(currentIndexChanged(int)), this, SLOT(doBoundaryChanged(int)));
        cmbBoundaries[fieldInfo] = cmbBoundary;

        QPushButton *btnBoundary = new QPushButton(icon("three-dots"), "");
        btnBoundary->setMaximumSize(btnBoundary->sizeHint());
        connect(btnBoundary, SIGNAL(clicked()), this, SLOT(doBoundaryClicked()));
        btnBoundaries[fieldInfo] = btnBoundary;

        QHBoxLayout *layoutBoundary = new QHBoxLayout();
        layoutBoundary->addWidget(cmbBoundary);
        layoutBoundary->addWidget(btnBoundary);

        layoutBoundaries->addRow(QString::fromStdString(fieldInfo->module()->name),
                                 layoutBoundary);
    }

    // layout
    QFormLayout *layout = new QFormLayout();
    layout->addRow(grpBoundaries);

    if(m_singleEdge)
    {
        cmbNodeStart = new QComboBox();
        cmbNodeEnd = new QComboBox();
        connect(cmbNodeStart, SIGNAL(currentIndexChanged(int)), this, SLOT(doNodeChanged()));
        connect(cmbNodeEnd, SIGNAL(currentIndexChanged(int)), this, SLOT(doNodeChanged()));
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

        layout->addRow(grpCoordinates);
        layout->addRow(grpMeshParameters);
        layout->addRow(tr("Length:"), lblLength);
    }

    fillComboBox();

    return layout;
}

void SceneEdgeDialog::fillComboBox()
{
    if(m_singleEdge)
    {
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
    }

    // markers
    foreach (FieldInfo *fieldInfo, cmbBoundaries.keys())
    {
        cmbBoundaries[fieldInfo]->clear();

        // none marker
        cmbBoundaries[fieldInfo]->addItem(QString::fromStdString(Util::scene()->boundaries->getNone(fieldInfo)->getName()),
                                  Util::scene()->boundaries->getNone(fieldInfo)->variant());

        // real markers
        foreach (SceneBoundary *boundary, Util::scene()->boundaries->filter(fieldInfo).items())
        {
            cmbBoundaries[fieldInfo]->addItem(QString::fromStdString(boundary->getName()),
                                      boundary->variant());
        }
    }
}

bool SceneEdgeDialog::load()
{
    if(m_singleEdge)
    {
        SceneEdge *sceneEdge = dynamic_cast<SceneEdge *>(m_object);

        cmbNodeStart->setCurrentIndex(cmbNodeStart->findData(sceneEdge->nodeStart->variant()));
        cmbNodeEnd->setCurrentIndex(cmbNodeEnd->findData(sceneEdge->nodeEnd->variant()));
        txtAngle->setNumber(sceneEdge->angle);
        chkRefineTowardsEdge->setChecked(sceneEdge->refineTowardsEdge > 0.0);
        txtRefineTowardsEdge->setEnabled(chkRefineTowardsEdge->isChecked());
        txtRefineTowardsEdge->setValue(sceneEdge->refineTowardsEdge);

        foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
        {
            cmbBoundaries[fieldInfo]->setCurrentIndex(cmbBoundaries[fieldInfo]->findData(sceneEdge->getMarker(fieldInfo)->variant()));
        }

        doNodeChanged(); //TODO what does this?
    }
    else
    {
        foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
        {
            SceneBoundary* boundary = NULL;
            bool match = true;
            foreach(SceneEdge* edge, m_edges.items())
            {
                if(boundary)
                    match = match && (boundary == edge->getMarker(fieldInfo));
                else
                    boundary = edge->getMarker(fieldInfo);
            }
            if(match)
                cmbBoundaries[fieldInfo]->setCurrentIndex(cmbBoundaries[fieldInfo]->findData(boundary->variant()));
            else
                cmbBoundaries[fieldInfo]->setCurrentIndex(-1);
        }


    }
    return true;
}

bool SceneEdgeDialog::save()
{
    if(m_singleEdge)
    {
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
        sceneEdge->angle = txtAngle->number();
        sceneEdge->refineTowardsEdge = chkRefineTowardsEdge->isChecked() ? txtRefineTowardsEdge->value() : 0;

        foreach (QComboBox* cmbBoundary, cmbBoundaries)
        {
            sceneEdge->addMarker(cmbBoundary->itemData(cmbBoundary->currentIndex()).value<SceneBoundary *>());
        }
    }
    else
    {
        foreach (SceneEdge* edge, m_edges.items())
        {
            foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
            {
                if(cmbBoundaries[fieldInfo]->currentIndex() != -1)
                    edge->addMarker(cmbBoundaries[fieldInfo]->itemData(cmbBoundaries[fieldInfo]->currentIndex()).value<SceneBoundary *>());

            }
        }

    }
    return true;
}

void SceneEdgeDialog::doBoundaryChanged(int index)
{
    //TODO - enable detail button
    // foreach (QComboBox* cmbBoundary, cmbBoundaries)
    //    btnBoundary->setEnabled(true);//cmbBoundary->currentIndex() > 0);
}

void SceneEdgeDialog::doBoundaryClicked()
{
    //TODO
    assert(0);
    //    SceneBoundary *marker = cmbBoundary->itemData(cmbBoundary->currentIndex()).value<SceneBoundary *>();
    //    if (marker->showDialog(this) == QDialog::Accepted)
    //    {
    //        cmbBoundary->setItemText(cmbBoundary->currentIndex(), QString::fromStdString(marker->getName()));
    //        Util::scene()->refresh();
    //    }
}

void SceneEdgeDialog::doNodeChanged()
{
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
            SceneEdge edge(nodeStart, nodeEnd, txtAngle->number(), 0); //TODO - do it better
            lblLength->setText(QString("%1 m").arg(edge.radius() * edge.angle / 180.0 * M_PI));
        }
    }
}

void SceneEdgeDialog::doRefineTowardsEdge(int state)
{
    txtRefineTowardsEdge->setEnabled(chkRefineTowardsEdge->isChecked());
}


// undo framework *******************************************************************************************************************
// **********************************************************************************************************************************

SceneEdgeCommandAdd::SceneEdgeCommandAdd(const Point &pointStart, const Point &pointEnd, const QString &markerName,
                                         double angle, int refineTowardsEdge, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_markerName = markerName;
    m_angle = angle;
    m_refineTowardsEdge = refineTowardsEdge;
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
                                         m_angle,
                                         m_refineTowardsEdge));
}

SceneEdgeCommandRemove::SceneEdgeCommandRemove(const Point &pointStart, const Point &pointEnd, const QString &markerName,
                                               double angle, int refineTowardsEdge, QUndoCommand *parent) : QUndoCommand(parent)
{    
    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_markerName = markerName;
    m_angle = angle;
    m_refineTowardsEdge = refineTowardsEdge;
}

void SceneEdgeCommandRemove::undo()
{
    assert(0); //TODO
    //    SceneBoundary *boundary = Util::scene()->getBoundary(m_markerName);
    //    if (boundary == NULL) boundary = Util::scene()->boundaries->get("none"); //TODO - do it better
    //    Util::scene()->addEdge(new SceneEdge(Util::scene()->addNode(new SceneNode(m_pointStart)),
    //                                         Util::scene()->addNode(new SceneNode(m_pointEnd)),
    //                                         boundary,
    //                                         m_angle,
    //                                         m_refineTowardsEdge));
}

void SceneEdgeCommandRemove::redo()
{
    Util::scene()->edges->remove(Util::scene()->getEdge(m_pointStart, m_pointEnd, m_angle));
}

SceneEdgeCommandEdit::SceneEdgeCommandEdit(const Point &pointStart, const Point &pointEnd, const Point &pointStartNew, const Point &pointEndNew,
                                           double angle, double angleNew, int refineTowardsEdge, int refineTowardsEdgeNew, QUndoCommand *parent) : QUndoCommand(parent)
{
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
