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

#include "loops.h"

#include "meshgenerator.h"

#include "scene.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "util/global.h"
#include "poly2tri.h"

#include "scene.h"

const static int LOOPS_NON_EXISTING = -100000;
const static double TOL = 0.001;

// **************************************************************************************

LoopsInfo::LoopsNodeEdgeData::LoopsNodeEdgeData() : node(LOOPS_NON_EXISTING)
{
}

// **************************************************************************************

bool LoopsInfo::LoopsNode::hasUnvisited()
{
    foreach(LoopsNodeEdgeData ned, data)
        if(!ned.visited)
            return true;

    return false;
}

LoopsInfo::LoopsNodeEdgeData LoopsInfo::LoopsNode::startLoop()
{
    for (int i = 0; i < data.size(); i++){
        LoopsNodeEdgeData ned = data.at(i);
        if(!ned.visited){
            data[i].visited = true;
            //cout << "start loop " << i << endl;
            return ned;
        }
    }

    assert(0);
}

LoopsInfo::LoopsNodeEdgeData LoopsInfo::LoopsNode::continueLoop(int previousNode)
{
    LoopsNodeEdgeData previousNED;
    int index;
    for (int i = 0; i < data.size(); i++){
        LoopsNodeEdgeData ned = data.at(i);
        if(ned.node == previousNode){
            previousNED = ned;
            index = i;
            break;
        }
    }

    assert(previousNED.node != LOOPS_NON_EXISTING);

    int nextIdx = (index + 1) % data.size();
    //cout << "continue loop " << previousNode << ", " << data[nextIdx].node << endl;
    assert(!data.at(nextIdx).visited);
    data[nextIdx].visited = true;
    return data[nextIdx];
}

void LoopsInfo::LoopsNode::insertEdge(int endNode, int edgeIdx, bool reverse, double angle)
{
    int index = 0;

    for (int i = 0; i < data.size(); i++)
        if(angle < data[i].angle)
            index = i+1;
    data.insert(index, LoopsNodeEdgeData(endNode, edgeIdx, reverse, angle));
}

// ***********************************************************************

LoopsInfo::LoopsGraph::LoopsGraph(int numNodes)
{
    for (int i = 0; i < numNodes; i++)
        data.push_back(LoopsNode());
}

void LoopsInfo::LoopsGraph::addEdge(int startNode, int endNode, int edgeIdx, double angle)
{
    double angle2 = angle + M_PI;
    if(angle2 >= 2 * M_PI)
        angle2 -= 2 * M_PI;

    data[startNode].insertEdge(endNode, edgeIdx, false, angle);
    data[endNode].insertEdge(startNode, edgeIdx, true, angle2);
}

void LoopsInfo::LoopsGraph::print()
{
    /*
    for (int i = 0; i < data.size(); i++)
    {
        cout << "node " << i << "\n";
        foreach(LoopsNodeEdgeData ned, data[i].data)
        {
            cout << "     node " << ned.node << ", edge " << (ned.reverse ? "-" : "") << ned.edge << ", angle " << ned.angle << ", visited " << ned.visited << "\n";
        }
    }
    cout << "\n";
    */
}

bool LoopsInfo::isInsideSeg(double angleSegStart, double angleSegEnd, double angle)
{
    if(angleSegEnd > angleSegStart)
    {
        if((angle >= angleSegStart) && (angle <= angleSegEnd))
            return true;
        else
            return false;
    }
    else
    {
        assert(angleSegStart >= 0);
        assert(angleSegEnd <= 0);
        if((angle >= angleSegStart) || (angle <= angleSegEnd))
            return true;
        else
            return false;
    }
}

