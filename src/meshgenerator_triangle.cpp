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

#include "meshgenerator_triangle.h"

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

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "util/loops.h"

MeshGeneratorTriangle::MeshGeneratorTriangle() : MeshGenerator()
{    
}

bool MeshGeneratorTriangle::mesh()
{
    m_isError = !prepare();

    // create triangle files
    if (writeToTriangle())
    {
        Agros2D::log()->printDebug(tr("Mesh generator"), tr("Poly file was created"));

        // exec triangle
        QProcess processTriangle;
        processTriangle.setStandardOutputFile(tempProblemFileName() + ".triangle.out");
        processTriangle.setStandardErrorFile(tempProblemFileName() + ".triangle.err");
        connect(&processTriangle, SIGNAL(finished(int)), this, SLOT(meshTriangleCreated(int)));

        QString triangleBinary = "triangle";
        if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "triangle.exe"))
            triangleBinary = "\"" + QApplication::applicationDirPath() + QDir::separator() + "triangle.exe\"";
        if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "triangle"))
            triangleBinary = QApplication::applicationDirPath() + QDir::separator() + "triangle";

        processTriangle.start(QString(Agros2D::problem()->configView()->commandTriangle).
                              arg(triangleBinary).
                              arg(tempProblemFileName()));

        if (!processTriangle.waitForStarted(100000))
        {
            Agros2D::log()->printError(tr("Mesh generator"), tr("could not start Triangle"));
            processTriangle.kill();

            m_isError = true;
        }
        else
        {
            // copy triangle files
            if ((!Agros2D::configComputer()->deleteMeshFiles) && (!Agros2D::problem()->config()->fileName().isEmpty()))
            {
                QFileInfo fileInfoOrig(Agros2D::problem()->config()->fileName());

                QFile::copy(tempProblemFileName() + ".poly", fileInfoOrig.absolutePath() + QDir::separator() + fileInfoOrig.baseName() + ".poly");
                QFile::copy(tempProblemFileName() + ".node", fileInfoOrig.absolutePath() + QDir::separator() + fileInfoOrig.baseName() + ".node");
                QFile::copy(tempProblemFileName() + ".edge", fileInfoOrig.absolutePath() + QDir::separator() + fileInfoOrig.baseName() + ".edge");
                QFile::copy(tempProblemFileName() + ".ele", fileInfoOrig.absolutePath() + QDir::separator() + fileInfoOrig.baseName() + ".ele");
            }

            while (!processTriangle.waitForFinished()) {}
        }
    }
    else
    {
        m_isError = true;
    }

    return !m_isError;
}

void MeshGeneratorTriangle::meshTriangleCreated(int exitCode)
{
    if (exitCode == 0)
    {
        Agros2D::log()->printMessage(tr("Mesh generator"), tr("mesh files were created"));

        // convert triangle mesh to hermes mesh
        if (readTriangleMeshFormat())
        {
            Agros2D::log()->printMessage(tr("Mesh generator"), tr("mesh was converted to Hermes2D mesh file"));

            // copy triangle files
            if ((!Agros2D::configComputer()->deleteHermesMeshFile) && (!Agros2D::problem()->config()->fileName().isEmpty()))
            {
                QFileInfo fileInfoOrig(Agros2D::problem()->config()->fileName());

                QFile::copy(cacheProblemDir() + "/initial.mesh", fileInfoOrig.absolutePath() + QDir::separator() + fileInfoOrig.baseName() + ".mesh");
            }

            //  remove triangle temp files
            QFile::remove(tempProblemFileName() + ".poly");
            QFile::remove(tempProblemFileName() + ".node");
            QFile::remove(tempProblemFileName() + ".edge");
            QFile::remove(tempProblemFileName() + ".ele");
            QFile::remove(tempProblemFileName() + ".neigh");
            QFile::remove(tempProblemFileName() + ".triangle.out");
            QFile::remove(tempProblemFileName() + ".triangle.err");
        }
        else
        {
            m_isError = true;
            QFile::remove(Agros2D::problem()->config()->fileName() + ".mesh");
        }
    }
    else
    {
        m_isError = true;
        QString errorMessage = readFileContent(tempProblemFileName() + ".triangle.err");
        errorMessage.insert(0, "\n");
        errorMessage.append("\n");
        Agros2D::log()->printError(tr("Mesh generator"), errorMessage.replace("\n", "<br/>"), false);
    }
}

