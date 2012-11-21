#include "solutionstore.h"
#include "field.h"
#include "block.h"
#include "scene.h"

const int notFoundSoFar = -999;

void SolutionStore::clearAll()
{
    m_multiSolutions.clear();
}

SolutionArray<double> SolutionStore::solution(FieldSolutionID solutionID, int component) const
{
    return multiSolution(solutionID).component(component);
}

MultiSolutionArray<double> SolutionStore::multiSolution(FieldSolutionID solutionID) const
{
    assert(m_multiSolutions.contains(solutionID));

    if (m_multiSolutionCache.first != solutionID)
    {
        MultiSolutionArray<double> msa;
        msa.loadFromFile(solutionID);

        m_multiSolutionCache.first = solutionID;
        m_multiSolutionCache.second = msa;
    }

    return m_multiSolutionCache.second;
}

bool SolutionStore::contains(FieldSolutionID solutionID) const
{
    return m_multiSolutions.contains(solutionID);
}

MultiSolutionArray<double> SolutionStore::multiSolution(BlockSolutionID solutionID) const
{
    MultiSolutionArray<double> msa;
    foreach(Field *field, solutionID.group->fields())
    {
        msa.append(multiSolution(solutionID.fieldSolutionID(field->fieldInfo())));
    }

    return msa;
}

void SolutionStore::addSolution(FieldSolutionID solutionID, MultiSolutionArray<double> multiSolution)
{
    assert(!m_multiSolutions.contains(solutionID));
    replaceSolution(solutionID, multiSolution);
}

void SolutionStore::removeSolution(FieldSolutionID solutionID)
{
    assert(m_multiSolutions.contains(solutionID));

    // remove from list
    m_multiSolutions.removeOne(solutionID);

    // TODO: remove files
}

void SolutionStore::replaceSolution(FieldSolutionID solutionID, MultiSolutionArray<double> multiSolution)
{
    //cout << "saving solution " << solutionID << std::endl;
    assert(solutionID.timeStep >= 0);
    assert(solutionID.adaptivityStep >= 0);

    multiSolution.saveToFile(solutionID);
    m_multiSolutions.append(solutionID);
}

void SolutionStore::addSolution(BlockSolutionID solutionID, MultiSolutionArray<double> multiSolution)
{
    foreach (Field* field, solutionID.group->fields())
    {
        FieldSolutionID fieldSID = solutionID.fieldSolutionID(field->fieldInfo());
        MultiSolutionArray<double> fieldMultiSolution = multiSolution.fieldPart(solutionID.group, field->fieldInfo());
        addSolution(fieldSID, fieldMultiSolution);
    }
}

void SolutionStore::replaceSolution(BlockSolutionID solutionID, MultiSolutionArray<double> multiSolution)
{
    foreach(Field* field, solutionID.group->fields())
    {
        FieldSolutionID fieldSID = solutionID.fieldSolutionID(field->fieldInfo());
        MultiSolutionArray<double> fieldMultiSolution = multiSolution.fieldPart(solutionID.group, field->fieldInfo());
        replaceSolution(fieldSID, fieldMultiSolution);
    }
}

void SolutionStore::removeSolution(BlockSolutionID solutionID)
{
    foreach(Field* field, solutionID.group->fields())
    {
        FieldSolutionID fieldSID = solutionID.fieldSolutionID(field->fieldInfo());
        removeSolution(fieldSID);
    }
}

void SolutionStore::removeTimeStep(int timeStep)
{
    foreach (FieldSolutionID sid, m_multiSolutions)
    {
        if (sid.timeStep == timeStep)
            removeSolution(sid);
    }

}

int SolutionStore::lastTimeStep(FieldInfo *fieldInfo, SolutionMode solutionType) const
{
    int timeStep = notFoundSoFar;
    foreach(FieldSolutionID sid, m_multiSolutions)
    {
        if((sid.group == fieldInfo) && (sid.solutionMode == solutionType) && (sid.timeStep > timeStep))
            timeStep = sid.timeStep;
    }

    return timeStep;
}

int SolutionStore::lastTimeStep(Block *block, SolutionMode solutionType) const
{
    int timeStep = lastTimeStep(block->fields().at(0)->fieldInfo(), solutionType);

    foreach(Field* field, block->fields())
    {
        assert(lastTimeStep(field->fieldInfo(), solutionType) == timeStep);
    }

    return timeStep;
}

MultiSolutionArray<double> SolutionStore::multiSolutionPreviousCalculatedTS(BlockSolutionID solutionID)
{
    MultiSolutionArray<double> msa;
    foreach(Field *field, solutionID.group->fields())
    {
        FieldSolutionID fieldSolutionID = solutionID.fieldSolutionID(field->fieldInfo());
        fieldSolutionID.timeStep = nearestTimeStep(field->fieldInfo(), solutionID.timeStep - 1);
        msa.append(multiSolution(fieldSolutionID));
    }

    return msa;

}


