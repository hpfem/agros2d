#ifndef SCENESOLUTION_H
#define SCENESOLUTION_H

#include "util.h"
#include "scene.h"
#include "scenemarker.h"
#include "solverdialog.h"

class Scene;
class SceneLabelMarker;

class ViewScalarFilter;

struct SolutionArray;

struct PointValue
{
    PointValue(double value, Point derivative, SceneLabelMarker *marker) { this->value = value; this->derivative = derivative; this->marker = marker; }

    double value;
    Point derivative;
    SceneLabelMarker *marker;
};

class SceneSolution
{
public:
    SceneSolution(Scene *scene);

    void clear();

    inline Mesh &mesh() { return m_mesh; }
    inline Solution *sln() { return m_sln1; }
    inline Solution *sln1() { return m_sln1; }
    inline Solution *sln2() { return m_sln2; }
    void setSolutionArray(SolutionArray *solutionArray);

    inline bool isSolved() { return (m_sln1); }
    bool isMeshed() { return (m_mesh.get_num_elements() > 0); }

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
    inline Orderizer &ordView() { return m_ordView; }

    PointValue pointValue(const Point &point);
    double volumeIntegral(int labelIndex, PhysicFieldIntegralVolume physicFieldIntegralVolume);
    double surfaceIntegral(int edgeIndex, PhysicFieldIntegralSurface physicFieldIntegralSurface);

    inline int timeElapsed() { return m_timeElapsed; }
    inline double adaptiveError() { return m_adaptiveError; }
    inline int adaptiveSteps() { return m_adaptiveSteps; }
    inline int setTimeElapsed(int timeElapsed) { m_timeElapsed = timeElapsed; }

    int findTriangleInMesh(Mesh &mesh, const Point &point);
    int findTriangleInVectorizer(const Vectorizer &vecVectorView, const Point &point);

private:
    Scene *m_scene;
    int m_timeElapsed;
    double m_adaptiveError;
    int m_adaptiveSteps;

    Solution *m_sln1; // general solution 1
    Solution *m_sln2; // general solution 2

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

    // order view
    Orderizer m_ordView;

    Mesh m_mesh; // linearizer only for mesh (on empty solution)

    Vectorizer m_vec;
};

class ViewScalarFilter : public Filter
{
public:
    ViewScalarFilter(MeshFunction* sln, Scene *scene, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    ViewScalarFilter(MeshFunction* sln1, MeshFunction* sln2, Scene *scene, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);

    double get_pt_value(double x, double y, int item = FN_VAL_0);

protected:
    virtual void precalculate(int order, int mask);
    double get_value(double value, double dudx, double dudy, double x, double y, SceneLabelMarker *marker);

private:
    Scene *m_scene;
    PhysicFieldVariable m_physicFieldVariable;
    PhysicFieldVariableComp m_physicFieldVariableComp;
};

#endif // SCENESOLUTION_H
