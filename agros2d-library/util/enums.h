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
    CoordinateType_Undefined = -1,
    CoordinateType_Planar = 0,
    CoordinateType_Axisymmetric = 1
};

enum AnalysisType
{
    AnalysisType_Undefined = -1,
    AnalysisType_SteadyState = 1,
    AnalysisType_Transient = 2,
    AnalysisType_Harmonic = 3
};

enum AdaptivityType
{
    AdaptivityType_Undefined = 1000,
    AdaptivityType_None = 3,
    AdaptivityType_H = 1,
    AdaptivityType_P = 2,
    AdaptivityType_HP = 0
};

enum AdaptivityStoppingCriterionType
{
    AdaptivityStoppingCriterionType_Undefined = -1,
    AdaptivityStoppingCriterionType_Cumulative = 0,
    AdaptivityStoppingCriterionType_SingleElement = 1,
    AdaptivityStoppingCriterionType_Levels = 2
};

enum TimeStepMethod
{
    TimeStepMethod_Undefined = -1,
    TimeStepMethod_Fixed = 0,
    TimeStepMethod_BDFTolerance = 1,
    TimeStepMethod_BDFNumSteps = 2
};

enum LinearityType
{
    LinearityType_Undefined = -1,
    LinearityType_Linear = 0,
    LinearityType_Picard = 1,
    LinearityType_Newton = 2
};

enum DampingType
{
    DampingType_Undefined = -1,
    DampingType_Automatic = 0,
    DampingType_Fixed = 1,
    DampingType_Off = 2
};

enum CouplingType
{
    CouplingType_Undefined = -1,
    CouplingType_None = 0,
    CouplingType_Weak = 1,
    CouplingType_Hard = 2
};

enum MeshType
{
    MeshType_Undefined = -1,
    MeshType_Triangle = 0,
    MeshType_Triangle_QuadFineDivision = 1,
    MeshType_Triangle_QuadRoughDivision = 2,
    MeshType_Triangle_QuadJoin = 3,
    MeshType_GMSH_Triangle = 4,
    MeshType_GMSH_Quad = 5,
    MeshType_GMSH_QuadDelaunay_Experimental = 6
};

enum PhysicFieldVariableComp
{
    PhysicFieldVariableComp_Undefined = -1,
    PhysicFieldVariableComp_Scalar = 0,
    PhysicFieldVariableComp_Magnitude = 1,
    PhysicFieldVariableComp_X = 2,
    PhysicFieldVariableComp_Y = 3
};

enum WeakFormKind
{
    WeakForm_MatVol = 0,
    WeakForm_MatSurf = 1,
    WeakForm_VecVol = 2,
    WeakForm_VecSurf = 3,
    WeakForm_ExactSol = 4
};

enum WeakFormVariant
{
    WeakFormVariant_Normal = 0,
    WeakFormVariant_Residual = 1,
    WeakFormVariant_TimeResidual = 2
};

enum SceneGeometryMode
{
    SceneGeometryMode_OperateOnNodes = 0,
    SceneGeometryMode_OperateOnEdges = 1,
    SceneGeometryMode_OperateOnLabels = 2
};

enum MouseSceneMode
{
    MouseSceneMode_Nothing = 0,
    MouseSceneMode_Pan = 1,
    MouseSceneMode_Rotate = 2,
    MouseSceneMode_Move = 3,
    MouseSceneMode_Add = 4
};

enum SceneModePostprocessor
{
    SceneModePostprocessor_Empty = 0,
    SceneModePostprocessor_LocalValue = 1,
    SceneModePostprocessor_SurfaceIntegral = 2,
    SceneModePostprocessor_VolumeIntegral = 3
};

enum PaletteType
{
    Palette_Agros2D = 0,
    Palette_Jet = 1,
    Palette_Copper = 2,
    Palette_Hot = 3,
    Palette_Cool = 4,
    Palette_Bone = 5,
    Palette_Pink = 6,
    Palette_Spring = 7,
    Palette_Summer = 8,
    Palette_Autumn = 9,
    Palette_Winter = 10,
    Palette_HSV = 11,
    Palette_BWAsc = 12,
    Palette_BWDesc = 13
};

enum PaletteQuality
{
    PaletteQuality_ExtremelyCoarse = 0,
    PaletteQuality_ExtraCoarse = 1,
    PaletteQuality_Coarser = 2,
    PaletteQuality_Coarse = 3,
    PaletteQuality_Normal = 4,
    PaletteQuality_Fine = 5,
    PaletteQuality_Finer = 6,
    PaletteQuality_ExtraFine = 7
};

