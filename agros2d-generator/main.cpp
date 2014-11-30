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

#include "util.h"
#include "generator.h"

#include "../3rdparty/tclap/CmdLine.h"

int main(int argc, char *argv[])
{
    try
    {
        // command line info
        TCLAP::CmdLine cmd("Agros2D solver", ' ', versionString().toStdString());

        TCLAP::ValueArg<std::string> moduleArg("m", "module", "Generate module", false, "", "string");

        cmd.add(moduleArg);

        // parse the argv array.
        cmd.parse(argc, argv);

        Agros2DGenerator a(argc, argv);

        // init lists
        initLists();

        QTimer::singleShot(0, &a, SLOT(run()));
        a.setModuleName(QString::fromStdString(moduleArg.getValue()));
        return a.exec();
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}
