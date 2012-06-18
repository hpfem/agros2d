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
#include "hermes2d/field.h"
#include "hermes2d/problem.h"

class Solution;
class SceneViewPreprocessor;
class SceneViewMesh;
class SceneViewPost2D;
class SceneViewPost3D;

class PythonEngineAgros : public PythonEngine
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

protected:
    virtual void addCustomExtensions();
    virtual void runPythonHeader();

private slots:
    void doExecutedScript();

private:
    SceneViewPreprocessor *m_sceneViewPreprocessor;
    SceneViewMesh *m_sceneViewMesh;
    SceneViewPost2D *m_sceneViewPost2D;
    SceneViewPost3D *m_sceneViewPost3D;
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

// current python engine agros
PythonEngineAgros *currentPythonEngineAgros();

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

// problem
class PyProblem
{
    public:
        PyProblem(bool clearproblem);
        ~PyProblem() {}

        // clear
        void clear();
        void field(char *fieldId);

        // name
        inline const char *getName() { return Util::problem()->config()->name().toStdString().c_str(); }
        void setName(const char *name) { Util::problem()->config()->setName(QString(name)); }

        // coordinate type
        inline const char *getCoordinateType() { return coordinateTypeToStringKey(Util::problem()->config()->coordinateType()).toStdString().c_str(); }
        void setCoordinateType(const char *coordinateType);

        // mesh type
        inline const char *getMeshType() { return meshTypeToStringKey(Util::problem()->config()->meshType()).toStdString().c_str(); }
        void setMeshType(const char *meshType);

        // matrix solver
        inline const char *getMatrixSolver() { return matrixSolverTypeToStringKey(Util::problem()->config()->matrixSolver()).toStdString().c_str(); }
        void setMatrixSolver(const char *matrixSolver);

        // frequency
        inline const double getFrequency() { return Util::problem()->config()->frequency(); }
        void setFrequency(const double frequency);

        // time step
        inline const double getTimeStep() { return Util::problem()->config()->timeStep().number(); }
        void setTimeStep(const double timeStep);

        // time total
        const double getTimeTotal() { return Util::problem()->config()->timeTotal().number(); }
        void setTimeTotal(const double timeTotal);

        // coupling
        char *getCouplingType(const char *sourceField, const char *targetField);
        void setCouplingType(const char *sourceField, const char *targetField, const char *type);

        void solve();
};

// field
class PyField
{
    private:
        FieldInfo *m_fieldInfo;

    public:
        PyField(char *fieldId);
        ~PyField() {}

        FieldInfo *fieldInfo();

        // field id
        inline const char *getFieldId() { return m_fieldInfo->fieldId().toStdString().c_str(); }

        // analysis type
        inline const char *getAnalysisType() { return analysisTypeToStringKey(Util::problem()->fieldInfo(m_fieldInfo->fieldId())->analysisType()).toStdString().c_str(); }
        void setAnalysisType(const char *analysisType);

        // number of refinements
        inline const int getNumberOfRefinements() { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->numberOfRefinements(); }
        void setNumberOfRefinements(const int numberOfRefinements);

        // polynomial order
        inline const int getPolynomialOrder() { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->polynomialOrder(); }
        void setPolynomialOrder(const int polynomialOrder);

        // linearity type
        inline const char *getLinearityType() { return linearityTypeToStringKey(Util::problem()->fieldInfo(m_fieldInfo->fieldId())->linearityType()).toStdString().c_str(); }
        void setLinearityType(const char *linearityType);

        // nonlinear tolerance
        inline const double getNonlinearTolerance() { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->nonlinearTolerance(); }
        void setNonlinearTolerance(const double nonlinearTolerance);

        // nonlinear steps
        inline const int getNonlinearSteps() { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->nonlinearSteps(); }
        void setNonlinearSteps(const int nonlinearSteps);

        // adaptivity type
        inline const char *getAdaptivityType() { return adaptivityTypeToStringKey(Util::problem()->fieldInfo(m_fieldInfo->fieldId())->adaptivityType()).toStdString().c_str(); }
        void setAdaptivityType(const char *adaptivityType);

