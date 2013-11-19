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

#include "pythonlab/pygeometry.h"
#include "pythonlab/pythonengine_agros.h"
#include "hermes2d/problem_config.h"
#include "sceneview_geometry.h"
#include "scenemarker.h"

void PyGeometry::activate()
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->trigger();
}

int PyGeometry::addNode(double x, double y)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actOperateOnNodes->trigger();

    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Axisymmetric && x < 0.0)
        throw out_of_range(QObject::tr("Radial component must be greater then or equal to zero.").toStdString());

    foreach (SceneNode *node, Agros2D::scene()->nodes->items())
    {
        if (node->point().x == x && node->point().y == y)
            throw logic_error(QObject::tr("Node already exist.").toStdString());
    }

    SceneNode *node = Agros2D::scene()->addNode(new SceneNode(Point(x, y)));
    return Agros2D::scene()->nodes->items().indexOf(node);
}

int PyGeometry::addEdge(double x1, double y1, double x2, double y2, double angle, int segments, int curvilinear,
                        const map<std::string, int> &refinements, const map<std::string, std::string> &boundaries)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actOperateOnEdges->trigger();

    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Axisymmetric && (x1 < 0.0 || x2 < 0.0))
        throw out_of_range(QObject::tr("Radial component must be greater then or equal to zero.").toStdString());

    testAngle(angle);
    testSegments(segments);

    foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
    {
        if (edge->nodeStart()->point().x == x1 && edge->nodeEnd()->point().x == x2 &&
                edge->nodeStart()->point().y == y1 && edge->nodeEnd()->point().y == y2)
            throw logic_error(QObject::tr("Edge already exist.").toStdString());
    }

    SceneNode *nodeStart = new SceneNode(Point(x1, y1));
    nodeStart = Agros2D::scene()->addNode(nodeStart);
    SceneNode *nodeEnd = new SceneNode(Point(x2, y2));
    nodeEnd = Agros2D::scene()->addNode(nodeEnd);
    SceneEdge *edge = new SceneEdge(nodeStart, nodeEnd,
                                    angle, segments, curvilinear);

    try
    {
        setBoundaries(edge, boundaries);
        setRefinementsOnEdge(edge, refinements);
    }
    catch (std::exception& e)
    {
        delete edge;
        throw;
    }

    Agros2D::scene()->addEdge(edge);

    return Agros2D::scene()->edges->items().indexOf(edge);
}

int PyGeometry::addEdgeByNodes(int nodeStartIndex, int nodeEndIndex, double angle, int segments, int curvilinear,
                               const map<std::string, int> &refinements, const map<std::string, std::string> &boundaries)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actOperateOnEdges->trigger();

    if (Agros2D::scene()->nodes->isEmpty())
        throw out_of_range(QObject::tr("Geometry does not contain nodes.").toStdString());

    if (nodeStartIndex == nodeEndIndex)
        throw logic_error(QObject::tr("Start node index is the same as index of end node.").toStdString());

    if (nodeStartIndex > (Agros2D::scene()->nodes->length() - 1) || nodeStartIndex < 0)
        throw out_of_range(QObject::tr("Node with index '%1' does not exist.").arg(nodeStartIndex).toStdString());
    if (nodeEndIndex > (Agros2D::scene()->nodes->length() - 1) || nodeEndIndex < 0)
        throw out_of_range(QObject::tr("Node with index '%1' does not exist.").arg(nodeEndIndex).toStdString());

    testAngle(angle);
    testSegments(segments);

    foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
    {
        if (Agros2D::scene()->nodes->items().indexOf(edge->nodeStart()) == nodeStartIndex &&
                Agros2D::scene()->nodes->items().indexOf(edge->nodeEnd()) == nodeEndIndex)
            throw logic_error(QObject::tr("Edge already exist.").toStdString());
    }

    SceneEdge *edge = new SceneEdge(Agros2D::scene()->nodes->at(nodeStartIndex), Agros2D::scene()->nodes->at(nodeEndIndex),
                                    angle, segments, curvilinear);

    try
    {
        setBoundaries(edge, boundaries);
        setRefinementsOnEdge(edge, refinements);
    }
    catch (std::exception& e)
    {
        delete edge;
        throw;
    }

    Agros2D::scene()->addEdge(edge);

    return Agros2D::scene()->edges->items().indexOf(edge);
}

