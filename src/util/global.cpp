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
#include "scene.h"

#include "pythonlab/pythonengine_agros.h"

#include "hermes2d/module.h"

#include "hermes2d/problem.h"
#include "hermes2d/coupling.h"
#include "hermes2d/solutionstore.h"
#include "hermes2d/plugin_interface.h"

static QSharedPointer<Agros2D> m_singleton;

Agros2D::Agros2D()
{
    m_problem = new Problem();
    m_scene = new Scene();
    QObject::connect(m_problem, SIGNAL(fieldsChanged()), m_scene, SLOT(doFieldsChanged()));
    QObject::connect(m_scene, SIGNAL(invalidated()), m_problem, SLOT(clearSolution()));

    initLists();

    m_solutionStore = new SolutionStore();

    m_configComputer = new Config();
    m_configComputer->load();

    // log
    m_log = new Log();
}

void Agros2D::clear()
{
    // remove temp and cache plugins
    removeDirectory(cacheProblemDir());
    removeDirectory(tempProblemDir());

    delete m_singleton.data()->m_scene;
    delete m_singleton.data()->m_problem;
    delete m_singleton.data()->m_configComputer;
    delete m_singleton.data()->m_solutionStore;
    delete m_singleton.data()->m_log;
}

void Agros2D::createSingleton()
{
    m_singleton = QSharedPointer<Agros2D>(new Agros2D());
}

Agros2D *Agros2D::singleton()
{
    return m_singleton.data();
}

PluginInterface *Agros2D::loadPlugin(const QString &plugin)
{
    QPluginLoader *loader = NULL;

#ifdef Q_WS_X11
    if (QFile::exists(QString("%1/libs/libagros2d_plugin_%2.so").arg(datadir()).arg(plugin)))
        loader = new QPluginLoader(QString("%1/libs/libagros2d_plugin_%2.so").arg(datadir()).arg(plugin));

    if (!loader)
    {
        if (QFile::exists(QString("/usr/local/lib/libagros2d_plugin_%1.so").arg(plugin)))
            loader = new QPluginLoader(QString("/usr/local/lib/libagros2d_plugin_%1.so").arg(plugin));
        else if (QFile::exists(QString("/usr/lib/libagros2d_plugin_%1.so").arg(plugin)))
            loader = new QPluginLoader(QString("/usr/lib/libagros2d_plugin_%1.so").arg(plugin));
    }
#endif

#ifdef Q_WS_WIN
    if (QFile::exists(QString("%1/libs/agros2d_plugin_%2.dll").arg(datadir()).arg(plugin)))
        loader = new QPluginLoader(QString("%1/libs/agros2d_plugin_%2.dll").arg(datadir()).arg(plugin));
#endif

    if (!loader)
    {
        throw AgrosPluginException(QObject::tr("Could not find 'agros2d_plugin_%1'").arg(plugin));
    }

    if (!loader->load())
    {
        delete loader;
        throw AgrosPluginException(QObject::tr("Could not load 'agros2d_plugin_%1'").arg(plugin));
    }

    assert(loader->instance());
    PluginInterface *plugion = qobject_cast<PluginInterface *>(loader->instance());
    delete loader;
    return plugion;
}