LoopsInfo::Intersection LoopsInfo::intersects(Point point, double tangent, SceneEdge* edge, Point& intersection)
{
    double x1 = edge->nodeStart()->point().x;
    double y1 = edge->nodeStart()->point().y;
    double x2 = edge->nodeEnd()->point().x;
    double y2 = edge->nodeEnd()->point().y;

    if(edge->angle() != 0)
    {
        double xC = edge->center().x;
        double yC = edge->center().y;

        double coef = - tangent * point.x + point.y - yC;
        double a = 1 + tangent*tangent;
        double b = - 2*xC + 2*tangent*coef;
        double c = xC*xC + coef*coef - edge->radius() * edge->radius();

        double disc = b*b - 4*c*a;
        if(disc <= 0)
            return Intersection_No;
        else
        {
            double xI1 = (-b + sqrt(disc))/ (2*a);
            double xI2 = (-b - sqrt(disc))/ (2*a);

            double yI1 = tangent * (xI1 - point.x) + point.y;
            double yI2 = tangent * (xI2 - point.x) + point.y;

            double angle1 = atan2(yI1 - yC, xI1 - xC);
            double angle2 = atan2(yI2 - yC, xI2 - xC);

            double angleSegStart = atan2(y1 - yC, x1 - xC);
            double angleSegEnd = atan2(y2 - yC, x2 - xC);

            int leftInter = 0;
            int rightInter = 0;

            //cout << "circle center " << xC << ", " << yC << ", radius " << edge->radius() << endl;
            //cout << "xI1 "<< xI1 << ", yI1 "<< yI1 << ", xI2 "<< xI2 << ", yI2 "<< yI2 << endl;
            //cout << "first: anglestart " << angleSegStart << ", end " << angleSegEnd << ", angle1 " << angle1 << ", x1" << x1 << ", point.x " << point.x << ", inside " << isInsideSeg(angleSegStart, angleSegEnd, angle1) << endl;
            //cout << "second: anglestart " << angleSegStart << ", end " << angleSegEnd << ", angle2 " << angle2 << ", x2" << x2 << ", point.x " << point.x << ", inside " << isInsideSeg(angleSegStart, angleSegEnd, angle2) << endl;

            if (isInsideSeg(angleSegStart, angleSegEnd, angle1))
            {
                if(xI1 < point.x)
                    leftInter++;
                else
                    rightInter++;
                intersection.x = xI1;
                intersection.y = yI1;
            }
            if (isInsideSeg(angleSegStart, angleSegEnd, angle2))
            {
                if(xI2 < point.x)
                    leftInter++;
                else
                    rightInter++;
                intersection.x = xI2;
                intersection.y = yI2;
            }

            //cout << "left " << leftInter << ", right " << rightInter << endl;

            if(leftInter == 2)
                return Intersection_No;
            else if(rightInter == 2)
                return Intersection_No;
            else if((leftInter == 1) && (rightInter == 0))
                return Intersection_Left;
            else if((leftInter == 0) && (rightInter == 1))
                return Intersection_Right;
            else if((leftInter == 1) && (rightInter == 1))
                return Intersection_Both;
            else if((leftInter == 0) && (rightInter == 0))
                return Intersection_No;
            else
                assert(0);
        }
    }

    if(fabs(x2-x1) > fabs(y2-y1))
    {
        double tangentSeg = (y2-y1) / (x2-x1);
        if(fabs(tangent - tangentSeg) < TOL)
            return Intersection_Uncertain;
        double xI = (tangentSeg * x1 - tangent * point.x - y1 + point.y) / (tangentSeg - tangent);

        if(fabs(x1 - xI) < TOL * edge->length())
            return Intersection_Uncertain;
        if(fabs(x2 - xI) < TOL * edge->length())
            return Intersection_Uncertain;

        if((xI > max(x1, x2)) || (xI < min(x1, x2)))
            return Intersection_No;

        if(xI < point.x)
            return Intersection_Left;
        else
            return Intersection_Right;
    }
    else
    {
        double invTangentSeg = (x2-x1) / (y2-y1);
        if(fabs(1/tangent - invTangentSeg) < TOL)
            return Intersection_Uncertain;
        double yI = (invTangentSeg * y1 - 1/tangent * point.y - x1 + point.x) / (invTangentSeg - 1/tangent);

        if(fabs(y1 - yI) < TOL * edge->length())
            return Intersection_Uncertain;
        if(fabs(y2 - yI) < TOL * edge->length())
            return Intersection_Uncertain;

        if((yI > max(y1, y2)) || (yI < min(y1, y2)))
            return Intersection_No;

        if(yI < point.y)
            return Intersection_Left;
        else
            return Intersection_Right;
    }
}

