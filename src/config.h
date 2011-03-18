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

#ifndef CONFIG_H
#define CONFIG_H

#include "util.h"

class Config : public QObject
{
    Q_OBJECT

public:
    Config();
    ~Config();

    // general
    QString guiStyle;
    QString language;
    PhysicField defaultPhysicField;

    bool checkVersion;
    bool showConvergenceChart;
    bool enabledApplicationLog;
    bool enabledProgressLog;
    bool lineEditValueShowResult;
    bool saveProblemWithSolution;

    // geometry
    double nodeSize;
    double edgeWidth;
    double labelSize;
    int angleSegmentsCount;
    bool curvilinearElements;

    // font
    QFont sceneFont;

    // zoom
    bool zoomToMouse;

    // delete files
    bool deleteTriangleMeshFiles;
    bool deleteHermes2DMeshFile;

    // grid
    bool showGrid;
    double gridStep;
    bool snapToGrid;

    // axes
    bool showAxes;

    // label
    bool showLabel;

    // contour
    int contoursCount;

    // scalar view
    bool scalarRangeLog;
    double scalarRangeBase;
    int scalarDecimalPlace;

    // vector view
    bool vectorProportional;
    bool vectorColor;
    int vectorCount;
    double vectorScale;

    // order view
    bool orderLabel;
    PaletteOrderType orderPaletteOrderType;

    // deformations
    bool deformScalar;
    bool deformContour;
    bool deformVector;

    // 3d
    bool scalarView3DLighting;
    double scalarView3DAngle;
    bool scalarView3DBackground;

    // palette
    PaletteType paletteType;
    int paletteSteps;
    bool paletteFilter;

    // rulers
    bool showRulers;

    // colors
    QColor colorBackground;
    QColor colorGrid;
    QColor colorCross;
    QColor colorNodes;
    QColor colorEdges;
    QColor colorLabels;
    QColor colorContours;
    QColor colorVectors;
    QColor colorInitialMesh;
    QColor colorSolutionMesh;
    QColor colorHighlighted;
    QColor colorSelected;

    // adaptivity
    bool isoOnly;
    double convExp;
    double threshold;
    int strategy;
    int meshRegularity;
    ProjNormType projNormType;

    // command argument
    QString commandTriangle;
    QString commandFFmpeg;

    // global script
    QString globalScript;

    void load();
    void save();
};

#endif // CONFIG_H
