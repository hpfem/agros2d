#ifndef UTIL_H
#define UTIL_H

#include <QTextStream>
#include <QTranslator>
#include <QTextCodec>
#include <QDir>
#include <QStringList>
#include <QApplication>
#include <QSettings>
#include <QString>
#include <QStyleFactory>
#include <QStyle>
#include <QtGui/QIcon>

#include <typeinfo>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <math.h>
#include <locale.h>

using namespace std;

const double EPS_ZERO = 1e-12;
const double EPS0 = 8.854e-12;
const double MU0 = 4*M_PI*1e-7;
const int NDOF_STOP = 40000;

struct Point
{
    double x, y;

    Point() { this->x = 0; this->y = 0; }
    Point(double x, double y) { this->x = x; this->y = y; }

    inline Point operator+(const Point &vec) const { return Point(x + vec.x, y + vec.y); }
    inline Point operator-(const Point &vec) const { return Point(x - vec.x, y - vec.y); }
    inline Point operator*(double num) const { return Point(x * num, y * num); }
    inline Point operator/(double num) const { return Point(x / num, y / num); }

    inline double magnitude() { return sqrt(x * x + y * y); }
    inline double angle() { return atan2(y, x); }

    Point normalizePoint()
    {
        double m = magnitude();

        double mx = x/m;
        double my = y/m;

        return Point(mx, my);
    }
};

struct Point3
{
    double x, y, z;

    Point3() { this->x = 0; this->y = 0; this->z = 0; }
    Point3(double x, double y, double z) { this->x = x; this->y = y; this->z = z; }

    inline Point3 operator+(const Point3 &vec) const { return Point3(x + vec.x, y + vec.y, z + vec.z); }
    inline Point3 operator-(const Point3 &vec) const { return Point3(x - vec.x, y - vec.y, z - vec.z); }
    inline Point3 operator*(double num) const { return Point3(x * num, y * num, z * num); }
    inline Point3 operator/(double num) const { return Point3(x / num, y / num, z / num); }

    inline double magnitude() { return sqrt(x * x + y * y); }
    inline double anglexy() { return atan2(y, x); }
    inline double angleyz() { return atan2(z, y); }
    inline double anglezx() { return atan2(x, z); }

    Point3 normalizePoint()
    {
        double m = magnitude();

        double mx = x/m;
        double my = y/m;
        double mz = z/m;

        return Point3(mx, my, mz);
    }
};

struct RectPoint
{
    Point start;
    Point end;

    inline RectPoint(const Point &start, const Point &end) { this->start = start; this->end = end; }
    inline RectPoint() { this->start = Point(); this->end = Point(); }

    inline void set(const Point &start, const Point &end) { this->start = start; this->end = end; }
    inline double width() { return fabs(end.x - start.x); }
    inline double height() { return fabs(end.y - start.y); }
};

enum SolverMode
{
    SOLVER_MESH,
    SOLVER_MESH_AND_SOLVE
};

enum ProblemType
{
    PROBLEMTYPE_PLANAR,
    PROBLEMTYPE_AXISYMMETRIC
};

inline QString problemTypeString(ProblemType problemType)
{
    if (problemType == PROBLEMTYPE_PLANAR)
        return QObject::tr("planar");
    else
        return QObject::tr("axisymmetric");
}

enum PhysicFieldVariableComp
{
    PHYSICFIELDVARIABLECOMP_SCALAR,
    PHYSICFIELDVARIABLECOMP_MAGNITUDE,
    PHYSICFIELDVARIABLECOMP_X,
    PHYSICFIELDVARIABLECOMP_Y
};

inline QString physicFieldVariableCompString(PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariableComp)
    {
    case PHYSICFIELDVARIABLECOMP_SCALAR:
        return QObject::tr("Scalar");
    case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
        return QObject::tr("Magnitude");
    case PHYSICFIELDVARIABLECOMP_X:
        return QObject::tr("X");
    case PHYSICFIELDVARIABLECOMP_Y:
        return QObject::tr("Y");
    }
}

enum PhysicField
{
    PHYSICFIELD_ELECTROSTATIC,
    PHYSICFIELD_MAGNETOSTATIC,
    PHYSICFIELD_CURRENT,
    PHYSICFIELD_HEAT_TRANSFER,
    PHYSICFIELD_ELASTICITY
};

