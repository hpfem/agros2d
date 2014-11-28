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

#ifndef SOLUTIONTYPES_H
#define SOLUTIONTYPES_H

#undef signals
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/lac/vector.h>
#define signals public

#include "util.h"
#include "util/enums.h"

#include "hermes2d.h"

/// this header file should be kept small, since it is included in other header files

class FieldInfo;
class FieldSolutionID;

//template <typename Scalar>
//std::vector<const Hermes::Hermes2D::SpaceSharedPtr<Scalar> > castConst(std::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > space)
//{
//    std::vector<const Hermes::Hermes2D::SpaceSharedPtr<Scalar> > out;
//    for (int i = 0; i < space.size(); i++)
//        out.push_back(const_cast<const Hermes::Hermes2D::SpaceSharedPtr<Scalar> >(space.at(i)));

//    return out;
//}

template <typename Scalar>
std::vector<Hermes::Hermes2D::MeshSharedPtr> spacesMeshes(std::vector<Hermes::Hermes2D::SpaceSharedPtr<Scalar> > spaces)
{
    std::vector<Hermes::Hermes2D::MeshSharedPtr> meshes;
    foreach (Hermes::Hermes2D::SpaceSharedPtr<Scalar> space, spaces)
        meshes.push_back(space->get_mesh());

    return meshes;
}

template <typename Scalar>
std::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<Scalar> > createSolutions(std::vector<Hermes::Hermes2D::MeshSharedPtr> meshes)
{
    std::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<Scalar> > slns;
    foreach (Hermes::Hermes2D::MeshSharedPtr mesh, meshes)
    {
        slns.push_back(Hermes::Hermes2D::MeshFunctionSharedPtr<Scalar>(new Hermes::Hermes2D::Solution<double>(mesh)));
    }

    return slns;
}

class AGROS_LIBRARY_API MultiArray
{
public:
    MultiArray();
    MultiArray(dealii::DoFHandler<2> *doFHandler, dealii::Vector<double> *solution)
        : m_doFHandler(doFHandler), m_solution(solution) {}
    ~MultiArray();

    void clear();

    // add next component
    void append(dealii::DoFHandler<2> *doFHandler, dealii::Vector<double> *solution);

    dealii::DoFHandler<2> *doFHandler() { return m_doFHandler; }
    dealii::Vector<double> *solution() { return m_solution; }

    void createEmpty(int numComp);

private:
    dealii::DoFHandler<2> *m_doFHandler;
    dealii::Vector<double> *m_solution;
};

//const int LAST_ADAPTIVITY_STEP = -1;
//const int LAST_TIME_STEP = -1;

/// !!!! In case of adding more data fields, update the following operator< !!!
template <typename Group>
struct SolutionID
{
    const Group* group;
    int timeStep;
    int adaptivityStep;
    SolutionMode solutionMode;

    SolutionID() : group(NULL), timeStep(0), adaptivityStep(0), solutionMode(SolutionMode_Normal) {}
    SolutionID(const Group* group, int timeStep, int adaptivityStep, SolutionMode solutionMode) :
        group(group), timeStep(timeStep), adaptivityStep(adaptivityStep), solutionMode(solutionMode) {}
};

template <typename Group>
inline bool operator<(const SolutionID<Group> &sid1, const SolutionID<Group> &sid2)
{
    if (sid1.group != sid2.group)
        return sid1.group < sid2.group;

    if (sid1.timeStep != sid2.timeStep)
        return sid1.timeStep < sid2.timeStep;

    if (sid1.adaptivityStep != sid2.adaptivityStep)
        return sid1.adaptivityStep < sid2.adaptivityStep;

    return sid1.solutionMode < sid2.solutionMode;
}

template <typename Group>
inline bool operator==(const SolutionID<Group> &sid1, const SolutionID<Group> &sid2)
{
    return !((sid1 < sid2) || (sid2 < sid1));
}

template <typename Group>
inline bool operator!=(const SolutionID<Group> &sid1, const SolutionID<Group> &sid2)
{
    return !(sid1 == sid2);
}

template <typename Group>
ostream& operator<<(ostream& output, const SolutionID<Group>& id)
{
    output << "(" << *id.group << ", timeStep " << id.timeStep << ", adaptStep " <<
              id.adaptivityStep << ", type "<< id.solutionMode << ")";
    return output;
}


//template class SolutionID<Block>;

class FieldSolutionID : public SolutionID<FieldInfo>
{
public:
    FieldSolutionID(const FieldInfo* fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType) :
        SolutionID<FieldInfo>(fieldInfo, timeStep, adaptivityStep, solutionType) {}

    FieldSolutionID() : SolutionID<FieldInfo>() {}

    QString toString();
};

enum SolverAction
{
    SolverAction_Solve,
    SolverAction_AdaptivityStep,
    SolverAction_TimeStep
};

struct SolverConfig
{
    SolverAction action;
    double timeStep;
};


#endif // SOLUTIONTYPES_H
