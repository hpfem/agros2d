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
#include "scene.h"
#include "sceneview.h"
#include <limits.h>

struct TriangleEdge
{
    TriangleEdge()
    {
        this->node_1 = -1;
        this->node_2 = -1;
        this->marker = -1;
    }

    TriangleEdge(int node_1, int node_2, int marker)
    {
        this->node_1 = node_1;
        this->node_2 = node_2;
        this->marker = marker;
    }

    int node_1, node_2, marker;
};

/*
struct NodeTriangle
{
    NodeTriangle(int n, double x, double y, int marker)
    {
        this->n = n;
        this->x = x;
        this->y = n;
        this->marker = marker;
    }

    int n;
    double x, y;
    int marker;
};
*/

SolutionArray::SolutionArray()
{
    logMessage("SolutionArray::SolutionArray()");

    sln = NULL;
    order = NULL;

    time = 0.0;
    adaptiveSteps = 0;
    adaptiveError = 100.0;
}

SolutionArray::~SolutionArray()
{
    logMessage("SolutionArray::~SolutionArray()");

    if (sln) { delete sln; sln = NULL; }
    if (order) { delete order; order = NULL; }
}

void SolutionArray::load(QDomElement *element)
{
    logMessage("SolutionArray::load()");

    QString fileNameSolution = tempProblemFileName() + ".sln";
    QString fileNameOrder = tempProblemFileName() + ".ord";

    // write content (saved solution)
    QByteArray contentSolution;
    contentSolution.append(element->elementsByTagName("sln").at(0).toElement().childNodes().at(0).nodeValue());
    writeStringContentByteArray(fileNameSolution, QByteArray::fromBase64(contentSolution));

    // write content (saved order)
    QByteArray contentOrder;
    contentOrder.append(element->elementsByTagName("order").at(0).toElement().childNodes().at(0).nodeValue());
    writeStringContentByteArray(fileNameOrder, QByteArray::fromBase64(contentOrder));

    order = new Orderizer();
    order->load_data(fileNameOrder.toStdString().c_str());
    sln = new Solution();
    sln->load(fileNameSolution.toStdString().c_str());
    adaptiveError = element->attribute("adaptiveerror").toDouble();
    adaptiveSteps = element->attribute("adaptivesteps").toInt();
    time = element->attribute("time").toDouble();

    // delete solution
    QFile::remove(fileNameSolution);
    QFile::remove(fileNameOrder);
}

void SolutionArray::save(QDomDocument *doc, QDomElement *element)
{
    logMessage("SolutionArray::save()");

    // solution
    QString fileNameSolution = tempProblemFileName() + ".sln";
    sln->save(fileNameSolution.toStdString().c_str(), false);
    QDomText textSolution = doc->createTextNode(readFileContentByteArray(fileNameSolution).toBase64());

    // order
    QString fileNameOrder = tempProblemFileName() + ".ord";
    order->save_data(fileNameOrder.toStdString().c_str());
    QDomNode textOrder = doc->createTextNode(readFileContentByteArray(fileNameOrder).toBase64());

    QDomNode eleSolution = doc->createElement("sln");
    QDomNode eleOrder = doc->createElement("order");

    eleSolution.appendChild(textSolution);
    eleOrder.appendChild(textOrder);

    element->setAttribute("adaptiveerror", adaptiveError);
    element->setAttribute("adaptivesteps", adaptiveSteps);
    element->setAttribute("time", time);
    element->appendChild(eleSolution);
    element->appendChild(eleOrder);

    // delete
    QFile::remove(fileNameSolution);
    QFile::remove(fileNameOrder);
}

// *********************************************************************************************

ProgressItem::ProgressItem()
{
    logMessage("ProgressItem::ProgressItem()");

    m_name = "";
    m_steps = 0;
    m_isError = false;
    m_isCanceled = false;

    connect(this, SIGNAL(message(QString, bool, int)), this, SLOT(showMessage(QString, bool, int)));
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
    m_steps = 4;
}

bool ProgressItemMesh::run()
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

            return !m_isError;
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

    return !m_isError;
}

