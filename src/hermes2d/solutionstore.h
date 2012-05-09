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

    int lastTimeStep(FieldInfo* fieldInfo, SolutionType solutionType);
    int lastTimeStep(Block* block, SolutionType solutionType);

    double lastTime(FieldInfo* fieldInfo);
    double lastTime(Block* block);

    // last adaptive step for given time step. If time step not given, last time step used implicitly
    int lastAdaptiveStep(FieldInfo* fieldInfo, SolutionType solutionType, int timeStep = -1);
    int lastAdaptiveStep(Block* block, SolutionType solutionType, int timeStep = -1);

    QList<double> timeLevels(FieldInfo* fieldInfo);

    FieldSolutionID lastTimeAndAdaptiveSolution(FieldInfo* fieldInfo, SolutionType solutionType);
    BlockSolutionID lastTimeAndAdaptiveSolution(Block* block, SolutionType solutionType);

    void clearAll();
    void clearOne(FieldSolutionID solutionID);

private:
    QMap<FieldSolutionID, MultiSolutionArray<double> > m_multiSolutions;
};

#endif // SOLUTIONSTORE_H