void PyGeometry::modifyEdge(int index, double angle, int segments, int isCurvilinear, const map<std::string, int> &refinements, const map<std::string, std::string> &boundaries)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actOperateOnEdges->trigger();

    if (Agros2D::scene()->edges->isEmpty())
        throw out_of_range(QObject::tr("No edges are defined.").toStdString());

    if (index < 0 || index >= Agros2D::scene()->edges->length())
        throw out_of_range(QObject::tr("Edge index must be between 0 and '%1'.").arg(Agros2D::scene()->edges->length()-1).toStdString());

    testAngle(angle);
    testSegments(segments);

    SceneEdge *edge = Agros2D::scene()->edges->items().at(index);

    edge->setAngle(angle);
    edge->setSegments(segments);
    edge->setCurvilinear(isCurvilinear);

    setRefinementsOnEdge(edge, refinements);
    setBoundaries(edge, boundaries);

    Agros2D::scene()->invalidate();
}

void PyGeometry::testAngle(double angle) const
{
    if (angle < 0.0 || angle > 90.0)
        throw out_of_range(QObject::tr("Angle '%1' is out of range.").arg(angle).toStdString());
}

void PyGeometry::testSegments(int segments) const
{
    if (segments < 3 || segments > 20)
        throw out_of_range(QObject::tr("Segments '%1' is out of range.").arg(segments).toStdString());
}

void PyGeometry::setBoundaries(SceneEdge *edge, const map<std::string, std::string> &boundaries)
{
    for (map<std::string, std::string>::const_iterator i = boundaries.begin(); i != boundaries.end(); ++i)
    {
        QString field = QString::fromStdString((*i).first);
        QString marker = QString::fromStdString((*i).second);

        if (!Agros2D::problem()->hasField(field))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(field).toStdString());

        bool assigned = false;
        foreach (SceneBoundary *boundary, Agros2D::scene()->boundaries->filter(Agros2D::problem()->fieldInfo(field)).items())
        {
            if (boundary->name() == marker)
            {
                assigned = true;
                edge->addMarker(boundary);
                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Boundary condition '%1' doesn't exists.").arg(marker).toStdString());
    }
}

void PyGeometry::setRefinementsOnEdge(SceneEdge *edge, const map<std::string, int> &refinements)
{
    for (map<std::string, int>::const_iterator i = refinements.begin(); i != refinements.end(); ++i)
    {
        QString field = QString::fromStdString((*i).first);
        int refinement = (*i).second;

        if (!Agros2D::problem()->hasField(field))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(field).toStdString());

        if ((refinement < 0) || (refinement > 10))
            throw out_of_range(QObject::tr("Number of refinements '%1' is out of range (0 - 10).").arg(refinement).toStdString());

        Agros2D::problem()->fieldInfo(field)->setEdgeRefinement(edge, refinement);
    }
}

int PyGeometry::addLabel(double x, double y, double area, const map<std::string, int> &refinements,
                         const map<std::string, int> &orders, const map<std::string, std::string> &materials)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actOperateOnLabels->trigger();

    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Axisymmetric && x < 0.0)
        throw out_of_range(QObject::tr("Radial component must be greater then or equal to zero.").toStdString());

    if (area < 0.0)
        throw out_of_range(QObject::tr("Area must be positive.").toStdString());

    foreach (SceneLabel *label, Agros2D::scene()->labels->items())
    {
        if (label->point().x == x && label->point().y == y)
            throw logic_error(QObject::tr("Label already exist.").toStdString());
    }

    SceneLabel *label = new SceneLabel(Point(x, y), area);

    try
    {
        setMaterials(label, materials);
        setRefinements(label, refinements);
        setPolynomialOrders(label, orders);
    }
    catch (std::exception& e)
    {
        delete label;
        throw;
    }

    Agros2D::scene()->addLabel(label);

    return Agros2D::scene()->labels->items().indexOf(label);
}

