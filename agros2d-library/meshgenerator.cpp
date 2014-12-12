// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

// deal.ii
#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>

#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_in.h>
#include <deal.II/grid/grid_reordering.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/dofs/dof_accessor.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_values.h>
#include <deal.II/base/quadrature_lib.h>

#include <deal.II/base/function.h>
#include <deal.II/numerics/vector_tools.h>
#include <deal.II/numerics/matrix_tools.h>

#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/compressed_sparsity_pattern.h>
#include <deal.II/lac/solver_cg.h>
#include <deal.II/lac/sparse_direct.h>
#include <deal.II/lac/precondition.h>

#include <deal.II/grid/grid_refinement.h>
#include <deal.II/numerics/error_estimator.h>

#include <deal.II/numerics/fe_field_function.h>
#include <deal.II/numerics/data_out.h>

#include <deal.II/base/timer.h>

#include <boost/config.hpp>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

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

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"

MeshGenerator::MeshGenerator() : QObject()
{    
}

MeshGenerator::~MeshGenerator()
{
}

void MeshGenerator::elementsSharingNode(MeshElement* e, Point* node, QList<MeshElement*>& elements)
{
    if(!elements.contains(e))
    {
        elements.append(e);
        for(int i = 0; i < 3; i++)
        {
            for(int j = 0; j < 3; j++)
            {
                if(e->neigh[i] != -1 && &this->nodeList[this->elementList[e->neigh[i]].node[j]] == node)
                {
                    elementsSharingNode(&this->elementList[e->neigh[i]], node, elements);
                    break;
                }
            }
        }
    }
}

bool MeshGenerator::getDeterminant(MeshElement* element)
{

    bool is_triangle = element->node[3] == -1;

    if(!is_triangle)
        throw AgrosException("Shifting nodes using get_determinant works only for triangles.");

    double x[3], y[3];
    for(int i = 0; i < 3; i++)
    {
        x[i] = this->nodeList[element->node[i]].x;
        y[i] = this->nodeList[element->node[i]].y;
    }

    double determinant = x[0]*( y[1] - y[2] ) - x[1]*( y[0] - y[2] ) + x[2]*( y[0] - y[1] );

    // std::cout << "E[" << x[0] << "," << y[0] << "],[" << x[1] << "," << y[1] << "],[" << x[2] << "," << y[2] << "]" << ",   D=" << determinant << std::endl;

    return (determinant > 0);
}

// Used in moveNode - this actually moves the node (if it has not been moved before).
void MeshGenerator::performActualNodeMove(Point* node, QList<Point*>& already_moved_nodes, const double x_displacement, const double y_displacement, const double multiplier)
{
    if(already_moved_nodes.contains(node))
        return;
    else
    {
        double x = node->x;
        double y = node->y;

        node->x += multiplier * x_displacement;
        node->y += multiplier * y_displacement;

        std::cout << "Shift: multiplier=" << multiplier << ", x=" << x << ", y=" << y << ", shifted to: x=" << node->x << ", y=" << node->y << "\n";

        already_moved_nodes.append(node);
    }
}

void MeshGenerator::moveNode(MeshElement* element, Point* node, QList<Point*>& already_moved_nodes, const double x_displacement, const double y_displacement, const double multiplier, const QList<std::pair<MeshElement*, bool> >& determinants)
{
    // We have to stop updating somewhere.
    if(multiplier < .01)
        return;

    /* For debugging purposes
    for (int i = 0; i < determinants.count(); i++)
    {
        double x[3], y[3];
        for(int j = 0; j < 3; j++)
        {
            x[j] = this->nodeList[determinants[i].first->node[j]].x;
            y[j] = this->nodeList[determinants[i].first->node[j]].y;
        }
        std::cout << "Old - E[" << x[0] << "," << y[0] << "],[" << x[1] << "," << y[1] << "],[" << x[2] << "," << y[2] << "]" << ",   D=" << determinants[i].second << std::endl;
    }

    */

    performActualNodeMove(node, already_moved_nodes, x_displacement, y_displacement, multiplier);

    for (int i = 0; i < determinants.count(); i++)
    {
        bool new_determinant = getDeterminant(determinants[i].first);
        bool old_determinant = (determinants[i].second > 0);
        // If we broke the element orientation, we need to recursively continue fixing the elements (vertices)
        if(new_determinant != old_determinant)
        {
            for(int j = 0; j < 3; j++)
            {
                if(!already_moved_nodes.contains(&this->nodeList[determinants[i].first->node[j]]))
                {
                    // Prepare the data structures to pass
                    QList<MeshElement*> elements_to_pass;
                    QList<std::pair<MeshElement*, bool> > determinants_to_pass;
                    // Elements sharing the vertex that broke something.
                    elementsSharingNode(determinants[i].first, &this->nodeList[determinants[i].first->node[j]], elements_to_pass);
                    // Calculate the determinants now - to check if this fix won't break something else.
                    for (int ifound_elems = 0; ifound_elems < elements_to_pass.count(); ifound_elems++)
                        determinants_to_pass.append(std::pair<MeshElement*, bool>(elements_to_pass[ifound_elems], getDeterminant(elements_to_pass[ifound_elems])));

                    // We have to stop updating somewhere - this reduces the multiplier multiplying the displacement of vertices.
                    // Very important is the right constant.
                    double new_multiplier = multiplier * 0.9;
                    this->moveNode(determinants[i].first, &this->nodeList[determinants[i].first->node[j]], already_moved_nodes, x_displacement, y_displacement, new_multiplier, determinants_to_pass);
                }
            }
        }
    }
}

