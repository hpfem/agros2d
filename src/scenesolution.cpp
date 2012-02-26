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

#include "scenesolution.h"

#include "scene.h"
#include "scenemarkerdialog.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"
#include "progressdialog.h"
#include "hermes2d/solver.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"

int findElementInMesh(Hermes::Hermes2D::Mesh *mesh, const Point &point)
{
    logMessage("findElementInMesh()");

    for (int i = 0; i < mesh->get_num_active_elements(); i++)
    {
        Hermes::Hermes2D::Element *element = mesh->get_element_fast(i);

        bool inElement = false;
        int j;
        int npol = (element->is_triangle()) ? 3 : 4;

        for (int i = 0, j = npol-1; i < npol; j = i++) {
            if ((((element->vn[i]->y <= point.y) && (point.y < element->vn[j]->y)) ||
                 ((element->vn[j]->y <= point.y) && (point.y < element->vn[i]->y))) &&
                    (point.x < (element->vn[j]->x - element->vn[i]->x) * (point.y - element->vn[i]->y)
                     / (element->vn[j]->y - element->vn[i]->y) + element->vn[i]->x))
                inElement = !inElement;
        }

        if (inElement)
            return i;
    }

    return -1;
}

int findElementInVectorizer(Hermes::Hermes2D::Views::Vectorizer &vec, const Point &point)
{
    logMessage("findElementInVectorizer()");

    double4* vecVert = vec.get_vertices();
    int3* vecTris = vec.get_triangles();

    for (int i = 0; i < vec.get_num_triangles(); i++)
    {
        bool inElement = true;

        // triangle element
        for (int l = 0; l < 3; l++)
        {
            int k = l + 1;
            if (k == 3)
                k = 0;

            double z = (vecVert[vecTris[i][k]][0] - vecVert[vecTris[i][l]][0]) * (point.y - vecVert[vecTris[i][l]][1]) -
                    (vecVert[vecTris[i][k]][1] - vecVert[vecTris[i][l]][1]) * (point.x - vecVert[vecTris[i][l]][0]);

            if (z < 0)
            {
                inElement = false;
                break;
            }
        }

        if (inElement)
            return i;
    }

    return -1;
}

template <typename Scalar>
SceneSolution<Scalar>::SceneSolution(FieldInfo* fieldInfo) : m_fieldInfo(fieldInfo)
{
    logMessage("SceneSolution::SceneSolution()");


    //m_timeStep = -1;
    //m_isSolving = false;
}

template <typename Scalar>
SceneSolution<Scalar>::~SceneSolution()
{
}

template <typename Scalar>
void SceneSolution<Scalar>::clear(bool all)
{
    logMessage("SceneSolution::clear()");

    // solution array
    if (all)
    {
        //m_timeStep = -1;

        //        for (int i = 0; i < m_solutionArrayList.size(); i++)
        //            delete m_solutionArrayList.at(i);
        m_solutionArrayList.clear();


    }



    //m_progressDialog->clear();
}


template <typename Scalar>
void SceneSolution<Scalar>::loadMeshInitial(QDomElement element)
{
    logMessage("SceneSolution::loadMeshInitial()");

    QDomText text = element.childNodes().at(0).toText();

    // write content (saved mesh)
    QString fileName = tempProblemFileName() + ".mesh";
    QByteArray content;
    content.append(text.nodeValue());
    writeStringContentByteArray(fileName, QByteArray::fromBase64(content));

    Hermes::Hermes2D::Mesh *mesh = readMeshFromFile(tempProblemFileName() + ".xml");
    // refineMesh(mesh, true, true);

    // setMeshInitial(mesh);
}

template <typename Scalar>
void SceneSolution<Scalar>::saveMeshInitial(QDomDocument *doc, QDomElement element)
{
    logMessage("SceneSolution::saveMeshInitial()");

    assert(0);
    //    if (isMeshed())
    //    {
    //        QString fileName = tempProblemFileName() + ".mesh";

    //        writeMeshFromFile(fileName, m_meshInitial);

    //        // read content
    //        QDomText text = doc->createTextNode(readFileContentByteArray(fileName).toBase64());
    //        element.appendChild(text);
    //    }
}

