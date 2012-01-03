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

// ************************************************************************************

// problem class
class PyProblem {
    public:
        PyProblem(char *name, char *coordinateType, char *meshType, char *matrixSolver, double frequency, double timeStep, double timeTotal);
        ~PyProblem() {}

        // name
        inline const char *getName() { return Util::scene()->problemInfo()->name.toStdString().c_str(); }
        void setName(const char *name) { Util::scene()->problemInfo()->name = QString(name); }

        // coordinate type
        inline const char *getCoordinateType() { return coordinateTypeToStringKey(Util::scene()->problemInfo()->coordinateType).toStdString().c_str(); }
        void setCoordinateType(const char *coordinateType) { Util::scene()->problemInfo()->coordinateType = coordinateTypeFromStringKey(QString(coordinateType)); }

        // mesh type
        inline const char *getMeshType() { return meshTypeToStringKey(Util::scene()->problemInfo()->meshType).toStdString().c_str(); }
        void setMeshType(const char *meshType) { Util::scene()->problemInfo()->meshType = meshTypeFromStringKey(QString(meshType)); }

        // matrix solver
        inline const char *getMatrixSolver() { return matrixSolverTypeToStringKey(Util::scene()->problemInfo()->matrixSolver).toStdString().c_str(); }
        void setMatrixSolver(const char *matrixSolver) { Util::scene()->problemInfo()->matrixSolver = matrixSolverTypeFromStringKey(QString(matrixSolver)); }

        // frequency
        inline const double getFrequency() { return Util::scene()->problemInfo()->frequency; }
        void setFrequency(const double frequency) { Util::scene()->problemInfo()->frequency = frequency; }

        // time step
        inline const double getTimeStep() { return Util::scene()->problemInfo()->timeStep.number(); }
        void setTimeStep(const double timeStep) { Util::scene()->problemInfo()->timeStep = Value(QString::number(timeStep)); }

        // time total
        inline const double getTimeTotal() { return Util::scene()->problemInfo()->timeTotal.number(); }
        void setTimeTotal(const double timeTotal) { Util::scene()->problemInfo()->timeTotal = Value(QString::number(timeTotal)); }

        void solve() { qDebug() << "Not now :)"; }
};

// field class
class PyField {
    private:
        FieldInfo *fieldInfo;

    public:
        PyField(char *fieldId, char *analysisType, int numberOfRefinements, int polynomialOrder, char *linearityType, double nonlinearTolerance, int nonlinearSteps,
                char *adaptivityType, double adaptivityTolerance, int adaptivitySteps, double initialCondition, char *weakForms);
        ~PyField() {}

        // field id
        inline const char *getFieldId() { return fieldInfo->fieldId().toStdString().c_str(); }

        // analysis type
        inline const char *getAnalysisType() { return analysisTypeToStringKey(Util::scene()->fieldInfo(fieldInfo->fieldId())->analysisType()).toStdString().c_str(); }
        void setAnalysisType(const char *analysisType) { Util::scene()->fieldInfo(fieldInfo->fieldId())->setAnalysisType(analysisTypeFromStringKey(QString(analysisType))); }

        // number of refinements
        inline const int getNumberOfRefinemens() { return Util::scene()->fieldInfo(fieldInfo->fieldId())->numberOfRefinements; }
        void setNumberOfRefinemens(const int numberOfRefinemens) { Util::scene()->fieldInfo(fieldInfo->fieldId())->numberOfRefinements = numberOfRefinemens; }

        // polynomial order
        inline const int getPolynomialOrder() { return Util::scene()->fieldInfo(fieldInfo->fieldId())->polynomialOrder; }
        void setPolynomialOrder(const int polynomialOrder) { Util::scene()->fieldInfo(fieldInfo->fieldId())->polynomialOrder = polynomialOrder; }

        // linearity type
        inline const char *getLinearityType() { return linearityTypeToStringKey(Util::scene()->fieldInfo(fieldInfo->fieldId())->linearityType).toStdString().c_str(); }
        void setLinearityType(const char *linearityType) { Util::scene()->fieldInfo(fieldInfo->fieldId())->linearityType = linearityTypeFromStringKey(QString(linearityType)); }

