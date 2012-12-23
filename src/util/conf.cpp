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

ConfigComputer::ConfigComputer()
{
    // set xml schemas dir
    Hermes::Hermes2D::Hermes2DApi.set_text_param_value(Hermes::Hermes2D::xmlSchemasDirPath, QString("%1/resources/xsd").arg(datadir()).toStdString());

    load();
}

ConfigComputer::~ConfigComputer()
{
    save();
}

void ConfigComputer::load()
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
    saveProblemWithSolution = settings.value("Solver/SaveProblemWithSolution", false).toBool();

    // delete files
    deleteMeshFiles = settings.value("Solver/DeleteTriangleMeshFiles", true).toBool();
    deleteHermesMeshFile = settings.value("Solver/DeleteHermes2DMeshFile", true).toBool();

    // discrete
    saveMatrixRHS = settings.value("SceneViewSettings/SaveMatrixAndRHS", SAVEMATRIXANDRHS).toBool();

    // cache size
    cacheSize = settings.value("Solution/CacheSize", CACHE_SIZE).toInt();

    // number of threads
    numberOfThreads = settings.value("Parallel/NumberOfThreads", omp_get_max_threads()).toInt();
    if (numberOfThreads > omp_get_max_threads())
        numberOfThreads = omp_get_max_threads();
    Hermes::Hermes2D::Hermes2DApi.set_integral_param_value(Hermes::Hermes2D::numThreads, numberOfThreads);

    // global script
    globalScript = settings.value("Python/GlobalScript", "").toString();
}

void ConfigComputer::save()
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
    settings.setValue("General/SaveProblemWithSolution", saveProblemWithSolution);

    // delete files
    settings.setValue("Solver/DeleteTriangleMeshFiles", deleteMeshFiles);
    settings.setValue("Solver/DeleteHermes2DMeshFile", deleteHermesMeshFile);

    // discrete
    settings.setValue("SceneViewSettings/SaveMatrixAndRHS", saveMatrixRHS);

    // cache size
    settings.setValue("Solution/CacheSize", cacheSize);

    // number of threads
    settings.setValue("Parallel/NumberOfThreads", numberOfThreads);
    Hermes::Hermes2D::Hermes2DApi.set_integral_param_value(Hermes::Hermes2D::numThreads, numberOfThreads);

    // global script
    settings.setValue("Python/GlobalScript", globalScript);
}

// ********************************************************************************************

Config::Config() : eleConfig(NULL)
{
    clear();
}

Config::~Config()
{
}

void Config::clear()
{
    load(NULL);
}

