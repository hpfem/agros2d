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

#include "solverdialog.h"
#include "scene.h"

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
    if (sln) delete sln;
    if (order) delete order;
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

    element->setAttribute("time", time);
    element->setAttribute("adaptivesteps", adaptiveSteps);
    element->setAttribute("adaptiveerror", adaptiveError);
    element->appendChild(eleSolution);
    element->appendChild(eleOrder);

    // delete
    QFile::remove(fileNameSolution);
    QFile::remove(fileNameOrder);
}

// *********************************************************************************************

SolverDialog::SolverDialog(QWidget *parent) : QDialog(parent)
{
    setWindowModality(Qt::ApplicationModal);
    setMinimumSize(420, 260);
    setMaximumSize(minimumSize());
    setModal(true);

    setWindowIcon(icon("run"));
    setWindowTitle(tr("Solve problem..."));

    createControls();

    refreshTimer = new QTimer(this);
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(doRefreshTimerUpdate()));

    connect(this, SIGNAL(solved()), this, SLOT(doFinished()));

    resize(minimumSize());
}

SolverDialog::~SolverDialog()
{
    QFile::remove(tempProblemFileName() + ".mesh");

    delete btnCancel;
    delete btnClose;
    delete progressBar;
    delete lblMessage;
    delete lstMessage;       
}

void SolverDialog::createControls()
{
    lblMessage = new QLabel(this);
    progressBar = new QProgressBar(this);

    lstMessage = new QTextEdit(this);
    lstMessage->setReadOnly(true);

    // cancel button
    btnCancel = new QPushButton(tr("&Cancel"));
    btnCancel->setDefault(true);
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
    connect(this, SIGNAL(finished(int)), this, SLOT(doCancel()));

    btnClose = new QPushButton(tr("&Close"));
    btnClose->setDefault(true);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(btnClose, QDialogButtonBox::RejectRole);
    buttonBox->addButton(btnCancel, QDialogButtonBox::RejectRole);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(lblMessage);
    layout->addWidget(progressBar);
    layout->addWidget(lstMessage);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

int SolverDialog::solve()
{
    lblMessage->setText(tr("Solver: solve problem..."));
    lstMessage->clear();
    progressBar->setValue(0);

    m_isCanceled = false;
    btnCancel->setEnabled(true);

    QApplication::processEvents();
    refreshTimer->start(1000);
    QTimer::singleShot(0, this, SLOT(doStart()));

    return exec();
}

void SolverDialog::cancel()
{
    QApplication::processEvents();
    m_isCanceled = true;
}

void SolverDialog::showMessage(const QString &message, bool isError)
{
    btnCancel->setEnabled(!isError);

    if (isError)
    {
        lstMessage->setTextColor(QColor(Qt::red));
        doFinished();
    }
    else
    {
        lstMessage->setTextColor(QColor(Qt::black));
    }

    lstMessage->insertPlainText(message + "\n");
    lstMessage->ensureCursorVisible();
    lblMessage->setText(message);

    // update
    QApplication::processEvents();
    lstMessage->update();
}

void SolverDialog::showProgress(int index)
{
    progressBar->setValue(index);
}

void SolverDialog::doStart()
{
    btnClose->setEnabled(false);
    runMesh();
}

void SolverDialog::doFinished()
{
    setFileNameOrig("");
    btnClose->setEnabled(true);
    btnCancel->setEnabled(false);

    refreshTimer->stop();
}

void SolverDialog::doCancel()
{
    m_isCanceled = true;
    doFinished();
    QApplication::processEvents();
}

void SolverDialog::doClose()
{
    doCancel();
    hide();
}

void SolverDialog::doRefreshTimerUpdate()
{
    foreach (QWidget *widget, QApplication::allWidgets())
        widget->update();
    QApplication::processEvents();
}

void SolverDialog::runMesh()
{
    if ((m_mode == SolverMode_MeshAndSolve) && (Util::scene()->sceneSolution()->isMeshed()))
        doMeshTriangleCreated(0);

    QFile::remove(tempProblemFileName() + ".mesh");

    // create triangle files
    if (writeToTriangle())
    {
        showMessage(tr("Triangle: poly file was created."), false);
        progressBar->setValue(20);

        // exec triangle
        QProcess *processTriangle = new QProcess();
        processTriangle->setStandardOutputFile(tempProblemFileName() + ".triangle.out");
        processTriangle->setStandardErrorFile(tempProblemFileName() + ".triangle.err");
        connect(processTriangle, SIGNAL(finished(int)), this, SLOT(doMeshTriangleCreated(int)));

        QString triangleBinary = "triangle";
        if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "triangle.exe"))
            triangleBinary = "\"" + QApplication::applicationDirPath() + QDir::separator() + "triangle.exe\"";
        if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "triangle"))
            triangleBinary = QApplication::applicationDirPath() + QDir::separator() + "triangle";

        processTriangle->start(QString("%1 -p -P -q30.0 -e -A -a -z -Q -I -p \"%2\"").
                               arg(triangleBinary).
                               arg(tempProblemFileName()));
        progressBar->setValue(30);

        if (!processTriangle->waitForStarted())
        {
            showMessage(tr("Triangle: could not start Triangle."), true);
            processTriangle->kill();

            progressBar->setValue(100);
            return;
        }

        // copy triangle files
        QSettings settings;
        bool deleteTriangleFiles = settings.value("Solver/DeleteTriangleMeshFiles", true).value<bool>();

        if ((!deleteTriangleFiles) && (!m_fileNameOrig.isEmpty()))
        {
            QFileInfo fileInfoOrig(m_fileNameOrig);

            QFile::copy(tempProblemFileName() + ".poly", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".poly");
            QFile::copy(tempProblemFileName() + ".node", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".node");
            QFile::copy(tempProblemFileName() + ".edge", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".edge");
            QFile::copy(tempProblemFileName() + ".ele", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".ele");
        }

        while (!processTriangle->waitForFinished()) {}
    }
}

