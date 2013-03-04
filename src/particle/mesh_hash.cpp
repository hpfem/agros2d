#include "mesh_hash.h"
#include "hermes2d.h"

MeshHashElement::MeshHashElement(Point p1, Point p2, int depth) : m_p1(p1), m_p2(p2), m_depth(depth)
{
    for(int i = 0; i < 2; i++)
        for(int j = 0; j < 2; j++)
            m_sons[i][j] = NULL;
    m_active = true;
}

MeshHashElement::~MeshHashElement()
{
    for(int i = 0; i < 2; i++)
        for(int j = 0; j < 2; j++)
            if(m_sons[i][j])
                delete m_sons[i][j];

}

bool MeshHashElement::belongs(Hermes::Hermes2D::Element *element)
{
    Point p1, p2;
    MeshHash::elementBoundingBox(element, p1, p2);
    return ((p1.x <= m_p2.x) && (p2.x >= m_p1.x) && (p1.y <= m_p2.y) && (p2.y >= m_p1.y));
}

void MeshHashElement::insert(Hermes::Hermes2D::Element *element)
{
    if(m_active)
    {
        if(! m_elements.contains(element))
            m_elements.push_back(element);

        if((m_elements.size() > MAX_ELEMENTS) && (m_depth < MAX_DEPTH))
        {
            m_active = false;
            double xx[3] = {m_p1.x, (m_p1.x + m_p2.x) / 2., m_p2.x};
            double yy[3] = {m_p1.y, (m_p1.y + m_p2.y) / 2., m_p2.y};
            for(int i = 0; i < 2; i++)
            {
                double x0 = xx[i];
                double x1 = xx[i+1];
                for(int j = 0; j < 2; j++)
                {
                    double y0 = yy[j];
                    double y1 = yy[j+1];

                    assert(m_sons[i][j] == NULL);
                    m_sons[i][j] = new MeshHashElement(Point(x0,y0), Point(x1,y1), m_depth + 1);

                    for(int elem = 0; elem < m_elements.size(); elem++)
                    {
                        if(m_sons[i][j]->belongs(m_elements[elem]))
                            m_sons[i][j]->insert(m_elements[elem]);
                    }
                }
            }

            m_elements.clear();
        }
    }
    else
    {
        for(int i = 0; i < 2; i++)
        {
            for(int j = 0; j < 2; j++)
            {
                assert(m_sons[i][j]);
                if(m_sons[i][j]->belongs(element))
                    m_sons[i][j]->insert(element);
            }
        }
    }

}

Hermes::Hermes2D::Element* MeshHashElement::getElement(double x, double y)
{
    if(m_active)
    {
        double x_ref, y_ref;
        foreach(Hermes::Hermes2D::Element* element, m_elements)
        {
            if(Hermes::Hermes2D::RefMap::is_element_on_physical_coordinates(element, x, y, &x_ref, &y_ref))
                return element;
        }
        return NULL;
    }
    else
    {
        Hermes::Hermes2D::Element* element;
        for(int i = 0; i < 2; i++)
        {
            for(int j = 0; j < 2; j++)
            {
                element = m_sons[i][j]->getElement(x,y);
                if(element)
                    return element;
            }
        }
        return NULL;
    }
}

void MeshHash::elementBoundingBox(Hermes::Hermes2D::Element *element, Point &p1, Point &p2)
{
    p1.x = p2.x = element->vn[0]->x;
    p1.y = p2.y = element->vn[0]->y;

    for(int i = 1; i < element->get_nvert(); i++)
    {
        double xx = element->vn[i]->x;
        double yy = element->vn[i]->y;
        if(xx > p2.x)
            p2.x = xx;
        if(xx < p1.x)
            p1.x = xx;
        if(yy > p2.y)
            p2.y = yy;
        if(yy < p1.y)
            p1.y = yy;
    }

    if(element->is_curved())
    {
        // todo: should be improved
        Point diameter = p2 - p1;
        p2 = p2 + diameter;
        p1 = p1 - diameter;
    }
}



MeshHash::MeshHash(const Hermes::Hermes2D::Mesh *mesh)
{
    // find bounding box of the whole mesh
    Point mesh_p1, mesh_p2;
    bool first = true;
    Hermes::Hermes2D::Node* n;
    for_all_vertex_nodes(n, mesh)
    {
        if(first)
        {
            mesh_p1.x = mesh_p2.x = n->x;
            mesh_p1.y = mesh_p2.y = n->y;
            first = false;
        }
        else
        {
            if(n->x > mesh_p2.x)
                mesh_p2.x = n->x;
            if(n->x < mesh_p1.x)
                mesh_p1.x = n->x;
            if(n->y > mesh_p2.y)
                mesh_p2.y = n->y;
            if(n->y < mesh_p1.y)
                mesh_p1.y = n->y;
        }
    }

    // create grid
    double interval_len_x = (mesh_p2.x - mesh_p1.x) / GRID_SIZE;
    double interval_len_y = (mesh_p2.y - mesh_p1.y) / GRID_SIZE;

    intervals_x[0] = mesh_p1.x;
    intervals_y[0] = mesh_p1.y;

    for(int i = 1; i < GRID_SIZE; i++)
    {
        intervals_x[i] = intervals_x[i-1] + interval_len_x;
        intervals_y[i] = intervals_y[i-1] + interval_len_y;
    }

    intervals_x[GRID_SIZE] = mesh_p2.x;
    intervals_y[GRID_SIZE] = mesh_p2.y;

    for(int i = 0; i < GRID_SIZE; i++)
    {
        for(int j = 0; j < GRID_SIZE; j++)
        {
            m_grid[i][j] = new MeshHashElement(Point(intervals_x[i], intervals_y[j]), Point(intervals_x[i+1], intervals_y[j+1]));
        }
    }

    // assign elements
    Hermes::Hermes2D::Element *element;
    Point p1, p2;
    int x_min, x_max, y_min, y_max;
    for_all_active_elements(element, mesh)
    {
        elementBoundingBox(element, p1, p2);
        x_min = 0;
        while(intervals_x[x_min + 1] < p1.x)
            x_min++;

        x_max = GRID_SIZE - 1;
        while(intervals_x[x_max] > p2.x)
            x_max--;

        y_min = 0;
        while(intervals_y[y_min + 1] < p1.y)
            y_min++;

        y_max = GRID_SIZE - 1;
        while(intervals_y[y_max] > p2.y)
            y_max--;

        assert((x_max >= x_min) && (y_max >= y_min));

        for(int i = x_min; i <= x_max; i++)
        {
            for(int j = y_min; j <= y_max; j++)
            {
                assert(m_grid[i][j]->belongs(element));
                m_grid[i][j]->insert(element);
            }
        }
    }
}

MeshHash::~MeshHash()
{
    for(int i = 0; i < GRID_SIZE; i++)
    {
        for(int j = 0; j < GRID_SIZE; j++)
        {
            delete m_grid[i][j];
        }
    }

}

Hermes::Hermes2D::Element* MeshHash::getElement(double x, double y)
{
    int i = 0;
    while((i < GRID_SIZE) && (intervals_x[i+1] < x))
        i++;

    int j = 0;
    while((j < GRID_SIZE) && (intervals_y[j+1] < y))
        j++;

    // this means that x or y is outside mesh, but it can hapen
    if((i >= GRID_SIZE) || (j >= GRID_SIZE))
        return NULL;
    else
        return m_grid[i][j]->getElement(x,y);
}