bool MeshGeneratorTriangle::writeToTriangle()
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
    QFile file(tempProblemFileName() + ".poly");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("could not create Triangle poly mesh file (%1)").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);


    // nodes
    QString outNodes;
    int nodesCount = 0;
    for (int i = 0; i<Agros2D::scene()->nodes->length(); i++)
    {
        outNodes += QString("%1  %2  %3  %4\n").
                arg(i).
                arg(Agros2D::scene()->nodes->at(i)->point().x, 0, 'f', 10).
                arg(Agros2D::scene()->nodes->at(i)->point().y, 0, 'f', 10).
                arg(0);
        nodesCount++;
    }

    // edges
    QString outEdges;
    int edgesCount = 0;
    for (int i = 0; i<Agros2D::scene()->edges->length(); i++)
    {
        if (Agros2D::scene()->edges->at(i)->angle() == 0)
        {
            // line
            outEdges += QString("%1  %2  %3  %4\n").
                    arg(edgesCount).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeStart())).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeEnd())).
                    arg(i+1);
            edgesCount++;
        }
        else
        {
            // arc
            // add pseudo nodes
            Point center = Agros2D::scene()->edges->at(i)->center();
            double radius = Agros2D::scene()->edges->at(i)->radius();
            double startAngle = atan2(center.y - Agros2D::scene()->edges->at(i)->nodeStart()->point().y,
                                      center.x - Agros2D::scene()->edges->at(i)->nodeStart()->point().x) - M_PI;

            int segments = Agros2D::scene()->edges->at(i)->segments();
            double theta = deg2rad(Agros2D::scene()->edges->at(i)->angle()) / double(segments);

            int nodeStartIndex = 0;
            int nodeEndIndex = 0;
            for (int j = 0; j < segments; j++)
            {
                double arc = startAngle + j*theta;

                double x = radius * cos(arc);
                double y = radius * sin(arc);

                nodeEndIndex = nodesCount+1;
                if (j == 0)
                {
                    nodeStartIndex = Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeStart());
                    nodeEndIndex = nodesCount;
                }
                if (j == segments - 1)
                {
                    nodeEndIndex = Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeEnd());
                }
                if ((j > 0) && (j < segments))
                {
                    outNodes += QString("%1  %2  %3  %4\n").
                            arg(nodesCount).
                            arg(center.x + x, 0, 'f', 10).
                            arg(center.y + y, 0, 'f', 10).
                            arg(0);
                    nodesCount++;
                }
                outEdges += QString("%1  %2  %3  %4\n").
                        arg(edgesCount).
                        arg(nodeStartIndex).
                        arg(nodeEndIndex).
                        arg(i+1);
                edgesCount++;
                nodeStartIndex = nodeEndIndex;
            }
        }
    }

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

    outNodes.insert(0, QString("%1 2 0 1\n").
                    arg(nodesCount)); // + additional Agros2D::scene()->nodes
    out << outNodes;
    outEdges.insert(0, QString("%1 1\n").
                    arg(edgesCount)); // + additional edges
    out << outEdges;
    out << outHoles;
    outLabels.insert(0, QString("%1 1\n").
                     arg(labelsCount)); // - holes
    out << outLabels;

    file.waitForBytesWritten(0);
    file.close();

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return true;
}