LoopsInfo::Intersection LoopsInfo::intersects(Point point, double tangent, SceneEdge* edge)
{
    Point intersection;
    return intersects(point, tangent, edge, intersection);
}

// *********************************************************************************************

LoopsInfo::LoopsInfo(Scene *scene)
    : QObject(), m_scene(scene)
{
    connect(m_scene, SIGNAL(invalidated()), this, SLOT(processPolygonTriangles()));
    connect(m_scene, SIGNAL(cleared()), this, SLOT(processPolygonTriangles()));
}

int LoopsInfo::intersectionsParity(Point point, QList<LoopsNodeEdgeData> loop)
{
    bool rejectTangent;
    double tangent = 0.;
    int left, right;
    do {
        tangent += 0.1;
        if (tangent > 10)
            throw AgrosGeometryException(tr("Intersection parity failed (tangent > 10)."));
        rejectTangent = false;
        left = right = 0;

        foreach (LoopsNodeEdgeData ned, loop)
        {
            Intersection result = intersects(point, tangent, m_scene->edges->at(ned.edge));
            if(result == Intersection_Uncertain)
            {
                rejectTangent = true;
                //cout << "rejected tangent\n";
                break;
            }
            else if(result == Intersection_Left)
                left++;
            else if(result == Intersection_Right)
                right++;
            else if(result == Intersection_Both)
            {
                left++;
                right++;
            }

        }

    }
    while (rejectTangent);

    //cout << "intersections left " << left << ", right " << right << endl;
    assert(left%2 == right%2);
    return left%2;
}

int LoopsInfo::windingNumber(Point point, QList<LoopsNodeEdgeData> loop)
{
    QList<double> angles;
    angles.reserve(loop.size() * 2);
    foreach (LoopsNodeEdgeData ned, loop)
    {
        // use two segments instead of arc. Point on arc to be found as intersection of arc with line going through
        // edge center and point
        SceneEdge* edge = m_scene->edges->at(ned.edge);
        if (!edge->isStraight())
        {
            Point intersection;
            Point edgePoint = edge->center();

            // if impossible to define tangent, we use different point on the edge (not center)
            if(fabs(edgePoint.x - point.x)/edge->length() < 0.00001)
                edgePoint = (edge->center() + edge->nodeEnd()->point()) / 2;

            double tangent = (edgePoint.y - point.y) / (edgePoint.x - point.x);
            Intersection intersectionType = intersects(point, tangent, edge, intersection);
            if ((intersectionType == Intersection_Left) || (intersectionType == Intersection_Right))
            {
                double additionalAngle = atan2(intersection.y - point.y,
                                                   intersection.x - point.x);
                angles.append(additionalAngle);
            }
        }

        // regular points
        Point nodePoint = m_scene->nodes->at(ned.node)->point();
        double angle = atan2(nodePoint.y - point.y,
                                 nodePoint.x - point.x);

        angles.append(angle);
    }

    double totalAngle = 0;
    for (int i = 0; i < angles.size(); i++)
    {
        double angle = angles[(i+1) % angles.size()] - angles[i];
        while (angle >= M_PI) angle -= 2*M_PI;
        while (angle < -M_PI) angle += 2*M_PI;
        assert((angle <= M_PI) && (angle >= -M_PI));

        totalAngle += angle;
    }

    double winding = totalAngle / (2*M_PI);
    int intWinding = floor(winding + 0.5);

    // check that total angle was multiple of 2*M_PI
    assert(fabs(winding - (double) intWinding) < 0.00001);
    return intWinding;
}

