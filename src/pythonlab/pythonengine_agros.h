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
#include "scene.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"

class Solution;
class SceneViewPreprocessor;
class SceneViewMesh;
class SceneViewPost2D;
class SceneViewPost3D;
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
    inline void setPostHermes(PostHermes *postHermes) { assert(postHermes); m_postHermes = postHermes; }
    inline PostHermes *postHermes() { assert(m_postHermes); return m_postHermes; }

protected:
    virtual void addCustomExtensions();
    virtual void addCustomFunctions();
    virtual void runPythonHeader();

private:
    SceneViewPreprocessor *m_sceneViewPreprocessor;
    SceneViewMesh *m_sceneViewMesh;
    SceneViewPost2D *m_sceneViewPost2D;
    SceneViewPost3D *m_sceneViewPost3D;

    PostHermes *m_postHermes;
};

class AGROS_API PythonLabAgros : public PythonEditorDialog
{
    Q_OBJECT
public:
    PythonLabAgros(PythonEngine *pythonEngine, QStringList args, QWidget *parent);

private:
    QAction *actCreateFromModel;

private slots:
    void doCreatePythonFromModel();
};

// current python engine agros
AGROS_API PythonEngineAgros *currentPythonEngineAgros();

// create script from model
QString createPythonFromModel();

// ************************************************************************************

// functions
void pyOpenDocument(char *str);
void pySaveDocument(char *str);
void pyCloseDocument();

#endif // PYTHONENGINEAGROS_H
