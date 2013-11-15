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

#include "../resources_source/classes/problem_a2d_31_xml.h"

ProblemConfig::ProblemConfig(QWidget *parent) : QObject(parent)
{
    setStringKeys();
    setDefaultValues();

    clear();
}

void ProblemConfig::clear()
{
    m_fileName = "";

    // coordinate type
    m_coordinateType = CoordinateType_Planar;

    // mesh type
    m_meshType = MeshType_Triangle;

    setDefaultValues();

    m_setting = m_settingDefault;
}

bool ProblemConfig::isTransientAdaptive() const
{
    if ((((TimeStepMethod) value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_BDFTolerance)
            || (((TimeStepMethod) value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_BDFNumSteps))
        return true;
    else if(((TimeStepMethod) value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_Fixed)
        return false;

    assert(0);
}

double ProblemConfig::initialTimeStepLength()
{
    if (((TimeStepMethod) value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_Fixed)
    {
        return constantTimeStepLength();
    }
    else if (((TimeStepMethod) value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_BDFNumSteps)
    {
        if (value(ProblemConfig::TimeInitialStepSize).toDouble() > 0.0)
            return value(ProblemConfig::TimeInitialStepSize).toDouble();
        else
            return constantTimeStepLength() / 3.0;
    }
    else if (((TimeStepMethod) value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_BDFTolerance)
    {
        if (value(ProblemConfig::TimeInitialStepSize).toDouble() > 0.0)
            return value(ProblemConfig::TimeInitialStepSize).toDouble();
        else
            return value(ProblemConfig::TimeTotal).toDouble() / 500.0;
    }
    else
        assert(0);
}

void ProblemConfig::load(XMLProblem::problem_config *configxsd)
{
    // default
    m_setting = m_settingDefault;

    for (int i = 0; i < configxsd->problem_item().size(); i ++)
    {
        Type key = stringKeyToType(QString::fromStdString(configxsd->problem_item().at(i).problem_key()));

        if (m_settingDefault.keys().contains(key))
        {
            if (m_settingDefault[key].type() == QVariant::Double)
                m_setting[key] = QString::fromStdString(configxsd->problem_item().at(i).problem_value()).toDouble();
            else if (m_settingDefault[key].type() == QVariant::Int)
                m_setting[key] = QString::fromStdString(configxsd->problem_item().at(i).problem_value()).toInt();
            else if (m_settingDefault[key].type() == QVariant::Bool)
                m_setting[key] = (QString::fromStdString(configxsd->problem_item().at(i).problem_value()) == "1");
            else if (m_settingDefault[key].type() == QVariant::String)
                m_setting[key] = QString::fromStdString(configxsd->problem_item().at(i).problem_value());
            else
                qDebug() << "Key not found" << QString::fromStdString(configxsd->problem_item().at(i).problem_key()) << QString::fromStdString(configxsd->problem_item().at(i).problem_value());
        }
    }
}

void ProblemConfig::save(XMLProblem::problem_config *configxsd)
{
    foreach (Type key, m_setting.keys())
    {
        if (m_settingDefault[key].type() == QVariant::StringList)
            configxsd->problem_item().push_back(XMLProblem::problem_item(typeToStringKey(key).toStdString(), m_setting[key].toStringList().join("|").toStdString()));
        else if (m_settingDefault[key].type() == QVariant::Bool)
            configxsd->problem_item().push_back(XMLProblem::problem_item(typeToStringKey(key).toStdString(), QString::number(m_setting[key].toInt()).toStdString()));
        else
            configxsd->problem_item().push_back(XMLProblem::problem_item(typeToStringKey(key).toStdString(), m_setting[key].toString().toStdString()));
    }
}

void ProblemConfig::setStringKeys()
{
    m_settingKey[Frequency] = "Frequency";
    m_settingKey[TimeMethod] = "TimeMethod";
    m_settingKey[TimeMethodTolerance] = "TimeMethodTolerance";
    m_settingKey[TimeInitialStepSize] = "TimeInitialStepSize";
    m_settingKey[TimeOrder] = "TimeOrder";
    m_settingKey[TimeConstantTimeSteps] = "TimeSteps";
    m_settingKey[TimeTotal] = "TimeTotal";
}

void ProblemConfig::setDefaultValues()
{
    m_settingDefault.clear();

    m_settingDefault[Frequency] = 50.0;
    m_settingDefault[TimeMethod] = TimeStepMethod_BDFNumSteps;
    m_settingDefault[TimeMethodTolerance] = 0.05;
    m_settingDefault[TimeInitialStepSize] = 0.0;
    m_settingDefault[TimeOrder] = 2;
    m_settingDefault[TimeConstantTimeSteps] = 10;
    m_settingDefault[TimeTotal] = 10.0;
}

// ********************************************************************************************

ProblemSetting::ProblemSetting()
{
    setStringKeys();
    setDefaultValues();

    clear();
}

ProblemSetting::~ProblemSetting()
{
}

void ProblemSetting::clear()
{
    // set default values and types
    m_setting.clear();
    setDefaultValues();

    m_setting = m_settingDefault;
}

void ProblemSetting::setStringKeys()
{
    m_settingKey[Problem_StartupScript] = "Problem_StartupScript";
    m_settingKey[View_RulersFontFamily] = "View_RulersFontFamily";
    m_settingKey[View_RulersFontPointSize] = "View_RulersFontPointSize";
    m_settingKey[View_PostFontFamily] = "View_PostFontFamily";
    m_settingKey[View_PostFontPointSize] = "View_PostFontPointSize";
    m_settingKey[View_ZoomToMouse] = "View_ZoomToMouse";
    m_settingKey[View_NodeSize] = "View_NodeSize";
    m_settingKey[View_EdgeWidth] = "View_EdgeWidth";
    m_settingKey[View_LabelSize] = "View_LabelSize";
    m_settingKey[View_ShowGrid] = "View_ShowGrid";
    m_settingKey[View_GridStep] = "View_GridStep";
    m_settingKey[View_ShowRulers] = "View_ShowRulers";
    m_settingKey[View_SnapToGrid] = "View_SnapToGrid";
    m_settingKey[View_ShowAxes] = "View_ShowAxes";
    m_settingKey[View_ScalarView3DMode] = "View_ScalarView3DMode";
    m_settingKey[View_ScalarView3DLighting] = "View_ScalarView3DLighting";
    m_settingKey[View_ScalarView3DAngle] = "View_ScalarView3DAngle";
    m_settingKey[View_ScalarView3DBackground] = "View_ScalarView3DBackground";
    m_settingKey[View_ScalarView3DHeight] = "View_ScalarView3DHeight";
    m_settingKey[View_ScalarView3DBoundingBox] = "View_ScalarView3DBoundingBox";
    m_settingKey[View_ScalarView3DSolidGeometry] = "View_ScalarView3DSolidGeometry";
    m_settingKey[View_DeformScalar] = "View_DeformScalar";
    m_settingKey[View_DeformContour] = "View_DeformContour";
    m_settingKey[View_DeformVector] = "View_DeformVector";
    m_settingKey[View_ColorBackgroundRed] = "View_ColorBackgroundRed";
    m_settingKey[View_ColorBackgroundGreen] = "View_ColorBackgroundGreen";
    m_settingKey[View_ColorBackgroundBlue] = "View_ColorBackgroundBlue";
    m_settingKey[View_ColorGridRed] = "View_ColorGridRed";
    m_settingKey[View_ColorGridGreen] = "View_ColorGridGreen";
    m_settingKey[View_ColorGridBlue] = "View_ColorGridBlue";
    m_settingKey[View_ColorCrossRed] = "View_ColorCrossRed";
    m_settingKey[View_ColorCrossGreen] = "View_ColorCrossGreen";
    m_settingKey[View_ColorCrossBlue] = "View_ColorCrossBlue";
    m_settingKey[View_ColorNodesRed] = "View_ColorNodesRed";
    m_settingKey[View_ColorNodesGreen] = "View_ColorNodesGreen";
    m_settingKey[View_ColorNodesBlue] = "View_ColorNodesBlue";
    m_settingKey[View_ColorEdgesRed] = "View_ColorEdgesRed";
    m_settingKey[View_ColorEdgesGreen] = "View_ColorEdgesGreen";
    m_settingKey[View_ColorEdgesBlue] = "View_ColorEdgesBlue";
    m_settingKey[View_ColorLabelsRed] = "View_ColorLabelsRed";
    m_settingKey[View_ColorLabelsGreen] = "View_ColorLabelsGreen";
    m_settingKey[View_ColorLabelsBlue] = "View_ColorLabelsBlue";
    m_settingKey[View_ColorContoursRed] = "View_ColorContoursRed";
    m_settingKey[View_ColorContoursGreen] = "View_ColorContoursGreen";
    m_settingKey[View_ColorContoursBlue] = "View_ColorContoursBlue";
    m_settingKey[View_ColorVectorsRed] = "View_ColorVectorsRed";
    m_settingKey[View_ColorVectorsGreen] = "View_ColorVectorsGreen";
    m_settingKey[View_ColorVectorsBlue] = "View_ColorVectorsBlue";
    m_settingKey[View_ColorInitialMeshRed] = "View_ColorInitialMeshRed";
    m_settingKey[View_ColorInitialMeshGreen] = "View_ColorInitialMeshGreen";
    m_settingKey[View_ColorInitialMeshBlue] = "View_ColorInitialMeshBlue";
    m_settingKey[View_ColorSolutionMeshRed] = "View_ColorSolutionMeshRed";
    m_settingKey[View_ColorSolutionMeshGreen] = "View_ColorSolutionMeshGreen";
    m_settingKey[View_ColorSolutionMeshBlue] = "View_ColorSolutionMeshBlue";
    m_settingKey[View_ColorHighlightedRed] = "View_ColorHighlightedRed";
    m_settingKey[View_ColorHighlightedGreen] = "View_ColorHighlightedGreen";
    m_settingKey[View_ColorHighlightedBlue] = "View_ColorHighlightedBlue";
    m_settingKey[View_ColorSelectedRed] = "View_ColorSelectedRed";
    m_settingKey[View_ColorSelectedGreen] = "View_ColorSelectedGreen";
    m_settingKey[View_ColorSelectedBlue] = "View_ColorSelectedBlue";
    m_settingKey[View_ColorCrossedRed] = "View_ColorCrossedRed";
    m_settingKey[View_ColorCrossedGreen] = "View_ColorCrossedGreen";
    m_settingKey[View_ColorCrossedBlue] = "View_ColorCrossedBlue";
    m_settingKey[View_ShowInitialMeshView] = "View_ShowInitialMeshView";
    m_settingKey[View_ShowSolutionMeshView] = "View_ShowSolutionMeshView";
    m_settingKey[View_ContourVariable] = "View_ContourVariable";
    m_settingKey[View_ShowContourView] = "View_ShowContourView";
    m_settingKey[View_ContoursCount] = "View_ContoursCount";
    m_settingKey[View_ContoursWidth] = "View_ContoursWidth";
    m_settingKey[View_ShowScalarView] = "View_ShowScalarView";
    m_settingKey[View_ShowScalarColorBar] = "View_ShowScalarColorBar";
    m_settingKey[View_ScalarVariable] = "View_ScalarVariable";
    m_settingKey[View_ScalarVariableComp] = "View_ScalarVariableComp";
    m_settingKey[View_PaletteType] = "View_PaletteType";
    m_settingKey[View_PaletteFilter] = "View_PaletteFilter";
    m_settingKey[View_PaletteSteps] = "View_PaletteSteps";
    m_settingKey[View_ScalarRangeLog] = "View_ScalarRangeLog";
    m_settingKey[View_ScalarRangeBase] = "View_ScalarRangeBase";
    m_settingKey[View_ScalarDecimalPlace] = "View_ScalarDecimalPlace";
    m_settingKey[View_ScalarRangeAuto] = "View_ScalarRangeAuto";
    m_settingKey[View_ScalarRangeMin] = "View_ScalarRangeMin";
    m_settingKey[View_ScalarRangeMax] = "View_ScalarRangeMax";
    m_settingKey[View_ShowVectorView] = "View_ShowVectorView";
    m_settingKey[View_VectorVariable] = "View_VectorVariable";
    m_settingKey[View_VectorProportional] = "View_VectorProportional";
    m_settingKey[View_VectorColor] = "View_VectorColor";
    m_settingKey[View_VectorCount] = "View_VectorCount";
    m_settingKey[View_VectorScale] = "View_VectorScale";
    m_settingKey[View_VectorType] = "View_VectorType";
    m_settingKey[View_VectorCenter] = "View_VectorCenter";
    m_settingKey[View_ShowOrderView] = "View_ShowOrderView";
    m_settingKey[View_OrderComponent] = "View_OrderComponent";
    m_settingKey[View_ShowOrderLabel] = "View_ShowOrderLabel";
    m_settingKey[View_ShowOrderColorBar] = "View_ShowOrderColorBar";
    m_settingKey[View_OrderPaletteOrderType] = "View_OrderPaletteOrderType";
    m_settingKey[View_ParticleButcherTableType] = "View_ParticleButcherTableType";
    m_settingKey[View_ParticleIncludeRelativisticCorrection] = "View_ParticleIncludeRelativisticCorrection";
    m_settingKey[View_ParticleMass] = "View_ParticleMass";
    m_settingKey[View_ParticleConstant] = "View_ParticleConstant";
    m_settingKey[View_ParticleStartX] = "View_ParticleStartX";
    m_settingKey[View_ParticleStartY] = "View_ParticleStartY";
    m_settingKey[View_ParticleStartVelocityX] = "View_ParticleStartVelocityX";
    m_settingKey[View_ParticleStartVelocityY] = "View_ParticleStartVelocityY";
    m_settingKey[View_ParticleNumberOfParticles] = "View_ParticleNumberOfParticles";
    m_settingKey[View_ParticleStartingRadius] = "View_ParticleStartingRadius";
    m_settingKey[View_ParticleReflectOnDifferentMaterial] = "View_ParticleReflectOnDifferentMaterial";
    m_settingKey[View_ParticleReflectOnBoundary] = "View_ParticleReflectOnBoundary";
    m_settingKey[View_ParticleCoefficientOfRestitution] = "View_ParticleCoefficientOfRestitution";
    m_settingKey[View_ParticleMaximumRelativeError] = "View_ParticleMaximumRelativeError";
    m_settingKey[View_ParticleShowPoints] = "View_ParticleShowPoints";
    m_settingKey[View_ParticleShowBlendedFaces] = "View_ParticleShowBlendedFaces";
    m_settingKey[View_ParticleNumShowParticlesAxi] = "View_ParticleNumShowParticlesAxi";
    m_settingKey[View_ParticleColorByVelocity] = "View_ParticleColorByVelocity";
    m_settingKey[View_ParticleMaximumNumberOfSteps] = "View_ParticleMaximumNumberOfSteps";
    m_settingKey[View_ParticleMinimumStep] = "View_ParticleMinimumStep";
    m_settingKey[View_ParticleDragDensity] = "View_ParticleDragDensity";
    m_settingKey[View_ParticleDragCoefficient] = "View_ParticleDragCoefficient";
    m_settingKey[View_ParticleDragReferenceArea] = "View_ParticleDragReferenceArea";
    m_settingKey[View_ParticleCustomForceX] = "View_ParticleCustomForceX";
    m_settingKey[View_ParticleCustomForceY] = "View_ParticleCustomForceY";
    m_settingKey[View_ParticleCustomForceZ] = "View_ParticleCustomForceZ";
    m_settingKey[View_ChartStartX] = "View_ChartStartX";
    m_settingKey[View_ChartStartY] = "View_ChartStartY";
    m_settingKey[View_ChartEndX] = "View_ChartEndX";
    m_settingKey[View_ChartEndY] = "View_ChartEndY";
    m_settingKey[View_ChartTimeX] = "View_ChartTimeX";
    m_settingKey[View_ChartTimeY] = "View_ChartTimeY";
    m_settingKey[View_ChartHorizontalAxis] = "View_ChartHorizontalAxis";
    m_settingKey[View_ChartHorizontalAxisReverse] = "View_ChartHorizontalAxisReverse";
    m_settingKey[View_ChartHorizontalAxisPoints] = "View_ChartHorizontalAxisPoints";
    m_settingKey[View_LinearizerQuality] = "View_LinearizerQuality";
    m_settingKey[View_SolidViewHide] = "View_SolidViewHide";    
}

void ProblemSetting::setDefaultValues()
{
    m_settingDefault.clear();

    m_settingDefault[Problem_StartupScript] = QString();
    m_settingDefault[View_RulersFontFamily] = QString("Droid");
    m_settingDefault[View_RulersFontPointSize] = 12;
    m_settingDefault[View_PostFontFamily] = QString("Droid");
    m_settingDefault[View_PostFontPointSize] = 16;
    m_settingDefault[View_ZoomToMouse] = true;
    m_settingDefault[View_NodeSize] = 6;
    m_settingDefault[View_EdgeWidth] = 2;
    m_settingDefault[View_LabelSize] = 6;
    m_settingDefault[View_ShowGrid] = true;
    m_settingDefault[View_GridStep] = 0.05;
    m_settingDefault[View_ShowRulers] = true;
    m_settingDefault[View_SnapToGrid] = true;
    m_settingDefault[View_ShowAxes] = true;
    m_settingDefault[View_ScalarView3DMode] = SceneViewPost3DMode_None;
    m_settingDefault[View_ScalarView3DLighting] = true;
    m_settingDefault[View_ScalarView3DAngle] = 240.0;
    m_settingDefault[View_ScalarView3DBackground] = true;
    m_settingDefault[View_ScalarView3DHeight] = 4.0;
    m_settingDefault[View_ScalarView3DBoundingBox] = true;
    m_settingDefault[View_ScalarView3DSolidGeometry] = true;
    m_settingDefault[View_DeformScalar] = true;
    m_settingDefault[View_DeformContour] = true;
    m_settingDefault[View_DeformVector] = true;
    m_settingDefault[View_ColorBackgroundRed] = 255;
    m_settingDefault[View_ColorBackgroundGreen] = 255;
    m_settingDefault[View_ColorBackgroundBlue] = 255;
    m_settingDefault[View_ColorGridRed] = 200;
    m_settingDefault[View_ColorGridGreen] = 200;
    m_settingDefault[View_ColorGridBlue] = 200;
    m_settingDefault[View_ColorCrossRed] = 150;
    m_settingDefault[View_ColorCrossGreen] = 150;
    m_settingDefault[View_ColorCrossBlue] = 150;
    m_settingDefault[View_ColorNodesRed] = 150;
    m_settingDefault[View_ColorNodesGreen] = 0;
    m_settingDefault[View_ColorNodesBlue] = 0;
    m_settingDefault[View_ColorEdgesRed] = 0;
    m_settingDefault[View_ColorEdgesGreen] = 0;
    m_settingDefault[View_ColorEdgesBlue] = 150;
    m_settingDefault[View_ColorLabelsRed] = 0;
    m_settingDefault[View_ColorLabelsGreen] = 150;
    m_settingDefault[View_ColorLabelsBlue] = 0;
    m_settingDefault[View_ColorContoursRed] = 0;
    m_settingDefault[View_ColorContoursGreen] = 0;
    m_settingDefault[View_ColorContoursBlue] = 0;
    m_settingDefault[View_ColorVectorsRed] = 0;
    m_settingDefault[View_ColorVectorsGreen] = 0;
    m_settingDefault[View_ColorVectorsBlue] = 0;
    m_settingDefault[View_ColorInitialMeshRed] = 250;
    m_settingDefault[View_ColorInitialMeshGreen] = 202;
    m_settingDefault[View_ColorInitialMeshBlue] = 119;
    m_settingDefault[View_ColorSolutionMeshRed] = 150;
    m_settingDefault[View_ColorSolutionMeshGreen] = 70;
    m_settingDefault[View_ColorSolutionMeshBlue] = 0;
    m_settingDefault[View_ColorSelectedRed] = 150;
    m_settingDefault[View_ColorSelectedGreen] = 0;
    m_settingDefault[View_ColorSelectedBlue] = 0;
    m_settingDefault[View_ColorHighlightedRed] = 250;
    m_settingDefault[View_ColorHighlightedGreen] = 150;
    m_settingDefault[View_ColorHighlightedBlue] = 0;
    m_settingDefault[View_ColorCrossedRed] = 255;
    m_settingDefault[View_ColorCrossedGreen] = 0;
    m_settingDefault[View_ColorCrossedBlue] = 0;
    m_settingDefault[View_ShowInitialMeshView] = true;
    m_settingDefault[View_ShowSolutionMeshView] = false;
    m_settingDefault[View_ContourVariable] = QString();
    m_settingDefault[View_ShowContourView] = false;
    m_settingDefault[View_ContoursCount] = 15;
    m_settingDefault[View_ContoursWidth] = 1.0;
    m_settingDefault[View_ShowScalarView] = true;
    m_settingDefault[View_ShowScalarColorBar] = true;
    m_settingDefault[View_ScalarVariable] = QString();
    m_settingDefault[View_ScalarVariableComp] = PhysicFieldVariableComp_Undefined;
    m_settingDefault[View_PaletteType] = Palette_Agros2D;
    m_settingDefault[View_PaletteFilter] = false;
    m_settingDefault[View_PaletteSteps] = 30;
    m_settingDefault[View_ScalarRangeLog] = false;
    m_settingDefault[View_ScalarRangeBase] = 10;
    m_settingDefault[View_ScalarDecimalPlace] = 4;
    m_settingDefault[View_ScalarRangeAuto] = true;
    m_settingDefault[View_ScalarRangeMin] = 0.0;
    m_settingDefault[View_ScalarRangeMax] = 1.0;
    m_settingDefault[View_ShowVectorView] = false;
    m_settingDefault[View_VectorVariable] = QString();
    m_settingDefault[View_VectorProportional] = true;
    m_settingDefault[View_VectorColor] = true;
    m_settingDefault[View_VectorCount] = 50;
    m_settingDefault[View_VectorScale] = 0.6;
    m_settingDefault[View_VectorType] = VectorType_Arrow;
    m_settingDefault[View_VectorCenter] = VectorCenter_Tail;
    m_settingDefault[View_OrderComponent] = 1;
    m_settingDefault[View_ShowOrderView] = true;
    m_settingDefault[View_ShowOrderLabel] = false;
    m_settingDefault[View_ShowOrderColorBar] = true;
    m_settingDefault[View_OrderPaletteOrderType] = PaletteOrder_Hermes;
    m_settingDefault[View_ParticleButcherTableType] = Hermes::Explicit_FEHLBERG_6_45_embedded;
    m_settingDefault[View_ParticleIncludeRelativisticCorrection] = true;
    m_settingDefault[View_ParticleMass] = 9.109e-31;
    m_settingDefault[View_ParticleConstant] = 1.602e-19;
    m_settingDefault[View_ParticleStartX] = 0.0;
    m_settingDefault[View_ParticleStartY] = 0.0;
    m_settingDefault[View_ParticleStartVelocityX] = 0.0;
    m_settingDefault[View_ParticleStartVelocityY] = 0.0;
    m_settingDefault[View_ParticleNumberOfParticles] = 1;
    m_settingDefault[View_ParticleStartingRadius] = 0.0;
    m_settingDefault[View_ParticleReflectOnDifferentMaterial] = false;
    m_settingDefault[View_ParticleReflectOnBoundary] = false;
    m_settingDefault[View_ParticleCoefficientOfRestitution] = 0.0;
    m_settingDefault[View_ParticleMaximumRelativeError] = 0.01;
    m_settingDefault[View_ParticleShowPoints] = false;
    m_settingDefault[View_ParticleShowBlendedFaces] = true;
    m_settingDefault[View_ParticleNumShowParticlesAxi] = 1;
    m_settingDefault[View_ParticleColorByVelocity] = true;
    m_settingDefault[View_ParticleMaximumNumberOfSteps] = 500;
    m_settingDefault[View_ParticleMinimumStep] = 0.0;
    m_settingDefault[View_ParticleDragDensity] = 1.2041;
    m_settingDefault[View_ParticleDragCoefficient] = 0.0;
    m_settingDefault[View_ParticleDragReferenceArea] = 0.0;
    m_settingDefault[View_ParticleCustomForceX] = 0.0;
    m_settingDefault[View_ParticleCustomForceY] = 0.0;
    m_settingDefault[View_ParticleCustomForceZ] = 0.0;
    m_settingDefault[View_ChartStartX] = 0.0;
    m_settingDefault[View_ChartStartY] = 0.0;
    m_settingDefault[View_ChartEndX] = 0.0;
    m_settingDefault[View_ChartEndY] = 0.0;
    m_settingDefault[View_ChartTimeX] = 0.0;
    m_settingDefault[View_ChartTimeY] = 0.0;
    m_settingDefault[View_ChartHorizontalAxis] = ChartAxis_Length;
    m_settingDefault[View_ChartHorizontalAxisReverse] = false;
    m_settingDefault[View_ChartHorizontalAxisPoints] = 200;        
    m_settingDefault[View_LinearizerQuality] = PaletteQuality_Normal;
    m_settingDefault[View_SolidViewHide] = QStringList();
}

void ProblemSetting::load21(QDomElement *config)
{
    // default
    m_setting = m_settingDefault;

    for (int i = 0; i < config->attributes().count(); i++)
    {
        QDomNode attribute = config->attributes().item(i);

        Type key = stringKeyToType(attribute.toAttr().name());

        if (m_settingDefault.keys().contains(key))
        {
            if (m_settingDefault[key].type() == QVariant::Double)
                m_setting[key] = attribute.toAttr().value().toDouble();
            else if (m_settingDefault[key].type() == QVariant::Int)
                m_setting[key] = attribute.toAttr().value().toInt();
            else if (m_settingDefault[key].type() == QVariant::Bool)
                m_setting[key] = (attribute.toAttr().value() == "1");
            else if (m_settingDefault[key].type() == QVariant::String)
                m_setting[key] = attribute.toAttr().value();
            else if (m_settingDefault[key].type() == QVariant::StringList)
                m_setting[key] = attribute.toAttr().value().split("|");
            else
                qDebug() << "Key not found" << attribute.toAttr().name() << attribute.toAttr().value();
        }
    }
}

void ProblemSetting::save21(QDomElement *config)
{
    // not implemented
    // use save(...)
}

void ProblemSetting::load(XMLProblem::config *configxsd)
{
    // default
    m_setting = m_settingDefault;

    for (int i = 0; i < configxsd->item().size(); i ++)
    {
        Type key = stringKeyToType(QString::fromStdString(configxsd->item().at(i).key()));

        if (m_settingDefault.keys().contains(key))
        {
            if (m_settingDefault[key].type() == QVariant::Double)
                m_setting[key] = QString::fromStdString(configxsd->item().at(i).value()).toDouble();
            else if (m_settingDefault[key].type() == QVariant::Int)
                m_setting[key] = QString::fromStdString(configxsd->item().at(i).value()).toInt();
            else if (m_settingDefault[key].type() == QVariant::Bool)
                m_setting[key] = (QString::fromStdString(configxsd->item().at(i).value()) == "1");
            else if (m_settingDefault[key].type() == QVariant::String)
                m_setting[key] = QString::fromStdString(configxsd->item().at(i).value());
            else if (m_settingDefault[key].type() == QVariant::StringList)
                m_setting[key] = QString::fromStdString(configxsd->item().at(i).value()).split("|");
            else
                qDebug() << "Key not found" << QString::fromStdString(configxsd->item().at(i).key()) << QString::fromStdString(configxsd->item().at(i).value());
        }
    }
}

void ProblemSetting::save(XMLProblem::config *configxsd)
{    
    foreach (Type key, m_setting.keys())
    {
        if (m_settingDefault[key].type() == QVariant::StringList)
            configxsd->item().push_back(XMLProblem::item(typeToStringKey(key).toStdString(), m_setting[key].toStringList().join("|").toStdString()));
        else if (m_settingDefault[key].type() == QVariant::Bool)
            configxsd->item().push_back(XMLProblem::item(typeToStringKey(key).toStdString(), QString::number(m_setting[key].toInt()).toStdString()));
        else
            configxsd->item().push_back(XMLProblem::item(typeToStringKey(key).toStdString(), m_setting[key].toString().toStdString()));
    }
}
