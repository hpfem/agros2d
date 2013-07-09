#include <QApplication>

#include <QTranslator>
#include <QTextCodec>
#include <QDir>
#include <QString>

#include "util.h"
#include "util/global.h"
#include "mainwindow.h"

class AgrosApplication : public QApplication
{
public:
    AgrosApplication(int& argc, char ** argv) : QApplication(argc, argv) {}

    // reimplemented from QApplication so we can throw exceptions in slots
    virtual bool notify(QObject *receiver, QEvent *event)
    {
        try
        {
            return QApplication::notify(receiver, event);
        }
        catch (std::exception& e)
        {
            qCritical() << "Exception thrown: " << e.what();
            throw;
        }
        catch (Hermes::Exceptions::Exception& e)
        {
            qCritical() << "Hermes exception thrown: " << QString("%1").arg(e.what());
            throw;
        }
        catch (AgrosException e)
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
    AgrosApplication a(argc, argv);
    // QApplication a(argc, argv);

#ifdef VERSION_BETA
    bool beta = true;
#else
    bool beta = false;
#endif

    a.setWindowIcon(icon("agros2d"));
    a.setApplicationVersion(versionString(VERSION_MAJOR, VERSION_MINOR, VERSION_SUB, VERSION_GIT, VERSION_YEAR, VERSION_MONTH, VERSION_DAY, beta));
    a.setOrganizationName("hpfem.org");
    a.setOrganizationDomain("hpfem.org");
    a.setApplicationName("Agros2D-3");

#ifdef Q_WS_MAC
    // don't show icons in menu
    a.setAttribute(Qt::AA_DontShowIconsInMenus, true);
#endif

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
#if QT_VERSION < 0x050000
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    // force number format
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    // init indicator (ubuntu - unity, windows - overlay icon, macosx - ???)
    Indicator::init();

    // bson
    /*
    bson br;
    bson_init(&br);
    bson_append_new_oid(&br, "_id");
    bson_append_new_oid(&br, "user_id");

    bson_append_start_array(&br, "items");
    bson_append_start_object(&br, "0");
    bson_append_string(&br, "name", "John Coltrane: Impressions");
    bson_append_int(&br, "price", 1099);
    bson_append_finish_object(&br);

    bson_append_start_object(&br, "1");
    bson_append_string(&br, "name", "Larry Young: Unity");
    bson_append_int(&br, "price", 1199);
    bson_append_finish_object(&br);
    bson_append_finish_object(&br);

    bson_append_start_object(&br, "address");
    bson_append_string(&br, "street", "59 18th St.");
    bson_append_int(&br, "zip", 10010);
    bson_append_finish_object(&br);

    bson_append_int(&br, "total", 2298);

    bson_finish(&br);

    FILE *fpw;
    fpw = fopen("pokus.bson", "wb");
    const char *dataw = (const char *) bson_data(&br);
    fwrite(dataw, bson_size(&br), 1, fpw);
    fclose(fpw);

    bson_destroy(&br);

    FILE *fpr;
    fpr = fopen("pokus.bson", "rb");

    // file size:
    fseek (fpr, 0, SEEK_END);
    int size = ftell(fpr);
    rewind(fpr);

    // allocate memory to contain the whole file:
    char *datar = (char*) malloc (sizeof(char)*size);
    fread(datar, size, 1, fpr);
    fclose(fpr);


    bson bw;
    bson_init_finished_data(&bw, datar, 0);
    bson_print(&bw);
    bson_destroy(&bw);
    */

    MainWindow w;
    w.show();

    return a.exec();

    // remove temp and cache plugins
    removeDirectory(cacheProblemDir());
    removeDirectory(tempProblemDir());
}
