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

#include "problem_config.h"

#include "util/global.h"
#include "util/constants.h"

#include "field.h"
#include "block.h"
#include "solutionstore.h"

#include "scene.h"
#include "scenemarker.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "module.h"
#include "coupling.h"
#include "solver.h"
#include "meshgenerator_triangle.h"
#include "meshgenerator_gmsh.h"
#include "logview.h"

ProblemConfig::ProblemConfig(QWidget *parent) : QObject(parent)
{
    clear();
}

void ProblemConfig::clear()
{
    m_coordinateType = CoordinateType_Planar;
    m_fileName = "";

    // matrix solver
    m_matrixSolver = Hermes::SOLVER_UMFPACK;

    // mesh type
    m_meshType = MeshType_Triangle;

    // harmonic
    m_frequency = 0.0;

    // transient
    m_timeStepMethod = TimeStepMethod_BDFNumSteps;
    m_timeOrder = 2;
    m_timeMethodTolerance = Value(0.1);
    m_timeTotal = Value(1.0);
    m_numConstantTimeSteps = 10;
}

bool ProblemConfig::isTransientAdaptive() const
{
    if((m_timeStepMethod == TimeStepMethod_BDFTolerance) || (m_timeStepMethod == TimeStepMethod_BDFNumSteps))
        return true;
    else if(m_timeStepMethod == TimeStepMethod_Fixed)
        return false;

    assert(0);
}

// todo: put to gui
const double initialTimeStepRatio = 500;
double ProblemConfig::initialTimeStepLength()
{
    if(timeStepMethod() == TimeStepMethod_BDFTolerance)
        return timeTotal().value() / initialTimeStepRatio;
    else if(timeStepMethod() == TimeStepMethod_BDFNumSteps)
        return constantTimeStepLength() / 3.;
    else if (timeStepMethod() == TimeStepMethod_Fixed)
        return constantTimeStepLength();
    else
        assert(0);
}

// ********************************************************************************************

ProblemConfigView::ProblemConfigView() : eleConfig(NULL)
{
    clear();
}

ProblemConfigView::~ProblemConfigView()
{
}

void ProblemConfigView::clear()
{
    load(NULL);
}

