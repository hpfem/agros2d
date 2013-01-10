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

#ifndef SOLUTIONSTORE_H
#define SOLUTIONSTORE_H

#include "solutiontypes.h"

class SolutionStore
{
public:
    ~SolutionStore();

    struct SolutionRunTimeDetails
    {
        SolutionRunTimeDetails(double time_step_length = 0.0, double error = 0.0, int DOFs = 0)
            : time_step_length(time_step_length), adaptivity_error(error), DOFs(DOFs) {}

        double time_step_length;
        double adaptivity_error;
        int DOFs;
    };

    bool contains(FieldSolutionID solutionID) const;
    MultiArray<double> multiArray(FieldSolutionID solutionID);
    MultiArray<double> multiArray(BlockSolutionID solutionID);

    // returns MultiSolution with components related to last time step, in which was each respective field calculated
    // this time step can be different for respective fields due to time step skipping
    // intented to be used as initial condition for the newton method
    MultiArray<double> multiSolutionPreviousCalculatedTS(BlockSolutionID solutionID);

    void addSolution(BlockSolutionID solutionID, MultiArray<double> multiArray, SolutionRunTimeDetails runTime);

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

    void loadRunTimeDetails();

    SolutionRunTimeDetails multiSolutionRunTimeDetail(FieldSolutionID solutionID) const { assert(m_multiSolutionRunTimeDetails.contains(solutionID)); return m_multiSolutionRunTimeDetails[solutionID]; }
    void multiSolutionRunTimeDetailReplace(FieldSolutionID solutionID, SolutionRunTimeDetails runTime);

    void clearAll();

private:
    QList<FieldSolutionID> m_multiSolutions;
    QMap<FieldSolutionID, SolutionRunTimeDetails> m_multiSolutionRunTimeDetails;
    QMap<FieldSolutionID, MultiArray<double> > m_multiSolutionCache;

    void addSolution(FieldSolutionID solutionID, MultiArray<double> multiArray, SolutionRunTimeDetails runTime);
    void removeSolution(FieldSolutionID solutionID);
    void removeSolution(BlockSolutionID solutionID);

    void insertMultiSolutionToCache(FieldSolutionID solutionID, MultiArray<double> multiArray);

    QString baseStoreFileName(FieldSolutionID solutionID) const;

    void saveRunTimeDetails();
};

#endif // SOLUTIONSTORE_H
