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

#ifdef WITH_UNITY
#include <unity.h>
#endif

#include "util.h"
#include "scene.h"
#include "pythonlabagros.h"
#include "style/manhattanstyle.h"

#include "hermes2d/module.h"

# define M_PI_2		1.57079632679489661923	/* pi/2 */

static QHash<CoordinateType, QString> coordinateTypeList;
static QHash<PhysicFieldVariableComp, QString> physicFieldVariableCompList;
static QHash<Mode, QString> modeList;
static QHash<SceneViewPost3DMode, QString> sceneViewPost3DModeList;
static QHash<WeakFormKind, QString> weakFormList;
static QHash<AdaptivityType, QString> adaptivityTypeList;
static QHash<TimeStepMethod, QString> timeStepMethodList;
static QHash<SolutionMode, QString> solutionTypeList;
static QHash<AnalysisType, QString> analysisTypeList;
static QHash<CouplingType, QString> couplingTypeList;
static QHash<LinearityType, QString> linearityTypeList;
static QHash<MeshType, QString> meshTypeList;
static QHash<Hermes::MatrixSolverType, QString> matrixSolverTypeList;
static QHash<PaletteType, QString> paletteTypeList;
static QHash<PaletteQuality, QString> paletteQualityList;
static QHash<PaletteOrderType, QString> paletteOrderTypeList;

QStringList coordinateTypeStringKeys() { return coordinateTypeList.values(); }
QString coordinateTypeToStringKey(CoordinateType coordinateType) { return coordinateTypeList[coordinateType]; }
CoordinateType coordinateTypeFromStringKey(const QString &coordinateType) { return coordinateTypeList.key(coordinateType); }

QStringList analysisTypeStringKeys() { return analysisTypeList.values(); }
QString analysisTypeToStringKey(AnalysisType analysisType) { return analysisTypeList[analysisType]; }
AnalysisType analysisTypeFromStringKey(const QString &analysisType) { return analysisTypeList.key(analysisType); }

QStringList couplingTypeStringKeys() { return couplingTypeList.values(); }
QString couplingTypeToStringKey(CouplingType couplingType) { return couplingTypeList[couplingType]; }
CouplingType couplingTypeFromStringKey(const QString &couplingType) { return couplingTypeList.key(couplingType); }

QStringList weakFormStringKeys() { return weakFormList.values(); }
QString weakFormToStringKey(WeakFormKind weakForm) { return weakFormList[weakForm]; }
WeakFormKind weakFormFromStringKey(const QString &weakForm) { return weakFormList.key(weakForm); }

QStringList meshTypeStringKeys() { return meshTypeList.values(); }
QString meshTypeToStringKey(MeshType meshType) { return meshTypeList[meshType]; }
MeshType meshTypeFromStringKey(const QString &meshType) { return meshTypeList.key(meshType); }

QStringList physicFieldVariableCompTypeStringKeys() { return physicFieldVariableCompList.values(); }
QString physicFieldVariableCompToStringKey(PhysicFieldVariableComp physicFieldVariableComp) { return physicFieldVariableCompList[physicFieldVariableComp]; }
PhysicFieldVariableComp physicFieldVariableCompFromStringKey(const QString &physicFieldVariableComp) { return physicFieldVariableCompList.key(physicFieldVariableComp); }

QStringList adaptivityTypeStringKeys() { return adaptivityTypeList.values(); }
QString adaptivityTypeToStringKey(AdaptivityType adaptivityType) { return adaptivityTypeList[adaptivityType]; }
AdaptivityType adaptivityTypeFromStringKey(const QString &adaptivityType) { return adaptivityTypeList.key(adaptivityType); }

QStringList timeStepMethodStringKeys() { return timeStepMethodList.values(); }
QString timeStepMethodToStringKey(TimeStepMethod timeStepMethod) { return timeStepMethodList[timeStepMethod]; }
TimeStepMethod timeStepMethodFromStringKey(const QString &timeStepMethod) { return timeStepMethodList.key(timeStepMethod); }

QStringList solutionTypeStringKeys() { return solutionTypeList.values(); }
QString solutionTypeToStringKey(SolutionMode solutionType) { return solutionTypeList[solutionType]; }
SolutionMode solutionTypeFromStringKey(const QString &solutionType) { return solutionTypeList.key(solutionType); }

QStringList linearityTypeStringKeys() { return linearityTypeList.values(); }
QString linearityTypeToStringKey(LinearityType linearityType) { return linearityTypeList[linearityType]; }
LinearityType linearityTypeFromStringKey(const QString &linearityType) { return linearityTypeList.key(linearityType); }

QStringList matrixSolverTypeStringKeys() { return matrixSolverTypeList.values(); }
QString matrixSolverTypeToStringKey(Hermes::MatrixSolverType matrixSolverType) { return matrixSolverTypeList[matrixSolverType]; }
Hermes::MatrixSolverType matrixSolverTypeFromStringKey(const QString &matrixSolverType) { return matrixSolverTypeList.key(matrixSolverType); }

QStringList sceneViewPost3DModeStringKeys() { return sceneViewPost3DModeList.values(); }
QString sceneViewPost3DModeToStringKey(SceneViewPost3DMode sceneViewPost3DMode) { return sceneViewPost3DModeList[sceneViewPost3DMode]; }
SceneViewPost3DMode sceneViewPost3DModeFromStringKey(const QString &sceneViewPost3DMode) { return sceneViewPost3DModeList.key(sceneViewPost3DMode); }

QString modeToStringKey(Mode mode) { return modeList[mode]; }
Mode modeFromStringKey(const QString &mode) { return modeList.key(mode); }

