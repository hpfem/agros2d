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

#include "meshgenerator_gmsh.h"

#include "util/global.h"

#include "scene.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "sceneview_common.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "logview.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"

MeshGeneratorGMSH::MeshGeneratorGMSH() : MeshGenerator()
{    
}

bool MeshGeneratorGMSH::mesh()
{
    m_isError = false;

    QFile::remove(tempProblemFileName() + ".mesh");

    // create gmsh files
    if (writeToGmsh())
    {
        Agros2D::log()->printDebug(tr("Mesh generator"), tr("GMSH geo file was created"));

        // exec triangle
        QProcess processGmsh;
        processGmsh.setStandardOutputFile(tempProblemFileName() + ".gmsh.out");
        processGmsh.setStandardErrorFile(tempProblemFileName() + ".gmsh.err");
        connect(&processGmsh, SIGNAL(finished(int)), this, SLOT(meshGmshCreated(int)));

        QString gmshBinary = "gmsh";
        if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "gmsh.exe"))
            gmshBinary = "\"" + QApplication::applicationDirPath() + QDir::separator() + "gmsh.exe\"";
        if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "gmsh"))
            gmshBinary = QApplication::applicationDirPath() + QDir::separator() + "gmsh";

        processGmsh.start(QString(Agros2D::config()->commandGmsh).
                          arg(gmshBinary).
                          arg(tempProblemFileName()));

        if (!processGmsh.waitForStarted(100000))
        {
            Agros2D::log()->printError(tr("Mesh generator"), tr("could not start GMSH."));
            processGmsh.kill();

            m_isError = true;
        }
        else
        {
            // copy gmsh files
            if ((!Agros2D::config()->deleteMeshFiles) && (!Agros2D::problem()->config()->fileName().isEmpty()))
            {
                QFileInfo fileInfoOrig(Agros2D::problem()->config()->fileName());

                QFile::copy(tempProblemFileName() + ".geo", fileInfoOrig.absolutePath() + QDir::separator() + fileInfoOrig.baseName() + ".geo");
            }

            while (!processGmsh.waitForFinished()) {}
        }
    }
    else
    {
        m_isError = true;
    }

    return !m_isError;
}

void MeshGeneratorGMSH::meshGmshCreated(int exitCode)
{
    if (exitCode == 0)
    {
        Agros2D::log()->printMessage(tr("Mesh generator"), tr("mesh files were created"));
        // convert gmsh mesh to hermes mesh
        if (readGmshMeshFile())
        {
            Agros2D::log()->printMessage(tr("Mesh generator"), tr("mesh was converted to Hermes2D mesh file"));

            // copy triangle files
            if ((!Agros2D::config()->deleteHermesMeshFile) && (!Agros2D::problem()->config()->fileName().isEmpty()))
            {
                QFileInfo fileInfoOrig(Agros2D::problem()->config()->fileName());

                QFile::copy(cacheProblemDir() + "/initial.mesh", fileInfoOrig.absolutePath() + QDir::separator() + fileInfoOrig.baseName() + ".mesh");
            }

            //  remove gmsh temp files
            /*
            QFile::remove(tempProblemFileName() + ".geo");
            QFile::remove(tempProblemFileName() + ".msh");
            QFile::remove(tempProblemFileName() + ".gmsh.out");
            QFile::remove(tempProblemFileName() + ".gmsh.err");
            */
            Agros2D::log()->printMessage(tr("Mesh generator"), tr("mesh files were deleted"));
        }
        else
        {
            m_isError = true;
            // QFile::remove(Agros2D::problem()->config()->fileName() + ".mesh");
        }
    }
    else
    {
        m_isError = true;
        QString errorMessage = readFileContent(tempProblemFileName() + ".gmsh.err");
        errorMessage.insert(0, "\n");
        errorMessage.append("\n");
        Agros2D::log()->printError(tr("Mesh generator"), errorMessage.replace("\n", "<br/>"), false);
    }
}

// todo: create different exceptions for different errors and pass arguments as labels, lists of edges etc.. to be used in preprocessor
class AgrosMeshException
{
public:
    AgrosMeshException(QString str) { this->str = str; }
    QString str;
};


const int NON_EXISTING = -100000;

