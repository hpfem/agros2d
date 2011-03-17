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

SceneSolution::SceneSolution()
{
    logMessage("SceneSolution::SceneSolution()");

    m_timeStep = -1;
    m_isSolving = false;

    m_meshInitial = NULL;
    m_slnContourView = NULL;
    m_slnScalarView = NULL;
    m_slnVectorXView = NULL;
    m_slnVectorYView = NULL;   
}

void SceneSolution::clear()
{
    logMessage("SceneSolution::clear()");

    m_timeStep = -1;

    // solution array
    if (!m_solutionArrayList.isEmpty())
    {
        for (int i = 0; i < m_solutionArrayList.count(); i++)
            delete m_solutionArrayList.at(i);
        m_solutionArrayList.clear();
    }

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
}

void SceneSolution::solve(SolverMode solverMode)
{
    logMessage("SceneSolution::solve()");

    if (isSolving()) return;

    // clear problem
    clear();

    m_isSolving = true;

    // save problem
    ErrorResult result = Util::scene()->writeToFile(tempProblemFileName() + ".a2d");
    if (result.isError())
        result.showDialog();

    ProgressDialog progressDialog;
    progressDialog.appendProgressItem(new ProgressItemMesh());
    if (solverMode == SolverMode_MeshAndSolve)
    {
        progressDialog.appendProgressItem(new ProgressItemSolve());
        progressDialog.appendProgressItem(new ProgressItemProcessView());
    }

    if (progressDialog.run())
    {
        Util::scene()->sceneSolution()->setTimeStep(Util::scene()->sceneSolution()->timeStepCount() - 1);
        emit meshed();
        emit solved();
    }

    if (isMeshed())
    {
        Solution tsln;
        tsln.set_zero(m_meshInitial);
        m_linInitialMeshView.process_solution(&tsln);
    }

    // delete temp file
    if (Util::scene()->problemInfo()->fileName == tempProblemFileName() + ".a2d")
    {
        QFile::remove(Util::scene()->problemInfo()->fileName);
        Util::scene()->problemInfo()->fileName = "";
    }

    m_isSolving = false;
}

void SceneSolution::loadMeshInitial(QDomElement *element)
{
    logMessage("SceneSolution::loadMeshInitial()");

    QDomText text = element->childNodes().at(0).toText();

    // write content (saved mesh)
    QString fileName = tempProblemFileName() + ".mesh";
    QByteArray content;
    content.append(text.nodeValue());
    writeStringContentByteArray(fileName, QByteArray::fromBase64(content));

    Mesh *mesh = readMeshFromFile(tempProblemFileName() + ".mesh");
    // refineMesh(mesh, true, true);

    setMeshInitial(mesh);
}

void SceneSolution::saveMeshInitial(QDomDocument *doc, QDomElement *element)
{
    logMessage("SceneSolution::saveMeshInitial()");

    if (isMeshed())
    {
        QString fileName = tempProblemFileName() + ".mesh";

        writeMeshFromFile(fileName, m_meshInitial);

        // read content
        QDomText text = doc->createTextNode(readFileContentByteArray(fileName).toBase64());
        element->appendChild(text);
    }
}

void SceneSolution::loadSolution(QDomElement *element)
{
    logMessage("SceneSolution::loadSolution()");

    QList<SolutionArray *> solutionArrayList;

    // constant solution cannot be saved
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        Util::scene()->problemInfo()->initialCondition.evaluate(true);

        SolutionArray *solutionArray = new SolutionArray();
        solutionArray->order = new Orderizer();
        solutionArray->sln = new Solution();
        solutionArray->sln->set_const(Util::scene()->sceneSolution()->meshInitial(), Util::scene()->problemInfo()->initialCondition.number);
        solutionArray->adaptiveError = 0.0;
        solutionArray->adaptiveSteps = 0.0;
        solutionArray->time = 0.0;

        solutionArrayList.append(solutionArray);
    }

    QDomNode n = element->firstChild();
    while(!n.isNull())
    {
        SolutionArray *solutionArray = new SolutionArray();
        solutionArray->load(&n.toElement());

        // add to the array
        solutionArrayList.append(solutionArray);

        n = n.nextSibling();
    }

    if (!solutionArrayList.isEmpty())
        setSolutionArrayList(solutionArrayList);
}

void SceneSolution::saveSolution(QDomDocument *doc, QDomElement *element)
{
    logMessage("SceneSolution::saveSolution()");

    if (isSolved())
    {
        // constant solution cannot be saved
        int start = (Util::scene()->problemInfo()->analysisType != AnalysisType_Transient) ? 0 : 1;

        for (int i = start; i < timeStepCount(); i++)
        {
            QDomNode eleSolution = doc->createElement("solution");
            m_solutionArrayList.at(i)->save(doc, &eleSolution.toElement());
            element->appendChild(eleSolution);
        }
    }
}

