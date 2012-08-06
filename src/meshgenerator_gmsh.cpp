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

#include "gui.h"
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
        Util::log()->printDebug(tr("Mesh generator"), tr("Poly file was created"));

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

        processGmsh.start(QString(Util::config()->commandGmsh).
                          arg(gmshBinary).
                          arg(tempProblemFileName()));

        if (!processGmsh.waitForStarted(100000))
        {
            Util::log()->printError(tr("Mesh generator"), tr("could not start GMSH."));
            processGmsh.kill();

            m_isError = true;
        }
        else
        {
            // copy gmsh files
            if ((!Util::config()->deleteMeshFiles) && (!Util::problem()->config()->fileName().isEmpty()))
            {
                QFileInfo fileInfoOrig(Util::problem()->config()->fileName());

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
        Util::log()->printMessage(tr("Mesh generator"), tr("mesh files were created"));
        // convert gmsh mesh to hermes mesh
        if (readGmshMeshFile())
        {
            Util::log()->printMessage(tr("Mesh generator"), tr("mesh was converted to Hermes2D mesh file"));

            // copy triangle files
            if ((!Util::config()->deleteHermesMeshFile) && (!Util::problem()->config()->fileName().isEmpty()))
            {
                QFileInfo fileInfoOrig(Util::problem()->config()->fileName());

                QFile::copy(tempProblemFileName() + ".mesh", fileInfoOrig.absolutePath() + QDir::separator() + fileInfoOrig.baseName() + ".mesh");
            }

            //  remove triangle temp files
            /*
            QFile::remove(tempProblemFileName() + ".geo");
            QFile::remove(tempProblemFileName() + ".msh");
            QFile::remove(tempProblemFileName() + ".gmsh.out");
            QFile::remove(tempProblemFileName() + ".gmsh.err");
            */
            Util::log()->printMessage(tr("Mesh generator"), tr("mesh files were deleted"));

            // load mesh
            try
            {
                QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> meshes = readMeshesFromFile(tempProblemFileName() + ".xml");

                // FIXME: jinak
                Util::problem()->setMeshesInitial(meshes);
            }
            catch (Hermes::Exceptions::Exception& e)
            {
                m_isError = true;

                Util::log()->printError(tr("Mesh generator"), QString("%1").arg(e.what()));
            }

        }
        else
        {
            m_isError = true;
            // QFile::remove(Util::problem()->config()->fileName() + ".mesh");
        }
    }
    else
    {
        m_isError = true;
        QString errorMessage = readFileContent(Util::problem()->config()->fileName() + ".gmsh.out");
        Util::log()->printError(tr("Mesh generator"), errorMessage);
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
    double angle;
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
            cout << "start loop " << i << endl;
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
    cout << "continue loop " << previousNode << ", " << data[nextIdx].node << endl;
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

    // increase edge index to count from 1
    data[startNode].insertEdge(endNode, edgeIdx+1, false, angle);
    data[endNode].insertEdge(startNode, edgeIdx+1, true, angle2);
}

void Graph::print()
{
    for(int i = 0; i < data.size(); i++)
    {
        cout << "node " << i << "\n";
        foreach(NodeEdgeData ned, data[i].data)
        {
            cout << "     node " << ned.node << ", edge " << (ned.reverse ? "-" : "") << ned.edge << ", angle " << ned.angle << ", visited " << ned.visited << "\n";
        }
    }
    cout << "\n";
}

double windingNumber(Point point, QList<NodeEdgeData> loop)
{
    QList<double> angles;
    foreach(NodeEdgeData ned, loop)
    {
        Point nodePoint = Util::scene()->nodes->at(ned.node)->point();
        double angle = atan2(nodePoint.y - point.y, nodePoint.x - point.x);
        assert((angle <= M_PI) && (angle >= -M_PI));

        // move to interval <0, 2*pi)
        if(angle < 0)
            angle += 2 * M_PI;
        angles.append(angle);
    }

    double totalAngle = 0;
    for(int i = 0; i < angles.size(); i++)
    {
        double angle = angles[(i+1) % angles.size()] - angles[i];
        //cout << ""
        if(angle < - M_PI)
            angle = 2*M_PI + angle;
        if(angle > M_PI)
            angle = -2*M_PI + angle;
        assert((angle <= M_PI) && (angle >= -M_PI));
        totalAngle += angle;
    }

    double winding = fabs(totalAngle / (2*M_PI));
    int intWinding = round(winding);
    assert(winding - (double)intWinding < 0.00001);
    return intWinding;
}

QMap<SceneLabel*, QList<NodeEdgeData> > findLoops()
{
    Graph graph(Util::scene()->nodes->length());
    for (int i = 0; i < Util::scene()->edges->length(); i++)
    {
        SceneNode* startNode = Util::scene()->edges->at(i)->nodeStart();
        SceneNode* endNode = Util::scene()->edges->at(i)->nodeEnd();
        int startNodeIdx = Util::scene()->nodes->items().indexOf(startNode);
        int endNodeIdx = Util::scene()->nodes->items().indexOf(endNode);

        double angle = atan2(endNode->point().y - startNode->point().y, endNode->point().x - startNode->point().x);
        if(angle < 0)
            angle += 2 * M_PI;

        graph.addEdge(startNodeIdx, endNodeIdx, i, angle);
    }

    graph.print();

    QList<QList<NodeEdgeData> > loops;
    for(int i = 0; i < graph.data.size(); i++)
    {
        cout << "** starting with node " << i << endl;
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
                cout << "call continue loop with node " << currentNodeIdx << "and previous node " << previousNodeIdx << endl;
                ned = actualNode.continueLoop(previousNodeIdx);
                previousNodeIdx = currentNodeIdx;
                loop.push_back(ned);
            } while(ned.node != i);

            loops.push_back(loop);
        }
    }

    QList<QList< SceneLabel* > > correspondingLabels;

    for(int loopIdx = 0; loopIdx < loops.size(); loopIdx++)
    {
        correspondingLabels.push_back(QList<SceneLabel*>());
        for(int labelIdx = 0; labelIdx < Util::scene()->labels->count(); labelIdx++)
        {
            SceneLabel* label = Util::scene()->labels->at(labelIdx);
            int wn = windingNumber(label->point(), loops[loopIdx]);
            cout << "winding number " << wn << endl;
            assert(wn < 2);
            if(wn == 1)
                correspondingLabels[loopIdx].push_back(label);
        }
    }

    // todo: improve exceptions strings

    int idxOuterLoop = -1;
    for(int i = 0; i < correspondingLabels.size(); i++)
    {
        if(correspondingLabels[i].size() == Util::scene()->labels->count())
            idxOuterLoop = i;
    }
    if(idxOuterLoop == -1)
        throw(AgrosMeshException("There is a label outside the domain"));

    QMap<SceneLabel*, QList<NodeEdgeData> > loopsMap;

    for(int i = 0; i < correspondingLabels.size(); i++)
    {
        int numLabels = correspondingLabels[i].size();
        if( numLabels == 0 )
        {
            throw(AgrosMeshException("There is no label in some domain"));
        }
        else if(numLabels > 1)
        {
            if(i != idxOuterLoop)
                throw(AgrosMeshException("There are two labels in some domain"));
        }
        else
        {
            SceneLabel* label = correspondingLabels[i][0];
            if(loopsMap.contains(label))
                assert((correspondingLabels.size() == 2) && (Util::scene()->labels->count() == 1));
            else
                loopsMap[label] = loops[i];
        }
    }

    return loopsMap;
}