void Config::load(QDomElement *config)
{
    eleConfig = config;

    // active field
    activeField = readConfig("SceneViewSettings/ActiveField", QString());

    // zoom
    zoomToMouse = readConfig("Geometry/ZoomToMouse", ZOOMTOMOUSE);

    // font
    rulersFont = readConfig("SceneViewSettings/RulersFont", RULERSFONT);
    postFont = readConfig("SceneViewSettings/PostFont", POSTFONT);

    // geometry
    nodeSize = readConfig("SceneViewSettings/NodeSize", GEOMETRYNODESIZE);
    edgeWidth = readConfig("SceneViewSettings/EdgeWidth", GEOMETRYEDGEWIDTH);
    labelSize = readConfig("SceneViewSettings/LabelSize", GEOMETRYLABELSIZE);

    // grid
    showGrid = readConfig("SceneViewSettings/ShowGrid", SHOWGRID);
    gridStep = readConfig("SceneViewSettings/GridStep", GRIDSTEP);

    // rulers
    showRulers = readConfig("SceneViewSettings/ShowRulers", SHOWRULERS);
    // snap to grid
    snapToGrid = readConfig("SceneViewSettings/SnapToGrid", SNAPTOGRID);

    // axes
    showAxes = readConfig("SceneViewSettings/ShowAxes", SHOWAXES);

    // 3d
    scalarView3DLighting = readConfig("SceneViewSettings/ScalarView3DLighting", VIEW3DLIGHTING);
    scalarView3DAngle = readConfig("SceneViewSettings/ScalarView3DAngle", VIEW3DANGLE);
    scalarView3DBackground = readConfig("SceneViewSettings/ScalarView3DBackground", VIEW3DBACKGROUND);
    scalarView3DHeight = readConfig("SceneViewSettings/ScalarView3DHeight", VIEW3DHEIGHT);
    scalarView3DBoundingBox = readConfig("SceneViewSettings/ScalarView3DBoundingBox", VIEW3DBOUNDINGBOX);

    // deformations
    deformScalar = readConfig("SceneViewSettings/DeformScalar", DEFORMSCALAR);
    deformContour = readConfig("SceneViewSettings/DeformContour", DEFORMCONTOUR);
    deformVector = readConfig("SceneViewSettings/DeformVector", DEFORMVECTOR);

    // colors
    colorBackground = readConfig("SceneViewSettings/ColorBackground", COLORBACKGROUND);
    colorGrid = readConfig("SceneViewSettings/ColorGrid", COLORGRID);
    colorCross = readConfig("SceneViewSettings/ColorCross", COLORCROSS);
    colorNodes = readConfig("SceneViewSettings/ColorNodes", COLORNODES);
    colorEdges = readConfig("SceneViewSettings/ColorEdges", COLOREDGES);
    colorLabels = readConfig("SceneViewSettings/ColorLabels", COLORLABELS);
    colorContours = readConfig("SceneViewSettings/ColorContours", COLORCONTOURS);
    colorVectors = readConfig("SceneViewSettings/ColorVectors", COLORVECTORS);
    colorInitialMesh = readConfig("SceneViewSettings/ColorInitialMesh", COLORINITIALMESH);
    colorSolutionMesh = readConfig("SceneViewSettings/ColorSolutionMesh", COLORSOLUTIONMESH);
    colorHighlighted = readConfig("SceneViewSettings/ColorHighlighted", COLORHIGHLIGHTED);
    colorSelected = readConfig("SceneViewSettings/ColorSelected", COLORSELECTED);
    colorCrossed = readConfig("SceneViewSettings/ColorCrossed", COLORCROSSED);
    colorNotConnected = readConfig("SceneViewSettings/ColorCrossed", COLORNOTCONNECTED);

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
    particleIncludeRelativisticCorrection = readConfig("SceneViewSettings/ParticleIncludeRelativisticCorrection", PARTICLEINCLUDERELATIVISTICCORRECTION);
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
    particleCustomForce.x = readConfig("SceneViewSettings/ParticleCustomForceX", PARTICLECUSTOMFORCEX);
    particleCustomForce.y = readConfig("SceneViewSettings/ParticleCustomForceY", PARTICLECUSTOMFORCEY);
    particleCustomForce.z = readConfig("SceneViewSettings/ParticleCustomForceZ", PARTICLECUSTOMFORCEZ);

    // mesh
    angleSegmentsCount = readConfig("SceneViewSettings/MeshAngleSegmentsCount", MESHANGLESEGMENTSCOUNT);
    curvilinearElements = readConfig("SceneViewSettings/MeshCurvilinearElements", MESHCURVILINEARELEMENTS);

    // adaptivity
    maxDofs = readConfig("Adaptivity/MaxDofs", MAX_DOFS);
    isoOnly = readConfig("Adaptivity/IsoOnly", ADAPTIVITY_ISOONLY);
    convExp = readConfig("Adaptivity/ConvExp", ADAPTIVITY_CONVEXP);
    threshold = readConfig("Adaptivity/Threshold", ADAPTIVITY_THRESHOLD);
    strategy = readConfig("Adaptivity/Strategy", ADAPTIVITY_STRATEGY);
    meshRegularity = readConfig("Adaptivity/MeshRegularity", ADAPTIVITY_MESHREGULARITY);
    projNormType = (Hermes::Hermes2D::ProjNormType) readConfig("Adaptivity/ProjNormType", ADAPTIVITY_PROJNORMTYPE);
    useAniso = readConfig("Adaptivity/UseAniso", ADAPTIVITY_ANISO);
    finerReference = readConfig("Adaptivity/FinerReference", ADAPTIVITY_FINER_REFERENCE_H_AND_P);

    // linearizer quality
    QString quality = readConfig("SceneViewSettings/LinearizerQuality", paletteQualityToStringKey(PALETTEQUALITY));
    linearizerQuality = paletteQualityFromStringKey(quality);

    // solid view
    solidViewHide = readConfig("SceneViewSettings/SolidViewHide", QStringList());

    // command argument
    commandGmsh = readConfig("Commands/Gmsh", COMMANDS_GMSH);
    commandTriangle = readConfig("Commands/Triangle", COMMANDS_TRIANGLE);
    // add quadratic elements (added points on the middle of edge used by rough triangle division)
    if (!commandTriangle.contains("-o2"))
        commandTriangle = COMMANDS_TRIANGLE;

    eleConfig = NULL;
}

