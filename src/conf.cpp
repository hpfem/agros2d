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
#include "scene.h"

#include "hermes2d/module.h"
#include "util/constants.h"

Config::Config() : eleConfig(NULL)
{
    load();
}

Config::~Config()
{
    save();
}

void Config::load()
{
    loadWorkspace();
    loadPostprocessor(NULL);
    loadAdvanced();
}

void Config::loadWorkspace()
{
    QSettings settings;

    // std log
    showLogStdOut = settings.value("SceneViewSettings/LogStdOut", false).toBool();

    // general
    guiStyle = settings.value("General/GUIStyle").toString();
    language = settings.value("General/Language", QLocale::system().name()).toString();
    defaultPhysicField = settings.value("General/DefaultPhysicField", "electrostatics").toString();

    if (!availableModules().keys().contains(defaultPhysicField))
        defaultPhysicField = "electrostatic";

    collaborationServerURL = settings.value("General/CollaborationServerURL", QString("http://agros2d.org/collaboration/")).toString();

    checkVersion = settings.value("General/CheckVersion", true).toBool();
    lineEditValueShowResult = settings.value("General/LineEditValueShowResult", false).toBool();
    if (showExperimentalFeatures)
        saveProblemWithSolution = settings.value("Solver/SaveProblemWithSolution", false).toBool();
    else
        saveProblemWithSolution = false;

    // zoom
    zoomToMouse = settings.value("Geometry/ZoomToMouse", true).toBool();

    // delete files
    deleteMeshFiles = settings.value("Solver/DeleteTriangleMeshFiles", true).toBool();
    deleteHermesMeshFile = settings.value("Solver/DeleteHermes2DMeshFile", true).toBool();

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
    colorCrossed = settings.value("SceneViewSettings/ColorCrossed", COLORCROSSED).value<QColor>();
    colorNotConnected = settings.value("SceneViewSettings/ColorCrossed", COLORNOTCONNECTED).value<QColor>();

    // geometry
    nodeSize = settings.value("SceneViewSettings/NodeSize", GEOMETRYNODESIZE).toDouble();
    edgeWidth = settings.value("SceneViewSettings/EdgeWidth", GEOMETRYEDGEWIDTH).toDouble();
    labelSize = settings.value("SceneViewSettings/LabelSize", GEOMETRYLABELSIZE).toDouble();

    // font
    rulersFont = settings.value("SceneViewSettings/RulersFont", RULERSFONT).toString();
    postFont = settings.value("SceneViewSettings/PostFont", POSTFONT).toString();

    // discrete
    saveMatrixRHS = settings.value("SceneViewSettings/SaveMatrixAndRHS", SAVEMATRIXANDRHS).toBool();
    discreteDirectory = settings.value("SceneViewSettings/DiscreteDirectory", DISCRETEDIRECTORY).toString();

    // grid
    showGrid = settings.value("SceneViewSettings/ShowGrid", SHOWGRID).toBool();
    gridStep = settings.value("SceneViewSettings/GridStep", GRIDSTEP).toDouble();

    // rulers
    showRulers = settings.value("SceneViewSettings/ShowRulers", SHOWRULERS).toBool();
    // snap to grid
    snapToGrid = settings.value("SceneViewSettings/SnapToGrid", SNAPTOGRID).toBool();

    // axes
    showAxes = settings.value("SceneViewSettings/ShowAxes", SHOWAXES).toBool();

    // linearizer quality
    QString quality = settings.value("SceneViewSettings/LinearizerQuality", paletteQualityToStringKey(PaletteQuality_Normal)).toString();
    linearizerQuality = paletteQualityFromStringKey(quality);

    // 3d
    scalarView3DLighting = settings.value("SceneViewSettings/ScalarView3DLighting", false).toBool();
    scalarView3DAngle = settings.value("SceneViewSettings/ScalarView3DAngle", 270).toDouble();
    scalarView3DBackground = settings.value("SceneViewSettings/ScalarView3DBackground", true).toBool();
    scalarView3DHeight = settings.value("SceneViewSettings/ScalarView3DHeight", 4.0).toDouble();
    scalarView3DBoundingBox = settings.value("SceneViewSettings/ScalarView3DBoundingBox", true).toBool();

    // deformations
    deformScalar = settings.value("SceneViewSettings/DeformScalar", true).toBool();
    deformContour = settings.value("SceneViewSettings/DeformContour", true).toBool();
    deformVector = settings.value("SceneViewSettings/DeformVector", true).toBool();
}

