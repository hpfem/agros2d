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

#ifndef SCRIPTEDITORCOMMANDPYTHON_H
#define SCRIPTEDITORCOMMANDPYTHON_H

#include "util.h"
#include "sceneview.h"

class PythonEngine : public QObject
{
    Q_OBJECT

signals:
    void printStdout(const QString &);

public:
    PythonEngine();
    ~PythonEngine();

    void setSceneView(SceneView *sceneView);
    void showMessage(const QString &message);

    ScriptResult runPythonScript(const QString &script, const QString &fileName);
    ExpressionResult runPythonExpression(const QString &expression);
    QString parseError();

private slots:
    void doPrintStdout(const QString &message);

private:
    QString m_stdOut;

    PyObject *m_dict;
    QString m_functions;
    SceneView *m_sceneView;

    void runPythonHeader();
};

// cython functions
void pythonMessage(char *str);
char *pythonInput(char *str);
char *pythonVersion();
char *pythonMeshFileName();
char *pythonSolutionFileName();
Solution *pythonSolutionObject();
void pythonQuit();

void pythonNewDocument(char *name, char *type, char *physicfield,
                       int numberofrefinements, int polynomialorder, char *adaptivitytype,
                       double adaptivitysteps, double adaptivitytolerance,
                       double frequency,
                       char *analysistype, double timestep, double totaltime, double initialcondition);
void pythonOpenDocument(char *str);
void pythonSaveDocument(char *str);

void pythonAddNode(double x, double y);
void pythonAddEdge(double x1, double y1, double x2, double y2, double angle, char *marker);
void pythonAddLabel(double x, double y, double area, char *marker);

void pythonSelectNone();
void pythonSelectAll();

void pythonSelectNodePoint(double x, double y);
void pythonSelectEdgePoint(double x, double y);
void pythonSelectLabelPoint(double x, double y);

void pythonRotateSelection(double x, double y, double angle, bool copy);
void pythonScaleSelection(double x, double y, double scale, bool copy);
void pythonMoveSelection(double dx, double dy, bool copy);
void pythonDeleteSelection();

void pythonMesh();
void pythonSolve();

void pythonZoomBestFit();
void pythonZoomIn();
void pythonZoomOut();
void pythonZoomRegion(double x1, double y1, double x2, double y2);

void pythonMode(char *str);
void pythonPostprocessorMode(char *str);

void pythonShowScalar(char *type, char *variable, char *component, int rangemin, int rangemax);
void pythonShowGrid(bool show);
void pythonShowGeometry(bool show);
void pythonShowInitialMesh(bool show);
void pythonShowSolutionMesh(bool show);
void pythonShowContours(bool show);
void pythonShowVectors(bool show);

void pythonSetTimeStep(int timestep);
int pythonTimeStepCount();

void pythonSaveImage(char *str, int w, int h);

#endif // SCRIPTEDITORCOMMANDPYTHON_H