QStringList paletteTypeStringKeys() { return paletteTypeList.values(); }
QString paletteTypeToStringKey(PaletteType paletteType) { return paletteTypeList[paletteType]; }
PaletteType paletteTypeFromStringKey(const QString &paletteType) { return paletteTypeList.key(paletteType); }

QStringList paletteQualityStringKeys() { return paletteQualityList.values(); }
QString paletteQualityToStringKey(PaletteQuality paletteQuality) { return paletteQualityList[paletteQuality]; }
PaletteQuality paletteQualityFromStringKey(const QString &paletteQuality) { return paletteQualityList.key(paletteQuality); }

QStringList paletteOrderTypeStringKeys() { return paletteOrderTypeList.values(); }
QString paletteOrderTypeToStringKey(PaletteOrderType paletteType) { return paletteOrderTypeList[paletteType]; }
PaletteOrderType paletteOrderTypeFromStringKey(const QString &paletteType) { return paletteOrderTypeList.key(paletteType); }

void initLists()
{
    // coordinate list
    coordinateTypeList.insert(CoordinateType_Undefined, "");
    coordinateTypeList.insert(CoordinateType_Planar, "planar");
    coordinateTypeList.insert(CoordinateType_Axisymmetric, "axisymmetric");

    // Analysis Type
    analysisTypeList.insert(AnalysisType_Undefined, "");
    analysisTypeList.insert(AnalysisType_SteadyState, "steadystate");
    analysisTypeList.insert(AnalysisType_Transient, "transient");
    analysisTypeList.insert(AnalysisType_Harmonic, "harmonic");

    // coupling type
    couplingTypeList.insert(CouplingType_Undefined, "");
    couplingTypeList.insert(CouplingType_Hard, "hard");
    couplingTypeList.insert(CouplingType_Weak, "weak");
    couplingTypeList.insert(CouplingType_None, "none");

    weakFormList.insert(WeakForm_MatVol, "matvol");
    weakFormList.insert(WeakForm_MatSurf, "matsur");
    weakFormList.insert(WeakForm_VecVol, "vecvol");
    weakFormList.insert(WeakForm_VecSurf, "vecsur");

    // Mesh Type
    meshTypeList.insert(MeshType_Triangle, "triangle");
    meshTypeList.insert(MeshType_Triangle_QuadFineDivision, "triangle_quad_fine_division");
    meshTypeList.insert(MeshType_Triangle_QuadRoughDivision, "triangle_quad_rough_division");
    meshTypeList.insert(MeshType_Triangle_QuadJoin, "triangle_quad_join");
    meshTypeList.insert(MeshType_GMSH_Triangle, "gmsh_triangle");
    meshTypeList.insert(MeshType_GMSH_Quad, "gmsh_quad");
    meshTypeList.insert(MeshType_GMSH_QuadDelaunay_Experimental, "gmsh_quad_delaunay");

    timeStepMethodList.insert(TimeStepMethod_Fixed, "fixed");
    timeStepMethodList.insert(TimeStepMethod_BDF2, "bdf2");

    // PHYSICFIELDVARIABLECOMP
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Undefined, "");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Scalar, "scalar");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Magnitude, "magnitude");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_X, "x");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Y, "y");

    // TEMODE
    modeList.insert(Mode_0, "mode_0");
    modeList.insert(Mode_01, "mode_01");
    modeList.insert(Mode_02, "mode_02");

    // post3d
    //sceneViewPost3DModeList.insert(SceneViewPost3DMode_Undefined, "");
    sceneViewPost3DModeList.insert(SceneViewPost3DMode_None, "none");
    sceneViewPost3DModeList.insert(SceneViewPost3DMode_ScalarView3D, "scalar");
    sceneViewPost3DModeList.insert(SceneViewPost3DMode_ScalarView3DSolid, "scalarsolid");
    sceneViewPost3DModeList.insert(SceneViewPost3DMode_ParticleTracing, "particletracing");
    sceneViewPost3DModeList.insert(SceneViewPost3DMode_Model, "model");

    // ADAPTIVITYTYPE
    adaptivityTypeList.insert(AdaptivityType_Undefined, "");
    adaptivityTypeList.insert(AdaptivityType_None, "disabled");
    adaptivityTypeList.insert(AdaptivityType_H, "h-adaptivity");
    adaptivityTypeList.insert(AdaptivityType_P, "p-adaptivity");
    adaptivityTypeList.insert(AdaptivityType_HP, "hp-adaptivity");

    // SolutionType
    solutionTypeList.insert(SolutionMode_Normal, "normal");
    solutionTypeList.insert(SolutionMode_Reference, "reference");

    // MatrixSolverType
    matrixSolverTypeList.insert(Hermes::SOLVER_UMFPACK, "umfpack");
    matrixSolverTypeList.insert(Hermes::SOLVER_PETSC, "petsc");
    matrixSolverTypeList.insert(Hermes::SOLVER_MUMPS, "mumps");
    matrixSolverTypeList.insert(Hermes::SOLVER_SUPERLU, "superlu");
    matrixSolverTypeList.insert(Hermes::SOLVER_AMESOS, "trilinos_amesos");
    matrixSolverTypeList.insert(Hermes::SOLVER_AZTECOO, "trilinos_aztecoo");

    // LinearityType
    linearityTypeList.insert(LinearityType_Undefined, "");
    linearityTypeList.insert(LinearityType_Linear, "linear");
    linearityTypeList.insert(LinearityType_Picard, "picard");
    linearityTypeList.insert(LinearityType_Newton, "newton");

    // PaletteType
    paletteTypeList.insert(Palette_Agros2D, "agros2d");
    paletteTypeList.insert(Palette_Jet, "jet");
    paletteTypeList.insert(Palette_Copper, "copper");
    paletteTypeList.insert(Palette_Hot, "hot");
    paletteTypeList.insert(Palette_Bone, "bone");
    paletteTypeList.insert(Palette_Pink, "pink");
    paletteTypeList.insert(Palette_Spring, "spring");
    paletteTypeList.insert(Palette_Summer, "summer");
    paletteTypeList.insert(Palette_Autumn, "autumn");
    paletteTypeList.insert(Palette_Winter, "winter");
    paletteTypeList.insert(Palette_HSV, "hsv");
    paletteTypeList.insert(Palette_BWAsc, "bw_ascending");
    paletteTypeList.insert(Palette_BWDesc, "bw_descending");

    // PaletteQuality
    paletteQualityList.insert(PaletteQuality_ExtremelyCoarse, "extremely_coarse");
    paletteQualityList.insert(PaletteQuality_ExtraCoarse, "extra_coarse");
    paletteQualityList.insert(PaletteQuality_Coarser, "coarser");
    paletteQualityList.insert(PaletteQuality_Coarse, "coarse");
    paletteQualityList.insert(PaletteQuality_Normal, "normal");
    paletteQualityList.insert(PaletteQuality_Fine, "fine");
    paletteQualityList.insert(PaletteQuality_Finer, "finer");
    paletteQualityList.insert(PaletteQuality_ExtraFine, "extra_fine");

    // PaletteOrderType
    paletteOrderTypeList.insert(PaletteOrder_Hermes, "hermes");
    paletteOrderTypeList.insert(PaletteOrder_Jet, "jet");
    paletteOrderTypeList.insert(PaletteOrder_Copper, "copper");
    paletteOrderTypeList.insert(PaletteOrder_Hot, "hot");
    paletteOrderTypeList.insert(PaletteOrder_Bone, "bone");
    paletteOrderTypeList.insert(PaletteOrder_Pink, "pink");
    paletteOrderTypeList.insert(PaletteOrder_Spring, "spring");
    paletteOrderTypeList.insert(PaletteOrder_Summer, "summer");
    paletteOrderTypeList.insert(PaletteOrder_Autumn, "autumn");
    paletteOrderTypeList.insert(PaletteOrder_Winter, "winter");
    paletteOrderTypeList.insert(PaletteOrder_HSV, "hsv");
    paletteOrderTypeList.insert(PaletteOrder_BWAsc, "bw_ascending");
    paletteOrderTypeList.insert(PaletteOrder_BWDesc, "bw_descending");
}