        // nonlinear tolerance
        inline const double getNonlinearTolerance() { return Util::scene()->fieldInfo(fieldInfo->fieldId())->nonlinearTolerance; }
        void setNonlinearTolerance(const double nonlinearTolerance) { Util::scene()->fieldInfo(fieldInfo->fieldId())->nonlinearTolerance = nonlinearTolerance; }

        // nonlinear steps
        inline const int getNonlinearSteps() { return Util::scene()->fieldInfo(fieldInfo->fieldId())->nonlinearSteps; }
        void setNonlinearSteps(const int nonlinearSteps) { Util::scene()->fieldInfo(fieldInfo->fieldId())->nonlinearSteps = nonlinearSteps; }

        // adaptivity type
        inline const char *getAdaptivityType() { return adaptivityTypeToStringKey(Util::scene()->fieldInfo(fieldInfo->fieldId())->adaptivityType).toStdString().c_str(); }
        void setAdaptivityType(const char *adaptivityType) { Util::scene()->fieldInfo(fieldInfo->fieldId())->adaptivityType = adaptivityTypeFromStringKey(QString(adaptivityType)); }

        // adaptivity tolerance
        inline const double getAdaptivityTolerance() { return Util::scene()->fieldInfo(fieldInfo->fieldId())->adaptivityTolerance; }
        void setAdaptivityTolerance(const double adaptivityTolerance) { Util::scene()->fieldInfo(fieldInfo->fieldId())->adaptivityTolerance = adaptivityTolerance; }

        // adaptivity steps
        inline const int getAdaptivitySteps() { return Util::scene()->fieldInfo(fieldInfo->fieldId())->adaptivitySteps; }
        void setAdaptivitySteps(const int adaptivitySteps) { Util::scene()->fieldInfo(fieldInfo->fieldId())->adaptivitySteps = adaptivitySteps; }

        // initial condition
        inline const double getInitialCondition() { return Util::scene()->fieldInfo(fieldInfo->fieldId())->initialCondition.number(); }
        void setInitialCondition(const double initialCondition) { Util::scene()->fieldInfo(fieldInfo->fieldId())->initialCondition = Value(QString::number(initialCondition)); }

        // weak forms
        inline const char *getWeakForms() { return weakFormsTypeToStringKey(Util::scene()->fieldInfo(fieldInfo->fieldId())->weakFormsType).toStdString().c_str(); }
        void setWeakForms(const char *weakForms) { Util::scene()->fieldInfo(fieldInfo->fieldId())->weakFormsType = weakFormsTypeFromStringKey(QString(weakForms)); }

        void solve() { qDebug() << "Not now :)"; }
};

// geometry class
class PyGeometry {
    public:
        PyGeometry() {}
        ~PyGeometry() {}

        void addNode(double x, double y);
        void addEdge(double x1, double y1, double x2, double y2, double angle, int refinement, char *boundary);
};

// version()
char *pyVersion();

void pythonAddLabel(double x, double y, char *material, double area, int order);

// cython functions
void pythonMessage(char *str);
char *pythonInput(char *str);
char *pythonMeshFileName();
char *pythonSolutionFileName();
Solution *pythonSolutionObject();
void pythonQuit();

void pythonNewDocument(char *name, char *type, char *physicfield,
                       int numberofrefinements, int polynomialorder, char *adaptivitytype,
                       double adaptivitysteps, double adaptivitytolerance,
                       double frequency,
                       char *analysistype, double timestep, double totaltime, double initialcondition,
                       char *linearitytype, double nonlineartolerance, int nonlinearsteps);
void pythonOpenDocument(char *str);
void pythonSaveDocument(char *str);
void pythonCloseDocument();

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
void pythonSolveAdaptiveStep();

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
void pythonShowContours(bool show);
void pythonShowVectors(bool show);

void pythonSetTimeStep(int timestep);
int pythonTimeStepCount();

void pythonSaveImage(char *str, int w, int h);

#endif // PYTHONLABAGROS_H