void ProgressItemMesh::meshTriangleCreated(int exitCode)
{
    logMessage("ProgressItemMesh::meshTriangleCreated()");

    if (exitCode == 0)
    {
        emit message(tr("Mesh files was created"), false, 2);

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
            QFile::remove(tempProblemFileName() + ".poly");
            QFile::remove(tempProblemFileName() + ".node");
            QFile::remove(tempProblemFileName() + ".edge");
            QFile::remove(tempProblemFileName() + ".ele");
            QFile::remove(tempProblemFileName() + ".triangle.out");
            QFile::remove(tempProblemFileName() + ".triangle.err");
            emit message(tr("Mesh files was deleted"), false, 4);

            // load mesh
            Mesh *mesh = readMeshFromFile(tempProblemFileName() + ".mesh");

            // check that all boundary edges have a marker assigned
            for (int i = 0; i < mesh->get_max_node_id(); i++)
            {
                if (Node *node = mesh->get_node(i))
                {
                    if (node->used)
                    {
                        if (node->ref < 2 && node->marker == 0)
                        {
                            emit message(tr("Boundary edge does not have a boundary marker"), true, 0);

                            delete mesh;
                            return;
                        }
                    }
                }
            }

            refineMesh(mesh, true, true);

            Util::scene()->sceneSolution()->setMeshInitial(mesh);
        }
        else
        {
            QFile::remove(Util::scene()->problemInfo()->fileName + ".mesh");
        }
    }
    else
    {
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
            if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) > 0)
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
            if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) > 0)
                count++;

        if (count == 0)
        {
            emit message(tr("At least one material has to be assigned"), true, 0);
            return false;
        }
    }
    if (Util::scene()->edgeMarkers.count() < 2) // + none marker
    {
        emit message(tr("Invalid number of boundary conditions (%1 < 1)").arg(Util::scene()->edgeMarkers.count()), true, 0);
        return false;
    }
    if (Util::scene()->labelMarkers.count() < 2) // + none marker
    {
        emit message(tr("Invalid number of materials (%1 < 1)").arg(Util::scene()->labelMarkers.count()), true, 0);
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
    for (int i = 0; i<Util::scene()->labels.count(); i++) if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0) holesCount++;
    QString outHoles = QString("%1\n").arg(holesCount);
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
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
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) > 0)
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

    QFile fileMesh(tempProblemFileName() + ".mesh");
    if (!fileMesh.open(QIODevice::WriteOnly))
    {
        emit message(tr("Could not create Hermes2D mesh file"), true, 0);
        return false;
    }
    QTextStream outMesh(&fileMesh);

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

    // triangle nodes
    sscanf(inNode.readLine().toStdString().c_str(), "%i", &k);
    Point nodeList[k];
    for (int i = 0; i<k; i++)
    {
        int marker;
        double x, y;

        sscanf(inNode.readLine().toStdString().c_str(), "%i   %lf %lf %i", &n, &x, &y, &marker);
        nodeList[i] = Point(x, y);
    }
    int nodeCount = k;

    // triangle edges
    sscanf(inEdge.readLine().toStdString().c_str(), "%i", &k);
    TriangleEdge edgeList[k];
    for (int i = 0; i<k; i++)
    {
        int node_1, node_2, marker;

        sscanf(inEdge.readLine().toStdString().c_str(), "%i	%i	%i	%i", &n, &node_1, &node_2, &marker);
        edgeList[i] = TriangleEdge(node_1, node_2, marker);
    }
    int edgeCount = k;

    // edges
    QString outEdges;
    outEdges += "boundaries =\n";
    outEdges += "{\n";
    int countEdges = 0;
    for (int i = 0; i<edgeCount; i++)
    {
        if (edgeList[i].marker != 0)
        {
            if (Util::scene()->edges[edgeList[i].marker-1]->marker->type != PhysicFieldBC_None)
            {
                countEdges++;
                outEdges += QString("  { %1, %2, %3 },\n").
                        arg(edgeList[i].node_1).
                        arg(edgeList[i].node_2).
                        arg(abs(edgeList[i].marker));
            }
        }
    }
    outEdges.truncate(outEdges.length()-2);
    outEdges += "\n}\n\n";

    if (countEdges < 1)
    {
        emit message(tr("Invalid number of edge markers"), true, 0);
        return false;
    }

    // curves
    QString outCurves;
    int countCurves = 0;
    if (Util::config()->curvilinearElements)
    {
        outCurves += "curves =\n";
        outCurves += "{\n";
        for (int i = 0; i<edgeCount; i++)
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
                    double chordShort = (nodeList[edgeList[i].node_2] - nodeList[edgeList[i].node_1]).magnitude();

                    // direction
                    Point center = Util::scene()->edges[edgeList[i].marker-1]->center();
                    int direction = (((nodeList[edgeList[i].node_1].x-center.x)*(nodeList[edgeList[i].node_2].y-center.y) -
                                      (nodeList[edgeList[i].node_1].y-center.y)*(nodeList[edgeList[i].node_2].x-center.x)) > 0) ? 1 : -1;

                    double angle = direction * theta * chordShort / chord;

                    outCurves += QString("  { %1, %2, %3 },\n").
                            arg(edgeList[i].node_1).
                            arg(edgeList[i].node_2).
                            arg(rad2deg(angle));
                }
            }
        }
        outCurves.truncate(outCurves.length()-2);
        outCurves += "\n}\n\n";

        // move nodes (arcs)
        for (int i = 0; i<edgeCount; i++)
        {
            if (edgeList[i].marker != 0)
            {
                // curve
                if (Util::scene()->edges[edgeList[i].marker-1]->angle > 0.0)
                {
                    // angle
                    Point center = Util::scene()->edges[edgeList[i].marker-1]->center();
                    double pointAngle1 = atan2(center.y - nodeList[edgeList[i].node_1].y,
                                               center.x - nodeList[edgeList[i].node_1].x) - M_PI;

                    double pointAngle2 = atan2(center.y - nodeList[edgeList[i].node_2].y,
                                               center.x - nodeList[edgeList[i].node_2].x) - M_PI;

                    nodeList[edgeList[i].node_1].x = center.x + Util::scene()->edges[edgeList[i].marker-1]->radius() * cos(pointAngle1);
                    nodeList[edgeList[i].node_1].y = center.y + Util::scene()->edges[edgeList[i].marker-1]->radius() * sin(pointAngle1);

                    nodeList[edgeList[i].node_2].x = center.x + Util::scene()->edges[edgeList[i].marker-1]->radius() * cos(pointAngle2);
                    nodeList[edgeList[i].node_2].y = center.y + Util::scene()->edges[edgeList[i].marker-1]->radius() * sin(pointAngle2);
                }
            }
        }
    }

    // nodes
    QString outNodes;
    outNodes += "vertices =\n";
    outNodes += "{\n";
    for (int i = 0; i<nodeCount; i++)
    {
        outNodes += QString("  { %1,  %2 },\n").
                arg(nodeList[i].x, 0, 'f', 10).
                arg(nodeList[i].y, 0, 'f', 10);
    }
    outNodes.truncate(outNodes.length()-2);
    outNodes += "\n}\n\n";

    // elements
    QString outElements;
    outElements += "elements =\n";
    outElements += "{\n";
    sscanf(inEle.readLine().toStdString().c_str(), "%i", &k);
    int countElements = 0;
    for (int i = 0; i<k; i++)
    {
        int node_1, node_2, node_3, marker;

        countElements++;
        sscanf(inEle.readLine().toStdString().c_str(), "%i	%i	%i	%i	%i", &n, &node_1, &node_2, &node_3, &marker);
        if (marker == 0)
        {
            emit message(tr("Some areas have no label marker"), true, 0);
            return false;
        }
        // triangle returns zero region number for areas without marker, markers must start from 1
        marker--;
        outElements += QString("  { %1, %2, %3, %4 },\n").
                arg(node_1).
                arg(node_2).
                arg(node_3).
                arg(abs(marker));
    }
    outElements.truncate(outElements.length()-2);
    outElements += "\n}\n\n";
    if (countElements < 1)
    {
        emit message(tr("Invalid number of label markers"), true, 0);
        return false;
    }

    outMesh << outNodes;
    outMesh << outElements;
    outMesh << outEdges;
    if (countCurves > 0)
        outMesh << outCurves;

    fileNode.close();
    fileEdge.close();
    fileEle.close();

    fileMesh.waitForBytesWritten(0);
    fileMesh.close();

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return true;
}