bool MeshGeneratorTriangle::readTriangleMeshFormat()
{
    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    QFile fileNode(tempProblemFileName() + ".node");
    if (!fileNode.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("could not read Triangle node file"));
        return false;
    }
    QTextStream inNode(&fileNode);    

    QFile fileEdge(tempProblemFileName() + ".edge");
    if (!fileEdge.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("could not read Triangle edge file"));
        return false;
    }
    QTextStream inEdge(&fileEdge);

    QFile fileEle(tempProblemFileName() + ".ele");
    if (!fileEle.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("could not read Triangle ele file"));
        return false;
    }
    QTextStream inEle(&fileEle);

    QFile fileNeigh(tempProblemFileName() + ".neigh");
    if (!fileNeigh.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("could not read Triangle neigh file"));
        return false;
    }
    QTextStream inNeigh(&fileNeigh);

    // white chars
    QRegExp whiteChar("\\s+");

    // triangle nodes
    QString lineNode = inNode.readLine().trimmed();
    int numberOfNodes = lineNode.split(whiteChar).at(0).toInt();
    for (int i = 0; i < numberOfNodes; i++)
    {
        // suspisious code, causes the "Concave element ...." exception
        QStringList parsedLine = inNode.readLine().trimmed().split(whiteChar);

        nodeList.append(Point(parsedLine.at(1).toDouble(),
                              parsedLine.at(2).toDouble()));
    }

    // triangle edges
    QString lineEdge = inEdge.readLine().trimmed();
    int numberOfEdges = lineEdge.split(whiteChar).at(0).toInt();
    for (int i = 0; i < numberOfEdges; i++)
    {
        QStringList parsedLine = inEdge.readLine().trimmed().split(whiteChar);

        // marker conversion from triangle, where it starts from 1
        edgeList.append(MeshEdge(parsedLine.at(1).toInt(),
                                 parsedLine.at(2).toInt(),
                                 parsedLine.at(3).toInt() - 1));
    }
    int edgeCountLinear = edgeList.count();

    // triangle elements
    QString lineElement = inEle.readLine().trimmed();
    int numberOfElements = lineElement.split(whiteChar).at(0).toInt();
    QSet<int> labelMarkersCheck;
    for (int i = 0; i < numberOfElements; i++)
    {
        QStringList parsedLine = inEle.readLine().trimmed().split(whiteChar);
        int marker = parsedLine.at(7).toInt();

        if (marker == 0)
        {
            Agros2D::log()->printError(tr("Mesh generator"), tr("Some areas do not have a marker"));
            return false;
        }

        // vertices
        int nodeA = parsedLine.at(1).toInt();
        int nodeB = parsedLine.at(2).toInt();
        int nodeC = parsedLine.at(3).toInt();
        // 2nd order nodes (in the middle of edges)
        int nodeNA = parsedLine.at(4).toInt();
        int nodeNB = parsedLine.at(5).toInt();
        int nodeNC = parsedLine.at(6).toInt();

        if (Agros2D::problem()->config()->meshType() == MeshType_Triangle ||
                Agros2D::problem()->config()->meshType() == MeshType_Triangle_QuadJoin ||
                Agros2D::problem()->config()->meshType() == MeshType_Triangle_QuadRoughDivision)
        {
            elementList.append(MeshElement(nodeA, nodeB, nodeC, marker - 1)); // marker conversion from triangle, where it starts from 1
        }

        if (Agros2D::problem()->config()->meshType() == MeshType_Triangle_QuadFineDivision)
        {
            // add additional node
            nodeList.append(Point((nodeList[nodeA].x + nodeList[nodeB].x + nodeList[nodeC].x) / 3.0,
                                  (nodeList[nodeA].y + nodeList[nodeB].y + nodeList[nodeC].y) / 3.0));
            // add three quad elements
            elementList.append(MeshElement(nodeNB, nodeA, nodeNC, nodeList.count() - 1, marker - 1)); // marker conversion from triangle, where it starts from 1
            elementList.append(MeshElement(nodeNC, nodeB, nodeNA, nodeList.count() - 1, marker - 1)); // marker conversion from triangle, where it starts from 1
            elementList.append(MeshElement(nodeNA, nodeC, nodeNB, nodeList.count() - 1, marker - 1)); // marker conversion from triangle, where it starts from 1
        }

        labelMarkersCheck.insert(marker - 1);
    }
    int elementCountLinear = elementList.count();

    // check for multiple label markers
    foreach (SceneLabel *label, Agros2D::scene()->labels->items())
    {
        // if (Agros2D::scene()->materials.indexOf(Agros2D::scene()->labels[i]->material) > 0)
        {
            //            if (!labelMarkersCheck.contains(i + 1))
            //            {
            //                emit message(tr("Label marker '%1' is not present in mesh file (multiple label markers in one area).").
            //                             arg(i), true, 0);
            //                return false;
            //            }
        }
    }
    labelMarkersCheck.clear();

    // TODO: move
    /*
    if (elementList.count() < 1)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("invalid number of label markers"));
        return false;
    }
    */

    // triangle neigh
    QString lineNeigh = inNeigh.readLine().trimmed();
    int numberOfNeigh = lineNeigh.split(whiteChar).at(0).toInt();
    for (int i = 0; i < numberOfNeigh; i++)
    {
        QStringList parsedLine = inNeigh.readLine().trimmed().split(whiteChar);

        elementList[i].neigh[0] = parsedLine.at(1).toInt();;
        elementList[i].neigh[1] = parsedLine.at(2).toInt();;
        elementList[i].neigh[2] = parsedLine.at(3).toInt();;
    }

    fileNode.close();
    fileEdge.close();
    fileEle.close();
    fileNeigh.close();

    // heterogeneous mesh
    // element division
    if (Agros2D::problem()->config()->meshType() == MeshType_Triangle_QuadFineDivision)
    {
        for (int i = 0; i < edgeCountLinear; i++)
        {
            if (edgeList[i].marker != -1)
            {
                for (int j = 0; j < elementList.count() / 3; j++)
                {
                    for (int k = 0; k < 3; k++)
                    {
                        if (edgeList[i].node[0] == elementList[3*j + k].node[1] && edgeList[i].node[1] == elementList[3*j + (k + 1) % 3].node[1])
                        {
                            edgeList.append(MeshEdge(edgeList[i].node[0], elementList[3*j + (k + 1) % 3].node[0], edgeList[i].marker));
                            edgeList[i].node[0] = elementList[3*j + (k + 1) % 3].node[0];
                        }
                    }
                }
            }
        }
    }

    if (Agros2D::problem()->config()->meshType() == MeshType_Triangle_QuadRoughDivision)
    {
        for (int i = 0; i < elementCountLinear; i++)
        {
            // check same material
            if (elementList[i].isActive &&
                    (elementList[i].neigh[0] != -1) &&
                    (elementList[i].neigh[1] != -1) &&
                    (elementList[i].neigh[2] != -1) &&
                    elementList[elementList[i].neigh[0]].isActive &&
                    elementList[elementList[i].neigh[1]].isActive &&
                    elementList[elementList[i].neigh[2]].isActive &&
                    (elementList[i].marker == elementList[elementList[i].neigh[0]].marker) &&
                    (elementList[i].marker == elementList[elementList[i].neigh[1]].marker) &&
                    (elementList[i].marker == elementList[elementList[i].neigh[2]].marker))
            {
                // add additional node
                nodeList.append(Point((nodeList[elementList[i].node[0]].x + nodeList[elementList[i].node[1]].x + nodeList[elementList[i].node[2]].x) / 3.0,
                                      (nodeList[elementList[i].node[0]].y + nodeList[elementList[i].node[1]].y + nodeList[elementList[i].node[2]].y) / 3.0));

                // add three quad elements
                for (int nd = 0; nd < 3; nd++)
                    for (int neigh = 0; neigh < 3; neigh++)
                        for (int neigh_nd = 0; neigh_nd < 3; neigh_nd++)
                            if ((elementList[i].node[(nd + 0) % 3] == elementList[elementList[i].neigh[neigh]].node[(neigh_nd + 1) % 3]) &&
                                    (elementList[i].node[(nd + 1) % 3] == elementList[elementList[i].neigh[neigh]].node[(neigh_nd + 0) % 3]))
                                elementList.append(MeshElement(elementList[elementList[i].neigh[neigh]].node[(neigh_nd + 1) % 3],
                                                               elementList[elementList[i].neigh[neigh]].node[(neigh_nd + 2) % 3],
                                                               elementList[elementList[i].neigh[neigh]].node[(neigh_nd + 0) % 3],
                                                               nodeList.count() - 1, elementList[i].marker));

                elementList[i].isUsed = false;
                elementList[i].isActive = false;
                for (int k = 0; k < 3; k++)
                {
                    elementList[elementList[i].neigh[k]].isUsed = false;
                    elementList[elementList[i].neigh[k]].isActive = false;
                }
            }
        }
    }

    if (Agros2D::problem()->config()->meshType() == MeshType_Triangle_QuadJoin)
    {
        for (int i = 0; i < elementCountLinear; i++)
        {
            // check same material
            if (elementList[i].isActive)
            {
                // add quad elements
                for (int nd = 0; nd < 3; nd++)
                    for (int neigh = 0; neigh < 3; neigh++)
                        for (int neigh_nd = 0; neigh_nd < 3; neigh_nd++)
                            if (elementList[i].isActive &&
                                    elementList[i].neigh[neigh] != -1 &&
                                    elementList[elementList[i].neigh[neigh]].isActive &&
                                    elementList[i].marker == elementList[elementList[i].neigh[neigh]].marker)
                                if ((elementList[i].node[(nd + 0) % 3] == elementList[elementList[i].neigh[neigh]].node[(neigh_nd + 1) % 3]) &&
                                        (elementList[i].node[(nd + 1) % 3] == elementList[elementList[i].neigh[neigh]].node[(neigh_nd + 0) % 3]))
                                {
                                    int tmp_node[3];
                                    for (int k = 0; k < 3; k++)
                                        tmp_node[k] = elementList[i].node[k];

                                    Point quad_check[4];
                                    quad_check[0] = nodeList[tmp_node[(nd + 1) % 3]];
                                    quad_check[1] = nodeList[tmp_node[(nd + 2) % 3]];
                                    quad_check[2] = nodeList[tmp_node[(nd + 0) % 3]];
                                    quad_check[3] = nodeList[elementList[elementList[i].neigh[neigh]].node[(neigh_nd + 2) % 3]];

                                    if ((!Hermes::Hermes2D::Mesh::same_line(quad_check[0].x, quad_check[0].y, quad_check[1].x, quad_check[1].y, quad_check[2].x, quad_check[2].y)) &&
                                            (!Hermes::Hermes2D::Mesh::same_line(quad_check[0].x, quad_check[0].y, quad_check[1].x, quad_check[1].y, quad_check[3].x, quad_check[3].y)) &&
                                            (!Hermes::Hermes2D::Mesh::same_line(quad_check[0].x, quad_check[0].y, quad_check[2].x, quad_check[2].y, quad_check[3].x, quad_check[3].y)) &&
                                            (!Hermes::Hermes2D::Mesh::same_line(quad_check[1].x, quad_check[1].y, quad_check[2].x, quad_check[2].y, quad_check[3].x, quad_check[3].y)) &&
                                            Hermes::Hermes2D::Mesh::is_convex(quad_check[1].x - quad_check[0].x, quad_check[1].y - quad_check[0].y, quad_check[2].x - quad_check[0].x, quad_check[2].y - quad_check[0].y) &&
                                            Hermes::Hermes2D::Mesh::is_convex(quad_check[2].x - quad_check[0].x, quad_check[2].y - quad_check[0].y, quad_check[3].x - quad_check[0].x, quad_check[3].y - quad_check[0].y) &&
                                            Hermes::Hermes2D::Mesh::is_convex(quad_check[2].x - quad_check[1].x, quad_check[2].y - quad_check[1].y, quad_check[3].x - quad_check[1].x, quad_check[3].y - quad_check[1].y) &&
                                            Hermes::Hermes2D::Mesh::is_convex(quad_check[3].x - quad_check[1].x, quad_check[3].y - quad_check[1].y, quad_check[0].x - quad_check[1].x, quad_check[0].y - quad_check[1].y))
                                    {
                                        // regularity check
                                        bool regular = true;
                                        for (int k = 0; k < 4; k++)
                                        {
                                            double length_1 = (quad_check[k] - quad_check[(k + 1) % 4]).magnitude();
                                            double length_2 = (quad_check[(k + 1) % 4] - quad_check[(k + 2) % 4]).magnitude();
                                            double length_together = (quad_check[k] - quad_check[(k + 2) % 4]).magnitude();

                                            if ((length_1 + length_2) / length_together < 1.03)
                                                regular = false;
                                        }

                                        if (!regular)
                                            break;

                                        elementList[i].node[0] = tmp_node[(nd + 1) % 3];
                                        elementList[i].node[1] = tmp_node[(nd + 2) % 3];
                                        elementList[i].node[2] = tmp_node[(nd + 0) % 3];
                                        elementList[i].node[3] = elementList[elementList[i].neigh[neigh]].node[(neigh_nd + 2) % 3];

                                        elementList[i].isActive = false;

                                        elementList[elementList[i].neigh[neigh]].isUsed = false;
                                        elementList[elementList[i].neigh[neigh]].isActive = false;

                                        break;
                                    }
                                }
            }
        }
    }

    writeToHermes();

    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    return true;
}