enum PaletteOrderType
{
    PaletteOrder_Hermes = 0,
    PaletteOrder_Jet = 1,
    PaletteOrder_Copper = 2,
    PaletteOrder_Hot = 3,
    PaletteOrder_Cool = 4,
    PaletteOrder_Bone = 5,
    PaletteOrder_Pink = 6,
    PaletteOrder_Spring = 7,
    PaletteOrder_Summer = 8,
    PaletteOrder_Autumn = 9,
    PaletteOrder_Winter = 10,
    PaletteOrder_HSV = 11,
    PaletteOrder_BWAsc = 12,
    PaletteOrder_BWDesc = 13
};


enum ChartAxisType
{
    ChartAxis_X = 0,
    ChartAxis_Y = 1,
    ChartAxis_Length = 2
};

enum SceneViewPost3DMode
{
    SceneViewPost3DMode_None = -1,
    SceneViewPost3DMode_ScalarView3D = 0,
    SceneViewPost3DMode_ScalarView3DSolid = 1,
    SceneViewPost3DMode_Model = 2
};

enum SceneTransformMode
{
    SceneTransformMode_Translate = 0,
    SceneTransformMode_Rotate = 1,
    SceneTransformMode_Scale = 2
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
    VectorType_Arrow = 0,
    VectorType_Cone = 1
};

enum VectorCenter
{
    VectorCenter_Tail = 0,
    VectorCenter_Head = 1,
    VectorCenter_Center = 2
};

enum DataTableType
{
    DataTableType_Undefined = -1,
    DataTableType_CubicSpline = 0,
    DataTableType_PiecewiseLinear = 1,
    DataTableType_Constant = 2
};

enum SpecialFunctionType
{
    SpecialFunctionType_Constant = 0,
    SpecialFunctionType_Function1D = 1
};

// keys
AGROS_LIBRARY_API void initLists();

// error norm
AGROS_LIBRARY_API QString errorNormString(Hermes::Hermes2D::NormType projNormType);

// coordinate type
AGROS_LIBRARY_API QString coordinateTypeString(CoordinateType coordinateType);
AGROS_LIBRARY_API QStringList coordinateTypeStringKeys();
AGROS_LIBRARY_API QString coordinateTypeToStringKey(CoordinateType coordinateType);
AGROS_LIBRARY_API CoordinateType coordinateTypeFromStringKey(const QString &coordinateType);

// analysis type
AGROS_LIBRARY_API QString analysisTypeString(AnalysisType analysisType);
AGROS_LIBRARY_API QStringList analysisTypeStringKeys();
AGROS_LIBRARY_API QString analysisTypeToStringKey(AnalysisType analysisType);
AGROS_LIBRARY_API AnalysisType analysisTypeFromStringKey(const QString &analysisType);

// coupling type
AGROS_LIBRARY_API QString couplingTypeString(CouplingType couplingType);
AGROS_LIBRARY_API QStringList couplingTypeStringKeys();
AGROS_LIBRARY_API QString couplingTypeToStringKey(CouplingType couplingType);
AGROS_LIBRARY_API CouplingType couplingTypeFromStringKey(const QString &couplingType);

// weakform type
AGROS_LIBRARY_API QString weakFormString(WeakFormKind weakForm);
AGROS_LIBRARY_API QStringList weakFormStringKeys();
AGROS_LIBRARY_API QString weakFormToStringKey(WeakFormKind weakForm);
AGROS_LIBRARY_API WeakFormKind weakFormFromStringKey(const QString &weakForm);

// weakform variant
AGROS_LIBRARY_API QString weakFormVariantString(WeakFormVariant weakFormVariant);
AGROS_LIBRARY_API QStringList weakFormVariantStringKeys();
AGROS_LIBRARY_API QString weakFormVariantToStringKey(WeakFormVariant weakFormVariant);
AGROS_LIBRARY_API WeakFormVariant weakFormVariantFromStringKey(const QString &weakFormVariant);

// mesh type
AGROS_LIBRARY_API QString meshTypeString(MeshType meshType);
AGROS_LIBRARY_API QStringList meshTypeStringKeys();
AGROS_LIBRARY_API QString meshTypeToStringKey(MeshType meshType);
AGROS_LIBRARY_API MeshType meshTypeFromStringKey(const QString &meshType);

// physic field variable component
AGROS_LIBRARY_API QString physicFieldVariableCompString(PhysicFieldVariableComp physicFieldVariableComp);
AGROS_LIBRARY_API QStringList physicFieldVariableCompTypeStringKeys();
AGROS_LIBRARY_API QString physicFieldVariableCompToStringKey(PhysicFieldVariableComp physicFieldVariableComp);
AGROS_LIBRARY_API PhysicFieldVariableComp physicFieldVariableCompFromStringKey(const QString &physicFieldVariableComp);

