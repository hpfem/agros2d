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
struct SolutionArray;

template <typename Scalar>
struct MultiSolutionArray;

class FieldInfo;

int findElementInMesh(Hermes::Hermes2D::Mesh *mesh, const Point &point);
int findElementInVectorizer(Hermes::Hermes2D::Views::Vectorizer &vecVectorView, const Point &point);

/// TODO in the future, this class should only encapsulate solution and provide methods for its visualisation
/// all code related to meshing and solving should be moved to Problem, Block and Field
template <typename Scalar>
class SceneSolution : public Hermes::Hermes2D::Solution<Scalar>
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
    void setSolutionArray(MultiSolutionArray<Scalar> solutionArrays) { *m_solutionArrayList = solutionArrays; }

    FieldInfo* fieldInfo() { return m_fieldInfo; }

    // solution
    SolutionArray<Scalar> solutionArray(int i/* = -1*/) { return m_solutionArrayList->component(i); }

    //TODO remove .. naked pointers
    Hermes::Hermes2D::Solution<Scalar> *sln(int i/* = -1*/);
    Hermes::Hermes2D::Space<Scalar> *space(int i/* = -1*/);

    inline MultiSolutionArray<Scalar> solutionArrayList() { return *m_solutionArrayList; }

private:
    // general solution array
    MultiSolutionArray<Scalar>* m_solutionArrayList;

    FieldInfo* m_fieldInfo;
};

typedef SceneSolution<double> SceneSolutionDouble;

#endif // SCENESOLUTION_H
