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
#include "util/constants.h"

#include "logview.h"
#include "field.h"
#include "block.h"
#include "scene.h"
#include "problem.h"
#include "problem_config.h"

#include "../../resources_source/classes/agros2d_structure_xml.h"

using namespace Hermes::Hermes2D;

MemoryInfo::MemoryInfo(MultiArray<double> multiArray)
{
    addMultiArray(multiArray);
}

void MemoryInfo::addMultiArray(MultiArray<double> multiArray)
{
    removeDealocated();
    for(int i = 0; i < multiArray.size(); i++)
    {
        tr1::weak_ptr<Hermes::Hermes2D::Mesh> meshWeak(multiArray.spaces().at(i)->get_mesh());
        meshes.push_back(meshWeak);

        tr1::weak_ptr<Hermes::Hermes2D::Space<double> > spaceWeak(multiArray.spaces().at(i));
        spaces.push_back(spaceWeak);

        tr1::weak_ptr<Hermes::Hermes2D::MeshFunction<double> > solutionWeak(multiArray.solutions().at(i));
        solutions.push_back(solutionWeak);
    }
}

void MemoryInfo::removeDealocated()
{
    int i = 0;
    while(i < meshes.size())
    {
        if(meshes.at(i).expired())
            meshes.removeAt(i);
        else
            i++;
    }

    i = 0;
    while(i < spaces.size())
    {
        if(spaces.at(i).expired())
            spaces.removeAt(i);
        else
            i++;
    }

    i = 0;
    while(i < solutions.size())
    {
        if(solutions.at(i).expired())
            solutions.removeAt(i);
        else
            i++;
    }
}

int MemoryInfo::numAlocatedMeshes()
{
    removeDealocated();
    return meshes.size();
}

int MemoryInfo::numAlocatedSpaces()
{
    removeDealocated();
    return spaces.size();
}

int MemoryInfo::numAlocatedSolutions()
{
    removeDealocated();
    return solutions.size();
}


void SolutionStore::printDebugMemoryInfo()
{
    int totalMeshes = 0;
    int totalSpaces = 0;
    int totalSolutions = 0;

    foreach(tr1::shared_ptr<MemoryInfo> mi, m_memoryInfos)
    {
        totalMeshes += mi->numAlocatedMeshes();
        totalSpaces += mi->numAlocatedSpaces();
        totalSolutions += mi->numAlocatedSolutions();
    }

    qDebug() << QString("Solution store: active %1 meshes, %2 spaces and %3 solutions. Each might be counted multiple times, fix it!")
                .arg(totalMeshes)
                .arg(totalSpaces)
                .arg(totalSolutions);
}

SolutionStore::~SolutionStore()
{
    clearAll();
}

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
        removeSolution(sid);

    assert(m_multiSolutions.count() == 0);
    assert(m_multiSolutionRunTimeDetails.count() == 0);
    assert(m_multiSolutionCache.count() == 0);
}

MultiArray<double> SolutionStore::multiArray(FieldSolutionID solutionID)
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
        // qDebug() << "Read from disk: " << solutionID.toString();

        MultiArray<double> msa;
//        try
//        {
            msa.loadFromFile(baseStoreFileName(solutionID), solutionID);

            // insert to the cache
            insertMultiSolutionToCache(solutionID, msa);

            // insert to memory info
            // assert(m_memoryInfos.keys().contains(solutionID));
            // m_memoryInfos[solutionID]->addMultiArray(msa);
//        }
//        catch (...)
//        {
//            Agros2D::problem()->clearSolution();
//            Agros2D::log()->printWarning(QObject::tr("Solver"), QObject::tr("Catched unknown exception while loading solution"));
//            qDebug() << "Solution Store: Catched unknown exception while loading solution";
//        }

        return msa;
    }
    else
    {
        // MultiArray<double> ma = m_multiSolutionCache[solutionID];
        // for (int i = 0; i < ma.solutions().size(); i++)
        //     qDebug() << "ma.solutions().at(" << i << "): " << ma.solutions().at(i).use_count();

        return m_multiSolutionCache[solutionID];
    }
}

bool SolutionStore::contains(FieldSolutionID solutionID) const
{
    return m_multiSolutions.contains(solutionID);
}

MultiArray<double> SolutionStore:: multiArray(BlockSolutionID solutionID)
{
    MultiArray<double> ma;
    foreach (Field *field, solutionID.group->fields())
    {
        MultiArray<double> maGroup = multiArray(solutionID.fieldSolutionID(field->fieldInfo()));
        ma.append(maGroup.spaces(), maGroup.solutions());
    }

    return ma;
}

