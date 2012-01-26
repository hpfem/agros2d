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

class FieldInfo;

class SceneSolutionQT : public QObject
{
    Q_OBJECT

signals:
    void processedSolutionMesh();
    void processedRangeContour();
    void processedRangeScalar();
    void processedRangeVector();


};


/// TODO in the future, this class should only encapsulate solution and provide methods for its visualisation
/// all code related to meshing and solving should be moved to Problem, Block and Field
template <typename Scalar>
class SceneSolution : public SceneSolutionQT, Hermes::Hermes2D::Solution<Scalar>
{
public:
    SceneSolution(FieldInfo* fieldInfo);
    ~SceneSolution();

    void clear(bool all = true);
    void loadMeshInitial(QDomElement element);
    void saveMeshInitial(QDomDocument *doc, QDomElement element);
    void loadSolution(QDomElement element);
    void saveSolution(QDomDocument *doc, QDomElement element);

    //TODO in the future, SceneSolution will hold only one SolutionArray, not arrays for all time levels

    //TODO temp
    void setSolutionArray(QList<SolutionArray<Scalar>* > solutionArrays);

    FieldInfo* fieldInfo() { return m_fieldInfo; }

    // solution
    SolutionArray<Scalar> *solutionArray(int i = -1);
    Hermes::Hermes2D::Solution<Scalar> *sln(int i = -1);
    Hermes::Hermes2D::Space<Scalar> *space(int i = -1);
    void setSolutionArrayList(Hermes::vector<SolutionArray<Scalar> *> solutionArrayList);
    inline Hermes::vector<SolutionArray<Scalar> *> solutionArrayList() { return m_solutionArrayList; }

    // mesh
    inline Hermes::Hermes2D::Views::Linearizer &linSolutionMeshView() { return m_linSolutionMeshView; }
    inline Hermes::Hermes2D::Views::Linearizer &linInitialMeshView() { return m_linInitialMeshView; }
    Hermes::Hermes2D::Mesh* meshInitial() { return m_meshInitial; }
    void setMeshInitial(Hermes::Hermes2D::Mesh *meshInitial);


    // contour
    inline ViewScalarFilter<Scalar> *slnContourView() { return m_slnContourView; }
    void setSlnContourView(ViewScalarFilter<Scalar> *slnScalarView);
    inline Hermes::Hermes2D::Views::Linearizer &linContourView() { return m_linContourView; }

    // scalar view
    inline ViewScalarFilter<Scalar> *slnScalarView() { return m_slnScalarView; }
    void setSlnScalarView(ViewScalarFilter<Scalar> *slnScalarView);
    inline Hermes::Hermes2D::Views::Linearizer &linScalarView() { return m_linScalarView; }

    // vector view
    void setSlnVectorView(ViewScalarFilter<Scalar> *slnVectorXView, ViewScalarFilter<Scalar> *slnVectorYView);
    inline ViewScalarFilter<Scalar> *slnVectorViewX() { return m_slnVectorXView; }
    inline ViewScalarFilter<Scalar> *slnVectorViewY() { return m_slnVectorYView; }
    inline Hermes::Hermes2D::Views::Vectorizer &vecVectorView() { return m_vecVectorView; }

    // order view
    void setOrderView(shared_ptr<Hermes::Hermes2D::Space<Scalar> > space);
    Hermes::Hermes2D::Views::Orderizer &ordView() { return m_orderView; }

    int findElementInMesh(Hermes::Hermes2D::Mesh *mesh, const Point &point) const;
    int findElementInVectorizer(Hermes::Hermes2D::Views::Vectorizer &vecVectorView, const Point &point) const;

    // process
    void processView(bool showViewProgress);
    void processSolutionMesh();
    void processRangeContour();
    void processRangeScalar();
    void processRangeVector();
    void processOrder();


private:
    // general solution array
    Hermes::vector<SolutionArray<Scalar> *> m_solutionArrayList;

    // mesh
    Hermes::Hermes2D::Views::Linearizer m_linSolutionMeshView;
    Hermes::Hermes2D::Views::Linearizer m_linInitialMeshView;
    Hermes::Hermes2D::Mesh* m_meshInitial;

    // contour
    ViewScalarFilter<Scalar> *m_slnContourView; // scalar view solution
    Hermes::Hermes2D::Views::Linearizer m_linContourView;

    // scalar view
    ViewScalarFilter<Scalar> *m_slnScalarView; // scalar view solution
    Hermes::Hermes2D::Views::Linearizer m_linScalarView; // linealizer for scalar view

    // vector view
    ViewScalarFilter<Scalar> *m_slnVectorXView; // vector view solution - x
    ViewScalarFilter<Scalar> *m_slnVectorYView; // vector view solution - y
    Hermes::Hermes2D::Views::Vectorizer m_vecVectorView; // vectorizer for vector view

    // order view
    Hermes::Hermes2D::Views::Orderizer m_orderView;

    FieldInfo* m_fieldInfo;
};

typedef SceneSolution<double> SceneSolutionDouble;

#endif // SCENESOLUTION_H