struct NodeEdgeData
{
    NodeEdgeData(): node(NON_EXISTING) {}
    NodeEdgeData(int node, int edge, bool reverse,  double angle) : node(node), edge(edge), reverse(reverse), angle(angle), visited(false) {}
    int node;
    int edge;
    bool reverse;
    double angle; // to order edges going from node (anti)clockwise
    bool visited;
};

struct Node
{
    void insertEdge(int endNode, int edgeIdx, bool reverse,  double angle);
    bool hasUnvisited();
    NodeEdgeData startLoop();
    NodeEdgeData continueLoop(int previousNode);
    void setVisited(int index) {data[index].visited = true;}

    QList<NodeEdgeData> data;
};

bool Node::hasUnvisited()
{
    foreach(NodeEdgeData ned, data)
        if(!ned.visited)
            return true;

    return false;
}

NodeEdgeData Node::startLoop()
{
    for(int i = 0; i < data.size(); i++){
        NodeEdgeData ned = data.at(i);
        if(!ned.visited){
            data[i].visited = true;
            //cout << "start loop " << i << endl;
            return ned;
        }
    }

    assert(0);
}

NodeEdgeData Node::continueLoop(int previousNode)
{
    NodeEdgeData previousNED;
    int index;
    for(int i = 0; i < data.size(); i++){
        NodeEdgeData ned = data.at(i);
        if(ned.node == previousNode){
            previousNED = ned;
            index = i;
            break;
        }
    }

    assert(previousNED.node != NON_EXISTING);

    int nextIdx = (index + 1) % data.size();
    //cout << "continue loop " << previousNode << ", " << data[nextIdx].node << endl;
    assert(!data.at(nextIdx).visited);
    data[nextIdx].visited = true;
    return data[nextIdx];
}

void Node::insertEdge(int endNode, int edgeIdx, bool reverse, double angle)
{
    int index = 0;

    for(int i = 0; i < data.size(); i++)
        if(angle < data[i].angle)
            index = i+1;
    data.insert(index, NodeEdgeData(endNode, edgeIdx, reverse, angle));
}

struct Graph
{
    Graph(int numNodes);
    void addEdge(int startNode, int endNode, int edgeIdx, double angle);
    void print();

    QList<Node> data;
};

Graph::Graph(int numNodes)
{
    for (int i = 0; i < numNodes; i++)
        data.push_back(Node());
}

void Graph::addEdge(int startNode, int endNode, int edgeIdx, double angle)
{
    double angle2 = angle + M_PI;
    if(angle2 >= 2 * M_PI)
        angle2 -= 2 * M_PI;

    data[startNode].insertEdge(endNode, edgeIdx, false, angle);
    data[endNode].insertEdge(startNode, edgeIdx, true, angle2);
}

void Graph::print()
{
    for(int i = 0; i < data.size(); i++)
    {
        //cout << "node " << i << "\n";
        foreach(NodeEdgeData ned, data[i].data)
        {
            //cout << "     node " << ned.node << ", edge " << (ned.reverse ? "-" : "") << ned.edge << ", angle " << ned.angle << ", visited " << ned.visited << "\n";
        }
    }
    //cout << "\n";
}

enum Intersection
{
    Intersection_Uncertain,
    Intersection_Left,
    Intersection_Right,
    Intersection_Both,
    Intersection_No
};

const double TOL = 0.001;

bool isInsideSeg(double angleSegStart, double angleSegEnd, double angle)
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

Intersection intersects(Point point, double tangent, SceneEdge* edge, Point& intersection)
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

            if(isInsideSeg(angleSegStart, angleSegEnd, angle1))
            {
                if(xI1 < point.x)
                    leftInter++;
                else
                    rightInter++;
                intersection.x = xI1;
                intersection.y = yI1;
            }
            if(isInsideSeg(angleSegStart, angleSegEnd, angle2))
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

Intersection intersects(Point point, double tangent, SceneEdge* edge)
{
    Point intersection;
    return intersects(point, tangent, edge, intersection);
}