void SolutionStore::addSolution(FieldSolutionID solutionID, MultiArray<double> multiSolution, SolutionRunTimeDetails runTime)
{
    // qDebug() << "saving solution " << solutionID;
    assert(!m_multiSolutions.contains(solutionID));
    assert(solutionID.timeStep >= 0);
    assert(solutionID.adaptivityStep >= 0);

    // save soloution
    multiSolution.saveToFile(baseStoreFileName(solutionID), solutionID);

    // append multisolution
    m_multiSolutions.append(solutionID);

    // append properties
    m_multiSolutionRunTimeDetails.insert(solutionID, runTime);

    // insert to the cache
    insertMultiSolutionToCache(solutionID, multiSolution);

    // save run time details to the file
    saveRunTimeDetails();

    // save to the memory info (for debug purposes)
    // m_memoryInfos[solutionID] = tr1::shared_ptr<MemoryInfo>(new MemoryInfo(multiSolution));
}

void SolutionStore::removeSolution(FieldSolutionID solutionID)
{
    assert(m_multiSolutions.contains(solutionID));

    // remove from list
    m_multiSolutions.removeOne(solutionID);
    // remove properties
    m_multiSolutionRunTimeDetails.remove(solutionID);
    // remove from cache
    if (m_multiSolutionCache.contains(solutionID))
    {
        // free ma
        m_multiSolutionCache[solutionID].clear();
        m_multiSolutionCache.remove(solutionID);
    }

    // remove old files
    QFileInfo info(Agros2D::problem()->config()->fileName());
    if (info.exists())
    {
        QString fn = baseStoreFileName(solutionID);

        QFile::remove(QString("%1.mesh").arg(fn));

        for (int solutionIndex = 0; solutionIndex < solutionID.group->numberOfSolutions(); solutionIndex++)
        {
            QFile::remove(QString("%1_%2.spc").arg(fn).arg(solutionIndex));
            QFile::remove(QString("%1_%2.sln").arg(fn).arg(solutionIndex));
        }
    }

    // save structure to the file
    saveRunTimeDetails();
}