// adaptivity type
AGROS_LIBRARY_API QString adaptivityTypeString(AdaptivityType adaptivityType);
AGROS_LIBRARY_API QStringList adaptivityTypeStringKeys();
AGROS_LIBRARY_API QString adaptivityTypeToStringKey(AdaptivityType adaptivityType);
AGROS_LIBRARY_API AdaptivityType adaptivityTypeFromStringKey(const QString &adaptivityType);

// adaptivity stopping criterion type
AGROS_LIBRARY_API QString adaptivityStoppingCriterionTypeString(AdaptivityStoppingCriterionType adaptivityStoppingCriterionType);
AGROS_LIBRARY_API QStringList adaptivityStoppingCriterionTypeStringKeys();
AGROS_LIBRARY_API QString adaptivityStoppingCriterionTypeToStringKey(AdaptivityStoppingCriterionType adaptivityStoppingCriterionType);
AGROS_LIBRARY_API AdaptivityStoppingCriterionType adaptivityStoppingCriterionFromStringKey(const QString &adaptivityStoppingCriterionType);

// adaptivity norm type
AGROS_LIBRARY_API QString errorNormString(Hermes::Hermes2D::NormType projNormType);
AGROS_LIBRARY_API QStringList adaptivityNormTypeStringKeys();
AGROS_LIBRARY_API QString adaptivityNormTypeToStringKey(Hermes::Hermes2D::NormType adaptivityNormType);
AGROS_LIBRARY_API Hermes::Hermes2D::NormType adaptivityNormTypeFromStringKey(const QString &adaptivityNormType);

// time step method
AGROS_LIBRARY_API QString timeStepMethodString(TimeStepMethod timeStepMethod);
AGROS_LIBRARY_API QStringList timeStepMethodStringKeys();
AGROS_LIBRARY_API QString timeStepMethodToStringKey(TimeStepMethod timeStepMethod);
AGROS_LIBRARY_API TimeStepMethod timeStepMethodFromStringKey(const QString &timeStepMethod);

// solution mode
AGROS_LIBRARY_API QString solutionTypeString(SolutionMode solutionMode);
AGROS_LIBRARY_API QStringList solutionTypeStringKeys();
AGROS_LIBRARY_API QString solutionTypeToStringKey(SolutionMode solutionType);
AGROS_LIBRARY_API SolutionMode solutionTypeFromStringKey(const QString &solutionType);

// matrix solver type
AGROS_LIBRARY_API bool isMatrixSolverIterative(Hermes::MatrixSolverType type);
AGROS_LIBRARY_API QString matrixSolverTypeString(Hermes::MatrixSolverType matrixSolverType);
AGROS_LIBRARY_API QStringList matrixSolverTypeStringKeys();
AGROS_LIBRARY_API QString matrixSolverTypeToStringKey(Hermes::MatrixSolverType matrixSolverType);
AGROS_LIBRARY_API Hermes::MatrixSolverType matrixSolverTypeFromStringKey(const QString &matrixSolverType);

// matrix dump format
AGROS_LIBRARY_API QString dumpFormatString(Hermes::Algebra::MatrixExportFormat format);
AGROS_LIBRARY_API QStringList dumpFormatStringKeys();
AGROS_LIBRARY_API QString dumpFormatToStringKey(Hermes::Algebra::MatrixExportFormat format);
AGROS_LIBRARY_API Hermes::Algebra::MatrixExportFormat dumpFormatFromStringKey(const QString &format);

// space type
AGROS_LIBRARY_API QStringList spaceTypeStringKeys();
AGROS_LIBRARY_API QString spaceTypeToStringKey(Hermes::Hermes2D::SpaceType spaceType);
AGROS_LIBRARY_API Hermes::Hermes2D::SpaceType spaceTypeFromStringKey(const QString &spaceType);

// linearity type
AGROS_LIBRARY_API QString linearityTypeString(LinearityType linearityType);
AGROS_LIBRARY_API QStringList linearityTypeStringKeys();
AGROS_LIBRARY_API QString linearityTypeToStringKey(LinearityType linearityType);
AGROS_LIBRARY_API LinearityType linearityTypeFromStringKey(const QString &linearityType);

// damping type
AGROS_LIBRARY_API QString dampingTypeString(DampingType dampingType);
AGROS_LIBRARY_API QStringList dampingTypeStringKeys();
AGROS_LIBRARY_API QString dampingTypeToStringKey(DampingType dampingType);
AGROS_LIBRARY_API DampingType dampingTypeFromStringKey(const QString &dampingType);

