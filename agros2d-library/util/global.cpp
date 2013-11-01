// This plugin is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "global.h"

#include "util/constants.h"

#include "util.h"
#include "logview.h"
#include "memory_monitor.h"
#include "scene.h"

#include "pythonlab/pythonengine_agros.h"
#include "pythonlab/remotecontrol.h"

#include "hermes2d/module.h"

#include "hermes2d/problem.h"
#include "hermes2d/coupling.h"
#include "hermes2d/solutionstore.h"
#include "hermes2d/plugin_interface.h"

#include "util/system_utils.h"
#include "../../util/config.h"

AgrosApplication::AgrosApplication(int& argc, char ** argv) : QApplication(argc, argv)
{
#ifdef VERSION_BETA
    bool beta = true;
#else
    bool beta = false;
#endif

    setWindowIcon(icon("agros2d"));
    setApplicationVersion(versionString(VERSION_MAJOR, VERSION_MINOR, VERSION_SUB, VERSION_GIT, VERSION_YEAR, VERSION_MONTH, VERSION_DAY, beta));
    setOrganizationName("hpfem.org");
    setOrganizationDomain("hpfem.org");
    setApplicationName("Agros2D-3");

#ifdef Q_WS_MAC
    // don't show icons in menu
    setAttribute(Qt::AA_DontShowIconsInMenus, true);
#endif

    // std::string codec
#if QT_VERSION < 0x050000
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
#endif
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    // force number format
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    // init singleton
    Agros2D::createSingleton();
}

// reimplemented from QApplication so we can throw exceptions in slots
bool AgrosApplication::notify(QObject *receiver, QEvent *event)
{
    try
    {
        // if (!receiver->objectName().isEmpty())
        //     qDebug() << "receiver" << receiver->objectName() << event->type();

        return QApplication::notify(receiver, event);
    }
    catch (Hermes::Exceptions::Exception& e)
    {
        qCritical() << "Hermes exception thrown: " << QString("%1").arg(e.what());
        throw;
    }
    catch (std::exception& e)
    {
        qCritical() << "Exception thrown: " << e.what();
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

void AgrosApplication::setLocale()
{
    QSettings settings;

    // language
    QString locale = settings.value("General/Language", QLocale::system().name()).value<QString>();
    setLanguage(locale);
}

void AgrosApplication::setStyle()
{
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

    // init indicator (ubuntu - unity, windows - overlay icon, macosx - ???)
    Indicator::init();
}

// *******************************************************************************************

void clearAgros2DCache()
{
    QFileInfoList listExamples = QFileInfo(cacheProblemDir()).absoluteDir().entryInfoList();
    for (int i = 0; i < listExamples.size(); ++i)
    {
        QFileInfo fileInfo = listExamples.at(i);
        if (fileInfo.fileName() == "." || fileInfo.fileName() == ".." || fileInfo.fileName() == QString::number(QCoreApplication::applicationPid()))
            continue;

        if (fileInfo.isDir())
        {
            // process doesn't exists
            if (!isProcessRunning(fileInfo.fileName().toInt()))
                removeDirectory(QString("%1/%2").arg(QFileInfo(cacheProblemDir()).absolutePath()).arg(fileInfo.fileName()));
        }
    }

}

static QSharedPointer<Agros2D> m_singleton;

Agros2D::Agros2D() : m_scriptEngineRemote(NULL)
{
    clearAgros2DCache();

    m_problem = new Problem();
    m_scene = new Scene();

    // script remote
    m_scriptEngineRemote = new ScriptEngineRemote();

    QObject::connect(m_problem, SIGNAL(fieldsChanged()), m_scene, SLOT(doFieldsChanged()));
    QObject::connect(m_scene, SIGNAL(invalidated()), m_problem, SLOT(clearSolution()));

    initLists();

    m_solutionStore = new SolutionStore();

    m_configComputer = new Config();
    m_configComputer->load();

    // log
    m_log = new Log();

    // memory monitor
    m_memoryMonitor = new MemoryMonitor();
}

void Agros2D::clear()
{
    delete m_singleton.data()->m_scene;
    delete m_singleton.data()->m_problem;
    delete m_singleton.data()->m_configComputer;
    delete m_singleton.data()->m_solutionStore;
    delete m_singleton.data()->m_log;
    if (m_singleton.data()->m_scriptEngineRemote)
        delete m_singleton.data()->m_scriptEngineRemote;
    delete m_singleton.data()->m_memoryMonitor;

    // remove temp and cache plugins
    removeDirectory(cacheProblemDir());
    removeDirectory(tempProblemDir());
}

void Agros2D::createSingleton()
{
    m_singleton = QSharedPointer<Agros2D>(new Agros2D());
}

Agros2D *Agros2D::singleton()
{
    return m_singleton.data();
}

PluginInterface *Agros2D::loadPlugin(const QString &pluginName)
{
    QPluginLoader *loader = NULL;

#ifdef Q_WS_X11
    if (QFile::exists(QString("%1/libs/libagros2d_plugin_%2.so").arg(datadir()).arg(pluginName)))
        loader = new QPluginLoader(QString("%1/libs/libagros2d_plugin_%2.so").arg(datadir()).arg(pluginName));

    if (!loader)
    {
        if (QFile::exists(QString("/usr/local/lib/libagros2d_plugin_%1.so").arg(pluginName)))
            loader = new QPluginLoader(QString("/usr/local/lib/libagros2d_plugin_%1.so").arg(pluginName));
        else if (QFile::exists(QString("/usr/lib/libagros2d_plugin_%1.so").arg(pluginName)))
            loader = new QPluginLoader(QString("/usr/lib/libagros2d_plugin_%1.so").arg(pluginName));
    }
#endif

#ifdef Q_WS_WIN
    if (QFile::exists(QString("%1/libs/agros2d_plugin_%2.dll").arg(datadir()).arg(pluginName)))
        loader = new QPluginLoader(QString("%1/libs/agros2d_plugin_%2.dll").arg(datadir()).arg(pluginName));
#endif

    if (!loader)
    {
        throw AgrosPluginException(QObject::tr("Could not find 'agros2d_plugin_%1'").arg(pluginName));
    }

    if (!loader->load())
    {
        QString error = loader->errorString();
        delete loader;
        throw AgrosPluginException(QObject::tr("Could not load 'agros2d_plugin_%1' (%2)").arg(pluginName).arg(error));
    }

    assert(loader->instance());
    PluginInterface *plugin = qobject_cast<PluginInterface *>(loader->instance());

    // loader->unload();
    delete loader;

    return plugin;
}
