// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#include "pythonlab.h"

PythonLabApplication::PythonLabApplication(int& argc, char ** argv) : QtSingleApplication(argc, argv)
{
    setWindowIcon(icon("pythonlab"));
    setApplicationVersion(versionString());
    setOrganizationName("agros");
    setOrganizationDomain("agros");
    setApplicationName("PythonLab");

#ifdef Q_WS_MAC
    // don't show icons in menu
    setAttribute(Qt::AA_DontShowIconsInMenus, true);
#endif

    // std::string codec
#if QT_VERSION < 0x050000
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    // force number format
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
}

PythonLab::PythonLab(QWidget *parent)
    : QMainWindow(parent)
{
}

PythonLab::~PythonLab()
{
    
}