int intersectionsParity(Point point, QList<NodeEdgeData> loop)
{
    bool rejectTangent;
    double tangent = 0.;
    int left, right;
    do{
        tangent += 0.1;
        //cout << "IntersectionParity, tangent " << tangent << endl;
        assert(tangent < 10);
        rejectTangent = false;
        left = right = 0;

        foreach(NodeEdgeData ned, loop)
        {
            Intersection result = intersects(point, tangent, Agros2D::scene()->edges->at(ned.edge));
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

    }while(rejectTangent);

    //cout << "intersections left " << left << ", right " << right << endl;
    assert(left%2 == right%2);
    return left%2;
}

int windingNumber(Point point, QList<NodeEdgeData> loop)
{
    QList<double> angles;
    foreach(NodeEdgeData ned, loop)
    {
        // use two segments instead of arc. Point on arc to be found as intersection of arc with line going through
        // edge center and point
        SceneEdge* edge = Agros2D::scene()->edges->at(ned.edge);
        if(edge->angle() != 0)
        {
            Point intersection;
            Point edgePoint = edge->center();

            // if impossible to define tangent, we use different point on the edge (not center)
            if(fabs(edgePoint.x - point.x)/edge->length() < 0.00001)
                edgePoint = (edge->center() + edge->nodeEnd()->point()) / 2;

            double tangent = (edgePoint.y - point.y) / (edgePoint.x - point.x);
            Intersection intersectionType = intersects(point, tangent, edge, intersection);
            if((intersectionType == Intersection_Left) || (intersectionType == Intersection_Right))
            {
                double additionalAngle = atan2(intersection.y - point.y, intersection.x - point.x);
                angles.append(additionalAngle);
            }
        }

        // regular points
        Point nodePoint = Agros2D::scene()->nodes->at(ned.node)->point();
        double angle = atan2(nodePoint.y - point.y, nodePoint.x - point.x);
        assert((angle <= M_PI) && (angle >= -M_PI));

        angles.append(angle);

    }

    double totalAngle = 0;
    for(int i = 0; i < angles.size(); i++)
    {
        double angle = angles[(i+1) % angles.size()] - angles[i];
        //cout << "angle: " << angle;
        if(angle < - M_PI)
            angle = 2*M_PI + angle;
        if(angle > M_PI)
            angle = -2*M_PI + angle;
        assert((angle <= M_PI) && (angle >= -M_PI));
        totalAngle += angle;
    }

    double winding = totalAngle / (2*M_PI);
    int intWinding = round(winding);

    // check that total angle was multiple of 2*M_PI
    assert(fabs(winding - (double)intWinding) < 0.00001);
    return intWinding;
}

bool areSameLoops(QList<NodeEdgeData> loop1, QList<NodeEdgeData> loop2)
{
    if(loop1.size() != loop2.size())
        return false;

    QList<int> nodes1, nodes2;
    foreach(NodeEdgeData ned, loop1)
        nodes1.push_back(ned.node);
    foreach(NodeEdgeData ned, loop2)
        nodes2.push_back(ned.node);

    for(int i = 0; i < nodes1.size(); i++)
        if(!nodes2.contains(nodes1.at(i)))
            return false;

    return true;
}

int longerLoop(QList<QList<NodeEdgeData> > loops, int idx1, int idx2)
{
    int size1 = loops[idx1].size();
    int size2 = loops[idx2].size();

    if(size1 > size2)
        return idx1;
    else if(size1 < size2)
        return idx2;
    else
        assert(0);
}

int shareEdge(QList<QList<NodeEdgeData> > loops, int idx1, int idx2)
{
    foreach(NodeEdgeData ned1, loops[idx1])
    {
        foreach(NodeEdgeData ned2, loops[idx2])
        {
            if(ned1.edge == ned2.edge)
                return true;
        }
    }
    return false;
}

struct LoopsInfo
{
    QList<QList<NodeEdgeData> > loops;
    QMap<SceneLabel*, QList<int> > labelToLoops;
    QList<int> outsideLoops;
};

void switchOrientation(QList<QList<NodeEdgeData> >& loops, int idx)
{
    QList<NodeEdgeData>& loop = loops[idx];
    for(int i = 0; i < loop.size() / 2; i++)
        swap(loop[i], loop[loop.size() - 1 - i]);
    for(int i = 0; i < loop.size(); i++)
        loop[i].reverse = !loop[i].reverse;

}

LoopsInfo findLoops()
{
    // find loops
    Graph graph(Agros2D::scene()->nodes->length());
    for (int i = 0; i < Agros2D::scene()->edges->length(); i++)
    {
        SceneNode* startNode = Agros2D::scene()->edges->at(i)->nodeStart();
        SceneNode* endNode = Agros2D::scene()->edges->at(i)->nodeEnd();
        int startNodeIdx = Agros2D::scene()->nodes->items().indexOf(startNode);
        int endNodeIdx = Agros2D::scene()->nodes->items().indexOf(endNode);

        double angle = atan2(endNode->point().y - startNode->point().y, endNode->point().x - startNode->point().x);
        if(angle < 0)
            angle += 2 * M_PI;

        graph.addEdge(startNodeIdx, endNodeIdx, i, angle);
    }

    graph.print();

    QList<QList<NodeEdgeData> > loops;
    for(int i = 0; i < graph.data.size(); i++)
    {
        //cout << "** starting with node " << i << endl;
        Node& node = graph.data[i];
        int previousNodeIdx, currentNodeIdx;
        while(node.hasUnvisited()){

            graph.print();

            QList<NodeEdgeData> loop;
            NodeEdgeData ned = node.startLoop();
            previousNodeIdx = i;
            loop.push_back(ned);
            do{
                currentNodeIdx = ned.node;
                Node& actualNode = graph.data[currentNodeIdx];
                //cout << "call continue loop with node " << currentNodeIdx << "and previous node " << previousNodeIdx << endl;
                ned = actualNode.continueLoop(previousNodeIdx);
                previousNodeIdx = currentNodeIdx;
                loop.push_back(ned);
            } while(ned.node != i);

            //for simple domains, we have the same loop twice. Do not include it second times
            if(loops.isEmpty() || !areSameLoops(loop, loops.last()))
                loops.push_back(loop);
        }
    }

    QList<QList< SceneLabel* > > labelsInsideLoop;
    QMap<SceneLabel*, QList<int> > loopsContainingLabel;
    QMap<SceneLabel*, int> principalLoopOfLabel;

    QMap<QPair<SceneLabel*, int>, int> windingNumbers;

    //find what labels are inside what loops
    for(int loopIdx = 0; loopIdx < loops.size(); loopIdx++)
    {
        labelsInsideLoop.push_back(QList<SceneLabel*>());
        for(int labelIdx = 0; labelIdx < Agros2D::scene()->labels->count(); labelIdx++)
        {
            SceneLabel* label = Agros2D::scene()->labels->at(labelIdx);
            int wn = windingNumber(label->point(), loops[loopIdx]);
            //cout << "winding number " << wn << endl;
            assert(wn < 2);
            windingNumbers[QPair<SceneLabel*, int>(label, loopIdx)] = wn;
            int ip = intersectionsParity(label->point(), loops[loopIdx]);
            assert(abs(wn) == ip);
            if(ip == 1){
                labelsInsideLoop[loopIdx].push_back(label);
                if(!loopsContainingLabel.contains(label))
                    loopsContainingLabel[label] = QList<int>();
                loopsContainingLabel[label].push_back(loopIdx);
            }
        }
        if(labelsInsideLoop[loopIdx].size() == 0)
            throw(AgrosMeshException("There is no label in some domain"));
    }

    for(int labelIdx = 0; labelIdx < Agros2D::scene()->labels->count(); labelIdx++)
    {
        SceneLabel* label = Agros2D::scene()->labels->at(labelIdx);
        if(!loopsContainingLabel.contains(label))
            throw(AgrosMeshException("There is a label outside of the domain"));
    }

    //direct super and sub domains (indexed by loop indices)
    QList<int> superDomains;
    QList<QList<int> > subDomains;

    for(int i = 0; i < loops.size(); i++)
    {
        superDomains.push_back(-1);
        subDomains.push_back(QList<int>());
    }

    // outiside loops, not to be considered
    QList<int> outsideLoops;

    for(int labelIdx = 0; labelIdx < Agros2D::scene()->labels->count(); labelIdx++)
    {
        SceneLabel* actualLabel = Agros2D::scene()->labels->at(labelIdx);
        QList<int> loopsWithLabel = loopsContainingLabel[actualLabel];
        if(loopsWithLabel.size() == 0)
            throw(AgrosMeshException("There is no label in some subdomain"));


        //sort
        for(int i = 0; i < loopsWithLabel.size(); i++)
        {
            for(int j = 0; j < loopsWithLabel.size() - 1; j++)
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

        //swith orientation if neccessary
        int windNum = windingNumbers[QPair<SceneLabel*, int>(actualLabel, indexOfInmost)];
        assert(abs(windNum) == 1);
        if(windNum == -1)
            switchOrientation(loops, indexOfInmost);

        if((labelsInsideLoop[indexOfOutmost].size() > 1) && (indexOfOutmost != indexOfInmost) && shareEdge(loops, indexOfOutmost, indexOfInmost))
            outsideLoops.push_back(indexOfOutmost);

        for(int j = 0; j < loopsWithLabel.size() -1; j++)
        {
            int numLabelsJ = labelsInsideLoop[loopsWithLabel[j]].size();
            int numLabelsPlus1 = labelsInsideLoop[loopsWithLabel[j+1]].size();
            if(numLabelsJ == numLabelsPlus1)
            {
                throw(AgrosMeshException("There is no label in some subdomain"));
            }
        }

        for(int i = 0; i < loopsWithLabel.size() - 1; i++)
        {
            int smallerLoop = loopsWithLabel[i];
            int biggerLoop = loopsWithLabel[i+1];
            assert(superDomains[smallerLoop] == biggerLoop || superDomains[smallerLoop] == -1);
            superDomains[smallerLoop] = biggerLoop;
            if(!subDomains[biggerLoop].contains(smallerLoop))
                subDomains[biggerLoop].append(smallerLoop);
        }
    }

    QMap<SceneLabel*, QList<int> > labelLoopsInfo;

    for(int labelIdx = 0; labelIdx < Agros2D::scene()->labels->count(); labelIdx++)
    {
        SceneLabel* label = Agros2D::scene()->labels->at(labelIdx);
        if(!principalLoopOfLabel.contains(label))
            throw(AgrosMeshException("There is a label outside of the domain"));

        int principalLoop = principalLoopOfLabel[label];
        labelLoopsInfo[label] = QList<int>();
        labelLoopsInfo[label].push_back(principalLoop);
        for(int i = 0; i < subDomains[principalLoop].count(); i++)
        {
            labelLoopsInfo[label].push_back(subDomains[principalLoop][i]);
        }
    }

    LoopsInfo loopsInfo;
    loopsInfo.loops = loops;
    loopsInfo.labelToLoops = labelLoopsInfo;
    loopsInfo.outsideLoops = outsideLoops;
    return loopsInfo;
}

bool MeshGeneratorGMSH::writeToGmsh()
{
    // basic check
    if (Agros2D::scene()->nodes->length() < 3)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("invalid number of nodes (%1 < 3)").arg(Agros2D::scene()->nodes->length()));
        return false;
    }
    if (Agros2D::scene()->edges->length() < 3)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("invalid number of edges (%1 < 3)").arg(Agros2D::scene()->edges->length()));
        return false;
    }

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    QDir dir;
    dir.mkdir(QDir::temp().absolutePath() + "/agros2d");
    QFile file(tempProblemFileName() + ".geo");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("could not create GMSH geo mesh file (%1)").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);

    // mesh size
    // RectPoint rect = Agros2D::scene()->boundingBox();
    // out << QString("mesh_size = %1;\n").arg(qMin(rect.width(), rect.height()) / 6.0);
    out << QString("mesh_size = 0;\n");

    // nodes
    QString outNodes;
    int nodesCount = 0;
    for (int i = 0; i<Agros2D::scene()->nodes->length(); i++)
    {
        outNodes += QString("Point(%1) = {%2, %3, 0, mesh_size};\n").
                arg(i).
                arg(Agros2D::scene()->nodes->at(i)->point().x, 0, 'f', 10).
                arg(Agros2D::scene()->nodes->at(i)->point().y, 0, 'f', 10);
        nodesCount++;
    }

    // edges
    QString outEdges;
    int edgesCount = 0;
    for (int i = 0; i<Agros2D::scene()->edges->length(); i++)
    {
        if (Agros2D::scene()->edges->at(i)->angle() == 0)
        {
            // line .. increase edge index to count from 1
            outEdges += QString("Line(%1) = {%2, %3};\n").
                    arg(edgesCount+1).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeStart())).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeEnd()));
            edgesCount++;
        }
        else
        {
            // arc
            // add pseudo nodes
            Point center = Agros2D::scene()->edges->at(i)->center();
            outNodes += QString("Point(%1) = {%2, %3, 0};\n").
                    arg(nodesCount).
                    arg(center.x, 0, 'f', 10).
                    arg(center.y, 0, 'f', 10);
            nodesCount++;

            outEdges += QString("Circle(%1) = {%2, %3, %4};\n").
                    arg(edgesCount+1).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeStart())).
                    arg(nodesCount - 1).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeEnd()));

            edgesCount++;
        }
    }

    /*
    // holes
    int holesCount = 0;
    foreach (SceneLabel *label, Agros2D::scene()->labels->items())
        if (label->markersCount() == 0)
            holesCount++;

    QString outHoles = QString("%1\n").arg(holesCount);
    holesCount = 0;
    foreach (SceneLabel *label, Agros2D::scene()->labels->items())
    {
        if (label->markersCount() == 0)
        {
            outHoles += QString("%1  %2  %3\n").
                    arg(holesCount).
                    // arg(Agros2D::scene()->labels->items().indexOf(label) + 1).
                    arg(label->point().x, 0, 'f', 10).
                    arg(label->point().y, 0, 'f', 10);

            holesCount++;
        }
    }

    // labels
    QString outLabels;
    int labelsCount = 0;
    foreach (SceneLabel *label, Agros2D::scene()->labels->items())
    {
        if (label->markersCount() > 0)
        {
            outLabels += QString("%1  %2  %3  %4  %5\n").
                    arg(labelsCount).
                    arg(label->point().x, 0, 'f', 10).
                    arg(label->point().y, 0, 'f', 10).
                    // arg(labelsCount + 1). // triangle returns zero region number for areas without marker, markers must start from 1
                    arg(Agros2D::scene()->labels->items().indexOf(label) + 1).
                    arg(label->area());
            labelsCount++;
        }
    }

    out << outHoles;
    outLabels.insert(0, QString("%1 1\n").
                     arg(labelsCount)); // - holes
    out << outLabels;
    */

    // TODO: find loops

    LoopsInfo loopsInfo;
    try{
        loopsInfo = findLoops();
    }
    catch(AgrosMeshException& ame)
    {
        cout << ame.str.toStdString() << endl;
    }

    QList<QList<NodeEdgeData> > loops = loopsInfo.loops;

    QString outLoops;
    for(int i = 0; i < loops.size(); i++)
    {
        if(! loopsInfo.outsideLoops.contains(i))
        {
            outLoops.append(QString("Line Loop(%1) = {").arg(i+1));
            for(int j = 0; j < loops[i].size(); j++)
            {
                if(loops[i][j].reverse)
                    outLoops.append("-");
                outLoops.append(QString("%1").arg(loops[i][j].edge + 1));
                if(j < loops[i].size() - 1)
                    outLoops.append(",");
            }
            outLoops.append(QString("};\n"));
        }
    }
    outLoops.append("\n");

    QList<int> surfaces;
    QMap<SceneLabel*, QList<int> > labelInfo = loopsInfo.labelToLoops;
    int surfaceCount = 0;
    for(int i = 0; i < Agros2D::scene()->labels->count(); i++)
    {
        surfaceCount++;
        SceneLabel* label = Agros2D::scene()->labels->at(i);
        if(!label->isHole())
        {
            surfaces.push_back(surfaceCount);
            outLoops.append(QString("Plane Surface(%1) = {").arg(surfaceCount));
            for(int j = 0; j < labelInfo[label].count(); j++)
            {
                outLoops.append(QString("%1").arg(labelInfo[label][j]+1));
                if(j < labelInfo[label].count() - 1)
                    outLoops.append(",");
            }
            outLoops.append(QString("};\n"));
        }
    }