void Config::loadPostprocessor(QDomElement *config)
{
    eleConfig = config;

    // active field
    activeField = readConfig("SceneViewSettings/ActiveField", QString());

    // view
    showPost3D = (SceneViewPost3DMode) readConfig("SceneViewSettings/ShowPost3D", (int) SCALARSHOWPOST3D);

    // mesh
    showInitialMeshView = readConfig("SceneViewSettings/ShowInitialMeshView", SHOWINITIALMESHVIEW);
    showSolutionMeshView = readConfig("SceneViewSettings/ShowSolutionMeshView", SHOWSOLUTIONMESHVIEW);

    // contour
    showContourView = readConfig("SceneViewSettings/ShowContourView", SHOWCONTOURVIEW);
    contourVariable = readConfig("SceneViewSettings/ContourVariable", QString());
    contoursCount = readConfig("SceneViewSettings/ContoursCount", CONTOURSCOUNT);
    contourWidth = readConfig("SceneViewSettings/ContoursWidth", CONTOURSWIDTH);

    // scalar view
    showScalarView = readConfig("SceneViewSettings/ShowScalarView", SHOWSCALARVIEW);
    showScalarColorBar = readConfig("SceneViewSettings/ShowScalarColorBar", SHOWSCALARCOLORBAR);
    scalarVariable = readConfig("SceneViewSettings/ScalarVariable", QString());
    scalarVariableComp = (PhysicFieldVariableComp) readConfig("SceneViewSettings/ScalarVariableComp", (int) PhysicFieldVariableComp_Scalar);
    // scalarVariable3D = readConfig("SceneViewSettings/ScalarVariable3D", QString());
    // scalarVariable3DComp = (PhysicFieldVariableComp) readConfig("SceneViewSettings/ScalarVariable3DComp", (int) PhysicFieldVariableComp_Scalar);
    paletteType = (PaletteType) readConfig("SceneViewSettings/PaletteType", PALETTETYPE);
    paletteFilter = readConfig("SceneViewSettings/PaletteFilter", PALETTEFILTER);
    paletteSteps = readConfig("SceneViewSettings/PaletteSteps", PALETTESTEPS);
    scalarRangeLog = readConfig("SceneViewSettings/ScalarRangeLog", SCALARFIELDRANGELOG);
    scalarRangeBase = readConfig("SceneViewSettings/ScalarRangeBase", SCALARFIELDRANGEBASE);
    scalarDecimalPlace = readConfig("SceneViewSettings/ScalarDecimalPlace", SCALARDECIMALPLACE);
    scalarRangeAuto = readConfig("SceneViewSettings/ScalarRangeAuto", SCALARRANGEAUTO);
    scalarRangeMin = readConfig("SceneViewSettings/ScalarRangeMin", SCALARRANGEMIN);
    scalarRangeMax = readConfig("SceneViewSettings/ScalarRangeMax", SCALARRANGEMAX);

    // vector view
    showVectorView = readConfig("SceneViewSettings/ShowVectorView", SHOWVECTORVIEW);
    vectorVariable = readConfig("SceneViewSettings/VectorVariable", QString());
    vectorProportional = readConfig("SceneViewSettings/VectorProportional", VECTORPROPORTIONAL);
    vectorColor = readConfig("SceneViewSettings/VectorColor", VECTORCOLOR);
    vectorCount = readConfig("SceneViewSettings/VectorNumber", VECTORCOUNT);
    vectorScale = readConfig("SceneViewSettings/VectorScale", VECTORSCALE);
    vectorType = (VectorType) readConfig("SceneViewSettings/VectorType", (int) VECTORTYPE);
    vectorCenter = (VectorCenter) readConfig("SceneViewSettings/VectorCenter", (int) VECTORCENTER);

    // order view
    showOrderView = readConfig("SceneViewSettings/ShowOrderView", SHOWORDERVIEW);
    showOrderColorBar = readConfig("SceneViewSettings/ShowOrderColorBar", SHOWORDERCOLORBAR);
    orderPaletteOrderType = (PaletteOrderType) readConfig("SceneViewSettings/OrderPaletteOrderType", ORDERPALETTEORDERTYPE);
    orderLabel = readConfig("SceneViewSettings/OrderLabel", ORDERLABEL);

    // particle tracing
    showParticleView = readConfig("SceneViewSettings/ShowParticleView", SHOWPARTICLEVIEW);
    particleIncludeGravitation = readConfig("SceneViewSettings/ParticleIncludeGravitation", PARTICLEINCLUDEGRAVITATION);
    particleMass = readConfig("SceneViewSettings/ParticleMass", PARTICLEMASS);
    particleConstant = readConfig("SceneViewSettings/ParticleConstant", PARTICLECONSTANT);
    particleStart.x = readConfig("SceneViewSettings/ParticleStartX", PARTICLESTARTX);
    particleStart.y = readConfig("SceneViewSettings/ParticleStartY", PARTICLESTARTY);
    particleStartVelocity.x = readConfig("SceneViewSettings/ParticleStartVelocityX", PARTICLESTARTVELOCITYX);
    particleStartVelocity.y = readConfig("SceneViewSettings/ParticleStartVelocityY", PARTICLESTARTVELOCITYY);
    particleNumberOfParticles = readConfig("SceneViewSettings/ParticleNumberOfParticles", PARTICLENUMBEROFPARTICLES);
    particleStartingRadius = readConfig("SceneViewSettings/ParticleStartingRadius", PARTICLESTARTINGRADIUS);
    particleReflectOnDifferentMaterial = readConfig("SceneViewSettings/ParticleReflectOnDifferentMaterial", PARTICLEREFLECTONDIFFERENTMATERIAL);
    particleReflectOnBoundary = readConfig("SceneViewSettings/ParticleReflectOnBoundary", PARTICLEREFLECTONBOUNDARY);
    particleCoefficientOfRestitution = readConfig("SceneViewSettings/ParticleCoefficientOfRestitution", PARTICLECOEFFICIENTOFRESTITUTION);
    particleMaximumRelativeError = readConfig("SceneViewSettings/ParticleMaximumRelativeError", PARTICLEMAXIMUMRELATIVEERROR);
    particleShowPoints = readConfig("SceneViewSettings/ParticleShowPoints", PARTICLESHOWPOINTS);
    particleColorByVelocity = readConfig("SceneViewSettings/ParticleColorByVelocity", PARTICLECOLORBYVELOCITY);
    particleMaximumNumberOfSteps = readConfig("SceneViewSettings/ParticleMaximumNumberOfSteps", PARTICLEMAXIMUMNUMBEROFSTEPS);
    particleMinimumStep = readConfig("SceneViewSettings/ParticleMinimumStep", PARTICLEMINIMUMSTEP);
    particleDragDensity = readConfig("SceneViewSettings/ParticleDragDensity", PARTICLEDRAGDENSITY);
    particleDragCoefficient = readConfig("SceneViewSettings/ParticleDragCoefficient", PARTICLEDRAGCOEFFICIENT);
    particleDragReferenceArea = readConfig("SceneViewSettings/ParticleDragReferenceArea", PARTICLEDRAGREFERENCEAREA);

    // mesh
    angleSegmentsCount = readConfig("SceneViewSettings/MeshAngleSegmentsCount", MESHANGLESEGMENTSCOUNT);
    curvilinearElements = readConfig("SceneViewSettings/MeshCurvilinearElements", MESHCURVILINEARELEMENTS);

    eleConfig = NULL;
}

