#ifndef SOLUTIONTYPES_H
#define SOLUTIONTYPES_H

#include <tr1/memory>
#include "util.h"
#include "hermes2d.h"

/// this header file shoul be kept small, since it is included in other header files

class FieldInfo;
class Block;

template <typename entity>
Hermes::vector<entity*> desmartize(Hermes::vector<shared_ptr<entity> > smart_vec)
{
    Hermes::vector<entity*> vec;
    for(int i = 0; i < smart_vec.size(); i++)
        vec.push_back(smart_vec.at(i).get());
    return vec;
}

template <typename entity>
Hermes::vector<shared_ptr<entity> > smartize(Hermes::vector<entity*>  vec)
{
    Hermes::vector<shared_ptr<entity> > smart_vec;
    for(int i = 0; i < vec.size(); i++)
        smart_vec.push_back(shared_ptr<entity>(vec.at(i)));
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
    double adaptiveError;
    int adaptiveSteps;

    std::tr1::shared_ptr<Hermes::Hermes2D::Solution<Scalar> > sln;
    std::tr1::shared_ptr<Hermes::Hermes2D::Space<Scalar> > space;

    SolutionArray();
    SolutionArray(std::tr1::shared_ptr<Hermes::Hermes2D::Solution<Scalar> > sln, std::tr1::shared_ptr<Hermes::Hermes2D::Space<Scalar> > space,
                  double adaptiveError, double adaptiveSteps, double time);
    ~SolutionArray();

    void load(QDomElement element);
    void save(QDomDocument *doc, QDomElement element);
};

template <typename Scalar>
class MultiSolutionArray
{
public:
    SolutionArray<Scalar> component(int component);

    //add next component
    void addComponent(SolutionArray<Scalar> solutionArray);

    void setSpaces(Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > spaces);
    void setSolutions(Hermes::vector<shared_ptr<Hermes::Hermes2D::Solution<Scalar> > > solutions);

    void setSpace(shared_ptr<Hermes::Hermes2D::Space<Scalar> > space, int component);
    void setSolution(shared_ptr<Hermes::Hermes2D::Solution<Scalar> > solution, int component);

    void append(MultiSolutionArray<Scalar> msa);

    //creates copy of spaces, used in solver
    Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > spaces();
    Hermes::vector<shared_ptr<Hermes::Hermes2D::Solution<Scalar> > > solutions();

    Hermes::vector<Hermes::Hermes2D::Space<Scalar>* > spacesNaked();
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar>* > solutionsNaked();

    // returns the same multi solution array with spaces only (solutions are empty)
    MultiSolutionArray<Scalar> copySpaces();

    // returns only that part of list that corresponds to given field (as part of the given block)
    MultiSolutionArray<Scalar> fieldPart(Block* block, FieldInfo* fieldInfo);

    int size() {return m_solutionArrays.size(); }

private:
    QList<SolutionArray<Scalar> > m_solutionArrays;
};

enum SolutionType{
    SolutionType_Normal,
    SolutionType_Reference,
    SolutionType_NonExisting,
    SolutionType_Finer  // used to choose reference if exists, normal otherwise
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
    SolutionType solutionType;

    SolutionID() : group(NULL), timeStep(0), adaptivityStep(0), solutionType(SolutionType_Normal) {}
    SolutionID(Group* group, int timeStep, int adaptivityStep, SolutionType solutionType) :
        group(group), timeStep(timeStep), adaptivityStep(adaptivityStep), solutionType(solutionType) {}

    bool exists() {return solutionType != SolutionType_NonExisting; }
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

    return sid1.solutionType < sid2.solutionType;
}

template <typename Group>
inline bool operator==(const SolutionID<Group> &sid1, const SolutionID<Group> &sid2)
{
    return !((sid1 < sid2) || (sid2 < sid1));
}

template <typename Group>
ostream& operator<<(ostream& output, const SolutionID<Group>& id);

//template class SolutionID<Block>;

class FieldSolutionID : public SolutionID<FieldInfo>
{
public:
    FieldSolutionID(FieldInfo* fieldInfo, int timeStep, int adaptivityStep, SolutionType solutionType) :
        SolutionID<FieldInfo>(fieldInfo, timeStep, adaptivityStep, solutionType) {}

    FieldSolutionID() : SolutionID<FieldInfo>() {}
};

class BlockSolutionID : public SolutionID<Block>
{
public:
    BlockSolutionID(Block* block, int timeStep, int adaptivityStep, SolutionType solutionType) :
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
