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

#include "util/global.h"
#include "util/conf.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "logview.h"

#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

MeshGenerator::MeshGenerator() : QObject()
{    
}

bool MeshGenerator::writeToHermes()
{
    // edges
    XMLSubdomains::edges_type edges;
    for (int i = 0; i < edgeList.count(); i++)
        if (edgeList[i].isUsed && edgeList[i].marker != -1)
            edges.edge().push_back(XMLSubdomains::edge(edgeList[i].node[0], edgeList[i].node[1],
                    QString::number(edgeList[i].marker).toStdString(), i));

    // curved edges
    XMLMesh::curves_type curves;
    if (Agros2D::problem()->configView()->curvilinearElements)
    {
        for (int i = 0; i<edgeList.count(); i++)
        {
            if (edgeList[i].marker != -1)
            {
                // curve
                if (Agros2D::scene()->edges->at(edgeList[i].marker)->angle() > 0.0)
                {
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

                    curves.arc().push_back(XMLMesh::arc(edgeList[i].node[0], edgeList[i].node[1], rad2deg(angle)));
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

    // vertices
    XMLMesh::vertices_type vertices;
    for (int i = 0; i<nodeList.count(); i++)
        vertices.vertex().push_back(std::auto_ptr<XMLMesh::vertex>(new XMLMesh::vertex(QString::number(nodeList[i].x).toStdString(),
                                                                                       QString::number(nodeList[i].y).toStdString(), i)));

    // elements
    XMLSubdomains::elements_type elements;
    for (int i = 0; i<elementList.count(); i++)
        if (elementList[i].isUsed)
            if (elementList[i].isTriangle())
                elements.element().push_back(XMLSubdomains::triangle_type(elementList[i].node[0], elementList[i].node[1], elementList[i].node[2],
                        QString::number(elementList[i].marker).toStdString(), i));
            else
                elements.element().push_back(XMLSubdomains::quad_type(elementList[i].node[0], elementList[i].node[1], elementList[i].node[2],
                        QString::number(elementList[i].marker).toStdString(), i,
                        elementList[i].node[3]));

    // find edge neighbours
    // for each vertex list elements that it belogns to
    QList<QSet<int> > vertexElements;
    vertexElements.reserve(nodeList.count());
    for (int i = 0; i < nodeList.count(); i++)
        vertexElements.push_back(QSet<int>());

    for (int i = 0; i < elementList.count(); i++)
        if (elementList[i].isUsed)
            for(int elemNode = 0; elemNode < (elementList[i].isTriangle() ? 3 : 4); elemNode++)
                vertexElements[elementList[i].node[elemNode]].insert(i);

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
    XMLSubdomains::subdomains subdomains;

    if (Agros2D::problem()->fieldInfos().count() == 0)
    {
        // one domain
        XMLSubdomains::subdomain subdomain(Agros2D::problem()->fieldInfos().begin().value()->fieldId().toStdString());
        subdomains.subdomain().push_back(subdomain);
    }
    else
    {
        // more subdomains
        foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
        {
            XMLSubdomains::subdomain subdomain(fieldInfo->fieldId().toStdString());
            subdomain.elements().set(XMLSubdomains::subdomain::elements_type());
            subdomain.boundary_edges().set(XMLSubdomains::subdomain::boundary_edges_type());
            subdomain.inner_edges().set(XMLSubdomains::subdomain::inner_edges_type());

            for (int i = 0; i<elementList.count(); i++)
                if (elementList[i].isUsed && (Agros2D::scene()->labels->at(elementList[i].marker)->marker(fieldInfo) != SceneMaterialContainer::getNone(fieldInfo)))
                    subdomain.elements()->i().push_back(i);

            QList<int> unassignedEdges;
            for (int i = 0; i < edgeList.count(); i++)
            {
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
                    bool hasFieldBoundaryCondition = (Agros2D::scene()->edges->at(edgeList[i].marker)->hasMarker(fieldInfo)
                                                      && (Agros2D::scene()->edges->at(edgeList[i].marker)->marker(fieldInfo) != SceneBoundaryContainer::getNone(fieldInfo)));

                    if (numNeighWithField == 1)
                    {
                        // edge is on "boundary" of the field, should have boundary condition prescribed

                        if (!hasFieldBoundaryCondition)
                            if (!unassignedEdges.contains(edgeList[i].marker))
                                unassignedEdges.append(edgeList[i].marker);

                        subdomain.boundary_edges()->i().push_back(i);
                    }
                    else if (numNeighWithField == 2)
                    {
                        // todo: we could enforce not to have boundary conditions prescribed inside:
                        // assert(!hasFieldBoundaryCondition);
                        subdomain.inner_edges()->i().push_back(i);
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

            subdomains.subdomain().push_back(subdomain);
        }
    }

    XMLSubdomains::domain xmldomain(vertices, elements, edges, subdomains);
    xmldomain.curves().set(curves);

    std::string mesh_schema_location(Hermes::Hermes2D::Hermes2DApi.get_text_param_value(Hermes::Hermes2D::xmlSchemasDirPath));
    mesh_schema_location.append("/mesh_h2d_xml.xsd");
    ::xml_schema::namespace_info namespace_info_mesh("XMLMesh", mesh_schema_location);

    std::string domain_schema_location(Hermes::Hermes2D::Hermes2DApi.get_text_param_value(Hermes::Hermes2D::xmlSchemasDirPath));
    domain_schema_location.append("/subdomains_h2d_xml.xsd");
    ::xml_schema::namespace_info namespace_info_domain("XMLSubdomains", domain_schema_location);

    ::xml_schema::namespace_infomap namespace_info_map;
    namespace_info_map.insert(std::pair<std::basic_string<char>, xml_schema::namespace_info>("mesh", namespace_info_mesh));
    namespace_info_map.insert(std::pair<std::basic_string<char>, xml_schema::namespace_info>("domain", namespace_info_domain));

    std::ofstream out((cacheProblemDir() + "/initial.mesh").toStdString().c_str());
    ::xml_schema::flags parsing_flags = ::xml_schema::flags::dont_pretty_print;
    XMLSubdomains::domain_(out, xmldomain, namespace_info_map, "UTF-8", parsing_flags);
    out.close();

    return true;
}

bool MeshGenerator::prepare()
{
    LoopsInfo loopsInfo;
    try
    {
        loopsInfo = findLoops();
    }
    catch (AgrosMeshException& ame)
    {
        Agros2D::log()->printError(tr("Mesh generator"), ame.toString());
        return false;
    }

    QFile::remove(tempProblemFileName() + ".mesh");

    return true;
}