void SolverDialog::doMeshTriangleCreated(int exitCode)
{
    if (exitCode == 0)
    {
        showMessage(tr("Triangle: mesh files was created."), false);
        progressBar->setValue(40);

        // convert triangle mesh to hermes mesh
        if (triangleToHermes2D())
        {
            showMessage(tr("Triangle: mesh was converted to Hermes2D mesh file."), false);

            // copy triangle files
            QSettings settings;
            bool deleteHermes2D = settings.value("Solver/DeleteHermes2DMeshFile", true).value<bool>();

            if ((!deleteHermes2D) && (!m_fileNameOrig.isEmpty()))
            {
                QFileInfo fileInfoOrig(m_fileNameOrig);

                QFile::copy(tempProblemFileName() + ".mesh", fileInfoOrig.absolutePath() + "/" + fileInfoOrig.baseName() + ".mesh");
            }

            //  remove triangle temp files
            QFile::remove(tempProblemFileName() + ".poly");
            QFile::remove(tempProblemFileName() + ".node");
            QFile::remove(tempProblemFileName() + ".edge");
            QFile::remove(tempProblemFileName() + ".ele");
            QFile::remove(tempProblemFileName() + ".triangle.out");
            QFile::remove(tempProblemFileName() + ".triangle.err");
            showMessage(tr("Triangle: mesh files was deleted."), false);

            if (m_mode == SolverMode_Mesh)
                progressBar->setValue(100);
            else
                progressBar->setValue(50);

            // load mesh
            // save locale
            char *plocale = setlocale (LC_NUMERIC, "");
            setlocale (LC_NUMERIC, "C");

            Mesh *mesh = new Mesh();
            H2DReader meshloader;
            meshloader.load((tempProblemFileName() + ".mesh").toStdString().c_str(), mesh);

            // set system locale
            setlocale(LC_NUMERIC, plocale);

            // check that all boundary edges have a marker assigned
            for (int i = 0; i < mesh->get_max_node_id(); i++)
            {
                if (Node *node = mesh->get_node(i))
                {
                    if (node->used)
                    {
                        if (node->ref < 2 && node->marker == 0)
                        {
                            showMessage(tr("Hermes2D: boundary edge does not have a boundary marker."), true);
                            delete mesh;
                            return;
                        }
                    }
                }
            }
            Util::scene()->sceneSolution()->setMesh(mesh);
        }
        else
        {
            // error
            progressBar->setValue(100);

            QFile::remove(Util::scene()->problemInfo()->fileName + ".mesh");
            return;
        }

        if (m_mode == SolverMode_MeshAndSolve)
        {
            runSolver();
            if (Util::scene()->sceneSolution()->isSolved())
                emit solved();
        }
        else
        {
            emit solved();
        }
    }
    else
    {
        // error
        progressBar->setValue(100);

        QString errorMessage = readFileContent(Util::scene()->problemInfo()->fileName + ".triangle.out");
        showMessage(tr("Triangle: ") + errorMessage, true);
    }
}

