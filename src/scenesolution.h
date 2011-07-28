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

#ifndef SCENESOLUTION_H
#define SCENESOLUTION_H

#include "util.h"
#include "hermes2d.h"

class Scene;
class SceneMaterial;

template <typename Scalar>
class ViewScalarFilter;

template <typename Scalar>
struct SolutionArray;

class ProgressDialog;
class ProgressItemMesh;
class ProgressItemSolve;
class ProgressItemProcessView;

template <typename Scalar>
class SceneSolution : public QObject
{
    Q_OBJECT

signals:
    void timeStepChanged(bool showViewProgress = true);
    void meshed();
    void solved();

    void processedSolutionMesh();
    void processedRangeContour();
    void processedRangeScalar();
    void processedRangeVector();


};

template <typename Scalar>
class SceneSolution : public SceneSolutionQT, Hermes::Hermes2D::Solution<Scalar>
{
public:
    SceneSolution();
    ~SceneSolution();

    void clear();
    void loadMeshInitial(QDomElement *element);
    void saveMeshInitial(QDomDocument *doc, QDomElement *element);
    void loadSolution(QDomElement *element);
    void saveSolution(QDomDocument *doc, QDomElement *element);

    // solve
    void solve(SolverMode solverMode);

    // mesh
    inline Hermes::Hermes2D::Mesh *meshInitial() { return m_meshInitial; }
    void setMeshInitial(Hermes::Hermes2D::Mesh *meshInitial);

    // solution
    Hermes::Hermes2D::Solution<Scalar> *sln(int i = -1);
    void setSolutionArrayList(Hermes::vector<SolutionArray *> solutionArrayList);
    inline Hermes::vector<SolutionArray *> solutionArrayList() { return m_solutionArrayList; }

    // time
    void setTimeStep(int timeStep, bool showViewProgress = true);
    inline int timeStep() const { return m_timeStep; }
    int timeStepCount() const;
    double time() const;

    bool isSolved() const { return (m_timeStep != -1); }
    bool isMeshed()  const { return m_meshInitial; }
    bool isSolving() const { return m_isSolving; }

    // mesh
    inline Hermes::Hermes2D::Views::Linearizer<Scalar> &linInitialMeshView() { return m_linInitialMeshView; }
    inline Hermes::Hermes2D::Views::Linearizer<Scalar> &linSolutionMeshView() { return m_linSolutionMeshView; }

    // contour
    inline ViewScalarFilter<Scalar> *slnContourView() { return m_slnContourView; }
    void setSlnContourView(ViewScalarFilter<Scalar> *slnScalarView);
    inline Hermes::Hermes2D::Views::Linearizer<Scalar> &linContourView() { return m_linContourView; }

    // scalar view
    inline ViewScalarFilter<Scalar> *slnScalarView() { return m_slnScalarView; }
    void setSlnScalarView(ViewScalarFilter<Scalar> *slnScalarView);
    inline Hermes::Hermes2D::Views::Linearizer<Scalar> &linScalarView() { return m_linScalarView; }

    // vector view
    void setSlnVectorView(ViewScalarFilter<Scalar> *slnVectorXView, ViewScalarFilter<Scalar> *slnVectorYView);
    inline ViewScalarFilter<Scalar> *slnVectorViewX() { return m_slnVectorXView; }
    inline ViewScalarFilter<Scalar> *slnVectorViewY() { return m_slnVectorYView; }
    inline Hermes::Hermes2D::Views::Vectorizer<Scalar> &vecVectorView() { return m_vecVectorView; }

    // order view
    Hermes::Hermes2D::Views::Orderizer *ordView();

    inline int timeElapsed() const { return m_timeElapsed; }
    double adaptiveError();
    int adaptiveSteps();
    inline void setTimeElapsed(int timeElapsed) { m_timeElapsed = timeElapsed; }

    int findElementInMesh(Hermes::Hermes2D::Mesh *mesh, const Point &point) const;
    int findElementInVectorizer(const Hermes::Hermes2D::Views::Vectorizer<Scalar> &vecVectorView, const Point &point) const;

    // process
    void processView(bool showViewProgress);
    void processSolutionMesh();
    void processRangeContour();
    void processRangeScalar();
    void processRangeVector();

    // progress dialog
    ProgressDialog *progressDialog();

private:
    int m_timeElapsed;
    bool m_isSolving;

    // general solution array
    Hermes::vector<SolutionArray<Scalar> *> m_solutionArrayList;
    int m_timeStep;

    // mesh
    Hermes::Hermes2D::Views::Linearizer<Scalar> m_linInitialMeshView;
    Hermes::Hermes2D::Views::Linearizer<Scalar> m_linSolutionMeshView;

    // contour
    ViewScalarFilter<Scalar> *m_slnContourView; // scalar view solution
    Hermes::Hermes2D::Views::Linearizer<Scalar> m_linContourView;

    // scalar view
    ViewScalarFilter<Scalar> *m_slnScalarView; // scalar view solution
    Hermes::Hermes2D::Views::Linearizer<Scalar> m_linScalarView; // linealizer for scalar view

    // vector view
    ViewScalarFilter<Scalar> *m_slnVectorXView; // vector view solution - x
    ViewScalarFilter<Scalar> *m_slnVectorYView; // vector view solution - y
    Hermes::Hermes2D::Views::Vectorizer<Scalar> m_vecVectorView; // vectorizer for vector view

    Hermes::Hermes2D::Mesh *m_meshInitial; // linearizer only for mesh (on empty solution)

    // progress dialog
    ProgressDialog *m_progressDialog;
    ProgressItemMesh *m_progressItemMesh;
    ProgressItemSolve *m_progressItemSolve;
    ProgressItemProcessView *m_progressItemProcessView;
};

typedef SceneSolution<double> SceneSolutionDouble;

#endif // SCENESOLUTION_H
