#include "util.h"

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

    QTranslator translator;
    translator.load(QDir::current().absolutePath() + "/lang/" + locale + ".qm");
    QApplication::installTranslator(&translator);
}

QStringList availableLanguages()
{
    QDir dir;
    dir.setPath(appdir() + "/lang");

    QStringList filters;
    filters << "*.qm";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    QStringList list = dir.entryList();
    list.replaceInStrings(".qm", "");

    return list;
}

QIcon icon(const QString &name) {
    QString fileName;

    fileName = name;
    #ifdef Q_WS_WIN
    if (QFile::exists(":/images/" + name + "-windows.png"))
        fileName = name + "-windows";
    #endif

    return QIcon(QPixmap(":images/" + fileName + ".png"));
}

QString appdir()
{
    QDir dirData;
    dirData.setPath(QDir::current().absolutePath() + "/data");
    if (dirData.exists())
        return QDir::current().absolutePath();

    dirData.setPath("/usr/share/carbon2d/data");
    if (dirData.exists())
        return "/usr/share/carbon2d";
}
