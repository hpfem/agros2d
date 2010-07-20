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

#include "progressdialog.h"
#include "scene.h"
#include "sceneview.h"

SolutionArray::SolutionArray()
{
    sln = NULL;
    order = NULL;

    time = 0.0;
    adaptiveSteps = 0;
    adaptiveError = 100.0;
}

SolutionArray::~SolutionArray()
{
    if (sln) { delete sln; sln = NULL; }
    if (order) { delete order; order = NULL; }
}

void SolutionArray::load(QDomElement *element)
{
    QString fileNameSolution = tempProblemFileName() + ".sln";
    QString fileNameOrder = tempProblemFileName() + ".ord";

    // write content (saved solution)
    QByteArray contentSolution;
    contentSolution.append(element->elementsByTagName("sln").at(0).toElement().childNodes().at(0).nodeValue());
    writeStringContentByteArray(fileNameSolution, QByteArray::fromBase64(contentSolution));

    // write content (saved order)
    QByteArray contentOrder;
    contentOrder.append(element->elementsByTagName("order").at(0).toElement().childNodes().at(0).nodeValue());
    writeStringContentByteArray(fileNameOrder, QByteArray::fromBase64(contentOrder));

    order = new Orderizer();
    order->load_data(fileNameOrder.toStdString().c_str());
    sln = new Solution();
    sln->load(fileNameSolution.toStdString().c_str());
    adaptiveError = element->attribute("adaptiveerror").toDouble();
    adaptiveSteps = element->attribute("adaptivesteps").toInt();
    time = element->attribute("time").toDouble();

    // delete solution
    QFile::remove(fileNameSolution);
    QFile::remove(fileNameOrder);
}

void SolutionArray::save(QDomDocument *doc, QDomElement *element)
{
    // solution
    QString fileNameSolution = tempProblemFileName() + ".sln";
    sln->save(fileNameSolution.toStdString().c_str(), false);
    QDomText textSolution = doc->createTextNode(readFileContentByteArray(fileNameSolution).toBase64());

    // order
    QString fileNameOrder = tempProblemFileName() + ".ord";
    order->save_data(fileNameOrder.toStdString().c_str());
    QDomNode textOrder = doc->createTextNode(readFileContentByteArray(fileNameOrder).toBase64());

    QDomNode eleSolution = doc->createElement("sln");
    QDomNode eleOrder = doc->createElement("order");

    eleSolution.appendChild(textSolution);
    eleOrder.appendChild(textOrder);

    element->setAttribute("adaptiveerror", adaptiveError);
    element->setAttribute("adaptivesteps", adaptiveSteps);
    element->setAttribute("time", time);
    element->appendChild(eleSolution);
    element->appendChild(eleOrder);

    // delete
    QFile::remove(fileNameSolution);
    QFile::remove(fileNameOrder);
}

// *********************************************************************************************

ProgressItem::ProgressItem()
{
    m_name = "";
    m_steps = 0;
    m_isError = false;
    m_isCanceled = false;

    connect(this, SIGNAL(message(QString, bool, int)), this, SLOT(showMessage(QString, bool, int)));
}

void ProgressItem::showMessage(const QString &msg, bool isError, int position)
{
    m_isError = isError;
}

// *********************************************************************************************

ProgressItemMesh::ProgressItemMesh() : ProgressItem()
{
    m_name = tr("Mesh");
    m_steps = 4;
}

bool ProgressItemMesh::run()
{
    QFile::remove(tempProblemFileName() + ".mesh");

    // create triangle files
    if (writeToTriangle())
    {
        emit message(tr("Poly file was created"), false, 1);

        // exec triangle
        QProcess processTriangle;
        processTriangle.setStandardOutputFile(tempProblemFileName() + ".triangle.out");
        processTriangle.setStandardErrorFile(tempProblemFileName() + ".triangle.err");
        connect(&processTriangle, SIGNAL(finished(int)), this, SLOT(meshTriangleCreated(int)));

        QString triangleBinary = "triangle";
        if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "triangle.exe"))
            triangleBinary = "\"" + QApplication::applicationDirPath() + QDir::separator() + "triangle.exe\"";
        if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "triangle"))
            triangleBinary = QApplication::applicationDirPath() + QDir::separator() + "triangle";

        processTriangle.start(QString(Util::config()->commandTriangle).
                              arg(triangleBinary).
                              arg(tempProblemFileName()));

        if (!processTriangle.waitForStarted())
        {
            emit message(tr("Could not start Triangle"), true, 0);
            processTriangle.kill();

            return !m_isError;
        }

        // copy triangle files
        if ((!Util::config()->deleteTriangleMeshFiles) && (!Util::scene()->problemInfo()->fileName.isEmpty()))
        {
            QFileInfo fileInfoOrig(Util::scene()->problemInfo()->fileName);

            QFile::copy(tempProblemFileName() + ".poly", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".poly");
            QFile::copy(tempProblemFileName() + ".node", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".node");
            QFile::copy(tempProblemFileName() + ".edge", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".edge");
            QFile::copy(tempProblemFileName() + ".ele", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".ele");
        }

        while (!processTriangle.waitForFinished()) {}
    }

    return !m_isError;
}

