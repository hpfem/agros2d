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

#ifndef UTIL_H
#define UTIL_H

#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include <QtHelp/QHelpEngine>

#include <typeinfo>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <math.h>
#include <locale.h>

const double EPS_ZERO = 1e-14;
const double EPS0 = 8.854e-12;
const double MU0 = 4*M_PI*1e-7;
const int NDOF_STOP = 40000;

using namespace std;

// enable log file
void enableLogFile(bool enable);

// set gui style
void setGUIStyle(const QString &styleName);

// set language
void setLanguage(const QString &locale);

// get available languages
QStringList availableLanguages();

// get icon with respect to actual theme
QIcon icon(const QString &name);

// get datadir
QString datadir();

// get external js functions
QString externalFunctions();

// get temp dir
QString tempProblemDir();

// get temp filename
QString tempProblemFileName();

// convert time in ms to QTime
QTime milliSecondsToTime(int ms);

// remove directory content
bool removeDirectory(const QDir &dir);

// sleep function
void msleep(unsigned long msecs);

// log to file
void log(const QString &message);

// read file content
QByteArray readFileContentByteArray(const QString &fileName);
QString readFileContent(const QString &fileName);

// write content into the file
void writeStringContent(const QString &fileName, QString *content);
void writeStringContentByteArray(const QString &fileName, QByteArray content);

// global exception handler
void exception_global();

struct Value
{
    QString text;
    double number;

    Value() { text = "0"; number = 0;}
    Value(const QString &value) { text = value; }

    bool evaluate(bool quiet = false);
};

struct Point
{
    double x, y;

    Point() { this->x = 0; this->y = 0; }
    Point(double x, double y) { this->x = x; this->y = y; }

    inline Point operator+(const Point &vec) const { return Point(x + vec.x, y + vec.y); }
    inline Point operator-(const Point &vec) const { return Point(x - vec.x, y - vec.y); }
    inline Point operator*(double num) const { return Point(x * num, y * num); }
    inline Point operator/(double num) const { return Point(x / num, y / num); }
    inline double operator&(const Point &vec) const { return x*vec.x + y*vec.y; } // dot product
    inline double operator%(const Point &vec) const { return x*vec.y - y*vec.x; } // cross product
    inline bool operator!=(const Point &vec) const { return ((fabs(vec.x-x) > EPS_ZERO) || (fabs(vec.y-y) > EPS_ZERO)); }
    inline bool operator==(const Point &vec) const { return ((fabs(vec.x-x) < EPS_ZERO) && (fabs(vec.y-y) < EPS_ZERO)); }

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

enum ErrorResultType
{
    ERRORRESULT_NONE,
    ERRORRESULT_INFORMATION,
    ERRORRESULT_WARNING,
    ERRORRESULT_CRITICAL
};

class ErrorResult
{
public:
    inline ErrorResultType type() { return m_type; }
    inline QString message() { return m_message; }

    inline ErrorResult()
    {
        m_type = ERRORRESULT_NONE;
        m_message = "";
    }

    inline ErrorResult(ErrorResultType type, QString message)
    {
        m_type = type;
        m_message = message;
    }

    inline bool isError() { return (m_type != ERRORRESULT_NONE); }

