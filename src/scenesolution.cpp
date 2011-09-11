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
#include "scenemarker.h"
#include "sceneview.h"
#include "progressdialog.h"
#include "hermes2d/module.h"
#include "hermes2d/solver.h"

template <typename Scalar>
SceneSolution<Scalar>::SceneSolution()
{
    logMessage("SceneSolution::SceneSolution()");

    m_progressDialog = new ProgressDialog();
    m_progressItemMesh = new ProgressItemMesh();
    m_progressItemSolve = new ProgressItemSolve();
    m_progressItemSolveAdaptiveStep = new ProgressItemSolveAdaptiveStep();
    m_progressItemProcessView = new ProgressItemProcessView();

    m_timeStep = -1;
    m_isSolving = false;

    m_meshInitial = NULL;
    m_slnContourView = NULL;
    m_slnScalarView = NULL;
    m_slnVectorXView = NULL;
    m_slnVectorYView = NULL;
}

template <typename Scalar>
SceneSolution<Scalar>::~SceneSolution()
{
    delete m_progressDialog;
    delete m_progressItemMesh;
    delete m_progressItemSolve;
    delete m_progressItemSolveAdaptiveStep;
    delete m_progressItemProcessView;
}

template <typename Scalar>
void SceneSolution<Scalar>::clear()
{
    logMessage("SceneSolution::clear()");

    m_timeStep = -1;

    // m_linInitialMeshView.free();
    // m_linSolutionMeshView.free();
    // m_linContourView.free();
    // m_linScalarView.free();
    // m_vecVectorView.free();

    // solution array
    for (int i = 0; i < m_solutionArrayList.size(); i++)
        delete m_solutionArrayList.at(i);
    m_solutionArrayList.clear();

    // mesh
    if (m_meshInitial)
    {
        delete m_meshInitial;
        m_meshInitial = NULL;
    }

    // countour
    if (m_slnContourView)
    {
        delete m_slnContourView;
        m_slnContourView = NULL;
    }
    
    // scalar
    if (m_slnScalarView)
    {
        delete m_slnScalarView;
        m_slnScalarView = NULL;
    }
    
    // vector
    if (m_slnVectorXView)
    {
        delete m_slnVectorXView;
        m_slnVectorXView = NULL;
    }
    if (m_slnVectorYView)
    {
        delete m_slnVectorYView;
        m_slnVectorYView = NULL;
    }

    m_progressDialog->clear();
}

template <typename Scalar>
void SceneSolution<Scalar>::solve(SolverMode solverMode)
{
    logMessage("SceneSolution::solve()");

    if (isSolving()) return;

    // clear problem
    if (solverMode == SolverMode_Mesh || solverMode == SolverMode_MeshAndSolve)
        clear();

    m_isSolving = true;

    // open indicator progress
    Indicator::openProgress();

    // save problem
    ErrorResult result = Util::scene()->writeToFile(tempProblemFileName() + ".a2d");
    if (result.isError())
        result.showDialog();

    if (solverMode == SolverMode_Mesh || solverMode == SolverMode_MeshAndSolve)
    {
        m_progressDialog->clear();
        m_progressDialog->appendProgressItem(m_progressItemMesh);
        if (solverMode == SolverMode_MeshAndSolve)
        {
            m_progressDialog->appendProgressItem(m_progressItemSolve);
            m_progressDialog->appendProgressItem(m_progressItemProcessView);
        }
    }
    else if (solverMode == SolverMode_SolveAdaptiveStep)
    {
        m_progressDialog->appendProgressItem(m_progressItemSolveAdaptiveStep);
        m_progressDialog->appendProgressItem(m_progressItemProcessView);
    }

    if (m_progressDialog->run())
    {
        Util::scene()->sceneSolution()->setTimeStep(Util::scene()->sceneSolution()->timeStepCount() - 1);
        emit meshed();
        emit solved();
    }

    if (isMeshed())
    {
        InitialCondition initial(m_meshInitial, 0.0);
        m_linInitialMeshView.process_solution(&initial);
    }

    // delete temp file
    if (Util::scene()->problemInfo()->fileName == tempProblemFileName() + ".a2d")
    {
        QFile::remove(Util::scene()->problemInfo()->fileName);
        Util::scene()->problemInfo()->fileName = "";
    }

    // close indicator progress
    Indicator::closeProgress();

    m_isSolving = false;
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

    Hermes::Hermes2D::Mesh *mesh = readMeshFromFile(tempProblemFileName() + ".mesh");
    // refineMesh(mesh, true, true);

    setMeshInitial(mesh);
}