void ProblemConfigView::load(QDomElement *config)
{
    eleConfig = config;

    // FIX ME - EOL conversion
    QPlainTextEdit textEdit;
    textEdit.setPlainText(readConfig("Problem/StartupScript", QString()));
    startupScript = textEdit.toPlainText();

    description = readConfig("Problem/Description", QString());

    // active field
    activeField = readConfig("View/ActiveField", QString());

    // font
    rulersFont = readConfig("View/RulersFont", RULERSFONT);
    postFont = readConfig("View/PostFont", POSTFONT);

    // geometry
    nodeSize = readConfig("View/NodeSize", GEOMETRYNODESIZE);
    edgeWidth = readConfig("View/EdgeWidth", GEOMETRYEDGEWIDTH);
    labelSize = readConfig("View/LabelSize", GEOMETRYLABELSIZE);

    // zoom and grid
    zoomToMouse = readConfig("View/ZoomToMouse", ZOOMTOMOUSE);
    showGrid = readConfig("View/ShowGrid", SHOWGRID);
    gridStep = readConfig("View/GridStep", GRIDSTEP);

    // rulers
    showRulers = readConfig("View/ShowRulers", SHOWRULERS);
    // snap to grid
    snapToGrid = readConfig("View/SnapToGrid", SNAPTOGRID);

    // axes
    showAxes = readConfig("View/ShowAxes", SHOWAXES);

    // 3d
    scalarView3DLighting = readConfig("View/ScalarView3DLighting", VIEW3DLIGHTING);
    scalarView3DAngle = readConfig("View/ScalarView3DAngle", VIEW3DANGLE);
    scalarView3DBackground = readConfig("View/ScalarView3DBackground", VIEW3DBACKGROUND);
    scalarView3DHeight = readConfig("View/ScalarView3DHeight", VIEW3DHEIGHT);
    scalarView3DBoundingBox = readConfig("View/ScalarView3DBoundingBox", VIEW3DBOUNDINGBOX);
    scalarView3DSolidGeometry = readConfig("View/ScalarView3DSolidGeometry", VIEW3DSOLIDGEOMETRY);

    // deformations
    deformScalar = readConfig("View/DeformScalar", DEFORMSCALAR);
    deformContour = readConfig("View/DeformContour", DEFORMCONTOUR);
    deformVector = readConfig("View/DeformVector", DEFORMVECTOR);

    // colors
    colorBackground = readConfig("View/ColorBackground", COLORBACKGROUND);
    colorGrid = readConfig("View/ColorGrid", COLORGRID);
    colorCross = readConfig("View/ColorCross", COLORCROSS);
    colorNodes = readConfig("View/ColorNodes", COLORNODES);
    colorEdges = readConfig("View/ColorEdges", COLOREDGES);
    colorLabels = readConfig("View/ColorLabels", COLORLABELS);
    colorContours = readConfig("View/ColorContours", COLORCONTOURS);
    colorVectors = readConfig("View/ColorVectors", COLORVECTORS);
    colorInitialMesh = readConfig("View/ColorInitialMesh", COLORINITIALMESH);
    colorSolutionMesh = readConfig("View/ColorSolutionMesh", COLORSOLUTIONMESH);
    colorHighlighted = readConfig("View/ColorHighlighted", COLORHIGHLIGHTED);
    colorSelected = readConfig("View/ColorSelected", COLORSELECTED);
    colorCrossed = readConfig("View/ColorCrossed", COLORCROSSED);
    colorNotConnected = readConfig("View/ColorCrossed", COLORNOTCONNECTED);

    // view
    showPost3D = (SceneViewPost3DMode) readConfig("View/ShowPost3D", (int) SCALARSHOWPOST3D);

    // mesh
    showInitialMeshView = readConfig("View/ShowInitialMeshView", SHOWINITIALMESHVIEW);
    showSolutionMeshView = readConfig("View/ShowSolutionMeshView", SHOWSOLUTIONMESHVIEW);

    // contour
    showContourView = readConfig("View/ShowContourView", SHOWCONTOURVIEW);
    contourVariable = readConfig("View/ContourVariable", QString());
    contoursCount = readConfig("View/ContoursCount", CONTOURSCOUNT);
    contourWidth = readConfig("View/ContoursWidth", CONTOURSWIDTH);

    // scalar view
    showScalarView = readConfig("View/ShowScalarView", SHOWSCALARVIEW);
    showScalarColorBar = readConfig("View/ShowScalarColorBar", SHOWSCALARCOLORBAR);
    scalarVariable = readConfig("View/ScalarVariable", QString());
    scalarVariableComp = (PhysicFieldVariableComp) readConfig("View/ScalarVariableComp", (int) PhysicFieldVariableComp_Scalar);
    // scalarVariable3D = readConfig("View/ScalarVariable3D", QString());
    // scalarVariable3DComp = (PhysicFieldVariableComp) readConfig("View/ScalarVariable3DComp", (int) PhysicFieldVariableComp_Scalar);
    paletteType = (PaletteType) readConfig("View/PaletteType", PALETTETYPE);
    paletteFilter = readConfig("View/PaletteFilter", PALETTEFILTER);
    paletteSteps = readConfig("View/PaletteSteps", PALETTESTEPS);
    scalarRangeLog = readConfig("View/ScalarRangeLog", SCALARFIELDRANGELOG);
    scalarRangeBase = readConfig("View/ScalarRangeBase", SCALARFIELDRANGEBASE);
    scalarDecimalPlace = readConfig("View/ScalarDecimalPlace", SCALARDECIMALPLACE);
    scalarRangeAuto = readConfig("View/ScalarRangeAuto", SCALARRANGEAUTO);
    scalarRangeMin = readConfig("View/ScalarRangeMin", SCALARRANGEMIN);
    scalarRangeMax = readConfig("View/ScalarRangeMax", SCALARRANGEMAX);

    // vector view
    showVectorView = readConfig("View/ShowVectorView", SHOWVECTORVIEW);
    vectorVariable = readConfig("View/VectorVariable", QString());
    vectorProportional = readConfig("View/VectorProportional", VECTORPROPORTIONAL);
    vectorColor = readConfig("View/VectorColor", VECTORCOLOR);
    vectorCount = readConfig("View/VectorNumber", VECTORCOUNT);
    vectorScale = readConfig("View/VectorScale", VECTORSCALE);
    vectorType = (VectorType) readConfig("View/VectorType", (int) VECTORTYPE);
    vectorCenter = (VectorCenter) readConfig("View/VectorCenter", (int) VECTORCENTER);

    // order view
    showOrderView = readConfig("View/ShowOrderView", SHOWORDERVIEW);
    showOrderColorBar = readConfig("View/ShowOrderColorBar", SHOWORDERCOLORBAR);
    orderPaletteOrderType = (PaletteOrderType) readConfig("View/OrderPaletteOrderType", ORDERPALETTEORDERTYPE);
    orderLabel = readConfig("View/OrderLabel", ORDERLABEL);

    // particle tracing
    showParticleView = readConfig("View/ShowParticleView", SHOWPARTICLEVIEW);
    particleIncludeRelativisticCorrection = readConfig("View/ParticleIncludeRelativisticCorrection", PARTICLEINCLUDERELATIVISTICCORRECTION);
    particleMass = readConfig("View/ParticleMass", PARTICLEMASS);
    particleConstant = readConfig("View/ParticleConstant", PARTICLECONSTANT);
    particleStart.x = readConfig("View/ParticleStartX", PARTICLESTARTX);
    particleStart.y = readConfig("View/ParticleStartY", PARTICLESTARTY);
    particleStartVelocity.x = readConfig("View/ParticleStartVelocityX", PARTICLESTARTVELOCITYX);
    particleStartVelocity.y = readConfig("View/ParticleStartVelocityY", PARTICLESTARTVELOCITYY);
    particleNumberOfParticles = readConfig("View/ParticleNumberOfParticles", PARTICLENUMBEROFPARTICLES);
    particleStartingRadius = readConfig("View/ParticleStartingRadius", PARTICLESTARTINGRADIUS);
    particleReflectOnDifferentMaterial = readConfig("View/ParticleReflectOnDifferentMaterial", PARTICLEREFLECTONDIFFERENTMATERIAL);
    particleReflectOnBoundary = readConfig("View/ParticleReflectOnBoundary", PARTICLEREFLECTONBOUNDARY);
    particleCoefficientOfRestitution = readConfig("View/ParticleCoefficientOfRestitution", PARTICLECOEFFICIENTOFRESTITUTION);
    particleMaximumRelativeError = readConfig("View/ParticleMaximumRelativeError", PARTICLEMAXIMUMRELATIVEERROR);
    particleShowPoints = readConfig("View/ParticleShowPoints", PARTICLESHOWPOINTS);
    particleColorByVelocity = readConfig("View/ParticleColorByVelocity", PARTICLECOLORBYVELOCITY);
    particleMaximumNumberOfSteps = readConfig("View/ParticleMaximumNumberOfSteps", PARTICLEMAXIMUMNUMBEROFSTEPS);
    particleMinimumStep = readConfig("View/ParticleMinimumStep", PARTICLEMINIMUMSTEP);
    particleDragDensity = readConfig("View/ParticleDragDensity", PARTICLEDRAGDENSITY);
    particleDragCoefficient = readConfig("View/ParticleDragCoefficient", PARTICLEDRAGCOEFFICIENT);
    particleDragReferenceArea = readConfig("View/ParticleDragReferenceArea", PARTICLEDRAGREFERENCEAREA);
    particleCustomForce.x = readConfig("View/ParticleCustomForceX", PARTICLECUSTOMFORCEX);
    particleCustomForce.y = readConfig("View/ParticleCustomForceY", PARTICLECUSTOMFORCEY);
    particleCustomForce.z = readConfig("View/ParticleCustomForceZ", PARTICLECUSTOMFORCEZ);

    // mesh
    angleSegmentsCount = readConfig("View/MeshAngleSegmentsCount", MESHANGLESEGMENTSCOUNT);
    curvilinearElements = readConfig("View/MeshCurvilinearElements", MESHCURVILINEARELEMENTS);

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
    QString quality = readConfig("View/LinearizerQuality", paletteQualityToStringKey(PALETTEQUALITY));
    linearizerQuality = paletteQualityFromStringKey(quality);

    // solid view
    solidViewHide = readConfig("View/SolidViewHide", QStringList());

    // command argument
    commandGmsh = readConfig("Commands/Gmsh", COMMANDS_GMSH);
    commandTriangle = readConfig("Commands/Triangle", COMMANDS_TRIANGLE);
    // add quadratic elements (added points on the middle of edge used by rough triangle division)
    if (!commandTriangle.contains("-o2"))
        commandTriangle = COMMANDS_TRIANGLE;

    eleConfig = NULL;
}

