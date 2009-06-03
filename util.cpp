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
    QDir dir(QDir::current().absolutePath() + "/lang");

    QStringList filters;
    filters << "*.qm";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    QStringList list = dir.entryList();
    list.replaceInStrings(".qm", "");

    return list;
}

QIcon getIcon(const QString &name) {
    QString fileName;
    #ifdef Q_WS_WIN
    if (QFile::exists(QDir::current().absolutePath() + "/images/windows/" + name + ".png"))
        fileName = QDir::current().absolutePath() + "/images/windows/";
    else
        fileName = QDir::current().absolutePath() + "/images/";
    #else
        fileName = QDir::current().absolutePath() + "/images/";
    #endif

    return QIcon(QPixmap(fileName + name + ".png"));
}
