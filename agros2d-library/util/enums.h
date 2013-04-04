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

#ifndef UTIL_ENUMS_H
#define UTIL_ENUMS_H

#include "QtCore"

#include "hermes_common.h"
#include "hermes2d.h"
#include "util.h"

enum CoordinateType
{
    CoordinateType_Undefined,
    CoordinateType_Planar,
    CoordinateType_Axisymmetric
};

enum AnalysisType
{
    AnalysisType_Undefined,
    AnalysisType_SteadyState,
    AnalysisType_Transient,
    AnalysisType_Harmonic
};

enum AdaptivityType
{
    AdaptivityType_Undefined = 1000,
    AdaptivityType_None = 3,
    AdaptivityType_H = 1,
    AdaptivityType_P = 2,
    AdaptivityType_HP = 0
};

enum TimeStepMethod
{
    TimeStepMethod_Fixed,
    TimeStepMethod_BDFTolerance,
    TimeStepMethod_BDFNumSteps
};

enum LinearityType
{
    LinearityType_Undefined,
    LinearityType_Linear,
    LinearityType_Picard,
    LinearityType_Newton
};

enum CouplingType
{
    CouplingType_Undefined,
    CouplingType_None,
    CouplingType_Weak,
    CouplingType_Hard
};

enum WeakFormKind
{
    WeakForm_MatVol,
    WeakForm_MatSurf,
    WeakForm_VecVol,
    WeakForm_VecSurf,
    WeakForm_ExactSol
};

enum MeshType
{
    MeshType_Triangle,
    MeshType_Triangle_QuadFineDivision,
    MeshType_Triangle_QuadRoughDivision,
    MeshType_Triangle_QuadJoin,
    MeshType_GMSH_Triangle,
    MeshType_GMSH_Quad,
    MeshType_GMSH_QuadDelaunay_Experimental
};

enum PhysicFieldVariableComp
{
    PhysicFieldVariableComp_Undefined,
    PhysicFieldVariableComp_Scalar,
    PhysicFieldVariableComp_Magnitude,
    PhysicFieldVariableComp_X,
    PhysicFieldVariableComp_Y
};

enum SceneGeometryMode
{
    SceneGeometryMode_OperateOnNodes,
    SceneGeometryMode_OperateOnEdges,
    SceneGeometryMode_OperateOnLabels
};

enum MouseSceneMode
{
    MouseSceneMode_Nothing,
    MouseSceneMode_Pan,
    MouseSceneMode_Rotate,
    MouseSceneMode_Move,
    MouseSceneMode_Add
};

enum SceneModePostprocessor
{
    SceneModePostprocessor_Empty,
    SceneModePostprocessor_LocalValue,
    SceneModePostprocessor_SurfaceIntegral,
    SceneModePostprocessor_VolumeIntegral
};

enum PaletteType
{
    Palette_Agros2D,
    Palette_Jet,
    Palette_Copper,
    Palette_Hot,
    Palette_Cool,
    Palette_Bone,
    Palette_Pink,
    Palette_Spring,
    Palette_Summer,
    Palette_Autumn,
    Palette_Winter,
    Palette_HSV,
    Palette_BWAsc,
    Palette_BWDesc
};

enum PaletteQuality
{
    PaletteQuality_ExtremelyCoarse,
    PaletteQuality_ExtraCoarse,
    PaletteQuality_Coarser,
    PaletteQuality_Coarse,
    PaletteQuality_Normal,
    PaletteQuality_Fine,
    PaletteQuality_Finer,
    PaletteQuality_ExtraFine
};

enum PaletteOrderType
{
    PaletteOrder_Hermes,
    PaletteOrder_Jet,
    PaletteOrder_Copper,
    PaletteOrder_Hot,
    PaletteOrder_Cool,
    PaletteOrder_Bone,
    PaletteOrder_Pink,
    PaletteOrder_Spring,
    PaletteOrder_Summer,
    PaletteOrder_Autumn,
    PaletteOrder_Winter,
    PaletteOrder_HSV,
    PaletteOrder_BWAsc,
    PaletteOrder_BWDesc
};