bool LoopsInfo::areSameLoops(QList<LoopsNodeEdgeData> loop1, QList<LoopsNodeEdgeData> loop2)
{
    if(loop1.size() != loop2.size())
        return false;

    QList<int> nodes1;
    nodes1.reserve(loop1.size());
    QList<int> nodes2;
    nodes2.reserve(loop2.size());

    foreach(LoopsNodeEdgeData ned, loop1)
        nodes1.push_back(ned.node);
    foreach(LoopsNodeEdgeData ned, loop2)
        nodes2.push_back(ned.node);

    for (int i = 0; i < nodes1.size(); i++)
        if(!nodes2.contains(nodes1.at(i)))
            return false;

    return true;
}

bool LoopsInfo::areEdgeDuplicities(QList<LoopsNodeEdgeData> loop)
{
    for (int i = 0; i < loop.length(); i++)
    {
        LoopsNodeEdgeData data = loop.at(i);

        for (int j = 0; j < loop.length(); j++)
        {
            if (i != j)
            {
                LoopsNodeEdgeData dataTest = loop.at(j);

                if (data.edge == dataTest.edge)
                    return true;
            }
        }
    }

    return false;
}

int LoopsInfo::longerLoop(int idx1, int idx2)
{
    int size1 = m_loops[idx1].size();
    int size2 = m_loops[idx2].size();

    if(size1 > size2)
        return idx1;
    else if(size1 < size2)
        return idx2;
    else
        assert(0);
}

bool LoopsInfo::shareEdge(int idx1, int idx2)
{
    foreach(LoopsNodeEdgeData ned1, m_loops[idx1])
    {
        foreach(LoopsNodeEdgeData ned2, m_loops[idx2])
        {
            if (ned1.edge == ned2.edge)
                return true;
        }
    }
    return false;
}

void LoopsInfo::switchOrientation(int idx)
{
    for (int i = 0; i < m_loops[idx].size() / 2; i++)
        swap(m_loops[idx][i], m_loops[idx][m_loops[idx].size() - 1 - i]);
    for (int i = 0; i < m_loops[idx].size(); i++)
        m_loops[idx][i].reverse = !m_loops[idx][i].reverse;
}

void LoopsInfo::addEdgePoints(QList<Point> *polyline, const SceneEdge &edge, bool reverse)
{
    QList<Point> localPolyline;

    if (!reverse)
        localPolyline.append(Point(edge.nodeStart()->point().x,
                                   edge.nodeStart()->point().y));

    if (!edge.isStraight())
    {
        Point center = edge.center();
        double radius = edge.radius();
        double startAngle = atan2(center.y - edge.nodeStart()->point().y,
                                  center.x - edge.nodeStart()->point().x) / M_PI*180.0 - 180.0;

        int segments = edge.angle() / 5.0;
        if (segments < 2) segments = 2;

        double theta = edge.angle() / double(segments);

        for (int i = 1; i < segments; i++)
        {
            double arc = (startAngle + i*theta)/180.0*M_PI;

            double x = radius * cos(arc);
            double y = radius * sin(arc);

            if (reverse)
                localPolyline.insert(0, Point(center.x + x, center.y + y));
            else
                localPolyline.append(Point(center.x + x, center.y + y));
        }
    }

    if (reverse)
        localPolyline.insert(0, Point(edge.nodeEnd()->point().x,
                                      edge.nodeEnd()->point().y));

    polyline->append(localPolyline);
}

