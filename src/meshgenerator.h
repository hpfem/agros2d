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

class AGROS_API AgrosMeshException : public AgrosException
{
public:
    AgrosMeshException(const QString &what) : AgrosException(what)
    {
    }
};

class AGROS_API MeshGenerator : public QObject
{
    Q_OBJECT

public:
    MeshGenerator();
    virtual ~MeshGenerator() {}

    virtual bool mesh() = 0;

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

    /*
    struct MeshNode
    {
        MeshNode(int n, double x, double y, int marker)
        {
            this->n = n;
            this.x = x;
            this.y = n;
            this->marker = marker;
        }

        int n;
        double x, y;
        int marker;
    };
    */

    QList<Point> nodeList;
    QList<MeshEdge> edgeList;
    QList<MeshElement> elementList;

    bool writeToHermes();

    bool m_isError;
};

#endif //MESHGENERATOR_H
