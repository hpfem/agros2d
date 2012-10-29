#ifndef SOLUTIONTYPES_H
#define SOLUTIONTYPES_H

#include "util.h"
#include "util/enums.h"

#include "hermes2d.h"

/// this header file shoul be kept small, since it is included in other header files

class FieldInfo;
class Block;

template <typename entity>
Hermes::vector<entity*> desmartize(Hermes::vector<QSharedPointer<entity> > smart_vec)
{
    Hermes::vector<entity*> vec;
    for(int i = 0; i < smart_vec.size(); i++)
        vec.push_back(smart_vec.at(i).data());
    return vec;
}

template <typename entity>
Hermes::vector<QSharedPointer<entity> > smartize(Hermes::vector<entity*>  vec)
{
    Hermes::vector<QSharedPointer<entity> > smart_vec;
    for(int i = 0; i < vec.size(); i++)
        smart_vec.push_back(QSharedPointer<entity>(vec.at(i)));
    return smart_vec;
}

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
    double time;

    QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > sln;
    QSharedPointer<Hermes::Hermes2D::Space<Scalar> > space;

    SolutionArray();
    SolutionArray(QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > sln, QSharedPointer<Hermes::Hermes2D::Space<Scalar> > space, double time);
    ~SolutionArray();
};

template <typename Scalar>
class MultiSolutionArray
{
public:
    MultiSolutionArray();
    SolutionArray<Scalar> component(int component);

    //add next component
    void addComponent(SolutionArray<Scalar> solutionArray);

    void setSpaces(Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > spaces);
    void setSolutions(Hermes::vector<QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > > solutions);

    void setSpace(QSharedPointer<Hermes::Hermes2D::Space<Scalar> > space, int component);
    void setSolution(QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > solution, int component);

    void append(MultiSolutionArray<Scalar> msa);

    //creates copy of spaces, used in solver
    Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > spaces();
    Hermes::vector<QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > > solutions();

    Hermes::vector<Hermes::Hermes2D::Space<Scalar>* > spacesNaked() { return desmartize(spaces()); }
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar>* > solutionsNaked() {return desmartize(solutions()); }

    // returns the same multi solution array with spaces only (solutions are empty)
    MultiSolutionArray<Scalar> copySpaces();

    // creates new solutions, spaces has to be allready created
    void createNewSolutions();

    // returns only that part of list that corresponds to given field (as part of the given block)
    MultiSolutionArray<Scalar> fieldPart(Block* block, FieldInfo* fieldInfo);

    int size() {return m_solutionArrays.size(); }

    void createEmpty(int numComp);
    void setTime(double);

    void saveToFile(const QString &solutionID);
    void loadFromFile(const QString &solutionID);

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
ostream& operator<<(ostream& output, const SolutionID<Group>& id)
{
    output << "(" << *id.group << ", timeStep " << id.timeStep << ", adaptStep " <<
              id.adaptivityStep << ", type "<< id.solutionMode << ")";
    return output;
}

//template class SolutionID<Block>;

class BlockSolutionID;

class FieldSolutionID : public SolutionID<FieldInfo>
{
public:
    FieldSolutionID(FieldInfo* fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType) :
        SolutionID<FieldInfo>(fieldInfo, timeStep, adaptivityStep, solutionType) {}

    FieldSolutionID() : SolutionID<FieldInfo>() {}

    BlockSolutionID blockSolutionID(Block* block);

    QString toString();
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