void ProblemConfigView::save(QDomElement *config)
{
    eleConfig = config;

    writeConfig("Problem/StartupScript", startupScript);
    writeConfig("Problem/Description", description);

    // font
    writeConfig("View/RulersFont", rulersFont);
    writeConfig("View/PostFont", postFont);

    // zoom
    writeConfig("General/ZoomToMouse", zoomToMouse);

    // geometry
    writeConfig("View/NodeSize", nodeSize);
    writeConfig("View/EdgeWidth", edgeWidth);
    writeConfig("View/LabelSize", labelSize);

    // grid
    writeConfig("View/ShowGrid", showGrid);
    writeConfig("View/GridStep", gridStep);

    // rulers
    writeConfig("View/ShowRulers", showRulers);
    // snap to grid
    writeConfig("View/SnapToGrid", snapToGrid);

    // axes
    writeConfig("View/ShowAxes", showAxes);

    // 3d
    writeConfig("View/ScalarView3DLighting", scalarView3DLighting);
    writeConfig("View/ScalarView3DAngle", scalarView3DAngle);
    writeConfig("View/ScalarView3DBackground", scalarView3DBackground);
    writeConfig("View/ScalarView3DHeight", scalarView3DHeight);
    writeConfig("View/ScalarView3DBoundingBox", scalarView3DBoundingBox);
    writeConfig("View/ScalarView3DSolidGeometry", scalarView3DSolidGeometry);

    // deformations
    writeConfig("View/DeformScalar", deformScalar);
    writeConfig("View/DeformContour", deformContour);
    writeConfig("View/DeformVector", deformVector);

    // colors
    writeConfig("View/ColorBackground", colorBackground);
    writeConfig("View/ColorGrid", colorGrid);
    writeConfig("View/ColorCross", colorCross);
    writeConfig("View/ColorNodes", colorNodes);
    writeConfig("View/ColorEdges", colorEdges);
    writeConfig("View/ColorLabels", colorLabels);
    writeConfig("View/ColorContours", colorContours);
    writeConfig("View/ColorVectors", colorVectors);
    writeConfig("View/ColorInitialMesh", colorInitialMesh);
    writeConfig("View/ColorSolutionMesh", colorSolutionMesh);
    writeConfig("View/ColorInitialMesh", colorHighlighted);
    writeConfig("View/ColorSolutionMesh", colorSelected);

    // active field
    writeConfig("View/ActiveField", activeField);

    // mesh
    writeConfig("View/ShowInitialMeshView", showInitialMeshView);
    writeConfig("View/ShowSolutionMeshView", showSolutionMeshView);

    writeConfig("View/ShowPost3D", showPost3D);

    // contour
    writeConfig("View/ContourVariable", contourVariable);
    writeConfig("View/ShowContourView", showContourView);
    writeConfig("View/ContoursCount", contoursCount);
    writeConfig("View/ContoursWidth", contourWidth);

    // scalar view
    writeConfig("View/ShowScalarView", showScalarView);
    writeConfig("View/ShowScalarColorBar", showScalarColorBar);
    writeConfig("View/ScalarVariable", scalarVariable);
    writeConfig("View/ScalarVariableComp", scalarVariableComp);
    writeConfig("View/PaletteType", paletteType);
    writeConfig("View/PaletteFilter", paletteFilter);
    writeConfig("View/PaletteSteps", paletteSteps);
    writeConfig("View/ScalarRangeLog", scalarRangeLog);
    writeConfig("View/ScalarRangeBase", scalarRangeBase);
    writeConfig("View/ScalarDecimalPlace", scalarDecimalPlace);
    writeConfig("View/ScalarRangeAuto", scalarRangeAuto);
    writeConfig("View/ScalarRangeMin", scalarRangeMin);
    writeConfig("View/ScalarRangeMax", scalarRangeMax);

    // vector view
    writeConfig("View/ShowVectorView", showVectorView);
    writeConfig("View/VectorVariable", vectorVariable);
    writeConfig("View/VectorProportional", vectorProportional);
    writeConfig("View/VectorColor", vectorColor);
    writeConfig("View/VectorNumber", vectorCount);
    writeConfig("View/VectorScale", vectorScale);
    writeConfig("View/VectorType", vectorType);
    writeConfig("View/VectorCenter", vectorCenter);

    // order view
    writeConfig("View/ShowOrderView", showOrderView);
    writeConfig("View/ShowOrderColorBar", showOrderColorBar);
    writeConfig("View/OrderPaletteOrderType", orderPaletteOrderType);
    writeConfig("View/OrderLabel", orderLabel);

    // particle tracing
    writeConfig("View/ShowParticleView", showParticleView);
    writeConfig("View/ParticleIncludeRelativisticCorrection", particleIncludeRelativisticCorrection);
    writeConfig("View/ParticleMass", particleMass);
    writeConfig("View/ParticleConstant", particleConstant);
    writeConfig("View/ParticleStartX", particleStart.x);
    writeConfig("View/ParticleStartY", particleStart.y);
    writeConfig("View/ParticleStartVelocityX", particleStartVelocity.x);
    writeConfig("View/ParticleStartVelocityY", particleStartVelocity.y);
    writeConfig("View/ParticleNumberOfParticles", particleNumberOfParticles);
    writeConfig("View/ParticleStartingRadius", particleStartingRadius);
    writeConfig("View/ParticleReflectOnDifferentMaterial", particleReflectOnDifferentMaterial);
    writeConfig("View/ParticleReflectOnBoundary", particleReflectOnBoundary);
    writeConfig("View/ParticleCoefficientOfRestitution", particleCoefficientOfRestitution);
    writeConfig("View/ParticleMaximumRelativeError", particleMaximumRelativeError);
    writeConfig("View/ParticleShowPoints", particleShowPoints);
    writeConfig("View/ParticleColorByVelocity", particleColorByVelocity);
    writeConfig("View/ParticleMaximumNumberOfSteps", particleMaximumNumberOfSteps);
    writeConfig("View/ParticleMinimumStep", particleMinimumStep);
    writeConfig("View/ParticleDragDensity", particleDragDensity);
    writeConfig("View/ParticleDragCoefficient", particleDragCoefficient);
    writeConfig("View/ParticleDragReferenceArea", particleDragReferenceArea);
    writeConfig("View/ParticleCustomForceX", particleCustomForce.x);
    writeConfig("View/ParticleCustomForceY", particleCustomForce.y);
    writeConfig("View/ParticleCustomForceZ", particleCustomForce.z);

    // mesh
    writeConfig("View/MeshAngleSegmentsCount", angleSegmentsCount);
    writeConfig("View/MeshCurvilinearElements", curvilinearElements);

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
    writeConfig("View/LinearizerQuality", paletteQualityToStringKey(linearizerQuality));

    // solid view
    writeConfig("View/SolidViewHide", solidViewHide);

    // command argument
    writeConfig("Commands/Triangle", commandTriangle);
    writeConfig("Commands/Gmsh", commandGmsh);

    eleConfig = NULL;
}

