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

#include "util.h"
#include "util/enums.h"

#include "hermes2d.h"

/// this header file shoul be kept small, since it is included in other header files

class FieldInfo;
class Block;

class BlockSolutionID;
class FieldSolutionID;

//template <typename Scalar>
//Hermes::vector<const SpaceSharedPtr<Scalar> > castConst(Hermes::vector<SpaceSharedPtr<Scalar> > space)
//{
//    Hermes::vector<const SpaceSharedPtr<Scalar> > out;
//    for (int i = 0; i < space.size(); i++)
//        out.push_back(const_cast<const SpaceSharedPtr<Scalar> >(space.at(i)));

//    return out;
//}

template <typename Scalar>
Hermes::vector<MeshSharedPtr> spacesMeshes(Hermes::vector<SpaceSharedPtr<Scalar> > spaces)
{
    Hermes::vector<MeshSharedPtr> meshes;
    foreach (SpaceSharedPtr<Scalar> space, spaces)
        meshes.push_back(space->get_mesh());

    return meshes;
}

template <typename Scalar>
Hermes::vector<MeshFunctionSharedPtr<Scalar> > createSolutions(Hermes::vector<MeshSharedPtr> meshes)
{
    Hermes::vector<MeshFunctionSharedPtr<Scalar> > slns;
    foreach (MeshSharedPtr mesh, meshes)
    {
        slns.push_back(MeshFunctionSharedPtr<Scalar>(new Hermes::Hermes2D::Solution<double>(mesh)));
    }

    return slns;
}

template <typename Scalar>
class AGROS_API MultiArray
{
public:
    MultiArray();
    MultiArray(Hermes::vector<SpaceSharedPtr<Scalar> > spaces,
               Hermes::vector<MeshFunctionSharedPtr<Scalar> > solutions) : m_spaces(spaces), m_solutions(solutions) {}
    ~MultiArray();

    void clear();

    // add next component
    void append(SpaceSharedPtr<Scalar> space, MeshFunctionSharedPtr<Scalar> solution);
    void append(Hermes::vector<SpaceSharedPtr<Scalar> > spaces, Hermes::vector<MeshFunctionSharedPtr<Scalar> > solutions);

    Hermes::vector<SpaceSharedPtr<Scalar> > spaces() { return m_spaces; }
    Hermes::vector<MeshFunctionSharedPtr<Scalar> > solutions() { return m_solutions; }

    //Hermes::vector<const SpaceSharedPtr<Scalar> > spacesConst() { return m_spaces; }

    // returns only that part of list that corresponds to given field (as part of the given block)
    MultiArray<Scalar> fieldPart(Block* block, FieldInfo* fieldInfo);

    int size() { assert(m_solutions.size() == m_spaces.size()); return m_spaces.size(); }

    void createEmpty(int numComp);

private:
    Hermes::vector<SpaceSharedPtr<Scalar> > m_spaces;
    Hermes::vector<MeshFunctionSharedPtr<Scalar> > m_solutions;
};

//const int LAST_ADAPTIVITY_STEP = -1;
//const int LAST_TIME_STEP = -1;

/// !!!! In case of adding more data fields, update the following operator< !!!
template <typename Group>
struct SolutionID
{
    Group* group;
    int timeStep;
    int adaptivityStep;
    SolutionMode solutionMode;

    SolutionID() : group(NULL), timeStep(0), adaptivityStep(0), solutionMode(SolutionMode_Normal) {}
    SolutionID(Group* group, int timeStep, int adaptivityStep, SolutionMode solutionMode) :
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
    FieldSolutionID(FieldInfo* fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType) :
        SolutionID<FieldInfo>(fieldInfo, timeStep, adaptivityStep, solutionType) {}

    FieldSolutionID() : SolutionID<FieldInfo>() {}

    BlockSolutionID blockSolutionID(Block* block);

    QString toString();
};

QDebug& operator<<(QDebug output, const FieldSolutionID& id);

class BlockSolutionID : public SolutionID<Block>
{
public:
    BlockSolutionID(Block* block, int timeStep, int adaptivityStep, SolutionMode solutionType) :
        SolutionID<Block>(block, timeStep, adaptivityStep, solutionType) {}

    BlockSolutionID() : SolutionID<Block>() {}

    FieldSolutionID fieldSolutionID(FieldInfo* fieldInfo);
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