template <typename Scalar>
void SceneSolution<Scalar>::loadSolution(QDomElement element)
{
    assert(0); //TODO
    //    logMessage("SceneSolution::loadSolution()");

    //    Hermes::vector<SolutionArray<Scalar> *> solutionArrayList;

    //    // constant solution cannot be saved
    //    if (Util::scene()->problemInfo()->analysisType() == AnalysisType_Transient)
    //    {
    //        Util::scene()->problemInfo()->initialCondition.evaluate(true);

    //        SolutionArray<Scalar> *solutionArray = new SolutionArray<Scalar>();
    //        // solutionArray->order = new Hermes::Hermes2D::Views::Orderizer();
    //        solutionArray->sln = new Hermes::Hermes2D::Solution<Scalar>();
    //        // FIXME
    //        // solutionArray->sln->set_const(Util::scene()->sceneSolution()->meshInitial(), Util::scene()->problemInfo()->initialCondition.number());
    //        solutionArray->adaptiveError = 0.0;
    //        solutionArray->adaptiveSteps = 0.0;
    //        solutionArray->time = 0.0;

    //        solutionArrayList.push_back(solutionArray);
    //    }

    //    QDomNode n = element.firstChild();
    //    while(!n.isNull())
    //    {
    //        SolutionArray<Scalar> *solutionArray = new SolutionArray<Scalar>();
    //        solutionArray->load(n.toElement());

    //        // add to the array
    //        solutionArrayList.push_back(solutionArray);

    //        n = n.nextSibling();
    //    }

    //    if (solutionArrayList.size() > 0)
    //        setSolutionArrayList(solutionArrayList);
}

template <typename Scalar>
void SceneSolution<Scalar>::saveSolution(QDomDocument *doc, QDomElement element)
{
    assert(0); //TODO
    //    logMessage("SceneSolution::saveSolution()");

    //    if (isSolved())
    //    {
    //        // constant solution cannot be saved
    //        int start = (Util::scene()->problemInfo()->analysisType() != AnalysisType_Transient) ? 0 : 1;

    //        for (int i = start; i < timeStepCount(); i++)
    //        {
    //            QDomNode eleSolution = doc->createElement("solution");
    //            m_solutionArrayList.at(i)->save(doc, eleSolution.toElement());
    //            element.appendChild(eleSolution);
    //        }
    //    }
}

template <typename Scalar>
SolutionArray<Scalar> SceneSolution<Scalar>::solutionArray(int i)
{
    logMessage("SceneSolution::solutionArray()");

    //    assert(i == 0);
    return m_solutionArrayList.at(i);

    //    int currentTimeStep = i;
    //    if (isSolved() && currentTimeStep < timeStepCount() * Util::scene()->fieldInfo("TODO")->module()->number_of_solution())
    //    {
    //        // default
    //        if (currentTimeStep == -1)
    //            currentTimeStep = m_timeStep * Util::scene()->fieldInfo("TODO")->module()->number_of_solution();

    //        if (m_solutionArrayList.at(currentTimeStep))
    //            return m_solutionArrayList.at(currentTimeStep);
    //    }
    //    return NULL;
}

template <typename Scalar>
Hermes::Hermes2D::Solution<Scalar> *SceneSolution<Scalar>::sln(int i)
{
    logMessage("SceneSolution::sln()");

    return solutionArray(i).sln.get();
}

template <typename Scalar>
Hermes::Hermes2D::Space<Scalar> *SceneSolution<Scalar>::space(int i)
{
    logMessage("SceneSolution::space()");

    return solutionArray(i).space.get();
}

//template <typename Scalar>
//double SceneSolution<Scalar>::adaptiveError()
//{
//    assert(0); //TODO
////    logMessage("SceneSolution::adaptiveError()");

////    return (isSolved()) ? m_solutionArrayList.at(m_timeStep * Util::scene()->problemInfo()->module()->number_of_solution())->adaptiveError : 100.0;
//}

//template <typename Scalar>
//int SceneSolution<Scalar>::adaptiveSteps()
//{
//    assert(0); //TODO
////    logMessage("SceneSolution::adaptiveSteps()");

