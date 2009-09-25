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

#ifdef Q_WS_WIN
    if (QFile::exists(":/images/" + name + "-windows.png"))
        return QIcon(QPixmap(:/images/" + name + "-windows.png));
#endif

#ifdef Q_WS_X11
    QDir dir;

    QString style = "";
    QStringList styles = QStyleFactory::keys();

    // kde 3
    if (getenv("KDE_FULL_SESSION") != NULL)
    {}
    // kde 4
    if (getenv("KDE_SESSION_VERSION") != NULL)
    {
        // oxygen
        fileName = "/usr/share/icons/oxygen/32x32/actions/" + name;
        if (QFile::exists(fileName + ".svg"))
            return QIcon(QPixmap(fileName + ".svg"));
        if (QFile::exists(fileName + ".png"))
            return QIcon(QPixmap(fileName + ".png"));
    }
    // gtk+
    if (style == "")
    {
        // humanity
        fileName = "/usr/share/icons/Humanity/actions/24/" + name;
        if (QFile::exists(fileName + ".svg"))
            return QIcon(QPixmap(fileName + ".svg"));
        if (QFile::exists(fileName + ".png"))
            return QIcon(QPixmap(fileName + ".png"));

        // human
        fileName = "/usr/share/icons/Human/actions/24/" + name;
        if (QFile::exists(fileName + ".svg"))
            return QIcon(QPixmap(fileName + ".svg"));
        if (QFile::exists(fileName + ".png"))
            return QIcon(QPixmap(fileName + ".png"));
    }
#endif

    if (QFile::exists(":images/" + name + ".svg"))
        return QIcon(QPixmap(":images/" + name + ".svg"));
    if (QFile::exists(":images/" + name + ".png"))
        return QIcon(QPixmap(":images/" + name + ".png"));

    return QIcon();
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
        QFile file(datadir() + "/functions.qs");
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

QString tempProblemDir()
{
    QDir(QDir::temp().absolutePath()).mkpath("agros2d/" + QString::number(QApplication::applicationPid()));

    return QString("%1/agros2d/%2").arg(QDir::temp().absolutePath()).arg(QApplication::applicationPid());
}

QString tempProblemFileName()
{
    return tempProblemDir() + "/temp";
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

bool removeDirectory(const QDir &dir)
{
    bool error = false;

    if (dir.exists())
    {
        QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
        int count = entries.size();
        for (int idx = 0; idx < count; idx++)
        {
            QFileInfo entryInfo = entries[idx];
            QString path = entryInfo.absoluteFilePath();
            if (entryInfo.isDir())
            {
                error = removeDirectory(QDir(path));
            }
            else
            {
                QFile file(path);
                if (!file.remove())
                {
                    error = true;
                    break;
                }
            }
        }
        if (!dir.rmdir(dir.absolutePath()))
            error = true;
    }

    return error;
}
