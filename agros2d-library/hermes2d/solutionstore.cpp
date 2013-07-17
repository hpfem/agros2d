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

void SolutionStore::printDebugCacheStatus()
{
    assert(m_multiSolutionCacheIDOrder.size() == m_multiSolutionCache.keys().size());
    qDebug() << "solution store cache status:";
    foreach(FieldSolutionID fsid, m_multiSolutionCacheIDOrder)
    {
        assert(m_multiSolutionCache.keys().contains(fsid));
        qDebug() << fsid;
    }
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
        //qDebug() << "Read from disk: " << solutionID.toString();

        FieldInfo *fieldInfo = solutionID.group;
        Block *block = Agros2D::problem()->blockOfField(fieldInfo);

        MultiArray<double> msa;
        SolutionRunTimeDetails runTime = m_multiSolutionRunTimeDetails[solutionID];

        for (int fieldCompIdx = 0; fieldCompIdx < solutionID.group->numberOfSolutions(); fieldCompIdx++)
        {
            // reuse space and mesh
            SpaceSharedPtr<double> space;
            foreach (FieldSolutionID searchSolutionID, m_multiSolutionCacheIDOrder)
            {
                SolutionRunTimeDetails searchRunTime = m_multiSolutionRunTimeDetails[searchSolutionID];
                if ((fieldCompIdx < searchRunTime.fileNames().size()) &&
                        (runTime.fileNames()[fieldCompIdx].meshFileName() == searchRunTime.fileNames()[fieldCompIdx].meshFileName()) &&
                        (runTime.fileNames()[fieldCompIdx].spaceFileName() == searchRunTime.fileNames()[fieldCompIdx].spaceFileName()))
                {
                    space = m_multiSolutionCache[searchSolutionID].spaces().at(fieldCompIdx);
                    break;
                }
            }

            // read space and mesh from file
            if (!space.get())
            {
                // load the mesh file
                QString fn = QString("%1/%2").arg(cacheProblemDir()).arg(runTime.fileNames()[fieldCompIdx].meshFileName());
                Hermes::vector<MeshSharedPtr> meshes;
                if (QFileInfo(fn).suffix() == "msh")
                    meshes = Module::readMeshFromFileXML(fn);
                else
                    meshes = Module::readMeshFromFileBSON(fn);

                MeshSharedPtr mesh;
                int globalFieldIdx = 0;
                foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
                {
                    if (fieldInfo == solutionID.group)
                    {
                        mesh = meshes.at(globalFieldIdx);
                        break;
                    }
                    globalFieldIdx++;
                }
                assert(mesh);

                try
                {
                    EssentialBCs<double>* essentialBcs = NULL;
                    if(fieldInfo->spaces()[fieldCompIdx].type() != HERMES_L2_SPACE)
                    {
                        int bcIndex = fieldCompIdx + block->offset(block->field(fieldInfo));
                        essentialBcs = block->bcs().at(bcIndex);
                    }
                    QString spaceFileName = QString("%1/%2").arg(cacheProblemDir()).arg(runTime.fileNames()[fieldCompIdx].spaceFileName());
                    // space = Space<double>::load(compatibleFilename(spaceFileName).toStdString().c_str(), mesh, false, essentialBcs);
                    space = Space<double>::load_bson(compatibleFilename(spaceFileName).toStdString().c_str(), mesh, essentialBcs);
                }
                catch (Hermes::Exceptions::Exception &e)
                {
                    Agros2D::log()->printError(QObject::tr("Solver"), QString::fromStdString(e.what()));
                    throw;
                }
            }

            // read solution
            Solution<double> *sln = new Solution<double>();
            sln->set_validation(false);
            // QTime time;
            // time.start();
            sln->load_bson(QString("%1/%2").
                            arg(compatibleFilename(cacheProblemDir())).
                            arg(runTime.fileNames()[fieldCompIdx].solutionFileName()).toStdString().c_str(), space);
            // sln->load((QString("%1/%2").arg(cacheProblemDir()).arg(runTime.fileNames()[fieldCompIdx].solutionFileName())).toLatin1().data(), space);
            // qDebug() << "LOAD" << time.elapsed();

            msa.append(space, sln);
        }

        // insert to the cache
        insertMultiSolutionToCache(solutionID, msa);

        //printDebugCacheStatus();

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

MultiArray<double> SolutionStore::multiArray(BlockSolutionID solutionID)
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
    QList<SolutionRunTimeDetails::FileName> fileNames;
    for (int i = 0; i < multiSolution.size(); i++)
        fileNames.append(SolutionRunTimeDetails::FileName());

    QString baseFN = baseStoreFileName(solutionID);
    FieldSolutionID previous = lastTimeAndAdaptiveSolution(solutionID.group, solutionID.solutionMode);
    if (m_multiSolutionCache.contains(previous))
    {
        MultiArray<double> ma = m_multiSolutionCache[previous];
        SolutionRunTimeDetails str = m_multiSolutionRunTimeDetails[previous];

        for (int i = 0; i < multiSolution.size(); i++)
        {
            if (ma.spaces().at(i).get()->get_mesh() == multiSolution.spaces().at(i).get()->get_mesh())
                fileNames[i].setMeshFileName(str.fileNames()[i].meshFileName());

            if (ma.spaces().at(i).get() == multiSolution.spaces().at(i).get())
                fileNames[i].setSpaceFileName(str.fileNames()[i].spaceFileName());
        }
    }

    // meshes
    for (int i = 0; i < multiSolution.size(); i++)
    {
        if (fileNames[i].meshFileName().isEmpty())
        {
            Hermes::vector<MeshSharedPtr> meshes;
            foreach(FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
            {
                if (fieldInfo == solutionID.group)
                    meshes.push_back(multiSolution.spaces().at(i)->get_mesh());
                else
                    meshes.push_back(fieldInfo->initialMesh());
            }

            // QString meshFN = QString("%1_%2.msh").arg(baseFN).arg(i);
            // Module::writeMeshToFileXML(meshFN, meshes);
            QString meshFN = QString("%1_%2.mbs").arg(baseFN).arg(i);
            Module::writeMeshToFileBSON(meshFN, meshes);

            fileNames[i].setMeshFileName(QFileInfo(meshFN).fileName());
        }
    }

    // spaces
    for (int i = 0; i < multiSolution.size(); i++)
    {
        if (fileNames[i].spaceFileName().isEmpty())
        {
            QString spaceFN = QString("%1_%2.spc").arg(baseFN).arg(i);
            // multiSolution.spaces().at(i)->save(compatibleFilename(spaceFN).toStdString().c_str());
            multiSolution.spaces().at(i)->save_bson(compatibleFilename(spaceFN).toStdString().c_str());

            fileNames[i].setSpaceFileName(QFileInfo(spaceFN).fileName());
        }
    }

    // solutions
    for (int i = 0; i < multiSolution.size(); i++)
    {
        if (fileNames[i].solutionFileName().isEmpty())
        {
            QString solutionFN = QString("%1_%2.sln").arg(baseFN).arg(i);
            // QTime time;
            // time.start();
            // dynamic_cast<Hermes::Hermes2D::Solution<double> *>(multiSolution.solutions().at(i).get())->save(solutionFN.toLatin1().data());
            dynamic_cast<Hermes::Hermes2D::Solution<double> *>(multiSolution.solutions().at(i).get())->save_bson(compatibleFilename(solutionFN).toStdString().c_str());
            // qDebug() << "SAVE" << time.elapsed();

            fileNames[i].setSolutionFileName(QFileInfo(solutionFN).fileName());
        }
    }

    runTime.setFileNames(fileNames);

    // append multisolution
    m_multiSolutions.append(solutionID);

    // append properties
    m_multiSolutionRunTimeDetails.insert(solutionID, runTime);

    // insert to the cache
    insertMultiSolutionToCache(solutionID, multiSolution);

    //printDebugCacheStatus();

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
        m_multiSolutionCacheIDOrder.removeOne(solutionID);
    }

    // remove old files
    QFileInfo info(Agros2D::problem()->config()->fileName());
    if (info.exists())
    {
        QString fn = baseStoreFileName(solutionID);

        for (int solutionIndex = 0; solutionIndex < solutionID.group->numberOfSolutions(); solutionIndex++)
        {
            QString fnMesh = QString("%1_%2.msh").arg(fn).arg(solutionIndex);
            if (QFile::exists(fnMesh))
                QFile::remove(fnMesh);

            QString fnSpace = QString("%1_%2.spc").arg(fn).arg(solutionIndex);
            if (QFile::exists(fnSpace))
                QFile::remove(fnSpace);

            QString fnSolution = QString("%1_%2.sln").arg(fn).arg(solutionIndex);
            if (QFile::exists(fnSolution))
                QFile::remove(fnSolution);
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
            assert(! m_multiSolutionCacheIDOrder.empty());
            FieldSolutionID idRemove = m_multiSolutionCacheIDOrder[0];
            m_multiSolutionCacheIDOrder.removeFirst();

            // free ma
            m_multiSolutionCache[idRemove].clear();
            m_multiSolutionCache.remove(idRemove);
        }

        // add solution
        m_multiSolutionCache.insert(solutionID, multiSolution);
        m_multiSolutionCacheIDOrder.append(solutionID);
    }
}