        // adaptivity tolerance
        inline const double getAdaptivityTolerance() { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->adaptivityTolerance(); }
        void setAdaptivityTolerance(const double adaptivityTolerance);

        // adaptivity steps
        inline const int getAdaptivitySteps() { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->adaptivitySteps(); }
        void setAdaptivitySteps(const int adaptivitySteps);

        // initial condition
        inline const double getInitialCondition() { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->initialCondition().number(); }
        void setInitialCondition(const double initialCondition);

        // time steps skip
        inline const int getTimeStepsSkip() { return Util::problem()->fieldInfo(m_fieldInfo->fieldId())->timeStepsSkip().number(); }
        void setTimeStepsSkip(const int timeStepsSkip);

        // weak forms
        inline const char *getWeakForms() { return weakFormsTypeToStringKey(Util::problem()->fieldInfo(m_fieldInfo->fieldId())->weakFormsType()).toStdString().c_str(); }
        void setWeakForms(const char *weakForms);

        // boundaries
        void addBoundary(char *name, char *type, map<char*, double> parameters);
        void setBoundary(char *name, char *type, map<char*, double> parameters);
        void removeBoundary(char *name);

        // materials
        void addMaterial(char *name, map<char*, double> parameters,
                         map<char*, vector<double> > nonlin_x,
                         map<char*, vector<double> > nonlin_y);
        void setMaterial(char *name, map<char*, double> parameters,
                         map<char *, vector<double> > nonlin_x,
                         map<char *, vector<double> > nonlin_y);
        void removeMaterial(char *name);

        // solve
        void solve() { assert(0); qDebug() << "Not now :)"; }

        // local values, integrals
        void localValues(double x, double y, map<std::string, double> &results);
        void surfaceIntegrals(vector<int> edges, map<std::string, double> &results);
        void volumeIntegrals(vector<int> labels, map<std::string, double> &results);
};

// geometry
class PyGeometry
{
    public:
        PyGeometry() {}
        ~PyGeometry() {}

        void activate();

        // elements
        void addNode(double x, double y);
        void addEdge(double x1, double y1, double x2, double y2, double angle, int refinement, map<char*, char*> boundaries);
        void addEdgeByNodes(int nodeStartIndex, int nodeEndIndex, double angle, int refinement, map<char*, char*> boundaries);
        void addLabel(double x, double y, double area, int order, map<char*, char*> materials);

        // remove operations
        void removeNode(int index);
        void removeEdge(int index);
        void removeLabel(int index);

        void removeNodePoint(double x, double y);
        void removeEdgePoint(double x1, double y1, double x2, double y2, double angle);
        void removeLabelPoint(double x, double y);

        // select operations
        void selectNodes(vector<int> nodes);
        void selectEdges(vector<int> edges);
        void selectLabels(vector<int> labels);

        void selectNodePoint(double x, double y);
        void selectEdgePoint(double x, double y);
        void selectLabelPoint(double x, double y);

        void selectNone();

        // transform operations
        void moveSelection(double dx, double dy, bool copy);
        void rotateSelection(double x, double y, double angle, bool copy);
        void scaleSelection(double x, double y, double scale, bool copy);
        void removeSelection();

        // mesh
        void mesh();
        char *meshFileName();

        // zoom operations
        void zoomBestFit();
        void zoomIn();
        void zoomOut();
        void zoomRegion(double x1, double y1, double x2, double y2);
};

// view
struct PyViewConfig
{
    void refresh();

    // field
    void setField(char *fieldid);
    inline char* getField() const { return const_cast<char*>(Util::scene()->activeViewField()->fieldId().toStdString().c_str()); }

    // time step
    void setActiveTimeStep(int timeStep);
    inline int getActiveTimeStep() const { return Util::scene()->activeTimeStep(); }

    // adaptivity step
    void setActiveAdaptivityStep(int adaptivityStep);
    inline int getActiveAdaptivityStep() const { return Util::scene()->activeAdaptivityStep(); }