void LoopsInfo::processLoops()
{
    // find loops
    LoopsGraph graph(m_scene->nodes->length());
    for (int i = 0; i < m_scene->edges->length(); i++)
    {
        SceneNode* startNode = m_scene->edges->at(i)->nodeStart();
        SceneNode* endNode = m_scene->edges->at(i)->nodeEnd();
        int startNodeIdx = m_scene->nodes->items().indexOf(startNode);
        int endNodeIdx = m_scene->nodes->items().indexOf(endNode);

        double angle = atan2(endNode->point().y - startNode->point().y,
                             endNode->point().x - startNode->point().x);
        if (angle < 0)
            angle += 2 * M_PI;

        graph.addEdge(startNodeIdx, endNodeIdx, i, angle);
    }

    graph.print();

    m_loops.clear();
    for (int i = 0; i < graph.data.size(); i++)
    {
        //cout << "** starting with node " << i << endl;
        LoopsNode& node = graph.data[i];
        int previousNodeIdx, currentNodeIdx;
        while (node.hasUnvisited())
        {
            graph.print();

            QList<LoopsNodeEdgeData> loop;
            LoopsNodeEdgeData ned = node.startLoop();
            previousNodeIdx = i;
            loop.push_back(ned);
            do
            {
                currentNodeIdx = ned.node;
                LoopsNode& actualNode = graph.data[currentNodeIdx];
                //cout << "call continue loop with node " << currentNodeIdx << "and previous node " << previousNodeIdx << endl;
                ned = actualNode.continueLoop(previousNodeIdx);
                previousNodeIdx = currentNodeIdx;
                loop.push_back(ned);
            } while (ned.node != i);

            if (areEdgeDuplicities(loop))
                throw AgrosGeometryException(QObject::tr("Two loops connected by one edge."));

            // for simple domains, we have the same loop twice. Do not include it second times
            if (m_loops.isEmpty() || !areSameLoops(loop, m_loops.last()))
                m_loops.append(loop);
        }
    }

    QList<QList< SceneLabel* > > labelsInsideLoop;
    QMap<SceneLabel*, QList<int> > loopsContainingLabel;
    QMap<SceneLabel*, int> principalLoopOfLabel;

    QMap<QPair<SceneLabel*, int>, int> windingNumbers;

    // find what labels are inside what loops
    for (int loopIdx = 0; loopIdx < m_loops.size(); loopIdx++)
    {
        labelsInsideLoop.push_back(QList<SceneLabel*>());
        for (int labelIdx = 0; labelIdx < m_scene->labels->count(); labelIdx++)
        {
            SceneLabel* label = m_scene->labels->at(labelIdx);
            int wn = windingNumber(label->point(), m_loops[loopIdx]);
            //cout << "winding number " << wn << endl;
            assert(wn < 2);
            windingNumbers[QPair<SceneLabel*, int>(label, loopIdx)] = wn;
            int ip = intersectionsParity(label->point(), m_loops[loopIdx]);
            // assert(abs(wn) == ip);
            if(ip == 1){
                labelsInsideLoop[loopIdx].push_back(label);
                if(!loopsContainingLabel.contains(label))
                    loopsContainingLabel[label] = QList<int>();
                loopsContainingLabel[label].push_back(loopIdx);
            }
        }
        if (labelsInsideLoop[loopIdx].size() == 0)
            throw AgrosGeometryException(tr("Some areas do not have a marker"));
    }

    for (int labelIdx = 0; labelIdx < m_scene->labels->count(); labelIdx++)
    {
        SceneLabel* label = m_scene->labels->at(labelIdx);
        if(!loopsContainingLabel.contains(label))
            throw AgrosGeometryException(tr("There is a label outside of the domain"));
    }

    // direct super and sub domains (indexed by loop indices)
    QList<int> superDomains;
    QList<QList<int> > subDomains;

    for (int i = 0; i < m_loops.size(); i++)
    {
        superDomains.push_back(-1);
        subDomains.push_back(QList<int>());
    }

    // outiside loops, not to be considered
    m_outsideLoops.clear();
    for (int labelIdx = 0; labelIdx < m_scene->labels->count(); labelIdx++)
    {
        SceneLabel* actualLabel = m_scene->labels->at(labelIdx);
        QList<int> loopsWithLabel = loopsContainingLabel[actualLabel];
        if(loopsWithLabel.size() == 0)
            throw AgrosGeometryException(tr("There is no label in some subdomain"));


        // sort
        for (int i = 0; i < loopsWithLabel.size(); i++)
        {
            for (int j = 0; j < loopsWithLabel.size() - 1; j++)
            {
                int numLabelsJ = labelsInsideLoop[loopsWithLabel[j]].size();
                int numLabelsJPlus1 = labelsInsideLoop[loopsWithLabel[j+1]].size();
                if(numLabelsJ > numLabelsJPlus1)
                    swap(loopsWithLabel[j+1], loopsWithLabel[j]);
            }
        }
        //        assert(labelsInsideLoop[loopsWithLabel[0]].size() == 1);
        //        assert(labelsInsideLoop[loopsWithLabel[0]][0] == actualLabel);

        int indexOfOutmost = loopsWithLabel[loopsWithLabel.size() - 1];
        int indexOfInmost = loopsWithLabel[0];
        principalLoopOfLabel[actualLabel] = indexOfInmost;

        // switch orientation if neccessary
        int windNum = windingNumbers[QPair<SceneLabel*, int>(actualLabel, indexOfInmost)];
        assert(abs(windNum) == 1);
        if(windNum == -1)
            switchOrientation(indexOfInmost);

        if ((labelsInsideLoop[indexOfOutmost].size() > 1) && (indexOfOutmost != indexOfInmost)
                && shareEdge(indexOfOutmost, indexOfInmost))
            m_outsideLoops.append(indexOfOutmost);

        for (int j = 0; j < loopsWithLabel.size() -1; j++)
        {
            int numLabelsJ = labelsInsideLoop[loopsWithLabel[j]].size();
            int numLabelsPlus1 = labelsInsideLoop[loopsWithLabel[j+1]].size();
            if (numLabelsJ == numLabelsPlus1)
                throw AgrosGeometryException(tr("There is no label in some subdomain"));
        }

        for (int i = 0; i < loopsWithLabel.size() - 1; i++)
        {
            int smallerLoop = loopsWithLabel[i];
            int biggerLoop = loopsWithLabel[i+1];
            assert(superDomains[smallerLoop] == biggerLoop || superDomains[smallerLoop] == -1);
            // if (!(superDomains[smallerLoop] == biggerLoop || superDomains[smallerLoop] == -1));
            //    throw AgrosGeometryException(tr("Unknown error"));
            superDomains[smallerLoop] = biggerLoop;
            if(!subDomains[biggerLoop].contains(smallerLoop))
                subDomains[biggerLoop].append(smallerLoop);
        }
    }

    m_labelLoops.clear();
    for (int labelIdx = 0; labelIdx < m_scene->labels->count(); labelIdx++)
    {
        SceneLabel* label = m_scene->labels->at(labelIdx);
        if(!principalLoopOfLabel.contains(label))
            throw AgrosGeometryException(tr("There is a label outside of the domain"));

        int principalLoop = principalLoopOfLabel[label];
        m_labelLoops[label] = QList<int>();
        m_labelLoops[label].push_back(principalLoop);
        for (int i = 0; i < subDomains[principalLoop].count(); i++)
        {
            m_labelLoops[label].append(subDomains[principalLoop][i]);
        }
    }

    // check for multiple labels
    QList<int> usedLoops;
    foreach (SceneLabel *label, principalLoopOfLabel.keys())
    {
        if (!usedLoops.contains(principalLoopOfLabel[label]))
            usedLoops.append(principalLoopOfLabel[label]);
        else
            throw AgrosGeometryException(tr("There is multiple labels in the domain"));
    }
}

