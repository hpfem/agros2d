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

#include "dxf_filter.h"

#include "util.h"

#include "util/global.h"

#include "scene.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

DxfInterfaceDXFRW::DxfInterfaceDXFRW(Scene *scene, const QString &fileName) : m_isBlock(false)
{
    this->m_scene = scene;
    m_dxf = new dxfRW(fileName.toStdString().c_str());
}

void DxfInterfaceDXFRW::read()
{
    m_isBlock = false;

    m_dxf->read(this, true);
}

void DxfInterfaceDXFRW::write()
{
    m_dxf->write(this, DRW::AC1015, false);
}

void DxfInterfaceDXFRW::addLine(const DRW_Line &l)
{
    if (!m_isBlock)
    {
        // start node
        SceneNode *nodeStart = m_scene->addNode(new SceneNode(Point(l.basePoint.x, l.basePoint.y)));
        // end node
        SceneNode *nodeEnd = m_scene->addNode(new SceneNode(Point(l.secPoint.x, l.secPoint.y)));

        // edge
        m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, 0));
    }
    else
    {
        m_activeInsert.lines[m_activeInsert.blockName].append(l);
        // qDebug() << m_activeInsert.lines[m_activeInsert.blockName].count() << l.basePoint.x << l.basePoint.y << l.secPoint.x << l.secPoint.y;
    }
}

void DxfInterfaceDXFRW::addArc(const DRW_Arc& a)
{
    double angle1 = a.staangle / M_PI * 180.0;
    double angle2 = a.endangle / M_PI * 180.0;

    while (angle1 < 0.0) angle1 += 360.0;
    while (angle1 >= 360.0) angle1 -= 360.0;
    while (angle2 < 0.0) angle2 += 360.0;
    while (angle2 >= 360.0) angle2 -= 360.0;

    if (!m_isBlock)
    {
        // start node
        SceneNode *nodeStart = m_scene->addNode(new SceneNode(Point(a.basePoint.x + a.radious*cos(angle1/180.0*M_PI),
                                                                    a.basePoint.y + a.radious*sin(angle1/180.0*M_PI))));
        // end node
        SceneNode *nodeEnd = m_scene->addNode(new SceneNode(Point(a.basePoint.x + a.radious*cos(angle2/180.0*M_PI),
                                                                  a.basePoint.y + a.radious*sin(angle2/180.0*M_PI))));

        // edge
        m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, (angle1 < angle2) ? angle2-angle1 : angle2+360.0-angle1));
    }
}

void DxfInterfaceDXFRW::addCircle(const DRW_Circle &c)
{
    if (!m_isBlock)
    {
        // nodes
        SceneNode *node1 = m_scene->addNode(new SceneNode(Point(c.basePoint.x + c.radious, c.basePoint.y)));
        SceneNode *node2 = m_scene->addNode(new SceneNode(Point(c.basePoint.x, c.basePoint.y + c.radious)));
        SceneNode *node3 = m_scene->addNode(new SceneNode(Point(c.basePoint.x - c.radious, c.basePoint.y)));
        SceneNode *node4 = m_scene->addNode(new SceneNode(Point(c.basePoint.x, c.basePoint.y - c.radious)));

        // edges
        m_scene->addEdge(new SceneEdge(node1, node2, 90));
        m_scene->addEdge(new SceneEdge(node2, node3, 90));
        m_scene->addEdge(new SceneEdge(node3, node4, 90));
        m_scene->addEdge(new SceneEdge(node4, node1, 90));
    }
}

void DxfInterfaceDXFRW::addPolyline(const DRW_Polyline& data)
{
    for (int i = 0; i < data.vertlist.size() - 1; i++)
    {
        DRW_Vertex *vertStart = data.vertlist.at(i);
        DRW_Vertex *vertEnd = data.vertlist.at(i+1);

        SceneNode *nodeStart = m_scene->addNode(new SceneNode(Point(vertStart->basePoint.x, vertStart->basePoint.y)));
        SceneNode *nodeEnd = m_scene->addNode(new SceneNode(Point(vertEnd->basePoint.x, vertEnd->basePoint.y)));

        m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, 0.0));
    }
}

void DxfInterfaceDXFRW::addLWPolyline(const DRW_LWPolyline& data)
{
    for (int i = 0; i < data.vertlist.size() - 1; i++)
    {
        DRW_Vertex2D *vertStart = data.vertlist.at(i);
        DRW_Vertex2D *vertEnd = data.vertlist.at(i+1);

        SceneNode *nodeStart = m_scene->addNode(new SceneNode(Point(vertStart->x, vertStart->y)));
        SceneNode *nodeEnd = m_scene->addNode(new SceneNode(Point(vertEnd->x, vertEnd->y)));

        m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, 0.0));
    }
}

void DxfInterfaceDXFRW::addSpline(const DRW_Spline *data)
{
    // 1st order
    if (data->degree == 1)
    {
        // first and last point
        DRW_Coord *vertStart = data->controllist.at(0);
        DRW_Coord *vertEnd = data->controllist.at(data->controllist.size() - 1);

        SceneNode *nodeStart = m_scene->addNode(new SceneNode(Point(vertStart->x, vertStart->y)));
        SceneNode *nodeEnd = m_scene->addNode(new SceneNode(Point(vertEnd->x, vertEnd->y)));

        m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, 0.0));
    }
    else if (data->degree > 1)
    {
        qDebug() << "spline > 1nd order - not implemented";

        // first and last point
        DRW_Coord *vertStart = data->controllist.at(0);
        DRW_Coord *vertEnd = data->controllist.at(data->controllist.size() - 1);

        SceneNode *nodeStart = m_scene->addNode(new SceneNode(Point(vertStart->x, vertStart->y)));
        SceneNode *nodeEnd = m_scene->addNode(new SceneNode(Point(vertEnd->x, vertEnd->y)));

        m_scene->addEdge(new SceneEdge(nodeStart, nodeEnd, 0.0));
    }
}