    // solution type
    void setActiveSolutionType(char *solutionType);
    inline char* getActiveSolutionType() const { return const_cast<char*>(solutionTypeToStringKey(Util::scene()->activeSolutionType()).toStdString().c_str()); }

    // grid
    void setGridShow(bool show);
    inline bool getGridShow() const { return Util::config()->showGrid; }
    void setGridStep(double step);
    inline double getGridStep() const { return Util::config()->gridStep; }

    // axes
    void setAxesShow(bool show);
    inline bool getAxesShow() const { return Util::config()->showAxes; }

    // rulers
    void setRulersShow(bool show);
    inline bool getRulersShow() const { return Util::config()->showRulers; }

    // todo: (Franta) font, size of nodes and edges and labels, colors
};

// view mesh
struct PyViewMesh
{
    void activate();
    void refresh();

    // mesh
    void setInitialMeshViewShow(bool show);
    inline bool getInitialMeshViewShow() const { return Util::config()->showInitialMeshView; }
    void setSolutionMeshViewShow(bool show);
    inline bool getSolutionMeshViewShow() const { return Util::config()->showSolutionMeshView; }

    // polynomial order
    void setOrderViewShow(bool show);
    inline bool getOrderViewShow() const { return Util::config()->showOrderView; }
    void setOrderViewColorBar(bool show);
    inline bool getOrderViewColorBar() const { return Util::config()->showOrderColorBar; }
    void setOrderViewLabel(bool show);
    inline bool getOrderViewLabel() const { return Util::config()->orderLabel; }
    void setOrderViewPalette(char* palette);
    inline char* getOrderViewPalette() const { return const_cast<char*>(paletteOrderTypeToStringKey(Util::config()->orderPaletteOrderType).toStdString().c_str()); }
};

// post2d
struct PyViewPost2D
{
    void activate();
    void refresh();

    // scalar view
    void setScalarViewShow(bool show);
    inline bool getScalarViewShow() const { return Util::config()->showScalarView; }
    void setScalarViewVariable(char* var);
    inline char* getScalarViewVariable() const { return const_cast<char*>(Util::config()->scalarVariable.toStdString().c_str()); }
    void setScalarViewVariableComp(char* component);
    inline char* getScalarViewVariableComp() const { return const_cast<char*>(physicFieldVariableCompToStringKey(Util::config()->scalarVariableComp).toStdString().c_str()); }

    void setScalarViewPalette(char* palette);
    inline char* getScalarViewPalette() const { return const_cast<char*>(paletteTypeToStringKey(Util::config()->paletteType).toStdString().c_str()); }
    void setScalarViewPaletteQuality(char* quality);
    inline char* getScalarViewPaletteQuality() const { return const_cast<char*>(paletteQualityToStringKey(Util::config()->linearizerQuality).toStdString().c_str()); }
    void setScalarViewPaletteSteps(int steps);
    inline int getScalarViewPaletteSteps() const { return Util::config()->paletteSteps; }
    void setScalarViewPaletteFilter(bool filter);
    inline bool getScalarViewPaletteFilter() const { return Util::config()->paletteFilter; }

    void setScalarViewRangeLog(bool log);
    inline bool getScalarViewRangeLog() const { return Util::config()->scalarRangeLog; }
    void setScalarViewRangeBase(double base);
    inline double getScalarViewRangeBase() const { return Util::config()->scalarRangeBase; }

    void setScalarViewColorBar(bool show);
    inline bool getScalarViewColorBar() const { return Util::config()->showScalarColorBar; }
    void setScalarViewDecimalPlace(int place);
    inline int getScalarViewDecimalPlace() const { return Util::config()->scalarDecimalPlace; }

    void setScalarViewRangeAuto(bool autoRange);
    inline bool getScalarViewRangeAuto() const { return Util::config()->scalarRangeAuto; }
    void setScalarViewRangeMin(double min);
    inline double getScalarViewRangeMin() const { return Util::config()->scalarRangeMin; }
    void setScalarViewRangeMax(double max);
    inline double getScalarViewRangeMax() const { return Util::config()->scalarRangeMax; }