bool MeshGeneratorGMSH::writeToGmsh()
{
    // basic check
    if (Util::scene()->nodes->length() < 3)
    {
        Util::log()->printError(tr("Mesh generator"), tr("invalid number of nodes (%1 < 3)").arg(Util::scene()->nodes->length()));
        return false;
    }
    if (Util::scene()->edges->length() < 3)
    {
        Util::log()->printError(tr("Mesh generator"), tr("invalid number of edges (%1 < 3)").arg(Util::scene()->edges->length()));
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
        Util::log()->printError(tr("Mesh generator"), tr("could not create GMSH geo mesh file (%1)").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);

    out << QString("mesh_size = 0;\n");

    // nodes
    QString outNodes;
    int nodesCount = 0;
    for (int i = 0; i<Util::scene()->nodes->length(); i++)
    {
        outNodes += QString("Point(%1) = {%2, %3, 0, mesh_size};\n").
                arg(i).
                arg(Util::scene()->nodes->at(i)->point().x, 0, 'f', 10).
                arg(Util::scene()->nodes->at(i)->point().y, 0, 'f', 10);
        nodesCount++;
    }

    // edges
    QString outEdges;
    int edgesCount = 0;
    for (int i = 0; i<Util::scene()->edges->length(); i++)
    {
        if (Util::scene()->edges->at(i)->angle() == 0)
        {
            // line .. increase edge index to count from 1
            outEdges += QString("Line(%1) = {%2, %3};\n").
                    arg(edgesCount+1).
                    arg(Util::scene()->nodes->items().indexOf(Util::scene()->edges->at(i)->nodeStart())).
                    arg(Util::scene()->nodes->items().indexOf(Util::scene()->edges->at(i)->nodeEnd()));
            edgesCount++;
        }
        else
        {
            // arc
            // add pseudo nodes
            Point center = Util::scene()->edges->at(i)->center();
            outNodes += QString("Point(%1) = {%2, %3, 0};\n").
                    arg(nodesCount).
                    arg(center.x, 0, 'f', 10).
                    arg(center.y, 0, 'f', 10);
            nodesCount++;

            outEdges += QString("Circle(%1) = {%2, %3, %4};\n").
                    arg(edgesCount+1).
                    arg(Util::scene()->nodes->items().indexOf(Util::scene()->edges->at(i)->nodeStart())).
                    arg(nodesCount - 1).
                    arg(Util::scene()->nodes->items().indexOf(Util::scene()->edges->at(i)->nodeEnd()));

            edgesCount++;
        }
    }

    /*
    // holes
    int holesCount = 0;
    foreach (SceneLabel *label, Util::scene()->labels->items())
        if (label->markersCount() == 0)
            holesCount++;

    QString outHoles = QString("%1\n").arg(holesCount);
    holesCount = 0;
    foreach (SceneLabel *label, Util::scene()->labels->items())
    {
        if (label->markersCount() == 0)
        {
            outHoles += QString("%1  %2  %3\n").
                    arg(holesCount).
                    // arg(Util::scene()->labels->items().indexOf(label) + 1).
                    arg(label->point().x, 0, 'f', 10).
                    arg(label->point().y, 0, 'f', 10);

            holesCount++;
        }
    }

    // labels
    QString outLabels;
    int labelsCount = 0;
    foreach (SceneLabel *label, Util::scene()->labels->items())
    {
        if (label->markersCount() > 0)
        {
            outLabels += QString("%1  %2  %3  %4  %5\n").
                    arg(labelsCount).
                    arg(label->point().x, 0, 'f', 10).
                    arg(label->point().y, 0, 'f', 10).
                    // arg(labelsCount + 1). // triangle returns zero region number for areas without marker, markers must start from 1
                    arg(Util::scene()->labels->items().indexOf(label) + 1).
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

    QMap<SceneLabel*, QList<NodeEdgeData> > loopsMap;
    try{
         loopsMap = findLoops();
    }
    catch(AgrosMeshException& ame)
    {
        cout << ame.str.toStdString() << endl;
    }

    QList<QList<NodeEdgeData> > loops = loopsMap.values();

    QString outLoops;
    for(int i = 0; i < loops.size(); i++)
    {
        outLoops.append(QString("Line Loop(%1) = {").arg(i+1));
        for(int j = 0; j < loops[i].size(); j++)
        {
            if(loops[i][j].reverse)
                outLoops.append("-");
            outLoops.append(QString("%1").arg(loops[i][j].edge));
            if(j < loops[i].size() - 1)
                outLoops.append(",");
        }
        outLoops.append(QString("};\n"));
        outLoops.append(QString("Plane Surface(%1) = {%1};\n").arg(i+1));
    }
    outLoops.append("\n");

    // quad mesh
    if (Util::problem()->config()->meshType() == MeshType_GMSH_Quad)
    {
        outLoops.append(QString("Recombine Surface {"));
        for(int i = 0; i < loops.size(); i++)
        {
            outLoops.append(QString("%1").arg(i+1));
            if(i < loops.size() - 1)
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
//    if (Util::problem()->config()->meshType() == MeshType_GMSH_Quad)
//        outLoops.append(QString("Recombine Surface {1, 2};\n"));

    // Mesh.Algorithm - 1=MeshAdapt, 2=Automatic, 5=Delaunay, 6=Frontal, 7=bamg, 8=delquad
    QString outCommands;
    if (Util::problem()->config()->meshType() == MeshType_GMSH_Quad)
    {
        outCommands.append(QString("Mesh.Algorithm = 8;\n"));
        outCommands.append(QString("Mesh.SubdivisionAlgorithm = 1;\n"));
    }
    else if (Util::problem()->config()->meshType() == MeshType_GMSH_Triangle)
    {
        outCommands.append(QString("Mesh.Algorithm = 2;\n"));
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
        Util::log()->printError(tr("Mesh generator"), tr("could not read GMSH mesh file"));
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
            Util::log()->printError(tr("Mesh generator"), tr("some areas have no label marker"));
            return false;
        }
        */
        labelMarkersCheck.insert(marker - 1);
    }
    int elementCountLinear = elementList.count();

    fileGMSH.close();

    writeToHermes();

    nodeList.clear();
    edgeList.clear();
    elementList.clear();  
}

