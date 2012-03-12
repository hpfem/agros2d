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

#ifndef PYTHONLABAGROS_H
#define PYTHONLABAGROS_H

#include "pythonlab/pythonconsole.h"
#include "pythonlab/pythonengine.h"
#include "pythonlab/pythoneditor.h"

#include "util.h"
#include "scene.h"

class Solution;

class PythonEngineAgros : public PythonEngine
{
    Q_OBJECT
public:
    PythonEngineAgros() : PythonEngine() {}

protected:
    virtual void addCustomExtensions();
    virtual void runPythonHeader();

private slots:
    void doExecutedScript();
};

class PythonLabAgros : public PythonEditorDialog
{
    Q_OBJECT
public:
    PythonLabAgros(PythonEngine *pythonEngine, QStringList args, QWidget *parent);

private:
    QAction *actCreateFromModel;

private slots:
    void doCreatePythonFromModel();
};

bool scriptIsRunning();

QString createPythonFromModel();
ScriptResult runPythonScript(const QString &script, const QString &fileName = "");
ExpressionResult runPythonExpression(const QString &expression, bool returnValue = true);

class ScriptEngineRemote : QObject
{
    Q_OBJECT
public:
    ScriptEngineRemote();
    ~ScriptEngineRemote();

private slots:
    void connected();
    void readCommand();
    void disconnected();

    void displayError(QLocalSocket::LocalSocketError socketError);

private:
    QString command;

    QLocalServer *m_server;
    QLocalSocket *m_server_socket;
    QLocalSocket *m_client_socket;
};

// cython classes
class PyParticleTracing
{
public:
    PyParticleTracing() {}
    ~PyParticleTracing() {}

    void setInitialPosition(double x, double y);
    void initialPosition(double &x, double &y)
    {
        x = Util::config()->particleStart.x;
        y = Util::config()->particleStart.y;
    }
    void setInitialVelocity(double x, double y);
    void initialVelocity(double &x, double &y)
    {
        x = Util::config()->particleStartVelocity.x;
        y = Util::config()->particleStartVelocity.y;
    }
    void setParticleMass(double mass);
    double particleMass() { return Util::config()->particleMass; }
    void setParticleCharge(double charge);
    double particleCharge() { return Util::config()->particleConstant; }
    void setDragForceDensity(double rho);
    double dragForceDensity() { return Util::config()->particleDragDensity; }
    void setDragForceReferenceArea(double area);
    double dragForceReferenceArea() { return Util::config()->particleDragReferenceArea; }
    void setDragForceCoefficient(double coeff);
    double dragForceCoefficient() { return Util::config()->particleDragCoefficient; }
    void setIncludeGravitation(int include);
    int includeGravitation() { return Util::config()->particleIncludeGravitation; }
    void setReflectOnDifferentMaterial(int reflect);
    int reflectOnDifferentMaterial() { return Util::config()->particleReflectOnDifferentMaterial; }
    void setReflectOnBoundary(int reflect);
    int reflectOnBoundary() { return Util::config()->particleReflectOnBoundary; }
    void setCoefficientOfRestitution(double coeff);
    int coefficientOfRestitution() { return Util::config()->particleCoefficientOfRestitution; }
    void setMaximumTolerance(double tolerance);
    double maximumTolerance() { return Util::config()->particleMaximumRelativeError; }
    void setMaximumNumberOfSteps(int steps);
    int maximumNumberOfSteps() { return Util::config()->particleMaximumNumberOfSteps; }
    void setMinimumStep(int step);
    int minimumStep() { return Util::config()->particleMinimumStep; }

    void solve();

    inline int length() { return m_positions.length(); }
    void positions(vector<double> &x, vector<double> &y, vector<double> &z);
    void velocities(vector<double> &x, vector<double> &y, vector<double> &z);

private:
    // position and velocity
    QList<Point3> m_positions;
    QList<Point3> m_velocities;
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
void pythonCloseDocument();

void pythonAddNode(double x, double y);
void pythonAddEdge(double x1, double y1, double x2, double y2, double angle, char *marker);
void pythonAddEdgeNodes(int nodeStartIndex, int nodeEndIndex, double angle, char *marker);
void pythonAddLabel(double x, double y, double area, int polynomialOrder, char *marker);

void pythonDeleteNode(int index);
void pythonDeleteNodePoint(double x, double y);
void pythonDeleteEdge(int index);
void pythonDeleteEdgePoint(double x1, double y1, double x2, double y2, double angle);
void pythonDeleteLabel(int index);
void pythonDeleteLabelPoint(double x, double y);

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

void pythonShowScalar(char *type, char *variable, char *component, double rangemin, double rangemax);
void pythonShowGrid(bool show);
void pythonShowGeometry(bool show);
void pythonShowInitialMesh(bool show);
void pythonShowSolutionMesh(bool show);
void pythonShowParticleTracing(bool show);
void pythonShowContours(bool show);
void pythonShowVectors(bool show);

void pythonSetTimeStep(int timestep);
int pythonTimeStepCount();

void pythonSaveImage(char *str, int w, int h);

#endif // PYTHONLABAGROS_H