void ProgressItemMesh::meshTriangleCreated(int exitCode)
{
    if (exitCode == 0)
    {
        emit message(tr("Mesh files was created"), false, 2);

        // convert triangle mesh to hermes mesh
        if (triangleToHermes2D())
        {
            emit message(tr("Mesh was converted to Hermes2D mesh file"), false, 3);

            // copy triangle files
            if ((!Util::config()->deleteHermes2DMeshFile) && (!Util::scene()->problemInfo()->fileName.isEmpty()))
            {
                QFileInfo fileInfoOrig(Util::scene()->problemInfo()->fileName);

                QFile::copy(tempProblemFileName() + ".mesh", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".mesh");
            }

            //  remove triangle temp files
            QFile::remove(tempProblemFileName() + ".poly");
            QFile::remove(tempProblemFileName() + ".node");
            QFile::remove(tempProblemFileName() + ".edge");
            QFile::remove(tempProblemFileName() + ".ele");
            QFile::remove(tempProblemFileName() + ".triangle.out");
            QFile::remove(tempProblemFileName() + ".triangle.err");
            emit message(tr("Mesh files was deleted"), false, 4);

            // load mesh
            Mesh *mesh = readMeshFromFile(tempProblemFileName() + ".mesh");

            // check that all boundary edges have a marker assigned
            for (int i = 0; i < mesh->get_max_node_id(); i++)
            {
                if (Node *node = mesh->get_node(i))
                {
                    if (node->used)
                    {
                        if (node->ref < 2 && node->marker == 0)
                        {
                            emit message(tr("Boundary edge does not have a boundary marker"), true, 0);

                            delete mesh;
                            return;
                        }
                    }
                }
            }

            Util::scene()->sceneSolution()->setMeshInitial(mesh);
        }
        else
        {
            QFile::remove(Util::scene()->problemInfo()->fileName + ".mesh");
        }
    }
    else
    {
        QString errorMessage = readFileContent(Util::scene()->problemInfo()->fileName + ".triangle.out");
        emit message(errorMessage, true, 0);
    }
}

