#include <QApplication>

#include <QtGui>
#include <QtCore>

#include "singleapp/qtsingleapplication.h"

#include "util.h"
#include "pythonlab/pythonengine.h"
#include "pythonlab/pythoneditor.h"
#include "pythonlab.h"

int main(int argc, char *argv[])
{
    // register message handler
    // qInstallMsgHandler(logOutput);

    QtSingleApplication a(argc, argv);
    if (a.isRunning())
    {
        QString msg;
        msg = QCoreApplication::arguments().join("#!#");
        msg += "#!#";
        a.sendMessage(msg);

        return 0;
    }

#ifdef VERSION_BETA
    bool beta = true;
#else
    bool beta = false;
#endif

    a.setWindowIcon(icon("pythonlab"));
    a.setApplicationVersion(versionString(VERSION_MAJOR, VERSION_MINOR, VERSION_SUB, VERSION_GIT, VERSION_YEAR, VERSION_MONTH, VERSION_DAY, beta));
    a.setOrganizationName("hpfem.org");
    a.setOrganizationDomain("hpfem.org");
    a.setApplicationName("PythonLab");

#ifdef Q_WS_MAC
    // don't show icons in menu
    a.setAttribute(Qt::AA_DontShowIconsInMenus, true);
#endif
    QSettings settings;

    // first run
    if (settings.value("General/GUIStyle").value<QString>().isEmpty())
    {
        QString styleName = "";
        QStringList styles = QStyleFactory::keys();

#ifdef Q_WS_X11
        // kde 3
        if (getenv("KDE_FULL_SESSION") != NULL)
            styleName = "Plastique";
        // kde 4
        if (getenv("KDE_SESSION_VERSION") != NULL)
        {
            if (styles.contains("Oxygen"))
                styleName = "Oxygen";
            else
                styleName = "Plastique";
        }
        // gtk+
        if (styleName == "")
            styleName = "GTK+";
#endif

#ifdef Q_WS_WIN
        if (styles.contains("WindowsVista"))
            styleName = "WindowsVista";
        else if (styles.contains("WindowsXP"))
            styleName = "WindowsXP";
        else
            styleName = "Windows";
#endif


#ifdef Q_WS_MAC
        styleName = "Aqua";
#endif

        settings.setValue("General/GUIStyle", styleName);
    }

    // setting gui style
    setGUIStyle(settings.value("General/GUIStyle").value<QString>());

    // language
    QString locale = settings.value("General/Language", QLocale::system().name()).value<QString>();
    setLanguage(locale);

    // std::string codec
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    createPythonEngine();
    PythonEngine *pythonEngine = currentPythonEngine();

    PythonEditorDialog w(pythonEngine, QApplication::arguments());

    QObject::connect(&a, SIGNAL(messageReceived(const QString &)),
                     &w, SLOT(onOtherInstanceMessage(const QString &)));
    w.show();

    return a.exec();
}