void MeshGenerator::moveNodesOnCurvedEdges()
{
    // First move the boundary elements - they are easy, they can not distort any element that lies between the boundary and themselves.
    for (int i = 0; i < edgeList.count(); i++)
    {
        MeshEdge edge = edgeList[i];

        if (edge.marker == -1)
            continue;

        if (!(Agros2D::scene()->edges->at(edge.marker)->angle() > 0.0 && Agros2D::scene()->edges->at(edge.marker)->isCurvilinear()))
            continue;

        // Only boundary now.
        if(edge.neighElem[0] == -1 || edge.neighElem[1] == -1)
        {
            // assert(edge.marker >= 0); // markers changed to marker - 1, check...
            if (edge.marker != -1)
            {
                // curve
                if (Agros2D::scene()->edges->at(edge.marker)->angle() > 0.0 &&
                        Agros2D::scene()->edges->at(edge.marker)->isCurvilinear())
                {
                    // Nodes.
                    Point* node[2] = { &nodeList[edge.node[0]], &nodeList[edge.node[1]]};
                    // Center
                    Point center = Agros2D::scene()->edges->at(edge.marker)->center();
                    // Radius
                    double radius = Agros2D::scene()->edges->at(edge.marker)->radius();

                    // First node handling
                    double pointAngle1 = atan2(center.y - node[0]->y, center.x - node[0]->x) - M_PI;
                    node[0]->x = center.x + radius * cos(pointAngle1);
                    node[0]->y = center.y + radius * sin(pointAngle1);

                    // Second node handling
                    double pointAngle2 = atan2(center.y - node[1]->y, center.x - node[1]->x) - M_PI;
                    node[1]->x = center.x + radius * cos(pointAngle2);
                    node[1]->y = center.y + radius * sin(pointAngle2);
                }
            }
        }
    }

    // Now move the problematic ones
    for (int i = 0; i < edgeList.count(); i++)
    {
        MeshEdge edge = edgeList[i];

        if (edge.marker == -1)
            continue;

        if (!(Agros2D::scene()->edges->at(edge.marker)->angle() > 0.0 && Agros2D::scene()->edges->at(edge.marker)->isCurvilinear()))
            continue;

        // Boundary has been taken care of.
        if(edge.neighElem[0] == -1 || edge.neighElem[1] == -1)
            continue;

        // assert(edge.marker >= 0); // markers changed to marker - 1, check...
        if (edge.marker != -1)
        {
            // curve
            if (Agros2D::scene()->edges->at(edge.marker)->angle() > 0.0 &&
                    Agros2D::scene()->edges->at(edge.marker)->isCurvilinear())
            {
                // Nodes.
                Point* node[2] = { &nodeList[edge.node[0]], &nodeList[edge.node[1]]};
                // Center
                Point center = Agros2D::scene()->edges->at(edge.marker)->center();
                // Radius
                double radius = Agros2D::scene()->edges->at(edge.marker)->radius();

                // Handle the nodes recursively using moveNode()
                for(int inode = 0; inode < 2; inode++)
                {
                    double pointAngle = atan2(center.y - node[inode]->y, center.x - node[inode]->x) - M_PI;
                    double x_displacement = center.x + radius * cos(pointAngle) - node[inode]->x;
                    double y_displacement = center.y + radius * sin(pointAngle) - node[inode]->y;

                    // Initialization for the one node algorithm
                    QList<Point*> already_moved_nodes;
                    QList<MeshElement*> elements_to_pass;
                    QList<std::pair<MeshElement*, bool> > determinants_to_pass;

                    // Find elements sharing this node
                    elementsSharingNode(&this->elementList[edge.neighElem[0]], node[inode], elements_to_pass);

                    // Calculate determinants for them
                    for (int ifound_elems = 0; ifound_elems < elements_to_pass.count(); ifound_elems++)
                        determinants_to_pass.append(std::pair<MeshElement*, bool>(elements_to_pass[ifound_elems], getDeterminant(elements_to_pass[ifound_elems])));

                    // Start the algorithm
                    moveNode(&this->elementList[edge.neighElem[0]], node[inode], already_moved_nodes, x_displacement, y_displacement, 1.0, determinants_to_pass);
                }
            }
        }
    }
}

