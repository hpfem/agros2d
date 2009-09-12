#include "util.h"
#include "scene.h"

void setGUIStyle(const QString &styleName)
{
    QStyle *style = QStyleFactory::create(styleName);

    QApplication::setStyle(style);
    if (QApplication::desktopSettingsAware()) {
        QApplication::setPalette(QApplication::palette());
    }
}

void setLanguage(const QString &locale)
{
    // non latin-1 chars
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    QTranslator *translator = new QTranslator();
    translator->load(datadir() + "/lang/" + locale + ".qm");
    QApplication::installTranslator(translator);
}

QStringList availableLanguages()
{
    QDir dir;
    dir.setPath(datadir() + "/lang");

    QStringList filters;
    filters << "*.qm";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    QStringList list = dir.entryList();
    list.replaceInStrings(".qm", "");

    return list;
}

QIcon icon(const QString &name)
{
    QString fileName;

    fileName = name;
#ifdef Q_WS_WIN
    if (QFile::exists(":/images/" + name + "-windows.png"))
        fileName = name + "-windows";
#endif

    return QIcon(QPixmap(":images/" + fileName + ".png"));
}

QString datadir()
{
    QDir dirData;
    dirData.setPath(QDir::current().absolutePath() + "/data");
    if (dirData.exists())
        return QDir::current().absolutePath();

    dirData.setPath(QApplication::applicationDirPath() + "/../share/agros2d/data");
    if (dirData.exists())
        return QApplication::applicationDirPath() + "/../share/agros2d";

    cerr << "Datadir not found." << endl;
    exit(1);
}

QString externalFunctions()
{
    static QString m_externalFunctions;

    if (m_externalFunctions.isEmpty())
    {
        QFile file(datadir() + "/functions.js");
        if (!file.open(QIODevice::ReadOnly))
        {
            m_externalFunctions = "";
        }
        else
        {
            QTextStream inFile(&file);
            m_externalFunctions = inFile.readAll();

            file.close();
        }
    }

    return m_externalFunctions;
}

QScriptEngine *scriptEngine()
{
    QScriptEngine *engine = new QScriptEngine();
    engine->evaluate(externalFunctions());

    return engine;
}

QString tempProjectFileName()
{
    return QString("%1/agros2d/temp_%2").arg(QDir::temp().absolutePath()).arg(QApplication::applicationPid());
}

QTime milliSecondsToTime(int ms)
{
     // store the current ms remaining
    int tmp_ms = ms;

    // the amount of days left
    int days = floor(tmp_ms/86400000);
    // adjust tmp_ms to leave remaining hours, minutes, seconds
    tmp_ms = tmp_ms - (days * 86400000);

    // calculate the amount of hours remaining
    int hours = floor(tmp_ms/3600000);
    // adjust tmp_ms to leave the remaining minutes and seconds
    tmp_ms = tmp_ms - (hours * 3600000);

    // the amount of minutes remaining
    int mins = floor(tmp_ms/60000);
    //adjust tmp_ms to leave only the remaining seconds
    tmp_ms = tmp_ms - (mins * 60000);

    // seconds remaining
    int secs = floor(tmp_ms/1000);

    // milliseconds remaining
    tmp_ms = tmp_ms - (secs * 1000);

    return QTime(hours, mins, secs, tmp_ms);
}
