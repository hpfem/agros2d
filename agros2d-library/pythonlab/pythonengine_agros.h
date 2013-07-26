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

#ifndef PYTHONENGINEAGROS_H
#define PYTHONENGINEAGROS_H

#include "pythonlab/pythonconsole.h"
#include "pythonlab/pythonengine.h"
#include "pythonlab/pythoneditor.h"

#include "util.h"
#include "util/global.h"
#include "scene.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "sceneview_particle.h"

class Solution;
class SceneViewPreprocessor;
class SceneViewMesh;
class SceneViewPost2D;
class SceneViewPost3D;
class SceneViewParticleTracing;
class PostHermes;

class AGROS_API PythonEngineAgros : public PythonEngine
{
    Q_OBJECT
public:
    PythonEngineAgros() : PythonEngine(),
        m_sceneViewPreprocessor(NULL), m_sceneViewMesh(NULL), m_sceneViewPost2D(NULL), m_sceneViewPost3D(NULL) {}

    inline void setSceneViewGeometry(SceneViewPreprocessor *sceneViewGeometry) { assert(sceneViewGeometry); m_sceneViewPreprocessor = sceneViewGeometry; }
    inline SceneViewPreprocessor *sceneViewPreprocessor() { assert(m_sceneViewPreprocessor); return m_sceneViewPreprocessor; }
    inline void setSceneViewMesh(SceneViewMesh *sceneViewMesh) { assert(sceneViewMesh); m_sceneViewMesh = sceneViewMesh; }
    inline SceneViewMesh *sceneViewMesh() { assert(m_sceneViewMesh); return m_sceneViewMesh; }
    inline void setSceneViewPost2D(SceneViewPost2D *sceneViewPost2D) { assert(sceneViewPost2D); m_sceneViewPost2D = sceneViewPost2D; }
    inline SceneViewPost2D *sceneViewPost2D() {assert(m_sceneViewPost2D); return m_sceneViewPost2D; }
    inline void setSceneViewPost3D(SceneViewPost3D *sceneViewPost3D) { assert(sceneViewPost3D); m_sceneViewPost3D = sceneViewPost3D; }
    inline SceneViewPost3D *sceneViewPost3D() { assert(m_sceneViewPost3D); return m_sceneViewPost3D; }
    inline void setSceneViewParticleTracing(SceneViewParticleTracing *sceneViewParticleTracing) { assert(sceneViewParticleTracing); m_sceneViewParticleTracing = sceneViewParticleTracing; }
    inline SceneViewParticleTracing *sceneViewParticleTracing() { assert(m_sceneViewParticleTracing); return m_sceneViewParticleTracing; }
    inline void setPostHermes(PostHermes *postHermes) { assert(postHermes); m_postHermes = postHermes; }
    inline PostHermes *postHermes() { assert(m_postHermes); return m_postHermes; }

    inline void setConsole(PythonScriptingConsole *console) { m_console = console; }
    inline void resetConsole() { m_console = NULL; }

    void materialValues(const QString &function, double from, double to,
                        QVector<double> *keys, QVector<double> *values, int count = 200);

public slots:
    virtual void abortScript();

protected:
    virtual void addCustomExtensions();
    virtual void addCustomFunctions();
    virtual void runPythonHeader();

private:
    SceneViewPreprocessor *m_sceneViewPreprocessor;
    SceneViewMesh *m_sceneViewMesh;
    SceneViewPost2D *m_sceneViewPost2D;
    SceneViewPost3D *m_sceneViewPost3D;
    SceneViewParticleTracing *m_sceneViewParticleTracing;

    PostHermes *m_postHermes;

    PythonScriptingConsole *m_console;
};

class AGROS_API PythonEditorAgrosDialog : public PythonEditorDialog
{
    Q_OBJECT
public:
    PythonEditorAgrosDialog(PythonEngine *pythonEngine, QStringList args, QWidget *parent);
    ~PythonEditorAgrosDialog();

protected:
    virtual void scriptPrepare();
    virtual void scriptFinish();

private:
    QAction *actCreateFromModel;

    QAction *actStartupScriptVariables;
    QAction *actStartupScriptValues;
    QAction *actConsoleOutput;

private slots:
    void doCreatePythonFromModel();

    void printMessage(const QString &module, const QString &message, bool escaped = true);
    void printError(const QString &module, const QString &message, bool escaped = true);
    void printWarning(const QString &module, const QString &message, bool escaped = true);
    void printDebug(const QString &module, const QString &message, bool escaped = true);
};

// current python engine agros
AGROS_API PythonEngineAgros *currentPythonEngineAgros();

enum StartupScript_Type
{
    StartupScript_Variable,
    StartupScript_Value
};

// create script from model
QString createPythonFromModel(StartupScript_Type startupScript);

// ************************************************************************************

void openFile(const std::string &file);
void saveFile(const std::string &file, bool saveWithSolution);

int appTime();
void memoryUsage(std::vector<int> &time, std::vector<int> &usage);

struct PyOptions
{
    // number of threads
    inline int getNumberOfThreads() const { return Agros2D::configComputer()->numberOfThreads; }
    void setNumberOfThreads(int threads);

    // cache size
    inline int getCacheSize() const { return Agros2D::configComputer()->cacheSize; }
    void setCacheSize(int size);

    // solver cache
    inline bool getSolverCache() const { return Agros2D::configComputer()->useSolverCache; }
    inline void setSolverCache(bool cache) { Agros2D::configComputer()->useSolverCache = cache; }

    // save matrix and rhs
    inline bool getSaveMatrixRHS() const { return Agros2D::configComputer()->saveMatrixRHS; }
    inline void setSaveMatrixRHS(bool save) { Agros2D::configComputer()->saveMatrixRHS = save; }
};

#endif // PYTHONENGINEAGROS_H
