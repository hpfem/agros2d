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
    logMessage("Config::Config()");

    load();
}

Config::~Config()
{
    logMessage("Config::~Config()");

    save();
}

void Config::load()
{
    logMessage("Config::load()");

    QSettings settings;

    // experimental features
    showExperimentalFeatures = settings.value("SceneViewSettings/ExperimentalFeatures", false).toBool();

    // general
    guiStyle = settings.value("General/GUIStyle").toString();
    language = settings.value("General/Language", QLocale::system().name()).toString();
    defaultPhysicField = (PhysicField) settings.value("General/DefaultPhysicField", PhysicField_Electrostatic).toInt();
    if (defaultPhysicField == PhysicField_Undefined) defaultPhysicField = PhysicField_Electrostatic;

    collaborationServerURL = settings.value("General/CollaborationServerURL", QString("http://agros2d.org/collaboration/")).toString();

    checkVersion = settings.value("General/CheckVersion", true).toBool();
    showConvergenceChart = settings.value("General/ShowConvergenceChart", true).toBool();
    enabledApplicationLog = settings.value("General/EnabledApplicationLog", true).toBool();
    enabledProgressLog = settings.value("General/EnabledProgressLog", true).toBool();
    lineEditValueShowResult = settings.value("General/LineEditValueShowResult", false).toBool();
    if (showExperimentalFeatures)
        saveProblemWithSolution = settings.value("Solver/SaveProblemWithSolution", false).toBool();
    else
        saveProblemWithSolution = false;

    // zoom
    zoomToMouse = settings.value("Geometry/ZoomToMouse", true).toBool();

    // delete files
    deleteTriangleMeshFiles = settings.value("Solver/DeleteTriangleMeshFiles", true).toBool();
    deleteHermes2DMeshFile = settings.value("Solver/DeleteHermes2DMeshFile", true).toBool();

    // colors
    colorBackground = settings.value("SceneViewSettings/ColorBackground", COLORBACKGROUND).value<QColor>();
    colorGrid = settings.value("SceneViewSettings/ColorGrid", COLORGRID).value<QColor>();
    colorCross = settings.value("SceneViewSettings/ColorCross", COLORCROSS).value<QColor>();
    colorNodes = settings.value("SceneViewSettings/ColorNodes", COLORNODES).value<QColor>();
    colorEdges = settings.value("SceneViewSettings/ColorEdges", COLOREDGES).value<QColor>();
    colorLabels = settings.value("SceneViewSettings/ColorLabels", COLORLABELS).value<QColor>();
    colorContours = settings.value("SceneViewSettings/ColorContours", COLORCONTOURS).value<QColor>();
    colorVectors = settings.value("SceneViewSettings/ColorVectors", COLORVECTORS).value<QColor>();
    colorInitialMesh = settings.value("SceneViewSettings/ColorInitialMesh", COLORINITIALMESH).value<QColor>();
    colorSolutionMesh = settings.value("SceneViewSettings/ColorSolutionMesh", COLORSOLUTIONMESH).value<QColor>();
    colorHighlighted = settings.value("SceneViewSettings/ColorHighlighted", COLORHIGHLIGHTED).value<QColor>();
    colorSelected = settings.value("SceneViewSettings/ColorSelected", COLORSELECTED).value<QColor>();

    // geometry
    nodeSize = settings.value("SceneViewSettings/NodeSize", 6.0).toDouble();
    edgeWidth = settings.value("SceneViewSettings/EdgeWidth", 2.0).toDouble();
    labelSize = settings.value("SceneViewSettings/LabelSize", 6.0).toDouble();

    // font
    sceneFont = settings.value("SceneViewSettings/SceneFont", FONT).value<QFont>();

    // mesh
    angleSegmentsCount = settings.value("SceneViewSettings/AngleSegmentsCount", 3).toInt();
    curvilinearElements = settings.value("SceneViewSettings/CurvilinearElements", true).toBool();

    // grid
    showGrid = settings.value("SceneViewSettings/ShowGrid", SHOWGRID).toBool();
    gridStep = settings.value("SceneViewSettings/GridStep", GRIDSTEP).toDouble();
    // rulers
    showRulers = settings.value("SceneViewSettings/ShowRulers", SHOWRULERS).toBool();
    // snap to grid
    snapToGrid = settings.value("SceneViewSettings/SnapToGrid", SNAPTOGRID).toBool();

    // axes
    showAxes = settings.value("SceneViewSettings/ShowAxes", SHOWAXES).toBool();

    // label
    showLabel = settings.value("SceneViewSettings/ShowLabel", SHOWLABEL).toBool();

    // linearizer quality
    linearizerQuality = settings.value("SceneViewSettings/LinearizerQuality", LINEARIZER_QUALITY).toDouble();

    // countour
    contoursCount = settings.value("SceneViewSettings/ContoursCount", CONTOURSCOUNT).toInt();

    // scalar view
    showScalarScale = settings.value("SceneViewSettings/ShowScalarScale", true).toBool();
    paletteType = (PaletteType) settings.value("SceneViewSettings/PaletteType", PALETTETYPE).toInt();
    paletteFilter = settings.value("SceneViewSettings/PaletteFilter", PALETTEFILTER).toBool();
    paletteSteps = settings.value("SceneViewSettings/PaletteSteps", PALETTESTEPS).toInt();
    scalarRangeLog = settings.value("SceneViewSettings/ScalarRangeLog", SCALARRANGELOG).toBool();
    scalarRangeBase = settings.value("SceneViewSettings/ScalarRangeBase", SCALARRANGEBASE).toDouble();
    scalarDecimalPlace = settings.value("SceneViewSettings/ScalarDecimalPlace", SCALARDECIMALPLACE).toDouble();

    // vector view
    vectorProportional = settings.value("SceneViewSettings/VectorProportional", VECTORPROPORTIONAL).toBool();
    vectorColor = settings.value("SceneViewSettings/VectorColor", VECTORCOLOR).toBool();
    vectorCount = settings.value("SceneViewSettings/VectorNumber", VECTORNUMBER).toInt();
    vectorScale = settings.value("SceneViewSettings/VectorScale", VECTORSCALE).toDouble();

    // order view
    showOrderScale = settings.value("SceneViewSettings/ShowOrderScale", true).toBool();
    orderPaletteOrderType = (PaletteOrderType) settings.value("SceneViewSettings/OrderPaletteOrderType", ORDERPALETTEORDERTYPE).toInt();
    orderLabel = settings.value("SceneViewSettings/OrderLabel", ORDERLABEL).toBool();

    // deformations
    deformScalar = settings.value("SceneViewSettings/DeformScalar", true).toBool();
    deformContour = settings.value("SceneViewSettings/DeformContour", true).toBool();
    deformVector = settings.value("SceneViewSettings/DeformVector", true).toBool();

    // particle tracing
    particleIncludeGravitation = settings.value("SceneViewSettings/ParticleIncludeGravitation", PARTICLEINCLUDEGRAVITATION).toBool();
    particleMass = settings.value("SceneViewSettings/ParticleMass", PARTICLEMASS).toDouble();
    particleConstant = settings.value("SceneViewSettings/ParticleConstant", PARTICLECONSTANT).toDouble();
    particleStart.x = settings.value("SceneViewSettings/ParticleStartX", PARTICLESTARTX).toDouble();
    particleStart.y = settings.value("SceneViewSettings/ParticleStartY", PARTICLESTARTY).toDouble();
    particleStartVelocity.x = settings.value("SceneViewSettings/ParticleStartVelocityX", PARTICLESTARTVELOCITYX).toDouble();
    particleStartVelocity.y = settings.value("SceneViewSettings/ParticleStartVelocityY", PARTICLESTARTVELOCITYY).toDouble();
    particleNumberOfParticles = settings.value("SceneViewSettings/ParticleNumberOfParticles", PARTICLENUMBEROFPARTICLES).toInt();
    particleStartingRadius = settings.value("SceneViewSettings/ParticleStartingRadius", PARTICLESTARTINGRADIUS).toDouble();
    particleTerminateOnDifferentMaterial = settings.value("SceneViewSettings/ParticleTerminateOnDifferentMaterial", PARTICLETERMINATEONDIFFERENTMATERIAL).toBool();
    particleMaximumRelativeError = settings.value("SceneViewSettings/ParticleMaximumRelativeError", PARTICLEMAXIMUMRELATIVEERROR).toDouble();
    particleShowPoints = settings.value("SceneViewSettings/ParticleShowPoints", PARTICLESHOWPOINTS).toBool();
    particleColorByVelocity = settings.value("SceneViewSettings/ParticleColorByVelocity", PARTICLECOLORBYVELOCITY).toBool();
    particleMaximumSteps = settings.value("SceneViewSettings/ParticleMaximumSteps", PARTICLEMAXIMUMSTEPS).toInt();

    // 3d
    scalarView3DLighting = settings.value("SceneViewSettings/ScalarView3DLighting", false).toBool();
    scalarView3DAngle = settings.value("SceneViewSettings/ScalarView3DAngle", 270).toDouble();
    scalarView3DBackground = settings.value("SceneViewSettings/ScalarView3DBackground", true).toBool();
    scalarView3DHeight = settings.value("SceneViewSettings/ScalarView3DHeight", 4.0).toDouble();

    // adaptivity
    maxDofs = settings.value("Adaptivity/MaxDofs", MAX_DOFS).toInt();
    isoOnly = settings.value("Adaptivity/IsoOnly", ADAPTIVITY_ISOONLY).toBool();
    convExp = settings.value("Adaptivity/ConvExp", ADAPTIVITY_CONVEXP).toDouble();
    threshold = settings.value("Adaptivity/Threshold", ADAPTIVITY_THRESHOLD).toDouble();
    strategy = settings.value("Adaptivity/Strategy", ADAPTIVITY_STRATEGY).toInt();
    meshRegularity = settings.value("Adaptivity/MeshRegularity", ADAPTIVITY_MESHREGULARITY).toInt();
    projNormType = (ProjNormType) settings.value("Adaptivity/ProjNormType", ADAPTIVITY_PROJNORMTYPE).toInt();

    // command argument
    commandTriangle = settings.value("Commands/Triangle", COMMANDS_TRIANGLE).toString();
    // add quadratic elements (added points on the middle of edge used by rough triangle division)
    if (!commandTriangle.contains("-o2"))
        commandTriangle = COMMANDS_TRIANGLE;
    commandFFmpeg = settings.value("Commands/FFmpeg", COMMANDS_FFMPEG).toString();

    // global script
    globalScript = settings.value("Python/GlobalScript", "").toString();
}

