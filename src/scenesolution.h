#ifndef SCENESOLUTION_H
#define SCENESOLUTION_H

#include "util.h"
#include "hermes2d.h"
#include "scene.h"
#include "scenemarker.h"

class Scene;
class SceneLabelMarker;

class ViewScalarFilter;

struct SolutionArray;

class Solution;
class Linearizer;
class Vectorizer;
class Orderizer;
class Mesh;

struct PointValue
{
    PointValue()
    {
        this->value = 0.0;
        this->derivative = Point();
        this->marker = NULL;
    }

    PointValue(double value, Point derivative, SceneLabelMarker *marker)
    {
        this->value = value;
        this->derivative = derivative;
        this->marker = marker;
    }

    double value;
    Point derivative;
    SceneLabelMarker *marker;
};

class SceneSolution
{
public:
    SceneSolution();

    void clear();
    void loadMesh(QDomElement *element);
    void saveMesh(QDomDocument *doc, QDomElement *element);
    void loadSolution(QDomElement *element);
    void saveSolution(QDomDocument *doc, QDomElement *element);

    inline Mesh *mesh() { return m_mesh; }
    inline void setMesh(Mesh *mesh) { if (m_mesh) { delete m_mesh; } m_mesh = mesh; }
    Solution *sln();
    Solution *sln1();
    Solution *sln2();
    void setSolutionArrayList(QList<SolutionArray *> *solutionArrayList);
    inline QList<SolutionArray *> *solutionArrayList() { return m_solutionArrayList; };
    void setTimeStep(int timeStep);
    inline int timeStep() { return m_timeStep; }
    inline int timeStepCount() { return (m_solutionArrayList) ? m_solutionArrayList->count() : 0; }
    double time();

    bool isSolved() { return (m_timeStep != -1); }
    bool isMeshed() { return (m_mesh); }

    // contour
    inline ViewScalarFilter *slnContourView() { return m_slnContourView; }
    void setSlnContourView(ViewScalarFilter *slnScalarView);
    inline Linearizer &linContourView() { return m_linContourView; }

    // scalar view
    inline ViewScalarFilter *slnScalarView() { return m_slnScalarView; }
    void setSlnScalarView(ViewScalarFilter *slnScalarView);
    inline Linearizer &linScalarView() { return m_linScalarView; }

    // vector view
    void setSlnVectorView(ViewScalarFilter *slnVectorXView, ViewScalarFilter *slnVectorYView);
    inline ViewScalarFilter *slnVectorViewX() { return m_slnVectorXView; }
    inline ViewScalarFilter *slnVectorViewY() { return m_slnVectorYView; }
    inline Vectorizer &vecVectorView() { return m_vecVectorView; }

    // order view
    Orderizer &ordView();

    PointValue pointValue(const Point &point, Solution *sln);
    double volumeIntegral(int labelIndex, PhysicFieldIntegralVolume physicFieldIntegralVolume);
    double surfaceIntegral(int edgeIndex, PhysicFieldIntegralSurface physicFieldIntegralSurface);

    inline int timeElapsed() { return m_timeElapsed; }
    double adaptiveError();
    int adaptiveSteps();
    inline int setTimeElapsed(int timeElapsed) { m_timeElapsed = timeElapsed; }

    int findTriangleInMesh(Mesh *mesh, const Point &point);
    int findTriangleInVectorizer(const Vectorizer &vecVectorView, const Point &point);

private:
    int m_timeElapsed;

    // general solution array
    QList<SolutionArray *> *m_solutionArrayList;
    int m_timeStep;

    // contour
    ViewScalarFilter *m_slnContourView; // scalar view solution
    Linearizer m_linContourView;

    // scalar view
    ViewScalarFilter *m_slnScalarView; // scalar view solution
    Linearizer m_linScalarView; // linealizer for scalar view

    // vector view
    ViewScalarFilter *m_slnVectorXView; // vector view solution - x
    ViewScalarFilter *m_slnVectorYView; // vector view solution - y
    Vectorizer m_vecVectorView; // vectorizer for vector view

    Mesh *m_mesh; // linearizer only for mesh (on empty solution)

    Vectorizer m_vec;
};

class ViewScalarFilter : public Filter
{
public:
    ViewScalarFilter(MeshFunction* sln, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    ViewScalarFilter(MeshFunction* sln1, MeshFunction* sln2, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);

    double get_pt_value(double x, double y, int item = FN_VAL_0);

protected:
    virtual void precalculate(int order, int mask);
    double get_value(double value, double dudx, double dudy, double x, double y, SceneLabelMarker *marker);

private:
    PhysicFieldVariable m_physicFieldVariable;
    PhysicFieldVariableComp m_physicFieldVariableComp;
};

#endif // SCENESOLUTION_H
