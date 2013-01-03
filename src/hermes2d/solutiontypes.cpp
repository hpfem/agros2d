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

#include "solutiontypes.h"

#include "util/global.h"

#include "scene.h"
#include "field.h"
#include "block.h"
#include "problem.h"
#include "module_agros.h"

using namespace Hermes::Hermes2D;

FieldSolutionID BlockSolutionID::fieldSolutionID(FieldInfo* fieldInfo)
{
    bool contains = false;
    foreach(Field* field, group->fields())
    {
        if(field->fieldInfo() == fieldInfo)
            contains = true;
    }
    assert(contains);

    return FieldSolutionID(fieldInfo, timeStep, adaptivityStep, solutionMode);
}

BlockSolutionID FieldSolutionID::blockSolutionID(Block *block)
{
    assert(block->contains(this->group));
    return BlockSolutionID(block, timeStep, adaptivityStep, solutionMode);
}

QString FieldSolutionID::toString()
{
    QString str = QString("%1_%2_%3_%4").
            arg(group->fieldId()).
            arg(timeStep).
            arg(adaptivityStep).
            arg(solutionTypeToStringKey(solutionMode));

    return str;
}

template <typename Scalar>
Hermes::vector<QSharedPointer<Hermes::Hermes2D::Mesh> > MultiSpace<Scalar>::meshes()
{
    Hermes::vector<QSharedPointer<Hermes::Hermes2D::Mesh> > meshes;
    for(int i = 0; i < spaces.size(); i++)
    {
        meshes.push_back(spaces.at(i).mesh);
    }
    return meshes;
}

template <typename Scalar>
void MultiSolution<Scalar>::createSolutions(Hermes::vector<QSharedPointer<Hermes::Hermes2D::Mesh> > meshes)
{
    for(int comp = 0; comp < meshes.size(); comp++)
    {
        QSharedPointer<Solution<Scalar> > newSolution(new Solution<double>(meshes.at(comp).data()));
        solutions.push_back(SolutionAndMesh<Scalar>(newSolution, meshes.at(comp)));
    }
}

template <typename Scalar>
SolutionArray<Scalar>::SolutionArray()
{
}

template <typename Scalar>
SolutionArray<Scalar>::SolutionArray(SolutionAndMesh<Scalar> sln, SpaceAndMesh<Scalar> space)
{
    this->sln = sln;
    this->space = space;
}

template <typename Scalar>
SolutionArray<Scalar>::~SolutionArray()
{
}

// *********************************************************************************************

template <typename Scalar>
MultiSolutionArray<Scalar>::MultiSolutionArray()
{
}

template <typename Scalar>
MultiSolutionArray<Scalar>::MultiSolutionArray(MultiSpace<Scalar> spaces, MultiSolution<Scalar> solutions)
{
    assert(spaces.size() == solutions.size());
    for (int i = 0; i < spaces.size(); i++)
    {
        assert(spaces.at(i).mesh.data() == solutions.at(i).mesh.data());
        this->append(SolutionArray<Scalar>(solutions.at(i), spaces.at(i)));
    }
}

template <typename Scalar>
void MultiSolutionArray<Scalar>::createEmpty(int numComp)
{
    m_solutionArrays.clear();
    for (int comp = 0; comp < numComp; comp++)
        append(SolutionArray<Scalar>());
}

template <typename Scalar>
void MultiSolutionArray<Scalar>::append(SolutionArray<Scalar> solutionArray)
{
    m_solutionArrays.push_back(solutionArray);
}

template <typename Scalar>
SolutionArray<Scalar> MultiSolutionArray<Scalar>::component(int component)
{
    assert(m_solutionArrays.size() > component);
    return m_solutionArrays.at(component);
}

template <typename Scalar>
void MultiSolutionArray<Scalar>::append(MultiSolutionArray<Scalar> msa)
{
    foreach(SolutionArray<Scalar> sa, msa.m_solutionArrays)
    {
        append(sa);
    }
}

template <typename Scalar>
MultiSpace<Scalar> MultiSolutionArray<Scalar>::spaces()
{
    Hermes::vector<SpaceAndMesh<Scalar> > sp;

    foreach(SolutionArray<Scalar> solutionArray, m_solutionArrays)
    {
        sp.push_back(solutionArray.space);
    }

    return MultiSpace<Scalar>(sp);
}