void Config::save(QDomElement *config)
{
    eleConfig = config;

    // font
    writeConfig("SceneViewSettings/RulersFont", rulersFont);
    writeConfig("SceneViewSettings/PostFont", postFont);

    // zoom
    writeConfig("General/ZoomToMouse", zoomToMouse);

    // geometry
    writeConfig("SceneViewSettings/NodeSize", nodeSize);
    writeConfig("SceneViewSettings/EdgeWidth", edgeWidth);
    writeConfig("SceneViewSettings/LabelSize", labelSize);

    // grid
    writeConfig("SceneViewSettings/ShowGrid", showGrid);
    writeConfig("SceneViewSettings/GridStep", gridStep);

    // rulers
    writeConfig("SceneViewSettings/ShowRulers", showRulers);
    // snap to grid
    writeConfig("SceneViewSettings/SnapToGrid", snapToGrid);

    // axes
    writeConfig("SceneViewSettings/ShowAxes", showAxes);

    // 3d
    writeConfig("SceneViewSettings/ScalarView3DLighting", scalarView3DLighting);
    writeConfig("SceneViewSettings/ScalarView3DAngle", scalarView3DAngle);
    writeConfig("SceneViewSettings/ScalarView3DBackground", scalarView3DBackground);
    writeConfig("SceneViewSettings/ScalarView3DHeight", scalarView3DHeight);
    writeConfig("SceneViewSettings/ScalarView3DBoundingBox", scalarView3DBoundingBox);

    // deformations
    writeConfig("SceneViewSettings/DeformScalar", deformScalar);
    writeConfig("SceneViewSettings/DeformContour", deformContour);
    writeConfig("SceneViewSettings/DeformVector", deformVector);

    // colors
    writeConfig("SceneViewSettings/ColorBackground", colorBackground);
    writeConfig("SceneViewSettings/ColorGrid", colorGrid);
    writeConfig("SceneViewSettings/ColorCross", colorCross);
    writeConfig("SceneViewSettings/ColorNodes", colorNodes);
    writeConfig("SceneViewSettings/ColorEdges", colorEdges);
    writeConfig("SceneViewSettings/ColorLabels", colorLabels);
    writeConfig("SceneViewSettings/ColorContours", colorContours);
    writeConfig("SceneViewSettings/ColorVectors", colorVectors);
    writeConfig("SceneViewSettings/ColorInitialMesh", colorInitialMesh);
    writeConfig("SceneViewSettings/ColorSolutionMesh", colorSolutionMesh);
    writeConfig("SceneViewSettings/ColorInitialMesh", colorHighlighted);
    writeConfig("SceneViewSettings/ColorSolutionMesh", colorSelected);

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
    writeConfig("SceneViewSettings/ParticleIncludeRelativisticCorrection", particleIncludeRelativisticCorrection);
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
    writeConfig("SceneViewSettings/ParticleCustomForceX", particleCustomForce.x);
    writeConfig("SceneViewSettings/ParticleCustomForceY", particleCustomForce.y);
    writeConfig("SceneViewSettings/ParticleCustomForceZ", particleCustomForce.z);

    // mesh
    writeConfig("SceneViewSettings/MeshAngleSegmentsCount", angleSegmentsCount);
    writeConfig("SceneViewSettings/MeshCurvilinearElements", curvilinearElements);

    // adaptivity
    writeConfig("Adaptivity/MaxDofs", maxDofs);
    writeConfig("Adaptivity/IsoOnly", isoOnly);
    writeConfig("Adaptivity/ConvExp", convExp);
    writeConfig("Adaptivity/Threshold", threshold);
    writeConfig("Adaptivity/Strategy", strategy);
    writeConfig("Adaptivity/MeshRegularity", meshRegularity);
    writeConfig("Adaptivity/ProjNormType", projNormType);
    writeConfig("Adaptivity/UseAniso", useAniso);
    writeConfig("Adaptivity/FinerReference", finerReference);

    // linearizer quality
    writeConfig("SceneViewSettings/LinearizerQuality", paletteQualityToStringKey(linearizerQuality));

    // solid view
    writeConfig("SceneViewSettings/SolidViewHide", solidViewHide);

    // command argument
    writeConfig("Commands/Triangle", commandTriangle);
    writeConfig("Commands/Gmsh", commandGmsh);

    eleConfig = NULL;
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

QColor Config::readConfig(const QString &key, const QColor &defaultValue)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        if (eleConfig->hasAttribute(att))
        {
            QColor color;

            color.setRed(eleConfig->attribute(att + "_red").toInt());
            color.setGreen(eleConfig->attribute(att + "_green").toInt());
            color.setBlue(eleConfig->attribute(att + "_blue").toInt());

            return color;
        }
    }

    return defaultValue;
}

QStringList Config::readConfig(const QString &key, const QStringList &defaultValue)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        if (eleConfig->hasAttribute(att))
            return eleConfig->attribute(att).split("|");
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

void Config::writeConfig(const QString &key, const QColor &value)
{
    QString att = key; att.replace("/", "_");
    eleConfig->setAttribute(att + "_red", value.red());
    eleConfig->setAttribute(att + "_green", value.green());
    eleConfig->setAttribute(att + "_blue", value.blue());
}

void Config::writeConfig(const QString &key, const QStringList &value)
{
    QString att = key; att.replace("/", "_");
    eleConfig->setAttribute(att, value.join("|"));
}
