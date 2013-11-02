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

#include <QTranslator>
#include <QTextCodec>
#include <QDir>
#include <QString>

#include "agros_solver.h"
#include "util/system_utils.h"

#include "../3rdparty/tclap/CmdLine.h"

int main(int argc, char *argv[])
{
    try
    {
        // command line info
        TCLAP::CmdLine cmd("Agros2D solver", ' ', versionString().toStdString());

        TCLAP::SwitchArg logArg("l", "enable-log", "Enable log", false);
        TCLAP::SwitchArg remoteArg("r", "remote-server", "Run remote server", false);
        TCLAP::ValueArg<std::string> problemArg("p", "problem", "Solve problem", false, "", "string");
        TCLAP::ValueArg<std::string> scriptArg("s", "script", "Solve script", false, "", "string");

        cmd.add(logArg);
        cmd.add(remoteArg);
        cmd.add(problemArg);
        cmd.add(scriptArg);

        // parse the argv array.
        cmd.parse(argc, argv);

        CleanExit cleanExit;
        AgrosSolver a(argc, argv);

        // enable log
        a.setEnableLog(logArg.getValue());

        // run remote server
        if (remoteArg.getValue())
        {
            a.runRemoteServer();
            return a.exec();
        }

        if (!problemArg.getValue().empty())
        {
            if (QFile::exists(QString::fromStdString(problemArg.getValue())))
            {
                QFileInfo info(QString::fromStdString(problemArg.getValue()));
                if (info.suffix() == "a2d")
                {
                    a.setFileName(QString::fromStdString(problemArg.getValue()));
                    QTimer::singleShot(0, &a, SLOT(solveProblem()));
                    return a.exec();
                }
                else
                {
                    std::cout << QObject::tr("Unknown suffix.").toStdString() << std::endl;
                    return 1;
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
                    a.setFileName(QString::fromStdString(scriptArg.getValue()));
                    QTimer::singleShot(0, &a, SLOT(runScript()));
                    return a.exec();
                }
                else
                {
                    std::cout << QObject::tr("Unknown suffix.").toStdString() << std::endl;
                    return 1;
                }
            }
        }
        else
        {
            return 1;
        }
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}