QString stringListToString(const QStringList &list)
{
    QString out;
    foreach (QString str, list)
        if (!str.isEmpty())
            out += str + ", ";

    if (out.length() > 0)
        out = out.left(out.length() - 2);

    return out;
}

QString analysisTypeString(AnalysisType analysisType)
{
    switch (analysisType)
    {
    case AnalysisType_SteadyState:
        return QObject::tr("Steady state");
    case AnalysisType_Transient:
        return QObject::tr("Transient");
    case AnalysisType_Harmonic:
        return QObject::tr("Harmonic");
    default:
        std::cerr << "Analysis type '" + QString::number(analysisType).toStdString() + "' is not implemented. analysisTypeString(AnalysisType analysisType)" << endl;
        throw;
    }
}

QString couplingTypeString(CouplingType couplingType)
{
    switch (couplingType)
    {
    case CouplingType_None:
        return QObject::tr("Not used");
    case CouplingType_Hard:
        return QObject::tr("Hard");
    case CouplingType_Weak:
        return QObject::tr("Weak");
    default:
        std::cerr << "Coupling type '" + QString::number(couplingType).toStdString() + "' is not implemented. couplingTypeString(CouplingType couplingType)" << endl;
        throw;
    }
}

QString teModeString(Mode teMode)
{
    switch (teMode)
    {
    case Mode_0:
        return QObject::tr("TE Mode 0");
    case Mode_01:
        return QObject::tr("TE Mode 01");
    case Mode_02:
        return QObject::tr("TE Mode 02");
    default:
        std::cerr << "TE mode '" + QString::number(teMode).toStdString() + "' is not implemented. TEModeString(TEMode teMode)" << endl;
        throw;
    }
}

QString physicFieldVariableCompString(PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariableComp)
    {
    case PhysicFieldVariableComp_Scalar:
        return QObject::tr("Scalar");
    case PhysicFieldVariableComp_Magnitude:
        return QObject::tr("Magnitude");
    case PhysicFieldVariableComp_X:
        return Util::problem()->config()->labelX();
    case PhysicFieldVariableComp_Y:
        return Util::problem()->config()->labelY();
    default:
        return QObject::tr("Undefined");
    }
}

QString coordinateTypeString(CoordinateType coordinateType)
{
    return ((coordinateType == CoordinateType_Planar) ? QObject::tr("Planar") : QObject::tr("Axisymmetric"));
}

QString adaptivityTypeString(AdaptivityType adaptivityType)
{
    switch (adaptivityType)
    {
    case AdaptivityType_None:
        return QObject::tr("Disabled");
    case AdaptivityType_H:
        return QObject::tr("h-adaptivity");
    case AdaptivityType_P:
        return QObject::tr("p-adaptivity");
    case AdaptivityType_HP:
        return QObject::tr("hp-adaptivity");
    default:
        std::cerr << "Adaptivity type '" + QString::number(adaptivityType).toStdString() + "' is not implemented. adaptivityTypeString(AdaptivityType adaptivityType)" << endl;
        throw;
    }
}