bool ProgressItemMesh::writeToTriangle()
{
    // basic check
    if (Util::scene()->nodes.count() < 3)
    {
        emit message(tr("Invalid number of nodes (%1 < 3)").arg(Util::scene()->nodes.count()), true, 0);
        return false;
    }
    if (Util::scene()->edges.count() < 3)
    {
        emit message(tr("Invalid number of edges (%1 < 3)").arg(Util::scene()->edges.count()), true, 0);
        return false;
    }
    else
    {
        // at least one boundary condition has to be assigned
        int count = 0;
        for (int i = 0; i<Util::scene()->edges.count(); i++)
            if (Util::scene()->edgeMarkers.indexOf(Util::scene()->edges[i]->marker) > 0)
                count++;

        if (count == 0)
        {
            emit message(tr("At least one boundary condition has to be assigned"), true, 0);
            return false;
        }
    }
    if (Util::scene()->labels.count() < 1)
    {
        emit message(tr("Invalid number of labels (%1 < 1)").arg(Util::scene()->labels.count()), true, 0);
        return false;
    }
    else
    {
        // at least one material has to be assigned
        int count = 0;
        for (int i = 0; i<Util::scene()->labels.count(); i++)
            if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) > 0)
                count++;

        if (count == 0)
        {
            emit message(tr("At least one material has to be assigned"), true, 0);
            return false;
        }
    }
    if (Util::scene()->edgeMarkers.count() < 2) // + none marker
    {
        emit message(tr("Invalid number of boundary conditions (%1 < 1)").arg(Util::scene()->edgeMarkers.count()), true, 0);
        return false;
    }
    if (Util::scene()->labelMarkers.count() < 2) // + none marker
    {
        emit message(tr("Invalid number of materials (%1 < 1)").arg(Util::scene()->labelMarkers.count()), true, 0);
        return false;
    }

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    QDir dir;
    dir.mkdir(QDir::temp().absolutePath() + "/agros2d");
    QFile file(tempProblemFileName() + ".poly");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        emit message(tr("Could not create Triangle poly mesh file (%1)").arg(file.errorString()), true, 0);
        return false;
    }
    QTextStream out(&file);


    // nodes
    QString outNodes;
    int nodesCount = 0;
    for (int i = 0; i<Util::scene()->nodes.count(); i++)
    {
        outNodes += QString("%1  %2  %3  %4\n").
                    arg(i).
                    arg(Util::scene()->nodes[i]->point.x, 0, 'f', 10).
                    arg(Util::scene()->nodes[i]->point.y, 0, 'f', 10).
                    arg(0);
        nodesCount++;
    }

    // edges
    QString outEdges;
    int edgesCount = 0;
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        if (Util::scene()->edges[i]->angle == 0)
        {
            // line
            outEdges += QString("%1  %2  %3  %4\n").
                        arg(edgesCount).
                        arg(Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeStart)).
                        arg(Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeEnd)).
                        arg(i+1);
            edgesCount++;
        }
        else
        {
            // arc
            // add pseudo nodes
            Point center = Util::scene()->edges[i]->center();
            double radius = Util::scene()->edges[i]->radius();
            double startAngle = atan2(center.y - Util::scene()->edges[i]->nodeStart->point.y,
                                      center.x - Util::scene()->edges[i]->nodeStart->point.x) - M_PI;
            int segments = Util::scene()->edges[i]->angle/5.0 + 1;
            if (segments < Util::config()->angleSegmentsCount) segments = Util::config()->angleSegmentsCount; // minimum segments

            double theta = deg2rad(Util::scene()->edges[i]->angle) / double(segments);

            int nodeStartIndex = 0;
            int nodeEndIndex = 0;
            for (int j = 0; j < segments; j++)
            {
                double arc = startAngle + j*theta;

                double x = radius * cos(arc);
                double y = radius * sin(arc);

                nodeEndIndex = nodesCount+1;
                if (j == 0)
                {
                    nodeStartIndex = Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeStart);
                    nodeEndIndex = nodesCount;
                }
                if (j == segments - 1)
                {
                    nodeEndIndex = Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeEnd);
                }
                if ((j > 0) && (j < segments))
                {
                    outNodes += QString("%1  %2  %3  %4\n").
                                arg(nodesCount).
                                arg(center.x + x, 0, 'f', 10).
                                arg(center.y + y, 0, 'f', 10).
                                arg(0);
                    nodesCount++;
                }
                outEdges += QString("%1  %2  %3  %4\n").
                            arg(edgesCount).
                            arg(nodeStartIndex).
                            arg(nodeEndIndex).
                            arg(i+1);
                edgesCount++;
                nodeStartIndex = nodeEndIndex;
            }
        }
    }

    // holes
    int holesCount = 0;
    for (int i = 0; i<Util::scene()->labels.count(); i++) if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0) holesCount++;
    QString outHoles = QString("%1\n").arg(holesCount);
    for (int i = 0; i<Util::scene()->labels.count(); i++)
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) == 0)
            outHoles += QString("%1  %2  %3\n").
                        arg(i).
                        arg(Util::scene()->labels[i]->point.x, 0, 'f', 10).
                        arg(Util::scene()->labels[i]->point.y, 0, 'f', 10);

    // labels
    QString outLabels;
    int labelsCount = 0;
    for(int i = 0; i<Util::scene()->labels.count(); i++)
    {
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) > 0)
        {
           outLabels += QString("%1  %2  %3  %4  %5\n").
                        arg(labelsCount).
                        arg(Util::scene()->labels[i]->point.x, 0, 'f', 10).
                        arg(Util::scene()->labels[i]->point.y, 0, 'f', 10).
                        arg(i + 1). // triangle returns zero region number for areas without marker, markers must start from 1
                        arg(Util::scene()->labels[i]->area);
           labelsCount++;
        }
    }

    outNodes.insert(0, QString("%1 2 0 1\n").
                    arg(nodesCount)); // + additional Util::scene()->nodes
    out << outNodes;
    outEdges.insert(0, QString("%1 1\n").
                    arg(edgesCount)); // + additional edges
    out << outEdges;
    out << outHoles;
    outLabels.insert(0, QString("%1 1\n").
                     arg(labelsCount)); // - holes
    out << outLabels;

    file.waitForBytesWritten(0);
    file.close();

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return true;
}

