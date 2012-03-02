#ifndef SOLUTIONTYPES_H
#define SOLUTIONTYPES_H

#include <tr1/memory>
#include "util.h"
#include "hermes2d.h"

class FieldInfo;

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

private:
    QList<SolutionArray<Scalar> > m_solutionArrays;
};

enum SolutionType{
    SolutionType_Normal,
    SolutionType_Reference
};

struct SolutionID
{
    FieldInfo* fieldInfo;
    int timeStep;
    int adaptivityStep;
    SolutionType solutionType;

    SolutionID() : fieldInfo(NULL), timeStep(0), adaptivityStep(0), solutionType(SolutionType_Normal) {}
    SolutionID(FieldInfo* fieldInfo, int timeStep, int adaptivityStep, SolutionType solutionType) :
        fieldInfo(fieldInfo), timeStep(timeStep), adaptivityStep(adaptivityStep), solutionType(solutionType) {}
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


#endif // SOLUTIONTYPES_H
