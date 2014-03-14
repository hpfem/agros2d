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

#ifndef MESHGENERATOR_H
#define MESHGENERATOR_H

#include "util.h"
#include "hermes2d/field.h"
#include "mesh.h"
#include "util/loops.h"
#ifdef Q_WS_X11
#include <tr1/memory>
#endif

namespace XMLSubdomains
{
    class domain;
};

class AGROS_LIBRARY_API MeshGenerator : public QObject
{
    Q_OBJECT

public:
    MeshGenerator();
    virtual ~MeshGenerator();

    virtual bool mesh() = 0;

    inline Hermes::vector<Hermes::Hermes2D::MeshSharedPtr> meshes() { return m_meshes; }

protected:
    struct MeshEdge
        {
            MeshEdge()
            {
                this->node[0] = -1;
                this->node[1] = -1;
                this->marker = -1;

                this->isActive = true;
                this->isUsed = true;

                this->neighElem[0] = -1;
                this->neighElem[1] = -1;
            }

            MeshEdge(int node_1, int node_2, int marker)
            {
                this->node[0] = node_1;
                this->node[1] = node_2;
                this->marker = marker;

                this->isActive = true;
                this->isUsed = true;

                this->neighElem[0] = -1;
                this->neighElem[1] = -1;
            }

            bool contains(int node)
            {
                for(int i = 0; i < 2; i++)
                    if(this->node[i] == node)
                        return true;
                return false;
            }

            int node[2], marker;
            bool isActive, isUsed;
            int neighElem[2];
        };

        struct MeshElement
        {
            MeshElement()
            {
                this->node[0] = -1;
                this->node[1] = -1;
                this->node[2] = -1;
                this->node[3] = -1;
                this->marker = -1;

                this->isActive = true;
                this->isUsed = true;
            }

            MeshElement(int node_1, int node_2, int node_3, int marker)
            {
                this->node[0] = node_1;
                this->node[1] = node_2;
                this->node[2] = node_3;
                this->node[3] = -1;
                this->marker = marker;

                this->isActive = true;
                this->isUsed = true;
            }

            MeshElement(int node_1, int node_2, int node_3, int node_4, int marker)
            {
                this->node[0] = node_1;
                this->node[1] = node_2;
                this->node[2] = node_3;
                this->node[3] = node_4;
                this->marker = marker;

                this->isActive = true;
                this->isUsed = true;
            }

            bool contains(int node)
            {
                for(int i = 0; i < isTriangle() ? 3 : 4; i++)
                    if(this->node[i] == node)
                        return true;
                return false;
            }

            inline bool isTriangle() const { return (node[3] == -1); }

            int node[4], marker;
            bool isActive, isUsed;

            int neigh[3];
        };

        struct MeshNode
        {
            MeshNode()
            {
                this->n = -1;
                this->x = -1;
                this->y = -1;
                this->marker = -1;
            }

            MeshNode(int n, double x, double y, int marker)
            {
                this->n = n;
                this->x = x;
                this->y = y;
                this->marker = marker;
            }

            int n;
            double x, y;
            int marker;
        };

        struct MeshLabel
        {
            MeshLabel()
            {
                this->n = -1;
                this->x = -1;
                this->y = -1;
                this->marker = -1;
                this->area = -1;
            }

            MeshLabel(int n, double x, double y, int marker, double area)
            {
                this->n = n;
                this->x = x;
                this->y = y;
                this->marker = marker;
                this->area = area;
            }

            int n;
            double x, y;
            int marker;
            double area;
        };

    QList<Point> nodeList;
    QList<MeshEdge> edgeList;
    QList<MeshElement> elementList;

    /// Complete method translating the internal generator structures into m_meshes.
    void writeToHermes();

    /// Utility method serving the purpose of (potential) multi-mesh setup.
    /// Translates the internal structures into the global mesh (of which every other mesh in the system is a submesh of).
    /// \param[out] global_mesh The global mesh structure being filled by this method.
    void writeTemporaryGlobalMeshToHermes(Hermes::Hermes2D::MeshSharedPtr global_mesh);

    /// Fills MeshEdge::neighElem structures for detecting subdomain boundaries etc.
    void fillNeighborStructures();

    /// Updates vertex nodes coordinates according to curvature on edges.
    void moveNodesOnCurvedEdges();

    /// Calculate the counts of elements, edges for a subdomain.
    void getDataCountsForSingleSubdomain(FieldInfo* fieldInfo, int& element_number_count, int& boundary_edge_number_count, int& inner_edge_number_count);

    bool prepare();

    bool m_isError;
    QSharedPointer<QProcess> m_process;
    Hermes::vector<Hermes::Hermes2D::MeshSharedPtr> m_meshes;
};

#endif //MESHGENERATOR_H