enum ChartAxisType
{
    ChartAxis_X,
    ChartAxis_Y,
    ChartAxis_Length
};

enum SceneViewPost3DMode
{
    //SceneViewPost3DMode_Undefined,
    SceneViewPost3DMode_None,
    SceneViewPost3DMode_ScalarView3D,
    SceneViewPost3DMode_ScalarView3DSolid,
    SceneViewPost3DMode_Model
};

enum SceneTransformMode
{
    SceneTransformMode_Translate,
    SceneTransformMode_Rotate,
    SceneTransformMode_Scale
};

enum SolutionMode
{
    SolutionMode_Undefined = -1,
    SolutionMode_Normal = 0,
    SolutionMode_Reference = 1,
    SolutionMode_Finer = 100 // used to choose reference if exists, normal otherwise
};

enum VectorType
{
    VectorType_Arrow,
    VectorType_Cone
};

enum VectorCenter
{
    VectorCenter_Tail,
    VectorCenter_Head,
    VectorCenter_Center
};

enum DataTableType
{
    DataTableType_CubicSpline,
    DataTableType_PiecewiseLinear,
    DataTableType_Constant
};


AGROS_API QString stringListToString(const QStringList &list);

// keys
AGROS_API void initLists();

// error norm
AGROS_API QString errorNormString(Hermes::Hermes2D::ProjNormType projNormType);

// coordinate type
AGROS_API QString coordinateTypeString(CoordinateType coordinateType);
AGROS_API QStringList coordinateTypeStringKeys();
AGROS_API QString coordinateTypeToStringKey(CoordinateType coordinateType);
AGROS_API CoordinateType coordinateTypeFromStringKey(const QString &coordinateType);

// analysis type
AGROS_API QString analysisTypeString(AnalysisType analysisType);
AGROS_API QStringList analysisTypeStringKeys();
AGROS_API QString analysisTypeToStringKey(AnalysisType analysisType);
AGROS_API AnalysisType analysisTypeFromStringKey(const QString &analysisType);

// coupling type
AGROS_API QString couplingTypeString(CouplingType couplingType);
AGROS_API QStringList couplingTypeStringKeys();
AGROS_API QString couplingTypeToStringKey(CouplingType couplingType);
AGROS_API CouplingType couplingTypeFromStringKey(const QString &couplingType);

// weakform type
AGROS_API QString weakFormString(WeakFormKind weakForm);
AGROS_API QStringList weakFormStringKeys();
AGROS_API QString weakFormToStringKey(WeakFormKind weakForm);
AGROS_API WeakFormKind weakFormFromStringKey(const QString &weakForm);

// mesh type
AGROS_API QString meshTypeString(MeshType meshType);
AGROS_API QStringList meshTypeStringKeys();
AGROS_API QString meshTypeToStringKey(MeshType meshType);
AGROS_API MeshType meshTypeFromStringKey(const QString &meshType);

// physic field variable component
AGROS_API QString physicFieldVariableCompString(PhysicFieldVariableComp physicFieldVariableComp);
AGROS_API QStringList physicFieldVariableCompTypeStringKeys();
AGROS_API QString physicFieldVariableCompToStringKey(PhysicFieldVariableComp physicFieldVariableComp);
AGROS_API PhysicFieldVariableComp physicFieldVariableCompFromStringKey(const QString &physicFieldVariableComp);

// adaptivity type
AGROS_API QString adaptivityTypeString(AdaptivityType adaptivityType);
AGROS_API QStringList adaptivityTypeStringKeys();
AGROS_API QString adaptivityTypeToStringKey(AdaptivityType adaptivityType);
AGROS_API AdaptivityType adaptivityTypeFromStringKey(const QString &adaptivityType);