bool ProgressItemMesh::triangleToHermes2D()
{
    int i, n, k, count, marker, node_1, node_2, node_3;
    double x, y;

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    QFile fileMesh(tempProblemFileName() + ".mesh");
    if (!fileMesh.open(QIODevice::WriteOnly))
    {
        emit message(tr("Could not create Hermes2D mesh file"), true, 0);
        return false;
    }
    QTextStream outMesh(&fileMesh);

    QFile fileNode(tempProblemFileName() + ".node");
    if (!fileNode.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit message(tr("Could not read Triangle node file"), true, 0);
        return false;
    }
    QTextStream inNode(&fileNode);

    QFile fileEdge(tempProblemFileName() + ".edge");
    if (!fileEdge.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit message(tr("Could not read Triangle edge file"), true, 0);
        return false;
    }
    QTextStream inEdge(&fileEdge);

    QFile fileEle(tempProblemFileName() + ".ele");
    if (!fileEle.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit message(tr("Could not read Triangle ele file"), true, 0);
        return false;
    }
    QTextStream inEle(&fileEle);

    // nodes
    QString outNodes;
    outNodes += "vertices = \n";
    outNodes += "{ \n";
    sscanf(inNode.readLine().toStdString().c_str(), "%i", &k);
    for (int i = 0; i<k; i++)
    {
        sscanf(inNode.readLine().toStdString().c_str(), "%i   %lf %lf %i", &n, &x, &y, &marker);
        outNodes += QString("\t{ %1,  %2 }, \n").arg(x, 0, 'f', 10).arg(y, 0, 'f', 10);
    }
    outNodes.truncate(outNodes.length()-3);
    outNodes += "\n} \n\n";

    // edges and curves
    QString outEdges;
    outEdges += "boundaries = \n";
    outEdges += "{ \n";
    sscanf(inEdge.readLine().toStdString().c_str(), "%i", &k);
    count = 0;
    for (int i = 0; i<k; i++)
    {
        sscanf(inEdge.readLine().toStdString().c_str(), "%i	%i	%i	%i", &n, &node_1, &node_2, &marker);
        if (marker != 0)
        {
            if (Util::scene()->edges[marker-1]->marker->type != PhysicFieldBC_None)
            {
                count++;
                outEdges += QString("\t{ %1, %2, %3 }, \n").arg(node_1).arg(node_2).arg(abs(marker));
            }
        }
    }
    outEdges.truncate(outEdges.length()-3);
    outEdges += "\n} \n\n";
    if (count < 1)
    {
        emit message(tr("Invalid number of edge markers"), true, 0);
        return false;
    }

    // elements
    QString outElements;
    outElements += "elements = \n";
    outElements += "{ \n";
    sscanf(inEle.readLine().toStdString().c_str(), "%i", &k);
    count = 0;
    for (int i = 0; i<k; i++)
    {
        count++;
        sscanf(inEle.readLine().toStdString().c_str(), "%i	%i	%i	%i	%i", &n, &node_1, &node_2, &node_3, &marker);
        if (marker == 0)
        {
            emit message(tr("Some areas have no label marker"), true, 0);
            return false;
        }
        // triangle returns zero region number for areas without marker, markers must start from 1
        marker--;
        outElements += QString("\t{ %1, %2, %3, %4  }, \n").arg(node_1).arg(node_2).arg(node_3).arg(abs(marker));
    }
    outElements.truncate(outElements.length()-3);
    outElements += "\n} \n\n";
    if (count < 1)
    {
        emit message(tr("Invalid number of label markers"), true, 0);
        return false;
    }

    outMesh << outNodes;
    outMesh << outElements;
    outMesh << outEdges;

    fileNode.close();
    fileEdge.close();
    fileEle.close();

    fileMesh.waitForBytesWritten(0);
    fileMesh.close();

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return true;
}