void SolverDialog::runSolver()
{
    if (!QFile::exists(tempProblemFileName() + ".mesh"))
        return;

    // benchmark
    QTime time;
    time.start();

    showMessage(tr("Solver: solver was started: %1 (%2, %3) ").
                 arg(physicFieldString(Util::scene()->problemInfo()->physicField())).
                 arg(problemTypeString(Util::scene()->problemInfo()->problemType)).
                 arg(analysisTypeString(Util::scene()->problemInfo()->analysisType)), false);
    progressBar->setValue(60);

    QList<SolutionArray *> *solutionArrayList = Util::scene()->problemInfo()->hermes()->solve(this);

    if (!solutionArrayList->isEmpty())
    {
        Util::scene()->sceneSolution()->setSolutionArrayList(solutionArrayList);
        showMessage(tr("Solver: problem was solved."), false);
        Util::scene()->sceneSolution()->setTimeElapsed(time.elapsed());

        progressBar->setValue(100);
    }
    else
    {
        Util::scene()->sceneSolution()->clear();
        showMessage(tr("Solver: problem was not solved."), true);
        Util::scene()->sceneSolution()->setTimeElapsed(0);
    }
}

bool SolverDialog::writeToTriangle()
{
    // basic check
    if (Util::scene()->nodes.count() < 3)
    {
        showMessage(tr("Triangle: invalid number of nodes (%1 < 3).").arg(Util::scene()->nodes.count()), true);
        return false;
    }
    if (Util::scene()->edges.count() < 3)
    {
        showMessage(tr("Triangle: invalid number of edges (%1 < 3).").arg(Util::scene()->edges.count()), true);
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
            showMessage(tr("Triangle: at least one boundary condition has to be assigned."), true);
            return false;
        }
    }
    if (Util::scene()->labels.count() < 1)
    {
        showMessage(tr("Triangle: invalid number of labels (%1 < 1).").arg(Util::scene()->labels.count()), true);
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
            showMessage(tr("Triangle: at least one material has to be assigned."), true);
            return false;
        }
    }
    if (Util::scene()->edgeMarkers.count() < 2) // + none marker
    {
        showMessage(tr("Triangle: invalid number of boundary conditions (%1 < 1).").arg(Util::scene()->edgeMarkers.count()), true);
        return false;
    }
    if (Util::scene()->labelMarkers.count() < 2) // + none marker
    {
        showMessage(tr("Triangle: invalid number of materials (%1 < 1).").arg(Util::scene()->labelMarkers.count()), true);
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
        showMessage(tr("Triangle: could not create triangle poly mesh file (%1).").arg(file.errorString()), true);
        return false;
    }
    QTextStream out(&file);


    // nodes
    QString outNodes;
    int nodesCount = 0;
    for (int i = 0; i<Util::scene()->nodes.count(); i++)
    {
        outNodes += QString("%1  %2  %3  %4\n").arg(i).arg(Util::scene()->nodes[i]->point.x, 0, 'f', 10).arg(Util::scene()->nodes[i]->point.y, 0, 'f', 10).arg(0);
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
            outEdges += QString("%1  %2  %3  %4\n").arg(edgesCount).arg(Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeStart)).arg(Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeEnd)).arg(i+1);
            edgesCount++;
        }
        else
        {
            // arc
            // add pseudo nodes
            Point center = Util::scene()->edges[i]->center();
            double radius = Util::scene()->edges[i]->radius();
            double startAngle = atan2(center.y - Util::scene()->edges[i]->nodeStart->point.y, center.x - Util::scene()->edges[i]->nodeStart->point.x) / M_PI*180 - 180;
            int segments = Util::scene()->edges[i]->angle/5.0 + 1;
            QSettings settings;
            double angleSegmentsCount = settings.value("Geometry/AngleSegmentsCount", 5).value<double>();
            if (segments < angleSegmentsCount) segments = angleSegmentsCount; // minimum segments

            double theta = Util::scene()->edges[i]->angle / float(segments - 1);

            int nodeStartIndex = 0;
            int nodeEndIndex = 0;
            for (int j = 0; j < segments; j++)
            {
                double arc = (startAngle + j*theta)/180.0*M_PI;
                double x = radius * cos(arc);
                double y = radius * sin(arc);

                nodeEndIndex = nodesCount+1;
                if (j == 0)
                {
                    nodeStartIndex = Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeStart);
                    nodeEndIndex = nodesCount;
                }
                if (j == segments-1)
                {
                    nodeEndIndex = Util::scene()->nodes.indexOf(Util::scene()->edges[i]->nodeEnd);
                }
                if ((j > 0) && (j < segments))
                {
                    outNodes += QString("%1  %2  %3  %4\n").arg(nodesCount).arg(center.x + x, 0, 'f', 10).arg(center.y + y, 0, 'f', 10).arg(0);
                    nodesCount++;
                }
                outEdges += QString("%1  %2  %3  %4\n").arg(edgesCount).arg(nodeStartIndex).arg(nodeEndIndex).arg(i+1);
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
            outHoles += QString("%1  %2  %3\n").arg(i).arg(Util::scene()->labels[i]->point.x, 0, 'f', 10).arg(Util::scene()->labels[i]->point.y, 0, 'f', 10);

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

    outNodes.insert(0, QString("%1 2 0 1\n").arg(nodesCount)); // + additional Util::scene()->nodes
    out << outNodes;
    outEdges.insert(0, QString("%1 1\n").arg(edgesCount)); // + additional edges
    out << outEdges;
    out << outHoles;
    outLabels.insert(0, QString("%1 1\n").arg(labelsCount)); // - holes
    out << outLabels;

    file.waitForBytesWritten(0);
    file.close();

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return true;
}