//    outLoops.append(QString("Physical Surface(1) = {"));
//    for(int i = 0; i < surfaceCount; i++)
//    {
//        outLoops.append(QString("%1").arg(i+1));
//        if(i < surfaceCount - 1)
//            outLoops.append(",");
//    }
//    outLoops.append(QString("};\n"));

    // quad mesh
    if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_Quad ||
            Agros2D::problem()->config()->meshType() == MeshType_GMSH_QuadDelaunay_Experimental)
    {
        outLoops.append(QString("Recombine Surface {"));
        for(int i = 0; i <  surfaces.count(); i++)
        {
            outLoops.append(QString("%1").arg(surfaces.at(i)));
            if(i < surfaces.count() - 1)
                outLoops.append(",");
        }
        outLoops.append(QString("};\n"));
    }    
    //    QString outLoops;
    //    outLoops.append(QString("Line Loop(1) = {0, 1, 2, 3};\n"));
    //    outLoops.append(QString("Plane Surface(1) = {1};\n"));
    //    outLoops.append(QString("Line Loop(2) = {4, 5, 6, -1};\n"));
    //    outLoops.append(QString("Plane Surface(2) = {2};\n"));
    //    outLoops.append("\n");

    //    // quad mesh
    //    if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_Quad)
    //        outLoops.append(QString("Recombine Surface {1, 2};\n"));

    // Mesh.Algorithm - 1=MeshAdapt, 2=Automatic, 5=Delaunay, 6=Frontal, 7=bamg, 8=delquad
    QString outCommands;
    if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_Triangle)
    {
        outCommands.append(QString("Mesh.Algorithm = 2;\n"));
    }
    else if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_Quad)
    {
        outCommands.append(QString("Mesh.Algorithm = 2;\n"));
        outCommands.append(QString("Mesh.SubdivisionAlgorithm = 1;\n"));
    }
    else if (Agros2D::problem()->config()->meshType() == MeshType_GMSH_QuadDelaunay_Experimental)
    {
        outCommands.append(QString("Mesh.Algorithm = 8;\n"));
        outCommands.append(QString("Mesh.SubdivisionAlgorithm = 1;\n"));
    }

    outNodes.insert(0, QString("\n// nodes\n"));
    out << outNodes;
    outEdges.insert(0, QString("\n// edges\n"));
    out << outEdges;
    outLoops.insert(0, QString("\n// loops\n"));
    out << outLoops;
    outCommands.insert(0, QString("\n// commands\n"));
    out << outCommands;

    file.waitForBytesWritten(0);
    file.close();

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return true;
}