void DxfInterfaceDXFRW::addBlock(const DRW_Block& data)
{
    m_activeInsert.blockName = QString::fromStdString(data.name);
    m_activeInsert.lines.clear();
    m_isBlock = true;

    // qDebug() << "addBlock" << QString::fromStdString(data.name);
}

void DxfInterfaceDXFRW::setBlock(const int handle)
{
    // qDebug() << "setBlock" << handle;
}

void DxfInterfaceDXFRW::endBlock()
{
    // qDebug() << "endBlock";

    m_isBlock = false;
}

void DxfInterfaceDXFRW::addInsert(const DRW_Insert& data)
{
    // qDebug() << "Insert " << m_activeInsert.lines.count() << QString::fromStdString(data.name) << m_activeInsert.lines[QString::fromStdString(data.name)].count();
    // line
    foreach (DRW_Line line, m_activeInsert.lines[QString::fromStdString(data.name)])
    {
        // DRW_Line newLine;

        // newLine.basePoint.x = line.basePoint.x + data.basePoint.x * cos(data.angle);
        // newLine.basePoint.y = line.basePoint.y + data.basePoint.y * sin(data.angle);
        // newLine.secPoint.x = line.secPoint.x + data.basePoint.x * cos(data.angle);
        // newLine.secPoint.y = line.secPoint.y + data.basePoint.y * sin(data.angle);

        // addLine(newLine);

        /*
        qDebug() << "Insert: " << QString::fromStdString(data.name)
                 << ", basepoint " << data.basePoint.x << data.basePoint.y
                 << ", scale " << data.xscale << data.yscale
                 << ", rotate " << data.angle
                 << ", space " << data.rowspace << data.colspace;
        */
    }


}

void DxfInterfaceDXFRW::writeHeader(DRW_Header& data)
{
    // bounding box
    RectPoint box = Agros2D::scene()->boundingBox();

    DRW_Variant *curr = NULL;

    curr = new DRW_Variant();
    curr->addCoord(new DRW_Coord());
    curr->setCoordX(box.start.x);
    curr->setCoordY(box.start.y);
    data.vars["$EXTMIN"] = curr;

    curr = new DRW_Variant();
    curr->addCoord(new DRW_Coord());
    curr->setCoordX(box.end.x);
    curr->setCoordY(box.end.y);
    data.vars["$EXTMAX"] =curr;
}

void DxfInterfaceDXFRW::writeEntities()
{
    // edges
    foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
    {
        if (fabs(edge->angle()) < EPS_ZERO)
        {
            // line
            double x1 = edge->nodeStart()->point().x;
            double y1 = edge->nodeStart()->point().y;
            double x2 = edge->nodeEnd()->point().x;
            double y2 = edge->nodeEnd()->point().y;

            DRW_Line line;
            line.basePoint.x = x1;
            line.basePoint.y = y1;
            line.secPoint.x = x2;
            line.secPoint.y = y2;
            line.layer = "AGROS2D";
            line.color = 256;
            line.color24 = -1;
            line.lWeight = DRW_LW_Conv::widthDefault;
            line.lineType = "BYLAYER";

            m_dxf->writeLine(&line);
        }
        else
        {
            // arc
            double cx = edge->center().x;
            double cy = edge->center().y;
            double radius = edge->radius();
            double angle1 = atan2(cy - edge->nodeStart()->point().y, cx - edge->nodeStart()->point().x)/M_PI*180.0 + 180.0;
            double angle2 = atan2(cy - edge->nodeEnd()->point().y, cx - edge->nodeEnd()->point().x)/M_PI*180.0 + 180.0;

            while (angle1 < 0.0) angle1 += 360.0;
            while (angle1 >= 360.0) angle1 -= 360.0;
            while (angle2 < 0.0) angle2 += 360.0;
            while (angle2 >= 360.0) angle2 -= 360.0;

            DRW_Arc arc;
            arc.basePoint.x = cx;
            arc.basePoint.y = cy;
            arc.radious = radius;
            arc.staangle = angle1 / 180.0 * M_PI;
            arc.endangle = angle2 / 180.0 * M_PI;
            arc.layer = "AGROS2D";
            arc.color = 256;
            arc.color24 = -1;
            arc.lWeight = DRW_LW_Conv::widthDefault;
            arc.lineType = "BYLAYER";

            m_dxf->writeArc(&arc);
        }
    }
}


// *******************************************************************************

void readFromDXF(const QString &fileName)
{
    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    Agros2D::scene()->blockSignals(true);

    DxfInterfaceDXFRW filter(Agros2D::scene(), fileName);
    filter.read();

    Agros2D::scene()->blockSignals(false);
    Agros2D::scene()->invalidate();

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

void writeToDXF(const QString &fileName)
{
    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    Agros2D::scene()->blockSignals(true);

    DxfInterfaceDXFRW filter(Agros2D::scene(), fileName);
    filter.write();

    Agros2D::scene()->blockSignals(false);
    Agros2D::scene()->invalidate();

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}
