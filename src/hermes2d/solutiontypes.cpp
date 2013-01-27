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

// *********************************************************************************************

template <typename Scalar>
MultiArray<Scalar>::MultiArray()
{
}

template <typename Scalar>
MultiArray<Scalar>::~MultiArray()
{

}

template <typename Scalar>
void MultiArray<Scalar>::clear()
{
    // clear solutions
    foreach (Hermes::Hermes2D::Solution<Scalar> *sln, m_solutions)
        delete sln;

    m_solutions.clear();

    // used meshes (should be shared between spaces)
    QList<Hermes::Hermes2D::Mesh *> meshes;
    foreach (Hermes::Hermes2D::Space<Scalar> *space, m_spaces)
        if (!meshes.contains(space->get_mesh()))
            meshes.append(space->get_mesh());

    // clear meshes
    foreach (Hermes::Hermes2D::Mesh *mesh, meshes)
        delete mesh;

    // clear spaces
    foreach (Hermes::Hermes2D::Space<Scalar> *space, m_spaces)
        delete space;

    m_spaces.clear();
}

template <typename Scalar>
void MultiArray<Scalar>::append(Hermes::Hermes2D::Space<Scalar> *space, Hermes::Hermes2D::Solution<Scalar> *solution)
{
    m_spaces.push_back(space);
    m_solutions.push_back(solution);
}

template <typename Scalar>
void MultiArray<Scalar>::append(Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaces, Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutions)
{
    assert(spaces.size() == solutions.size());
    for (int i = 0; i < solutions.size(); i++)
        append(spaces.at(i), solutions.at(i));
}

template <typename Scalar>
MultiArray<Scalar> MultiArray<Scalar>::fieldPart(Block *block, FieldInfo *fieldInfo)
{
    assert(block->contains(fieldInfo));
    MultiArray<Scalar> msa;
    int offset = block->offset(block->field(fieldInfo));
    int numSol = fieldInfo->numberOfSolutions();

    for(int i = offset; i < offset + numSol; i++)
    {
        msa.append(m_spaces.at(i), m_solutions.at(i));
    }
    return msa;
}

template <typename Scalar>
void MultiArray<Scalar>::loadFromFile(const QString &baseName, FieldSolutionID solutionID)
{
    // qDebug() << "void MultiSolutionArray<Scalar>::loadFromFile(FieldSolutionID solutionID)" << solutionID.toString();

    // QTime time;
    // time.start();

    clear();

    // load the mesh file
    Hermes::vector<Mesh *> meshes = Module::readMeshFromFile(QString("%1.mesh").arg(baseName));
    Mesh *mesh = NULL;
    int i = 0;
    foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        if (fieldInfo == solutionID.group)
        {
            mesh = meshes.at(i);
            break;
        }
        i++;
    }
    assert(mesh);

    for (int i = 0; i < solutionID.group->numberOfSolutions(); i++)
    {
        Space<Scalar> *space = Space<Scalar>::load(QString("%1_%2.spc").arg(baseName).arg(i).toStdString().c_str(),
                                                   mesh, false);

        Solution<Scalar> *sln = new Solution<Scalar>();
        sln->set_validation(false);
        sln->load((QString("%1_%2.sln").arg(baseName).arg(i)).toStdString().c_str(), space);

        append(space, sln);
    }

    // qDebug() << "void MultiSolutionArray<Scalar>::loadFromFile(FieldSolutionID solutionID)" << solutionID.toString() << time.elapsed();
}

template <typename Scalar>
void MultiArray<Scalar>::saveToFile(const QString &baseName, FieldSolutionID solutionID)
{
    // qDebug() << "void MultiSolutionArray<Scalar>::saveToFile(FieldSolutionID solutionID)" << solutionID.toString();

    // QTime time;
    // time.start();

    Hermes::vector<Mesh *> meshes;
    foreach(FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        if (fieldInfo == solutionID.group)
            meshes.push_back(m_spaces.at(0)->get_mesh());
        else
            meshes.push_back(fieldInfo->initialMesh());
    }

    Module::writeMeshToFile(QString("%1.mesh").arg(baseName), meshes);

    int solutionIndex = 0;
    for (int i = 0; i < m_solutions.size(); i++)
    {
        // TODO: check write access
        m_spaces.at(i)->save((QString("%1_%2.spc").arg(baseName).arg(solutionIndex)).toStdString().c_str());
        m_solutions.at(i)->save((QString("%1_%2.sln").arg(baseName).arg(solutionIndex)).toStdString().c_str());

        solutionIndex++;
    }

    // qDebug() << "void MultiSolutionArray<Scalar>::loadFromFile(FieldSolutionID solutionID)" << solutionID.toString() << time.elapsed();
}


template class MultiArray<double>;
template class SolutionID<FieldInfo>;