QString timeStepMethodString(TimeStepMethod timeStepMethod)
{
    switch (timeStepMethod)
    {
    case TimeStepMethod_Fixed:
        return QObject::tr("fixed");
    case TimeStepMethod_BDF2:
        return QObject::tr("adaptive");
    default:
        std::cerr << "Time step method '" + QString::number(timeStepMethod).toStdString() + "' is not implemented. timeStepMethodString(TimeStepMethod timeStepMethod)" << endl;
        throw;
    }
}

QString weakFormString(WeakFormKind weakForm)
{
    switch (weakForm)
    {
    case WeakForm_MatVol:
        return QObject::tr("Matrix volume");
    case WeakForm_MatSurf:
        return QObject::tr("Matrix surface");
    case WeakForm_VecVol:
        return QObject::tr("Vector volume");
    case WeakForm_VecSurf:
        return QObject::tr("Vector surface");
    default:
        std::cerr << "Weak form '" + QString::number(weakForm).toStdString() + "' is not implemented. weakFormString(WeakForm weakForm)" << endl;
        throw;
    }
}

QString meshTypeString(MeshType meshType)
{
    switch (meshType)
    {
    case MeshType_Triangle:
        return QObject::tr("Triangle");
    case MeshType_Triangle_QuadFineDivision:
        return QObject::tr("Triangle - quad fine div.");
    case MeshType_Triangle_QuadRoughDivision:
        return QObject::tr("Triangle - quad rough div.");
    case MeshType_Triangle_QuadJoin:
        return QObject::tr("Triangle - quad join");
    case MeshType_GMSH_Triangle:
        return QObject::tr("GMSH - triangle");
    case MeshType_GMSH_Quad:
        return QObject::tr("GMSH - quad");
    case MeshType_GMSH_QuadDelaunay_Experimental:
        return QObject::tr("GMSH - quad Delaunay (exp.)");
    default:
        std::cerr << "Mesh type '" + QString::number(meshType).toStdString() + "' is not implemented. meshTypeString(MeshType meshType)" << endl;
        throw;
    }
}

double paletteQualityToDouble(PaletteQuality paletteQuality)
{
    switch (paletteQuality)
    {
    case PaletteQuality_ExtremelyCoarse:
        return 0.1;
    case PaletteQuality_ExtraCoarse:
        return 0.01;
    case PaletteQuality_Coarser:
        return 0.001;
    case PaletteQuality_Coarse:
        return 0.0007;
    case PaletteQuality_Normal:
        return 0.0004;
    case PaletteQuality_Fine:
        return 0.0002;
    case PaletteQuality_Finer:
        return 0.00001;
    case PaletteQuality_ExtraFine:
        return 0.00003;
    default:
        std::cerr << "Palette quality '" + QString::number(paletteQuality).toStdString() + "' is not implemented. paletteQualityToDouble(PaletteQuality paletteQuality)" << endl;
        throw;
    }
}

QString paletteQualityString(PaletteQuality paletteQuality)
{
    switch (paletteQuality)
    {
    case PaletteQuality_ExtremelyCoarse:
        return QObject::tr("Extremely coarse");
    case PaletteQuality_ExtraCoarse:
        return QObject::tr("Extra coarse");
    case PaletteQuality_Coarser:
        return QObject::tr("Coarser");
    case PaletteQuality_Coarse:
        return QObject::tr("Coarse");
    case PaletteQuality_Normal:
        return QObject::tr("Normal");
    case PaletteQuality_Fine:
        return QObject::tr("Fine");
    case PaletteQuality_Finer:
        return QObject::tr("Finer");
    case PaletteQuality_ExtraFine:
        return QObject::tr("Extra fine");
    default:
        std::cerr << "Palette quality '" + QString::number(paletteQuality).toStdString() + "' is not implemented. paletteQualityString(PaletteQuality paletteQuality)" << endl;
        throw;
    }
}

QString paletteTypeString(PaletteType paletteType)
{
    switch (paletteType)
    {
    case Palette_Agros2D:
        return QObject::tr("Agros2D");
    case Palette_Jet:
        return QObject::tr("Jet");
    case Palette_Copper:
        return QObject::tr("Copper");
    case Palette_Hot:
        return QObject::tr("Hot");
    case Palette_Cool:
        return QObject::tr("Cool");
    case Palette_Bone:
        return QObject::tr("Bone");
    case Palette_Pink:
        return QObject::tr("Pink");
    case Palette_Spring:
        return QObject::tr("Spring");
    case Palette_Summer:
        return QObject::tr("Summer");
    case Palette_Autumn:
        return QObject::tr("Autumn");
    case Palette_Winter:
        return QObject::tr("Winter");
    case Palette_HSV:
        return QObject::tr("HSV");
    case Palette_BWAsc:
        return QObject::tr("B/W ascending");
    case Palette_BWDesc:
        return QObject::tr("B/W descending");
    default:
        std::cerr << "Palette type '" + QString::number(paletteType).toStdString() + "' is not implemented. paletteTypeString(PaletteType paletteType)" << endl;
        throw;
    }
}