Solution *SceneSolution::sln(int i)
{
    logMessage("SceneSolution::sln()");

    int currentTimeStep = i;
    if (isSolved() && currentTimeStep < timeStepCount() * Util::scene()->problemInfo()->hermes()->numberOfSolution())
    {
        // default
        if (currentTimeStep == -1)
            currentTimeStep = m_timeStep * Util::scene()->problemInfo()->hermes()->numberOfSolution();

        if (m_solutionArrayList.value(currentTimeStep)->sln)
            return m_solutionArrayList.value(currentTimeStep)->sln;
    }
    return NULL;
}

Orderizer &SceneSolution::ordView()
{
    logMessage("SceneSolution::ordView()");

    if (isSolved())
        return *m_solutionArrayList.value(m_timeStep * Util::scene()->problemInfo()->hermes()->numberOfSolution())->order;
}

double SceneSolution::adaptiveError()
{
    logMessage("SceneSolution::adaptiveError()");

    return (isSolved()) ? m_solutionArrayList.value(m_timeStep * Util::scene()->problemInfo()->hermes()->numberOfSolution())->adaptiveError : 100.0;
}

int SceneSolution::adaptiveSteps()
{
    logMessage("SceneSolution::adaptiveSteps()");

    return (isSolved()) ? m_solutionArrayList.value(m_timeStep * Util::scene()->problemInfo()->hermes()->numberOfSolution())->adaptiveSteps : 0.0;
}

int SceneSolution::findTriangleInVectorizer(const Vectorizer &vec, const Point &point)
{
    logMessage("SceneSolution::findTriangleInVectorizer()");

    double4* vecVert = vec.get_vertices();
    int3* vecTris = vec.get_triangles();

    for (int i = 0; i < vec.get_num_triangles(); i++)
    {
        bool inTriangle = true;

        int k;
        double z;
        for (int l = 0; l < 3; l++)
        {
            k = l + 1;
            if (k == 3)
                k = 0;

            z = (vecVert[vecTris[i][k]][0] - vecVert[vecTris[i][l]][0]) * (point.y - vecVert[vecTris[i][l]][1]) -
                (vecVert[vecTris[i][k]][1] - vecVert[vecTris[i][l]][1]) * (point.x - vecVert[vecTris[i][l]][0]);

            if (z < 0)
            {
                inTriangle = false;
                break;
            }
        }

        if (inTriangle)
            return i;
    }

    return -1;
}

int SceneSolution::findTriangleInMesh(Mesh *mesh, const Point &point)
{
    logMessage("SceneSolution::findTriangleInMesh()");

    for (int i = 0; i < mesh->get_num_active_elements(); i++)
    {
        bool inTriangle = true;
        
        Element *element = mesh->get_element_fast(i);
        
        int k;
        double z;
        for (int l = 0; l < 3; l++)
        {
            k = l + 1;
            if (k == 3)
                k = 0;
            
            z = (element->vn[k]->x - element->vn[l]->x) * (point.y - element->vn[l]->y) - (element->vn[k]->y - element->vn[l]->y) * (point.x - element->vn[l]->x);
            
            if (z < 0)
            {
                inTriangle = false;
                break;
            }
        }
        
        if (inTriangle)
            return i;
    }
    
    return -1;
}

void SceneSolution::setSolutionArrayList(QList<SolutionArray *> solutionArrayList)
{
    logMessage("SceneSolution::setSolutionArrayList()");

    if (!m_solutionArrayList.isEmpty())
    {
        for (int i = 0; i < m_solutionArrayList.count(); i++)
            delete m_solutionArrayList.at(i);
        m_solutionArrayList.clear();
    }

    m_solutionArrayList = solutionArrayList;

    // if (!isSolving())
    setTimeStep(timeStepCount() - 1);
}

void SceneSolution::setTimeStep(int timeStep, bool showViewProgress)
{
    logMessage("SceneSolution::setTimeStep()");

    m_timeStep = timeStep;
    if (!isSolved()) return;

    emit timeStepChanged(showViewProgress);
}

int SceneSolution::timeStepCount()
{
    logMessage("SceneSolution::timeStepCount()");

    return (!m_solutionArrayList.isEmpty()) ? m_solutionArrayList.count() / Util::scene()->problemInfo()->hermes()->numberOfSolution() : 0;
}

