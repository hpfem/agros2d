#include "solutiontypes.h"
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
SolutionArray<Scalar>::SolutionArray()
{
    sln.clear();
    space.clear();

    time = 0.0;
}

template <typename Scalar>
SolutionArray<Scalar>::SolutionArray(QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > sln, QSharedPointer<Hermes::Hermes2D::Space<Scalar> > space, double time)
{
    this->sln = sln;
    this->space = space;
    this->time = time;
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
void MultiSolutionArray<Scalar>::createEmpty(int numComp)
{
    m_solutionArrays.clear();
    for (int comp = 0; comp < numComp; comp++)
        addComponent(SolutionArray<Scalar>());
}

template <typename Scalar>
void MultiSolutionArray<Scalar>::addComponent(SolutionArray<Scalar> solutionArray)
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
        addComponent(sa);
    }
}

template <typename Scalar>
Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > MultiSolutionArray<Scalar>::spaces()
{
    Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > spaces;

    foreach(SolutionArray<Scalar> solutionArray, m_solutionArrays)
    {
        spaces.push_back(solutionArray.space);
    }

    return spaces;
}

template <typename Scalar>
Hermes::vector<QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > > MultiSolutionArray<Scalar>::solutions()
{
    Hermes::vector<QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > > solutions;

    foreach(SolutionArray<Scalar> solutionArray, m_solutionArrays)
    {
        solutions.push_back(solutionArray.sln);
    }

    return solutions;
}

//template <typename Scalar>
//Hermes::vector<Hermes::Hermes2D::Solution<Scalar>* > MultiSolutionArray<Scalar>::solutionsNaked()
//{
//    return desmartize(solutions());
//}

//template <typename Group>
//ostream& operator<<(ostream& output, const SolutionID<Group>& id)
//{
//    output << "(" << id.group << ", timeStep " << id.timeStep << ", adaptStep " <<
//              id.adaptivityStep << ", type "<< id.solutionType << ")";
//    return output;
//}

template <typename Scalar>
MultiSolutionArray<Scalar> MultiSolutionArray<Scalar>::copySpaces()
{
    MultiSolutionArray<Scalar> msa;
    foreach (SolutionArray<Scalar> solutionArray, m_solutionArrays)
    {
        Space<Scalar>* oldSpace = solutionArray.space.data();
//        Mesh* newMesh = new Mesh(); //TODO probably leak ... where is the mesh released
//        newMesh->copy(oldSpace->get_mesh());
//        Space<Scalar>* newSpace = oldSpace->duplicate(newMesh);
//        msa.addComponent(SolutionArray<Scalar>(QSharedPointer<Solution<Scalar> >(), QSharedPointer<Space<Scalar> >(newSpace), 0));

        Mesh::ReferenceMeshCreator meshCreator(oldSpace->get_mesh());
        Mesh *mesh = meshCreator.create_ref_mesh();

        Space<double>::ReferenceSpaceCreator spaceCreator(oldSpace, mesh, 0);
        msa.addComponent(SolutionArray<Scalar>(QSharedPointer<Solution<Scalar> >(), QSharedPointer<Space<Scalar> >(spaceCreator.create_ref_space()), 0));
    }

    return msa;
}

template <typename Scalar>
void MultiSolutionArray<Scalar>::setSpace(QSharedPointer<Hermes::Hermes2D::Space<Scalar> > space, int component)
{

    SolutionArray<Scalar> newSA(m_solutionArrays.at(component));
    newSA.space = space;
    m_solutionArrays.replace(component, newSA);
}

template <typename Scalar>
void MultiSolutionArray<Scalar>::setSpaces(Hermes::vector<QSharedPointer<Hermes::Hermes2D::Space<Scalar> > > spaces)
{
    if(m_solutionArrays.size() == 0)
    {
        for(int comp = 0; comp < spaces.size(); comp++)
        {
            m_solutionArrays.push_back(SolutionArray<Scalar>());
        }
    }
    assert(m_solutionArrays.size() == spaces.size());
    for(int comp = 0; comp < spaces.size(); comp++)
    {
        setSpace(spaces.at(comp), comp);
    }
}

template <typename Scalar>
void  MultiSolutionArray<Scalar>::createNewSolutions()
{
    for(int comp = 0; comp < size(); comp++)
    {
        Mesh* mesh = component(comp).space->get_mesh();
        setSolution(QSharedPointer<Solution<double> >(new Solution<double>(mesh)), comp);
    }
}


template <typename Scalar>
void MultiSolutionArray<Scalar>::setSolution(QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > solution, int component)
{
    SolutionArray<Scalar> newSA(m_solutionArrays.at(component));
    newSA.sln = solution;
    m_solutionArrays.replace(component, newSA);
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
        msa.addComponent(this->component(comp));
    }
    return msa;
}

template <typename Scalar>
void MultiSolutionArray<Scalar>::setTime(double time)
{
    for(int i = 0; i < m_solutionArrays.size(); i++)
    {
        SolutionArray<Scalar> newSA = m_solutionArrays.at(i);
        newSA.time = time;
        m_solutionArrays.replace(i, newSA);
    }
}

template <typename Scalar>
void MultiSolutionArray<Scalar>::loadFromFile(const QString &solutionID)
{

}

template <typename Scalar>
void MultiSolutionArray<Scalar>::saveToFile(const QString &solutionID)
{
    int solutionIndex = 0;
    foreach (SolutionArray<Scalar> solutionArray, m_solutionArrays)
    {        
        QString fn = QString("%1_%2_%3").
                arg(Util::problem()->config()->fileName().left(Util::problem()->config()->fileName().count() - 4)).
                arg(solutionID).
                arg(solutionIndex);

        // TODO: check write access
        solutionArray.sln.data()->save((fn + ".sln").toStdString().c_str());
        solutionArray.space.data()->save((fn + ".spc").toStdString().c_str());

        solutionIndex++;
    }
}


template class SolutionArray<double>;
template class MultiSolutionArray<double>;
template class SolutionID<FieldInfo>;