void Config::loadAdvanced()
{
    QSettings settings;

    // adaptivity
    maxDofs = settings.value("Adaptivity/MaxDofs", MAX_DOFS).toInt();
    isoOnly = settings.value("Adaptivity/IsoOnly", ADAPTIVITY_ISOONLY).toBool();
    convExp = settings.value("Adaptivity/ConvExp", ADAPTIVITY_CONVEXP).toDouble();
    threshold = settings.value("Adaptivity/Threshold", ADAPTIVITY_THRESHOLD).toDouble();
    strategy = settings.value("Adaptivity/Strategy", ADAPTIVITY_STRATEGY).toInt();
    meshRegularity = settings.value("Adaptivity/MeshRegularity", ADAPTIVITY_MESHREGULARITY).toInt();
    projNormType = (Hermes::Hermes2D::ProjNormType) settings.value("Adaptivity/ProjNormType", ADAPTIVITY_PROJNORMTYPE).toInt();
    useAniso = settings.value("Adaptivity/UseAniso", ADAPTIVITY_ANISO).toBool();
    finerReference = settings.value("Adaptivity/FinerReference", ADAPTIVITY_FINER_REFERENCE_H_AND_P).toBool();

    // command argument
    commandGmsh = settings.value("Commands/Gmsh", COMMANDS_GMSH).toString();
    commandTriangle = settings.value("Commands/Triangle", COMMANDS_TRIANGLE).toString();
    // add quadratic elements (added points on the middle of edge used by rough triangle division)
    if (!commandTriangle.contains("-o2"))
        commandTriangle = COMMANDS_TRIANGLE;

    // number of threads
    numberOfThreads = settings.value("Parallel/NumberOfThreads", omp_get_max_threads()).toInt();
    if (numberOfThreads > omp_get_max_threads())
        numberOfThreads = omp_get_max_threads();
    Hermes::Hermes2D::Hermes2DApi.set_param_value(Hermes::Hermes2D::numThreads, numberOfThreads);

    // global script
    globalScript = settings.value("Python/GlobalScript", "").toString();
}

