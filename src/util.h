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
#include <QDomDocument>

#include <Python.h>

#include <typeinfo>
#include <iostream>
#include <cmath>
#include <limits>
#include <vector>

#include <locale.h>
#include <stdlib.h>
#include <common.h>

#include "value.h"

#define EPS_ZERO 1e-10
#define EPS0 8.854e-12
#define MU0 4*M_PI*1e-7
#define NDOF_STOP 40000
#define CONST_DOUBLE 1e100

#define deg2rad(degrees) (degrees*M_PI/180.0)
#define rad2deg(radians) (radians*180.0/M_PI)

using namespace std;

// verbose
void setVerbose(bool verb);

// log file
void logOutput(QtMsgType type, const char *msg);
void logMessage(const QString &msg);

// show page
void showPage(const QString &str = "");

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
QTime milisecondsToTime(int ms);

// remove directory content
bool removeDirectory(const QDir &dir);

// sleep function
void msleep(unsigned long msecs);

// read file content
QByteArray readFileContentByteArray(const QString &fileName);
QString readFileContent(const QString &fileName);

// write content into the file
void writeStringContent(const QString &fileName, QString *content);
void writeStringContentByteArray(const QString &fileName, QByteArray content);

// append to the file
void appendToFile(const QString &fileName, const QString &str);

// fill physic field combo box
void fillComboBoxPhysicField(QComboBox *cmbPhysicField);

// check for new version
void checkForNewVersion(bool quiet = false);

// join version
inline QString versionString(int major, int minor, int sub, int git, int year, int month, int day, bool beta)
{
    return QString("%1.%2.%3.%4 %5 (%6-%7-%8)")
            .arg(major)
            .arg(minor)
            .arg(sub)
            .arg(git)
            .arg(beta ? "beta" : "")
            .arg(year)
            .arg(QString("0%1").arg(month).right(2))
            .arg(QString("0%1").arg(day).right(2));
}

class CheckVersion : public QObject
{
    Q_OBJECT
public:
    CheckVersion(QUrl url);
    ~CheckVersion();
    void run(bool quiet);

private:
    bool m_quiet;
    QUrl m_url;
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_networkReply;

private slots:
    void downloadFinished(QNetworkReply *networkReply);
    void showProgress(qint64, qint64);
    void handleError(QNetworkReply::NetworkError error);
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

    inline double magnitude() const { return sqrt(x * x + y * y); }
    inline double angle() const { return atan2(y, x); }

    Point normalizePoint() const
    {
        double m = magnitude();

        double mx = x/m;
        double my = y/m;

        return Point(mx, my);
    }
};

// return center
Point centerPoint(const Point &pointStart, const Point &pointEnd, double angle);

struct Point3
{
    double x, y, z;

    Point3() { this->x = 0; this->y = 0; this->z = 0; }
    Point3(double x, double y, double z) { this->x = x; this->y = y; this->z = z; }

    inline Point3 operator+(const Point3 &vec) const { return Point3(x + vec.x, y + vec.y, z + vec.z); }
    inline Point3 operator-(const Point3 &vec) const { return Point3(x - vec.x, y - vec.y, z - vec.z); }
    inline Point3 operator*(double num) const { return Point3(x * num, y * num, z * num); }
    inline Point3 operator/(double num) const { return Point3(x / num, y / num, z / num); }

    inline double magnitude() const { return sqrt(x * x + y * y); }
    inline double anglexy() const { return atan2(y, x); }
    inline double angleyz() const { return atan2(z, y); }
    inline double anglezx() const { return atan2(x, z); }

    Point3 normalizePoint() const
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
    inline double width() const { return fabs(end.x - start.x); }
    inline double height() const { return fabs(end.y - start.y); }
};

struct ScriptResult
{
    ScriptResult()
    {
        text = "";
        isError = false;
        this->line = -1;
    }

    ScriptResult(const QString &text, bool isError = false, int line = -1)
    {
        this->text = text;
        this->isError = isError;
        this->line = line;
    }

    QString text;
    bool isError;
    int line;
};

struct ExpressionResult
{
    ExpressionResult()
    {
        this->error = "";
        this->value = 0.0;
    }

    ExpressionResult(double value, const QString &error)
    {
        this->error = error;
        this->value = value;
    }

    QString error;
    double value;
};

