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

#include "progressdialog.h"

#include "gui.h"
#include "scene.h"
#include "scenebasic.h"
#include "sceneview.h"
#include "scenemarker.h"
#include "scenesolution.h"
#include "hermes2d/module.h"


ProgressItem::ProgressItem()
{
    logMessage("ProgressItem::ProgressItem()");

    m_name = "";

    connect(this, SIGNAL(message(QString, bool, int)), this, SLOT(showMessage(QString, bool, int)));
}

void ProgressItem::init()
{
    m_steps = 0;
    m_isError = false;
    m_isCanceled = false;
}

void ProgressItem::showMessage(const QString &msg, bool isError, int position)
{
    logMessage("ProgressItem::()");

    m_isError = isError;
}

// *********************************************************************************************

ProgressItemMesh::ProgressItemMesh() : ProgressItem()
{
    logMessage("ProgressItemMesh::ProgressItemMesh()");

    m_name = tr("Mesh");
}

void ProgressItemMesh::setSteps()
{
    m_steps = 4;
}

bool ProgressItemMesh::run(bool quiet)
{
    logMessage("ProgressItemMesh::run()");

    if (quiet) blockSignals(true);
    mesh();
    if (quiet) blockSignals(false);

    return !m_isError;
}

void ProgressItemMesh::mesh()
{
    logMessage("ProgressItemMesh::run()");

    QFile::remove(tempProblemFileName() + ".mesh");

    // create triangle files
    if (writeToTriangle())
    {
        emit message(tr("Poly file was created"), false, 1);

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

        if (!processTriangle.waitForStarted())
        {
            emit message(tr("Could not start Triangle"), true, 0);
            processTriangle.kill();

            m_isError = true;
            return;
        }

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
    else
    {
        m_isError = true;
    }
}

void ProgressItemMesh::meshTriangleCreated(int exitCode)
{
    logMessage("ProgressItemMesh::meshTriangleCreated()");

    if (exitCode == 0)
    {
        emit message(tr("Mesh files were created"), false, 2);

        // convert triangle mesh to hermes mesh
        if (triangleToHermes2D())
        {
            emit message(tr("Mesh was converted to Hermes2D mesh file"), false, 3);

            // copy triangle files
            if ((!Util::config()->deleteHermes2DMeshFile) && (!Util::scene()->problemInfo()->fileName.isEmpty()))
            {
                QFileInfo fileInfoOrig(Util::scene()->problemInfo()->fileName);

                QFile::copy(tempProblemFileName() + ".mesh", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".mesh");
            }

            //  remove triangle temp files
            // QFile::remove(tempProblemFileName() + ".poly");
            // QFile::remove(tempProblemFileName() + ".node");
            // QFile::remove(tempProblemFileName() + ".edge");
            // QFile::remove(tempProblemFileName() + ".ele");
            // QFile::remove(tempProblemFileName() + ".neigh");
            // QFile::remove(tempProblemFileName() + ".triangle.out");
            // QFile::remove(tempProblemFileName() + ".triangle.err");
            emit message(tr("Mesh files were deleted"), false, 4);

            // load mesh
            Hermes::Hermes2D::Mesh *mesh = readMeshFromFile(tempProblemFileName() + ".xml");

            // check that all boundary edges have a marker assigned
            for (int i = 0; i < mesh->get_max_node_id(); i++)
            {
                if (Hermes::Hermes2D::Node *node = mesh->get_node(i))
                {
                    if (node->used == 1 && node->type == 1 && node->ref < 2 && node->marker == 0)
                    {
                        qDebug() << "p1: " << node->p1 << "p2: " << node->p2;
                        emit message(tr("Boundary edge does not have a boundary marker"), true, 0);

                        delete mesh;
                        m_isError = true;
                        return;
                    }
                }
            }

            refineMesh(mesh, true, true);

            Util::scene()->sceneSolution()->setMeshInitial(mesh);
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
        emit message(errorMessage, true, 0);
    }
}

bool ProgressItemMesh::writeToTriangle()
{
    logMessage("ProgressItemMesh::writeToTriangle()");

    // basic check
    if (Util::scene()->nodes.count() < 3)
    {
        emit message(tr("Invalid number of nodes (%1 < 3)").arg(Util::scene()->nodes.count()), true, 0);
        return false;
    }
    if (Util::scene()->edges.count() < 3)
    {
        emit message(tr("Invalid number of edges (%1 < 3)").arg(Util::scene()->edges.count()), true, 0);
        return false;
    }
    else
    {
        // at least one boundary condition has to be assigned
        int count = 0;
        for (int i = 0; i<Util::scene()->edges.count(); i++)
            if (Util::scene()->boundaries.indexOf(Util::scene()->edges[i]->boundary) > 0)
                count++;

        if (count == 0)
        {
            emit message(tr("At least one boundary condition has to be assigned"), true, 0);
            return false;
        }
    }
    if (Util::scene()->labels.count() < 1)
    {
        emit message(tr("Invalid number of labels (%1 < 1)").arg(Util::scene()->labels.count()), true, 0);
        return false;
    }
    else
    {
        // at least one material has to be assigned
        int count = 0;
        for (int i = 0; i<Util::scene()->labels.count(); i++)
            if (Util::scene()->materials.indexOf(Util::scene()->labels[i]->material) > 0)
                count++;

        if (count == 0)
        {
            emit message(tr("At least one material has to be assigned"), true, 0);
            return false;
        }
    }
    if (Util::scene()->boundaries.count() < 2) // + none marker
    {
        emit message(tr("Invalid number of boundary conditions (%1 < 1)").arg(Util::scene()->boundaries.count()), true, 0);
        return false;
    }
    if (Util::scene()->materials.count() < 2) // + none marker
    {
        emit message(tr("Invalid number of materials (%1 < 1)").arg(Util::scene()->materials.count()), true, 0);
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
        emit message(tr("Could not create Triangle poly mesh file (%1)").arg(file.errorString()), true, 0);
        return false;
    }
    QTextStream out(&file);


    // nodes
    QString outNodes;
    int nodesCount = 0;
    for (int i = 0; i<Util::scene()->nodes.count(); i++)
    {
        outNodes += QString("%1  %2  %3  %4\n").
                arg(i).
                arg(Util::scene()->nodes[i]->point.x, 0, 'f', 10).
                arg(Util::scene()->nodes[i]->point.y, 0, 'f', 10).
                arg(0);
        nodesCount++;
    }

    // edges
    QString outEdges;
    int edgesCount = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edges[i]->angle == 0)
        {
            // line
            outEdges += QString("%1  %2  %3  %4\n").
                    arg(edgesCount).
                    arg(Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeStart)).
                    arg(Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeEnd)).
                    arg(i+1);
            edgesCount++;
        }
        else
        {
            // arc
            // add pseudo nodes
            Point center = Util::scene()->edges[i]->center();
            double radius = Util::scene()->edges[i]->radius();
            double startAngle = atan2(center.y - Util::scene()->edges[i]->nodeStart->point.y,
                                      center.x - Util::scene()->edges[i]->nodeStart->point.x) - M_PI;

            int segments = Util::scene()->edges[i]->segments();
            double theta = deg2rad(Util::scene()->edges[i]->angle) / double(segments);

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
                    nodeStartIndex = Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeStart);
                    nodeEndIndex = nodesCount;
                }
                if (j == segments - 1)
                {
                    nodeEndIndex = Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeEnd);
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
    for (int i = 0; i<Util::scene()->labels.count(); i++) if (Util::scene()->materials.indexOf(Util::scene()->labels[i]->material) == 0) holesCount++;
    QString outHoles = QString("%1\n").arg(holesCount);
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->materials.indexOf(Util::scene()->labels[i]->material) == 0)
        {
            outHoles += QString("%1  %2  %3\n").
                    arg(i).
                    arg(Util::scene()->labels[i]->point.x, 0, 'f', 10).
                    arg(Util::scene()->labels[i]->point.y, 0, 'f', 10);
        }
    }

    // labels
    QString outLabels;
    int labelsCount = 0;
    for(int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->materials.indexOf(Util::scene()->labels[i]->material) > 0)
        {
            outLabels += QString("%1  %2  %3  %4  %5\n").
                    arg(labelsCount).
                    arg(Util::scene()->labels[i]->point.x, 0, 'f', 10).
                    arg(Util::scene()->labels[i]->point.y, 0, 'f', 10).
                    arg(i + 1). // triangle returns zero region number for areas without marker, markers must start from 1
                    arg(Util::scene()->labels[i]->area);
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

bool ProgressItemMesh::triangleToHermes2D()
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
    eleMesh.setAttribute("xsi:schemaLocation", QString("XMLMesh %1/mesh_h2d_xml.xsd").arg(datadir() + "/resources"));
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
        emit message(tr("Could not read Triangle node file"), true, 0);
        return false;
    }
    QTextStream inNode(&fileNode);

    QFile fileEdge(tempProblemFileName() + ".edge");
    if (!fileEdge.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit message(tr("Could not read Triangle edge file"), true, 0);
        return false;
    }
    QTextStream inEdge(&fileEdge);

    QFile fileEle(tempProblemFileName() + ".ele");
    if (!fileEle.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit message(tr("Could not read Triangle ele file"), true, 0);
        return false;
    }
    QTextStream inEle(&fileEle);

    QFile fileNeigh(tempProblemFileName() + ".neigh");
    if (!fileNeigh.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit message(tr("Could not read Triangle neigh file"), true, 0);
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

    // check for multiple edge markers
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->boundaries.indexOf(Util::scene()->edges[i]->boundary) > 0)
        {
            if (!edgeMarkersCheck.contains(i))
            {
                // emit message(tr("Edge marker '%1' is not present in mesh file.").
                //              arg(i), true, 0);
                // return false;
            }
        }
    }
    edgeMarkersCheck.clear();

    // no edge marker
    if (edgeCountLinear < 1)
    {
        emit message(tr("Invalid number of edge markers"), true, 0);
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
            emit message(tr("Some areas have no label marker"), true, 0);
            return false;
        }

        labelMarkersCheck.insert(marker);
    }
    int elementCountLinear = elementList.count();

    // check for multiple label markers
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->materials.indexOf(Util::scene()->labels[i]->material) > 0)
        {
            if (!labelMarkersCheck.contains(i + 1))
            {
                emit message(tr("Label marker '%1' is not present in mesh file (multiple label markers in one area).").
                             arg(i), true, 0);
                return false;
            }
        }
    }
    labelMarkersCheck.clear();

    // no label marker
    if (elementList.count() < 1)
    {
        emit message(tr("Invalid number of label markers"), true, 0);
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
            int marker = 0;
            if (Util::scene()->edges[edgeList[i].marker-1]->boundary->type != "")
                // boundary marker
                marker = edgeList[i].marker;
            else
                // inner edge marker (minus markers are ignored)
                marker = - (edgeList[i].marker-1);

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
                if (Util::scene()->edges[edgeList[i].marker-1]->angle > 0.0)
                {
                    countCurves++;
                    int segments = Util::scene()->edges[edgeList[i].marker-1]->segments();

                    // subdivision angle and chord
                    double theta = deg2rad(Util::scene()->edges[edgeList[i].marker-1]->angle) / double(segments);
                    double chord = 2 * Util::scene()->edges[edgeList[i].marker-1]->radius() * sin(theta / 2.0);

                    // length of short chord
                    double chordShort = (nodeList[edgeList[i].node[1]] - nodeList[edgeList[i].node[0]]).magnitude();

                    // direction
                    Point center = Util::scene()->edges[edgeList[i].marker-1]->center();
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
                if (Util::scene()->edges[edgeList[i].marker-1]->angle > 0.0)
                {
                    // angle
                    Point center = Util::scene()->edges[edgeList[i].marker-1]->center();
                    double pointAngle1 = atan2(center.y - nodeList[edgeList[i].node[0]].y,
                                               center.x - nodeList[edgeList[i].node[0]].x) - M_PI;

                    double pointAngle2 = atan2(center.y - nodeList[edgeList[i].node[1]].y,
                                               center.x - nodeList[edgeList[i].node[1]].x) - M_PI;

                    nodeList[edgeList[i].node[0]].x = center.x + Util::scene()->edges[edgeList[i].marker-1]->radius() * cos(pointAngle1);
                    nodeList[edgeList[i].node[0]].y = center.y + Util::scene()->edges[edgeList[i].marker-1]->radius() * sin(pointAngle1);

                    nodeList[edgeList[i].node[1]].x = center.x + Util::scene()->edges[edgeList[i].marker-1]->radius() * cos(pointAngle2);
                    nodeList[edgeList[i].node[1]].y = center.y + Util::scene()->edges[edgeList[i].marker-1]->radius() * sin(pointAngle2);
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

// *********************************************************************************************

ProgressItemSolve::ProgressItemSolve() : ProgressItem()
{
    logMessage("ProgressItemSolve::ProgressItemSolve()");

    m_name = tr("Solver");
}

void ProgressItemSolve::setSteps()
{
    m_steps = 1;
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        Util::scene()->problemInfo()->timeTotal.evaluate();
        Util::scene()->problemInfo()->timeStep.evaluate();

        m_steps += floor(Util::scene()->problemInfo()->timeTotal.number() / Util::scene()->problemInfo()->timeStep.number());
    }
}

bool ProgressItemSolve::run(bool quiet)
{
    logMessage("ProgressItemSolve::()");

    if (quiet) blockSignals(true);
    solve();
    if (quiet) blockSignals(false);

    return !m_isError;
}

void ProgressItemSolve::solve()
{
    logMessage("ProgressItemSolve::solve()");

    m_adaptivityError.clear();
    m_adaptivityDOF.clear();
    m_nonlinearError.clear();

    if (!QFile::exists(tempProblemFileName() + ".xml"))
        return;

    // benchmark
    QTime time;
    time.start();

    emit message(tr("Problem analysis: %1 (%2, %3)").
                 arg(QString::fromStdString(Util::scene()->problemInfo()->module()->name)).
                 arg(problemTypeString(Util::scene()->problemInfo()->problemType)).
                 arg(analysisTypeString(Util::scene()->problemInfo()->analysisType)), false, 1);

    emit message(tr("Solver was started: %1 ").arg(matrixSolverTypeString(Util::scene()->problemInfo()->matrixSolver)), false, 1);

    Hermes::vector<SolutionArray<double> *> solutionArrayList = Util::scene()->problemInfo()->module()->solve(this);  //TODO PK <double>

    if (solutionArrayList.size() > 0)
    {
        emit message(tr("Problem was solved"), false, 2);
        Util::scene()->sceneSolution()->setTimeElapsed(time.elapsed());
    }
    else
    {
        emit message(tr("Problem was not solved"), true, 0);
        Util::scene()->sceneSolution()->setTimeElapsed(0);
    }

    Util::scene()->sceneSolution()->setSolutionArrayList(solutionArrayList);
}

// *********************************************************************************************

ProgressItemSolveAdaptiveStep::ProgressItemSolveAdaptiveStep() : ProgressItemSolve()
{
    logMessage("ProgressItemSolve::ProgressItemSolveAdaptive()");

    m_name = tr("Adapt. solver");
}

void ProgressItemSolveAdaptiveStep::solve()
{
    logMessage("ProgressItemSolveAdaptive::solve()");

    // benchmark
    QTime time;
    time.start();

    emit message(tr("Problem analysis: %1 (%2, %3)").
                 arg(QString::fromStdString(Util::scene()->problemInfo()->module()->name)).
                 arg(problemTypeString(Util::scene()->problemInfo()->problemType)).
                 arg(analysisTypeString(Util::scene()->problemInfo()->analysisType)), false, 1);

    emit message(tr("Solver was started: %1 ").arg(matrixSolverTypeString(Util::scene()->problemInfo()->matrixSolver)), false, 1);

    Hermes::vector<SolutionArray<double> *> solutionArrayList = Util::scene()->problemInfo()->module()->solveAdaptiveStep(this);  //TODO PK <double>

    if (solutionArrayList.size() > 0)
    {
        emit message(tr("Problem was solved"), false, 2);
        Util::scene()->sceneSolution()->setTimeElapsed(time.elapsed());
    }
    else
    {
        emit message(tr("Problem was not solved"), true, 0);
        Util::scene()->sceneSolution()->setTimeElapsed(0);
    }

    Util::scene()->sceneSolution()->setSolutionArrayList(solutionArrayList);
}

// *********************************************************************************************

ProgressItemProcessView::ProgressItemProcessView() : ProgressItem()
{
    logMessage("ProgressItemProcessView::ProgressItemProcessView()");

    m_name = tr("View");
}

void ProgressItemProcessView::setSteps()
{
    m_steps = 0;
    if (sceneView()->sceneViewSettings().showContours == 1)
        m_steps += 1;
    if (sceneView()->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView ||
            sceneView()->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView3D ||
            sceneView()->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid)
        m_steps += 1;
    if (sceneView()->sceneViewSettings().showVectors == 1)
        m_steps += 1;
}

bool ProgressItemProcessView::run(bool quiet)
{
    logMessage("ProgressItemProcessView::run()");

    if (quiet) blockSignals(true);
    process();
    if (quiet) blockSignals(false);

    return !m_isError;
}

void ProgressItemProcessView::process()
{
    logMessage("ProgressItemProcessView::process()");

    int step = 0;

    // process order
    Util::scene()->sceneSolution()->processOrder();

    if (sceneView()->sceneViewSettings().showSolutionMesh)
    {
        step++;
        emit message(tr("Processing solution mesh cache"), false, step);
        Util::scene()->sceneSolution()->processSolutionMesh();
    }
    if (sceneView()->sceneViewSettings().showContours)
    {
        step++;
        emit message(tr("Processing contour view cache"), false, step);
        Util::scene()->sceneSolution()->processRangeContour();
    }
    if (sceneView()->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView ||
            sceneView()->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView3D ||
            sceneView()->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid)
    {
        step++;
        emit message(tr("Processing scalar view cache"), false, step);
        Util::scene()->sceneSolution()->processRangeScalar();
    }
    if (sceneView()->sceneViewSettings().showVectors)
    {
        step++;
        emit message(tr("Processing vector view cache"), false, step);
        Util::scene()->sceneSolution()->processRangeVector();
    }
}

// ***********************************************************************************************

ProgressDialog::ProgressDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("ProgressDialog::ProgressDialog()");

    setWindowModality(Qt::ApplicationModal);
    setModal(true);

    setWindowIcon(icon("run"));
    setWindowTitle(tr("Progress..."));

    createControls();

    setMinimumSize(550, 360);
    setMaximumSize(minimumSize());

    QSettings settings;
    restoreGeometry(settings.value("ProgressDialog/Geometry", saveGeometry()).toByteArray());
}

ProgressDialog::~ProgressDialog()
{
    logMessage("ProgressDialog::~ProgressDialog()");

    if (Util::config()->enabledProgressLog)
        saveProgressLog();
}

void ProgressDialog::clear()
{
    logMessage("ProgressDialog::clear()");

    // progress items
    m_progressItem.clear();
    m_currentProgressItem = NULL;

    // convergence charts
    curveError->setData(0);
    curveErrorMax->setData(0);
    curveDOF->setData(0);
    curveErrorDOF->setData(0);
    curveErrorDOFMax->setData(0);

    m_showViewProgress = true;
}

void ProgressDialog::createControls()
{
    logMessage("ProgressDialog::createControls()");

    controlsProgress = createControlsProgress();
    controlsConvergenceErrorChart = createControlsConvergenceErrorChart();
    controlsConvergenceDOFChart = createControlsConvergenceDOFChart();
    controlsConvergenceErrorDOFChart = createControlsConvergenceErrorDOFChart();
    controlsNonlinear = createControlsNonlinear();

    tabType = new QTabWidget();
    tabType->addTab(controlsProgress, icon(""), tr("Progress"));
    tabType->addTab(controlsNonlinear, icon(""), tr("Nonlin. error"));
    tabType->addTab(controlsConvergenceErrorChart, icon(""), tr("Adapt. error"));
    tabType->addTab(controlsConvergenceDOFChart, icon(""), tr("Adapt. DOFs"));
    tabType->addTab(controlsConvergenceErrorDOFChart, icon(""), tr("Adapt. conv."));
    connect(tabType, SIGNAL(currentChanged(int)), this, SLOT(resetControls(int)));

    btnCancel = new QPushButton(tr("Cance&l"));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(this, SIGNAL(finished(int)), this, SLOT(cancel()));

    btnClose = new QPushButton(tr("&Close"));
    btnClose->setDefault(true);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

    btnSaveImage = new QPushButton(tr("&Save image"));
    btnSaveImage->setDisabled(true);;
    connect(btnSaveImage, SIGNAL(clicked()), this, SLOT(saveImage()));

    btnSaveData = new QPushButton(tr("Save data"));
    btnSaveData->setDisabled(true);;
    connect(btnSaveData, SIGNAL(clicked()), this, SLOT(saveData()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(btnClose, QDialogButtonBox::RejectRole);
    buttonBox->addButton(btnCancel, QDialogButtonBox::RejectRole);
    buttonBox->addButton(btnSaveImage, QDialogButtonBox::RejectRole);
    buttonBox->addButton(btnSaveData, QDialogButtonBox::RejectRole);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabType);
    layout->addWidget(buttonBox);

    setLayout(layout);
}

QWidget *ProgressDialog::createControlsProgress()
{
    logMessage("ProgressDialog::createControlsProgress()");

    progressBar = new QProgressBar(this);
    lblMessage = new QLabel("", this);
    lstMessage = new QTextEdit(this);
    lstMessage->setReadOnly(true);

    QVBoxLayout *layoutProgress = new QVBoxLayout();
    layoutProgress->addWidget(lblMessage);
    layoutProgress->addWidget(progressBar);
    layoutProgress->addWidget(lstMessage);

    QWidget *widProgress = new QWidget();
    widProgress->setLayout(layoutProgress);

    return widProgress;
}

QWidget *ProgressDialog::createControlsConvergenceErrorChart()
{
    logMessage("ProgressDialog::createControlsConvergenceErrorChart()");

    chartError = new Chart(this);

    // curves
    curveError = new QwtPlotCurve();
    curveError->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveError->setPen(QPen(Qt::blue));
    curveError->setCurveAttribute(QwtPlotCurve::Inverted);
    curveError->setYAxis(QwtPlot::yLeft);
    curveError->setTitle(tr("current error"));
    curveError->attach(chartError);

    // curves
    curveErrorMax = new QwtPlotCurve();
    curveErrorMax->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveErrorMax->setPen(QPen(Qt::red));
    curveErrorMax->setCurveAttribute(QwtPlotCurve::Inverted);
    curveErrorMax->setYAxis(QwtPlot::yLeft);
    curveErrorMax->setTitle(tr("max. error"));
    curveErrorMax->attach(chartError);

    // labels
    QwtText textErrorLeft(tr("Error (%)"));
    textErrorLeft.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartError->setAxisTitle(QwtPlot::yLeft, textErrorLeft);

    QwtText textErrorBottom(tr("Steps (-)"));
    textErrorBottom.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartError->setAxisTitle(QwtPlot::xBottom, textErrorBottom);

    // legend
    /*
    QwtLegend *legend = new QwtLegend();
    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
    chart->insertLegend(legend, QwtPlot::BottomLegend);
    */

    QVBoxLayout *layoutConvergenceChart = new QVBoxLayout();
    layoutConvergenceChart->addWidget(chartError);

    QWidget *widConvergenceChart = new QWidget();
    widConvergenceChart->setLayout(layoutConvergenceChart);

    return widConvergenceChart;
}

QWidget *ProgressDialog::createControlsConvergenceDOFChart()
{
    logMessage("ProgressDialog::createControlsConvergenceDOFChart()");

    chartDOF = new Chart(this);

    // curves
    curveDOF = new QwtPlotCurve();
    curveDOF->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveDOF->setPen(QPen(Qt::blue));
    curveDOF->setCurveAttribute(QwtPlotCurve::Inverted);
    curveDOF->setYAxis(QwtPlot::yLeft);
    curveDOF->setTitle(tr("current error"));
    curveDOF->attach(chartDOF);

    // labels
    QwtText textDOFLeft(tr("DOFs (-)"));
    textDOFLeft.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartDOF->setAxisTitle(QwtPlot::yLeft, textDOFLeft);

    QwtText textDOFBottom(tr("Steps (-)"));
    textDOFBottom.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartDOF->setAxisTitle(QwtPlot::xBottom, textDOFBottom);

    QVBoxLayout *layoutConvergenceChart = new QVBoxLayout();
    layoutConvergenceChart->addWidget(chartDOF);

    QWidget *widConvergenceChart = new QWidget();
    widConvergenceChart->setLayout(layoutConvergenceChart);

    return widConvergenceChart;
}

QWidget *ProgressDialog::createControlsConvergenceErrorDOFChart()
{
    logMessage("ProgressDialog::createControlsConvergenceErrorDOFChart()");

    chartErrorDOF = new Chart(this);
    //    chartErrorDOF->setAxisScaleEngine(0, new QwtLog10ScaleEngine);
    //    chartErrorDOF->setAxisScaleEngine(2, new QwtLog10ScaleEngine);

    // curves
    curveErrorDOF = new QwtPlotCurve();
    curveErrorDOF->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveErrorDOF->setPen(QPen(Qt::blue));
    curveErrorDOF->setCurveAttribute(QwtPlotCurve::Inverted);
    curveErrorDOF->setYAxis(QwtPlot::yLeft);
    curveErrorDOF->setTitle(tr("conv. chart"));
    curveErrorDOF->attach(chartErrorDOF);

    // curves
    curveErrorDOFMax = new QwtPlotCurve();
    curveErrorDOFMax->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveErrorDOFMax->setPen(QPen(Qt::red));
    curveErrorDOFMax->setCurveAttribute(QwtPlotCurve::Inverted);
    curveErrorDOFMax->setYAxis(QwtPlot::yLeft);
    curveErrorDOFMax->setTitle(tr("max. error"));
    curveErrorDOFMax->attach(chartErrorDOF);

    // labels
    QwtText textErrorDOFLeft(tr("Error (%)"));
    textErrorDOFLeft.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartErrorDOF->setAxisTitle(QwtPlot::yLeft, textErrorDOFLeft);

    QwtText textErrorDOFBottom(tr("DOFs (-)"));
    textErrorDOFBottom.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartErrorDOF->setAxisTitle(QwtPlot::xBottom, textErrorDOFBottom);

    // legend
    /*
    QwtLegend *legend = new QwtLegend();
    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
    chart->insertLegend(legend, QwtPlot::BottomLegend);
    */

    QVBoxLayout *layoutConvergenceChart = new QVBoxLayout();
    layoutConvergenceChart->addWidget(chartErrorDOF);

    QWidget *widConvergenceChart = new QWidget();
    widConvergenceChart->setLayout(layoutConvergenceChart);

    return widConvergenceChart;
}

QWidget *ProgressDialog::createControlsNonlinear()
{
    logMessage("ProgressDialog::createControlsNonlinear()");

    chartNonlinear = new Chart(this);

    // curves
    curveNonlinear = new QwtPlotCurve();
    curveNonlinear->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveNonlinear->setPen(QPen(Qt::blue));
    curveNonlinear->setCurveAttribute(QwtPlotCurve::Inverted);
    curveNonlinear->setYAxis(QwtPlot::yLeft);
    curveNonlinear->setTitle(tr("nonlinearity"));
    curveNonlinear->attach(chartNonlinear);

    // curves
    curveNonlinearMax = new QwtPlotCurve();
    curveNonlinearMax->setRenderHint(QwtPlotItem::RenderAntialiased);
    curveNonlinearMax->setPen(QPen(Qt::red));
    curveNonlinearMax->setCurveAttribute(QwtPlotCurve::Inverted);
    curveNonlinearMax->setYAxis(QwtPlot::yLeft);
    curveNonlinearMax->setTitle(tr("max. error"));
    curveNonlinearMax->attach(chartNonlinear);

    // labels
    QwtText textErrorLeft(tr("Error (%)"));
    textErrorLeft.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartNonlinear->setAxisTitle(QwtPlot::yLeft, textErrorLeft);

    QwtText textErrorBottom(tr("Steps (-)"));
    textErrorBottom.setFont(QFont("Helvetica", 10, QFont::Normal));
    chartNonlinear->setAxisTitle(QwtPlot::xBottom, textErrorBottom);

    // legend
    /*
QwtLegend *legend = new QwtLegend();
legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
chart->insertLegend(legend, QwtPlot::BottomLegend);
*/

    QVBoxLayout *layoutNonlinearity = new QVBoxLayout();
    layoutNonlinearity->addWidget(chartNonlinear);

    QWidget *widNonlinearityChart = new QWidget();
    widNonlinearityChart->setLayout(layoutNonlinearity);

    return widNonlinearityChart;
}

void ProgressDialog::resetControls(int currentTab)
{
    btnSaveImage->setEnabled(false);
    btnSaveData->setEnabled(false);

    if (Util::scene()->sceneSolution()->isSolved())
    {
        btnSaveImage->setDisabled(tabType->currentIndex() == 0);
        btnSaveData->setDisabled(tabType->currentIndex() == 0);
    }
}

int ProgressDialog::progressSteps()
{
    logMessage("ProgressDialog::progressSteps()");

    int steps = 0;
    for (int i = 0; i < m_progressItem.count(); i++)
        steps += m_progressItem.at(i)->steps();

    return steps;
}

int ProgressDialog::currentProgressStep()
{
    logMessage("ProgressDialog::currentProgressStep()");

    int steps = 0;
    for (int i = 0; i < m_progressItem.count(); i++)
    {
        if (m_progressItem.at(i) == m_currentProgressItem)
            return steps;

        steps += m_progressItem.at(i)->steps();
    }

    return -1;
}

void ProgressDialog::appendProgressItem(ProgressItem *progressItem)
{
    logMessage("ProgressDialog::appendProgressItem()");

    progressItem->init();
    progressItem->setSteps();
    progressItem->disconnect();

    m_progressItem.append(progressItem);
    connect(progressItem, SIGNAL(changed()), this, SLOT(itemChanged()));
    connect(progressItem, SIGNAL(message(QString, bool, int)), this, SLOT(showMessage(QString, bool, int)));
    connect(this, SIGNAL(cancelProgressItem()), progressItem, SLOT(cancelProgressItem()));
}

bool ProgressDialog::run(bool showViewProgress)
{
    logMessage("ProgressDialog::run()");

    // current widget
    tabType->setCurrentWidget(controlsProgress);

    if (Util::scene()->problemInfo()->adaptivityType == AdaptivityType_None)
    {
        controlsConvergenceErrorChart->setEnabled(false);
        controlsConvergenceDOFChart->setEnabled(false);
        controlsConvergenceErrorDOFChart->setEnabled(false);
    }

    m_showViewProgress = showViewProgress;
    QTimer::singleShot(0, this, SLOT(start()));

    return exec();
}

void ProgressDialog::start()
{
    logMessage("ProgressDialog::start()");

    lstMessage->clear();

    progressBar->setRange(0, progressSteps());
    progressBar->setValue(0);

    for (int i = 0; i < m_progressItem.count(); i++)
    {
        m_currentProgressItem = m_progressItem.at(i);

        if (!m_currentProgressItem->run())
        {
            // error
            finished();
            clear();
            return;
        }
    }

    // successfull run
    if ((Util::config()->showConvergenceChart
         && Util::scene()->problemInfo()->adaptivityType != AdaptivityType_None
         && curveError->dataSize() > 0)
            || (Util::config()->showNonlinearChart && curveNonlinear->dataSize() > 0))
    {
        btnCancel->setEnabled(false);
        btnSaveImage->setEnabled(false);
        btnSaveData->setEnabled(false);
    }
    else
    {
        clear();
        close();
    }
}

void ProgressDialog::showMessage(const QString &msg, bool isError, int position)
{
    logMessage("ProgressDialog::showMessage()");

    btnCancel->setEnabled(!isError);

    if (isError)
    {
        lstMessage->setTextColor(QColor(Qt::red));
        finished();
    }
    else
    {
        lstMessage->setTextColor(QColor(Qt::black));
    }

    QString message = QString("%1: %2\n").
            arg(m_currentProgressItem->name()).
            arg(msg);

    lstMessage->insertPlainText(message);

    lstMessage->ensureCursorVisible();
    lblMessage->setText(message);

    if (position > 0)
    {
        progressBar->setValue(currentProgressStep() + position);
        Indicator::setProgress((double) position / progressSteps());
    }

    // update
    if (position % 3 == 0) QApplication::processEvents();
    lstMessage->update();
}

void ProgressDialog::itemChanged()
{
    logMessage("ProgressDialog::itemChanged()");

    if (m_progressItem.count() == 0)
        return;

    ProgressItemSolve *itemSolve = dynamic_cast<ProgressItemSolve *>(m_currentProgressItem);
    if (itemSolve)
    {
        // adaptivity error
        int adaptivityCount = itemSolve->adaptivityError().count();
        if (adaptivityCount > 0)
        {
            double *xval = new double[adaptivityCount];
            double *yvalError = new double[adaptivityCount];
            double *yvalDOF = new double[adaptivityCount];

            double minDOF = numeric_limits<double>::max();
            double maxDOF = numeric_limits<double>::min();

            for (int i = 0; i<adaptivityCount; i++)
            {
                xval[i] = i+1;
                yvalError[i] = itemSolve->adaptivityError().at(i);
                yvalDOF[i] = itemSolve->adaptivityDOF().at(i);

                minDOF = min(minDOF, yvalDOF[i]);
                maxDOF = max(maxDOF, yvalDOF[i]);
            }

            // max error
            double *xvalErrorMax = new double[2];
            double *yvalErrorMax = new double[2];
            xvalErrorMax[0] = 1;
            xvalErrorMax[1] = adaptivityCount;
            yvalErrorMax[0] = Util::scene()->problemInfo()->adaptivityTolerance;
            yvalErrorMax[1] = Util::scene()->problemInfo()->adaptivityTolerance;

            // plot error
            bool doReplotError = chartError->autoReplot();
            chartError->setAutoReplot(false);

            curveError->setData(xval, yvalError, adaptivityCount);
            curveErrorMax->setData(xvalErrorMax, yvalErrorMax, 2);

            chartError->setAutoReplot(doReplotError);
            chartError->replot();

            // plot dof
            bool doReplotDOF = chartDOF->autoReplot();
            chartDOF->setAutoReplot(false);

            curveDOF->setData(xval, yvalDOF, adaptivityCount);

            chartDOF->setAutoReplot(doReplotDOF);
            chartDOF->replot();

            // max error
            double *xvalErrorDOFMax = new double[2];
            double *yvalErrorDOFMax = new double[2];
            xvalErrorDOFMax[0] = minDOF;
            xvalErrorDOFMax[1] = maxDOF;
            yvalErrorDOFMax[0] = Util::scene()->problemInfo()->adaptivityTolerance;
            yvalErrorDOFMax[1] = Util::scene()->problemInfo()->adaptivityTolerance;

            // plot conv. chart
            bool doReplotErrorDOF = chartErrorDOF->autoReplot();
            chartErrorDOF->setAutoReplot(false);

            curveErrorDOF->setData(yvalDOF, yvalError, adaptivityCount);
            curveErrorDOFMax->setData(xvalErrorDOFMax, yvalErrorDOFMax, 2);

            chartErrorDOF->setAutoReplot(doReplotErrorDOF);
            chartErrorDOF->replot();

            // save data
            QFile fileErr(tempProblemDir() + "/adaptivity_error.csv");
            QTextStream outErr(&fileErr);
            if (fileErr.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                for (unsigned i = 0; i < curveError->data().size(); i++)
                    outErr << curveError->data().x(i) << ";" << curveError->data().y(i) << endl;
            }
            fileErr.close();

            QFile fileDOF(tempProblemDir() + "/adaptivity_dof.csv");
            QTextStream outDOF(&fileDOF);
            if (fileDOF.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                for (unsigned i = 0; i < curveDOF->data().size(); i++)
                    outDOF << curveDOF->data().x(i) << ";" << curveDOF->data().y(i) << endl;
            }
            fileDOF.close();

            QFile fileErrDOF(tempProblemDir() + "/adaptivity_conv.csv");
            QTextStream outErrDOF(&fileErrDOF);
            if (fileErrDOF.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                for (unsigned i = 0; i < curveErrorDOF->data().size(); i++)
                    outErrDOF << curveErrorDOF->data().x(i) << ";" << curveErrorDOF->data().y(i) << endl;
            }
            fileErrDOF.close();

            // save images
            QFile::remove(tempProblemDir() + "/adaptivity_error.png");
            QFile::remove(tempProblemDir() + "/adaptivity_dof.png");
            QFile::remove(tempProblemDir() + "/adaptivity_conv.png");

            chartError->saveImage(tempProblemDir() + "/adaptivity_error.png");
            chartDOF->saveImage(tempProblemDir() + "/adaptivity_dof.png");
            chartErrorDOF->saveImage(tempProblemDir() + "/adaptivity_conv.png");

            delete[] xval;
            delete[] yvalError;
            delete[] xvalErrorMax;
            delete[] yvalErrorMax;
            delete[] yvalDOF;
            delete[] xvalErrorDOFMax;
            delete[] yvalErrorDOFMax;
        }

        // nonlinearity
        int nonlinearErrorCount = itemSolve->nonlinearError().count();
        if (nonlinearErrorCount > 0)
        {
            double *xval = new double[nonlinearErrorCount];
            double *yvalError = new double[nonlinearErrorCount];

            for (int i = 0; i<nonlinearErrorCount; i++)
            {
                xval[i] = i+1;
                yvalError[i] = itemSolve->nonlinearError().at(i);
            }

            // max error
            double *xvalErrorMax = new double[2];
            double *yvalErrorMax = new double[2];
            xvalErrorMax[0] = 1;
            xvalErrorMax[1] = nonlinearErrorCount;
            yvalErrorMax[0] = Util::scene()->problemInfo()->nonlinearTolerance;
            yvalErrorMax[1] = Util::scene()->problemInfo()->nonlinearTolerance;

            // plot error
            bool doReplotError = chartNonlinear->autoReplot();
            chartNonlinear->setAutoReplot(false);

            curveNonlinear->setData(xval, yvalError, nonlinearErrorCount);
            curveNonlinearMax->setData(xvalErrorMax, yvalErrorMax, 2);

            chartNonlinear->setAutoReplot(doReplotError);
            chartNonlinear->replot();

            // save data
            QFile fileErr(tempProblemDir() + "/nonlinear_error.csv");
            QTextStream outErr(&fileErr);
            if (fileErr.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                for (int i = 0; i < curveNonlinear->data().size(); i++)
                    outErr << curveNonlinear->data().x(i) << ";" << curveNonlinear->data().y(i) << endl;
            }
            fileErr.close();

            // save image
            chartNonlinear->saveImage(tempProblemDir() + "/nonlinear_error.png");

            delete[] xval;
            delete[] yvalError;
            delete[] xvalErrorMax;
            delete[] yvalErrorMax;
        }
    }
}

void ProgressDialog::finished()
{
    logMessage("ProgressDialog::finished()");

    btnClose->setEnabled(true);
    btnCancel->setEnabled(false);
}

void ProgressDialog::cancel()
{
    logMessage("ProgressDialog::cancel()");

    emit cancelProgressItem();
    finished();
    QApplication::processEvents();
}

void ProgressDialog::close()
{
    logMessage("ProgressDialog::close()");

    QSettings settings;
    settings.setValue("ProgressDialog/Geometry", saveGeometry());

    // save progress messages
    if (Util::config()->enabledProgressLog)
        saveProgressLog();

    cancel();
    accept();
}

void ProgressDialog::saveProgressLog()
{
    logMessage("ProgressDialog::saveProgressLog()");

    QFile file(tempProblemDir() + "/messages.log");
    if (file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream messages(&file);
        messages << QDateTime(QDateTime::currentDateTime()).toString("dd.MM.yyyy hh:mm") + ",\x20" << Util::scene()->problemInfo()->name + "\n" << lstMessage->toPlainText() + "\n";
    }
}

void ProgressDialog::saveImage()
{
    logMessage("ProgressDialog::saveImage()");

    if (tabType->currentWidget() == controlsConvergenceErrorChart)
    {
        chartError->saveImage();
    }
    else if (tabType->currentWidget() == controlsConvergenceDOFChart)
    {
        chartDOF->saveImage();
    }
    else if (tabType->currentWidget() == controlsConvergenceErrorDOFChart)
    {
        chartErrorDOF->saveImage();
    }
    else if (tabType->currentWidget() == controlsNonlinear)
    {
        chartNonlinear->saveImage();
    }
}

void ProgressDialog::saveData()
{
    logMessage("ProgressDialog::saveData()");

    QSettings settings;
    QString dir = settings.value("General/LastDataDir").toString();

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export data to file"), dir, tr("CSV files (*.csv)"), &selectedFilter);
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);

        // open file for write
        if (fileInfo.suffix().isEmpty())
            fileName = fileName + ".csv";

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            cerr << "Could not create " + fileName.toStdString() + " file." << endl;
            return;
        }
        QTextStream out(&file);

        if (tabType->currentWidget() == controlsConvergenceErrorChart)
        {
            for (unsigned i = 0; i < curveError->data().size(); i++)
                out << curveError->data().x(i) << ";" << curveError->data().y(i) << endl;
        }
        else if (tabType->currentWidget() == controlsConvergenceDOFChart)
        {
            for (unsigned i = 0; i < curveDOF->data().size(); i++)
                out << curveDOF->data().x(i) << ";" << curveDOF->data().y(i) << endl;
        }
        else if (tabType->currentWidget() == controlsConvergenceErrorDOFChart)
        {
            for (unsigned i = 0; i < curveErrorDOF->data().size(); i++)
                out << curveErrorDOF->data().x(i) << ";" << curveErrorDOF->data().y(i) << endl;
        }
        else if (tabType->currentWidget() == controlsNonlinear)
        {
            for (unsigned i = 0; i < curveNonlinear->data().size(); i++)
                out << curveNonlinear->data().x(i) << ";" << curveNonlinear->data().y(i) << endl;
        }

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastDataDir", fileInfo.absolutePath());

        file.close();
    }

}