void Config::save()
{
    saveWorkspace();
    saveAdvanced();
}

void Config::saveWorkspace()
{
    QSettings settings;

    // std log
    settings.setValue("SceneViewSettings/LogStdOut", showLogStdOut);

    // general
    settings.setValue("General/GUIStyle", guiStyle);
    settings.setValue("General/Language", language);
    settings.setValue("General/DefaultPhysicField", defaultPhysicField);

    settings.setValue("General/CollaborationServerURL", collaborationServerURL);

    settings.setValue("General/CheckVersion", checkVersion);
    settings.setValue("General/LineEditValueShowResult", lineEditValueShowResult);
    if (showExperimentalFeatures)
        settings.setValue("General/SaveProblemWithSolution", saveProblemWithSolution);
    else
        saveProblemWithSolution = false;

    // delete files
    settings.setValue("Solver/DeleteTriangleMeshFiles", deleteMeshFiles);
    settings.setValue("Solver/DeleteHermes2DMeshFile", deleteHermesMeshFile);

    // font
    settings.setValue("SceneViewSettings/RulersFont", rulersFont);
    settings.setValue("SceneViewSettings/PostFont", postFont);

    // zoom
    settings.setValue("General/ZoomToMouse", zoomToMouse);

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

    // discrete
    settings.setValue("SceneViewSettings/SaveMatrixAndRHS", saveMatrixRHS);
    settings.setValue("SceneViewSettings/DiscreteDirectory", discreteDirectory);

    // grid
    settings.setValue("SceneViewSettings/ShowGrid", showGrid);
    settings.setValue("SceneViewSettings/GridStep", gridStep);

    // rulers
    settings.setValue("SceneViewSettings/ShowRulers", showRulers);
    // snap to grid
    settings.setValue("SceneViewSettings/SnapToGrid", snapToGrid);

    // axes
    settings.setValue("SceneViewSettings/ShowAxes", showAxes);

    // linearizer quality
    settings.setValue("SceneViewSettings/LinearizerQuality", paletteQualityToStringKey(linearizerQuality));

    // 3d
    settings.setValue("SceneViewSettings/ScalarView3DLighting", scalarView3DLighting);
    settings.setValue("SceneViewSettings/ScalarView3DAngle", scalarView3DAngle);
    settings.setValue("SceneViewSettings/ScalarView3DBackground", scalarView3DBackground);
    settings.setValue("SceneViewSettings/ScalarView3DHeight", scalarView3DHeight);
    settings.setValue("SceneViewSettings/ScalarView3DBoundingBox", scalarView3DBoundingBox);

    // deformations
    settings.setValue("SceneViewSettings/DeformScalar", deformScalar);
    settings.setValue("SceneViewSettings/DeformContour", deformContour);
    settings.setValue("SceneViewSettings/DeformVector", deformVector);
}