void PyGeometry::modifyLabel(int index, double area, const map<std::string, int> &refinements,
                             const map<std::string, int> &orders, const map<std::string, std::string> &materials)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actOperateOnLabels->trigger();

    if (Agros2D::scene()->labels->isEmpty())
        throw out_of_range(QObject::tr("No labels are defined.").toStdString());

    if (index < 0 || index >= Agros2D::scene()->labels->length())
        throw out_of_range(QObject::tr("Label index must be between 0 and '%1'.").arg(Agros2D::scene()->labels->length()-1).toStdString());

    if (area < 0.0)
        throw out_of_range(QObject::tr("Area must be positive.").toStdString());

    SceneLabel *label = Agros2D::scene()->labels->at(index);

    label->setArea(area);
    setMaterials(label, materials);
    setRefinements(label, refinements);
    setPolynomialOrders(label, orders);

    Agros2D::scene()->invalidate();
}

void PyGeometry::setMaterials(SceneLabel *label, const map<std::string, std::string> &materials)
{
    for( map<std::string, std::string>::const_iterator i = materials.begin(); i != materials.end(); ++i)
    {
        QString field = QString::fromStdString((*i).first);
        QString marker = QString::fromStdString((*i).second);

        if (!Agros2D::problem()->hasField(field))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(field).toStdString());

        if (marker != "none")
        {

            bool assigned = false;
            foreach (SceneMaterial *material, Agros2D::scene()->materials->filter(Agros2D::problem()->fieldInfo(field)).items())
            {
                if ((material->fieldId() == field) && (material->name() == marker))
                {
                    assigned = true;
                    label->addMarker(material);
                    break;
                }
            }

            if (!assigned)
                throw invalid_argument(QObject::tr("Material '%1' doesn't exists.").arg(marker).toStdString());
        }
    }
}

void PyGeometry::setRefinements(SceneLabel *label, const map<std::string, int> &refinements)
{
    for (map<std::string, int>::const_iterator i = refinements.begin(); i != refinements.end(); ++i)
    {
        QString field = QString::fromStdString((*i).first);
        int refinement = (*i).second;

        if (!Agros2D::problem()->hasField(field))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(field).toStdString());

        if ((refinement < 0) || (refinement > 10))
            throw out_of_range(QObject::tr("Number of refinements '%1' is out of range (0 - 10).").arg(refinement).toStdString());

        Agros2D::problem()->fieldInfo(field)->setLabelRefinement(label, refinement);
    }
}

void PyGeometry::setPolynomialOrders(SceneLabel *label, const map<std::string, int> &orders)
{
    for (map<std::string, int>::const_iterator i = orders.begin(); i != orders.end(); ++i)
    {
        QString field = QString::fromStdString((*i).first);
        int order = (*i).second;

        if (!Agros2D::problem()->hasField(field))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(field).toStdString());

        if ((order < 1) || (order > 10))
            throw out_of_range(QObject::tr("Polynomial order '%1' is out of range (1 - 10).").arg(order).toStdString());

        Agros2D::problem()->fieldInfo(field)->setLabelPolynomialOrder(label, order);
    }
}

void PyGeometry::removeNodes(const vector<int> &nodes)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actOperateOnNodes->trigger();
    Agros2D::scene()->selectNone();

    if (!nodes.empty())
    {
        for (vector<int>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros2D::scene()->nodes->length()))
                Agros2D::scene()->nodes->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Node index must be between 0 and '%1'.").arg(Agros2D::scene()->nodes->length()-1).toStdString());
        }
    }
    else
        Agros2D::scene()->selectAll(SceneGeometryMode_OperateOnNodes);

    Agros2D::scene()->deleteSelected();

    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
}

void PyGeometry::removeEdges(const vector<int> &edges)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actOperateOnEdges->trigger();
    Agros2D::scene()->selectNone();

    if (!edges.empty())
    {
        for (vector<int>::const_iterator it = edges.begin(); it != edges.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros2D::scene()->edges->length()))
                Agros2D::scene()->edges->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Edge index must be between 0 and '%1'.").arg(Agros2D::scene()->edges->length()-1).toStdString());
        }
    }
    else
        Agros2D::scene()->selectAll(SceneGeometryMode_OperateOnEdges);

    Agros2D::scene()->deleteSelected();

    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
}

void PyGeometry::removeLabels(const vector<int> &labels)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actOperateOnLabels->trigger();
    Agros2D::scene()->selectNone();

    if (!labels.empty())
    {
        for (vector<int>::const_iterator it = labels.begin(); it != labels.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros2D::scene()->labels->length()))
                Agros2D::scene()->labels->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Label index must be between 0 and '%1'.").arg(Agros2D::scene()->labels->length()-1).toStdString());
        }
    }
    else
        Agros2D::scene()->selectAll(SceneGeometryMode_OperateOnLabels);

    Agros2D::scene()->deleteSelected();

    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
}