////    return (isSolved()) ? m_solutionArrayList.at(m_timeStep * Util::scene()->problemInfo()->module()->number_of_solution())->adaptiveSteps : 0.0;
//}

#define sign(x) (( x > 0 ) - ( x < 0 ))

/*
template <typename Scalar>
void SceneSolution<Scalar>::setMeshInitial(Hermes::Hermes2D::Mesh *meshInitial)
{
    if (m_meshInitial)
    {
        delete m_meshInitial;
    }

    m_meshInitial = meshInitial;
}
*/


template <typename Scalar>
void SceneSolution<Scalar>::setSolutionArrayList(Hermes::vector<SolutionArray<Scalar> > solutionArrayList)
{
    logMessage("SceneSolution::setSolutionArrayList()");

    //    for (int i = 0; i < m_solutionArrayList.size(); i++)
    //        delete m_solutionArrayList.at(i);
    m_solutionArrayList.clear();

    m_solutionArrayList = solutionArrayList;

    // if (!isSolving())
    //setTimeStep(timeStepCount() - 1);
}

//template <typename Scalar>
//void SceneSolution<Scalar>::setTimeStep(int timeStep, bool showViewProgress)
//{
//    logMessage("SceneSolution::setTimeStep()");

//    m_timeStep = timeStep;
//    if (!isSolved()) return;

//    emit timeStepChanged(showViewProgress);
//}

//template <typename Scalar>
//int SceneSolution<Scalar>::timeStepCount() const
//{
//    logMessage("SceneSolution::timeStepCount()");

//    return (m_solutionArrayList.size() > 0) ? m_solutionArrayList.size() / Util::scene()->fieldInfo("TODO")->module()->number_of_solution() : 0;
//}

//template <typename Scalar>
//double SceneSolution<Scalar>::time() const
//{
//    assert(0); //TODO
////    logMessage("SceneSolution::time()");

////    if (isSolved())
////    {
////        if (m_solutionArrayList.at(m_timeStep * Util::scene()->problemInfo()->module()->number_of_solution())->sln)
////            return m_solutionArrayList.at(m_timeStep * Util::scene()->problemInfo()->module()->number_of_solution())->time;
////    }
////    return 0.0;
//}
/*
template <typename Scalar>
void SceneSolution<Scalar>::processView(bool showViewProgress)
{
    int step = 0;

    // process order
    Util::scene()->activeSceneSolution()->processOrder();

    if (Util::config()->showSolutionMeshView)
    {
        step++;
        //emit message(tr("Processing solution mesh cache"), false, step);
        Util::scene()->activeSceneSolution()->processSolutionMesh();
    }
    if (Util::config()->showContourView)
    {
        step++;
        //emit message(tr("Processing contour view cache"), false, step);
        Util::scene()->activeSceneSolution()->processRangeContour();
    }
    if (Util::config()->showScalarView)
    {
        step++;
        //emit message(tr("Processing scalar view cache"), false, step);
        cout << "process Range Scalar" << endl;
        Util::scene()->activeSceneSolution()->processRangeScalar();
    }
    if (Util::config()->showVectorView)
    {
        step++;
        //emit message(tr("Processing vector view cache"), false, step);
        Util::scene()->activeSceneSolution()->processRangeVector();
    }


    //    if (showViewProgress)
    //    {
    //        m_progressDialog->clear();
    //        m_progressDialog->appendProgressItem(m_progressItemProcessView);
    //        m_progressDialog->run(showViewProgress);
    //    }
    //    else
    //    {
    //        m_progressItemProcessView->setSteps();
    //        m_progressItemProcessView->run(true);
    //    }
}
*/

template <typename Scalar>
void SceneSolution<Scalar>::setSolutionArray(QList<SolutionArray<Scalar> > solutionArrays)
{
    assert(solutionArrays.size() == fieldInfo()->module()->number_of_solution());

    for(int i = 0; i < solutionArrays.size(); i++)
        m_solutionArrayList.push_back(solutionArrays[i]);
}

template class SceneSolution<double>;
