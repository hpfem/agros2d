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
#include <QtWebKit>
#include <QtXmlPatterns>

#include <muParser.h>

#include <typeinfo>
#include <iostream>
#include <cmath>
#include <limits>
#include <vector>
#include <tr1/memory>


#include <locale.h>
#include <stdlib.h>
#include <common.h>

#include "value.h"

#include "indicators/indicators.h"

#include "hermes2d.h"

// zero
#define EPS_ZERO 1e-10

// physical constants
#define EPS0 8.854e-12
#define MU0 4*M_PI*1e-7
#define SIGMA0 5.670373e-8
#define PRESSURE_MIN_AIR 20e-6

#define deg2rad(degrees) (degrees*M_PI/180.0)
#define rad2deg(radians) (radians*180.0/M_PI)

using namespace std;
using std::tr1::shared_ptr;

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

// dirty html unit replace
QString unitToHTML(const QString &str);

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

    QString toString() const
    {
        return QString("x = %1, y = %2, magnitude = %3").
                arg(x).
                arg(y).
                arg(magnitude());
    }
};

// return center
Point centerPoint(const Point &pointStart, const Point &pointEnd, double angle);

// intersection of two lines
Point *intersection(Point p1s, Point p1e, Point p2s, Point p2e);

// intersection of two lines or line and arc
QList<Point> intersection(Point p1s, Point p1e, Point center1, double radius1, double angle1,
                          Point p2s, Point p2e, Point center2, double radius2, double angle2);

struct Point3
{
    double x, y, z;

    Point3() { this->x = 0; this->y = 0; this->z = 0; }
    Point3(double x, double y, double z) { this->x = x; this->y = y; this->z = z; }

    inline Point3 operator+(const Point3 &vec) const { return Point3(x + vec.x, y + vec.y, z + vec.z); }
    inline Point3 operator-(const Point3 &vec) const { return Point3(x - vec.x, y - vec.y, z - vec.z); }
    inline Point3 operator*(double num) const { return Point3(x * num, y * num, z * num); }
    inline Point3 operator/(double num) const { return Point3(x / num, y / num, z / num); }
    inline double operator&(const Point3 &vec) const { return x*vec.x + y*vec.y + z*vec.z; } // dot product
    inline Point3 operator%(const Point3 &vec) const { return Point3(- z*vec.y, z*vec.x, x*vec.y - y*vec.x); } // cross product

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

    QString toString()
    {
        return QString("x = %1, y = %2, z = %3, magnitude = %4").
                arg(x).
                arg(y).
                arg(z).
                arg(magnitude());
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
        traceback = "";
        isError = false;
        this->line = -1;
    }

    ScriptResult(const QString &text, const QString &traceback, bool isError = false, int line = -1)
    {
        this->text = text;
        this->traceback = traceback;
        this->isError = isError;
        this->line = line;
    }

    QString text;
    QString traceback;
    bool isError;
    int line;
};

struct ExpressionResult
{
    ExpressionResult()
    {
        this->error = "";
        this->traceback = "";
        this->value = 0.0;
    }

    ExpressionResult(double value, const QString &traceback, const QString &error)
    {
        this->error = error;
        this->traceback = traceback;
        this->value = value;
    }

    QString error;
    QString traceback;
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

class MessageHandler : public QAbstractMessageHandler
 {
     public:
         MessageHandler()
             : QAbstractMessageHandler(0)
         {
         }

         QString statusMessage() const
         {
             return m_description;
         }

         int line() const
         {
             return m_sourceLocation.line();
         }

         int column() const
         {
             return m_sourceLocation.column();
         }

     protected:
         virtual void handleMessage(QtMsgType type, const QString &description,
                                    const QUrl &identifier, const QSourceLocation &sourceLocation)
         {
             Q_UNUSED(type);
             Q_UNUSED(identifier);

             m_messageType = type;
             m_description = description;
             m_sourceLocation = sourceLocation;
         }

