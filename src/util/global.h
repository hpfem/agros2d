// This file is part of Agros2D.
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

#ifndef GLOBAL_H
#define GLOBAL_H

#include "util.h"
#include "util/enums.h"

class Problem;
class ProblemConfigView;
class Config;
class SolutionStore;
class Log;
class Scene;
class ScriptEngineRemote;
class PluginInterface;

class Agros2D
{
public:
    static void createSingleton();
    static Agros2D* singleton();
    static inline Scene *scene() { return Agros2D::singleton()->m_scene; }
    static inline Config *configComputer() { return Agros2D::singleton()->m_configComputer; }
    static inline Problem *problem() { return Agros2D::singleton()->m_problem; }
    static inline SolutionStore *solutionStore() { return Agros2D::singleton()->m_solutionStore; }
    static inline Log *log() { return Agros2D::singleton()->m_log; }

    static void loadPlugins(QStringList plugins);
    static void loadActivePlugins();
    static QMap<QString, PluginInterface *> plugins() { return Agros2D::singleton()->m_plugins; }

    Agros2D(const Agros2D &);
    Agros2D & operator = (const Agros2D &);
    Agros2D();
    ~Agros2D();

private:
    Scene *m_scene;
    Config *m_configComputer;
    Problem *m_problem;
    SolutionStore *m_solutionStore;
    ScriptEngineRemote *m_scriptEngineRemote;
    Log *m_log;

    QMap<QString, PluginInterface *> m_plugins;
};

#endif /* GLOBAL_H */