void Config::savePostprocessor(QDomElement *config)
{
    eleConfig = config;

    // active field
    writeConfig("SceneViewSettings/ActiveField", activeField);

    // mesh
    writeConfig("SceneViewSettings/ShowInitialMeshView", showInitialMeshView);
    writeConfig("SceneViewSettings/ShowSolutionMeshView", showSolutionMeshView);

    writeConfig("SceneViewSettings/ShowPost3D", showPost3D);

    // contour
    writeConfig("SceneViewSettings/ContourVariable", contourVariable);
    writeConfig("SceneViewSettings/ShowContourView", showContourView);
    writeConfig("SceneViewSettings/ContoursCount", contoursCount);
    writeConfig("SceneViewSettings/ContoursWidth", contourWidth);

    // scalar view
    writeConfig("SceneViewSettings/ShowScalarView", showScalarView);
    writeConfig("SceneViewSettings/ShowScalarColorBar", showScalarColorBar);
    writeConfig("SceneViewSettings/ScalarVariable", scalarVariable);
    writeConfig("SceneViewSettings/ScalarVariableComp", scalarVariableComp);
    // writeConfig("SceneViewSettings/ScalarVariable3D", scalarVariable3D);
    // writeConfig("SceneViewSettings/ScalarVariable3DComp", scalarVariable3DComp);
    writeConfig("SceneViewSettings/PaletteType", paletteType);
    writeConfig("SceneViewSettings/PaletteFilter", paletteFilter);
    writeConfig("SceneViewSettings/PaletteSteps", paletteSteps);
    writeConfig("SceneViewSettings/ScalarRangeLog", scalarRangeLog);
    writeConfig("SceneViewSettings/ScalarRangeBase", scalarRangeBase);
    writeConfig("SceneViewSettings/ScalarDecimalPlace", scalarDecimalPlace);
    writeConfig("SceneViewSettings/ScalarRangeAuto", scalarRangeAuto);
    writeConfig("SceneViewSettings/ScalarRangeMin", scalarRangeMin);
    writeConfig("SceneViewSettings/ScalarRangeMax", scalarRangeMax);

    // vector view
    writeConfig("SceneViewSettings/ShowVectorView", showVectorView);
    writeConfig("SceneViewSettings/VectorVariable", vectorVariable);
    writeConfig("SceneViewSettings/VectorProportional", vectorProportional);
    writeConfig("SceneViewSettings/VectorColor", vectorColor);
    writeConfig("SceneViewSettings/VectorNumber", vectorCount);
    writeConfig("SceneViewSettings/VectorScale", vectorScale);
    writeConfig("SceneViewSettings/VectorType", vectorType);
    writeConfig("SceneViewSettings/VectorCenter", vectorCenter);

    // order view
    writeConfig("SceneViewSettings/ShowOrderView", showOrderView);
    writeConfig("SceneViewSettings/ShowOrderColorBar", showOrderColorBar);
    writeConfig("SceneViewSettings/OrderPaletteOrderType", orderPaletteOrderType);
    writeConfig("SceneViewSettings/OrderLabel", orderLabel);

    // particle tracing
    writeConfig("SceneViewSettings/ShowParticleView", showParticleView);
    writeConfig("SceneViewSettings/ParticleIncludeGravitation", particleIncludeGravitation);
    writeConfig("SceneViewSettings/ParticleMass", particleMass);
    writeConfig("SceneViewSettings/ParticleConstant", particleConstant);
    writeConfig("SceneViewSettings/ParticleStartX", particleStart.x);
    writeConfig("SceneViewSettings/ParticleStartY", particleStart.y);
    writeConfig("SceneViewSettings/ParticleStartVelocityX", particleStartVelocity.x);
    writeConfig("SceneViewSettings/ParticleStartVelocityY", particleStartVelocity.y);
    writeConfig("SceneViewSettings/ParticleNumberOfParticles", particleNumberOfParticles);
    writeConfig("SceneViewSettings/ParticleStartingRadius", particleStartingRadius);
    writeConfig("SceneViewSettings/ParticleReflectOnDifferentMaterial", particleReflectOnDifferentMaterial);
    writeConfig("SceneViewSettings/ParticleReflectOnBoundary", particleReflectOnBoundary);
    writeConfig("SceneViewSettings/ParticleCoefficientOfRestitution", particleCoefficientOfRestitution);
    writeConfig("SceneViewSettings/ParticleMaximumRelativeError", particleMaximumRelativeError);
    writeConfig("SceneViewSettings/ParticleShowPoints", particleShowPoints);
    writeConfig("SceneViewSettings/ParticleColorByVelocity", particleColorByVelocity);
    writeConfig("SceneViewSettings/ParticleMaximumNumberOfSteps", particleMaximumNumberOfSteps);
    writeConfig("SceneViewSettings/ParticleMinimumStep", particleMinimumStep);
    writeConfig("SceneViewSettings/ParticleDragDensity", particleDragDensity);
    writeConfig("SceneViewSettings/ParticleDragCoefficient", particleDragCoefficient);
    writeConfig("SceneViewSettings/ParticleDragReferenceArea", particleDragReferenceArea);

    // mesh
    writeConfig("SceneViewSettings/MeshAngleSegmentsCount", angleSegmentsCount);
    writeConfig("SceneViewSettings/MeshCurvilinearElements", curvilinearElements);

    eleConfig = NULL;
}

