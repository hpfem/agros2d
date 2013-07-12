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

#ifndef PROBLEM_CONFIG_H
#define PROBLEM_CONFIG_H

#include "util.h"
#include "value.h"
#include "solutiontypes.h"

class FieldInfo;
class CouplingInfo;

class Field;
class Problem;

namespace XMLProblem
{
    class config;
    class problem_config;
}

class ProblemConfig : public QObject
{
    Q_OBJECT
public:
    enum Type
    {
        Unknown,
        Frequency,
        TimeMethod,
        TimeMethodTolerance,
        TimeOrder,
        TimeConstantTimeSteps,
        TimeTotal
    };

    ProblemConfig(QWidget *parent = 0);

    inline QString labelX() { return ((m_coordinateType == CoordinateType_Planar) ? "X" : "R");  }
    inline QString labelY() { return ((m_coordinateType == CoordinateType_Planar) ? "Y" : "Z");  }
    inline QString labelZ() { return ((m_coordinateType == CoordinateType_Planar) ? "Z" : "a");  }

    void clear();

    inline QString fileName() const { return m_fileName; }
    void setFileName(const QString &fileName) { m_fileName = fileName; }

    // coordinates
    inline CoordinateType coordinateType() const { return m_coordinateType; }
    void setCoordinateType(const CoordinateType coordinateType) { m_coordinateType = coordinateType; emit changed(); }

    // mesh
    inline MeshType meshType() const { return m_meshType; }
    void setMeshType(const MeshType meshType) { m_meshType = meshType; emit changed(); }


    void load(XMLProblem::problem_config *configxsd);
    void save(XMLProblem::problem_config *configxsd);

    inline QString typeToStringKey(Type type) { return m_settingKey[type]; }
    inline Type stringKeyToType(const QString &key) { return m_settingKey.key(key); }

    inline QVariant value(Type type) const { return m_setting[type]; }
    inline void setValue(Type type, int value, bool emitChanged = true) {  m_setting[type] = value; if (emitChanged) emit changed(); }
    inline void setValue(Type type, double value, bool emitChanged = true) {  m_setting[type] = value; emit changed(); if (emitChanged) emit changed(); }
    inline void setValue(Type type, bool value, bool emitChanged = true) {  m_setting[type] = value; emit changed(); if (emitChanged) emit changed(); }
    inline void setValue(Type type, const QString &value, bool emitChanged = true) { m_setting[type] = value; emit changed(); if (emitChanged) emit changed(); }

    inline QVariant defaultValue(Type type) {  return m_settingDefault[type]; }

    inline double constantTimeStepLength() { return value(ProblemConfig::TimeTotal).toDouble() / value(ProblemConfig::TimeConstantTimeSteps).toInt(); }
    double initialTimeStepLength();
    bool isTransientAdaptive() const;

    void refresh() { emit changed(); }

signals:
    void changed();

private:
    QString m_fileName;
    CoordinateType m_coordinateType;

    // mesh type
    MeshType m_meshType;   

    QMap<Type, QVariant> m_setting;
    QMap<Type, QVariant> m_settingDefault;
    QMap<Type, QString> m_settingKey;

    void setDefaultValues();
    void setStringKeys();
};

class ProblemSetting : public QObject
{
    Q_OBJECT

public:
    ProblemSetting();
    ~ProblemSetting();

