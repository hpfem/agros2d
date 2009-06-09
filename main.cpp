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

    a.setWindowIcon(icon("carbon2d"));  
    a.setApplicationVersion("0.6.0 (2009-06-09)");
    a.setOrganizationName("Carbon");
    a.setOrganizationDomain("hpfem.org");
    a.setApplicationName("Carbon2D");

    QSettings settings;

    // setting gui style
    setGUIStyle(settings.value("General/GUIStyle").value<QString>());

    // language
    QString locale = settings.value("General/Language", QLocale::system().name()).value<QString>();
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    cout << locale.toStdString() << endl;

    QTranslator translator;
    if (QFile::exists("/usr/share/carbon2d/lang/" + locale + ".qm"))
        translator.load("/usr/share/carbon2d/lang/" + locale + ".qm");
    if (QFile::exists(QDir::current().absolutePath() + "/lang/" + locale + ".qm"))
        translator.load(QDir::current().absolutePath() + "/lang/" + locale + ".qm");
    QApplication::installTranslator(&translator);

    MainWindow w;
    w.show();
    return a.exec();
}