enum ErrorResultType
{
    ErrorResultType_None,
    ErrorResultType_Information,
    ErrorResultType_Warning,
    ErrorResultType_Critical
};

class ErrorResult
{
public:
    inline ErrorResultType type() { return m_type; }
    inline QString message() { return m_message; }

    inline ErrorResult()
    {
        m_type = ErrorResultType_None;
        m_message = "";
    }

    inline ErrorResult(ErrorResultType type, QString message)
    {
        m_type = type;
        m_message = message;
    }

    inline bool isError() { return (m_type != ErrorResultType_None); }

    void showDialog()
    {
        switch (m_type)
        {
        case ErrorResultType_None:
            return;
        case ErrorResultType_Information:
            QMessageBox::information(QApplication::activeWindow(), QObject::tr("Information"), m_message);
            break;
        case ErrorResultType_Warning:
            QMessageBox::warning(QApplication::activeWindow(), QObject::tr("Warning"), m_message);
            break;
        case ErrorResultType_Critical:
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
    SolverMode_Mesh,
    SolverMode_MeshAndSolve
};

enum ProblemType
{
    ProblemType_Undefined,
    ProblemType_Planar,
    ProblemType_Axisymmetric
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

enum PhysicFieldVariableComp
{
    PhysicFieldVariableComp_Undefined,
    PhysicFieldVariableComp_Scalar,
    PhysicFieldVariableComp_Magnitude,
    PhysicFieldVariableComp_X,
    PhysicFieldVariableComp_Y
};

enum PhysicField
{
    PhysicField_Undefined,
    PhysicField_General,
    PhysicField_Electrostatic,
    PhysicField_Magnetic,
    PhysicField_Current,
    PhysicField_Heat,
    PhysicField_Elasticity,
    PhysicField_Flow,
    PhysicField_RF,
    PhysicField_Acoustic
};

enum PhysicFieldVariable
{
    PhysicFieldVariable_Undefined,
    PhysicFieldVariable_None,
    PhysicFieldVariable_Order,
    PhysicFieldVariable_Variable,
    PhysicFieldVariable_General_Gradient,
    PhysicFieldVariable_General_Constant,
    PhysicFieldVariable_Electrostatic_Potential,
    PhysicFieldVariable_Electrostatic_ElectricField,
    PhysicFieldVariable_Electrostatic_Displacement,
    PhysicFieldVariable_Electrostatic_EnergyDensity,
    PhysicFieldVariable_Electrostatic_Permittivity,
    PhysicFieldVariable_Magnetic_VectorPotential,
    PhysicFieldVariable_Magnetic_VectorPotentialReal,
    PhysicFieldVariable_Magnetic_VectorPotentialImag,
    PhysicFieldVariable_Magnetic_FluxDensity,
    PhysicFieldVariable_Magnetic_FluxDensityReal,
    PhysicFieldVariable_Magnetic_FluxDensityImag,
    PhysicFieldVariable_Magnetic_MagneticField,
    PhysicFieldVariable_Magnetic_MagneticFieldReal,
    PhysicFieldVariable_Magnetic_MagneticFieldImag,
    PhysicFieldVariable_Magnetic_CurrentDensity,
    PhysicFieldVariable_Magnetic_CurrentDensityReal,
    PhysicFieldVariable_Magnetic_CurrentDensityImag,
    PhysicFieldVariable_Magnetic_CurrentDensityTotal,
    PhysicFieldVariable_Magnetic_CurrentDensityTotalReal,
    PhysicFieldVariable_Magnetic_CurrentDensityTotalImag,
    PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform,
    PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal,
    PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformImag,
    PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity,
    PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal,
    PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityImag,
    PhysicFieldVariable_Magnetic_PowerLosses,
    PhysicFieldVariable_Magnetic_LorentzForce,
    PhysicFieldVariable_Magnetic_Remanence,
    PhysicFieldVariable_Magnetic_EnergyDensity,
    PhysicFieldVariable_Magnetic_Permeability,
    PhysicFieldVariable_Magnetic_Conductivity,
    PhysicFieldVariable_Magnetic_Velocity,
    PhysicFieldVariable_Current_Potential,
    PhysicFieldVariable_Current_ElectricField,
    PhysicFieldVariable_Current_CurrentDensity,
    PhysicFieldVariable_Current_Losses,
    PhysicFieldVariable_Current_Conductivity,
    PhysicFieldVariable_Heat_Temperature,
    PhysicFieldVariable_Heat_TemperatureGradient,
    PhysicFieldVariable_Heat_Flux,
    PhysicFieldVariable_Heat_Conductivity,
    PhysicFieldVariable_Elasticity_VonMisesStress,
    PhysicFieldVariable_Elasticity_Displacement,
    PhysicFieldVariable_Elasticity_StrainXX,
    PhysicFieldVariable_Elasticity_StrainYY,
    PhysicFieldVariable_Elasticity_StrainZZ,
    PhysicFieldVariable_Elasticity_StrainXY,
    PhysicFieldVariable_Elasticity_StressXX,
    PhysicFieldVariable_Elasticity_StressYY,
    PhysicFieldVariable_Elasticity_StressZZ,
    PhysicFieldVariable_Elasticity_StressXY,
    PhysicFieldVariable_Flow_Velocity,
    PhysicFieldVariable_Flow_VelocityX,
    PhysicFieldVariable_Flow_VelocityY,
    PhysicFieldVariable_Flow_Pressure,
    PhysicFieldVariable_RF_ElectricField,
    PhysicFieldVariable_RF_ElectricFieldReal,
    PhysicFieldVariable_RF_ElectricFieldImag,
    PhysicFieldVariable_RF_MagneticField,
    PhysicFieldVariable_RF_MagneticFieldReal,
    PhysicFieldVariable_RF_MagneticFieldImag,
    PhysicFieldVariable_RF_PowerLosses,
    PhysicFieldVariable_RF_EnergyDensity,
    PhysicFieldVariable_RF_Permittivity,
    PhysicFieldVariable_RF_Permeability,
    PhysicFieldVariable_RF_Conductivity,
    PhysicFieldVariable_RF_J_Ext_real,
    PhysicFieldVariable_RF_J_Ext_imag,
    PhysicFieldVariable_Acoustic_Pressure,
    PhysicFieldVariable_Acoustic_PressureReal,
    PhysicFieldVariable_Acoustic_PressureImag,
    PhysicFieldVariable_Acoustic_LocalVelocity,
    PhysicFieldVariable_Acoustic_LocalAcceleration,
    PhysicFieldVariable_Acoustic_PressureLevel,
    PhysicFieldVariable_Acoustic_Density,
    PhysicFieldVariable_Acoustic_Speed
};


enum PhysicFieldBC
{
    PhysicFieldBC_Undefined,
    PhysicFieldBC_None,
    PhysicFieldBC_General_Value,
    PhysicFieldBC_General_Derivative,
    PhysicFieldBC_Electrostatic_Potential,
    PhysicFieldBC_Electrostatic_SurfaceCharge,
    PhysicFieldBC_Magnetic_VectorPotential,
    PhysicFieldBC_Magnetic_SurfaceCurrent,
    PhysicFieldBC_Heat_Temperature,
    PhysicFieldBC_Heat_Flux,
    PhysicFieldBC_Current_Potential,
    PhysicFieldBC_Current_InwardCurrentFlow,
    PhysicFieldBC_Elasticity_Fixed,
    PhysicFieldBC_Elasticity_Free,
    PhysicFieldBC_Flow_Velocity,
    PhysicFieldBC_Flow_Pressure,
    PhysicFieldBC_Flow_Outlet,
    PhysicFieldBC_Flow_Wall,
    PhysicFieldBC_RF_ElectricField,
    PhysicFieldBC_RF_MagneticField,
    PhysicFieldBC_RF_MatchedBoundary,
    PhysicFieldBC_RF_Port,
    PhysicFieldBC_Acoustic_Pressure,
    PhysicFieldBC_Acoustic_NormalAcceleration,
    PhysicFieldBC_Acoustic_Impedance,
    PhysicFieldBC_Acoustic_MatchedBoundary
};

inline bool isPhysicFieldVariableScalar(PhysicFieldVariable physicFieldVariable)
{
    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Variable:
    case PhysicFieldVariable_General_Constant:

    case PhysicFieldVariable_Electrostatic_Potential:
    case PhysicFieldVariable_Electrostatic_EnergyDensity:
    case PhysicFieldVariable_Electrostatic_Permittivity:

    case PhysicFieldVariable_Magnetic_VectorPotential:
    case PhysicFieldVariable_Magnetic_VectorPotentialReal:
    case PhysicFieldVariable_Magnetic_VectorPotentialImag:

    case PhysicFieldVariable_Magnetic_CurrentDensity:
    case PhysicFieldVariable_Magnetic_CurrentDensityReal:
    case PhysicFieldVariable_Magnetic_CurrentDensityImag:
    case PhysicFieldVariable_Magnetic_CurrentDensityTotal:
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalReal:
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalImag:
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform:
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal:
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformImag:
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity:
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal:
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityImag:
    case PhysicFieldVariable_Magnetic_EnergyDensity:
    case PhysicFieldVariable_Magnetic_PowerLosses:
    case PhysicFieldVariable_Magnetic_Permeability:
    case PhysicFieldVariable_Magnetic_FluxDensity:
    case PhysicFieldVariable_Magnetic_MagneticField:
    case PhysicFieldVariable_Magnetic_Conductivity:

    case PhysicFieldVariable_Heat_Temperature:
    case PhysicFieldVariable_Heat_Conductivity:

    case PhysicFieldVariable_Current_Potential:
    case PhysicFieldVariable_Current_Losses:

    case PhysicFieldVariable_Elasticity_VonMisesStress:
    case PhysicFieldVariable_Elasticity_StrainXX:
    case PhysicFieldVariable_Elasticity_StrainYY:
    case PhysicFieldVariable_Elasticity_StrainZZ:
    case PhysicFieldVariable_Elasticity_StrainXY:
    case PhysicFieldVariable_Elasticity_StressXX:
    case PhysicFieldVariable_Elasticity_StressYY:
    case PhysicFieldVariable_Elasticity_StressZZ:
    case PhysicFieldVariable_Elasticity_StressXY:

    case PhysicFieldVariable_Flow_Pressure:

    case PhysicFieldVariable_RF_ElectricField:
    case PhysicFieldVariable_RF_ElectricFieldReal:
    case PhysicFieldVariable_RF_ElectricFieldImag:
    case PhysicFieldVariable_RF_MagneticField:
    case PhysicFieldVariable_RF_MagneticFieldReal:
    case PhysicFieldVariable_RF_MagneticFieldImag:
    case PhysicFieldVariable_RF_PowerLosses:
    case PhysicFieldVariable_RF_EnergyDensity:
    case PhysicFieldVariable_RF_Permittivity:
    case PhysicFieldVariable_RF_Permeability:
    case PhysicFieldVariable_RF_Conductivity:
    case PhysicFieldVariable_RF_J_Ext_real:
    case PhysicFieldVariable_RF_J_Ext_imag:

    case PhysicFieldVariable_Acoustic_Pressure:
    case PhysicFieldVariable_Acoustic_PressureReal:
    case PhysicFieldVariable_Acoustic_PressureImag:
    case PhysicFieldVariable_Acoustic_PressureLevel:
    case PhysicFieldVariable_Acoustic_Density:
    case PhysicFieldVariable_Acoustic_Speed:
        return true;
        break;
    default:
        return false;
        break;
    }
}

enum SceneMode
{
    SceneMode_OperateOnNodes,
    SceneMode_OperateOnEdges,
    SceneMode_OperateOnLabels,
    SceneMode_Postprocessor
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
    SceneMode_LocalValue,
    SceneMode_SurfaceIntegral,
    SceneMode_VolumeIntegral
};

enum PaletteType
{
    Palette_Jet,
    Palette_Autumn,
    Palette_Copper,
    Palette_Hot,
    Palette_Cool,
    Palette_BWAsc,
    Palette_BWDesc
};

enum PaletteOrderType
{
    PaletteOrder_Hermes,
    PaletteOrder_Jet,
    PaletteOrder_BWAsc,
    PaletteOrder_BWDesc
};

enum SceneViewPostprocessorShow
{
    SceneViewPostprocessorShow_Undefined,
    SceneViewPostprocessorShow_None,
    SceneViewPostprocessorShow_ScalarView,
    SceneViewPostprocessorShow_ScalarView3D,
    SceneViewPostprocessorShow_ScalarView3DSolid,
    SceneViewPostprocessorShow_Model,
    SceneViewPostprocessorShow_Order
};

// captions
QString physicFieldVariableString(PhysicFieldVariable physicFieldVariable);
QString physicFieldVariableUnitsString(PhysicFieldVariable physicFieldVariable);
QString physicFieldVariableShortcutString(PhysicFieldVariable physicFieldVariable);
QString physicFieldString(PhysicField physicField);
QString analysisTypeString(AnalysisType analysisType);
QString physicFieldBCString(PhysicFieldBC physicFieldBC);
QString physicFieldVariableCompString(PhysicFieldVariableComp physicFieldVariableComp);
QString problemTypeString(ProblemType problemType);
QString adaptivityTypeString(AdaptivityType adaptivityType);
QString matrixSolverTypeString(MatrixSolverType matrixSolverType);

inline QString errorNormString(ProjNormType projNormType)
{
    switch (projNormType)
    {
    case HERMES_H1_NORM:
        return QObject::tr("H1 norm");
    case HERMES_L2_NORM:
        return QObject::tr("L2 norm");
    case HERMES_H1_SEMINORM:
        return QObject::tr("H1 seminorm");
    case HERMES_HDIV_NORM:
        return QObject::tr("Hdiv norm");
    case HERMES_HCURL_NORM:
        return QObject::tr("Hcurl norm");
    default:
        std::cerr << "Norm '" + QString::number(projNormType).toStdString() + "' is not implemented. QString errorNormString(ProjNormType projNormType)" << endl;
        throw;
    }
}

// keys
void initLists();

QString physicFieldToStringKey(PhysicField physicField);
PhysicField physicFieldFromStringKey(const QString &physicField);

inline QString problemTypeToStringKey(ProblemType problemType) { return ((problemType == ProblemType_Planar) ? "planar" : "axisymmetric"); }
inline ProblemType problemTypeFromStringKey(const QString &problemType) { if (problemType == "planar") return ProblemType_Planar; else if (problemType == "axisymmetric") return ProblemType_Axisymmetric; else return ProblemType_Undefined; }

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

QString matrixSolverTypeToStringKey(MatrixSolverType matrixSolverType);
MatrixSolverType matrixSolverTypeFromStringKey(const QString &matrixSolverType);

// constants
const QColor COLORBACKGROUND = QColor::fromRgb(255, 255, 255);
const QColor COLORGRID = QColor::fromRgb(200, 200, 200);
const QColor COLORCROSS = QColor::fromRgb(150, 150, 150);
const QColor COLORNODES = QColor::fromRgb(150, 0, 0);
const QColor COLOREDGES = QColor::fromRgb(0, 0, 150);
const QColor COLORLABELS = QColor::fromRgb(0, 150, 0);
const QColor COLORCONTOURS = QColor::fromRgb(0, 0, 0);
const QColor COLORVECTORS = QColor::fromRgb(0, 0, 0);
const QColor COLORINITIALMESH = QColor::fromRgb(250, 202, 119);
const QColor COLORSOLUTIONMESH = QColor::fromRgb(150, 70, 0);
const QColor COLORHIGHLIGHTED = QColor::fromRgb(250, 150, 0);
const QColor COLORSELECTED = QColor::fromRgb(150, 0, 0);

// posprocessor
const int CONTOURSCOUNT = 15;

const PaletteType PALETTETYPE = Palette_Jet;
const bool PALETTEFILTER = false;
const int PALETTESTEPS = 30;
const bool SCALARRANGELOG = false;
const double SCALARRANGEBASE = 10;
const double SCALARDECIMALPLACE = 2;

const bool VECTORPROPORTIONAL = true;
const bool VECTORCOLOR = true;
const int VECTORNUMBER = 50;
const double VECTORSCALE = 0.6;

const bool ORDERLABEL = false;
const PaletteOrderType ORDERPALETTEORDERTYPE = PaletteOrder_Hermes;

// adaptivity
const bool ADAPTIVITY_ISOONLY = false;
const double ADAPTIVITY_CONVEXP = 1.0;
const double ADAPTIVITY_THRESHOLD = 0.3;
const int ADAPTIVITY_STRATEGY = 0;
const int ADAPTIVITY_MESHREGULARITY = -1;
const ProjNormType ADAPTIVITY_PROJNORMTYPE = HERMES_H1_NORM;

// command argument
const QString COMMANDS_TRIANGLE = "%1 -p -P -q30.0 -e -A -a -z -Q -I \"%2\"";
const QString COMMANDS_FFMPEG = "%1 -r %2 -y -i \"%3video_%08d.png\" -vcodec %4 \"%5\"";

#endif // UTIL_H