void Config::save()
{
    logMessage("Config::Save()");

    QSettings settings;

    // experimental features
    settings.setValue("SceneViewSettings/ExperimentalFeatures", showExperimentalFeatures);

    // general
    settings.setValue("General/GUIStyle", guiStyle);
    settings.setValue("General/Language", language);
    settings.setValue("General/DefaultPhysicField", defaultPhysicField);

    settings.setValue("General/CollaborationServerURL", collaborationServerURL);

    settings.setValue("General/CheckVersion", checkVersion);
    settings.setValue("General/ShowConvergenceChart", showConvergenceChart);
    settings.setValue("General/EnabledApplicationLog", enabledApplicationLog);
    settings.setValue("General/EnabledProgressLog", enabledProgressLog);
    settings.setValue("General/LineEditValueShowResult", lineEditValueShowResult);
    if (showExperimentalFeatures)
        settings.setValue("General/SaveProblemWithSolution", saveProblemWithSolution);
    else
        saveProblemWithSolution = false;

    // zoom
    settings.setValue("General/ZoomToMouse", zoomToMouse);

    // delete files
    settings.setValue("Solver/DeleteTriangleMeshFiles", deleteTriangleMeshFiles);
    settings.setValue("Solver/DeleteHermes2DMeshFile", deleteHermes2DMeshFile);

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

    // font
    settings.setValue("SceneViewSettings/SceneFont", sceneFont);

    // mesh
    settings.setValue("SceneViewSettings/AngleSegmentsCount", angleSegmentsCount);
    settings.setValue("SceneViewSettings/CurvilinearElements", curvilinearElements);

    // grid
    settings.setValue("SceneViewSettings/ShowGrid", showGrid);
    settings.setValue("SceneViewSettings/GridStep", gridStep);

    // scene font
    settings.setValue("SceneViewSettings/SceneFont", sceneFont);

    // rulers
    settings.setValue("SceneViewSettings/ShowRulers", showRulers);
    // snap to grid
    settings.setValue("SceneViewSettings/SnapToGrid", snapToGrid);

    // axes
    settings.setValue("SceneViewSettings/ShowAxes", showAxes);

    // label
    settings.setValue("SceneViewSettings/ShowLabel", showLabel);

    // linearizer quality
    settings.setValue("SceneViewSettings/LinearizerQuality", linearizerQuality);

    // countour
    settings.setValue("SceneViewSettings/ContoursCount", contoursCount);

    // scalar view
    settings.setValue("SceneViewSettings/ShowScalarScale", showScalarScale);
    settings.setValue("SceneViewSettings/PaletteType", paletteType);
    settings.setValue("SceneViewSettings/PaletteFilter", paletteFilter);
    settings.setValue("SceneViewSettings/PaletteSteps", paletteSteps);
    settings.setValue("SceneViewSettings/ScalarRangeLog", scalarRangeLog);
    settings.setValue("SceneViewSettings/ScalarRangeBase", scalarRangeBase);
    settings.setValue("SceneViewSettings/ScalarDecimalPlace", scalarDecimalPlace);

    // vector view
    settings.setValue("SceneViewSettings/VectorProportional", vectorProportional);
    settings.setValue("SceneViewSettings/VectorColor", vectorColor);
    settings.setValue("SceneViewSettings/VectorNumber", vectorCount);
    settings.setValue("SceneViewSettings/VectorScale", vectorScale);

    // order view
    settings.setValue("SceneViewSettings/ShowOrderScale", showOrderScale);
    settings.setValue("SceneViewSettings/OrderPaletteOrderType", orderPaletteOrderType);
    settings.setValue("SceneViewSettings/OrderLabel", orderLabel);

    // deformations
    settings.setValue("SceneViewSettings/DeformScalar", deformScalar);
    settings.setValue("SceneViewSettings/DeformContour", deformContour);
    settings.setValue("SceneViewSettings/DeformVector", deformVector);

    // particle tracing
    settings.setValue("SceneViewSettings/ParticleIncludeGravitation", particleIncludeGravitation);
    settings.setValue("SceneViewSettings/ParticleMass", particleMass);
    settings.setValue("SceneViewSettings/ParticleConstant", particleConstant);
    settings.setValue("SceneViewSettings/ParticleStartX", particleStart.x);
    settings.setValue("SceneViewSettings/ParticleStartY", particleStart.y);
    settings.setValue("SceneViewSettings/ParticleStartVelocityX", particleStartVelocity.x);
    settings.setValue("SceneViewSettings/ParticleStartVelocityY", particleStartVelocity.y);
    settings.setValue("SceneViewSettings/ParticleNumberOfParticles", particleNumberOfParticles);
    settings.setValue("SceneViewSettings/ParticleStartingRadius", particleStartingRadius);
    settings.setValue("SceneViewSettings/ParticleTerminateOnDifferentMaterial", particleTerminateOnDifferentMaterial);
    settings.setValue("SceneViewSettings/ParticleMaximumRelativeError", particleMaximumRelativeError);
    settings.setValue("SceneViewSettings/ParticleShowPoints", particleShowPoints);
    settings.setValue("SceneViewSettings/ParticleColorByVelocity", particleColorByVelocity);
    settings.setValue("SceneViewSettings/ParticleMaximumSteps", particleMaximumSteps);

    // 3d
    settings.setValue("SceneViewSettings/ScalarView3DLighting", scalarView3DLighting);
    settings.setValue("SceneViewSettings/ScalarView3DAngle", scalarView3DAngle);
    settings.setValue("SceneViewSettings/ScalarView3DBackground", scalarView3DBackground);
    settings.setValue("SceneViewSettings/ScalarView3DHeight", scalarView3DHeight);

    // adaptivity
    settings.setValue("Adaptivity/MaxDofs", maxDofs);
    settings.setValue("Adaptivity/IsoOnly", isoOnly);
    settings.setValue("Adaptivity/ConvExp", convExp);
    settings.setValue("Adaptivity/Threshold", threshold);
    settings.setValue("Adaptivity/Strategy", strategy);
    settings.setValue("Adaptivity/MeshRegularity", meshRegularity);
    settings.setValue("Adaptivity/ProjNormType", projNormType);

    // command argument
    settings.setValue("Commands/Triangle", commandTriangle);
    settings.setValue("Commands/FFmpeg", commandFFmpeg);

    // global script
    settings.setValue("Python/GlobalScript", globalScript);
}
