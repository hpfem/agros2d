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

#include "conf.h"
#include "util/global.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "logview.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"

MeshGenerator::MeshGenerator() : QObject()
{    
}

bool MeshGenerator::writeToHermes()
{
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

    // edges
    int countEdges = 0;
    for (int i = 0; i < edgeList.count(); i++)
    {
        if (edgeList[i].isUsed && edgeList[i].marker != -1)
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
    if (Agros2D::config()->curvilinearElements)
    {
        for (int i = 0; i<edgeList.count(); i++)
        {
            if (edgeList[i].marker != -1)
            {
                // curve
                if (Agros2D::scene()->edges->at(edgeList[i].marker)->angle() > 0.0)
                {
                    countCurves++;
                    int segments = Agros2D::scene()->edges->at(edgeList[i].marker)->segments();

                    // subdivision angle and chord
                    double theta = deg2rad(Agros2D::scene()->edges->at(edgeList[i].marker)->angle()) / double(segments);
                    double chord = 2 * Agros2D::scene()->edges->at(edgeList[i].marker)->radius() * sin(theta / 2.0);

                    // length of short chord
                    double chordShort = (nodeList[edgeList[i].node[1]] - nodeList[edgeList[i].node[0]]).magnitude();

                    // direction
                    Point center = Agros2D::scene()->edges->at(edgeList[i].marker)->center();
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
                if (Agros2D::scene()->edges->at(edgeList[i].marker)->angle() > 0.0)
                {
                    // angle
                    Point center = Agros2D::scene()->edges->at(edgeList[i].marker)->center();
                    double pointAngle1 = atan2(center.y - nodeList[edgeList[i].node[0]].y,
                                               center.x - nodeList[edgeList[i].node[0]].x) - M_PI;

                    double pointAngle2 = atan2(center.y - nodeList[edgeList[i].node[1]].y,
                                               center.x - nodeList[edgeList[i].node[1]].x) - M_PI;

                    nodeList[edgeList[i].node[0]].x = center.x + Agros2D::scene()->edges->at(edgeList[i].marker)->radius() * cos(pointAngle1);
                    nodeList[edgeList[i].node[0]].y = center.y + Agros2D::scene()->edges->at(edgeList[i].marker)->radius() * sin(pointAngle1);

                    nodeList[edgeList[i].node[1]].x = center.x + Agros2D::scene()->edges->at(edgeList[i].marker)->radius() * cos(pointAngle2);
                    nodeList[edgeList[i].node[1]].y = center.y + Agros2D::scene()->edges->at(edgeList[i].marker)->radius() * sin(pointAngle2);
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
        if (edgeList[i].isUsed && edgeList[i].marker != -1)
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
    foreach(FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        QDomElement eleSubdomain = doc.createElement("subdomain");
        eleSubdomains.appendChild(eleSubdomain);
        eleSubdomain.setAttribute("name", fieldInfo->fieldId());

        QDomElement eleSubElements = doc.createElement("elements");
        eleSubdomain.appendChild(eleSubElements);

        for (int i = 0; i<elementList.count(); i++)
        {
            if (elementList[i].isUsed && (Agros2D::scene()->labels->at(elementList[i].marker)->marker(fieldInfo) != SceneMaterialContainer::getNone(fieldInfo)))
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
            if (edgeList[i].isUsed && edgeList[i].marker != -1)
            {
                int numNeighWithField = 0;
                for (int neigh_i = 0; neigh_i < 2; neigh_i++)
                {
                    int neigh = edgeList[i].neighElem[neigh_i];
                    if (neigh != -1)
                    {
                        if (Agros2D::scene()->labels->at(elementList[neigh].marker)->marker(fieldInfo)
                                != SceneMaterialContainer::getNone(fieldInfo))
                            numNeighWithField++;
                    }
                }

                // edge has boundary condition prescribed for this field
                bool hasFieldBoundaryCondition = (Agros2D::scene()->edges->at(edgeList[i].marker)->marker(fieldInfo)
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

            Agros2D::log()->printError(tr("Mesh generator"), tr("Boundary condition is not assigned on following edges %1.").arg(list.left(list.count() - 2)));

            return false;
        }
    }

    // save to file
    QFile file(cacheProblemDir() + "/initial.mesh");
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
