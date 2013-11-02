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

#include "QCoreApplication"

#include "client.h"
#include "util.h"
#include "util/system_utils.h"
#include "../3rdparty/tclap/CmdLine.h"

int main(int argc, char *argv[])
{
    try
    {
        // command line info
        TCLAP::CmdLine cmd("Agros2D remote control", ' ', versionString().toStdString());

        TCLAP::ValueArg<std::string> ipArg("i", "ip", "IP address", true, "127.0.0.1", "string");
        TCLAP::ValueArg<int> portArg("p", "port", "Port", true, 14000, "int");
        TCLAP::ValueArg<std::string> commandArg("c", "command", "Run command", false, "", "string");
        TCLAP::ValueArg<std::string> scriptArg("s", "script", "Script filename", false, "", "string");

        cmd.add(ipArg);
        cmd.add(portArg);
        cmd.add(commandArg);
        cmd.add(scriptArg);

        // parse the argv array.
        cmd.parse(argc, argv);

        CleanExit cleanExit;
        QCoreApplication a(argc, argv);
        Client *client = new Client(QString::fromStdString(ipArg.getValue()), portArg.getValue());

        if (!commandArg.getValue().empty())
        {
            // run command
            client->run(QString::fromStdString(commandArg.getValue()));
            return a.exec();
        }
        else if (!scriptArg.getValue().empty())
        {
            QString script;

            QFile file(QString::fromStdString(scriptArg.getValue()));
            if (file.open(QFile::ReadOnly | QFile::Text))
            {
                script = file.readAll();
            }
            else
            {
                qDebug() << file.errorString();
            }
            file.close();

            // run script
            if (!script.isEmpty())
            {
                client->run(script);
                return a.exec();
            }
            else
            {
                return 1;
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