QString matrixSolverTypeString(Hermes::MatrixSolverType matrixSolverType)
{
    switch (matrixSolverType)
    {
    case Hermes::SOLVER_UMFPACK:
        return QObject::tr("UMFPACK");
    case Hermes::SOLVER_PETSC:
        return QObject::tr("PETSc");
    case Hermes::SOLVER_MUMPS:
        return QObject::tr("MUMPS");
    case Hermes::SOLVER_SUPERLU:
        return QObject::tr("SuperLU");
    case Hermes::SOLVER_AMESOS:
        return QObject::tr("Trilinos/Amesos");
    case Hermes::SOLVER_AZTECOO:
        return QObject::tr("Trilinos/AztecOO");
    default:
        std::cerr << "Matrix solver type '" + QString::number(matrixSolverType).toStdString() + "' is not implemented. matrixSolverTypeString(MatrixSolverType matrixSolverType)" << endl;
        throw;
    }
}

QString linearityTypeString(LinearityType linearityType)
{
    switch (linearityType)
    {
    case LinearityType_Linear:
        return QObject::tr("Linear");
    case LinearityType_Picard:
        return QObject::tr("Picard's method");
    case LinearityType_Newton:
        return QObject::tr("Newton's method");
    default:
        std::cerr << "Linearity type '" + QString::number(linearityType).toStdString() + "' is not implemented. linearityTypeString(LinearityType linearityType)" << endl;
        throw;
    }
}

void setGUIStyle(const QString &styleName)
{
    QStyle *style = NULL;
    if (styleName == "Manhattan")
    {
        QString styleName = "";
        QStringList styles = QStyleFactory::keys();

#ifdef Q_WS_X11
        // kde 3
        if (getenv("KDE_FULL_SESSION") != NULL)
            styleName = "Plastique";
        // kde 4
        if (getenv("KDE_SESSION_VERSION") != NULL)
        {
            if (styles.contains("Oxygen"))
                styleName = "Oxygen";
            else
                styleName = "Plastique";
        }

        // gtk+
        if (styleName == "")
            styleName = "GTK+";
#else
        styleName = "Plastique";
#endif

        style = new ManhattanStyle(styleName);
    }
    else
    {
        // standard style
        style = QStyleFactory::create(styleName);
    }

    QApplication::setStyle(style);
    if (QApplication::desktopSettingsAware())
    {
        QApplication::setPalette(QApplication::palette());
    }
}

void setLanguage(const QString &locale)
{
    // non latin-1 chars
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    QTranslator *qtTranslator = new QTranslator();
    QTranslator *appTranslator = new QTranslator();

    QString country = locale.section('_',0,0);
    if (QFile::exists(QLibraryInfo::location(QLibraryInfo::TranslationsPath) + "/qt_" + country + ".qm"))
        qtTranslator->load(QLibraryInfo::location(QLibraryInfo::TranslationsPath) + "/qt_" + country + ".qm");
    else if (QFile::exists(datadir() + LANGUAGEROOT + "/qt_" + country + ".qm"))
        qtTranslator->load(datadir() + LANGUAGEROOT + "/qt_" + country + ".qm");
    else
        qDebug() << "Qt language file not found.";

    if (QFile::exists(datadir() + LANGUAGEROOT + QDir::separator() + locale + ".qm"))
        appTranslator->load(datadir() + LANGUAGEROOT + QDir::separator() + locale + ".qm");
    else if (QFile::exists(datadir() + LANGUAGEROOT + "/en_US.qm"))
        appTranslator->load(datadir() + LANGUAGEROOT + "/en_US.qm");
    else
        qDebug() << "Language file not found.";

    QApplication::installTranslator(qtTranslator);
    QApplication::installTranslator(appTranslator);
}

QStringList availableLanguages()
{
    QDir dir;
    dir.setPath(datadir() + LANGUAGEROOT);

    // add all translations
    QStringList filters;
    filters << "*.qm";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    // remove extension
    QStringList list = dir.entryList();
    list.replaceInStrings(".qm", "");

    // remove system translations
    foreach (QString str, list)
        if (str.startsWith("qt_"))
            list.removeOne(str);

    return list;
}

QIcon icon(const QString &name)
{
    QString fileName;

#ifdef Q_WS_WIN
    if (QFile::exists(":/" + name + "-windows.png")) return QIcon(":/" + name + "-windows.png");
#endif

#ifdef Q_WS_X11
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
    return QIcon::fromTheme(name, QIcon(":/" + name + ".png"));
#endif

    QDir dir;

    QString style = "";
    QStringList styles = QStyleFactory::keys();

    // kde 3
    if (getenv("KDE_FULL_SESSION") != NULL)
    {}
    // kde 4
    if (getenv("KDE_SESSION_VERSION") != NULL)
    {
        // oxygen
        fileName = "/usr/share/icons/oxygen/22x22/actions/" + name;
        if (QFile::exists(fileName + ".svg")) return QIcon(fileName + ".svg");
        if (QFile::exists(fileName + ".png")) return QIcon(fileName + ".png");
    }
    // gtk+
    if (style == "")
    {
        // humanity (disabled - corrupted svg reader - Qt 4.6 has new method QIcon::fromTheme)
        // fileName = "/usr/share/icons/Humanity/actions/24/" + name;
        // if (QFile::exists(fileName + ".svg")) return QIcon(fileName + ".svg");
        // if (QFile::exists(fileName + ".png")) return QIcon(fileName + ".png");
    }
#endif

    if (QFile::exists(":/" + name + ".svg")) return QIcon(":/" + name + ".svg");
    if (QFile::exists(":/" + name + ".png")) return QIcon(":/" + name + ".png");

    return QIcon();
}

QString datadir()
{
    // windows and local installation
    if (QFile::exists(QApplication::applicationDirPath() + "/functions.py"))
        return QApplication::applicationDirPath();

    // linux
    if (QFile::exists(QApplication::applicationDirPath() + "/../share/agros2d/functions.py"))
        return QApplication::applicationDirPath() + "/../share/agros2d";

    qCritical() << "Datadir not found.";
    exit(1);
}

