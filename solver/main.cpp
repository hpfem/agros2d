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

#include <QtGui/QApplication>

#include <QTranslator>
#include <QTextCodec>
#include <QDir>
#include <QString>

#include "agros_solver.h"

const QString CONSOLE_DISABLE_LOG = "disable-log";
const QString CONSOLE_HELP = "help";

int main(int argc, char *argv[])
{
    ArgosSolver a(argc, argv);

#ifdef VERSION_BETA
    bool beta = true;
#else
    bool beta = false;
#endif

    a.setApplicationVersion(versionString(VERSION_MAJOR, VERSION_MINOR, VERSION_SUB, VERSION_GIT, VERSION_YEAR, VERSION_MONTH, VERSION_DAY, beta));
    a.setOrganizationName("hpfem.org");
    a.setOrganizationDomain("hpfem.org");
    a.setApplicationName("Agros2D-3");

    QSettings settings;

    // language
    QString locale = settings.value("General/Language", QLocale::system().name()).value<QString>();
    setLanguage(locale);

    // parameters
    QStringList args = QCoreApplication::arguments();
    if ((args.count() == 2) && (args.contains( "--" + CONSOLE_HELP) || args.contains("/" + CONSOLE_HELP)))
    {
        cout << "agros2d_solver fileName (*.a2d; *.py) | --help [--disable-log]" << endl;
        exit(0);
        return 0;
    }
    if (args.count() > 1)
    {
        // disable log
        a.setEnableLog(!(args.contains("--" + CONSOLE_DISABLE_LOG) || args.contains("/" + CONSOLE_DISABLE_LOG)));

        QFileInfo info(args[1]);

        if (info.exists())
        {
            a.setFileName(args[1]);

            if (info.suffix() == "a2d")
            {
                QTimer::singleShot(0, &a, SLOT(solveProblem()));
            }
            else if (info.suffix() == "py")
            {
                QTimer::singleShot(0, &a, SLOT(runScript()));
            }
            else
            {
                std::cout << QObject::tr("Unknown suffix.").toStdString() << std::endl;

                return false;
            }

            return a.exec();
        }
        else
        {
            std::cout << QObject::tr("File '%1' not found.").arg(args[1]).toStdString() << std::endl;
        }
    }

    return false;
}
