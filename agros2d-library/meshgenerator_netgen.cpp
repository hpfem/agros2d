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

#define debugIN2D

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

        // convert triangle mesh to hermes mesh
        if (!readNetgenMeshFormat())
        {
            m_isError = true;
            //  QFile::remove(Agros2D::problem()->config()->fileName() + ".in2d");
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

    geom = new SplineGeometry2dAgros();

#ifdef debugIN2D
    QFile file(tempProblemFileName() + ".in2d");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Could not create NETGEN geometry file (%1)").arg(file.errorString()));
        return false;
    }

    QString outNodes;
    QString outEdges;
    QString outLabels;

    int nodesCount = 0;
    int labelsCount = 0;
#endif

    for (int i = 0; i < Agros2D::scene()->nodes->length(); i++)
    {
        geom->AppendPoint(netgen::Point<2>(Agros2D::scene()->nodes->at(i)->point().x,
                                           Agros2D::scene()->nodes->at(i)->point().y));

#ifdef debugIN2D
        outNodes += QString("%1\t%2\t%3\n").
                arg(i + 1).
                arg(Agros2D::scene()->nodes->at(i)->point().x).
                arg(Agros2D::scene()->nodes->at(i)->point().y);

        nodesCount++;
#endif
    }

    for (int i = 0; i<Agros2D::scene()->edges->length(); i++)
    {
        netgen::SplineSeg<2> *spline = NULL;

        SceneEdge *edge = Agros2D::scene()->edges->at(i);

        if (edge->angle() == 0)
        {
            spline = new netgen::LineSeg<2>(geom->GetPoint(Agros2D::scene()->nodes->items().indexOf(edge->nodeStart())),
                                            geom->GetPoint(Agros2D::scene()->nodes->items().indexOf(edge->nodeEnd())));

#ifdef debugIN2D
            outEdges += QString("%1\t%2\t%3\t%4\t%5\t%6\n").
                    arg(edge->leftLabelIdx() + 1).
                    arg(edge->rightLabelIdx() + 1).
                    arg(2).
                    arg(Agros2D::scene()->nodes->items().indexOf(edge->nodeStart()) + 1).
                    arg(Agros2D::scene()->nodes->items().indexOf(edge->nodeEnd()) + 1).
                    arg(QString("-bc=%1").arg(i+1));
#endif
        }
        else
        {
            // arc
            // add center

            Point center = centerPoint(edge->nodeEnd()->point(),
                                       edge->nodeStart()->point(),
                                       edge->angle());

            Point midpoint = (edge->nodeStart()->point() + edge->nodeEnd()->point()) / 2;
            Point perpendicular = Point(-(edge->nodeEnd()->point() - midpoint).y, (edge->nodeEnd()->point() - midpoint).x);

            double h = (edge->nodeEnd()->point() - edge->nodeStart()->point()).magnitude() / 2.0 / (center - midpoint).magnitude();
            Point H = midpoint - perpendicular * h;

            geom->AppendPoint(netgen::Point<2>(H.x, H.y));

            spline = new netgen::SplineSeg3<2>(geom->GetPoint(Agros2D::scene()->nodes->items().indexOf(edge->nodeStart())),
                                               geom->GetPoint(geom->GetNP() - 1),
                                               geom->GetPoint(Agros2D::scene()->nodes->items().indexOf(edge->nodeEnd())));

#ifdef debugIN2D
            nodesCount++;

            outNodes += QString("%1\t%2\t%3\n").
                    arg(nodesCount).
                    arg(H.x).
                    arg(H.y);

            outEdges += QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\n").
                    arg(edge->leftLabelIdx() + 1).
                    arg(edge->rightLabelIdx() + 1).
                    arg(3).
                    arg(Agros2D::scene()->nodes->items().indexOf(edge->nodeStart()) + 1).
                    arg(nodesCount).
                    arg(Agros2D::scene()->nodes->items().indexOf(edge->nodeEnd()) + 1).
                    arg(QString("-bc=%1").arg(i+1));
#endif
        }

        netgen::SplineSegExt *spex = new netgen::SplineSegExt(*spline);
        spex->leftdom = edge->leftLabelIdx() + 1;
        spex->rightdom = edge->rightLabelIdx() + 1;
        spex->bc = i + 1;
        spex->hmax = edge->length() / 2;
        spex->hpref_left = false;
        spex->hpref_right = false;
        spex->copyfrom = -1;
        spex->reffak = 1;

        geom->AppendSegment(spex);
    }


    // labels
    foreach (SceneLabel *label, Agros2D::scene()->labels->items())
    {
        // if (label->markersCount() > 0)
        {
            double ar = (label->area() > 0) ? label->area() * 10: 1e22;

            geom->AppendMaterial(Agros2D::scene()->labels->items().indexOf(label) + 1,
                                 ar,
                                 Agros2D::problem()->config()->meshType() == MeshType_NETGEN_QuadDominated);

#ifdef debugIN2D
            outLabels += QString("%1\tdomain%2\t\t%3\n").
                    arg(labelsCount + 1).
                    arg(Agros2D::scene()->labels->items().indexOf(label) + 1).
                    arg(QString("-maxh=%1").arg(ar));

            labelsCount++;
#endif
        }
    }


#ifdef debugIN2D
    QTextStream out(&file);

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
#endif

    return true;
}

bool MeshGeneratorNetgen::readNetgenMeshFormat()
{
    // writeToHermes();
    // return false;

    netgen::MeshingParameters params;

    netgen::Mesh *mesh = NULL;
    // geom = new SplineGeometry2dAgros();
    // geom->Load((tempProblemFileName() + ".in2d").toStdString().c_str());
    geom->GenerateMesh(mesh, params, 1, 1);

    // qDebug() << geom->GetNP() << geom->GetNSplines() << geom->GetNMaterial();
    // qDebug() << mesh->GetNP() << mesh->GetNSeg() << mesh->GetNSE();

    for (int i = 1; i < mesh->GetNP() + 1; i++)
    {
        netgen::MeshPoint point = mesh->Point(i);
        nodeList.append(Point(point[0], point[1]));
        // qDebug() << "node" << i - 1 << point[0] << point[1];
    }

    for (int i = 1; i < mesh->GetNSeg() + 1; i++)
    {
        netgen::Segment segment = mesh->LineSegment(i);
        edgeList.append(MeshEdge(segment.pnums[0] - 1, segment.pnums[1] - 1, segment.si - 1));
        // qDebug() << "edge" << i - 1 << segment.pnums[0] - 1 << segment.pnums[1] - 1 << segment.si - 1;
    }

    for (int i = 1; i < mesh->GetNSE() + 1; i++)
    {
        netgen::Element2d element = mesh->SurfaceElement(i);
        if (element.GetType() == netgen::TRIG)
            elementList.append(MeshElement(element.PNum(1) - 1, element.PNum(2) - 1, element.PNum(3) - 1, element.GetIndex() - 1));
        // qDebug() << "element" << i - 1 << element.PNum(1) - 1 << element.PNum(2) - 1 << element.PNum(3) - 1 << element.GetIndex() - 1;
        else if (element.GetType() == netgen::QUAD)
            elementList.append(MeshElement(element.PNum(1) - 1, element.PNum(2) - 1, element.PNum(3) - 1, element.PNum(4) - 1, element.GetIndex() - 1));
        // qDebug() << "element" << i - 1 << element.PNum(1) - 1 << element.PNum(2) - 1 << element.PNum(3) - 1 << element.PNum(4) - 1 << element.GetIndex() - 1;
    }

    writeToHermes();

    mesh->DeleteMesh();
    delete mesh;
    delete geom;

    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    return true;
}

