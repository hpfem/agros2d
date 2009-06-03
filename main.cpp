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

    a.setWindowIcon(getIcon("icon"));
    a.setApplicationVersion("1.0.0");
    
    a.setOrganizationName("Hermes");
    a.setOrganizationDomain("hpfem.org");
    a.setApplicationName("Hermes2D");

    QSettings settings;

    // setting gui style
    setGUIStyle(settings.value("General/GUIStyle").value<QString>());
    // setLanguage(settings.value("General/Language", QLocale::system().name()).value<QString>());

    // language
    QString locale = settings.value("General/Language", QLocale::system().name()).value<QString>();
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    QTranslator translator;
    translator.load(QDir::current().absolutePath() + "/lang/" + locale + ".qm");
    QApplication::installTranslator(&translator);

    MainWindow w;
    w.show();
    return a.exec();
}