inline QString physicFieldStringKey(PhysicField physicField)
{
    switch (physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        return "electrostatic";
    case PHYSICFIELD_MAGNETOSTATIC:
        return "magnetostatic";
    case PHYSICFIELD_CURRENT:
        return "current field";
    case PHYSICFIELD_HEAT_TRANSFER:
        return "heat transfer";
    case PHYSICFIELD_ELASTICITY:
        return "elasticity";
    default:
        cerr << "Physical field '" + QString::number(physicField).toStdString() + "' is not implemented. physicFieldString(PhysicField physicField)" << endl;
        throw;
        break;
    }
}

inline PhysicField physicFieldFromStringKey(const QString &physicFieldString)
{
    if (physicFieldString == "electrostatic")
        return PHYSICFIELD_ELECTROSTATIC;
    if (physicFieldString == "magnetostatic")
        return PHYSICFIELD_MAGNETOSTATIC;
    if (physicFieldString == "current field")
        return PHYSICFIELD_CURRENT;
    if (physicFieldString == "heat transfer")
        return PHYSICFIELD_HEAT_TRANSFER;
    if (physicFieldString == "elasticity")
        return PHYSICFIELD_ELASTICITY;
    cerr << "Physical field '" + physicFieldString.toStdString() + "' is not implemented. physicFieldFromString(const QString &physicFieldString)" << endl;
    throw;
}

enum PhysicFieldVariable
{
    PHYSICFIELDVARIABLE_NONE,
    PHYSICFIELDVARIABLE_ORDER,
    PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL,
    PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD,
    PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT,
    PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY,
    PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY,
    PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL,
    PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY,
    PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD,
    PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY,
    PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY,
    PHYSICFIELDVARIABLE_CURRENT_POTENTIAL,
    PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD,
    PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY,
    PHYSICFIELDVARIABLE_CURRENT_LOSSES,
    PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY,
    PHYSICFIELDVARIABLE_HEAT_TEMPERATURE,
    PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT,
    PHYSICFIELDVARIABLE_HEAT_FLUX,
    PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY,
    PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS
};

inline QString physicFieldVariableString(PhysicFieldVariable physicFieldVariable)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL:
        return QObject::tr("Scalar potential");
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD:
        return QObject::tr("Electric field");
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT:
        return QObject::tr("Displacement");
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY:
        return QObject::tr("Energy density");
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY:
        return QObject::tr("Permittivity");
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL:
        return QObject::tr("Vector potential");
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY:
        return QObject::tr("Flux density");
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD:
        return QObject::tr("Magnetic field");
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY:
        return QObject::tr("Energy density");
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY:
        return QObject::tr("Permeability");
        break;
    case PHYSICFIELDVARIABLE_CURRENT_POTENTIAL:
        return QObject::tr("Scalar potential");
        break;
    case PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD:
        return QObject::tr("Electic field");
        break;
    case PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY:
        return QObject::tr("Current density");
        break;
    case PHYSICFIELDVARIABLE_CURRENT_LOSSES:
        return QObject::tr("Power losses");
        break;
    case PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY:
        return QObject::tr("Conductivity");
        break;
    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE:
        return QObject::tr("Temperature");
        break;
    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT:
        return QObject::tr("Temperature gradient");
        break;
    case PHYSICFIELDVARIABLE_HEAT_FLUX:
        return QObject::tr("Heat flux");
        break;
    case PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY:
        return QObject::tr("Conductivity");
        break;
    case PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS:
        return QObject::tr("Von Mises stress");
        break;
    default:
        cerr << "Physical field '" + QString::number(physicFieldVariable).toStdString() + "' is not implemented. physicFieldVariableString(PhysicFieldVariable physicFieldVariable)" << endl;
        throw;
        break;
    }
}