void Config::saveAdvanced()
{
    QSettings settings;

    // adaptivity
    settings.setValue("Adaptivity/MaxDofs", maxDofs);
    settings.setValue("Adaptivity/IsoOnly", isoOnly);
    settings.setValue("Adaptivity/ConvExp", convExp);
    settings.setValue("Adaptivity/Threshold", threshold);
    settings.setValue("Adaptivity/Strategy", strategy);
    settings.setValue("Adaptivity/MeshRegularity", meshRegularity);
    settings.setValue("Adaptivity/ProjNormType", projNormType);
    settings.setValue("Adaptivity/UseAniso", useAniso);
    settings.setValue("Adaptivity/FinerReference", finerReference);

    // command argument
    settings.setValue("Commands/Triangle", commandTriangle);
    settings.setValue("Commands/Gmsh", commandGmsh);

    // number of threads
    settings.setValue("Parallel/NumberOfThreads", numberOfThreads);
    Hermes::Hermes2D::Hermes2DApi.set_param_value(Hermes::Hermes2D::numThreads, numberOfThreads);

    // global script
    settings.setValue("Python/GlobalScript", globalScript);
}

bool Config::readConfig(const QString &key, bool defaultValue)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        if (eleConfig->hasAttribute(att))
            return (eleConfig->attribute(att).toInt() == 1) ? true : false;
    }

    return defaultValue;
}

int Config::readConfig(const QString &key, int defaultValue)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        if (eleConfig->hasAttribute(att))
            return eleConfig->attribute(att).toInt();
    }

    return defaultValue;
}

double Config::readConfig(const QString &key, double defaultValue)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        if (eleConfig->hasAttribute(att))
            return eleConfig->attribute(att).toDouble();
    }

    return defaultValue;
}

QString Config::readConfig(const QString &key, const QString &defaultValue)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        if (eleConfig->hasAttribute(att))
            return eleConfig->attribute(att);
    }

    return defaultValue;
}

void Config::writeConfig(const QString &key, bool value)
{
    QString att = key; att.replace("/", "_");
    eleConfig->setAttribute(att, value);
}

void Config::writeConfig(const QString &key, int value)
{
    QString att = key; att.replace("/", "_");
    eleConfig->setAttribute(att, value);
}

void Config::writeConfig(const QString &key, double value)
{
    QString att = key; att.replace("/", "_");
    eleConfig->setAttribute(att, value);
}

void Config::writeConfig(const QString &key, const QString &value)
{
    QString att = key; att.replace("/", "_");
    eleConfig->setAttribute(att, value);
}
