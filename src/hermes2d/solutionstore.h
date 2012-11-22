#ifndef SOLUTIONSTORE_H
#define SOLUTIONSTORE_H

#include "solutiontypes.h"

class SolutionStore
{
public:
    SolutionArray<double> solution(FieldSolutionID solutionID, int component) const;
    bool contains(FieldSolutionID solutionID) const;
    MultiSolutionArray<double> multiSolution(FieldSolutionID solutionID) const;
    MultiSolutionArray<double> multiSolution(BlockSolutionID solutionID) const;

    // returns MultiSolution with components related to last time step, in which was each respective field calculated
    // this time step can be different for respective fields due to time step skipping
    // intented to be used as initial condition for the newton method
    MultiSolutionArray<double> multiSolutionPreviousCalculatedTS(BlockSolutionID solutionID);

    void addSolution(BlockSolutionID solutionID, MultiSolutionArray<double> multiSolution);
    void replaceSolution(BlockSolutionID solutionID, MultiSolutionArray<double> multiSolution);
    void removeSolution(BlockSolutionID solutionID);

    // removes all solutions with the given time step
    void removeTimeStep(int timeStep);

    int lastTimeStep(FieldInfo* fieldInfo, SolutionMode solutionType) const;
    int lastTimeStep(Block* block, SolutionMode solutionType) const;

    // finds nearest smaller(or equal) time step, where this fieldInfo was calculated
    int nearestTimeStep(FieldInfo* fieldInfo, int timeStep) const;

    double lastTime(FieldInfo* fieldInfo);
    double lastTime(Block* block);

    // last adaptive step for given time step. If time step not given, last time step used implicitly
    int lastAdaptiveStep(FieldInfo* fieldInfo, SolutionMode solutionType, int timeStep = -1);
    int lastAdaptiveStep(Block* block, SolutionMode solutionType, int timeStep = -1);

    QList<double> timeLevels(FieldInfo* fieldInfo);

    // number of time steps, where this fieldInfo was calculated up to this time
    int timeLevelIndex(FieldInfo* fieldInfo, double time);
    double timeLevel(FieldInfo* fieldInfo, int timeLevelIndex);

    FieldSolutionID lastTimeAndAdaptiveSolution(FieldInfo* fieldInfo, SolutionMode solutionType);
    BlockSolutionID lastTimeAndAdaptiveSolution(Block* block, SolutionMode solutionType);

    void clearAll();

private:
    QList<FieldSolutionID> m_multiSolutions;
    QMap<FieldSolutionID, MultiSolutionArray<double> > m_multiSolutionCache;

    void addSolution(FieldSolutionID solutionID, MultiSolutionArray<double> multiSolution);
    void replaceSolution(FieldSolutionID solutionID, MultiSolutionArray<double> multiSolution);
    void removeSolution(FieldSolutionID solutionID);
};

#endif // SOLUTIONSTORE_H
