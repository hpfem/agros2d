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
    Solution *sln(int i = -1);
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

#endif // SCENESOLUTION_H
