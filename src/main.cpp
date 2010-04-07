#include <QtGui/QApplication>

#include <QTranslator>
#include <QTextCodec>
#include <QDir>
#include <QString>

#include "util.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef VERSION_BETA
    bool beta = true;
#else
    bool beta = false;
#endif

    a.setWindowIcon(icon("agros2d"));
    a.setApplicationVersion(versionString(VERSION_MAJOR, VERSION_MINOR, VERSION_SUB, VERSION_GIT, VERSION_YEAR, VERSION_MONTH, VERSION_DAY, beta));
    a.setOrganizationName("hpfem.org");
    a.setOrganizationDomain("hpfem.org");
    a.setApplicationName("Agros2D");

#ifdef Q_WS_MAC
    // don't show icons in menu
    a.setAttribute(Qt::AA_DontShowIconsInMenus, true);
#endif

    QSettings settings;
    enableLogFile(settings.value("General/EnableLogFile", false).value<bool>());

    // first run
    if (settings.value("General/GUIStyle").value<QString>().isEmpty())
    {
        QString style = "";
        QStringList styles = QStyleFactory::keys();

#ifdef Q_WS_X11
        // kde 3
        if (getenv("KDE_FULL_SESSION") != NULL)
            style = "Plastique";
        // kde 4
        if (getenv("KDE_SESSION_VERSION") != NULL)
        {
            if (styles.contains("Oxygen"))
                style = "Oxygen";
            else
                style = "Plastique";
        }
        // gtk+
        if (style == "")
            style = "GTK+";
#endif

#ifdef Q_WS_WIN
        if (styles.contains("WindowsVista"))
            style = "WindowsVista";
        else if (styles.contains("WindowsXP"))
            style = "WindowsXP";
        else
            style = "Windows";
#endif


#ifdef Q_WS_MAC
    style = "Aqua";
#endif

        settings.setValue("General/GUIStyle", style);
    }

    // setting gui style
    setGUIStyle(settings.value("General/GUIStyle").value<QString>());

    // language
    QString locale = settings.value("General/Language", QLocale::system().name()).value<QString>();
    setLanguage(locale);

    // parameters
    QStringList args = QCoreApplication::arguments();
    if (args.count() == 2)
    {
        if ((args[1] == "--help") || (args[1] == "/help"))
        {
            qWarning() << "agros2d [fileName (*.a2d; *.py) | -run fileName (*.py) | --help]";
            a.exit(0);
            return 0;
        }
    }

    MainWindow w;
    w.show();

    return a.exec();
}