void SolutionStore::addSolution(BlockSolutionID blockSolutionID, MultiArray<double> multiSolution, SolutionRunTimeDetails runTime)
{
    foreach (Field* field, blockSolutionID.group->fields())
    {
        FieldSolutionID fieldSID = blockSolutionID.fieldSolutionID(field->fieldInfo());
        MultiArray<double> fieldMultiSolution = multiSolution.fieldPart(blockSolutionID.group, field->fieldInfo());
        addSolution(fieldSID, fieldMultiSolution, runTime);
    }

    // printDebugMemoryInfo();
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

int SolutionStore::lastTimeStep(const FieldInfo *fieldInfo, SolutionMode solutionType) const
{
    int timeStep = NOT_FOUND_SO_FAR;
    foreach (FieldSolutionID sid, m_multiSolutions)
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

MultiArray<double> SolutionStore::multiSolutionPreviousCalculatedTS(BlockSolutionID solutionID)
{
    MultiArray<double> ma;
    foreach(Field *field, solutionID.group->fields())
    {
        FieldSolutionID fieldSolutionID = solutionID.fieldSolutionID(field->fieldInfo());
        fieldSolutionID.timeStep = nearestTimeStep(field->fieldInfo(), solutionID.timeStep - 1);

        MultiArray<double> maGroup = multiArray(fieldSolutionID);
        ma.append(maGroup.spaces(), maGroup.solutions());
    }

    return ma;
}

int SolutionStore::nthCalculatedTimeStep(FieldInfo* fieldInfo, int n) const
{
    int count = 0;
    for(int step = 0; step <= lastTimeStep(fieldInfo, SolutionMode_Normal); step++)
    {
        if(this->contains(FieldSolutionID(fieldInfo, step, 0, SolutionMode_Normal)))
            count++;

        // n is counted from zero
        if(count == n + 1)
            return step;
    }

    assert(0);
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
    double time = NOT_FOUND_SO_FAR;

    foreach (FieldSolutionID id, m_multiSolutions)
    {
        if ((id.group == fieldInfo) && (id.timeStep == timeStep))
        {
            if (time == NOT_FOUND_SO_FAR)
                time = Agros2D::problem()->timeStepToTotalTime(id.timeStep);
            else
                assert(time == Agros2D::problem()->timeStepToTotalTime(id.timeStep));
        }
    }
    assert(time != NOT_FOUND_SO_FAR);
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

    int adaptiveStep = NOT_FOUND_SO_FAR;
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
    if (solutionType == SolutionMode_Finer) {
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

QList<double> SolutionStore::timeLevels(const FieldInfo *fieldInfo)
{
    QList<double> list;

    foreach(FieldSolutionID fsid, m_multiSolutions)
    {
        if (fsid.group == fieldInfo)
        {
            double time = Agros2D::problem()->timeStepToTotalTime(fsid.timeStep);
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

void SolutionStore::insertMultiSolutionToCache(FieldSolutionID solutionID, MultiArray<double> multiSolution)
{
    if (!m_multiSolutionCache.contains(solutionID))
    {
        // flush cache
        if (m_multiSolutionCache.count() > Agros2D::configComputer()->cacheSize)
        {
            // free ma
            m_multiSolutionCache[m_multiSolutionCache.keys().first()].clear();
            m_multiSolutionCache.remove(m_multiSolutionCache.keys().first());
        }

        // add solution
        m_multiSolutionCache.insert(solutionID, multiSolution);
    }
}

void SolutionStore::loadRunTimeDetails()
{
    QString fn = QString("%1/runtime.xml").arg(cacheProblemDir());

    try
    {
        std::auto_ptr<XMLStructure::structure> structure_xsd = XMLStructure::structure_(fn.toLatin1().data(), xml_schema::flags::dont_validate);
        XMLStructure::structure *structure = structure_xsd.get();

        int time_step = 0;
        for (unsigned int i = 0; i < structure->element_data().size(); i++)
        {
            XMLStructure::element_data data = structure->element_data().at(i);

            // check field
            if (!Agros2D::problem()->hasField(QString::fromStdString(data.field_id())))
                throw AgrosException(QObject::tr("Field '%1' info mismatch.").arg(QString::fromStdString(data.field_id())));

            FieldSolutionID solutionID(Agros2D::problem()->fieldInfo(QString::fromStdString(data.field_id())),
                                       data.time_step(),
                                       data.adaptivity_step(),
                                       solutionTypeFromStringKey(QString::fromStdString(data.solution_type())));
            // append multisolution
            m_multiSolutions.append(solutionID);

            // TODO: remove "problem time step structures"
            // define transient time step
            if (data.time_step() > time_step)
            {
                // new time step
                time_step = data.time_step();

                Agros2D::problem()->defineActualTimeStepLength(data.time_step_length().get());
            }

            // append run time details
            m_multiSolutionRunTimeDetails.insert(solutionID,
                                                 SolutionRunTimeDetails(data.time_step_length().get(),
                                                                        data.adaptivity_error().get(),
                                                                        data.dofs().get()));
        }
    }
    catch (const xml_schema::exception& e)
    {
        std::cerr << e << std::endl;
    }
}

void SolutionStore::saveRunTimeDetails()
{
    QString fn = QString("%1/runtime.xml").arg(cacheProblemDir());

    try
    {
        XMLStructure::structure structure;
        foreach (FieldSolutionID solutionID, m_multiSolutions)
        {
            SolutionRunTimeDetails str = m_multiSolutionRunTimeDetails[solutionID];

            // solution id
            XMLStructure::element_data data(solutionID.group->fieldId().toStdString(),
                                            solutionID.timeStep,
                                            solutionID.adaptivityStep,
                                            solutionTypeToStringKey(solutionID.solutionMode).toStdString());


            // properties
            data.time_step_length().set(str.time_step_length);
            data.adaptivity_error().set(str.adaptivity_error);
            data.dofs().set(str.DOFs);

            structure.element_data().push_back(data);
        }

        std::string mesh_schema_location("");

        // TODO: set path more general
        mesh_schema_location.append(QString("%1/agros2d_structure_xml.xsd").arg(QFileInfo(datadir() + XSDROOT).absoluteFilePath()).toStdString());
        ::xml_schema::namespace_info namespace_info_mesh("XMLStructure", mesh_schema_location);

        ::xml_schema::namespace_infomap namespace_info_map;
        namespace_info_map.insert(std::pair<std::basic_string<char>, xml_schema::namespace_info>("structure", namespace_info_mesh));

        std::ofstream out(fn.toLatin1().data());
        XMLStructure::structure_(out, structure, namespace_info_map);
    }
    catch (const xml_schema::exception& e)
    {
        std::cerr << e << std::endl;
    }
}

void SolutionStore::multiSolutionRunTimeDetailReplace(FieldSolutionID solutionID, SolutionRunTimeDetails runTime)
{
    assert(m_multiSolutionRunTimeDetails.contains(solutionID));
    m_multiSolutionRunTimeDetails[solutionID] = runTime;

    // save structure to the file
    saveRunTimeDetails();
}