// *********************************************************************************************

ProgressItemSolve::ProgressItemSolve() : ProgressItem()
{
    m_name = tr("Solver");
    m_steps = 1;
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        m_steps += floor(Util::scene()->problemInfo()->timeTotal.number / Util::scene()->problemInfo()->timeStep.number);
}

bool ProgressItemSolve::run()
{
    solve();

    return !m_isError;
}

void ProgressItemSolve::solve()
{
    qDebug() << "ProgressItemSolve::solve()";

    if (!QFile::exists(tempProblemFileName() + ".mesh"))
      return;

    // benchmark
    QTime time;
    time.start();

    emit message(tr("Solver was started: %1 (%2, %3)").
                 arg(physicFieldString(Util::scene()->problemInfo()->physicField())).
                 arg(problemTypeString(Util::scene()->problemInfo()->problemType)).
                 arg(analysisTypeString(Util::scene()->problemInfo()->analysisType)), false, 1);

    QList<SolutionArray *> *solutionArrayList = Util::scene()->problemInfo()->hermes()->solve(this);

    if (!solutionArrayList->isEmpty())
    {
        emit message(tr("Problem was solved"), false, 2);
        Util::scene()->sceneSolution()->setTimeElapsed(time.elapsed());
    }
    else
    {
        emit message(tr("Problem was not solved"), true, 0);
        Util::scene()->sceneSolution()->setTimeElapsed(0);
    }

    Util::scene()->sceneSolution()->setSolutionArrayList(solutionArrayList);
}

// *********************************************************************************************

ProgressItemProcessView::ProgressItemProcessView() : ProgressItem()
{
    m_name = tr("View");

    m_steps = 0;
    if (sceneView()->sceneViewSettings().showContours == 1)
        m_steps += 1;
    if (sceneView()->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView ||
        sceneView()->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView3D ||
        sceneView()->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid)
        m_steps += 1;
    if (sceneView()->sceneViewSettings().showVectors == 1)
        m_steps += 1;
}

bool ProgressItemProcessView::run()
{
    process();

    return !m_isError;
}

void ProgressItemProcessView::process()
{
    int step = 0;

    if (sceneView()->sceneViewSettings().showContours == 1)
    {
        step++;
        emit message(tr("Processing countour view cache"), false, step);
        Util::scene()->sceneSolution()->processRangeContour();
    }
    if (sceneView()->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView ||
        sceneView()->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView3D ||
        sceneView()->sceneViewSettings().postprocessorShow == SceneViewPostprocessorShow_ScalarView3DSolid)
    {
        step++;
        emit message(tr("Processing scalar view cache"), false, step);
        Util::scene()->sceneSolution()->processRangeScalar();
    }
    if (sceneView()->sceneViewSettings().showVectors == 1)
    {
        step++;
        emit message(tr("Processing vector view cache"), false, step);
        Util::scene()->sceneSolution()->processRangeVector();
    }
}

// ***********************************************************************************************

ProgressDialog::ProgressDialog(QWidget *parent) : QDialog(parent)
{
    setWindowModality(Qt::ApplicationModal);
    setModal(true);

    setWindowIcon(icon("run"));
    setWindowTitle(tr("Progress..."));

    createControls();
    clear();

    setMinimumSize(520, 360);
    setMaximumSize(minimumSize());
}

ProgressDialog::~ProgressDialog()
{
    saveProgressLog();
    clear();
}

void ProgressDialog::clear()
{
    // delete progress items
    for (int i = 0; i < m_progressItem.count(); i++)
        delete m_progressItem.at(i);
    m_progressItem.clear();

    m_currentProgressItem = NULL;
    m_showViewProgress = true;
}

void ProgressDialog::createControls()
{
    QTabWidget *tabType = new QTabWidget();
    tabType->addTab(createControlsProgress(), icon(""), tr("Progress"));
    tabType->addTab(createControlsConvergenceChart(), icon(""), tr("Convergence chart"));

    if (Util::scene()->problemInfo()->adaptivityType > 2)
        tabType->widget(1)->setDisabled(true);

    btnCancel = new QPushButton(tr("Cance&l"));
    btnCancel->setDefault(true);
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(this, SIGNAL(finished(int)), this, SLOT(cancel()));

    btnClose = new QPushButton(tr("&Close"));
    btnClose->setDefault(true);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(btnClose, QDialogButtonBox::RejectRole);
    buttonBox->addButton(btnCancel, QDialogButtonBox::RejectRole);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabType);
    layout->addWidget(buttonBox);

    setLayout(layout);
}

