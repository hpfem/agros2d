#include "solutiontypes.h"
#include "scene.h"
#include "problem.h"
#include "module_agros.h"
using namespace Hermes::Hermes2D;

FieldSolutionID BlockSolutionID::fieldSolutionID(FieldInfo* fieldInfo)
{
    bool contains = false;
    foreach(Field* field, group->m_fields)
    {
        if(field->fieldInfo() == fieldInfo)
            contains = true;
    }
    assert(contains);

    return FieldSolutionID(fieldInfo, timeStep, adaptivityStep, solutionType);
}

BlockSolutionID FieldSolutionID::blockSolutionID(Block *block)
{
    assert(block->contains(this->group));
    return BlockSolutionID(block, timeStep, adaptivityStep, solutionType);
}

template <typename Scalar>
SolutionArray<Scalar>::SolutionArray()
{
    logMessage("SolutionArray::SolutionArray()");

    sln.reset();
    space.reset();

    time = 0.0;
    adaptiveSteps = 0;
    adaptiveError = 100.0;
}

template <typename Scalar>
SolutionArray<Scalar>::SolutionArray(shared_ptr<Hermes::Hermes2D::Solution<Scalar> > sln, shared_ptr<Hermes::Hermes2D::Space<Scalar> > space,
                                     double adaptiveError, double adaptiveSteps, double time)
{
    this->sln = sln;
    this->space = space;
    this->adaptiveError = adaptiveError;
    this->adaptiveSteps = adaptiveSteps;
    this->time = time;
}

template <typename Scalar>
SolutionArray<Scalar>::~SolutionArray()
{
    logMessage("SolutionArray::~SolutionArray()");
}

template <typename Scalar>
void SolutionArray<Scalar>::load(QDomElement element)
{
    logMessage("SolutionArray::load()");

    QString fileNameSolution = tempProblemFileName() + ".sln";
    QString fileNameSpace = tempProblemFileName() + ".spc";

    // write content (saved solution)
    QByteArray contentSolution;
    contentSolution.append(element.elementsByTagName("sln").at(0).toElement().childNodes().at(0).nodeValue());
    writeStringContentByteArray(fileNameSolution, QByteArray::fromBase64(contentSolution));

    // write content (saved space)
    QByteArray contentSpace;
    contentSpace.append(element.elementsByTagName("space").at(0).toElement().childNodes().at(0).nodeValue());
    writeStringContentByteArray(fileNameSpace, QByteArray::fromBase64(contentSpace));

    sln = shared_ptr<Solution<Scalar> >(new Hermes::Hermes2D::Solution<Scalar>());
    // sln->load(fileNameSolution.toStdString().c_str());
    //space = new Hermes::Hermes2D::Space<Scalar>();
    //space->load(fileNameSpace.toStdString().c_str());
    adaptiveError = element.attribute("adaptiveerror").toDouble();
    adaptiveSteps = element.attribute("adaptivesteps").toInt();
    time = element.attribute("time").toDouble();

    // delete solution
    QFile::remove(fileNameSolution);
    QFile::remove(fileNameSpace);
}

template <typename Scalar>
void SolutionArray<Scalar>::save(QDomDocument *doc, QDomElement element)
{
    logMessage("SolutionArray::save()");

    // solution
    QString fileNameSolution = tempProblemFileName() + ".sln";
    // sln->save(fileNameSolution.toStdString().c_str(), false);
    QDomText textSolution = doc->createTextNode(readFileContentByteArray(fileNameSolution).toBase64());

    // space
    QString fileNameSpace = tempProblemFileName() + ".spc";
    //space->save_data(fileNameSpace.toStdString().c_str());
    QDomNode textSpace = doc->createTextNode(readFileContentByteArray(fileNameSpace).toBase64());

    QDomNode eleSolution = doc->createElement("sln");
    QDomNode eleSpace = doc->createElement("space");

    eleSolution.appendChild(textSolution);
    eleSpace.appendChild(textSpace);

    element.setAttribute("adaptiveerror", adaptiveError);
    element.setAttribute("adaptivesteps", adaptiveSteps);
    element.setAttribute("time", time);
    element.appendChild(eleSolution);
    element.appendChild(eleSpace);

    // delete
    QFile::remove(fileNameSolution);
    QFile::remove(fileNameSpace);
}

// *********************************************************************************************

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
Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > MultiSolutionArray<Scalar>::spaces()
{
    Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > spaces;

    foreach(SolutionArray<Scalar> solutionArray, m_solutionArrays)
    {
        spaces.push_back(solutionArray.space);
    }

    return spaces;
}

template <typename Scalar>
Hermes::vector<shared_ptr<Hermes::Hermes2D::Solution<Scalar> > > MultiSolutionArray<Scalar>::solutions()
{
    Hermes::vector<shared_ptr<Hermes::Hermes2D::Solution<Scalar> > > solutions;

    foreach(SolutionArray<Scalar> solutionArray, m_solutionArrays)
    {
        solutions.push_back(solutionArray.sln);
    }

    return solutions;
}

template <typename Scalar>
Hermes::vector<Hermes::Hermes2D::Solution<Scalar>* > MultiSolutionArray<Scalar>::solutionsNaked()
{
    return desmartize(solutions());
}

template <typename Scalar>
ostream& operator<<(ostream& output, const SolutionID<Scalar>& id) {
    output << "(" << id.group << ", timeStep " << id.timeStep << ", adaptStep " <<
              id.adaptivityStep << ", type "<< id.solutionType << ")";
    return output;
}

template <typename Scalar>
MultiSolutionArray<Scalar> MultiSolutionArray<Scalar>::copySpaces()
{
    MultiSolutionArray<Scalar> msa;
    foreach(SolutionArray<Scalar> solutionArray, m_solutionArrays)
    {
        msa.addComponent(SolutionArray<Scalar>(shared_ptr<Solution<Scalar> >(), solutionArray.space, 0, 0, 0));
    }

    return msa;
}

template <typename Scalar>
void MultiSolutionArray<Scalar>::setSpace(shared_ptr<Hermes::Hermes2D::Space<Scalar> > space, int component)
{

    SolutionArray<Scalar> newSA(m_solutionArrays.at(component));
    newSA.space = space;
    m_solutionArrays.replace(component, newSA);
}

template <typename Scalar>
void MultiSolutionArray<Scalar>::setSpaces(Hermes::vector<shared_ptr<Hermes::Hermes2D::Space<Scalar> > > spaces)
{
    assert(m_solutionArrays.size() == spaces.size());
    for(int comp = 0; comp < spaces.size(); comp++)
    {
        setSpace(spaces.at(comp), comp);
    }
}

template <typename Scalar>
void MultiSolutionArray<Scalar>::setSolution(shared_ptr<Hermes::Hermes2D::Solution<Scalar> > solution, int component)
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
    int numSol = fieldInfo->module()->number_of_solution();
    for(int comp = offset; comp < offset + numSol; comp++)
    {
        msa.addComponent(this->component(comp));
    }
    return msa;
}

template class SolutionArray<double>;
template class MultiSolutionArray<double>;