    void showDialog()
    {
        switch (m_type)
        {
        case ERRORRESULT_NONE:
            return;
        case ERRORRESULT_INFORMATION:
            QMessageBox::information(QApplication::activeWindow(), QObject::tr("Information"), m_message);
            break;
        case ERRORRESULT_WARNING:
            QMessageBox::warning(QApplication::activeWindow(), QObject::tr("Warning"), m_message);
            break;
        case ERRORRESULT_CRITICAL:
            QMessageBox::critical(QApplication::activeWindow(), QObject::tr("Critical"), m_message);
            break;
        }
    }

private:
     ErrorResultType m_type;
     QString m_message;
};

enum SolverMode
{
    SOLVER_MESH,
    SOLVER_MESH_AND_SOLVE
};

enum ProblemType
{
    PROBLEMTYPE_UNDEFINED,
    PROBLEMTYPE_PLANAR,
    PROBLEMTYPE_AXISYMMETRIC
};

enum AnalysisType
{
    ANALYSISTYPE_UNDEFINED,
    ANALYSISTYPE_STEADYSTATE,
    ANALYSISTYPE_TRANSIENT,
    ANALYSISTYPE_HARMONIC
};

enum AdaptivityType
{
    ADAPTIVITYTYPE_UNDEFINED = 1000,
    ADAPTIVITYTYPE_NONE = 3,
    ADAPTIVITYTYPE_H = 1,
    ADAPTIVITYTYPE_P = 2,
    ADAPTIVITYTYPE_HP = 0
                    };

enum PhysicFieldVariableComp
{
    PHYSICFIELDVARIABLECOMP_UNDEFINED,
    PHYSICFIELDVARIABLECOMP_SCALAR,
    PHYSICFIELDVARIABLECOMP_MAGNITUDE,
    PHYSICFIELDVARIABLECOMP_X,
    PHYSICFIELDVARIABLECOMP_Y
};

enum PhysicField
{
    PHYSICFIELD_UNDEFINED,
    PHYSICFIELD_GENERAL,
    PHYSICFIELD_ELECTROSTATIC,
    PHYSICFIELD_CURRENT,
    PHYSICFIELD_HEAT,
    PHYSICFIELD_ELASTICITY,
    PHYSICFIELD_MAGNETIC
};

enum PhysicFieldVariable
{
    PHYSICFIELDVARIABLE_UNDEFINED,
    PHYSICFIELDVARIABLE_NONE,
    PHYSICFIELDVARIABLE_ORDER,
    PHYSICFIELDVARIABLE_GENERAL_VARIABLE,
    PHYSICFIELDVARIABLE_GENERAL_GRADIENT,
    PHYSICFIELDVARIABLE_GENERAL_CONSTANT,
    PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL,
    PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD,
    PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT,
    PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY,
    PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY,
    PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL,
    PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_REAL,
    PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_IMAG,
    PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY,
    PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL,
    PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_IMAG,
    PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD,
    PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_REAL,
    PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_IMAG,
    PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY,
    PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_REAL,
    PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_IMAG,
    PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL,
    PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL,
    PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG,
    PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM,
    PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_REAL,
    PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_IMAG,
    PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY,
    PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL,
    PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_IMAG,
    PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TRANSFORM,
    PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_VELOCITY,
    PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TOTAL,
    PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE,
    PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE_ANGLE,
    PHYSICFIELDVARIABLE_MAGNETIC_ENERGY_DENSITY,
    PHYSICFIELDVARIABLE_MAGNETIC_PERMEABILITY,
    PHYSICFIELDVARIABLE_MAGNETIC_CONDUCTIVITY,
    PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY,
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
    case PHYSICFIELDVARIABLE_GENERAL_VARIABLE:
        return QObject::tr("Variable");
    case PHYSICFIELDVARIABLE_GENERAL_GRADIENT:
        return QObject::tr("Gradient");
    case PHYSICFIELDVARIABLE_GENERAL_CONSTANT:
        return QObject::tr("Constant");