template <typename Scalar>
void SceneSolution<Scalar>::saveMeshInitial(QDomDocument *doc, QDomElement element)
{
    logMessage("SceneSolution::saveMeshInitial()");

    if (isMeshed())
    {
        QString fileName = tempProblemFileName() + ".mesh";

        writeMeshFromFile(fileName, m_meshInitial);

        // read content
        QDomText text = doc->createTextNode(readFileContentByteArray(fileName).toBase64());
        element.appendChild(text);
    }
}

template <typename Scalar>
void SceneSolution<Scalar>::loadSolution(QDomElement element)
{
    logMessage("SceneSolution::loadSolution()");

    Hermes::vector<SolutionArray<Scalar> *> solutionArrayList;

    // constant solution cannot be saved
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        Util::scene()->problemInfo()->initialCondition.evaluate(true);

        SolutionArray<Scalar> *solutionArray = new SolutionArray<Scalar>();
        // solutionArray->order = new Hermes::Hermes2D::Views::Orderizer();
        solutionArray->sln = new Hermes::Hermes2D::Solution<Scalar>();
        // FIXME
        // solutionArray->sln->set_const(Util::scene()->sceneSolution()->meshInitial(), Util::scene()->problemInfo()->initialCondition.number());
        solutionArray->adaptiveError = 0.0;
        solutionArray->adaptiveSteps = 0.0;
        solutionArray->time = 0.0;

        solutionArrayList.push_back(solutionArray);
    }

    QDomNode n = element.firstChild();
    while(!n.isNull())
    {
        SolutionArray<Scalar> *solutionArray = new SolutionArray<Scalar>();
        solutionArray->load(n.toElement());

        // add to the array
        solutionArrayList.push_back(solutionArray);

        n = n.nextSibling();
    }

    if (solutionArrayList.size() > 0)
        setSolutionArrayList(solutionArrayList);
}

template <typename Scalar>
void SceneSolution<Scalar>::saveSolution(QDomDocument *doc, QDomElement element)
{
    logMessage("SceneSolution::saveSolution()");

    if (isSolved())
    {
        // constant solution cannot be saved
        int start = (Util::scene()->problemInfo()->analysisType != AnalysisType_Transient) ? 0 : 1;

        for (int i = start; i < timeStepCount(); i++)
        {
            QDomNode eleSolution = doc->createElement("solution");
            m_solutionArrayList.at(i)->save(doc, eleSolution.toElement());
            element.appendChild(eleSolution);
        }
    }
}

template <typename Scalar>
Hermes::Hermes2D::Solution<Scalar> *SceneSolution<Scalar>::sln(int i)
{
    logMessage("SceneSolution::sln()");

    int currentTimeStep = i;
    if (isSolved() && currentTimeStep < timeStepCount() * Util::scene()->problemInfo()->module()->number_of_solution())
    {
        // default
        if (currentTimeStep == -1)
            currentTimeStep = m_timeStep * Util::scene()->problemInfo()->module()->number_of_solution();

        if (m_solutionArrayList.at(currentTimeStep)->sln)
            return m_solutionArrayList.at(currentTimeStep)->sln;
    }
    return NULL;
}

template <typename Scalar>
double SceneSolution<Scalar>::adaptiveError()
{
    logMessage("SceneSolution::adaptiveError()");

    return (isSolved()) ? m_solutionArrayList.at(m_timeStep * Util::scene()->problemInfo()->module()->number_of_solution())->adaptiveError : 100.0;
}

template <typename Scalar>
int SceneSolution<Scalar>::adaptiveSteps()
{
    logMessage("SceneSolution::adaptiveSteps()");

    return (isSolved()) ? m_solutionArrayList.at(m_timeStep * Util::scene()->problemInfo()->module()->number_of_solution())->adaptiveSteps : 0.0;
}

