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

#include <QApplication>

#include "util.h"
#include "util/conf.h"
#include "util/global.h"
#include "util/system_utils.h"
#include "mainwindow.h"

#include "../3rdparty/tclap/CmdLine.h"

int main(int argc, char *argv[])
{
    try
    {
        // command line info
        TCLAP::CmdLine cmd("Agros2D", ' ', versionString().toStdString());

        TCLAP::SwitchArg remoteArg("r", "remote-server", "Run remote server", false);
        TCLAP::ValueArg<std::string> problemArg("p", "problem", "Open problem", false, "", "string");
        TCLAP::ValueArg<std::string> scriptArg("s", "script", "Open script", false, "", "string");
        TCLAP::SwitchArg executeArg("x", "execute", "Execute problem or script", false);

        cmd.add(remoteArg);
        cmd.add(problemArg);
        cmd.add(scriptArg);
        cmd.add(executeArg);

        // parse the argv array.
        cmd.parse(argc, argv);

        CleanExit cleanExit;
        AgrosApplication a(argc, argv);
        // setting gui style
        setGUIStyle(Agros2D::configComputer()->value(Config::Config_GUIStyle).toString());
        // language
        setLocale(Agros2D::configComputer()->value(Config::Config_Locale).toString());
        a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

        // init indicator (ubuntu - unity, windows - overlay icon, macosx - ???)
        Indicator::init();

        MainWindow w;

        // run remote server
        if (remoteArg.getValue())
        {
            a.runRemoteServer();
        }

        if (!problemArg.getValue().empty())
        {
            if (QFile::exists(QString::fromStdString(problemArg.getValue())))
            {
                QFileInfo info(QString::fromStdString(problemArg.getValue()));
                if (info.suffix() == "a2d")
                {
                    w.setStartupProblemFilename(QString::fromStdString(problemArg.getValue()));

                    if (executeArg.getValue())
                        w.setStartupExecute(true);
                }
                else
                {
                    std::cout << QObject::tr("Unknown suffix.").toStdString() << std::endl;
                }
            }
        }
        else if (!scriptArg.getValue().empty())
        {
            if (QFile::exists(QString::fromStdString(scriptArg.getValue())))
            {
                QFileInfo info(QString::fromStdString(scriptArg.getValue()));
                if (info.suffix() == "py")
                {
                    w.setStartupScriptFilename(QString::fromStdString(scriptArg.getValue()));

                    if (executeArg.getValue())
                        w.setStartupExecute(true);
                }
                else
                {
                    std::cout << QObject::tr("Unknown suffix.").toStdString() << std::endl;
                }
            }
        }

        w.show();

        return a.exec();
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }    
}
