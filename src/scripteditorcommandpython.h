#ifndef SCRIPTEDITORCOMMANDPYTHON_H
#define SCRIPTEDITORCOMMANDPYTHON_H

#include "util.h"
#include "sceneview.h"

class PythonEngine
{
public:
    PythonEngine();
    ~PythonEngine();

    void setSceneView(SceneView *sceneView);

    ScriptResult runPython(const QString &script, bool isExpression = false, const QString &fileName = "");

private:
    PyObject *m_dict;
    QString m_functions;
    SceneView *m_sceneView;
};

// cython functions

void pythonMessage(char *str);

void pythonAddNode(double x, double y);

#endif // SCRIPTEDITORCOMMANDPYTHON_H