void MeshGenerator::fillNeighborStructures()
{
    QList<QSet<int> > vertexElements;
    vertexElements.reserve(nodeList.count());
    for (int i = 0; i < nodeList.count(); i++)
        vertexElements.push_back(QSet<int>());

    for (int i = 0; i < elementList.count(); i++)
        if (elementList[i].isUsed)
            for (int elemNode = 0; elemNode < (elementList[i].isTriangle() ? 3 : 4); elemNode++)
                vertexElements[elementList[i].node[elemNode]].insert(i);

    for (int i = 0; i < edgeList.count(); i++)
    {
        if (edgeList[i].isUsed && edgeList[i].marker != -1)
        {
            QSet<int> neighbours = vertexElements[edgeList[i].node[0]];
            neighbours.intersect(vertexElements[edgeList[i].node[1]]);
            assert((neighbours.size() > 0) && (neighbours.size() <= 2));
            edgeList[i].neighElem[0] = neighbours.values()[0];
            if (neighbours.size() == 2)
                edgeList[i].neighElem[1] = neighbours.values()[1];
        }
    }
}

void MeshGenerator::writeTodealii()
{
    foreach(FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        dealii::Triangulation<2> *triangulation = new dealii::Triangulation<2>();

        // vertices
        std::vector<dealii::Point<2> > vertices;
        for (int vertex_i = 0; vertex_i < nodeList.count(); vertex_i++)
        {
            vertices.push_back(dealii::Point<2>(
                                   nodeList[vertex_i].x,
                                   nodeList[vertex_i].y));
        }

        // elements
        std::vector<dealii::CellData<2> > cells;
        for (int element_i = 0; element_i < elementList.count(); element_i++)
        {
            if (elementList[element_i].isUsed &&
                    (Agros2D::scene()->labels->at(elementList[element_i].marker)->marker(fieldInfo) != SceneMaterialContainer::getNone(fieldInfo)))
            {
                if (elementList[element_i].isTriangle())
                {
                    assert("triangle");
                }
                else
                {
                    dealii::CellData<2> cell;
                    cell.vertices[0] = elementList[element_i].node[0];
                    cell.vertices[1] = elementList[element_i].node[1];
                    cell.vertices[2] = elementList[element_i].node[2];
                    cell.vertices[3] = elementList[element_i].node[3];
                    cell.material_id = elementList[element_i].marker + 1;

                    cells.push_back(cell);
                }
            }
        }

        // boundary markers
        dealii::SubCellData subcelldata;
        for (int edge_i = 0; edge_i < edgeList.count(); edge_i++)
        {
            if (edgeList[edge_i].marker == -1)
                continue;

            if (Agros2D::scene()->edges->at(edgeList[edge_i].marker)->marker(fieldInfo) == SceneBoundaryContainer::getNone(fieldInfo))
                continue;

            dealii::CellData<1> cell_data;
            cell_data.vertices[0] = edgeList[edge_i].node[0];
            cell_data.vertices[1] = edgeList[edge_i].node[1];
            cell_data.boundary_id = edgeList[edge_i].marker + 1;

            subcelldata.boundary_lines.push_back(cell_data);
        }

        dealii::GridTools::delete_unused_vertices (vertices, cells, subcelldata);
        dealii::GridReordering<2>::invert_all_cells_of_negative_grid (vertices, cells);
        dealii::GridReordering<2>::reorder_cells (cells);

        triangulation->create_triangulation_compatibility (vertices, cells, subcelldata);

        m_triangulations[fieldInfo] = triangulation;

        // save to disk
        QString fnMesh = QString("%1/%2_initial.msh").arg(cacheProblemDir()).arg(fieldInfo->fieldId());
        std::ofstream ofsMesh(fnMesh.toStdString());
        boost::archive::binary_oarchive sbMesh(ofsMesh);
        triangulation->save(sbMesh, 0);
    }
}

bool MeshGenerator::prepare()
{
    try
    {
        Agros2D::scene()->loopsInfo()->processLoops();
    }
    catch (AgrosMeshException& ame)
    {
        Agros2D::log()->printError(tr("Mesh generator"), ame.toString());
        return false;
    }

    QFile::remove(tempProblemFileName() + ".msh");

    return true;
}
