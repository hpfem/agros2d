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

class Scene;
class SceneLabel;
class SceneEdge;

#include "util.h"

class AGROS_API LoopsInfo : public QObject
{
    Q_OBJECT

public:
    LoopsInfo(Scene *scene);
    virtual ~LoopsInfo() {}

    enum Intersection
    {
        Intersection_Uncertain,
        Intersection_Left,
        Intersection_Right,
        Intersection_Both,
        Intersection_No
    };

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

    struct Triangle
    {
        Triangle(const Point &a, const Point &b, const Point &c) : a(a), b(b), c(c)
        {
        }

        Point a, b, c;
    };

    inline QList<QList<LoopsNodeEdgeData> > loops() const { return m_loops; }
    inline QList<int> outsideLoops() const { return m_outsideLoops; }
    inline QMap<SceneLabel*, QList<int> > labelLoops() const { return m_labelLoops; }

    // polygon triangles
    inline QMap<SceneLabel*, QList<Triangle> > polygonTriangles() const { return m_polygonTriangles; }

    inline bool isProcessPolygonError() { return m_isProcessPolygonError; }

public slots:
    void processLoops();
    void processPolygonTriangles();

    void clear();

private:
    Scene *m_scene;

    bool m_isProcessPolygonError;

    QList<QList<LoopsNodeEdgeData> > m_loops;
    QMap<SceneLabel*, QList<int> > m_labelLoops;
    QList<int> m_outsideLoops;

    QMap<SceneLabel*, QList<Triangle> > m_polygonTriangles;

    Intersection intersects(Point point, double tangent, SceneEdge* edge);
    Intersection intersects(Point point, double tangent, SceneEdge* edge, Point& intersection);
    int intersectionsParity(Point point, QList<LoopsNodeEdgeData> loop);
    bool isInsideSeg(double angleSegStart, double angleSegEnd, double angle);

    QList<Triangle> triangulateLabel(const QList<Point> &polyline, const QList<QList<Point> > &holes);
    int windingNumber(Point point, QList<LoopsNodeEdgeData> loop);
    bool areSameLoops(QList<LoopsNodeEdgeData> loop1, QList<LoopsNodeEdgeData> loop2);
    bool areEdgeDuplicities(QList<LoopsNodeEdgeData> loop);
    int longerLoop(int idx1, int idx2);
    bool shareEdge(int idx1, int idx2);
    void switchOrientation(int idx);
    void addEdgePoints(QList<Point> *polyline, const SceneEdge &edge, bool reverse = false);
};


#endif // UTIL_LOOPS_H