     private:
         QtMsgType m_messageType;
         QString m_description;
         QSourceLocation m_sourceLocation;
 };

// xml transformation
QString transformXML(const QString &fileName, const QString &stylesheetFileName);

// xml validation
ErrorResult validateXML(const QString &fileName, const QString &schemaFileName);

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

enum LinearityType
{
    LinearityType_Undefined,
    LinearityType_Linear,
    LinearityType_Picard,
    LinearityType_Newton
};

enum CouplingType
{
    CouplingType_None,
    CouplingType_Weak,
    CouplingType_Hard
};

enum WeakFormsType
{
    WeakFormsType_Undefined,
    WeakFormsType_Interpreted,
    WeakFormsType_Compiled
};

enum MeshType
{
    MeshType_Triangle,
    MeshType_QuadFineDivision,
    MeshType_QuadRoughDivision,
    MeshType_QuadJoin
};

enum PhysicFieldVariableComp
{
    PhysicFieldVariableComp_Undefined,
    PhysicFieldVariableComp_Scalar,
    PhysicFieldVariableComp_Magnitude,
    PhysicFieldVariableComp_X,
    PhysicFieldVariableComp_Y
};

enum Mode
{
    Mode_0,
    Mode_01,
    Mode_02
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
    SceneModePostprocessor_LocalValue,
    SceneModePostprocessor_SurfaceIntegral,
    SceneModePostprocessor_VolumeIntegral
};

enum PaletteType
{
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

// FIXME: deprecated - DO NOT USE
enum SceneViewPost3DShow
{
    SceneViewPost3DShow_Undefined,
    SceneViewPost3DShow_None,
    SceneViewPost3DShow_ScalarView3D,
    SceneViewPost3DShow_ScalarView3DSolid,
    SceneViewPost3DShow_Model,
    SceneViewPost3DShow_ParticleTracing
};

enum SceneTransformMode
{
    SceneTransformMode_Translate,
    SceneTransformMode_Rotate,
    SceneTransformMode_Scale
};

enum SolutionType
{
    SolutionType_Normal,
    SolutionType_Reference,
    SolutionType_NonExisting,
    SolutionType_Finer  // used to choose reference if exists, normal otherwise
};

// captions
QString analysisTypeString(AnalysisType analysisType);
QString teModeString(Mode teMode);
QString physicFieldVariableCompString(PhysicFieldVariableComp physicFieldVariableComp);
QString coordinateTypeString(CoordinateType coordinateType);
QString adaptivityTypeString(AdaptivityType adaptivityType);
QString couplingTypeString(CouplingType couplingType);
QString weakFormsTypeString(WeakFormsType weakFormsType);
QString meshTypeString(MeshType meshType);
QString linearityTypeString(LinearityType linearityType);
QString matrixSolverTypeString(Hermes::MatrixSolverType matrixSolverType);

inline QString errorNormString(Hermes::Hermes2D::ProjNormType projNormType)
{
    switch (projNormType)
    {
    case Hermes::Hermes2D::HERMES_H1_NORM:
        return QObject::tr("H1 norm");
    case Hermes::Hermes2D::HERMES_L2_NORM:
        return QObject::tr("L2 norm");
    case Hermes::Hermes2D::HERMES_H1_SEMINORM:
        return QObject::tr("H1 seminorm");
    case Hermes::Hermes2D::HERMES_HDIV_NORM:
        return QObject::tr("Hdiv norm");
    case Hermes::Hermes2D::HERMES_HCURL_NORM:
        return QObject::tr("Hcurl norm");
    default:
        std::cerr << "Norm '" + QString::number(projNormType).toStdString() + "' is not implemented. QString errorNormString(ProjNormType projNormType)" << endl;
        throw;
    }
}

QString stringListToString(const QStringList &list);

// keys
void initLists();

QStringList coordinateTypeStringKeys();
QString coordinateTypeToStringKey(CoordinateType coordinateType);
CoordinateType coordinateTypeFromStringKey(const QString &coordinateType);

QStringList analysisTypeStringKeys();
QString analysisTypeToStringKey(AnalysisType analysisType);
AnalysisType analysisTypeFromStringKey(const QString &analysisType);

QStringList weakFormsTypeStringKeys();
QString weakFormsTypeToStringKey(WeakFormsType weakFormsType);
WeakFormsType weakFormsTypeFromStringKey(const QString &weakFormsType);

QStringList meshTypeStringKeys();
QString meshTypeToStringKey(MeshType meshType);
MeshType meshTypeFromStringKey(const QString &meshType);

QStringList physicFieldVariableCompTypeStringKeys();
QString physicFieldVariableCompToStringKey(PhysicFieldVariableComp physicFieldVariableComp);
PhysicFieldVariableComp physicFieldVariableCompFromStringKey(const QString &physicFieldVariableComp);

QStringList adaptivityTypeStringKeys();
QString adaptivityTypeToStringKey(AdaptivityType adaptivityType);
AdaptivityType adaptivityTypeFromStringKey(const QString &adaptivityType);

QStringList solutionTypeStringKeys();
QString solutionTypeToStringKey(SolutionType solutionType);
SolutionType solutionTypeFromStringKey(const QString &solutionType);

QStringList matrixSolverTypeStringKeys();
QString matrixSolverTypeToStringKey(Hermes::MatrixSolverType matrixSolverType);
Hermes::MatrixSolverType matrixSolverTypeFromStringKey(const QString &matrixSolverType);

QStringList linearityTypeStringKeys();
QString linearityTypeToStringKey(LinearityType linearityType);
LinearityType linearityTypeFromStringKey(const QString &linearityType);

// FIXME: deprecated - DO NOT USE
QString sceneViewPost3DShowToStringKey(SceneViewPost3DShow sceneViewPost3DShow);
SceneViewPost3DShow sceneViewPost3DShowFromStringKey(const QString &sceneViewPost3DShow);


QString modeToStringKey(Mode teMode);
Mode modeFromStringKey(const QString &teMode);

// constants
const QString IMAGEROOT = "../resources_source/images";
const QString LANGUAGEROOT = "/resources/lang";
const QString MODULEROOT = "/resources/modules";
const QString COUPLINGROOT = "/resources/couplings";
const QString TEMPLATEROOT = "/resources/templates";

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
const QColor COLORCROSSED = QColor::fromRgb(255, 0, 0);
const QColor COLORNOTCONNECTED = QColor::fromRgb(255, 0, 0);

// workspace
const double GRIDSTEP = 0.05;
const bool SHOWGRID = true;
const bool SNAPTOGRID = true;

#ifdef Q_WS_X11
    const QFont FONT = QFont("Monospace", 9);
#endif
#ifdef Q_WS_WIN
    const QFont FONT = QFont("Courier New", 9);
#endif
#ifdef Q_WS_MAC
    const QFont FONT = QFont("Monaco", 12);
#endif

const bool SHOWAXES = true;
const bool SHOWRULERS = true;
const bool ZOOMTOMOUSE = true;

const int GEOMETRYNODESIZE = 6;
const int GEOMETRYEDGEWIDTH = 2;
const int GEOMETRYLABELSIZE = 6;

// mesh
const bool SHOWINITIALMESHVIEW = true;
const bool SHOWSOLUTIONMESHVIEW = false;
const int MESHANGLESEGMENTSCOUNT = 3;
const bool MESHCURVILINEARELEMENTS = true;

// post2d

// post3d
const SceneViewPost3DShow SCALARSHOWPOST3D = SceneViewPost3DShow_ScalarView3D;

const double LINEARIZER_QUALITY = 0.0006;

const bool SHOWCONTOURVIEW = false;
const int CONTOURSCOUNT = 15;

const PaletteType PALETTETYPE = Palette_Jet;
const bool PALETTEFILTER = false;
const int PALETTESTEPS = 30;

const bool SHOWVECTORVIEW = false;
const bool VECTORPROPORTIONAL = true;
const bool VECTORCOLOR = true;
const int VECTORNUMBER = 50;
const double VECTORSCALE = 0.6;

const bool SHOWORDERVIEW = true;
const bool SHOWORDERCOLORBAR = true;
const PaletteOrderType ORDERPALETTEORDERTYPE = PaletteOrder_Hermes;
const bool ORDERLABEL = false;

// particle
const bool SHOWPARTICLEVIEW = false;
const bool PARTICLEINCLUDEGRAVITATION = true;
const int PARTICLENUMBEROFPARTICLES = 5;
const double PARTICLESTARTINGRADIUS = 0;
const double PARTICLESTARTX = 0;
const double PARTICLESTARTY = 0;
const double PARTICLESTARTVELOCITYX = 0;
const double PARTICLESTARTVELOCITYY = 0;
const double PARTICLEMASS = 9.109e-31; // mass of the electron
const double PARTICLECONSTANT = 1.602e-19; // charge of the electron
const double PARTICLETERMINATEONDIFFERENTMATERIAL = true;
const double PARTICLEMAXIMUMRELATIVEERROR = 0.0;
const bool PARTICLECOLORBYVELOCITY = true;
const bool PARTICLESHOWPOINTS = false;
const int PARTICLEMAXIMUMSTEPS = 1000;
const double PARTICLEDRAGDENSITY = 1.2041;
const double PARTICLEDRAGCOEFFICIENT = 0.0;
const double PARTICLEDRAGREFERENCEAREA = 1e-6;

// scalarview
const bool SHOWSCALARVIEW = true;
const bool SHOWSCALARCOLORBAR = true;
const bool SCALARFIELDRANGELOG = false;
const int SCALARFIELDRANGEBASE = 10;
const int SCALARDECIMALPLACE = 2;
const bool SCALARRANGEAUTO = true;
const double SCALARRANGEMIN = 0.0;
const double SCALARRANGEMAX = 1.0;

const bool VIEW3DLIGHTING = false;
const double VIEW3DANGLE = 230.0;
const bool VIEW3DBACKGROUND = true;
const double VIEW3DHEIGHT = 4.0;

const bool DEFORMSCALAR = true;
const bool DEFORMCONTOUR = true;
const bool DEFORMVECTOR = true;

// adaptivity
const bool ADAPTIVITY_ISOONLY = false;
const double ADAPTIVITY_CONVEXP = 1.0;
const double ADAPTIVITY_THRESHOLD = 0.3;
const int ADAPTIVITY_STRATEGY = 0;
const int ADAPTIVITY_MESHREGULARITY = -1;
const Hermes::Hermes2D::ProjNormType ADAPTIVITY_PROJNORMTYPE = Hermes::Hermes2D::HERMES_H1_NORM;

// command argument
const QString COMMANDS_TRIANGLE = "%1 -p -P -q31.0 -e -A -a -z -Q -I -n -o2 \"%2\"";
const QString COMMANDS_FFMPEG = "%1 -r %2 -y -i \"%3video_%08d.png\" -vcodec %4 \"%5\"";

// max dofs
const int MAX_DOFS = 60e3;

#endif // UTIL_H

