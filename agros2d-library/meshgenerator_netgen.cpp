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

#include "meshgenerator_netgen.h"

#include "util/global.h"

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

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "util/loops.h"

#include "nglib.h"
#include "meshing.hpp"
#include "basegeom.hpp"
#include "geometry2d.hpp"

MeshGeneratorNetgen::MeshGeneratorNetgen()
    : MeshGenerator()
{
}


bool MeshGeneratorNetgen::mesh()
{
    m_isError = !prepare();

    // create triangle files
    if (writeToNetgen())
    {
        // Agros2D::log()->printDebug(tr("Mesh generator"), tr("Mesh files were created"));

        // FIX: remove
        m_isError = false;

        // convert triangle mesh to hermes mesh
        // if (!readNetgenMeshFormat())
        {
            // m_isError = true;
            // QFile::remove(Agros2D::problem()->config()->fileName() + ".in2d");
        }
    }
    else
    {
        m_isError = true;
    }

    return !m_isError;
}

bool MeshGeneratorNetgen::writeToNetgen()
{
    // basic check
    if (Agros2D::scene()->nodes->length() < 3)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Invalid number of nodes (%1 < 3)").arg(Agros2D::scene()->nodes->length()));
        return false;
    }
    if (Agros2D::scene()->edges->length() < 3)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Invalid number of edges (%1 < 3)").arg(Agros2D::scene()->edges->length()));
        return false;
    }

    QFile file(tempProblemFileName() + ".in2d");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Could not create NETGEN geometry file (%1)").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);

    QString outNodes;
    int nodesCount = 0;
    for (int i = 0; i < Agros2D::scene()->nodes->length(); i++)
    {
        outNodes += QString("%1\t%2\t%3\n").
               arg(i + 1).
               arg(Agros2D::scene()->nodes->at(i)->point().x).
               arg(Agros2D::scene()->nodes->at(i)->point().y);

        nodesCount++;
    }

    QList<MeshEdge> inEdges;
    QString outEdges;
    for (int i = 0; i<Agros2D::scene()->edges->length(); i++)
    {
        if (Agros2D::scene()->edges->at(i)->angle() == 0)
        {
            outEdges += QString("%1\t%2\t%3\t%4\t%5\t%6\n").
                   arg(Agros2D::scene()->edges->at(i)->leftLabelIdx() + 1).
                   arg(Agros2D::scene()->edges->at(i)->rightLabelIdx() + 1).
                   arg(2).
                   arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeStart()) + 1).
                   arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeEnd()) + 1).
                   arg(QString("-bc=%1").arg(i+1));
        }
        else
        {
            // arc
            // add center
            nodesCount++;
            Point center = centerPoint(Agros2D::scene()->edges->at(i)->nodeEnd()->point(),
                                       Agros2D::scene()->edges->at(i)->nodeStart()->point(),
                                       Agros2D::scene()->edges->at(i)->angle());

            outNodes += QString("%1\t%2\t%3\n").
                   arg(nodesCount).
                   arg(center.x).
                   arg(center.y);

            outEdges += QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\n").
                   arg(Agros2D::scene()->edges->at(i)->leftLabelIdx() + 1).
                   arg(Agros2D::scene()->edges->at(i)->rightLabelIdx() + 1).
                   arg(3).
                   arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeStart()) + 1).
                   arg(nodesCount).
                   arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeEnd()) + 1).
                   arg(QString("-bc=%1").arg(i+1));
        }
    }

    /*
    QList<MeshNode> inHoles;
    holesCount = 0;
    foreach (SceneLabel *label, Agros2D::scene()->labels->items())
    {
        if (label->markersCount() == 0)
        {
            inHoles.append(MeshNode(holesCount,
                                    label->point().x,
                                    label->point().y,
                                    -1));

            holesCount++;
        }
    }
    */

    // labels

    QString outLabels;
    int labelsCount = 0;
    foreach (SceneLabel *label, Agros2D::scene()->labels->items())
    {
        if (label->markersCount() > 0)
        {
            outLabels += QString("%1\tdomain%2\t\t%3\n").
                   arg(labelsCount + 1).
                   arg(Agros2D::scene()->labels->items().indexOf(label) + 1).
                   arg(QString("-maxh=%1").arg((label->area() > 0) ? label->area() : 1e22));

            labelsCount++;
        }
    }

    // file
    out << QString("# 2D geometry, version 2\nsplinecurves2dv2\n\n");
    out << QString("# a global grading factor\n");
    out << QString("1\n\n");

    // nodes
    out << QString("# the points (point number, x and y coordinates)\n");
    out << QString("points\n");
    out << outNodes;
    out << QString("\n");

    // edges
    out << QString("# boundary curves\n");
    out << QString("# dl ... sub-domain nr on left side\n");
    out << QString("# dr ... sub-domain nr on right side\n");
    out << QString("# np ... curve is given by 2 (or 3) points\n");
    out << QString("# p1, p2 ... points defining the curve\n");
    out << QString("# flagslist\n");
    out << QString("segments\n");
    out << outEdges;
    out << QString("\n");

    // labels
    out << QString("materials\n");
    out << outLabels;
    out << QString("\n");

    file.waitForBytesWritten(0);
    file.close();

    // netgen::SplineGeometry2d * geom = new netgen::SplineGeometry2d();
    // geom->Load("x");

    return true;
}

bool MeshGeneratorNetgen::readNetgenMeshFormat()
{
    nodeList.clear();
    edgeList.clear();
    elementList.clear();


    writeToHermes();

    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    return true;
}