inline QString physicFieldVariableUnits(PhysicFieldVariable physicFieldVariable)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL:
        return QObject::tr("V");
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD:
        return QObject::tr("V/m");
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT:
        return QObject::tr("C/m2");
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY:
        return QObject::tr("J/m3");
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY:
        return QObject::tr("F/m");
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL:
        return QObject::tr("Wb/m");
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY:
        return QObject::tr("T");
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD:
        return QObject::tr("A/m");
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY:
        return QObject::tr("J/m3");
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY:
        return QObject::tr("H/m");
        break;
    case PHYSICFIELDVARIABLE_CURRENT_POTENTIAL:
        return QObject::tr("V");
        break;
    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE:
        return QObject::tr("deg.");
        break;
    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT:
        return QObject::tr("K/m");
        break;
    case PHYSICFIELDVARIABLE_HEAT_FLUX:
        return QObject::tr("W/m2");
        break;
    case PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY:
        return QObject::tr("W/m.K");
        break;
    case PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS:
        return QObject::tr("Pa");
        break;
    default:
        cerr << "Physical field '" + QString::number(physicFieldVariable).toStdString() + "' is not implemented. physicFieldVariableUnits(PhysicFieldVariable physicFieldVariable)" << endl;
        throw;
        break;
    }
}

enum PhysicFieldBC
{
    PHYSICFIELDBC_NONE,
    PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL,
    PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE,
    PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL,
    PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT,
    PHYSICFIELDBC_HEAT_TEMPERATURE,
    PHYSICFIELDBC_HEAT_HEAT_FLUX,
    PHYSICFIELDBC_CURRENT_POTENTIAL,
    PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW,
    PHYSICFIELDBC_ELASTICITY_FIXED,
    PHYSICFIELDBC_ELASTICITY_FREE
};

inline QString physicFieldBCString(PhysicFieldBC physicFieldBC)
{
    switch (physicFieldBC)
    {
    case PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL:
        return QObject::tr("Fixed voltage");
        break;
    case PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE:
        return QObject::tr("Surface charge density");
        break;
    case PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL:
        return QObject::tr("Vector potential");
        break;
    case PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT:
        return QObject::tr("Surface current density");
        break;
    case PHYSICFIELDBC_HEAT_TEMPERATURE:
        return QObject::tr("Temperature");
        break;
    case PHYSICFIELDBC_HEAT_HEAT_FLUX:
        return QObject::tr("Heat flux");
        break;
    case PHYSICFIELDBC_CURRENT_POTENTIAL:
        return QObject::tr("Potential");
        break;
    case PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW:
        return QObject::tr("Inward current flow");
        break;
    case PHYSICFIELDBC_ELASTICITY_FIXED:
        return QObject::tr("Fixed");
        break;
    case PHYSICFIELDBC_ELASTICITY_FREE:
        return QObject::tr("Free");
        break;
    default:
        cerr << "Physical field '" + QString::number(physicFieldBC).toStdString() + "' is not implemented. physicFieldBCString(PhysicFieldBC physicFieldBC)" << endl;
        throw;
        break;
    }
}

inline QString physicFieldBCStringKey(PhysicFieldBC physicFieldBC)
{
    switch (physicFieldBC)
    {
    case PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL:
        return "potential";
        break;
    case PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE:
        return "surface_charge_density";
        break;
    case PHYSICFIELDBC_MAGNETOSTATIC_VECTOR_POTENTIAL:
        return "vector_potential";
        break;
    case PHYSICFIELDBC_MAGNETOSTATIC_SURFACE_CURRENT:
        return "surface_current_density";
        break;
    case PHYSICFIELDBC_HEAT_TEMPERATURE:
        return "temperature";
        break;
    case PHYSICFIELDBC_HEAT_HEAT_FLUX:
        return "heat_flux";
        break;
    case PHYSICFIELDBC_CURRENT_POTENTIAL:
        return "potential";
        break;
    case PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW:
        return "inward_current_flow";
        break;
    case PHYSICFIELDBC_ELASTICITY_FIXED:
        return "fixed";
        break;
    case PHYSICFIELDBC_ELASTICITY_FREE:
        return "free";
        break;
    default:
        cerr << "Physical field '" + QString::number(physicFieldBC).toStdString() + "' is not implemented. physicFieldBCStringKey(PhysicFieldBC physicFieldBC)" << endl;
        throw;
        break;
    }
}

