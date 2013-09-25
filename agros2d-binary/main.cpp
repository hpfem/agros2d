#include <QApplication>

#include "util.h"
#include "util/global.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    AgrosApplication a(argc, argv);
    a.setStyle();
    a.setLocale();

    // parameters
    QStringList args = QCoreApplication::arguments();
    if (args.count() == 2)
    {
        if (args.contains( "--help") || args.contains("/help"))
        {
            cout << "agros2d [fileName (*.a2d; *.py) | -run fileName (*.py) | --help | --verbose]" << endl;
            exit(0);
            return 0;
        }
    }

    MainWindow w;
    w.show();

    return a.exec();
}