QList<LoopsInfo::Triangle> LoopsInfo::triangulateLabel(const QList<Point> &polyline, const QList<QList<Point> > &holes)
{
    // create p2t structure
    vector<p2t::Point*> polylineP2T;

    foreach (Point point, polyline)
        polylineP2T.push_back(new p2t::Point(point.x, point.y));

    // create CDT
    p2t::CDT cdt(polylineP2T);

    vector<vector<p2t::Point*> > holesP2T;
    foreach (QList<Point> hole, holes)
    {
        vector<p2t::Point*> holeP2T;

        foreach (Point point, hole)
            holeP2T.push_back(new p2t::Point(point.x, point.y));

        holesP2T.push_back(holeP2T);
        cdt.AddHole(holeP2T);
    }

    // triangulate
    cdt.Triangulate();

    vector<p2t::Triangle*> tri = cdt.GetTriangles();

    // convert
    QList<Triangle> triangles;
    for (int i = 0; i < tri.size(); i++)
    {
        p2t::Triangle *triangle = tri[i];

        p2t::Point *a = triangle->GetPoint(0);
        p2t::Point *b = triangle->GetPoint(1);
        p2t::Point *c = triangle->GetPoint(2);

        triangles.append(Triangle(Point(a->x, a->y),
                                  Point(b->x, b->y),
                                  Point(c->x, c->y)));
    }

    tri.clear();

    // delete structures
    for (int i = 0; i < polylineP2T.size(); i++)
        delete polylineP2T.at(i);
    polylineP2T.clear();

    for (int i = 0; i < holesP2T.size(); i++)
    {
        for (int j = 0; j < holesP2T.at(i).size(); j++)
            delete holesP2T.at(i).at(j);
        holesP2T.at(i).clear();
    }
    holesP2T.clear();

    return triangles;
}

