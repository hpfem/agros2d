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
    WeakForm_MatVol,
    WeakForm_MatSurf,
    WeakForm_VecVol,
    WeakForm_VecSurf,
    WeakForm_ExactSol
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
    SceneViewPost3DMode_None = -1,
    SceneViewPost3DMode_ScalarView3D = 0,
    SceneViewPost3DMode_ScalarView3DSolid = 1,
    SceneViewPost3DMode_Model = 2
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
    DataTableType_Undefined = -1,
    DataTableType_CubicSpline = 0,
    DataTableType_PiecewiseLinear = 1,
    DataTableType_Constant = 2
};

AGROS_LIBRARY_API QString stringListToString(const QStringList &list);

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

// butcher table type
AGROS_LIBRARY_API QString butcherTableTypeString(Hermes::ButcherTableType tableType);
AGROS_LIBRARY_API QStringList butcherTableTypeStringKeys();
AGROS_LIBRARY_API QString butcherTableTypeToStringKey(Hermes::ButcherTableType tableType);
AGROS_LIBRARY_API Hermes::ButcherTableType butcherTableTypeFromStringKey(const QString &tableType);

// nonlinear measurement method
AGROS_LIBRARY_API QString nonlinearSolverConvergenceMeasurementString(Hermes::Hermes2D::NewtonSolverConvergenceMeasurementType measurement);
AGROS_LIBRARY_API QStringList nonlinearSolverConvergenceMeasurementStringKeys();
AGROS_LIBRARY_API QString nonlinearSolverConvergenceMeasurementToStringKey(Hermes::Hermes2D::NewtonSolverConvergenceMeasurementType measurement);
AGROS_LIBRARY_API Hermes::Hermes2D::NewtonSolverConvergenceMeasurementType nonlinearSolverConvergenceMeasurementFromStringKey(const QString &measurement);

// iterative solver - method
AGROS_LIBRARY_API QString iterLinearSolverMethodString(Hermes::Solvers::IterativeParalutionLinearMatrixSolver<double>::ParalutionSolverType type);
AGROS_LIBRARY_API QStringList iterLinearSolverMethodStringKeys();
AGROS_LIBRARY_API QString iterLinearSolverMethodToStringKey(Hermes::Solvers::IterativeParalutionLinearMatrixSolver<double>::ParalutionSolverType type);
AGROS_LIBRARY_API Hermes::Solvers::IterativeParalutionLinearMatrixSolver<double>::ParalutionSolverType iterLinearSolverMethodFromStringKey(const QString &type);

// iterative solver - preconditioner
AGROS_LIBRARY_API QString iterLinearSolverPreconditionerTypeString(Hermes::Solvers::ParalutionPrecond<double>::ParalutionPreconditionerType type);
AGROS_LIBRARY_API QStringList iterLinearSolverPreconditionerTypeStringKeys();
AGROS_LIBRARY_API QString iterLinearSolverPreconditionerTypeToStringKey(Hermes::Solvers::ParalutionPrecond<double>::ParalutionPreconditionerType type);
AGROS_LIBRARY_API Hermes::Solvers::ParalutionPrecond<double>::ParalutionPreconditionerType iterLinearSolverPreconditionerTypeFromStringKey(const QString &type);

#endif // UTIL_ENUMS_H