QString tempProblemDir()
{
    QDir(QDir::temp().absolutePath()).mkpath("agros2d/" + QString::number(QApplication::applicationPid()));

    return QString("%1/agros2d/%2").arg(QDir::temp().absolutePath()).arg(QApplication::applicationPid());
}

QString tempProblemFileName()
{
    return tempProblemDir() + "/temp";
}

QTime milisecondsToTime(int ms)
{
    // store the current ms remaining
    int tmp_ms = ms;

    // the amount of days left
    int days = floorf(tmp_ms/86400000);
    // adjust tmp_ms to leave remaining hours, minutes, seconds
    tmp_ms = tmp_ms - (days * 86400000);

    // calculate the amount of hours remaining
    int hours = floorf(tmp_ms/3600000);
    // adjust tmp_ms to leave the remaining minutes and seconds
    tmp_ms = tmp_ms - (hours * 3600000);

    // the amount of minutes remaining
    int mins = floorf(tmp_ms/60000);
    //adjust tmp_ms to leave only the remaining seconds
    tmp_ms = tmp_ms - (mins * 60000);

    // seconds remaining
    int secs = floorf(tmp_ms/1000);

    // milliseconds remaining
    tmp_ms = tmp_ms - (secs * 1000);

    return QTime(hours, mins, secs, tmp_ms);
}

bool removeDirectory(const QDir &dir)
{
    bool error = false;

    if (dir.exists())
    {
        QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
        int count = entries.size();
        for (int idx = 0; idx < count; idx++)
        {
            QFileInfo entryInfo = entries[idx];
            QString path = entryInfo.absoluteFilePath();
            if (entryInfo.isDir())
            {
                error = removeDirectory(QDir(path));
            }
            else
            {
                QFile file(path);
                if (!file.remove())
                {
                    error = true;
                    break;
                }
            }
        }
        if (!dir.rmdir(dir.absolutePath()))
            error = true;
    }

    return error;
}

void msleep(unsigned long msecs)
{
    QWaitCondition w;
    QMutex sleepMutex;
    sleepMutex.lock();
    w.wait(&sleepMutex, msecs);
    sleepMutex.unlock();
}

void appendToFile(const QString &fileName, const QString &str)
{
    QFile file(fileName);

    if (file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream outFile(&file);
        outFile << str << endl;

        file.close();
    }
}

void showPage(const QString &str)
{
    if (str.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(datadir() + "/resources/help/index.html"));
    else
        QDesktopServices::openUrl(QUrl::fromLocalFile(datadir() + "/resources/help/" + str));
}


QString readFileContent(const QString &fileName)
{
    QString content;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        content = stream.readAll();
        file.close();
        return content;
    }
    return NULL;
}

void writeStringContent(const QString &fileName, QString *content)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << *content;

        file.waitForBytesWritten(0);
        file.close();
    }
}

QByteArray readFileContentByteArray(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray content = file.readAll();
        file.close();
        return content;
    }
    return NULL;
}

void writeStringContentByteArray(const QString &fileName, QByteArray content)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(content);

        file.waitForBytesWritten(0);
        file.close();
    }
}

Point centerPoint(const Point &pointStart, const Point &pointEnd, double angle)
{
    double distance = (pointEnd - pointStart).magnitude();
    Point t = (pointEnd - pointStart) / distance;
    double R = distance / (2.0*sin(angle/180.0*M_PI / 2.0));

    Point p = Point(distance/2.0, sqrt(Hermes::sqr(R) - Hermes::sqr(distance)/4.0 > 0.0 ? Hermes::sqr(R) - Hermes::sqr(distance)/4.0 : 0.0));
    Point center = pointStart + Point(p.x*t.x - p.y*t.y, p.x*t.y + p.y*t.x);

    return center;
}

//   Function tests if angle specified by the third parameter is between angles
//   angle_1 and angle_2
bool isBetween(double angle_1, double angle_2, double angle)
{
    // tolerance 1e-3 degree
    double tol = 1e-3 * M_PI / 180;

    // angle_1 has to be lower then angle_2
    if (angle_2 < angle_1)
    {
        double temp = angle_1;
        angle_1 = angle_2;
        angle_2 = temp;
    }

    // angle_1 is in the third quadrant, angle_2 is in the second quadrant
    if ((angle_1 <= - (M_PI_2 - tol) ) && (angle_2 >= (M_PI_2 - tol)))
    {
        if((angle <= angle_1) || (angle >= angle_2))
        {
           return true;
        }

    } else
    {
        if((angle >= angle_1) && (angle <= angle_2))
        {
           return true;
        }
    }
    return false;
}

bool intersection(Point p, Point p1s, Point p1e)
{
    Point dv_line = p1e - p1s;
    Point dv_point = p1e - p;
    if ((dv_line.angle() - dv_point.angle()) < EPS_ZERO)
        return true;
    else
        return false;
}

