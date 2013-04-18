#ifndef MESHHASH_H
#define MESHHASH_H

#include "util.h"
#include "util/global.h"

namespace Hermes
{
namespace Hermes2D
{
class Element;
}
}

class MeshHashElement
{
public:
    MeshHashElement(Point p1, Point p2, int depth = 0);
    ~MeshHashElement();
    inline bool belongs(Point p) { return (p.x >= m_p1.x) && (p.x <= m_p2.x) && (p.y <= m_p1.y) && (p.y >= m_p2.y); }
    bool belongs(Hermes::Hermes2D::Element* element);
    void insert(Hermes::Hermes2D::Element* element);

    Hermes::Hermes2D::Element* getElement(double x, double y);

private:
    Point m_p1, m_p2;
    QList<Hermes::Hermes2D::Element*> m_elements;
    MeshHashElement* m_sons[2][2];
    int m_depth;

    static const int MAX_ELEMENTS = 50;
    static const int MAX_DEPTH = 10;
    bool m_active;
};

#define GRID_SIZE  30

class MeshHash
{
public:
    MeshHash(const MeshSharedPtr mesh);
    ~MeshHash();

    // smallest box interval_x X interval_y in which element is contained. If element is curvilinear, has to be made larger
    // if we knew more about the shape of curvilinear element, this increase could be smaller
    static void elementBoundingBox(Hermes::Hermes2D::Element* element, Point& p1, Point& p2);

    Hermes::Hermes2D::Element* getElement(double x, double y);

private:
    MeshHashElement* m_grid[GRID_SIZE][GRID_SIZE];

    double intervals_x[GRID_SIZE + 1];
    double intervals_y[GRID_SIZE + 1];
};


#endif // MESHHASH_H
