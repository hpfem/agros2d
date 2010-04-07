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

#include "config.h"

Config::Config()
{
    load();
}

Config::~Config()
{
    save();
}

void Config::load()
{
    QSettings settings;

    // general
    guiStyle = settings.value("General/GUIStyle").toString();
    language = settings.value("General/Language", QLocale::system().name()).toString();
    defaultPhysicField = (PhysicField) settings.value("General/DefaultPhysicField", PhysicField_Electrostatic).toInt();

    checkVersion = settings.value("General/CheckVersion", true).toBool();
    lineEditValueShowResult = settings.value("General/LineEditValueShowResult", false).toBool();
    saveProblemWithSolution = settings.value("Solver/SaveProblemWithSolution", false).toBool();

    // zoom
    zoomToMouse = settings.value("Geometry/ZoomToMouse", true).toBool();

    // delete files
    deleteTriangleMeshFiles = settings.value("Solver/DeleteTriangleMeshFiles", true).toBool();
    deleteHermes2DMeshFile = settings.value("Solver/DeleteHermes2DMeshFile", true).toBool();

    // colors
    colorBackground = settings.value("SceneViewSettings/ColorBackground", QColor::fromRgb(255, 255, 255)).value<QColor>();
    colorGrid = settings.value("SceneViewSettings/ColorGrid", QColor::fromRgb(200, 200, 200)).value<QColor>();
    colorCross = settings.value("SceneViewSettings/ColorCross", QColor::fromRgb(150, 150, 150)).value<QColor>();
    colorNodes = settings.value("SceneViewSettings/ColorNodes", QColor::fromRgb(150, 0, 0)).value<QColor>();
    colorEdges = settings.value("SceneViewSettings/ColorEdges", QColor::fromRgb(0, 0, 150)).value<QColor>();
    colorLabels = settings.value("SceneViewSettings/ColorLabels", QColor::fromRgb(0, 150, 0)).value<QColor>();
    colorContours = settings.value("SceneViewSettings/ColorContours", QColor::fromRgb(0, 0, 0)).value<QColor>();
    colorVectors = settings.value("SceneViewSettings/ColorVectors", QColor::fromRgb(0, 0, 0)).value<QColor>();
    colorInitialMesh = settings.value("SceneViewSettings/ColorInitialMesh", QColor::fromRgb(250, 250, 0)).value<QColor>();
    colorSolutionMesh = settings.value("SceneViewSettings/ColorSolutionMesh", QColor::fromRgb(150, 70, 0)).value<QColor>();
    colorHighlighted = settings.value("SceneViewSettings/ColorHighlighted", QColor::fromRgb(250, 150, 0)).value<QColor>();
    colorSelected = settings.value("SceneViewSettings/ColorSelected", QColor::fromRgb(150, 0, 0)).value<QColor>();

    // geometry
    nodeSize = settings.value("SceneViewSettings/NodeSize", 6.0).toDouble();
    edgeWidth = settings.value("SceneViewSettings/EdgeWidth", 2.0).toDouble();
    labelSize = settings.value("SceneViewSettings/LabelSize", 6.0).toDouble();
    angleSegmentsCount = settings.value("Geometry/AngleSegmentsCount", 5).toInt();

    // grid
    gridStep = settings.value("SceneViewSettings/GridStep", 0.05).toDouble();
    // rulers
    showRulers = settings.value("SceneViewSettings/ShowRulers", false).toBool();
    // snap to grid
    snapToGrid = settings.value("SceneViewSettings/SnapToGrid", false).toBool();

    // countour
    contoursCount = settings.value("SceneViewSettings/ContoursCount", 15).toInt();

    // scalar view
    paletteType = (PaletteType) settings.value("SceneViewSettings/PaletteType", Palette_Jet).toInt();
    paletteFilter = settings.value("SceneViewSettings/PaletteFilter", false).toBool();
    paletteSteps = settings.value("SceneViewSettings/PaletteSteps", 30).toInt();

    // vector view
    vectorProportional = settings.value("SceneViewSettings/VectorProportional", false).toBool();
    vectorColor = settings.value("SceneViewSettings/VectorColor", true).toBool();
    vectorCount = settings.value("SceneViewSettings/VectorNumber", 50).toInt();
    vectorScale = settings.value("SceneViewSettings/VectorScale", 0.6).toDouble();

    // 3d
    scalarView3DLighting = settings.value("SceneViewSettings/ScalarView3DLighting", false).toBool();

    // adaptivity
    isoOnly = settings.value("Adaptivity/IsoOnly", ADAPTIVITY_ISOONLY).toBool();
    convExp = settings.value("Adaptivity/ConvExp", ADAPTIVITY_CONVEXP).toDouble();
    threshold = settings.value("Adaptivity/Threshold", ADAPTIVITY_THRESHOLD).toDouble();
    strategy = settings.value("Adaptivity/Strategy", ADAPTIVITY_STRATEGY).toInt();
    meshRegularity = settings.value("Adaptivity/MeshRegularity", ADAPTIVITY_MESHREGULARITY).toInt();

    // command argument
    commandTriangle = settings.value("Commands/Triangle", COMMANDS_TRIANGLE).toString();
    commandFFmpeg = settings.value("Commands/FFmpeg", COMMANDS_FFMPEG).toString();
}

void Config::save()
{
    QSettings settings;

    // colors
    settings.setValue("SceneViewSettings/ColorBackground", colorBackground);
    settings.setValue("SceneViewSettings/ColorGrid", colorGrid);
    settings.setValue("SceneViewSettings/ColorCross", colorCross);
    settings.setValue("SceneViewSettings/ColorNodes", colorNodes);
    settings.setValue("SceneViewSettings/ColorEdges", colorEdges);
    settings.setValue("SceneViewSettings/ColorLabels", colorLabels);
    settings.setValue("SceneViewSettings/ColorContours", colorContours);
    settings.setValue("SceneViewSettings/ColorVectors", colorVectors);
    settings.setValue("SceneViewSettings/ColorInitialMesh", colorInitialMesh);
    settings.setValue("SceneViewSettings/ColorSolutionMesh", colorSolutionMesh);
    settings.setValue("SceneViewSettings/ColorInitialMesh", colorHighlighted);
    settings.setValue("SceneViewSettings/ColorSolutionMesh", colorSelected);

    // geometry
    settings.setValue("SceneViewSettings/NodeSize", nodeSize);
    settings.setValue("SceneViewSettings/EdgeWidth", edgeWidth);
    settings.setValue("SceneViewSettings/LabelSize", labelSize);

    // grid
    settings.setValue("SceneViewSettings/GridStep", gridStep);
    // rulers
    settings.setValue("SceneViewSettings/ShowRulers", showRulers);
    // snap to grid
    settings.setValue("SceneViewSettings/SnapToGrid", snapToGrid);

    // countour
    settings.setValue("SceneViewSettings/ContoursCount", contoursCount);

    // scalar view
    settings.setValue("SceneViewSettings/PaletteType", paletteType);
    settings.setValue("SceneViewSettings/PaletteFilter", paletteFilter);
    settings.setValue("SceneViewSettings/PaletteSteps", paletteSteps);

    // vector view
    settings.setValue("SceneViewSettings/VectorProportional", vectorProportional);
    settings.setValue("SceneViewSettings/VectorColor", vectorColor);
    settings.setValue("SceneViewSettings/VectorNumber", vectorCount);
    settings.setValue("SceneViewSettings/VectorScale", vectorScale);

    // 3d
    settings.setValue("SceneViewSettings/ScalarView3DLighting", scalarView3DLighting);
}