// *********************************************************************************************

ProgressItemSolve::ProgressItemSolve() : ProgressItem()
{
    logMessage("ProgressItemSolve::ProgressItemSolve()");

    m_name = tr("Solver");
    m_steps = 1;
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        m_steps += floor(Util::scene()->problemInfo()->timeTotal.number / Util::scene()->problemInfo()->timeStep.number);
}

bool ProgressItemSolve::run()
{
    logMessage("ProgressItemSolve::()");

    solve();

    return !m_isError;
}

void ProgressItemSolve::solve()
{
    logMessage("ProgressItemSolve::solve()");

    m_adaptivityError.clear();
    m_adaptivityDOF.clear();

    qDebug() << "ProgressItemSolve::solve()";

    if (!QFile::exists(tempProblemFileName() + ".mesh"))
        return;

    // benchmark
    QTime time;
    time.start();

    emit message(tr("Problem analysis: %1 (%2, %3)").
                 arg(physicFieldString(Util::scene()->problemInfo()->physicField())).
                 arg(problemTypeString(Util::scene()->problemInfo()->problemType)).
                 arg(analysisTypeString(Util::scene()->problemInfo()->analysisType)), false, 1);

    emit message(tr("Solver was started: %1 ").arg(matrixSolverTypeString(Util::scene()->problemInfo()->matrixSolver)), false, 1);

    QList<SolutionArray *> solutionArrayList = Util::scene()->problemInfo()->hermes()->solve(this);

    if (!solutionArrayList.isEmpty())
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

bool ProgressItemProcessView::run()
{
    logMessage("ProgressItemProcessView::run()");

    process();

    return !m_isError;
}

void ProgressItemProcessView::process()
{
    logMessage("ProgressItemProcessView::process()");

    int step = 0;

    if (sceneView()->sceneViewSettings().showSolutionMesh)
    {
        step++;
        emit message(tr("Processing solution mesh cache"), false, step);
        Util::scene()->sceneSolution()->processSolutionMesh();
    }
    if (sceneView()->sceneViewSettings().showContours)
    {
        step++;
        emit message(tr("Processing countour view cache"), false, step);
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
    clear();

    QFile::remove(tempProblemDir() + "/adaptivity_error.png");
    QFile::remove(tempProblemDir() + "/adaptivity_dof.png");
    QFile::remove(tempProblemDir() + "/adaptivity_conv.png");

    setMinimumSize(520, 360);
    setMaximumSize(minimumSize());
}

ProgressDialog::~ProgressDialog()
{
    logMessage("ProgressDialog::~ProgressDialog()");

    if (Util::config()->enabledProgressLog)
        saveProgressLog();

    clear();
}

void ProgressDialog::clear()
{
    logMessage("ProgressDialog::clear()");

    // delete progress items
    for (int i = 0; i < m_progressItem.count(); i++)
        delete m_progressItem.at(i);
    m_progressItem.clear();

    m_currentProgressItem = NULL;
    m_showViewProgress = true;
}

void ProgressDialog::createControls()
{
    logMessage("ProgressDialog::createControls()");

    controlsProgress = createControlsProgress();
    controlsConvergenceErrorChart = createControlsConvergenceErrorChart();
    controlsConvergenceDOFChart = createControlsConvergenceDOFChart();
    controlsConvergenceErrorDOFChart = createControlsConvergenceErrorDOFChart();

    tabType = new QTabWidget();
    tabType->addTab(controlsProgress, icon(""), tr("Progress"));
    tabType->addTab(controlsConvergenceErrorChart, icon(""), tr("Adapt. error"));
    tabType->addTab(controlsConvergenceDOFChart, icon(""), tr("Adapt. DOFs"));
    tabType->addTab(controlsConvergenceErrorDOFChart, icon(""), tr("Adapt. conv."));
    connect(tabType, SIGNAL(currentChanged(int)), this, SLOT(resetControls(int)));

    if (Util::scene()->problemInfo()->adaptivityType == AdaptivityType_None)
    {
       controlsConvergenceErrorChart->setEnabled(false);
       controlsConvergenceDOFChart->setEnabled(false);
       controlsConvergenceErrorDOFChart->setEnabled(false);
    }

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

    m_progressItem.append(progressItem);
    connect(progressItem, SIGNAL(changed()), this, SLOT(itemChanged()));
    connect(progressItem, SIGNAL(message(QString, bool, int)), this, SLOT(showMessage(QString, bool, int)));
    connect(this, SIGNAL(cancelProgressItem()), progressItem, SLOT(cancelProgressItem()));
}

bool ProgressDialog::run(bool showViewProgress)
{
    logMessage("ProgressDialog::run()");

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
    QApplication::processEvents();

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
    if (!Util::config()->showConvergenceChart ||
            Util::scene()->problemInfo()->adaptivityType == AdaptivityType_None ||
            curveError->dataSize() == 0)
    {
        clear();
        close();
    }
    else
    {
        btnCancel->setEnabled(false);
        btnSaveImage->setEnabled(false);
        btnSaveData->setEnabled(false);
        // tabType->setCurrentWidget(controlsConvergenceChart);
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
        progressBar->setValue(currentProgressStep() + position);

    // update
    QApplication::processEvents();
    lstMessage->update();
}

void ProgressDialog::itemChanged()
{
    logMessage("ProgressDialog::itemChanged()");

    if (m_progressItem.count() == 0)
        return;

    if (ProgressItemSolve *itemSolve = dynamic_cast<ProgressItemSolve *>(m_currentProgressItem))
    {
        // error
        int count = itemSolve->adaptivityError().count();

        double *xval = new double[count];
        double *yvalError = new double[count];
        double *yvalDOF = new double[count];

        double minDOF = numeric_limits<double>::max();
        double maxDOF = numeric_limits<double>::min();

        for (int i = 0; i<count; i++)
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
        xvalErrorMax[1] = count;
        yvalErrorMax[0] = Util::scene()->problemInfo()->adaptivityTolerance;
        yvalErrorMax[1] = Util::scene()->problemInfo()->adaptivityTolerance;

        // plot error
        bool doReplotError = chartError->autoReplot();
        chartError->setAutoReplot(false);

        curveError->setData(xval, yvalError, count);
        curveErrorMax->setData(xvalErrorMax, yvalErrorMax, 2);

        chartError->setAutoReplot(doReplotError);
        chartError->replot();

        // plot dof
        bool doReplotDOF = chartDOF->autoReplot();
        chartDOF->setAutoReplot(false);

        curveDOF->setData(xval, yvalDOF, count);

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

        curveErrorDOF->setData(yvalDOF, yvalError, count);
        curveErrorDOFMax->setData(xvalErrorDOFMax, yvalErrorDOFMax, 2);

        chartErrorDOF->setAutoReplot(doReplotErrorDOF);
        chartErrorDOF->replot();

        // save data
        QFile fileErr(tempProblemDir() + "/adaptivity_error.csv");
        QTextStream outErr(&fileErr);
        if (fileErr.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            for (int i = 0; i < curveError->data().size(); i++)
                outErr << curveError->data().x(i) << ";" << curveError->data().y(i) << endl;
        }
        fileErr.close();

        QFile fileDOF(tempProblemDir() + "/adaptivity_dof.csv");
        QTextStream outDOF(&fileDOF);
        if (fileDOF.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            for (int i = 0; i < curveDOF->data().size(); i++)
                outDOF << curveDOF->data().x(i) << ";" << curveDOF->data().y(i) << endl;
        }
        fileDOF.close();

        QFile fileErrDOF(tempProblemDir() + "/adaptivity_conv.csv");
        QTextStream outErrDOF(&fileErrDOF);
        if (fileErrDOF.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            for (int i = 0; i < curveErrorDOF->data().size(); i++)
                outErrDOF << curveErrorDOF->data().x(i) << ";" << curveErrorDOF->data().y(i) << endl;
        }
        fileErrDOF.close();

        // save image
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
            for (int i = 0; i < curveError->data().size(); i++)
                out << curveError->data().x(i) << ";" << curveError->data().y(i) << endl;

        }
        else if (tabType->currentWidget() == controlsConvergenceDOFChart)
        {
            for (int i = 0; i < curveDOF->data().size(); i++)
                out << curveDOF->data().x(i) << ";" << curveDOF->data().y(i) << endl;
        }
        else if (tabType->currentWidget() == controlsConvergenceErrorDOFChart)
        {
            for (int i = 0; i < curveErrorDOF->data().size(); i++)
                out << curveErrorDOF->data().x(i) << ";" << curveErrorDOF->data().y(i) << endl;
        }

        settings.setValue("General/LastDataDir", fileInfo.absolutePath());

        file.close();
    }

}