template <typename Scalar>
MultiSolution<Scalar> MultiSolutionArray<Scalar>::solutions()
{
    Hermes::vector<SolutionAndMesh<Scalar> > solutions;

    foreach(SolutionArray<Scalar> solutionArray, m_solutionArrays)
    {
        solutions.push_back(solutionArray.sln);
    }

    return MultiSolution<Scalar>(solutions);
}

template <typename Scalar>
MultiSolutionArray<Scalar> MultiSolutionArray<Scalar>::fieldPart(Block *block, FieldInfo *fieldInfo)
{
    assert(block->contains(fieldInfo));
    MultiSolutionArray<Scalar> msa;
    int offset = block->offset(block->field(fieldInfo));
    int numSol = fieldInfo->module()->numberOfSolutions();
    for(int comp = offset; comp < offset + numSol; comp++)
    {
        msa.append(this->component(comp));
    }
    return msa;
}

template <typename Scalar>
void MultiSolutionArray<Scalar>::loadFromFile(const QString &baseName, FieldSolutionID solutionID)
{
    // qDebug() << "void MultiSolutionArray<Scalar>::loadFromFile(FieldSolutionID solutionID)" << solutionID.toString();

    // QTime time;
    // time.start();

    m_solutionArrays.clear();

    // load the mesh file
    QList<QSharedPointer<Mesh> > meshes = readMeshFromFile(QString("%1.mesh").arg(baseName));
    QSharedPointer<Mesh> mesh;
    int i = 0;
    foreach(FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        if(fieldInfo == solutionID.group)
        {
            mesh = meshes.at(i);
            break;
        }
        i++;
    }
    assert(!mesh.isNull());

    for (int i = 0; i < solutionID.group->module()->numberOfSolutions(); i++)
    {
        QSharedPointer<Space<Scalar> > space(Space<Scalar>::load(QString("%1_%2.spc").arg(baseName).arg(i).toStdString().c_str(), mesh.data()));

        QSharedPointer<Solution<Scalar> > sln(new Solution<Scalar>());
        sln.data()->load((QString("%1_%2.sln").arg(baseName).arg(i)).toStdString().c_str(), space.data());

        SolutionArray<Scalar> solutionArray;
        solutionArray.space = SpaceAndMesh<Scalar>(space, mesh);
        solutionArray.sln = SolutionAndMesh<Scalar>(sln, mesh);

        m_solutionArrays.append(solutionArray);
    }

    // qDebug() << "void MultiSolutionArray<Scalar>::loadFromFile(FieldSolutionID solutionID)" << solutionID.toString() << time.elapsed();
}

template <typename Scalar>
void MultiSolutionArray<Scalar>::saveToFile(const QString &baseName, FieldSolutionID solutionID)
{
    // qDebug() << "void MultiSolutionArray<Scalar>::saveToFile(FieldSolutionID solutionID)" << solutionID.toString();

    // QTime time;
    // time.start();

    QList<QSharedPointer<Mesh> > meshes;
    foreach(FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        if(fieldInfo == solutionID.group)
            meshes.push_back(m_solutionArrays.at(0).space.mesh);
        else
            meshes.push_back(fieldInfo->initialMesh());
    }

    writeMeshToFile(QString("%1.mesh").arg(baseName), meshes);

    int solutionIndex = 0;
    foreach (SolutionArray<Scalar> solutionArray, m_solutionArrays)
    {
        // TODO: check write access
        solutionArray.space.data()->save((QString("%1_%2.spc").arg(baseName).arg(solutionIndex)).toStdString().c_str());
        solutionArray.sln.data()->save((QString("%1_%2.sln").arg(baseName).arg(solutionIndex)).toStdString().c_str());

        solutionIndex++;
    }

    // qDebug() << "void MultiSolutionArray<Scalar>::loadFromFile(FieldSolutionID solutionID)" << solutionID.toString() << time.elapsed();
}


template class MultiSpace<double>;
template class MultiSolution<double>;
template class SolutionArray<double>;
template class MultiSolutionArray<double>;
template class SolutionID<FieldInfo>;
