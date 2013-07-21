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

#ifndef UTIL_DXF_FILTER_H
#define UTIL_DXF_FILTER_H

#include "util.h"

#include "libdxfrw.h"

class Scene;

class DxfInterfaceDXFRW : public DRW_Interface
{
public:
    DxfInterfaceDXFRW(Scene *scene, const QString &fileName);

    void read();
    void write();

    virtual void addArc(const DRW_Arc &a);
    virtual void addLine(const DRW_Line &l);
    virtual void addCircle(const DRW_Circle& c);
    virtual void addPolyline(const DRW_Polyline& data);
    virtual void addLWPolyline(const DRW_LWPolyline& data);
    virtual void addSpline(const DRW_Spline* data);
    virtual void addBlock(const DRW_Block& data);
    virtual void setBlock(const int handle);
    virtual void endBlock();
    virtual void addInsert(const DRW_Insert& data);

    // Methods from DRW_CreationInterface:
    virtual void addHeader(const DRW_Header* data) {}
    virtual void addLType(const DRW_LType& data) {}
    virtual void addLayer(const DRW_Layer& data) {}
    virtual void addDimStyle(const DRW_Dimstyle& data) {}
    virtual void addVport(const DRW_Vport& data) {}
    virtual void addTextStyle(const DRW_Textstyle& data) {}
    virtual void addPoint(const DRW_Point& data) {}
    virtual void addRay(const DRW_Ray& data) {}
    virtual void addXline(const DRW_Xline& data) {}
    virtual void addEllipse(const DRW_Ellipse& data) {}
    virtual void addText(const DRW_Text& data) {}    
    virtual void addKnot(const DRW_Entity&) {}
    virtual void addTrace(const DRW_Trace& data) {}
    virtual void addSolid(const DRW_Solid& data) {}
    virtual void addMText(const DRW_MText& data) {}
    virtual void addDimAlign(const DRW_DimAligned *data) {}
    virtual void addDimLinear(const DRW_DimLinear *data) {}
    virtual void addDimRadial(const DRW_DimRadial *data) {}
    virtual void addDimDiametric(const DRW_DimDiametric *data) {}
    virtual void addDimAngular(const DRW_DimAngular *data) {}
    virtual void addDimAngular3P(const DRW_DimAngular3p *data) {}
    virtual void addDimOrdinate(const DRW_DimOrdinate *data) {}
    virtual void addLeader(const DRW_Leader *data) {}
    virtual void addHatch(const DRW_Hatch* data) {}
    virtual void addViewport(const DRW_Viewport& data) {}
    virtual void addImage(const DRW_Image* data) {}
    virtual void linkImage(const DRW_ImageDef* data) {}

    virtual void add3dFace(const DRW_3Dface& data) {}
    virtual void addComment(const char*) {}

    // Export:
    virtual void writeHeader(DRW_Header& data);
    virtual void writeEntities();
    virtual void writeLTypes() {}
    virtual void writeLayers() {}
    virtual void writeTextstyles() {}
    virtual void writeVports() {}
    virtual void writeBlockRecords() {}
    virtual void writeBlocks() {}
    virtual void writeDimstyles() {}

private:
    Scene *m_scene;
    dxfRW *m_dxf;

    struct DXFInsert
    {
        QString blockName;

        QMap<QString, QList<DRW_Line> > lines;

        void clear()
        {
            blockName.clear();

            // entities
            lines.clear();
        }

        inline bool isEmpty() { return blockName.isEmpty(); }
    };

    bool m_isBlock;
    DXFInsert m_activeInsert;
};

void readFromDXF(const QString &fileName);
void writeToDXF(const QString &fileName);

#endif // UTIL_DXF_FILTER_H

