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

#ifndef UTIL_LOOPS_H
#define UTIL_LOOPS_H

class SceneLabel;

#include "util.h"

struct LoopsNodeEdgeData
{
    LoopsNodeEdgeData();
    LoopsNodeEdgeData(int node, int edge, bool reverse,  double angle) : node(node), edge(edge), reverse(reverse), angle(angle), visited(false) {}
    int node;
    int edge;
    bool reverse;
    double angle; // to order edges going from node (anti)clockwise
    bool visited;
};

struct LoopsNode
{
    void insertEdge(int endNode, int edgeIdx, bool reverse,  double angle);
    bool hasUnvisited();
    LoopsNodeEdgeData startLoop();
    LoopsNodeEdgeData continueLoop(int previousNode);
    void setVisited(int index) {data[index].visited = true;}

    QList<LoopsNodeEdgeData> data;
};

struct LoopsGraph
{
    LoopsGraph(int numNodes);
    void addEdge(int startNode, int endNode, int edgeIdx, double angle);
    void print();

    QList<LoopsNode> data;
};

struct LoopsInfo
{
    QList<QList<LoopsNodeEdgeData> > loops;
    QMap<SceneLabel*, QList<int> > labelToLoops;
    QList<int> outsideLoops;

    inline void clear()
    {
        loops.clear();
        labelToLoops.clear();
        outsideLoops.clear();
    }
};

struct Triangle
{
    Triangle(const Point &a, const Point &b, const Point &c) : a(a), b(b), c(c)
    {
    }

    Point a, b, c;
};

// find loops
LoopsInfo findLoops();

// find polygon triangles
QMap<SceneLabel*, QList<Triangle> > findPolygonTriangles();

#endif // UTIL_LOOPS_H