Point *intersection(Point p1s, Point p1e, Point p2s, Point p2e)
{
    double denom = (p2e.y-p2s.y)*(p1e.x-p1s.x) - (p2e.x-p2s.x)*(p1e.y-p1s.y);

    double nume_a = ((p2e.x-p2s.x)*(p1s.y-p2s.y) - (p2e.y-p2s.y)*(p1s.x-p2s.x));
    double nume_b = ((p1e.x-p1s.x)*(p1s.y-p2s.y) - (p1e.y-p1s.y)*(p1s.x-p2s.x));

    double ua = nume_a / denom;
    double ub = nume_b / denom;

    if ((p2e != p1s) && (p1e != p2s) && (p1e != p2e) && (p1s != p2s))
    {
        if ((abs(denom) > EPS_ZERO) && (ua >= 0.0) && (ua <= 1.0) && (ub >= 0.0) && (ub <= 1.0))
        {
            double xi = p1s.x + ua*(p1e.x - p1s.x);
            double yi = p1s.y + ua*(p1e.y - p1s.y);

            return new Point(xi, yi);
        }
    }

    return NULL;
}

QList<Point> intersection(Point p1s, Point p1e, Point center1, double radius1, double angle1,
                          Point p2s, Point p2e, Point center2, double radius2, double angle2)
{
    QList<Point> out;

    double dx = p1e.x - p1s.x;      // component of direction vector of the line
    double dy = p1e.y - p1s.y;      // component of direction vector of the line
    double a = dx * dx + dy * dy;   // square of the length of direction vector
    double tol = sqrt(a)/1e4;

    // Crossing of two arcs
    if ((angle1 > 0.0) && (angle2 > 0.0))
    {
        if (((p1s == p2e) && (p1e == p2s)) || ((p1e == p2e) && (p1s == p2s)))
        {
            // Crossing of arcs is not possible
        }
        else
        {
            // Calculate distance between centres of circle
            float distance = (center1 - center2).magnitude();
            float dx = center2.x - center1.x;
            float dy = center2.y - center1.y;

            if ((distance < (radius1 + radius2)))
            {
                // Determine the distance from point 0 to point 2.
                double a = ((radius1*radius1) - (radius2*radius2) + (distance*distance)) / (2.0 * distance);

                // Determine the coordinates of point 2.
                Point middle;
                middle.x = center1.x + (dx * a/distance);
                middle.y = center1.y + (dy * a/distance);

                // Determine the distance from point 2 to either of the
                // intersection points.
                double h = std::sqrt((radius1 * radius1) - (a*a));

                // Now determine the offsets of the intersection points from
                // point 2.
                double rx = -dy * (h/distance);
                double ry =  dx * (h/distance);

                // Determine the absolute intersection points.
                Point p1(middle.x + rx, middle.y + ry);
                Point p2(middle.x - rx, middle.y - ry);

                // Angles of starting and end points due to center of the arc 1
                double angle1_1 = (p1e - center1).angle();
                double angle2_1 = (p1s - center1).angle();

                // Angles of starting and end points due to center of the arc 2
                double angle1_2 = (p2e - center2).angle();
                double angle2_2 = (p2s - center2).angle();

                // Angles of intersection points due to center of the arc 1
                double iangle1_1 = (p1 - center1).angle();
                double iangle2_1 = (p2 - center1).angle();

                // Angles of intersection points due to center of the arc 2
                double iangle1_2 = (p1 - center2).angle();
                double iangle2_2 = (p2 - center2).angle();

                // Test if intersection 1 lies on arc
                if ((isBetween(angle1_1, angle2_1, iangle1_1) && isBetween(angle1_2 , angle2_2, iangle1_2)))
                {
                    if (((p1 - p1s).magnitude() > tol) &&
                            ((p1 - p1e).magnitude() > tol) &&
                            ((p1 - p2e).magnitude() > tol) &&
                            ((p1 - p2s).magnitude() > tol))
                        out.append(p1);
                }

                // Test if intersection 1 lies on arc
                if (isBetween(angle1_1, angle2_1, iangle2_1) && isBetween(angle1_2, angle2_2, iangle2_2))
                {
                    if(((p2 - p1s).magnitude() > tol) &&
                            ((p2 - p1e).magnitude() > tol) &&
                            ((p2 - p2e).magnitude() > tol) &&
                            ((p2 - p2s).magnitude() > tol))
                        out.append(p2);
                }
            }
        }
    }
    else
    {
        if (angle2 > 0.0)
            // crossing of arc and line
        {
            double b = 2 * (dx * (p1s.x - center2.x) + dy * (p1s.y - center2.y));
            double c = p1s.x * p1s.x + p1s.y * p1s.y + center2.x * center2.x + center2.y * center2.y - 2 * (center2.x * p1s.x + center2.y * p1s.y)-(radius2 * radius2);

            double bb4ac = b * b - 4 * a * c;

            double mu1 = (-b + sqrt(bb4ac)) / (2*a);
            double mu2 = (-b - sqrt(bb4ac)) / (2*a);

            double i1x = p1s.x + mu1*(dx);
            double i1y = p1s.y + mu1*(dy);

            double i2x = p1s.x + mu2*(dx);
            double i2y = p1s.y + mu2*(dy);


            Point p1(i1x, i1y);     // possible intersection point
            Point p2(i2x, i2y);     // possible intersection point

            double t1;
            double t2;

            if ((dx - dy) > tol)
            {
                t1 = (p1.x - p1s.x - p1.y + p1s.y) / (dx - dy); // tangent
                t2 = (p2.x - p1s.x - p2.y + p1s.y) / (dx - dy); // tangent
            }
            else
            {
                t1 = (p1.x - p1s.x) / dx; // tangent
                t2 = (p2.x - p1s.x) / dx; // tangent
            }

            double angle_1 = (p2e - center2).angle();
            double angle_2 = (p2s - center2).angle();
            double iangle1 = (p1 - center2).angle();
            double iangle2 = (p2 - center2).angle();


            if ((t1 >= 0) && (t1 <= 1))
            {
                // 1 solution: One Point in the circle
                if (isBetween(angle_1, angle_2, iangle1) && ((p1 - p2s).magnitude() > tol) && ((p1 - p2e).magnitude() > tol))
                    out.append(p1);
            }

            if ((t2 >= 0) && (t2 <= 1))
            {
                // 1 solution: One Point in the circle
                if (isBetween(angle_1, angle_2, iangle2) && ((p2 -  p2s).magnitude() > tol) && ((p2 - p2e).magnitude() > tol))
                    out.append(p2);
            }
        }
        else
        {
            // straight line
            Point *point = intersection(p1s, p1e, p2s, p2e);
            if (point)
                out.append(Point(point->x, point->y));
            delete point;
        }
    }
    return out;
}