bool SolverDialog::triangleToHermes2D()
{
    int i, n, k, l, count, marker, node_1, node_2, node_3;
    double x, y;

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    QFile fileMesh(tempProblemFileName() + ".mesh");
    if (!fileMesh.open(QIODevice::WriteOnly))
    {
        showMessage(tr("Hermes2D: could not create hermes2d mesh file."), true);
        return false;
    }
    QTextStream outMesh(&fileMesh);

    QFile fileNode(tempProblemFileName() + ".node");
    if (!fileNode.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        showMessage(tr("Hermes2D: could not read triangle node file."), true);
        return false;
    }
    QTextStream inNode(&fileNode);

    QFile fileEdge(tempProblemFileName() + ".edge");
    if (!fileEdge.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        showMessage(tr("Hermes2D: could not read triangle edge file."), true);
        return false;
    }
    QTextStream inEdge(&fileEdge);

    QFile fileEle(tempProblemFileName() + ".ele");
    if (!fileEle.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        showMessage(tr("Hermes2D: could not read triangle ele file."), true);
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
        showMessage(tr("Hermes2D: invalid number of edge markers."), true);
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
            showMessage(tr("Hermes2D: some areas have no label marker."), true);
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
        showMessage(tr("Hermes2D: invalid number of label markers."), true);
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
