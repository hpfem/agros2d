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
#include <QtPlugin>
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets>
#include <QtWebKitWidgets>
#include <QtPrintSupport>
#include <QtOpenGL>
#endif

#include <typeinfo>
#include <iostream>
#include <cmath>
#include <limits>
#include <vector>

#include <locale.h>
#include <stdlib.h>

// qt5
#ifdef Q_OS_WIN
#define Q_WS_WIN
#endif

#ifdef Q_OS_MAC
#define Q_WS_MAC
#endif

#ifdef Q_OS_LINUX
#define Q_WS_X11
#endif

// Windows DLL export/import definitions
#ifdef Q_WS_WIN
// DLL build
#ifdef AGROS_DLL
#define AGROS_API __declspec(dllexport)
// DLL usage
#else
#define AGROS_API __declspec(dllimport)
#endif
#else
#define AGROS_API
#endif

#include "util/point.h"
#include "indicators/indicators.h"

// zero
#define EPS_ZERO 1e-10

// physical constants
#define EPS0 8.854e-12
#define MU0 4*M_PI*1e-7
#define SIGMA0 5.670373e-8
#define PRESSURE_MIN_AIR 20e-6
#define GRAVITATIONAL_ACCELERATION 9.81

#define deg2rad(degrees) (degrees*M_PI/180.0)
#define rad2deg(radians) (radians*180.0/M_PI)

using namespace std;

AGROS_API bool almostEqualRelAndAbs(double A, double B, double maxDiff, double maxRelDiff);

// approximation of atan2(y, x).
// maximum error of 0.0061 radians at 0.35 degrees
AGROS_API double fastatan2(double y, double x);
AGROS_API double fastsin(double angle);
AGROS_API double fastcos(double angle);

AGROS_API QString stringListToString(const QStringList &list);

// show page
AGROS_API void showPage(const QString &str = "");

// set gui style
AGROS_API void setGUIStyle(const QString &styleName);

// set language
AGROS_API void setLanguage(const QString &locale);

// get available languages
AGROS_API QStringList availableLanguages();

// get icon with respect to actual theme
AGROS_API QIcon icon(const QString &name);

// windows short name
AGROS_API QString compatibleFilename(const QString &fileName);

// get datadir
AGROS_API QString datadir();

// get external js functions
AGROS_API QString externalFunctions();

// get temp dir
AGROS_API QString tempProblemDir();
AGROS_API QString cacheProblemDir();

// get user dir
AGROS_API QString userDataDir();

// get temp filename
AGROS_API QString tempProblemFileName();

// convert time in ms to QTime
AGROS_API QTime milisecondsToTime(int ms);

// remove directory content
AGROS_API bool removeDirectory(const QString &str);

// sleep function
AGROS_API void msleep(unsigned long msecs);

// read file content
AGROS_API QByteArray readFileContentByteArray(const QString &fileName);
AGROS_API QString readFileContent(const QString &fileName);

// write content into the file
AGROS_API void writeStringContent(const QString &fileName, QString content);
AGROS_API void writeStringContent(const QString &fileName, QString *content);
AGROS_API void writeStringContentByteArray(const QString &fileName, QByteArray content);

// append to the file
AGROS_API void appendToFile(const QString &fileName, const QString &str);

// join version
AGROS_API inline QString versionString(int major, int minor, int sub, int git, int year, int month, int day, bool beta)
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

class AgrosException
{
public:
    AgrosException(const QString &what)
        : m_what(what)
    {
    }

    inline QString what() { return m_what; }
    inline QString toString(){ return m_what; }

protected:
    QString m_what;
};

class AgrosSolverException : public AgrosException
{
public:
    AgrosSolverException(const QString &what) : AgrosException(what)
    {
    }
};

class AgrosGeometryException : public AgrosException
{
public:
    AgrosGeometryException(const QString &what) : AgrosException(what)
    {
    }
};

class AgrosMeshException : public AgrosException
{
public:
    AgrosMeshException(const QString &what) : AgrosException(what)
    {
    }
};

class AgrosPluginException : public AgrosException
{
public:
    AgrosPluginException(const QString &what) : AgrosException(what)
    {
    }
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

#endif // UTIL_H