double SceneSolution::time()
{
    logMessage("SceneSolution::time()");

    if (isSolved())
    {
        if (m_solutionArrayList.value(m_timeStep * Util::scene()->problemInfo()->hermes()->numberOfSolution())->sln)
            return m_solutionArrayList.value(m_timeStep * Util::scene()->problemInfo()->hermes()->numberOfSolution())->time;
    }
    return 0.0;
}

void SceneSolution::setSlnContourView(ViewScalarFilter *slnScalarView)
{
    logMessage("SceneSolution::setSlnContourView()");

    if (m_slnContourView)
    {
        delete m_slnContourView;
        m_slnContourView = NULL;
    }
    
    m_slnContourView = slnScalarView;
    m_linContourView.process_solution(m_slnContourView);

    // deformed shape
    if (Util::config()->deformContour)
        Util::scene()->problemInfo()->hermes()->deformShape(m_linContourView.get_vertices(), m_linContourView.get_num_vertices());
}

void SceneSolution::setSlnScalarView(ViewScalarFilter *slnScalarView)
{
    logMessage("SceneSolution::setSlnScalarView()");

    if (m_slnScalarView)
    {
        delete m_slnScalarView;
        m_slnScalarView = NULL;
    }
    
    m_slnScalarView = slnScalarView;
    m_linScalarView.process_solution(m_slnScalarView, H2D_FN_VAL_0);

    // deformed shape
    if (Util::config()->deformScalar)
        Util::scene()->problemInfo()->hermes()->deformShape(m_linScalarView.get_vertices(), m_linScalarView.get_num_vertices());
}

void SceneSolution::setSlnVectorView(ViewScalarFilter *slnVectorXView, ViewScalarFilter *slnVectorYView)
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
    
    m_vecVectorView.process_solution(m_slnVectorXView, H2D_FN_VAL_0, m_slnVectorYView, H2D_FN_VAL_0, HERMES_EPS_LOW);

    // deformed shape
    if (Util::config()->deformVector)
        Util::scene()->problemInfo()->hermes()->deformShape(m_vecVectorView.get_vertices(), m_vecVectorView.get_num_vertices());
}

void SceneSolution::processSolutionMesh()
{
    logMessage("SceneSolution::processSolutionMesh()");

    if (isSolved())
    {
        Solution tsln;
        tsln.set_zero(sln()->get_mesh());
        m_linSolutionMeshView.process_solution(&tsln);

        emit processedSolutionMesh();
    }
}

void SceneSolution::processRangeContour()
{
    logMessage("SceneSolution::processRangeContour()");

    if (isSolved())
    {
        ViewScalarFilter *viewScalarFilter;
        if (isPhysicFieldVariableScalar(sceneView()->sceneViewSettings().contourPhysicFieldVariable))
            viewScalarFilter = Util::scene()->problemInfo()->hermes()->viewScalarFilter(sceneView()->sceneViewSettings().contourPhysicFieldVariable,
                                                                                                          PhysicFieldVariableComp_Scalar);
        else
            viewScalarFilter = Util::scene()->problemInfo()->hermes()->viewScalarFilter(sceneView()->sceneViewSettings().contourPhysicFieldVariable,
                                                                                                          PhysicFieldVariableComp_Magnitude);

        setSlnContourView(viewScalarFilter);
        emit processedRangeContour();
    }
}

void SceneSolution::processRangeScalar()
{
    logMessage("SceneSolution::processRangeScalar()");

    if (isSolved())
    {
        ViewScalarFilter *viewScalarFilter = Util::scene()->problemInfo()->hermes()->viewScalarFilter(sceneView()->sceneViewSettings().scalarPhysicFieldVariable,
                                                                                                      sceneView()->sceneViewSettings().scalarPhysicFieldVariableComp);
        setSlnScalarView(viewScalarFilter);
        emit processedRangeScalar();
    }
}

void SceneSolution::processRangeVector()
{
    logMessage("SceneSolution::processRangeVector()");

    if (isSolved())
    {
        ViewScalarFilter *viewVectorXFilter = Util::scene()->problemInfo()->hermes()->viewScalarFilter(sceneView()->sceneViewSettings().vectorPhysicFieldVariable,
                                                                                                       PhysicFieldVariableComp_X);

        ViewScalarFilter *viewVectorYFilter = Util::scene()->problemInfo()->hermes()->viewScalarFilter(sceneView()->sceneViewSettings().vectorPhysicFieldVariable,
                                                                                                       PhysicFieldVariableComp_Y);

        setSlnVectorView(viewVectorXFilter, viewVectorYFilter);
        emit processedRangeVector();
    }
}