static CheckVersion *checkVersion = NULL;
void checkForNewVersion(bool quiet)
{
    // download version
    QUrl url("http://agros2d.org/version/version.xml");
    if (checkVersion == NULL)
        checkVersion = new CheckVersion(url);

    checkVersion->run(quiet);
}

QString unitToHTML(const QString &str)
{
    // dirty hack
    QString out = str;

    out.replace("-2", "<sup>&#8722;2</sup>");
    out.replace("-3", "<sup>&#8722;3</sup>");
    out.replace("2", "<sup>2</sup>");
    out.replace("3", "<sup>3</sup>");

    return out;
}

CheckVersion::CheckVersion(QUrl url) : QObject()
{
    m_url = url;

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(downloadFinished(QNetworkReply *)));
}

CheckVersion::~CheckVersion()
{
    delete m_manager;
}

void CheckVersion::run(bool quiet)
{
    m_quiet = quiet;
    m_networkReply = m_manager->get(QNetworkRequest(m_url));

    connect(m_networkReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(showProgress(qint64,qint64)));
    connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(handleError(QNetworkReply::NetworkError)));
}

void CheckVersion::downloadFinished(QNetworkReply *networkReply)
{
    QString text = networkReply->readAll();

    if (!text.isEmpty())
    {
        QDomDocument doc;
        doc.setContent(text);

        // main document
        QDomElement eleDoc = doc.documentElement();

        // problems
        QDomNode eleVersion = eleDoc.toElement().elementsByTagName("version").at(0);

        int beta = eleVersion.toElement().attribute("beta").toInt() == 1;
        int major = eleVersion.toElement().attribute("major").toInt();
        int minor = eleVersion.toElement().attribute("minor").toInt();
        int sub = eleVersion.toElement().attribute("sub").toInt();
        int git = eleVersion.toElement().attribute("git").toInt();
        int year = eleVersion.toElement().attribute("year").toInt();
        int month = eleVersion.toElement().attribute("month").toInt();
        int day = eleVersion.toElement().attribute("day").toInt();

        QDomNode eleUrl = eleDoc.toElement().elementsByTagName("url").at(0);

        if (!m_quiet && git == 0)
        {
            QMessageBox::critical(QApplication::activeWindow(), tr("New version"), tr("File is corrupted or network is disconnected."));
            return;
        }

        QString downloadUrl = eleUrl.toElement().text();
        if (git > VERSION_GIT)
        {
            QString str(tr("<b>New version available.</b><br/><br/>"
                           "Actual version: %1<br/>"
                           "New version: %2<br/><br/>"
                           "URL: <a href=\"%3\">%3</a>").
                        arg(QApplication::applicationVersion()).
                        arg(versionString(major, minor, sub, git, year, month, day, beta)).
                        arg(downloadUrl));

            QMessageBox::information(QApplication::activeWindow(), tr("New version"), str);
        }
        else if (!m_quiet)
        {
            QMessageBox::information(QApplication::activeWindow(), tr("New version"), tr("You are using actual version."));
        }
    }
}

void CheckVersion::showProgress(qint64 dl, qint64 all)
{
    // qDebug() << QString("\rDownloaded %1 bytes of %2).").arg(dl).arg(all);
}

void CheckVersion::handleError(QNetworkReply::NetworkError error)
{
    qDebug() << "An error ocurred (code #" << error << ").";
}

QString transformXML(const QString &fileName, const QString &stylesheetFileName)
{
    QString out;

    QXmlQuery query(QXmlQuery::XSLT20);
    query.setFocus(QUrl(fileName));
    query.setQuery(QUrl(stylesheetFileName));
    query.evaluateTo(&out);

    return out;
}

ErrorResult validateXML(const QString &fileName, const QString &schemaFileName)
{
    QXmlSchema schema;
    schema.load(QUrl(schemaFileName));

    MessageHandler schemaMessageHandler;
    schema.setMessageHandler(&schemaMessageHandler);

    if (!schema.isValid())
        return ErrorResult(ErrorResultType_Critical, QObject::tr("Schena '%1' is not valid. %2").
                           arg(schemaFileName).
                           arg(schemaMessageHandler.statusMessage()));

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);

    QXmlSchemaValidator validator(schema);
    MessageHandler validatorMessageHandler;
    validator.setMessageHandler(&validatorMessageHandler);

    //TODO neslo mi nacist soubor se dvema poli
//    if (!validator.validate(&file, QUrl::fromLocalFile(file.fileName())))
//        return ErrorResult(ErrorResultType_Critical, QObject::tr("File '%1' is not valid Agros2D problem file. Error (line %3, column %4): %2").
//                           arg(fileName).
//                           arg(validatorMessageHandler.statusMessage()).
//                           arg(validatorMessageHandler.line()).
//                           arg(validatorMessageHandler.column()));

    return ErrorResult();
}
