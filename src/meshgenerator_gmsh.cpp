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

        qDebug() << QString(Util::config()->commandGmsh).
                    arg(gmshBinary).
                    arg(tempProblemFileName());
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
        if (gmshToHermes2D())
        {
            Util::log()->printMessage(tr("Mesh generator"), tr("mesh was converted to Hermes2D mesh file"));

            // copy triangle files
            if ((!Util::config()->deleteHermes2DMeshFile) && (!Util::problem()->config()->fileName().isEmpty()))
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
            // line
            outEdges += QString("Line(%1) = {%2, %3};\n").
                    arg(edgesCount).
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
                    arg(edgesCount).
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

    // find loops
    QList<int> loop;
    for (int i = 0; i<Util::scene()->edges->length(); i++)
    {
        SceneEdge *edge = Util::scene()->edges->items().at(i);

        if (loop.isEmpty())
        {
            loop.append(i);
        }
        else
        {
            for (int j = 0; j<Util::scene()->edges->length(); j++)
            {
                SceneEdge *edgeNew = Util::scene()->edges->items().at(j);
                if (i != j)
                {

                }
            }
        }
    }

    QString outLoops;
    outLoops.append(QString("Line Loop(1) = {0, 3, 2, 1};\n"));
    outLoops.append(QString("Plane Surface(1) = {1};\n"));
    outLoops.append(QString("Line Loop(2) = {4, 5, 6, -1};\n"));
    outLoops.append(QString("Plane Surface(2) = {2};\n"));
    outLoops.append("\n");
    outLoops.append(QString("Recombine Surface {1, 2};\n"));

    QString outCommands;
    outCommands.append(QString("Mesh.Algorithm = 8;\n")); // 1=MeshAdapt, 2=Automatic, 5=Delaunay, 6=Frontal, 7=bamg, 8=delquad
    // outCommands.append(QString("Mesh.CharacteristicLengthFactor = 5;\n"));
    outCommands.append(QString("Mesh.SubdivisionAlgorithm = 1;\n"));

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

bool MeshGeneratorGMSH::gmshToHermes2D()
{
    int k;

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    QDomDocument doc;
    QDomProcessingInstruction instr = doc.createProcessingInstruction("xml", "version='1.0' encoding='UTF-8' standalone='no'");
    doc.appendChild(instr);

    // main document
    QDomElement eleMesh = doc.createElement("domain:domain");
    eleMesh.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    eleMesh.setAttribute("xmlns:domain", "XMLSubdomains");
    eleMesh.setAttribute("xsi:schemaLocation", QString("XMLSubdomains %1/subdomains_h2d_xml.xsd").arg(datadir() + "/resources/xsd"));
    doc.appendChild(eleMesh);

    QDomElement eleVertices = doc.createElement("vertices");
    eleMesh.appendChild(eleVertices);
    QDomElement eleElements = doc.createElement("elements");
    eleMesh.appendChild(eleElements);
    QDomElement eleEdges = doc.createElement("edges");
    eleMesh.appendChild(eleEdges);
    QDomElement eleCurves = doc.createElement("curves");
    eleMesh.appendChild(eleCurves);
    QDomElement eleSubdomains = doc.createElement("subdomains");
    eleMesh.appendChild(eleSubdomains);

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
    QList<Point> nodeList;
    for (int i = 0; i < k; i++)
    {
        int n;
        double x, y, z;

        sscanf(inGMSH.readLine().toStdString().c_str(), "%i %lf %lf %lf", &n, &x, &y, &z);
        nodeList.append(Point(x, y));
    }

    // edges and elements
    QList<MeshEdge> edgeList;
    // triangle elements
    QList<MeshElement> elementList;
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
                edgeList.append(MeshEdge(quad[0] - 1, quad[1] - 1, marker)); // marker conversion from gmsh, where it starts from 1
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

    // edges
    int countEdges = 0;
    for (int i = 0; i < edgeList.count(); i++)
    {
        if (edgeList[i].isUsed)
        {
            int marker = edgeList[i].marker;

            //assert(countEdges == i+1);
            QDomElement eleEdge = doc.createElement("edge");
            eleEdge.setAttribute("v1", edgeList[i].node[0]);
            eleEdge.setAttribute("v2", edgeList[i].node[1]);
            eleEdge.setAttribute("i", i);
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
            if (edgeList[i].marker != -1)
            {
                // curve
                if (Util::scene()->edges->at(edgeList[i].marker)->angle() > 0.0)
                {
                    countCurves++;
                    int segments = Util::scene()->edges->at(edgeList[i].marker)->segments();

                    // subdivision angle and chord
                    double theta = deg2rad(Util::scene()->edges->at(edgeList[i].marker)->angle()) / double(segments);
                    double chord = 2 * Util::scene()->edges->at(edgeList[i].marker)->radius() * sin(theta / 2.0);

                    // length of short chord
                    double chordShort = (nodeList[edgeList[i].node[1]] - nodeList[edgeList[i].node[0]]).magnitude();

                    // direction
                    Point center = Util::scene()->edges->at(edgeList[i].marker)->center();
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
            // assert(edgeList[i].marker >= 0); // markers changed to marker - 1, check...
            if (edgeList[i].marker != -1)
            {
                // curve
                if (Util::scene()->edges->at(edgeList[i].marker)->angle() > 0.0)
                {
                    // angle
                    Point center = Util::scene()->edges->at(edgeList[i].marker)->center();
                    double pointAngle1 = atan2(center.y - nodeList[edgeList[i].node[0]].y,
                                               center.x - nodeList[edgeList[i].node[0]].x) - M_PI;

                    double pointAngle2 = atan2(center.y - nodeList[edgeList[i].node[1]].y,
                                               center.x - nodeList[edgeList[i].node[1]].x) - M_PI;

                    nodeList[edgeList[i].node[0]].x = center.x + Util::scene()->edges->at(edgeList[i].marker)->radius() * cos(pointAngle1);
                    nodeList[edgeList[i].node[0]].y = center.y + Util::scene()->edges->at(edgeList[i].marker)->radius() * sin(pointAngle1);

                    nodeList[edgeList[i].node[1]].x = center.x + Util::scene()->edges->at(edgeList[i].marker)->radius() * cos(pointAngle2);
                    nodeList[edgeList[i].node[1]].y = center.y + Util::scene()->edges->at(edgeList[i].marker)->radius() * sin(pointAngle2);
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
            QDomElement eleElement = doc.createElement(QString("domain:%1").arg(elementList[i].isTriangle() ? "triangle" : "quad"));
            eleElement.setAttribute("v1", elementList[i].node[0]);
            eleElement.setAttribute("v2", elementList[i].node[1]);
            eleElement.setAttribute("v3", elementList[i].node[2]);
            if (!elementList[i].isTriangle())
                eleElement.setAttribute("v4", elementList[i].node[3]);
            eleElement.setAttribute("i", i);
            eleElement.setAttribute("marker", QString("%1").arg(abs(elementList[i].marker)));

            eleElements.appendChild(eleElement);
        }
    }

    // find edge neighbours
    // for each vertex list elements that it belogns to
    QList<QSet<int> > vertexElements;
    vertexElements.reserve(nodeList.count());
    for (int i = 0; i < nodeList.count(); i++)
        vertexElements.push_back(QSet<int>());

    for (int i = 0; i < elementList.count(); i++)
    {
        if (elementList[i].isUsed)
        {
            for(int elemNode = 0; elemNode < (elementList[i].isTriangle() ? 3 : 4); elemNode++)
            {
                vertexElements[elementList[i].node[elemNode]].insert(i);
            }
        }
    }

    for (int i = 0; i < edgeList.count(); i++)
    {
        if (edgeList[i].isUsed)
        {
            QSet<int> neighbours = vertexElements[edgeList[i].node[0]];
            neighbours.intersect(vertexElements[edgeList[i].node[1]]);
            assert((neighbours.size() > 0) && (neighbours.size() <= 2));
            edgeList[i].neighElem[0] = neighbours.values()[0];
            if(neighbours.size() == 2)
                edgeList[i].neighElem[1] = neighbours.values()[1];
        }
    }

    // subdomains
    foreach(FieldInfo* fieldInfo, Util::problem()->fieldInfos())
    {
        QDomElement eleSubdomain = doc.createElement("subdomain");
        eleSubdomains.appendChild(eleSubdomain);
        eleSubdomain.setAttribute("name", fieldInfo->fieldId());

        QDomElement eleSubElements = doc.createElement("elements");
        eleSubdomain.appendChild(eleSubElements);

        for (int i = 0; i<elementList.count(); i++)
        {
            if (elementList[i].isUsed && (Util::scene()->labels->at(elementList[i].marker)->marker(fieldInfo) != SceneMaterialContainer::getNone(fieldInfo)))
            {
                QDomElement eleSubElement = doc.createElement("i");
                eleSubElements.appendChild(eleSubElement);
                QDomText number = doc.createTextNode(QString::number(i));
                eleSubElement.appendChild(number);
            }
        }

        QDomElement eleBoundaryEdges = doc.createElement("boundary_edges");
        eleSubdomain.appendChild(eleBoundaryEdges);
        QDomElement eleInnerEdges = doc.createElement("inner_edges");
        eleSubdomain.appendChild(eleInnerEdges);

        QList<int> unassignedEdges;
        for (int i = 0; i < edgeList.count(); i++)
        {
            QDomElement eleEdge = doc.createElement("i");
            QDomText number = doc.createTextNode(QString::number(i));
            eleEdge.appendChild(number);

            //assert(edgeList[i].marker >= 0);
            if (edgeList[i].isUsed)
            {
                int numNeighWithField = 0;
                for (int neigh_i = 0; neigh_i < 2; neigh_i++)
                {
                    int neigh = edgeList[i].neighElem[neigh_i];
                    if (neigh != -1)
                    {
                        if (Util::scene()->labels->at(elementList[neigh].marker)->marker(fieldInfo)
                                != SceneMaterialContainer::getNone(fieldInfo))
                            numNeighWithField++;
                    }
                }

                // edge has boundary condition prescribed for this field
                bool hasFieldBoundaryCondition = (Util::scene()->edges->at(edgeList[i].marker)->marker(fieldInfo)
                                                  != SceneBoundaryContainer::getNone(fieldInfo));

                if (numNeighWithField == 1)
                {
                    // edge is on "boundary" of the field, should have boundary condition prescribed

                    if (!hasFieldBoundaryCondition)
                        if (!unassignedEdges.contains(edgeList[i].marker))
                            unassignedEdges.append(edgeList[i].marker);

                    eleBoundaryEdges.appendChild(eleEdge);
                }
                else if (numNeighWithField == 2)
                {
                    // todo: we could enforce not to have boundary conditions prescribed inside:
                    // assert(!hasFieldBoundaryCondition);

                    eleInnerEdges.appendChild(eleEdge);
                }
            }
        }

        // not assigned boundary
        if (unassignedEdges.count() > 0)
        {
            QString list;
            foreach (int index, unassignedEdges)
                list += QString::number(index) + ", ";

            Util::log()->printError(tr("Mesh generator"), tr("Boundary condition is not assigned on following edges %1.").arg(list.left(list.count() - 2)));

            return false;
        }
    }

    nodeList.clear();
    edgeList.clear();
    elementList.clear();

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