    case PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL:
        return QObject::tr("Scalar potential");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD:
        return QObject::tr("Electric field");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT:
        return QObject::tr("Displacement");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY:
        return QObject::tr("Energy density");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY:
        return QObject::tr("Permittivity");

    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_REAL:
        return QObject::tr("Vector potential - real");
    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_IMAG:
        return QObject::tr("Vector potential - imag");
    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL:
        return QObject::tr("Vector potential");
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL:
        return QObject::tr("Flux density - real");
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_IMAG:
        return QObject::tr("Flux density - imag");
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY:
        return QObject::tr("Flux density");
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_REAL:
        return QObject::tr("Magnetic field - real");
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_IMAG:
        return QObject::tr("Magnetic field - imag");
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD:
        return QObject::tr("Magnetic field");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_REAL:
        return QObject::tr("Current density - external - real");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_IMAG:
        return QObject::tr("Current density - external - imag");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY:
        return QObject::tr("Current density - external");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL:
        return QObject::tr("Current density - total - real");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG:
        return QObject::tr("Current density - total - imag");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL:
        return QObject::tr("Current density - total");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_REAL:
        return QObject::tr("Current density - induced transform - real");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_IMAG:
        return QObject::tr("Current density - induced transform - imag");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM:
        return QObject::tr("Current density - induced transform");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL:
        return QObject::tr("Current density - induced velocity - real");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_IMAG:
        return QObject::tr("Current density - induced velocity - imag");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY:
        return QObject::tr("Current density - induced velocity");
    case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TRANSFORM:
        return QObject::tr("Power losses - transform average");
    case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_VELOCITY:
        return QObject::tr("Power losses - velocity");
    case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TOTAL:
        return QObject::tr("Power losses - total");
    case PHYSICFIELDVARIABLE_MAGNETIC_ENERGY_DENSITY:
        return QObject::tr("Energy density");
    case PHYSICFIELDVARIABLE_MAGNETIC_PERMEABILITY:
        return QObject::tr("Permeability");
    case PHYSICFIELDVARIABLE_MAGNETIC_CONDUCTIVITY:
        return QObject::tr("Conductivity");
    case PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE:
        return QObject::tr("Remanent flux density");
    case PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE_ANGLE:
        return QObject::tr("Remanent flux density - direction");
    case PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY:
        return QObject::tr("Velocity");

    case PHYSICFIELDVARIABLE_CURRENT_POTENTIAL:
        return QObject::tr("Scalar potential");
    case PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD:
        return QObject::tr("Electic field");
    case PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY:
        return QObject::tr("Current density");
    case PHYSICFIELDVARIABLE_CURRENT_LOSSES:
        return QObject::tr("Power losses");
    case PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY:
        return QObject::tr("Conductivity");

    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE:
        return QObject::tr("Temperature");
    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT:
        return QObject::tr("Temperature gradient");
    case PHYSICFIELDVARIABLE_HEAT_FLUX:
        return QObject::tr("Heat flux");
    case PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY:
        return QObject::tr("Conductivity");

    case PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS:
        return QObject::tr("Von Mises stress");
    default:
        std::cerr << "Physical field '" + QString::number(physicFieldVariable).toStdString() + "' is not implemented. physicFieldVariableString(PhysicFieldVariable physicFieldVariable)" << endl;
        throw;
    }
}

