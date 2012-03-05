#ifndef SOLUTIONTYPES_H
#define SOLUTIONTYPES_H

#include <tr1/memory>
#include "util.h"
#include "hermes2d.h"

/// this header file shoul be kept small, since it is included in other header files

class FieldInfo;

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

    void append(MultiSolutionArray<Scalar> msa);

    //creates copy of spaces, used in solver
    Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > spaces();
    Hermes::vector<shared_ptr<Hermes::Hermes2D::Solution<Scalar> > > solutions();

    Hermes::vector<Hermes::Hermes2D::Space<Scalar>* > spacesNaked();
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar>* > solutionsNaked();

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
struct SolutionID
{
    FieldInfo* fieldInfo;
    int timeStep;
    int adaptivityStep;
    SolutionType solutionType;

    SolutionID() : fieldInfo(NULL), timeStep(0), adaptivityStep(0), solutionType(SolutionType_Normal) {}
    SolutionID(FieldInfo* fieldInfo, int timeStep, int adaptivityStep, SolutionType solutionType) :
        fieldInfo(fieldInfo), timeStep(timeStep), adaptivityStep(adaptivityStep), solutionType(solutionType) {}

    bool exists() {return solutionType != SolutionType_NonExisting; }
};

inline bool operator<(const SolutionID &sid1, const SolutionID &sid2)
{
    if (sid1.fieldInfo != sid2.fieldInfo)
        return sid1.fieldInfo < sid2.fieldInfo;

    if (sid1.timeStep != sid2.timeStep)
        return sid1.timeStep < sid2.timeStep;

    if (sid1.adaptivityStep != sid2.adaptivityStep)
        return sid1.adaptivityStep < sid2.adaptivityStep;

    return sid1.solutionType < sid2.solutionType;
}

inline bool operator==(const SolutionID &sid1, const SolutionID &sid2)
{
    return !((sid1 < sid2) || (sid2 < sid1));
}

ostream& operator<<(ostream& output, const SolutionID& id);

struct BlockSolutionID

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