// scene view 3d mode
AGROS_LIBRARY_API QStringList sceneViewPost3DModeStringKeys();
AGROS_LIBRARY_API QString sceneViewPost3DModeToStringKey(SceneViewPost3DMode sceneViewPost3DMode);
AGROS_LIBRARY_API SceneViewPost3DMode sceneViewPost3DModeFromStringKey(const QString &sceneViewPost3DMode);

// palette type
AGROS_LIBRARY_API QStringList paletteTypeStringKeys();
AGROS_LIBRARY_API QString paletteTypeToStringKey(PaletteType paletteType);
AGROS_LIBRARY_API PaletteType paletteTypeFromStringKey(const QString &paletteType);
AGROS_LIBRARY_API QString paletteTypeString(PaletteType paletteType);

// palette quality
AGROS_LIBRARY_API QStringList paletteQualityStringKeys();
AGROS_LIBRARY_API QString paletteQualityToStringKey(PaletteQuality paletteQuality);
AGROS_LIBRARY_API PaletteQuality paletteQualityFromStringKey(const QString &quality);
AGROS_LIBRARY_API double paletteQualityToDouble(PaletteQuality paletteQuality);
AGROS_LIBRARY_API QString paletteQualityString(PaletteQuality paletteQuality);

// palette order
AGROS_LIBRARY_API QStringList paletteOrderTypeStringKeys();
AGROS_LIBRARY_API QString paletteOrderTypeToStringKey(PaletteOrderType paletteType);
AGROS_LIBRARY_API PaletteOrderType paletteOrderTypeFromStringKey(const QString &paletteType);

// vector type
AGROS_LIBRARY_API QString vectorTypeString(VectorType vectorType);
AGROS_LIBRARY_API QStringList vectorTypeStringKeys();
AGROS_LIBRARY_API QString vectorTypeToStringKey(VectorType vectorType);
AGROS_LIBRARY_API VectorType vectorTypeFromStringKey(const QString &vectorType);

// vector center
AGROS_LIBRARY_API QString vectorCenterString(VectorCenter vectorCenter);
AGROS_LIBRARY_API QStringList vectorCenterStringKeys();
AGROS_LIBRARY_API QString vectorCenterToStringKey(VectorCenter vectorCenter);
AGROS_LIBRARY_API VectorCenter vectorCenterFromStringKey(const QString &vectorCenter);

// data table type
AGROS_LIBRARY_API QString dataTableTypeString(DataTableType dataTableType);
AGROS_LIBRARY_API QStringList dataTableTypeStringKeys();
AGROS_LIBRARY_API QString dataTableTypeToStringKey(DataTableType dataTableType);
AGROS_LIBRARY_API DataTableType dataTableTypeFromStringKey(const QString &dataTableType);

// special function type
AGROS_LIBRARY_API QString specialFunctionTypeString(SpecialFunctionType specialFunctionType);
AGROS_LIBRARY_API QStringList specialFunctionTypeStringKeys();
AGROS_LIBRARY_API QString specialFunctionTypeToStringKey(SpecialFunctionType specialFunctionType);
AGROS_LIBRARY_API SpecialFunctionType specialFunctionTypeFromStringKey(const QString &specialFunctionType);

// butcher table type
AGROS_LIBRARY_API QString butcherTableTypeString(Hermes::ButcherTableType tableType);
AGROS_LIBRARY_API QStringList butcherTableTypeStringKeys();
AGROS_LIBRARY_API QString butcherTableTypeToStringKey(Hermes::ButcherTableType tableType);
AGROS_LIBRARY_API Hermes::ButcherTableType butcherTableTypeFromStringKey(const QString &tableType);

// iterative solver - method
AGROS_LIBRARY_API QString iterLinearSolverMethodString(Hermes::Solvers::IterSolverType type);
AGROS_LIBRARY_API QStringList iterLinearSolverMethodStringKeys();
AGROS_LIBRARY_API QString iterLinearSolverMethodToStringKey(Hermes::Solvers::IterSolverType type);
AGROS_LIBRARY_API Hermes::Solvers::IterSolverType iterLinearSolverMethodFromStringKey(const QString &type);

// iterative solver - preconditioner
AGROS_LIBRARY_API QString iterLinearSolverPreconditionerTypeString(Hermes::Solvers::PreconditionerType type);
AGROS_LIBRARY_API QStringList iterLinearSolverPreconditionerTypeStringKeys();
AGROS_LIBRARY_API QString iterLinearSolverPreconditionerTypeToStringKey(Hermes::Solvers::PreconditionerType type);
AGROS_LIBRARY_API Hermes::Solvers::PreconditionerType iterLinearSolverPreconditionerTypeFromStringKey(const QString &type);

#endif // UTIL_ENUMS_H