void LoopsInfo::processPolygonTriangles()
{
    m_polygonTriangles.clear();

    // TODO: rewrite to exceptions
    // find loops
    try
    {
        processLoops();

        QList<QList<Point> > polylines;
        for (int i = 0; i < m_loops.size(); i++)
        {
            QList<Point> polyline;

            // QList<Point> contour;
            for (int j = 0; j < m_loops[i].size(); j++)
            {
                SceneEdge *edge = m_scene->edges->items().at(m_loops[i][j].edge);
                if ((edge->nodeStart()->connectedEdges().size() > 1) && (edge->nodeEnd()->connectedEdges().size() > 1))
                {
                    if (m_loops[i][j].reverse)
                        addEdgePoints(&polyline, SceneEdge(edge->nodeStart(), edge->nodeEnd(), edge->angle()), true);
                    else
                        addEdgePoints(&polyline, SceneEdge(edge->nodeStart(), edge->nodeEnd(), edge->angle()));
                }
            }

            polylines.append(polyline);
        }

        foreach (SceneLabel* label, m_scene->labels->items())
        {
            // if (!label->isHole() && loopsInfo.labelToLoops[label].count() > 0)
            if (m_labelLoops[label].count() > 0)
            {
                // main polyline
                QList<Point> polyline = polylines[m_labelLoops[label][0]];

                // holes
                QList<QList<Point> > holes;
                for (int j = 1; j < m_labelLoops[label].count(); j++)
                {
                    QList<Point> hole = polylines[m_labelLoops[label][j]];
                    holes.append(hole);
                }

                QList<Triangle> triangles = triangulateLabel(polyline, holes);
                m_polygonTriangles.insert(label, triangles);
            }
        }

        // clear polylines
        foreach (QList<Point> polyline, polylines)
            polyline.clear();
        polylines.clear();

        m_isProcessPolygonError = false;
    }
    catch (AgrosGeometryException &e)
    {
        // do nothing
        m_isProcessPolygonError = true;
    }
}

void LoopsInfo::clear()
{
    m_loops.clear();
    m_labelLoops.clear();
    m_outsideLoops.clear();

    m_polygonTriangles.clear();
}