// time step method
AGROS_API QString timeStepMethodString(TimeStepMethod timeStepMethod);
AGROS_API QStringList timeStepMethodStringKeys();
AGROS_API QString timeStepMethodToStringKey(TimeStepMethod timeStepMethod);
AGROS_API TimeStepMethod timeStepMethodFromStringKey(const QString &timeStepMethod);

// solution mode
AGROS_API QString solutionTypeString(SolutionMode solutionMode);
AGROS_API QStringList solutionTypeStringKeys();
AGROS_API QString solutionTypeToStringKey(SolutionMode solutionType);
AGROS_API SolutionMode solutionTypeFromStringKey(const QString &solutionType);

// matrix solver type
AGROS_API QString matrixSolverTypeString(Hermes::MatrixSolverType matrixSolverType);
AGROS_API QStringList matrixSolverTypeStringKeys();
AGROS_API QString matrixSolverTypeToStringKey(Hermes::MatrixSolverType matrixSolverType);
AGROS_API Hermes::MatrixSolverType matrixSolverTypeFromStringKey(const QString &matrixSolverType);

// space type
AGROS_API QStringList spaceTypeStringKeys();
AGROS_API QString spaceTypeToStringKey(Hermes::Hermes2D::SpaceType spaceType);
AGROS_API Hermes::Hermes2D::SpaceType spaceTypeFromStringKey(const QString &spaceType);

// linearity type
AGROS_API QString linearityTypeString(LinearityType linearityType);
AGROS_API QStringList linearityTypeStringKeys();
AGROS_API QString linearityTypeToStringKey(LinearityType linearityType);
AGROS_API LinearityType linearityTypeFromStringKey(const QString &linearityType);

// scene view 3d mode
AGROS_API QStringList sceneViewPost3DModeStringKeys();
AGROS_API QString sceneViewPost3DModeToStringKey(SceneViewPost3DMode sceneViewPost3DMode);
AGROS_API SceneViewPost3DMode sceneViewPost3DModeFromStringKey(const QString &sceneViewPost3DMode);

// palette type
AGROS_API QStringList paletteTypeStringKeys();
AGROS_API QString paletteTypeToStringKey(PaletteType paletteType);
AGROS_API PaletteType paletteTypeFromStringKey(const QString &paletteType);
AGROS_API QString paletteTypeString(PaletteType paletteType);

// palette quality
AGROS_API QStringList paletteQualityStringKeys();
AGROS_API QString paletteQualityToStringKey(PaletteQuality paletteQuality);
AGROS_API PaletteQuality paletteQualityFromStringKey(const QString &quality);
AGROS_API double paletteQualityToDouble(PaletteQuality paletteQuality);
AGROS_API QString paletteQualityString(PaletteQuality paletteQuality);

// palette order
AGROS_API QStringList paletteOrderTypeStringKeys();
AGROS_API QString paletteOrderTypeToStringKey(PaletteOrderType paletteType);
AGROS_API PaletteOrderType paletteOrderTypeFromStringKey(const QString &paletteType);

// vector type
AGROS_API QString vectorTypeString(VectorType vectorType);
AGROS_API QStringList vectorTypeStringKeys();
AGROS_API QString vectorTypeToStringKey(VectorType vectorType);
AGROS_API VectorType vectorTypeFromStringKey(const QString &vectorType);

// vector center
AGROS_API QString vectorCenterString(VectorCenter vectorCenter);
AGROS_API QStringList vectorCenterStringKeys();
AGROS_API QString vectorCenterToStringKey(VectorCenter vectorCenter);
AGROS_API VectorCenter vectorCenterFromStringKey(const QString &vectorCenter);

// data table type
AGROS_API QString dataTableTypeString(DataTableType dataTableType);
AGROS_API QStringList dataTableTypeStringKeys();
AGROS_API QString dataTableTypeToStringKey(DataTableType dataTableType);
AGROS_API DataTableType dataTableTypeFromStringKey(const QString &dataTableType);

#endif // UTIL_ENUMS_H