void SolutionStore::loadRunTimeDetails()
{
    QString fn = QString("%1/runtime.xml").arg(cacheProblemDir());

    try
    {
        std::auto_ptr<XMLStructure::structure> structure_xsd = XMLStructure::structure_(compatibleFilename(fn).toStdString(), xml_schema::flags::dont_validate);
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

            QList<SolutionRunTimeDetails::FileName> fileNames;
            for (int j = 0; j < data.files().file().size(); j++)
            {
                XMLStructure::file file = data.files().file().at(j);

                fileNames.append(SolutionRunTimeDetails::FileName(QString::fromStdString(file.mesh_filename()),
                                                                  QString::fromStdString(file.space_filename()),
                                                                  QString::fromStdString(file.solution_filename())));
            }

            SolutionRunTimeDetails runTime(data.time_step_length().get(),
                                           data.adaptivity_error().get(),
                                           data.dofs().get());
            runTime.setFileNames(fileNames);

            // append run time details
            m_multiSolutionRunTimeDetails.insert(solutionID,
                                                 runTime);
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

            XMLStructure::files files;
            for (int solutionIndex = 0; solutionIndex < solutionID.group->numberOfSolutions(); solutionIndex++)
            {
                files.file().push_back(XMLStructure::file(solutionIndex,
                                                          str.fileNames()[solutionIndex].meshFileName().toStdString(),
                                                          str.fileNames()[solutionIndex].spaceFileName().toStdString(),
                                                          str.fileNames()[solutionIndex].solutionFileName().toStdString()));
            }

            XMLStructure::newton_residuals newton_residuals;
            for (int iteration = 0; iteration < str.newtonResidual().size(); iteration++)
            {
                newton_residuals.residual().push_back(str.newtonResidual().at(iteration));
            }

            XMLStructure::newton_damping_coefficients newton_damping_coefficients;
            for (int iteration = 0; iteration < str.newtonDamping().size(); iteration++)
            {
                newton_damping_coefficients.damping_coefficient().push_back(str.newtonDamping().at(iteration));
            }

            // solution id
            XMLStructure::element_data data(files,
                                            newton_residuals,
                                            newton_damping_coefficients,
                                            solutionID.group->fieldId().toStdString(),
                                            solutionID.timeStep,
                                            solutionID.adaptivityStep,
                                            solutionTypeToStringKey(solutionID.solutionMode).toStdString());

            // properties
            data.time_step_length().set(str.timeStepLength());
            data.adaptivity_error().set(str.adaptivityError());
            data.dofs().set(str.DOFs());

            structure.element_data().push_back(data);
        }

        std::string mesh_schema_location("");

        mesh_schema_location.append(QString("%1/agros2d_structure_xml.xsd").arg(QFileInfo(datadir() + XSDROOT).absoluteFilePath()).toStdString());
        ::xml_schema::namespace_info namespace_info_mesh("XMLStructure", mesh_schema_location);

        ::xml_schema::namespace_infomap namespace_info_map;
        namespace_info_map.insert(std::pair<std::basic_string<char>, xml_schema::namespace_info>("structure", namespace_info_mesh));

        std::ofstream out(compatibleFilename(fn).toStdString().c_str());
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

