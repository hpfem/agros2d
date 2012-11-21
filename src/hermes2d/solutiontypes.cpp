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
    time = 0.0;
}

template <typename Scalar>
SolutionArray<Scalar>::SolutionArray(SolutionAndMesh<Scalar> sln, SpaceAndMesh<Scalar> space, double time)
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
MultiSolutionArray<Scalar>::MultiSolutionArray(MultiSpace<Scalar> spaces, MultiSolution<Scalar> solutions, double time)
{
    assert(spaces.size() == solutions.size());
    for (int i = 0; i < spaces.size(); i++)
    {
        assert(spaces.at(i).mesh.data() == solutions.at(i).mesh.data());
        this->append(SolutionArray<Scalar>(solutions.at(i), spaces.at(i), time));
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

//template <typename Scalar>
//MultiSolutionArray<Scalar> MultiSolutionArray<Scalar>::copySpaces()
//{
//    MultiSolutionArray<Scalar> msa;
//    foreach (SolutionArray<Scalar> solutionArray, m_solutionArrays)
//    {
//        Space<Scalar>* oldSpace = solutionArray.space.data();
////        Mesh* newMesh = new Mesh(); //TODO probably leak ... where is the mesh released
////        newMesh->copy(oldSpace->get_mesh());
////        Space<Scalar>* newSpace = oldSpace->duplicate(newMesh);
////        msa.addComponent(SolutionArray<Scalar>(QSharedPointer<Solution<Scalar> >(), QSharedPointer<Space<Scalar> >(newSpace), 0));

//        Mesh *newMesh = new Mesh(); //TODO probably leak ... where is the mesh released
//        newMesh->copy(oldSpace->get_mesh());

//        Space<double>::ReferenceSpaceCreator spaceCreator(oldSpace, newMesh, 0);
//        msa.addComponent(SolutionArray<Scalar>(QSharedPointer<Solution<Scalar> >(), QSharedPointer<Space<Scalar> >(spaceCreator.create_ref_space()), 0));
//    }

//    return msa;
//}

//template <typename Scalar>
//void MultiSolutionArray<Scalar>::setSpace(QSharedPointer<Hermes::Hermes2D::Space<Scalar> > space, int component)
//{

//    SolutionArray<Scalar> newSA(m_solutionArrays.at(component));
//    newSA.space = space;
//    m_solutionArrays.replace(component, newSA);
//}

//template <typename Scalar>
//void MultiSolutionArray<Scalar>::setSpaces(MultiSpace<Scalar> spacesParam)
//{
//    if(m_solutionArrays.size() == 0)
//    {
//        for(int comp = 0; comp < spacesParam.spaces.size(); comp++)
//        {
//            m_solutionArrays.push_back(SolutionArray<Scalar>());
//        }
//    }
//    assert(m_solutionArrays.size() == spacesParam.spaces.size());
//    for(int comp = 0; comp < spacesParam.spaces.size(); comp++)
//    {
//        setSpace(spacesParam.spaces.at(comp), comp);
//    }
//}

//template <typename Scalar>
//void  MultiSolutionArray<Scalar>::createNewSolutions()
//{
//    for(int comp = 0; comp < size(); comp++)
//    {
//        Mesh* mesh = component(comp).space->get_mesh();
//        setSolution(QSharedPointer<Solution<double> >(new Solution<double>(mesh)), comp);
//    }
//}


//template <typename Scalar>
//void MultiSolutionArray<Scalar>::setSolution(QSharedPointer<Hermes::Hermes2D::Solution<Scalar> > solution, int component)
//{
//    SolutionArray<Scalar> newSA(m_solutionArrays.at(component));
//    newSA.sln = solution;
//    m_solutionArrays.replace(component, newSA);
//}

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
void MultiSolutionArray<Scalar>::loadFromFile(FieldSolutionID solutionID)
{
    // qDebug() << "void MultiSolutionArray<Scalar>::loadFromFile(FieldSolutionID solutionID)" << solutionID.toString();

    m_solutionArrays.clear();

    QString fn = QString("%1_%2").
            arg(Util::problem()->config()->fileName().left(Util::problem()->config()->fileName().count() - 4)).
            arg(solutionID.toString());

    // load the mesh file
    QSharedPointer<Mesh> mesh = readMeshFromFile(QString("%1.mesh").arg(fn));

    for (int i = 0; i < solutionID.group->module()->numberOfSolutions(); i++)
    {
        QSharedPointer<Space<Scalar> > space(new H1Space<Scalar>(mesh.data()));
        space.data()->load((QString("%1_%2.spc").arg(fn).arg(i)).toStdString().c_str());

        QSharedPointer<Solution<Scalar> > sln(new Solution<Scalar>());
        sln.data()->load((QString("%1_%2.sln").arg(fn).arg(i)).toStdString().c_str(), mesh.data());

        SolutionArray<Scalar> solutionArray;
        solutionArray.space = SpaceAndMesh<Scalar>(space, mesh);
        solutionArray.sln = SolutionAndMesh<Scalar>(sln, mesh);
        solutionArray.time = 0.0;

        m_solutionArrays.append(solutionArray);
    }
}

template <typename Scalar>
void MultiSolutionArray<Scalar>::saveToFile(FieldSolutionID solutionID)
{
    // qDebug() << "void MultiSolutionArray<Scalar>::saveToFile(FieldSolutionID solutionID)" << solutionID.toString();

    QString fn = QString("%1_%2").
            arg(Util::problem()->config()->fileName().left(Util::problem()->config()->fileName().count() - 4)).
            arg(solutionID.toString());

    writeMeshToFile(QString("%1.mesh").arg(fn), m_solutionArrays.at(0).space.data()->get_mesh());

    int solutionIndex = 0;
    foreach (SolutionArray<Scalar> solutionArray, m_solutionArrays)
    {           
        // TODO: check write access
        solutionArray.space.data()->save((QString("%1_%2.spc").arg(fn).arg(solutionIndex)).toStdString().c_str());
        solutionArray.sln.data()->save((QString("%1_%2.sln").arg(fn).arg(solutionIndex)).toStdString().c_str());

        solutionIndex++;
    }
}


template class MultiSpace<double>;
template class MultiSolution<double>;
template class SolutionArray<double>;
template class MultiSolutionArray<double>;
template class SolutionID<FieldInfo>;