void PyGeometry::selectNodes(const vector<int> &nodes)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actOperateOnNodes->trigger();
    Agros2D::scene()->selectNone();

    if (!nodes.empty())
    {
        for (vector<int>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros2D::scene()->nodes->length()))
                Agros2D::scene()->nodes->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Node index must be between 0 and '%1'.").arg(Agros2D::scene()->nodes->length()-1).toStdString());
        }
    }
    else
    {
        Agros2D::scene()->selectAll(SceneGeometryMode_OperateOnNodes);
    }

    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();

}

void PyGeometry::selectEdges(const vector<int> &edges)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actOperateOnEdges->trigger();
    Agros2D::scene()->selectNone();

    if (!edges.empty())
    {
        for (vector<int>::const_iterator it = edges.begin(); it != edges.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros2D::scene()->edges->length()))
                Agros2D::scene()->edges->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Edge index must be between 0 and '%1'.").arg(Agros2D::scene()->edges->length()-1).toStdString());
        }
    }
    else
    {
        Agros2D::scene()->selectAll(SceneGeometryMode_OperateOnEdges);
    }

    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
}

void PyGeometry::selectLabels(const vector<int> &labels)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actOperateOnLabels->trigger();
    Agros2D::scene()->selectNone();

    if (!labels.empty())
    {
        for (vector<int>::const_iterator it = labels.begin(); it != labels.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros2D::scene()->labels->length()))
                Agros2D::scene()->labels->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Label index must be between 0 and '%1'.").arg(Agros2D::scene()->labels->length()-1).toStdString());
        }
    }
    else
    {
        Agros2D::scene()->selectAll(SceneGeometryMode_OperateOnLabels);
    }

    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
}

void PyGeometry::selectNodeByPoint(double x, double y)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actOperateOnNodes->trigger();
    Agros2D::scene()->selectNone();

    SceneNode *node = SceneNode::findClosestNode(Point(x, y));
    if (node)
    {
        node->setSelected(true);
        if (!silentMode())
            currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
    }
    else
        throw logic_error(QObject::tr("There are no nodes around the point [%1, %2].").arg(x).arg(y).toStdString());
}

void PyGeometry::selectEdgeByPoint(double x, double y)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actOperateOnEdges->trigger();
    Agros2D::scene()->selectNone();

    SceneEdge *edge = SceneEdge::findClosestEdge(Point(x, y));
    if (edge)
    {
        edge->setSelected(true);
        if (!silentMode())
            currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
    }
    else
        throw logic_error(QObject::tr("There are no edges around the point [%1, %2].").arg(x).arg(y).toStdString());
}

void PyGeometry::selectLabelByPoint(double x, double y)
{
    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actOperateOnLabels->trigger();
    Agros2D::scene()->selectNone();

    SceneLabel *label = SceneLabel::findClosestLabel(Point(x, y));
    if (label)
    {
        label->setSelected(true);
        if (!silentMode())
            currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
    }
    else
        throw logic_error(QObject::tr("There are no labels around the point [%1, %2].").arg(x).arg(y).toStdString());
}

void PyGeometry::selectNone()
{
    Agros2D::scene()->selectNone();

    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
}

void PyGeometry::moveSelection(double dx, double dy, bool copy, bool withMarkers)
{
    Agros2D::scene()->transformTranslate(Point(dx, dy), copy, withMarkers);

    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
}

void PyGeometry::rotateSelection(double x, double y, double angle, bool copy, bool withMarkers)
{
    Agros2D::scene()->transformRotate(Point(x, y), angle, copy, withMarkers);

    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
}

void PyGeometry::scaleSelection(double x, double y, double scale, bool copy, bool withMarkers)
{
    Agros2D::scene()->transformScale(Point(x, y), scale, copy, withMarkers);

    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
}

void PyGeometry::removeSelection()
{
    Agros2D::scene()->deleteSelected();

    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->refresh();
}

void PyGeometry::exportVTK(const std::string &fileName) const
{
    Agros2D::scene()->exportVTKGeometry(QString::fromStdString(fileName));
}
