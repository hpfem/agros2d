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

#include "meshgenerator.h"

#include "gui.h"
#include "scene.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "sceneview_common.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "scenesolution.h"
#include "logview.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"

MeshGeneratorTriangle::MeshGeneratorTriangle() : QObject()
{    
}

bool MeshGeneratorTriangle::mesh()
{
    m_isError = false;

    QFile::remove(tempProblemFileName() + ".mesh");

    // create triangle files
    if (writeToTriangle())
    {
        Util::log()->printDebug(tr("Poly file was created"));

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

        processTriangle.start(QString(Util::config()->commandTriangle).
                              arg(triangleBinary).
                              arg(tempProblemFileName()));

        if (!processTriangle.waitForStarted(100000))
        {
            Util::log()->printError(tr("Could not start Triangle"));
            processTriangle.kill();

            m_isError = true;
        }
        else
        {
            // copy triangle files
            if ((!Util::config()->deleteTriangleMeshFiles) && (!Util::scene()->problemInfo()->fileName.isEmpty()))
            {
                QFileInfo fileInfoOrig(Util::scene()->problemInfo()->fileName);

                QFile::copy(tempProblemFileName() + ".poly", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".poly");
                QFile::copy(tempProblemFileName() + ".node", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".node");
                QFile::copy(tempProblemFileName() + ".edge", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".edge");
                QFile::copy(tempProblemFileName() + ".ele", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".ele");
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
        Util::log()->printMessage(tr("Mesh files were created"));

        // convert triangle mesh to hermes mesh
        if (triangleToHermes2D())
        {
            Util::log()->printMessage(tr("Mesh was converted to Hermes2D mesh file"));

            // copy triangle files
            if ((!Util::config()->deleteHermes2DMeshFile) && (!Util::scene()->problemInfo()->fileName.isEmpty()))
            {
                QFileInfo fileInfoOrig(Util::scene()->problemInfo()->fileName);

                QFile::copy(tempProblemFileName() + ".mesh", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".mesh");
            }

            //  remove triangle temp files
            QFile::remove(tempProblemFileName() + ".poly");
            QFile::remove(tempProblemFileName() + ".node");
            QFile::remove(tempProblemFileName() + ".edge");
            QFile::remove(tempProblemFileName() + ".ele");
            QFile::remove(tempProblemFileName() + ".neigh");
            QFile::remove(tempProblemFileName() + ".triangle.out");
            QFile::remove(tempProblemFileName() + ".triangle.err");
            Util::log()->printMessage(tr("Mesh files were deleted"));

            // load mesh
            Hermes::Hermes2D::Mesh *mesh = readMeshFromFile(tempProblemFileName() + ".xml");

            // TODO jinak
            Util::problem()->setMeshInitial(mesh);
        }
        else
        {
            m_isError = true;
            QFile::remove(Util::scene()->problemInfo()->fileName + ".mesh");
        }
    }
    else
    {
        m_isError = true;
        QString errorMessage = readFileContent(Util::scene()->problemInfo()->fileName + ".triangle.out");
        Util::log()->printError(errorMessage);
    }
}

bool MeshGeneratorTriangle::writeToTriangle()
{
    logMessage("ProgressItemMesh::writeToTriangle()");

    // basic check
    if (Util::scene()->nodes->length() < 3)
    {
        Util::log()->printError(tr("Invalid number of nodes (%1 < 3)").arg(Util::scene()->nodes->length()));
        return false;
    }
    if (Util::scene()->edges->length() < 3)
    {
        Util::log()->printError(tr("Invalid number of edges (%1 < 3)").arg(Util::scene()->edges->length()));
        return false;
    }
    else
    {
        // at least one boundary condition has to be assigned
        int count = 0;
        foreach (SceneEdge *edge, Util::scene()->edges->items())
            if (edge->markersCount() > 0)
                count++;

        if (count == 0)
        {
            Util::log()->printError(tr("At least one boundary condition has to be assigned"));
            return false;
        }
    }
    if (Util::scene()->labels->length() < 1)
    {
        Util::log()->printError(tr("Invalid number of labels (%1 < 1)").arg(Util::scene()->labels->length()));
        return false;
    }
    else
    {
        // at least one material has to be assigned
        int count = 0;
        foreach (SceneLabel *label, Util::scene()->labels->items())
            if (label->markersCount() > 0)
                count++;

        if (count == 0)
        {
            Util::log()->printError(tr("At least one material has to be assigned"));
            return false;
        }
    }
    if (Util::scene()->boundaries->length() < 2) // + none marker
    {
        Util::log()->printError(tr("Invalid number of boundary conditions (%1 < 1)").arg(Util::scene()->boundaries->length()));
        return false;
    }
    if (Util::scene()->materials->length() < 2) // + none marker
    {
        Util::log()->printError(tr("Invalid number of materials (%1 < 1)").arg(Util::scene()->materials->length()));
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
        Util::log()->printError(tr("Could not create Triangle poly mesh file (%1)").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);


    // nodes
    QString outNodes;
    int nodesCount = 0;
    for (int i = 0; i<Util::scene()->nodes->length(); i++)
    {
        outNodes += QString("%1  %2  %3  %4\n").
                arg(i).
                arg(Util::scene()->nodes->at(i)->point.x, 0, 'f', 10).
                arg(Util::scene()->nodes->at(i)->point.y, 0, 'f', 10).
                arg(0);
        nodesCount++;
    }

    // edges
    QString outEdges;
    int edgesCount = 0;
    for (int i = 0; i<Util::scene()->edges->length(); i++)
    {
        if (Util::scene()->edges->at(i)->angle == 0)
        {
            // line
            outEdges += QString("%1  %2  %3  %4\n").
                    arg(edgesCount).
                    arg(Util::scene()->nodes->items().indexOf(Util::scene()->edges->at(i)->nodeStart)).
                    arg(Util::scene()->nodes->items().indexOf(Util::scene()->edges->at(i)->nodeEnd)).
                    arg(i+1);
            edgesCount++;
        }
        else
        {
            // arc
            // add pseudo nodes
            Point center = Util::scene()->edges->at(i)->center();
            double radius = Util::scene()->edges->at(i)->radius();
            double startAngle = atan2(center.y - Util::scene()->edges->at(i)->nodeStart->point.y,
                                      center.x - Util::scene()->edges->at(i)->nodeStart->point.x) - M_PI;

            int segments = Util::scene()->edges->at(i)->segments();
            double theta = deg2rad(Util::scene()->edges->at(i)->angle) / double(segments);

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
                    nodeStartIndex = Util::scene()->nodes->items().indexOf(Util::scene()->edges->at(i)->nodeStart);
                    nodeEndIndex = nodesCount;
                }
                if (j == segments - 1)
                {
                    nodeEndIndex = Util::scene()->nodes->items().indexOf(Util::scene()->edges->at(i)->nodeEnd);
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
                    arg(holesCount + 1).
                    arg(label->point.x, 0, 'f', 10).
                    arg(label->point.y, 0, 'f', 10);

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
                    arg(label->point.x, 0, 'f', 10).
                    arg(label->point.y, 0, 'f', 10).
                    arg(labelsCount + 1). // triangle returns zero region number for areas without marker, markers must start from 1
                    arg(label->area);
            labelsCount++;
        }
    }

    outNodes.insert(0, QString("%1 2 0 1\n").
                    arg(nodesCount)); // + additional Util::scene()->nodes
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

bool MeshGeneratorTriangle::triangleToHermes2D()
{
    logMessage("ProgressItemMesh::triangleToHermes2D()");

    int n, k;

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    QDomDocument doc;
    QDomProcessingInstruction instr = doc.createProcessingInstruction("xml", "version='1.0' encoding='UTF-8' standalone='no'");
    doc.appendChild(instr);

    // main document
    QDomElement eleMesh = doc.createElement("mesh:mesh");
    eleMesh.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    eleMesh.setAttribute("xmlns:mesh", "XMLMesh");
    eleMesh.setAttribute("xmlns:element", "XMLMesh");
    eleMesh.setAttribute("xsi:schemaLocation", QString("XMLMesh %1/mesh_h2d_xml.xsd").arg(datadir() + "/resources/xsd"));
    doc.appendChild(eleMesh);

    QDomElement eleVertices = doc.createElement("vertices");
    eleMesh.appendChild(eleVertices);
    QDomElement eleElements = doc.createElement("elements");
    eleMesh.appendChild(eleElements);
    QDomElement eleEdges = doc.createElement("edges");
    eleMesh.appendChild(eleEdges);
    QDomElement eleCurves = doc.createElement("curves");
    eleMesh.appendChild(eleCurves);

    QFile fileNode(tempProblemFileName() + ".node");
    if (!fileNode.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Util::log()->printError(tr("Could not read Triangle node file"));
        return false;
    }
    QTextStream inNode(&fileNode);

    QFile fileEdge(tempProblemFileName() + ".edge");
    if (!fileEdge.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Util::log()->printError(tr("Could not read Triangle edge file"));
        return false;
    }
    QTextStream inEdge(&fileEdge);

    QFile fileEle(tempProblemFileName() + ".ele");
    if (!fileEle.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Util::log()->printError(tr("Could not read Triangle ele file"));
        return false;
    }
    QTextStream inEle(&fileEle);

    QFile fileNeigh(tempProblemFileName() + ".neigh");
    if (!fileNeigh.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Util::log()->printError(tr("Could not read Triangle neigh file"));
        return false;
    }
    QTextStream inNeigh(&fileNeigh);

    // triangle nodes
    sscanf(inNode.readLine().toStdString().c_str(), "%i", &k);
    QList<Point> nodeList;
    for (int i = 0; i<k; i++)
    {
        int marker;
        double x, y;

        sscanf(inNode.readLine().toStdString().c_str(), "%i   %lf %lf %i", &n, &x, &y, &marker);
        nodeList.append(Point(x, y));
    }

    // triangle edges
    sscanf(inEdge.readLine().toStdString().c_str(), "%i", &k);
    QList<MeshEdge> edgeList;
    QSet<int> edgeMarkersCheck;
    for (int i = 0; i<k; i++)
    {
        int node[2];
        int marker;

        sscanf(inEdge.readLine().toStdString().c_str(), "%i	%i	%i	%i", &n, &node[0], &node[1], &marker);
        edgeList.append(MeshEdge(node[0], node[1], marker));
    }
    int edgeCountLinear = edgeList.count();

    FieldInfo *fieldInfoTMP;
    foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
    {
        fieldInfoTMP = fieldInfo;
    }

    // check for multiple edge markers
    foreach (SceneEdge *edge, Util::scene()->edges->items())
    {
        // if (Util::scene()->boundaries.indexOf(edge->getMarker(fieldInfoTMP) > 0)
        {
            // if (!edgeMarkersCheck.contains(i))
            {
                // emit message(tr("Edge marker '%1' is not present in mesh file.").
                //              arg(i), true, 0);
                // return false;
            }
        }
    }
    // edgeMarkersCheck.clear();

    // no edge marker
    if (edgeCountLinear < 1)
    {
        Util::log()->printError(tr("Invalid number of edge markers"));
        return false;
    }

    // triangle elements
    sscanf(inEle.readLine().toStdString().c_str(), "%i", &k);
    QList<MeshElement> elementList;
    QSet<int> labelMarkersCheck;
    for (int i = 0; i < k; i++)
    {
        int node[6];
        int marker;

        sscanf(inEle.readLine().toStdString().c_str(), "%i	%i	%i	%i	%i	%i	%i	%i",
               &n, &node[0], &node[1], &node[2], &node[3], &node[4], &node[5], &marker);

        if (Util::scene()->problemInfo()->meshType == MeshType_Triangle ||
                Util::scene()->problemInfo()->meshType == MeshType_QuadJoin ||
                Util::scene()->problemInfo()->meshType == MeshType_QuadRoughDivision)
        {
            elementList.append(MeshElement(node[0], node[1], node[2], marker));
        }

        if (Util::scene()->problemInfo()->meshType == MeshType_QuadFineDivision)
        {
            // add additional node
            nodeList.append(Point((nodeList[node[0]].x + nodeList[node[1]].x + nodeList[node[2]].x) / 3.0,
                                  (nodeList[node[0]].y + nodeList[node[1]].y + nodeList[node[2]].y) / 3.0));
            // add three quad elements
            elementList.append(MeshElement(node[4], node[0], node[5], nodeList.count() - 1, marker));
            elementList.append(MeshElement(node[5], node[1], node[3], nodeList.count() - 1, marker));
            elementList.append(MeshElement(node[3], node[2], node[4], nodeList.count() - 1, marker));
        }

        if (elementList[i].marker == 0)
        {
            Util::log()->printError(tr("Some areas have no label marker"));
            return false;
        }

        labelMarkersCheck.insert(marker);
    }
    int elementCountLinear = elementList.count();

    // check for multiple label markers
    foreach (SceneLabel *label, Util::scene()->labels->items())
    {
        // if (Util::scene()->materials.indexOf(Util::scene()->labels[i]->material) > 0)
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

    // no label marker
    if (elementList.count() < 1)
    {
        Util::log()->printError(tr("Invalid number of label markers"));
        return false;
    }

    // triangle neigh
    sscanf(inNeigh.readLine().toStdString().c_str(), "%i", &k);
    for (int i = 0; i < k; i++)
    {
        int ele_1, ele_2, ele_3;

        sscanf(inNeigh.readLine().toStdString().c_str(), "%i	%i	%i	%i", &n, &ele_1, &ele_2, &ele_3);
        elementList[i].neigh[0] = ele_1;
        elementList[i].neigh[1] = ele_2;
        elementList[i].neigh[2] = ele_3;
    }

    // heterogeneous mesh
    // element division
    if (Util::scene()->problemInfo()->meshType == MeshType_QuadFineDivision)
    {
        for (int i = 0; i < edgeCountLinear; i++)
        {
            if (edgeList[i].marker != 0)
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

    if (Util::scene()->problemInfo()->meshType == MeshType_QuadRoughDivision)
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

    if (Util::scene()->problemInfo()->meshType == MeshType_QuadJoin)
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

    // edges
    int countEdges = 0;
    for (int i = 0; i < edgeList.count(); i++)
    {
        if (edgeList[i].isUsed && edgeList[i].marker != 0)
        {
            //TODO - no "inner edges" in new xml mesh file format - remove?
            // inner edge marker (minus markers are ignored)
            int marker = - (edgeList[i].marker-1);
            foreach (FieldInfo *fieldInfo, Util::scene()->fieldInfos())
                if (Util::scene()->edges->at(edgeList[i].marker-1)->getMarker(fieldInfoTMP)
                        != SceneBoundaryContainer::getNone(fieldInfoTMP))
                {
                    // boundary marker
                    marker = edgeList[i].marker;

                    break;
                }

            countEdges++;

            QDomElement eleEdge = doc.createElement("edge");
            eleEdge.setAttribute("v1", edgeList[i].node[0]);
            eleEdge.setAttribute("v2", edgeList[i].node[1]);
            eleEdge.setAttribute("marker", marker);

            eleEdges.appendChild(eleEdge);
        }
    }

    // curves
    int countCurves = 0;
    if (Util::config()->curvilinearElements)
    {
        for (int i = 0; i<edgeList.count(); i++)
        {
            if (edgeList[i].marker != 0)
            {
                // curve
                if (Util::scene()->edges->at(edgeList[i].marker-1)->angle > 0.0)
                {
                    countCurves++;
                    int segments = Util::scene()->edges->at(edgeList[i].marker-1)->segments();

                    // subdivision angle and chord
                    double theta = deg2rad(Util::scene()->edges->at(edgeList[i].marker-1)->angle) / double(segments);
                    double chord = 2 * Util::scene()->edges->at(edgeList[i].marker-1)->radius() * sin(theta / 2.0);

                    // length of short chord
                    double chordShort = (nodeList[edgeList[i].node[1]] - nodeList[edgeList[i].node[0]]).magnitude();

                    // direction
                    Point center = Util::scene()->edges->at(edgeList[i].marker-1)->center();
                    int direction = (((nodeList[edgeList[i].node[0]].x-center.x)*(nodeList[edgeList[i].node[1]].y-center.y) -
                                      (nodeList[edgeList[i].node[0]].y-center.y)*(nodeList[edgeList[i].node[1]].x-center.x)) > 0) ? 1 : -1;

                    double angle = direction * theta * chordShort / chord;

                    QDomElement eleArc = doc.createElement("arc");
                    eleArc.setAttribute("v1", edgeList[i].node[0]);
                    eleArc.setAttribute("v2", edgeList[i].node[1]);
                    eleArc.setAttribute("angle", QString("%1").arg(rad2deg(angle)));

                    eleCurves.appendChild(eleArc);
                }
            }
        }

        // move nodes (arcs)
        for (int i = 0; i<edgeList.count(); i++)
        {
            if (edgeList[i].marker != 0)
            {
                // curve
                if (Util::scene()->edges->at(edgeList[i].marker-1)->angle > 0.0)
                {
                    // angle
                    Point center = Util::scene()->edges->at(edgeList[i].marker-1)->center();
                    double pointAngle1 = atan2(center.y - nodeList[edgeList[i].node[0]].y,
                                               center.x - nodeList[edgeList[i].node[0]].x) - M_PI;

                    double pointAngle2 = atan2(center.y - nodeList[edgeList[i].node[1]].y,
                                               center.x - nodeList[edgeList[i].node[1]].x) - M_PI;

                    nodeList[edgeList[i].node[0]].x = center.x + Util::scene()->edges->at(edgeList[i].marker-1)->radius() * cos(pointAngle1);
                    nodeList[edgeList[i].node[0]].y = center.y + Util::scene()->edges->at(edgeList[i].marker-1)->radius() * sin(pointAngle1);

                    nodeList[edgeList[i].node[1]].x = center.x + Util::scene()->edges->at(edgeList[i].marker-1)->radius() * cos(pointAngle2);
                    nodeList[edgeList[i].node[1]].y = center.y + Util::scene()->edges->at(edgeList[i].marker-1)->radius() * sin(pointAngle2);
                }
            }
        }
    }

    // nodes
    for (int i = 0; i<nodeList.count(); i++)
    {
        QDomElement eleVertex = doc.createElement("vertex");
        eleVertex.setAttribute("i", i);
        eleVertex.setAttribute("x", QString("%1").arg(nodeList[i].x));
        eleVertex.setAttribute("y", QString("%1").arg(nodeList[i].y));

        eleVertices.appendChild(eleVertex);
    }

    // elements
    for (int i = 0; i<elementList.count(); i++)
    {
        if (elementList[i].isUsed)
        {
            QDomElement eleElement = doc.createElement(QString("element:%1").arg(elementList[i].isTriangle() ? "triangle" : "quad"));
            eleElement.setAttribute("v1", elementList[i].node[0]);
            eleElement.setAttribute("v2", elementList[i].node[1]);
            eleElement.setAttribute("v3", elementList[i].node[2]);
            if (!elementList[i].isTriangle())
                eleElement.setAttribute("v4", elementList[i].node[3]);
            eleElement.setAttribute("marker", QString("%1").arg(abs(elementList[i].marker) - 1));

            eleElements.appendChild(eleElement);
        }
    }

    // subdomains
    //TODO

    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    fileNode.close();
    fileEdge.close();
    fileEle.close();
    fileNeigh.close();

    // save to file
    QFile file(tempProblemFileName() + ".xml");
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QTextStream out(&file);
    doc.save(out, 4);

    file.waitForBytesWritten(0);
    file.close();

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return true;
}