bool MeshGeneratorGMSH::readGmshMeshFile()
{
    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    int k;

    QFile fileGMSH(tempProblemFileName() + ".msh");
    if (!fileGMSH.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("could not read GMSH mesh file"));
        return false;
    }
    QTextStream inGMSH(&fileGMSH);

    // nodes
    inGMSH.readLine();
    inGMSH.readLine();
    inGMSH.readLine();
    inGMSH.readLine();
    sscanf(inGMSH.readLine().toStdString().c_str(), "%i", &k);
    for (int i = 0; i < k; i++)
    {
        int n;
        double x, y, z;

        sscanf(inGMSH.readLine().toStdString().c_str(), "%i %lf %lf %lf", &n, &x, &y, &z);
        nodeList.append(Point(x, y));
    }

    // elements
    inGMSH.readLine();
    inGMSH.readLine();
    sscanf(inGMSH.readLine().toStdString().c_str(), "%i", &k);
    QSet<int> labelMarkersCheck;
    for (int i = 0; i < k; i++)
    {
        int quad[4];
        int n, type, phys, part, marker;

        if (sscanf(inGMSH.readLine().toStdString().c_str(), "%i %i %i %i %i %i %i %i %i",
                   &n, &type, &phys, &part, &marker, &quad[0], &quad[1], &quad[2], &quad[3]))
        {
            // edge
            if (type == 1)
                edgeList.append(MeshEdge(quad[0] - 1, quad[1] - 1, marker - 1)); // marker conversion from gmsh, where it starts from 1
            // triangle
            if (type == 2)
                elementList.append(MeshElement(quad[0] - 1, quad[1] - 1, quad[2] - 1, marker - 1)); // marker conversion from gmsh, where it starts from 1
            // quad
            if (type == 3)
                elementList.append(MeshElement(quad[0] - 1, quad[1] - 1, quad[2] - 1, quad[3] - 1, marker - 1)); // marker conversion from gmsh, where it starts from 1
        }
        /*

        if (marker == 0)
        {
            Agros2D::log()->printError(tr("Mesh generator"), tr("some areas have no label marker"));
            return false;
        }
        */
        labelMarkersCheck.insert(marker - 1);
    }

    fileGMSH.close();

    writeToHermes();

    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    return true;
}