enum PhysicFieldIntegralVolume
{
    PHYSICFIELDINTEGRAL_VOLUME_VOLUME,
    PHYSICFIELDINTEGRAL_VOLUME_CROSSSECTION,
    PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD_X,
    PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD_Y,
    PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD,
    PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT_X,
    PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT_Y,
    PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT,
    PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ENERGY_DENSITY,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD_X,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD_Y,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY_X,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY_Y,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_ENERGY_DENSITY,
    PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE,
    PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT_X,
    PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT_Y,
    PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT,
    PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX_X,
    PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX_Y,
    PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX,
    PHYSICFIELDINTEGRAL_VOLUME_CURRENT_ELECTRICFIELD_X,
    PHYSICFIELDINTEGRAL_VOLUME_CURRENT_ELECTRICFIELD_Y,
    PHYSICFIELDINTEGRAL_VOLUME_CURRENT_ELECTRICFIELD,
    PHYSICFIELDINTEGRAL_VOLUME_CURRENT_CURRENT_DENSITY_X,
    PHYSICFIELDINTEGRAL_VOLUME_CURRENT_CURRENT_DENSITY_Y,
    PHYSICFIELDINTEGRAL_VOLUME_CURRENT_CURRENT_DENSITY,
    PHYSICFIELDINTEGRAL_VOLUME_CURRENT_LOSSES
};

enum PhysicFieldIntegralSurface
{
    PHYSICFIELDINTEGRAL_SURFACE_LENGTH,
    PHYSICFIELDINTEGRAL_SURFACE_SURFACE,
    PHYSICFIELDINTEGRAL_SURFACE_ELECTROSTATIC_CHARGE_DENSITY,
    PHYSICFIELDINTEGRAL_SURFACE_HEAT_TEMPERATURE,
    PHYSICFIELDINTEGRAL_SURFACE_HEAT_TEMPERATURE_DIFFERENCE,
    PHYSICFIELDINTEGRAL_SURFACE_HEAT_FLUX,
    PHYSICFIELDINTEGRAL_SURFACE_CURRENT_CURRENT_DENSITY
};

inline bool isPhysicFieldVariableScalar(PhysicFieldVariable physicFieldVariable)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL:
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY:
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY:
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL:
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY:
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY:
    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE:
    case PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY:
    case PHYSICFIELDVARIABLE_CURRENT_POTENTIAL:
    case PHYSICFIELDVARIABLE_CURRENT_LOSSES:
    case PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS:
        return true;
        break;
    }

    return false;
}

enum SceneMode
{
    SCENEMODE_OPERATE_ON_NODES,
    SCENEMODE_OPERATE_ON_EDGES,
    SCENEMODE_OPERATE_ON_LABELS,
    SCENEMODE_POSTPROCESSOR
};

enum SceneModePostprocessor
{
    POSTPROCESSOR_LOCALVALUE,
    POSTPROCESSOR_SURFACEINTEGRAL,
    POSTPROCESSOR_VOLUMEINTEGRAL
};

enum PaletteType
{
    PALETTE_JET,
    PALETTE_AUTUMN,
    PALETTE_COPPER,
    PALETTE_HOT,
    PALETTE_COOL,
    PALETTE_BW_ASC,
    PALETTE_BW_DESC
};

struct SceneViewSettings
{
    bool showOrder;
    bool showScalarField;
    bool showVectors;
    bool showContours;
    bool showGeometry;
    bool showInitialMesh;
    bool showSolutionMesh;
    bool showGrid;

    double gridStep;

    int contoursCount;
    PhysicFieldVariable contourPhysicFieldVariable;

    // scalar view
    PhysicFieldVariable scalarPhysicFieldVariable;
    PhysicFieldVariableComp scalarPhysicFieldVariableComp;
    bool scalarRangeAuto;
    double scalarRangeMin;
    double scalarRangeMax;
    bool scalarView3D;

    // vector view
    PhysicFieldVariable vectorPhysicFieldVariable;
    bool vectorRangeAuto;
    double vectorRangeMin;
    double vectorRangeMax;

    // palete
    PaletteType paletteType;
    int paletteSteps;
    bool paletteFilter;    
};

// set gui style
void setGUIStyle(const QString &styleName);

// set language
void setLanguage(const QString &locale);

// get available languages
QStringList availableLanguages();

// get icon with respect to actual theme
QIcon icon(const QString &name);

// get appdir
QString appdir();

#endif // UTIL_H