int SolutionStore::nearestTimeStep(FieldInfo *fieldInfo, int timeStep) const
{
    int ts = timeStep;
    while (!this->contains(FieldSolutionID(fieldInfo, ts, 0, SolutionMode_Normal)))
    {
        ts--;
        if (ts <= 0)
            return 0;
    }
    return ts;
}

double SolutionStore::lastTime(FieldInfo *fieldInfo)
{
    int timeStep = lastTimeStep(fieldInfo, SolutionMode_Normal);
    double time = notFoundSoFar;

    foreach(FieldSolutionID id, m_multiSolutions)
    {
        if((id.group == fieldInfo) && (id.timeStep == timeStep) && (id.exists()))
        {
            if(time == notFoundSoFar)
                time = multiSolution(id).component(0).time;
            else
                assert(time == multiSolution(id).component(0).time);
        }
    }
    assert(time != notFoundSoFar);
    return time;
}

double SolutionStore::lastTime(Block *block)
{
    double time = lastTime(block->fields().at(0)->fieldInfo());

    foreach(Field* field, block->fields())
    {
        assert(lastTime(field->fieldInfo()) == time);
    }

    return time;

}

int SolutionStore::lastAdaptiveStep(FieldInfo *fieldInfo, SolutionMode solutionType, int timeStep)
{
    if (timeStep == -1)
        timeStep = lastTimeStep(fieldInfo, solutionType);

    int adaptiveStep = notFoundSoFar;
    foreach (FieldSolutionID sid, m_multiSolutions)
    {
        if ((sid.group == fieldInfo) && (sid.solutionMode == solutionType) && (sid.timeStep == timeStep) && (sid.adaptivityStep > adaptiveStep))
            adaptiveStep = sid.adaptivityStep;
    }

    return adaptiveStep;
}

int SolutionStore::lastAdaptiveStep(Block *block, SolutionMode solutionType, int timeStep)
{
    int adaptiveStep = lastAdaptiveStep(block->fields().at(0)->fieldInfo(), solutionType, timeStep);

    foreach(Field* field, block->fields())
    {
        assert(lastAdaptiveStep(field->fieldInfo(), solutionType, timeStep) == adaptiveStep);
    }

    return adaptiveStep;
}

FieldSolutionID SolutionStore::lastTimeAndAdaptiveSolution(FieldInfo *fieldInfo, SolutionMode solutionType)
{
    FieldSolutionID solutionID;
    if(solutionType == SolutionMode_Finer) {
        FieldSolutionID solutionIDNormal = lastTimeAndAdaptiveSolution(fieldInfo, SolutionMode_Normal);
        FieldSolutionID solutionIDReference = lastTimeAndAdaptiveSolution(fieldInfo, SolutionMode_Reference);
        if((solutionIDNormal.timeStep > solutionIDReference.timeStep) ||
                (solutionIDNormal.adaptivityStep > solutionIDReference.adaptivityStep))
        {
            solutionID = solutionIDNormal;
        }
        else
        {
            solutionID = solutionIDReference;
        }
    }
    else
    {
        solutionID.group = fieldInfo;
        solutionID.adaptivityStep = lastAdaptiveStep(fieldInfo, solutionType);
        solutionID.timeStep = lastTimeStep(fieldInfo, solutionType);
        solutionID.solutionMode = solutionType;
    }

    return solutionID;
}

BlockSolutionID SolutionStore::lastTimeAndAdaptiveSolution(Block *block, SolutionMode solutionType)
{
    FieldSolutionID fsid = lastTimeAndAdaptiveSolution(block->fields().at(0)->fieldInfo(), solutionType);
    BlockSolutionID bsid = fsid.blockSolutionID(block);


    foreach(Field* field, block->fields())
    {
        assert(bsid == lastTimeAndAdaptiveSolution(field->fieldInfo(), solutionType).blockSolutionID(block));
    }

    return bsid;
}

QList<double> SolutionStore::timeLevels(FieldInfo *fieldInfo)
{
    QList<double> list;

    foreach(FieldSolutionID fsid, m_multiSolutions)
    {
        if(fsid.group == fieldInfo)
        {
            double time = multiSolution(fsid).component(0).time;
            if(!list.contains(time))
                list.push_back(time);
        }
    }

    return list;
}

int SolutionStore::timeLevelIndex(FieldInfo *fieldInfo, double time)
{
    int level = -1;
    QList<double> levels = timeLevels(fieldInfo);
    if(levels.isEmpty())
        return 0;

    foreach(double timeLevel, levels)
    {
        if(timeLevel <= time)
            level++;
    }
    assert(level >= 0);
    return level;
}

double SolutionStore::timeLevel(FieldInfo *fieldInfo, int timeLevelIndex)
{
    QList<double> levels = timeLevels(fieldInfo);
    if (timeLevelIndex >= 0 and timeLevelIndex < levels.count())
        return levels.at(timeLevelIndex);
}
