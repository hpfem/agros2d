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

#include "solutionstore.h"

#include "util/global.h"

#include "field.h"
#include "block.h"
#include "scene.h"
#include "problem.h"
#include "module_agros.h"

const int notFoundSoFar = -999;

QString SolutionStore::baseStoreFileName(FieldSolutionID solutionID) const
{
    QString fn = QString("%1/%2").
            arg(cacheProblemDir()).
            arg(solutionID.toString());

    return fn;
}

void SolutionStore::clearAll()
{
    // m_multiSolutions.clear();
    foreach (FieldSolutionID sid, m_multiSolutions)
    {
        removeSolution(sid);
    }
}

SolutionArray<double> SolutionStore::solution(FieldSolutionID solutionID, int component)
{
    return multiSolution(solutionID).component(component);
}

MultiSolutionArray<double> SolutionStore::multiSolution(FieldSolutionID solutionID)
{
    if(solutionID.solutionMode == SolutionMode_Finer)
    {
        solutionID.solutionMode = SolutionMode_Reference;
        if(!m_multiSolutions.contains(solutionID))
            solutionID.solutionMode = SolutionMode_Normal;
    }

    assert(m_multiSolutions.contains(solutionID));

    if (!m_multiSolutionCache.contains(solutionID))
    {
        MultiSolutionArray<double> msa;
        msa.loadFromFile(baseStoreFileName(solutionID), solutionID);

        // insert to the cache
        insertMultiSolutionToCache(solutionID, msa);

        return msa;
    }
    else
    {
        return m_multiSolutionCache[solutionID];
    }
}

bool SolutionStore::contains(FieldSolutionID solutionID) const
{
    return m_multiSolutions.contains(solutionID);
}

MultiSolutionArray<double> SolutionStore::multiSolution(BlockSolutionID solutionID)
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
    assert(solutionID.timeStep >= 0);
    assert(solutionID.adaptivityStep >= 0);

    // add to structures used to generate general xml file
    if(!structure.contains(solutionID.timeStep))
        structure.insert(solutionID.timeStep, StructTimeLevel(solutionID.timeStep, solutionID.time()));
    StructTimeLevel& strTimeLevel(structure[solutionID.timeStep]);

    if(!strTimeLevel.fields.contains(solutionID.group))
        strTimeLevel.fields.insert(solutionID.group, StructField(solutionID.group));
    StructField& strField(strTimeLevel.fields[solutionID.group]);

    if(!strField.adaptivitySteps.contains(solutionID.adaptivityStep))
        strField.adaptivitySteps.insert(solutionID.adaptivityStep, StructAdaptivityStep(solutionID.adaptivityStep));
    StructAdaptivityStep& strAdaptStep(strField.adaptivitySteps[solutionID.adaptivityStep]);

    if(solutionID.solutionMode == SolutionMode_Normal)
    {
        assert(!strAdaptStep.normalPresent);
        strAdaptStep.normalPresent = true;
    }

    if(solutionID.solutionMode == SolutionMode_Reference)
    {
        assert(!strAdaptStep.referencePresent);
        strAdaptStep.referencePresent = true;
    }

    // save soloution
    multiSolution.saveToFile(baseStoreFileName(solutionID), solutionID);

    m_multiSolutions.append(solutionID);

    // insert to the cache
    insertMultiSolutionToCache(solutionID, multiSolution);
}

void SolutionStore::removeSolution(FieldSolutionID solutionID)
{
    assert(m_multiSolutions.contains(solutionID));

    // remove from structures used to generate xml file
    assert(structure.contains(solutionID.timeStep));
    StructTimeLevel& strTimeLevel(structure[solutionID.timeStep]);

    assert(strTimeLevel.fields.contains(solutionID.group));
    StructField& strField(strTimeLevel.fields[solutionID.group]);

    assert(strField.adaptivitySteps.contains(solutionID.adaptivityStep));
    StructAdaptivityStep& strAdaptStep(strField.adaptivitySteps[solutionID.adaptivityStep]);

    if(solutionID.solutionMode == SolutionMode_Normal)
    {
        assert(strAdaptStep.normalPresent);
        strAdaptStep.normalPresent = false;
    }

    if(solutionID.solutionMode == SolutionMode_Reference)
    {
        assert(strAdaptStep.referencePresent);
        strAdaptStep.referencePresent = false;
    }

    if((!strAdaptStep.normalPresent) && (!strAdaptStep.referencePresent))
        strField.adaptivitySteps.remove(solutionID.adaptivityStep);

    if(strField.adaptivitySteps.count() == 0)
        strTimeLevel.fields.remove(solutionID.group);

    if(strTimeLevel.fields.count() == 0)
        structure.remove(solutionID.timeStep);

    // remove from list
    m_multiSolutions.removeOne(solutionID);
    // remove from cache
    m_multiSolutionCache.remove(solutionID);

    // remove old files
    QFileInfo info(Agros2D::problem()->config()->fileName());
    if (info.exists())
    {
        QString fn = baseStoreFileName(solutionID);

        QFile::remove(QString("%1.mesh").arg(fn));

        for (int solutionIndex = 0; solutionIndex < solutionID.group->module()->numberOfSolutions(); solutionIndex++)
        {
            QFile::remove(QString("%1_%2.spc").arg(fn).arg(solutionIndex));
            QFile::remove(QString("%1_%2.sln").arg(fn).arg(solutionIndex));
        }
    }
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

    foreach (FieldSolutionID id, m_multiSolutions)
    {
        if((id.group == fieldInfo) && (id.timeStep == timeStep) && (id.exists()))
        {
            if (time == notFoundSoFar)
                time = id.time();
            else
                assert(time == id.time());
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
        if (fsid.group == fieldInfo)
        {
            double time = fsid.time();
            if (!list.contains(time))
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
    if (timeLevelIndex >= 0 && timeLevelIndex < levels.count())
        return levels.at(timeLevelIndex);
}

void SolutionStore::insertMultiSolutionToCache(FieldSolutionID solutionID, MultiSolutionArray<double> multiSolution)
{
    if (!m_multiSolutionCache.contains(solutionID))
    {
        // flush cache
        if (m_multiSolutionCache.count() > Agros2D::configComputer()->cacheSize)
            m_multiSolutionCache.remove(m_multiSolutionCache.keys().first());

        // add solution
        m_multiSolutionCache.insert(solutionID, multiSolution);
    }
}

QString SolutionStore::StructTimeLevel::generate()
{
    QString str;
    str = "open";
    foreach(StructField sf, fields)
    {
        str += sf.generate();
    }
    str += "close";
    return str;
}

QString SolutionStore::StructField::generate()
{
    QString str;
    str = "open";
    foreach(StructAdaptivityStep as, adaptivitySteps)
    {
        str += as.generate();
    }
    str += "close";
    return str;
}

QString SolutionStore::StructAdaptivityStep::generate()
{
    QString str;
    str = "normal = .., reference = ..";
    return str;
}