    // contour
    void setContourShow(bool show);
    inline bool getContourShow() const { return Util::config()->showContourView; }
    void setContourCount(int count);
    inline int getContourCount() const { return Util::config()->contoursCount; }
    void setContourVariable(char* var);
    inline char* getContourVariable() const { return const_cast<char*>(Util::config()->contourVariable.toStdString().c_str()); }

    // vector
    void setVectorShow(bool show);
    inline bool getVectorShow() const { return Util::config()->showVectorView; }
    void setVectorCount(int count);
    inline int getVectorCount() const { return Util::config()->vectorCount; }
    void setVectorScale(double scale);
    inline int getVectorScale() const { return Util::config()->vectorScale; }
    void setVectorVariable(char* var);
    inline char* getVectorVariable() const { return const_cast<char*>(Util::config()->vectorVariable.toStdString().c_str()); }
    void setVectorProportional(bool show);
    inline bool getVectorProportional() const { return Util::config()->vectorProportional; }
    void setVectorColor(bool show);
    inline bool getVectorColor() const { return Util::config()->vectorColor; }
};

// post3d
struct PyViewPost3D
{
    void activate();
    void refresh();

    // scalar view
    void setPost3DMode(char* mode);
    inline char* getPost3DMode() const { return const_cast<char*>(sceneViewPost3DModeToStringKey(Util::config()->showPost3D).toStdString().c_str()); }

    // TODO: (Franta) duplicated code
    void setScalarViewVariable(char* var);
    inline char* getScalarViewVariable() const { return const_cast<char*>(Util::config()->scalarVariable.toStdString().c_str()); }
    void setScalarViewVariableComp(char* component);
    inline char* getScalarViewVariableComp() const { return const_cast<char*>(physicFieldVariableCompToStringKey(Util::config()->scalarVariableComp).toStdString().c_str()); }

    void setScalarViewPalette(char* palette);
    inline char* getScalarViewPalette() const { return const_cast<char*>(paletteTypeToStringKey(Util::config()->paletteType).toStdString().c_str()); }
    void setScalarViewPaletteQuality(char* quality);
    inline char* getScalarViewPaletteQuality() const { return const_cast<char*>(paletteQualityToStringKey(Util::config()->linearizerQuality).toStdString().c_str()); }
    void setScalarViewPaletteSteps(int steps);
    inline int getScalarViewPaletteSteps() const { return Util::config()->paletteSteps; }
    void setScalarViewPaletteFilter(bool filter);
    inline bool getScalarViewPaletteFilter() const { return Util::config()->paletteFilter; }

    void setScalarViewRangeLog(bool log);
    inline bool getScalarViewRangeLog() const { return Util::config()->scalarRangeLog; }
    void setScalarViewRangeBase(double base);
    inline double getScalarViewRangeBase() const { return Util::config()->scalarRangeBase; }

    void setScalarViewColorBar(bool show);
    inline bool getScalarViewColorBar() const { return Util::config()->showScalarColorBar; }
    void setScalarViewDecimalPlace(int place);
    inline int getScalarViewDecimalPlace() const { return Util::config()->scalarDecimalPlace; }

    void setScalarViewRangeAuto(bool autoRange);
    inline bool getScalarViewRangeAuto() const { return Util::config()->scalarRangeAuto; }
    void setScalarViewRangeMin(double min);
    inline double getScalarViewRangeMin() const { return Util::config()->scalarRangeMin; }
    void setScalarViewRangeMax(double max);
    inline double getScalarViewRangeMax() const { return Util::config()->scalarRangeMax; }
};

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

// functions
char *pyVersion();
void pyQuit();

char *pyInput(char *str);
void pyMessage(char *str);

void pyOpenDocument(char *str);
void pySaveDocument(char *str);
void pyCloseDocument();

void pySaveImage(char *str, int w, int h);

#endif // PYTHONLABAGROS_H