bool ProblemConfigView::readConfig(const QString &key, bool defaultValue)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        if (eleConfig->hasAttribute(att))
            return (eleConfig->attribute(att).toInt() == 1) ? true : false;
    }

    return defaultValue;
}

int ProblemConfigView::readConfig(const QString &key, int defaultValue)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        if (eleConfig->hasAttribute(att))
            return eleConfig->attribute(att).toInt();
    }

    return defaultValue;
}

double ProblemConfigView::readConfig(const QString &key, double defaultValue)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        if (eleConfig->hasAttribute(att))
            return eleConfig->attribute(att).toDouble();
    }

    return defaultValue;
}

QString ProblemConfigView::readConfig(const QString &key, const QString &defaultValue)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        if (eleConfig->hasAttribute(att))
            return eleConfig->attribute(att);
    }

    return defaultValue;
}

QColor ProblemConfigView::readConfig(const QString &key, const QColor &defaultValue)
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

QStringList ProblemConfigView::readConfig(const QString &key, const QStringList &defaultValue)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        if (eleConfig->hasAttribute(att))
            return eleConfig->attribute(att).split("|");
    }

    return defaultValue;
}

void ProblemConfigView::writeConfig(const QString &key, bool value)
{
    QString att = key; att.replace("/", "_");
    eleConfig->setAttribute(att, value);
}

void ProblemConfigView::writeConfig(const QString &key, int value)
{
    QString att = key; att.replace("/", "_");
    eleConfig->setAttribute(att, value);
}

void ProblemConfigView::writeConfig(const QString &key, double value)
{
    QString att = key; att.replace("/", "_");
    eleConfig->setAttribute(att, value);
}

void ProblemConfigView::writeConfig(const QString &key, const QString &value)
{
    QString att = key; att.replace("/", "_");
    eleConfig->setAttribute(att, value);
}

void ProblemConfigView::writeConfig(const QString &key, const QColor &value)
{
    QString att = key; att.replace("/", "_");
    eleConfig->setAttribute(att + "_red", value.red());
    eleConfig->setAttribute(att + "_green", value.green());
    eleConfig->setAttribute(att + "_blue", value.blue());
}

void ProblemConfigView::writeConfig(const QString &key, const QStringList &value)
{
    QString att = key; att.replace("/", "_");
    eleConfig->setAttribute(att, value.join("|"));
}