    enum Type
    {
        Unknown,
        Problem_StartupScript,
        Problem_Description,
        View_RulersFontFamily,
        View_RulersFontPointSize,
        View_PostFontFamily,
        View_PostFontPointSize,
        View_ZoomToMouse,
        View_NodeSize,
        View_EdgeWidth,
        View_LabelSize,
        View_ShowGrid,
        View_GridStep,
        View_ShowRulers,
        View_SnapToGrid,
        View_ShowAxes,
        View_ScalarView3DMode,
        View_ScalarView3DLighting,
        View_ScalarView3DAngle,
        View_ScalarView3DBackground,
        View_ScalarView3DHeight,
        View_ScalarView3DBoundingBox,
        View_ScalarView3DSolidGeometry,
        View_DeformScalar,
        View_DeformContour,
        View_DeformVector,
        View_ColorBackgroundRed,
        View_ColorBackgroundGreen,
        View_ColorBackgroundBlue,
        View_ColorGridRed,
        View_ColorGridGreen,
        View_ColorGridBlue,
        View_ColorCrossRed,
        View_ColorCrossGreen,
        View_ColorCrossBlue,
        View_ColorNodesRed,
        View_ColorNodesGreen,
        View_ColorNodesBlue,
        View_ColorEdgesRed,
        View_ColorEdgesGreen,
        View_ColorEdgesBlue,
        View_ColorLabelsRed,
        View_ColorLabelsGreen,
        View_ColorLabelsBlue,
        View_ColorContoursRed,
        View_ColorContoursGreen,
        View_ColorContoursBlue,
        View_ColorVectorsRed,
        View_ColorVectorsGreen,
        View_ColorVectorsBlue,
        View_ColorInitialMeshRed,
        View_ColorInitialMeshGreen,
        View_ColorInitialMeshBlue,
        View_ColorSolutionMeshRed,
        View_ColorSolutionMeshGreen,
        View_ColorSolutionMeshBlue,
        View_ColorSelectedRed,
        View_ColorSelectedGreen,
        View_ColorSelectedBlue,
        View_ColorHighlightedRed,
        View_ColorHighlightedGreen,
        View_ColorHighlightedBlue,
        View_ColorCrossedRed,
        View_ColorCrossedGreen,
        View_ColorCrossedBlue,
        View_ShowInitialMeshView,
        View_ShowSolutionMeshView,
        View_ContourVariable,
        View_ShowContourView,
        View_ContoursCount,
        View_ContoursWidth,
        View_ShowScalarView,
        View_ShowScalarColorBar,
        View_ScalarVariable,
        View_ScalarVariableComp,
        View_PaletteType,
        View_PaletteFilter,
        View_PaletteSteps,
        View_ScalarRangeLog,
        View_ScalarRangeBase,
        View_ScalarDecimalPlace,
        View_ScalarRangeAuto,
        View_ScalarRangeMin,
        View_ScalarRangeMax,
        View_ShowVectorView,
        View_VectorVariable,
        View_VectorProportional,
        View_VectorColor,
        View_VectorCount,
        View_VectorScale,
        View_VectorType,
        View_VectorCenter,
        View_ShowOrderView,
        View_ShowOrderColorBar,
        View_ShowOrderLabel,
        View_OrderPaletteOrderType,
        View_ParticleButcherTableType,
        View_ParticleIncludeRelativisticCorrection,
        View_ParticleMass,
        View_ParticleConstant,
        View_ParticleStartX,
        View_ParticleStartY,
        View_ParticleStartVelocityX,
        View_ParticleStartVelocityY,
        View_ParticleNumberOfParticles,
        View_ParticleStartingRadius,
        View_ParticleReflectOnDifferentMaterial,
        View_ParticleReflectOnBoundary,
        View_ParticleCoefficientOfRestitution,
        View_ParticleMaximumRelativeError,
        View_ParticleShowPoints,
        View_ParticleShowBlendedFaces,
        View_ParticleNumShowParticlesAxi,
        View_ParticleColorByVelocity,
        View_ParticleMaximumNumberOfSteps,
        View_ParticleMinimumStep,
        View_ParticleDragDensity,
        View_ParticleDragCoefficient,
        View_ParticleDragReferenceArea,
        View_ParticleCustomForceX,
        View_ParticleCustomForceY,
        View_ParticleCustomForceZ,
        View_ChartStartX,
        View_ChartStartY,
        View_ChartEndX,
        View_ChartEndY,
        View_ChartTimeX,
        View_ChartTimeY,
        View_ChartHorizontalAxis,
        View_ChartHorizontalAxisReverse,
        View_ChartHorizontalAxisPoints,
        View_MeshAngleSegmentsCount,
        View_MeshCurvilinearElements,
        View_LinearizerQuality,
        View_SolidViewHide,
        Commands_Triangle,
        Commands_Gmsh
    };

    void load21(QDomElement *config);
    void save21(QDomElement *config);
    void load(XMLProblem::config *configxsd);
    void save(XMLProblem::config *configxsd);

    void clear();

    inline QString typeToStringKey(Type type) { return m_settingKey[type]; }
    inline Type stringKeyToType(const QString &key) { return m_settingKey.key(key); }

    inline QVariant value(Type type) {  return m_setting[type]; }
    inline void setValue(Type type, int value) {  m_setting[type] = value; }
    inline void setValue(Type type, double value) {  m_setting[type] = value; }
    inline void setValue(Type type, bool value) {  m_setting[type] = value; }
    inline void setValue(Type type, const QString &value) { m_setting[type] = value; }
    inline void setValue(Type type, const QStringList &value) { m_setting[type] = value; }

    inline QVariant defaultValue(Type type) {  return m_settingDefault[type]; }

private:
    QMap<Type, QVariant> m_setting;
    QMap<Type, QVariant> m_settingDefault;
    QMap<Type, QString> m_settingKey;

    void setDefaultValues();
    void setStringKeys();
};

#endif // PROBLEM_CONFIG_H