inline QString physicFieldVariableUnits(PhysicFieldVariable physicFieldVariable)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_GENERAL_VARIABLE:
        return QObject::tr("");
    case PHYSICFIELDVARIABLE_GENERAL_GRADIENT:
        return QObject::tr("");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL:
        return QObject::tr("V");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD:
        return QObject::tr("V/m");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT:
        return QObject::tr("C/m2");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY:
        return QObject::tr("J/m3");
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY:
        return QObject::tr("-");
    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_REAL:
        return QObject::tr("Wb/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_IMAG:
        return QObject::tr("Wb/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL:
        return QObject::tr("Wb/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_REAL:
        return QObject::tr("T");
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY_IMAG:
        return QObject::tr("T");
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY:
        return QObject::tr("T");
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_REAL:
        return QObject::tr("A/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD_IMAG:
        return QObject::tr("A/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD:
        return QObject::tr("A/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_REAL:
        return QObject::tr("A/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_IMAG:
        return QObject::tr("A/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY:
        return QObject::tr("A/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_REAL:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_IMAG:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_IMAG:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TRANSFORM:
        return QObject::tr("W/m3");
    case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_VELOCITY:
        return QObject::tr("W/m3");
    case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TOTAL:
        return QObject::tr("W/m3");
    case PHYSICFIELDVARIABLE_MAGNETIC_ENERGY_DENSITY:
        return QObject::tr("J/m3");
    case PHYSICFIELDVARIABLE_MAGNETIC_PERMEABILITY:
        return QObject::tr("-");
    case PHYSICFIELDVARIABLE_MAGNETIC_CONDUCTIVITY:
        return QObject::tr("S/m");
    case PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE:
        return QObject::tr("T");
    case PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE_ANGLE:
        return QObject::tr("deg.");
    case PHYSICFIELDVARIABLE_MAGNETIC_VELOCITY:
        return QObject::tr("m/s");
    case PHYSICFIELDVARIABLE_CURRENT_POTENTIAL:
        return QObject::tr("V");
    case PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD:
        return QObject::tr("V/m");
    case PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY:
        return QObject::tr("A/m2");
    case PHYSICFIELDVARIABLE_CURRENT_LOSSES:
        return QObject::tr("W/m3");
    case PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY:
        return QObject::tr("S/m");
    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE:
        return QObject::tr("deg.");
    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT:
        return QObject::tr("K/m");
    case PHYSICFIELDVARIABLE_HEAT_FLUX:
        return QObject::tr("W/m2");
    case PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY:
        return QObject::tr("W/m.K");
    case PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS:
        return QObject::tr("Pa");
    default:
        std::cerr << "Physical field '" + QString::number(physicFieldVariable).toStdString() + "' is not implemented. physicFieldVariableUnits(PhysicFieldVariable physicFieldVariable)" << endl;
        throw;
    }
}

enum PhysicFieldBC
{
    PHYSICFIELDBC_UNDEFINED,
    PHYSICFIELDBC_NONE,
    PHYSICFIELDBC_GENERAL_VALUE,
    PHYSICFIELDBC_GENERAL_DERIVATIVE,
    PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL,
    PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE,
    PHYSICFIELDBC_MAGNETIC_VECTOR_POTENTIAL,
    PHYSICFIELDBC_MAGNETIC_SURFACE_CURRENT,
    PHYSICFIELDBC_HEAT_TEMPERATURE,
    PHYSICFIELDBC_HEAT_HEAT_FLUX,
    PHYSICFIELDBC_CURRENT_POTENTIAL,
    PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW,
    PHYSICFIELDBC_ELASTICITY_FIXED,
    PHYSICFIELDBC_ELASTICITY_FREE
};

enum PhysicFieldIntegralVolume
{
    PHYSICFIELDINTEGRAL_VOLUME_UNDEFINED,
    PHYSICFIELDINTEGRAL_VOLUME_VOLUME,
    PHYSICFIELDINTEGRAL_VOLUME_CROSSSECTION,

    PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD_X,
    PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD_Y,
    PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD,
    PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT_X,
    PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT_Y,
    PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT,
    PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ENERGY_DENSITY,

    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_LORENTZ_FORCE,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_LORENTZ_FORCE_X_REAL,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_LORENTZ_FORCE_X_IMAG,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_LORENTZ_FORCE_Y_REAL,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_LORENTZ_FORCE_Y_IMAG,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_TORQUE_REAL,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_TORQUE_IMAG,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_REAL,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_IMAG,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_INDUCED_REAL,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_INDUCED_IMAG,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_IMAG,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_POWER_LOSSES_TRANSFORM,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_POWER_LOSSES_VELOCITY,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_ENERGY_DENSITY,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_TORQUE_REAL,
    PHYSICFIELDINTEGRAL_VOLUME_MAGNETIC_TORQUE_IMAG,

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
    PHYSICFIELDINTEGRAL_VOLUME_CURRENT_LOSSES,
};

enum PhysicFieldIntegralSurface
{
    PHYSICFIELDINTEGRAL_SURFACE_UNDEFINED,
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
    case PHYSICFIELDVARIABLE_GENERAL_VARIABLE:
    case PHYSICFIELDVARIABLE_GENERAL_CONSTANT:

    case PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL:
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY:
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY:

    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL:
    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_REAL:
    case PHYSICFIELDVARIABLE_MAGNETIC_VECTOR_POTENTIAL_IMAG:

    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY:
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_REAL:
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_IMAG:
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL:
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL:
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG:
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM:
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_REAL:
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_TRANSFORM_IMAG:
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY:
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_REAL:
    case PHYSICFIELDVARIABLE_MAGNETIC_CURRENT_DENSITY_INDUCED_VELOCITY_IMAG:
    case PHYSICFIELDVARIABLE_MAGNETIC_ENERGY_DENSITY:
    case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TRANSFORM:
    case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_VELOCITY:
    case PHYSICFIELDVARIABLE_MAGNETIC_POWER_LOSSES_TOTAL:
    case PHYSICFIELDVARIABLE_MAGNETIC_PERMEABILITY:
    case PHYSICFIELDVARIABLE_MAGNETIC_FLUX_DENSITY:
    case PHYSICFIELDVARIABLE_MAGNETIC_MAGNETICFIELD:
    case PHYSICFIELDVARIABLE_MAGNETIC_CONDUCTIVITY:
    case PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE:
    case PHYSICFIELDVARIABLE_MAGNETIC_REMANENCE_ANGLE:

    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE:
    case PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY:

    case PHYSICFIELDVARIABLE_CURRENT_POTENTIAL:
    case PHYSICFIELDVARIABLE_CURRENT_LOSSES:

    case PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS:
        return true;
        break;
    default:
        return false;
        break;
    }
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

enum SceneViewPostprocessorShow
{
    SCENEVIEW_POSTPROCESSOR_SHOW_UNDEFINED,
    SCENEVIEW_POSTPROCESSOR_SHOW_NONE,
    SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW,
    SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3D,
    SCENEVIEW_POSTPROCESSOR_SHOW_SCALARVIEW3DSOLID,
    SCENEVIEW_POSTPROCESSOR_SHOW_ORDER
};

// captions

inline QString physicFieldString(PhysicField physicField)
{
    switch (physicField)
    {
    case PHYSICFIELD_GENERAL:
        return QObject::tr("General");
    case PHYSICFIELD_ELECTROSTATIC:
        return QObject::tr("Electrostatic");
    case PHYSICFIELD_MAGNETIC:
        return QObject::tr("Magnetic");
    case PHYSICFIELD_CURRENT:
        return QObject::tr("Current field");
    case PHYSICFIELD_HEAT:
        return QObject::tr("Heat transfer");
    case PHYSICFIELD_ELASTICITY:
        return QObject::tr("Elasticity");
    default:
        std::cerr << "Physical field '" + QString::number(physicField).toStdString() + "' is not implemented. physicFieldString(PhysicField physicField)" << endl;
        throw;
    }
}

inline QString analysisTypeString(AnalysisType analysisType)
{
    switch (analysisType)
    {
    case ANALYSISTYPE_STEADYSTATE:
        return QObject::tr("Steady state");
    case ANALYSISTYPE_TRANSIENT:
        return QObject::tr("Transient");
    case ANALYSISTYPE_HARMONIC:
        return QObject::tr("Harmonic");
    default:
        std::cerr << "Analysis type '" + QString::number(analysisType).toStdString() + "' is not implemented. analysisTypeString(AnalysisType analysisType)" << endl;
        throw;
    }
}

inline QString physicFieldBCString(PhysicFieldBC physicFieldBC)
{
    switch (physicFieldBC)
    {
    case PHYSICFIELDBC_GENERAL_VALUE:
        return QObject::tr("Value");
    case PHYSICFIELDBC_GENERAL_DERIVATIVE:
        return QObject::tr("Derivative");
    case PHYSICFIELDBC_ELECTROSTATIC_POTENTIAL:
        return QObject::tr("Fixed voltage");
    case PHYSICFIELDBC_ELECTROSTATIC_SURFACE_CHARGE:
        return QObject::tr("Surface charge density");
    case PHYSICFIELDBC_MAGNETIC_VECTOR_POTENTIAL:
        return QObject::tr("Vector potential");
    case PHYSICFIELDBC_MAGNETIC_SURFACE_CURRENT:
        return QObject::tr("Surface current density");
    case PHYSICFIELDBC_HEAT_TEMPERATURE:
        return QObject::tr("Temperature");
    case PHYSICFIELDBC_HEAT_HEAT_FLUX:
        return QObject::tr("Heat flux");
    case PHYSICFIELDBC_CURRENT_POTENTIAL:
        return QObject::tr("Potential");
    case PHYSICFIELDBC_CURRENT_INWARD_CURRENT_FLOW:
        return QObject::tr("Inward current flow");
    case PHYSICFIELDBC_ELASTICITY_FIXED:
        return QObject::tr("Fixed");
    case PHYSICFIELDBC_ELASTICITY_FREE:
        return QObject::tr("Free");
    default:
        std::cerr << "Physical field '" + QString::number(physicFieldBC).toStdString() + "' is not implemented. physicFieldBCString(PhysicFieldBC physicFieldBC)" << endl;
        throw;
    }
}

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
    default:
        return QObject::tr("Undefined");
    }
}

inline QString problemTypeString(ProblemType problemType)
{
    return ((problemType == PROBLEMTYPE_PLANAR) ? QObject::tr("Planar") : QObject::tr("Axisymmetric"));
}

inline QString adaptivityTypeString(AdaptivityType adaptivityType)
{
    switch (adaptivityType)
    {
    case ADAPTIVITYTYPE_NONE:
        return QObject::tr("Disabled");
    case ADAPTIVITYTYPE_H:
        return QObject::tr("h-adaptivity");
    case ADAPTIVITYTYPE_P:
        return QObject::tr("p-adaptivity");
    case ADAPTIVITYTYPE_HP:
        return QObject::tr("hp-adaptivity");
    default:
        std::cerr << "Adaptivity type '" + QString::number(adaptivityType).toStdString() + "' is not implemented. adaptivityTypeString(AdaptivityType adaptivityType)" << endl;
        throw;
    }
}

void initLists();

QString physicFieldToStringKey(PhysicField physicField);
PhysicField physicFieldFromStringKey(const QString &physicField);

inline QString problemTypeToStringKey(ProblemType problemType) { return ((problemType == PROBLEMTYPE_PLANAR) ? "planar" : "axisymmetric"); }
inline ProblemType problemTypeFromStringKey(const QString &problemType) { if (problemType == "planar") return PROBLEMTYPE_PLANAR; else if (problemType == "axisymmetric") return PROBLEMTYPE_AXISYMMETRIC; else return PROBLEMTYPE_UNDEFINED; }

QString analysisTypeToStringKey(AnalysisType analysisType);
AnalysisType analysisTypeFromStringKey(const QString &analysisType);

QString physicFieldVariableToStringKey(PhysicFieldVariable physicFieldVariable);
PhysicFieldVariable physicFieldVariableFromStringKey(const QString &physicFieldVariable);

QString physicFieldVariableCompToStringKey(PhysicFieldVariableComp physicFieldVariableComp);
PhysicFieldVariableComp physicFieldVariableCompFromStringKey(const QString &physicFieldVariableComp);

QString physicFieldBCToStringKey(PhysicFieldBC physicFieldBC);
PhysicFieldBC physicFieldBCFromStringKey(const QString &physicFieldBC);

QString sceneViewPostprocessorShowToStringKey(SceneViewPostprocessorShow sceneViewPostprocessorShow);
SceneViewPostprocessorShow sceneViewPostprocessorShowFromStringKey(const QString &sceneViewPostprocessorShow);

QString adaptivityTypeToStringKey(AdaptivityType adaptivityType);
AdaptivityType adaptivityTypeFromStringKey(const QString &adaptivityType);

#endif // UTIL_H
