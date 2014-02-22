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
#include "util/system_utils.h"
#include "optilab/optilab.h"

#include "../3rdparty/tclap/CmdLine.h"

class AGROS_LIBRARY_API OptilabApplication : public QApplication
{
public:
    OptilabApplication(int& argc, char ** argv) : QApplication(argc, argv)
    {
        setWindowIcon(icon("optilab"));
        setApplicationVersion(versionString());
        setOrganizationName("hpfem.org");
        setOrganizationDomain("hpfem.org");
        setApplicationName("Optilab");

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

    ~OptilabApplication()
    {

    }

    virtual bool notify(QObject *receiver, QEvent *event)
    {

        try
        {
            // if (!receiver->objectName().isEmpty())
            //     qDebug() << "receiver" << receiver->objectName() << event->type();

            return QApplication::notify(receiver, event);
        }
        catch (std::exception& e)
        {
            qCritical() << "Exception thrown: " << e.what();
            throw;
        }
        catch (...)
        {
            qCritical() << "Unknown exception thrown";
            throw;
        }

        return false;
    }
};

int main(int argc, char *argv[])
{
    try
    {
        // command line info
        TCLAP::CmdLine cmd("Optilab", ' ');

        TCLAP::ValueArg<std::string> dummyArg("d", "dummy", "Dummy", true, "", "string");

        // cmd.add(dummyArg);

        // parse the argv array.
        // cmd.parse(argc, argv);

        CleanExit cleanExit;
        OptilabApplication a(argc, argv);
        // setting gui style
        // setGUIStyle(Agros2D::configComputer()->value(Config::Config_GUIStyle).toString());
        // language
        // setLocale(Agros2D::configComputer()->value(Config::Config_Locale).toString());
        a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

        // init indicator (ubuntu - unity, windows - overlay icon, macosx - ???)
        Indicator::init();

        OptilabWindow w;
        w.show();

        return a.exec();
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}
