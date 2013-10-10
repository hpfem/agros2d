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

#ifndef SCENEVIEW_POST_H
#define SCENEVIEW_POST_H

#include "util.h"
#include "sceneview_common.h"

template <typename Scalar> class SceneSolution;
template <typename Scalar> class MultiArray;

class ParticleTracing;
class FieldInfo;

class PostHermes : public QObject
{
    Q_OBJECT

public:
    PostHermes();
    ~PostHermes();

    // mesh
    inline bool initialMeshIsPrepared() { return !m_linInitialMeshView.is_empty(); }
    inline Hermes::Hermes2D::Views::Linearizer &linInitialMeshView() { return m_linInitialMeshView; }
    inline bool solutionMeshIsPrepared() { return !m_linSolutionMeshView.is_empty(); }
    inline Hermes::Hermes2D::Views::Linearizer &linSolutionMeshView() { return m_linSolutionMeshView; }

    // order view
    inline bool orderIsPrepared() { return !m_orderView.is_empty(); }
    Hermes::Hermes2D::Views::Orderizer &ordView() { return m_orderView; }

    // contour
    inline bool contourIsPrepared() { return !m_linContourView.is_empty(); }
    inline Hermes::Hermes2D::Views::Linearizer &linContourView() { return m_linContourView; }

    // scalar view
    inline bool scalarIsPrepared() { return !m_linScalarView.is_empty(); }
    inline Hermes::Hermes2D::Views::Linearizer &linScalarView() { return m_linScalarView; }

    // vector view
    inline bool vectorIsPrepared() { return !m_vecVectorView.is_empty(); }
    inline Hermes::Hermes2D::Views::Vectorizer &vecVectorView() { return m_vecVectorView; }

    Hermes::Hermes2D::MeshFunctionSharedPtr<double> viewScalarFilter(Module::LocalVariable physicFieldVariable,
                                                                     PhysicFieldVariableComp physicFieldVariableComp);

    // view
    inline FieldInfo* activeViewField() const { return m_activeViewField; } // assert(m_activeViewField);
    void setActiveViewField(FieldInfo* fieldInfo);

    inline int activeTimeStep() const { return m_activeTimeStep; }
    void setActiveTimeStep(int ts);

    inline int activeAdaptivityStep() const { return m_activeAdaptivityStep; }
    void setActiveAdaptivityStep(int as);

    inline SolutionMode activeAdaptivitySolutionType() const { return m_activeSolutionMode; }
    void setActiveAdaptivitySolutionType(SolutionMode st) { m_activeSolutionMode = st; }

    MultiArray<double> activeMultiSolutionArray();

    inline bool isProcessed() const { return m_isProcessed; }

signals:
    void processed();

public slots:
    void refresh();
    void clear();
    void clearView();

private:
    bool m_isProcessed;

    // initial mesh
    Hermes::Hermes2D::Views::Linearizer m_linInitialMeshView;

    // solution mesh
    Hermes::Hermes2D::Views::Linearizer m_linSolutionMeshView;

    // order view
    Hermes::Hermes2D::Views::Orderizer m_orderView;

    // contour
    Hermes::Hermes2D::Views::Linearizer m_linContourView;

    // scalar view
    Hermes::Hermes2D::Views::Linearizer m_linScalarView; // linealizer for scalar view

    // vector view
    Hermes::Hermes2D::Views::Vectorizer m_vecVectorView; // vectorizer for vector view

    // view
    FieldInfo *m_activeViewField;
    int m_activeTimeStep;
    int m_activeAdaptivityStep;
    SolutionMode m_activeSolutionMode;

private slots:
    void processMeshed();
    void processSolved();

    void processInitialMesh();
    void processSolutionMesh();
    void processOrder();

    void processRangeContour();
    void processRangeScalar();
    void processRangeVector();

    virtual void clearGLLists() {}

    void problemMeshed();
    void problemSolved();
};

class SceneViewPostInterface : public SceneViewCommon
{
    Q_OBJECT

public:
    SceneViewPostInterface(PostHermes *postHermes, QWidget *parent = 0);

    inline PostHermes *postHermes() { assert(m_postHermes); return m_postHermes; }

protected:
    double m_texScale;
    double m_texShift;

    GLuint m_textureScalar;

    PostHermes *m_postHermes;

    virtual void initializeGL();

    void paintScalarFieldColorBar(double min, double max);

    // palette
    const QVector3D paletteColor2(const int pos) const;
    const double *paletteColor(double x) const;
    const double *paletteColorOrder(int n) const;
    void paletteCreate();

protected slots:
    virtual void clearGLLists() {}
};

#endif // SCENEVIEW_POST_H
