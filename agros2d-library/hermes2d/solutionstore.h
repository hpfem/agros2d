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

class AGROS_LIBRARY_API SolutionStore
{
public:
    ~SolutionStore();

    class SolutionRunTimeDetails
    {
    public:
        class FileName
        {
        public:
            FileName(QString meshFileName = "", QString spaceFileName = "", QString solutionFileName = "")
                : m_meshFileName(meshFileName), m_spaceFileName(spaceFileName), m_solutionFileName(solutionFileName) {}

            inline QString meshFileName() const { return m_meshFileName; }
            inline void setMeshFileName(const QString &value) { m_meshFileName = value; }
            inline QString spaceFileName() const { return m_spaceFileName; }
            inline void setSpaceFileName(const QString &value) { m_spaceFileName = value; }
            inline QString solutionFileName() const { return m_solutionFileName; }
            inline void setSolutionFileName(const QString &value) { m_solutionFileName = value; }

        private:
            QString m_meshFileName;
            QString m_spaceFileName;
            QString m_solutionFileName;
        };

        SolutionRunTimeDetails(double time_step_length = 0, double error = 0, int DOFs = 0)
            : m_timeStepLength(time_step_length), m_adaptivityError(error), m_DOFs(DOFs) {}

        inline double timeStepLength() const { return m_timeStepLength; }
        inline void setTimeStepLength(double value) { m_timeStepLength = value; }
        inline double adaptivityError() const { return m_adaptivityError; }
        inline void setAdaptivityError(double value) { m_adaptivityError = value; }
        inline int DOFs() const { return m_DOFs; }
        inline void setDOFs(int value) { m_DOFs = value; }
        inline int jacobianCalculations() const { return m_jacobianCalculations; }
        inline void setJacobianCalculations(int value) { m_jacobianCalculations = value; }
        inline QList<FileName> fileNames() const { return m_fileNames; }
        inline void setFileNames(QList<FileName> value) { m_fileNames = value; }
        inline QVector<double> newtonResidual() const { return m_newtonResidual; }
        inline void setNewtonResidual(QVector<double> value) { m_newtonResidual = value; }
        inline QVector<double> nonlinearDamping() const { return m_nonlinearDamping; }
        inline void setNonlinearDamping(QVector<double> value) { m_nonlinearDamping = value; }

    private:
        double m_timeStepLength;
        double m_adaptivityError;
        int m_DOFs;
        int m_jacobianCalculations;

        QList<FileName> m_fileNames;
        QVector<double> m_newtonResidual;
        QVector<double> m_nonlinearDamping;
    };

    bool contains(FieldSolutionID solutionID) const;
    MultiArray<double> multiArray(FieldSolutionID solutionID);
    MultiArray<double> multiArray(BlockSolutionID solutionID);

    // returns MultiSolution with components related to last time step, in which was each respective field calculated
    // this time step can be different for respective fields due to time step skipping
    // intented to be used as initial condition for the newton method
    MultiArray<double> multiSolutionPreviousCalculatedTS(BlockSolutionID solutionID);

    void addSolution(BlockSolutionID solutionID, MultiArray<double> multiArray, SolutionRunTimeDetails runTime);
    void removeSolution(BlockSolutionID solutionID);

    // removes all solutions with the given time step
    void removeTimeStep(int timeStep);

    int lastTimeStep(const FieldInfo* fieldInfo, SolutionMode solutionType) const;
    int lastTimeStep(const Block* block, SolutionMode solutionType) const;

    // finds nearest smaller(or equal) time step, where this fieldInfo was calculated
    int nearestTimeStep(const FieldInfo* fieldInfo, int timeStep) const;

    // finds nth calculated time step for the given field
    int nthCalculatedTimeStep(const FieldInfo* fieldInfo, int n) const;

    double lastTime(const FieldInfo* fieldInfo);
    double lastTime(const Block* block);

    // last adaptive step for given time step. If time step not given, last time step used implicitly
    int lastAdaptiveStep(const FieldInfo* fieldInfo, SolutionMode solutionType, int timeStep = -1);
    int lastAdaptiveStep(const Block* block, SolutionMode solutionType, int timeStep = -1);

    QList<double> timeLevels(const FieldInfo* fieldInfo);

    // number of time steps, where this fieldInfo was calculated up to this time
    int timeLevelIndex(const FieldInfo* fieldInfo, double time);
    double timeLevel(const FieldInfo* fieldInfo, int timeLevelIndex);

    FieldSolutionID lastTimeAndAdaptiveSolution(const FieldInfo* fieldInfo, SolutionMode solutionType);
    BlockSolutionID lastTimeAndAdaptiveSolution(const Block *block, SolutionMode solutionType);

    void loadRunTimeDetails();

    SolutionRunTimeDetails multiSolutionRunTimeDetail(FieldSolutionID solutionID) const { assert(m_multiSolutionRunTimeDetails.contains(solutionID)); return m_multiSolutionRunTimeDetails[solutionID]; }
    void multiSolutionRunTimeDetailReplace(FieldSolutionID solutionID, SolutionRunTimeDetails runTime);

    inline bool isEmpty() const { return m_multiSolutions.isEmpty(); }
    void clearAll();

    void printDebugCacheStatus();

private:
    QList<FieldSolutionID> m_multiSolutions;
    QMap<FieldSolutionID, SolutionRunTimeDetails> m_multiSolutionRunTimeDetails;
    QMap<FieldSolutionID, MultiArray<double> > m_multiSolutionCache;
    QList<FieldSolutionID> m_multiSolutionCacheIDOrder;

    void addSolution(FieldSolutionID solutionID, MultiArray<double> multiArray, SolutionRunTimeDetails runTime);
    void removeSolution(FieldSolutionID solutionID);

    void insertMultiSolutionToCache(FieldSolutionID solutionID, MultiArray<double> multiArray);

    QString baseStoreFileName(FieldSolutionID solutionID) const;

    void saveRunTimeDetails();
};

#endif // SOLUTIONSTORE_H
