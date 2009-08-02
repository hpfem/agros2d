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

    a.setWindowIcon(icon("agros2d"));
    a.setApplicationVersion("0.9.2 (2009-08-02)");
    a.setOrganizationName("hpfem.org");
    a.setOrganizationDomain("hpfem.org");
    a.setApplicationName("Agros2D");

    QSettings settings;

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
            if (styles.contains("WindowsXP"))
                style = "WindowsXP";
            else
                style = "Windows";
        #endif

        settings.setValue("General/GUIStyle", style);
    }

    // setting gui style
    setGUIStyle(settings.value("General/GUIStyle").value<QString>());

    // language
    QString locale = settings.value("General/Language", QLocale::system().name()).value<QString>();
    setLanguage(locale);

    MainWindow w;
    w.show();
    return a.exec();
}