QWidget *ProgressDialog::createControlsProgress()
{
    lblMessage = new QLabel("", this);
    progressBar = new QProgressBar(this);

    lstMessage = new QTextEdit(this);
    lstMessage->setReadOnly(true);

    QVBoxLayout *layoutProgress = new QVBoxLayout();
    layoutProgress->addWidget(lblMessage);
    layoutProgress->addWidget(progressBar);
    layoutProgress->addWidget(lstMessage);

    QWidget *widProgress = new QWidget();
    widProgress->setLayout(layoutProgress);

    return widProgress;
}

QWidget *ProgressDialog::createControlsConvergenceChart()
{
    chart = new Chart(this);
    //chart->setMinimumSize(220, 160);

    QVBoxLayout *layoutConvergenceChart = new QVBoxLayout();
    layoutConvergenceChart->addWidget(chart);

    QWidget *widConvergenceChart = new QWidget();
    widConvergenceChart->setLayout(layoutConvergenceChart);

    return widConvergenceChart;

}

int ProgressDialog::progressSteps()
{
    int steps = 0;
    for (int i = 0; i < m_progressItem.count(); i++)
        steps += m_progressItem.at(i)->steps();

    return steps;
}

int ProgressDialog::currentProgressStep()
{
    int steps = 0;
    for (int i = 0; i < m_progressItem.count(); i++)
    {
        if (m_progressItem.at(i) == m_currentProgressItem)
            return steps;

        steps += m_progressItem.at(i)->steps();
    }

    return -1;
}

void ProgressDialog::appendProgressItem(ProgressItem *progressItem)
{
    m_progressItem.append(progressItem);

    connect(progressItem, SIGNAL(message(QString, bool, int)), this, SLOT(showMessage(QString, bool, int)));
    connect(this, SIGNAL(cancelProgressItem()), progressItem, SLOT(cancelProgressItem()));
}

bool ProgressDialog::run(bool showViewProgress)
{
    m_showViewProgress = showViewProgress;
    QTimer::singleShot(0, this, SLOT(start()));

    return exec();
}

void ProgressDialog::start()
{
    lstMessage->clear();

    progressBar->setRange(0, progressSteps());
    progressBar->setValue(0);
    QApplication::processEvents();

    for (int i = 0; i < m_progressItem.count(); i++)
    {
        m_currentProgressItem = m_progressItem.at(i);

        if (!m_currentProgressItem->run())
        {
            // error
            finished();

            clear();
            return;
        }
    }

    // successfull run
    if (!Util::config()->showConvergenceChart || Util::scene()->problemInfo()->adaptivityType > 2)
    {
        clear();
        close();
    }
    else
    {
        // swith to convergence chart tab
    }
}

void ProgressDialog::showMessage(const QString &msg, bool isError, int position)
{
    btnCancel->setEnabled(!isError);

    if (isError)
    {
        lstMessage->setTextColor(QColor(Qt::red));
        finished();
    }
    else
    {
        lstMessage->setTextColor(QColor(Qt::black));
    }

    QString message = QString("%1: %2\n").
                      arg(m_currentProgressItem->name()).
                      arg(msg);

    lstMessage->insertPlainText(message);

    lstMessage->ensureCursorVisible();
    lblMessage->setText(message);

    if (position > 0)
        progressBar->setValue(currentProgressStep() + position);

    // update
    QApplication::processEvents();
    lstMessage->update();
}

void ProgressDialog::finished()
{
    btnClose->setEnabled(true);
    btnCancel->setEnabled(false);
}

void ProgressDialog::cancel()
{
    emit cancelProgressItem();
    finished();
    QApplication::processEvents();
}

void ProgressDialog::close()
{
    cancel();
    accept();
}

void ProgressDialog::saveProgressLog()
{
    QFile file(tempProblemDir() + "/messages.log");
    if (file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream messages(&file);
        messages << QDateTime(QDateTime::currentDateTime()).toString("dd.MM.yyyy hh:mm") + ",\x20" << Util::scene()->problemInfo()->name + "\n" << lstMessage->toPlainText() + "\n";
    }
}