template <typename Scalar>
int SceneSolution<Scalar>::findElementInVectorizer(Hermes::Hermes2D::Views::Vectorizer &vec, const Point &point) const
{
    logMessage("SceneSolution::findTriangleInVectorizer()");

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

#define sign(x) (( x > 0 ) - ( x < 0 ))

template <typename Scalar>
int SceneSolution<Scalar>::findElementInMesh(Hermes::Hermes2D::Mesh *mesh, const Point &point) const
{
    logMessage("SceneSolution::findTriangleInMesh()");

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

template <typename Scalar>
void SceneSolution<Scalar>::setMeshInitial(Hermes::Hermes2D::Mesh *meshInitial)
{
    if (m_meshInitial)
    {
        delete m_meshInitial;
    }

    m_meshInitial = meshInitial;
}

template <typename Scalar>
void SceneSolution<Scalar>::setSolutionArrayList(Hermes::vector<SolutionArray<Scalar> *> solutionArrayList)
{
    logMessage("SceneSolution::setSolutionArrayList()");

    for (int i = 0; i < m_solutionArrayList.size(); i++)
        delete m_solutionArrayList.at(i);
    m_solutionArrayList.clear();

    m_solutionArrayList = solutionArrayList;

    // if (!isSolving())
    setTimeStep(timeStepCount() - 1);
}

template <typename Scalar>
void SceneSolution<Scalar>::setTimeStep(int timeStep, bool showViewProgress)
{
    logMessage("SceneSolution::setTimeStep()");

    m_timeStep = timeStep;
    if (!isSolved()) return;

    emit timeStepChanged(showViewProgress);
}

template <typename Scalar>
int SceneSolution<Scalar>::timeStepCount() const
{
    logMessage("SceneSolution::timeStepCount()");

    return (m_solutionArrayList.size() > 0) ? m_solutionArrayList.size() / Util::scene()->problemInfo()->module()->number_of_solution() : 0;
}

template <typename Scalar>
double SceneSolution<Scalar>::time() const
{
    logMessage("SceneSolution::time()");

    if (isSolved())
    {
        if (m_solutionArrayList.at(m_timeStep * Util::scene()->problemInfo()->module()->number_of_solution())->sln)
            return m_solutionArrayList.at(m_timeStep * Util::scene()->problemInfo()->module()->number_of_solution())->time;
    }
    return 0.0;
}

template <typename Scalar>
void SceneSolution<Scalar>::setSlnContourView(ViewScalarFilter<Scalar> *slnScalarView)
{
    logMessage("SceneSolution::setSlnContourView()");

    if (m_slnContourView)
    {
        delete m_slnContourView;
        m_slnContourView = NULL;
    }
    
    m_slnContourView = slnScalarView;
    m_linContourView.process_solution(m_slnContourView,
                                      Hermes::Hermes2D::H2D_FN_VAL_0,
                                      Util::config()->linearizerQuality);

    // deformed shape
    if (Util::config()->deformContour)
        Util::scene()->problemInfo()->module()->deform_shape(m_linContourView.get_vertices(), m_linContourView.get_num_vertices());
}

template <typename Scalar>
void SceneSolution<Scalar>::setSlnScalarView(ViewScalarFilter<Scalar> *slnScalarView)
{
    logMessage("SceneSolution::setSlnScalarView()");

    if (m_slnScalarView)
    {
        delete m_slnScalarView;
        m_slnScalarView = NULL;
    }
    
    m_slnScalarView = slnScalarView;
    // QTime time;
    // time.start();
    m_linScalarView.process_solution(m_slnScalarView,
                                     Hermes::Hermes2D::H2D_FN_VAL_0,
                                     Util::config()->linearizerQuality);
    /*
    m_linScalarView.save_solution_vtk(m_slnScalarView, "/home/karban/xxx.vtk", "xxx");
    m_linScalarView.process_solution(m_slnScalarView,
                                     Hermes::Hermes2D::H2D_FN_VAL_0,
                                     0.000001);
    */
    // qDebug() << "linScalarView.process_solution: " << time.elapsed();

    // deformed shape
    if (Util::config()->deformScalar)
        Util::scene()->problemInfo()->module()->deform_shape(m_linScalarView.get_vertices(),
                                                             m_linScalarView.get_num_vertices());
}

template <typename Scalar>
void SceneSolution<Scalar>::setSlnVectorView(ViewScalarFilter<Scalar> *slnVectorXView, ViewScalarFilter<Scalar> *slnVectorYView)
{
    logMessage("SceneSolution::setSlnVectorView()");

    if (m_slnVectorXView)
    {
        delete m_slnVectorXView;
        m_slnVectorXView = NULL;
    }
    if (m_slnVectorYView)
    {
        delete m_slnVectorYView;
        m_slnVectorYView = NULL;
    }
    
    m_slnVectorXView = slnVectorXView;
    m_slnVectorYView = slnVectorYView;

    m_vecVectorView.process_solution(m_slnVectorXView, m_slnVectorYView,
                                     Hermes::Hermes2D::H2D_FN_VAL_0, Hermes::Hermes2D::H2D_FN_VAL_0,
                                     Hermes::Hermes2D::Views::HERMES_EPS_LOW);

    // deformed shape
    if (Util::config()->deformVector)
        Util::scene()->problemInfo()->module()->deform_shape(m_vecVectorView.get_vertices(), m_vecVectorView.get_num_vertices());
}

template <typename Scalar>
void SceneSolution<Scalar>::setOrderView(Hermes::Hermes2D::Space<Scalar> *space)
{
    logMessage("SceneSolution::setOrderView()");

    m_orderView.process_space(space);
}

template <typename Scalar>
void SceneSolution<Scalar>::processView(bool showViewProgress)
{
    if (showViewProgress)
    {
        m_progressDialog->clear();
        m_progressDialog->appendProgressItem(m_progressItemProcessView);
        m_progressDialog->run(showViewProgress);
    }
    else
    {
        m_progressItemProcessView->setSteps();
        m_progressItemProcessView->run(true);
    }
}

template <typename Scalar>
void SceneSolution<Scalar>::processSolutionMesh()
{
    logMessage("SceneSolution::processSolutionMesh()");

    if (isSolved())
    {
        InitialCondition initial(sln()->get_mesh(), 0.0);
        m_linSolutionMeshView.process_solution(&initial);

        emit processedSolutionMesh();
    }
}

template <typename Scalar>
void SceneSolution<Scalar>::processRangeContour()
{
    logMessage("SceneSolution::processRangeContour()");

    if (isSolved() && sceneView()->sceneViewSettings().contourPhysicFieldVariable != "")
    {
        ViewScalarFilter<Scalar> *viewScalarFilter;
        if (Util::scene()->problemInfo()->module()->get_variable(sceneView()->sceneViewSettings().contourPhysicFieldVariable)->is_scalar)
            viewScalarFilter = Util::scene()->problemInfo()->module()->view_scalar_filter(Util::scene()->problemInfo()->module()->get_variable(sceneView()->sceneViewSettings().contourPhysicFieldVariable),
                                                                                          PhysicFieldVariableComp_Scalar);
        else
            viewScalarFilter = Util::scene()->problemInfo()->module()->view_scalar_filter(Util::scene()->problemInfo()->module()->get_variable(sceneView()->sceneViewSettings().contourPhysicFieldVariable),
                                                                                          PhysicFieldVariableComp_Magnitude);

        setSlnContourView(viewScalarFilter);
        emit processedRangeContour();
    }
}

template <typename Scalar>
void SceneSolution<Scalar>::processRangeScalar()
{
    logMessage("SceneSolution::processRangeScalar()");

    if (isSolved() && sceneView()->sceneViewSettings().scalarPhysicFieldVariable != "")
    {
        ViewScalarFilter<Scalar> *viewScalarFilter = Util::scene()->problemInfo()->module()->view_scalar_filter(Util::scene()->problemInfo()->module()->get_variable(sceneView()->sceneViewSettings().scalarPhysicFieldVariable),
                                                                                                                sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp);

        setSlnScalarView(viewScalarFilter);
        emit processedRangeScalar();
    }
}

template <typename Scalar>
void SceneSolution<Scalar>::processRangeVector()
{
    logMessage("SceneSolution::processRangeVector()");

    if (isSolved() && sceneView()->sceneViewSettings().vectorPhysicFieldVariable != "")
    {
        ViewScalarFilter<Scalar> *viewVectorXFilter = Util::scene()->problemInfo()->module()->view_scalar_filter(Util::scene()->problemInfo()->module()->get_variable(sceneView()->sceneViewSettings().vectorPhysicFieldVariable),
                                                                                                                 PhysicFieldVariableComp_X);

        ViewScalarFilter<Scalar> *viewVectorYFilter = Util::scene()->problemInfo()->module()->view_scalar_filter(Util::scene()->problemInfo()->module()->get_variable(sceneView()->sceneViewSettings().vectorPhysicFieldVariable),
                                                                                                                 PhysicFieldVariableComp_Y);

        setSlnVectorView(viewVectorXFilter, viewVectorYFilter);
        emit processedRangeVector();
    }
}

template <typename Scalar>
void SceneSolution<Scalar>::processOrder()
{
    logMessage("SceneSolution::processOrder()");

    setOrderView(m_solutionArrayList.at(m_timeStep * Util::scene()->problemInfo()->module()->number_of_solution())->space);
    // emit processedRangeVector();
}

template <typename Scalar>
ProgressDialog *SceneSolution<Scalar>::progressDialog()  //TODO PK <double>
{
    return m_progressDialog;
}


template class SceneSolution<double>;
