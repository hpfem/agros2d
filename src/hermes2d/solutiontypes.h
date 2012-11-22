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

template <typename Scalar>
struct SpaceAndMesh
{
    SpaceAndMesh() {}
    SpaceAndMesh(QSharedPointer<Hermes::Hermes2D::Space<Scalar> > sp, QSharedPointer<Hermes::Hermes2D::Mesh> ms) : space(sp), mesh(ms) {}

    QSharedPointer<Hermes::Hermes2D::Mesh> mesh;
    QSharedPointer<Hermes::Hermes2D::Space<Scalar> > space;
    Hermes::Hermes2D::Space<Scalar>* data() { return space.data(); }
};

template <typename Scalar>
struct SolutionAndMesh
{
    SolutionAndMesh() {}
    SolutionAndMesh(QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > sol, QSharedPointer<Hermes::Hermes2D::Mesh> ms) : solution(sol), mesh(ms) {}

    QSharedPointer<Hermes::Hermes2D::Mesh> mesh;
    QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > solution;
    Hermes::Hermes2D::Solution<Scalar>* data() { return solution.data(); }
};

template <typename Scalar>
Hermes::vector<Hermes::Hermes2D::Space<Scalar>* > desmartize(Hermes::vector<SpaceAndMesh<Scalar> > smart_vec)
{
    Hermes::vector<Hermes::Hermes2D::Space<Scalar>* > vec;
    for(int i = 0; i < smart_vec.size(); i++)
        vec.push_back(smart_vec.at(i).data());
    return vec;
}

template <typename Scalar>
Hermes::vector<Hermes::Hermes2D::Solution<Scalar>* > desmartize(Hermes::vector<SolutionAndMesh<Scalar> > smart_vec)
{
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar>* > vec;
    for(int i = 0; i < smart_vec.size(); i++)
        vec.push_back(smart_vec.at(i).data());
    return vec;
}


//template <typename entity>
//Hermes::vector<QSharedPointer<entity> > smartize(Hermes::vector<entity*>  vec)
//{
//    Hermes::vector<QSharedPointer<entity> > smart_vec;
//    for(int i = 0; i < vec.size(); i++)
//        smart_vec.push_back(QSharedPointer<entity>(vec.at(i)));
//    return smart_vec;
//}

template <typename Scalar>
Hermes::vector<const Hermes::Hermes2D::Space<Scalar> *> castConst(Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> space)
{
    Hermes::vector<const Hermes::Hermes2D::Space<Scalar> *> out;
    for (int i = 0; i < space.size(); i++)
        out.push_back(const_cast<const Hermes::Hermes2D::Space<Scalar> *>(space.at(i)));

    return out;
}

template <typename Scalar>
struct SolutionArray
{
    SolutionAndMesh<Scalar> sln;
    SpaceAndMesh<Scalar> space;

    SolutionArray();
    SolutionArray(SolutionAndMesh<Scalar> sln, SpaceAndMesh<Scalar>  space, double time);
    ~SolutionArray();
};

template <typename Scalar>
struct MultiSpace
{
    MultiSpace() {}
    MultiSpace(Hermes::vector<SpaceAndMesh<Scalar>  > sp ) : spaces(sp) {}
    Hermes::vector<Hermes::Hermes2D::Space<Scalar>* > naked() { return desmartize(spaces); }
    Hermes::vector<const Hermes::Hermes2D::Space<Scalar>* > nakedConst() { return castConst(desmartize(spaces)); }
    bool empty() const {return spaces.empty(); }
    int size() const {return spaces.size(); }
    SpaceAndMesh<Scalar>  at(int index) {return spaces.at(index); }
    Hermes::vector<QSharedPointer<Hermes::Hermes2D::Mesh> >meshes();
    void add(QSharedPointer<Hermes::Hermes2D::Space<Scalar> > space, QSharedPointer<Hermes::Hermes2D::Mesh> mesh) {spaces.push_back(SpaceAndMesh<Scalar>(space, mesh)); }

    Hermes::vector<SpaceAndMesh<Scalar> > spaces;
};

template <typename Scalar>
struct MultiSolution
{
    MultiSolution() {}
    MultiSolution(Hermes::vector<SolutionAndMesh<Scalar> > sols ) : solutions(sols) {}
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar>* > naked() { return desmartize(solutions); }
    bool empty() const {return solutions.empty(); }
    int size() const {return solutions.size(); }
    SolutionAndMesh<Scalar> at(int index) {return solutions.at(index); }
    void createSolutions(Hermes::vector<QSharedPointer<Hermes::Hermes2D::Mesh> > meshes);
    void add(QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > solution, QSharedPointer<Hermes::Hermes2D::Mesh> mesh) {solutions.push_back(SolutionAndMesh<Scalar>(solution, mesh)); }

    Hermes::vector<SolutionAndMesh<Scalar> > solutions;
};

template <typename Scalar>
class MultiSolutionArray
{
public:
    MultiSolutionArray();
    MultiSolutionArray(MultiSpace<Scalar> spaces, MultiSolution<Scalar> solutions, double time);
    SolutionArray<Scalar> component(int component);    

//    void setSpaces(MultiSpace<Scalar> spaces);
//    void setSolutions(MultiSolution<Scalar> solutions);

//    void setSpace(SpaceAndMesh<Scalar>  space, int component);
//    void setSolution(SolutionAndMesh<Scalar> solution, int component);

    //add next component
    void append(SolutionArray<Scalar> solutionArray);
    void append(MultiSolutionArray<Scalar> msa);

    //creates copy of spaces, used in solver
    MultiSpace<Scalar> spaces();
    MultiSolution<Scalar> solutions();

    Hermes::vector<Hermes::Hermes2D::Space<Scalar>* > spacesNaked() { return spaces().naked(); }
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar>* > solutionsNaked() { return solutions().naked(); }

    Hermes::vector<const Hermes::Hermes2D::Space<Scalar>* > spacesNakedConst() { return spaces().nakedConst(); }

    // returns the same multi solution array with spaces only (solutions are empty)
    //MultiSolutionArray<Scalar> copySpaces();

    // creates new solutions, spaces has to be allready created
    //void createNewSolutions();

    // returns only that part of list that corresponds to given field (as part of the given block)
    MultiSolutionArray<Scalar> fieldPart(Block* block, FieldInfo* fieldInfo);

    int size() {return m_solutionArrays.size(); }

    void createEmpty(int numComp);

    void saveToFile(FieldSolutionID solutionID);
    void loadFromFile(FieldSolutionID solutionID);

private:
    QList<SolutionArray<Scalar> > m_solutionArrays;
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

    inline bool exists() { return solutionMode != SolutionMode_NonExisting; }
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
        SolutionID<FieldInfo>(fieldInfo, timeStep, adaptivityStep, solutionType), m_time(0.0) {}

    FieldSolutionID() : SolutionID<FieldInfo>() {}

    BlockSolutionID blockSolutionID(Block* block);

    inline void setTime(double time) { m_time = time; }
    double time() { return m_time; }

    QString toString();

private:
    double m_time;
};

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
