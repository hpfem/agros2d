#ifndef SOLUTIONSTORE_H
#define SOLUTIONSTORE_H

#include "solutiontypes.h"

class SolutionStore
{
public:
    SolutionArray<double> solution(FieldSolutionID solutionID, int component);
    bool contains(FieldSolutionID solutionID);
    MultiSolutionArray<double> multiSolution(FieldSolutionID solutionID);
    MultiSolutionArray<double> multiSolution(BlockSolutionID solutionID);
    void addSolution(FieldSolutionID solutionID, MultiSolutionArray<double> multiSolution);
    void addSolution(BlockSolutionID solutionID, MultiSolutionArray<double> multiSolution);
    void replaceSolution(FieldSolutionID solutionID, MultiSolutionArray<double> multiSolution);
    void replaceSolution(BlockSolutionID solutionID, MultiSolutionArray<double> multiSolution);
    void removeSolution(FieldSolutionID solutionID);
    void removeSolution(BlockSolutionID solutionID);

    int lastTimeStep(FieldInfo* fieldInfo, SolutionMode solutionType);
    int lastTimeStep(Block* block, SolutionMode solutionType);

    double lastTime(FieldInfo* fieldInfo);
    double lastTime(Block* block);

    // last adaptive step for given time step. If time step not given, last time step used implicitly
    int lastAdaptiveStep(FieldInfo* fieldInfo, SolutionMode solutionType, int timeStep = -1);
    int lastAdaptiveStep(Block* block, SolutionMode solutionType, int timeStep = -1);

    QList<double> timeLevels(FieldInfo* fieldInfo);

    FieldSolutionID lastTimeAndAdaptiveSolution(FieldInfo* fieldInfo, SolutionMode solutionType);
    BlockSolutionID lastTimeAndAdaptiveSolution(Block* block, SolutionMode solutionType);

    void clearAll();
    void clearOne(FieldSolutionID solutionID);

private:
    QMap<FieldSolutionID, MultiSolutionArray<double> > m_multiSolutions;
};

#endif // SOLUTIONSTORE_H
