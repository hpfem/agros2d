#include "solverdialog.h"
#include "scene.h"

SolverThread::SolverThread(QObject *parent) : QThread(parent)
{
    connect(this, SIGNAL(started()), SLOT(doStarted()));
}

SolverThread::~SolverThread()
{

}

void SolverThread::doStarted()
{
    runMesh();
}

void SolverThread::run()
{
    m_isCanceled = false;
    exec();
}

void SolverThread::cancel()
{
    QApplication::processEvents();
    while (isRunning())
    {
        quit();
        wait(50);
    }
    m_isCanceled = true;
}

void SolverThread::showMessage(const QString &msg, bool isError)
{
    emit message(msg, isError);
    QApplication::processEvents();
}

void SolverThread::runMesh()
{
    Util::scene()->sceneSolution()->mesh().free();
    QFile::remove(tempProblemFileName() + ".mesh");

    // create triangle files
    if (writeToTriangle())
    {
        emit message(tr("Triangle: poly file was created."), false);
        updateProgress(20);

        // exec triangle
        QProcess *processTriangle = new QProcess();
        processTriangle->setStandardOutputFile(tempProblemFileName() + ".triangle.out");
        processTriangle->setStandardErrorFile(tempProblemFileName() + ".triangle.err");
        connect(processTriangle, SIGNAL(finished(int)), this, SLOT(doMeshTriangleCreated(int)));

        processTriangle->start("triangle -p -P -q30.0 -e -A -a -z -Q -I -p \"" + tempProblemFileName() + "\"");
        updateProgress(30);

        if (!processTriangle->waitForStarted())
        {
            emit message(tr("Triangle: could not start Triangle."), false);
            processTriangle->kill();

            updateProgress(100);
            emit solved();
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
    else
    {
        // error
        updateProgress(100);
        return;
    }
}

void SolverThread::doMeshTriangleCreated(int exitCode)
{
    if (exitCode == 0)
    {
        emit message(tr("Triangle: mesh files was created."), false);
        updateProgress(40);

        // convert triangle mesh to hermes mesh
        if (triangleToHermes2D())
        {
            emit message(tr("Triangle: mesh was converted to Hermes2D mesh file."), false);

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
            emit message(tr("Triangle: mesh files was deleted."), false);

            if (m_mode == SOLVER_MESH)
                updateProgress(100);
            else
                updateProgress(50);

            // load mesh
            // save locale
            char *plocale = setlocale (LC_NUMERIC, "");
            setlocale (LC_NUMERIC, "C");

            Util::scene()->sceneSolution()->mesh().load((tempProblemFileName() + ".mesh").toStdString().c_str());

            // set system locale
            setlocale(LC_NUMERIC, plocale);

            // check that all boundary edges have a marker assigned
            for (int i = 0; i < Util::scene()->sceneSolution()->mesh().get_max_node_id(); i++)
            {
                if (Node *node = Util::scene()->sceneSolution()->mesh().get_node(i))
                {
                    if (node->used)
                    {
                        if (node->ref < 2 && node->marker == 0)
                        {
                            emit message(tr("Hermes2D: boundary edge does not have a boundary marker."), true);
                            Util::scene()->sceneSolution()->mesh().free();
                            return;
                        }
                    }
                }
            }
        }
        else
        {
            // error
            updateProgress(100);

            QFile::remove(Util::scene()->problemInfo().fileName + ".mesh");
            return;
        }

        if (m_mode == SOLVER_MESH_AND_SOLVE)
        {
            mutex.lock();
            runSolver();
            mutex.unlock();
        }

        emit solved();
    }
    else
    {
        // error
        updateProgress(100);

        QFile file(Util::scene()->problemInfo().fileName + ".triangle.out");

        QTextStream standardOutput(&file);
        QString errorMessage = standardOutput.readAll();
        emit message(tr("Triangle: ") + errorMessage, true);

        if (!file.open(QIODevice::ReadOnly))
            return;
        file.close();
    }
}

void SolverThread::runSolver()
{   
    QString fileName(tempProblemFileName() + ".mesh");
    if (!QFile::exists(fileName)) return;

    // benchmark
    QTime time;
    time.start();

    emit message(tr("Solver: solver was started: ") + physicFieldString(Util::scene()->problemInfo().physicField()) + " (" + problemTypeString(Util::scene()->problemInfo().problemType) + ")", false);
    updateProgress(60);

    QList<SolutionArray *> *solutionArrayList = Util::scene()->problemInfo().hermes->solve(this);

    if (solutionArrayList)
    {
        Util::scene()->sceneSolution()->setSolutionArrayList(solutionArrayList);

        emit message(tr("Solver: problem was solved."), false);
        Util::scene()->sceneSolution()->setTimeElapsed(time.elapsed());
    }
    else
    {
        Util::scene()->sceneSolution()->clear();
        emit message(tr("Solver: problem was not solved."), true);
        Util::scene()->sceneSolution()->setTimeElapsed(0);
    }

    updateProgress(100);
}

bool SolverThread::writeToTriangle()
{
    // basic check
    if (Util::scene()->nodes.count() < 3)
    {
        emit message(tr("Triangle: invalid number of nodes (%1 < 3).").arg(Util::scene()->nodes.count()), true);
        return false;
    }
    if (Util::scene()->edges.count() < 3)
    {
        emit message(tr("Triangle: invalid number of edges (%1 < 3).").arg(Util::scene()->edges.count()), true);
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
            emit message(tr("Triangle: at least one boundary condition has to be assigned."), true);
            return false;
        }
    }
    if (Util::scene()->labels.count() < 1)
    {
        emit message(tr("Triangle: invalid number of labels (%1 < 1).").arg(Util::scene()->labels.count()), true);
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
            emit message(tr("Triangle: at least one material has to be assigned."), true);
            return false;
        }
    }
    if (Util::scene()->edgeMarkers.count() < 2) // + none marker
    {
        emit message(tr("Triangle: invalid number of boundary conditions (%1 < 1).").arg(Util::scene()->edgeMarkers.count()), true);
        return false;
    }
    if (Util::scene()->labelMarkers.count() < 2) // + none marker
    {
        emit message(tr("Triangle: invalid number of materials (%1 < 1).").arg(Util::scene()->labelMarkers.count()), true);
        return false;
    }

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    QDir dir;
    dir.mkdir(QDir::temp().absolutePath() + "/agros2d");
    QFile file(tempProblemFileName() + ".poly");

    if (!file.open(QIODevice::WriteOnly))
    {
        emit message(tr("Triangle: could not create triangle poly mesh file."), true);
        return false;
    }
    QTextStream out(&file);


    // Util::scene()->nodes
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
            // add pseudoUtil::scene()->nodes
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
        if (Util::scene()->labelMarkers.indexOf(Util::scene()->labels[i]->marker) > 0)
        {
        outLabels += QString("%1  %2  %3  %4  %5\n").arg(labelsCount).arg(Util::scene()->labels[i]->point.x, 0, 'f', 10).arg(Util::scene()->labels[i]->point.y, 0, 'f', 10).arg(i).arg(Util::scene()->labels[i]->area);
        labelsCount++;
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

bool SolverThread::triangleToHermes2D()
{
    int i, n, k, l, count, marker, node_1, node_2, node_3;
    double x, y;

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    QFile fileMesh(tempProblemFileName() + ".mesh");
    if (!fileMesh.open(QIODevice::WriteOnly))
    {
        emit message(tr("Hermes2D: could not create hermes2d mesh file."), true);
        return false;
    }
    QTextStream outMesh(&fileMesh);

    QFile fileNode(tempProblemFileName() + ".node");
    if (!fileNode.open(QIODevice::ReadOnly))
    {
        emit message(tr("Hermes2D: could not read triangle node file."), true);
        return false;
    }
    QTextStream inNode(&fileNode);

    QFile fileEdge(tempProblemFileName() + ".edge");
    if (!fileEdge.open(QIODevice::ReadOnly))
    {
        emit message(tr("Hermes2D: could not read triangle edge file."), true);
        return false;
    }
    QTextStream inEdge(&fileEdge);

    QFile fileEle(tempProblemFileName() + ".ele");
    if (!fileEle.open(QIODevice::ReadOnly))
    {
        emit message(tr("Hermes2D: could not read triangle ele file."), true);
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
            if (Util::scene()->edges[marker-1]->marker->type != PHYSICFIELDBC_NONE)
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
        emit message(tr("Hermes2D: invalid number of edge markers."), true);
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
        outElements += QString("\t{ %1, %2, %3, %4  }, \n").arg(node_1).arg(node_2).arg(node_3).arg(abs(marker));
    }
    outElements.truncate(outElements.length()-3);
    outElements += "\n} \n\n";
    if (count < 1)
    {
        emit message(tr("Hermes2D: invalid number of label markers."), true);
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

SolverDialog::SolverDialog(QWidget *parent) : QDialog(parent)
{
    setWindowModality(Qt::ApplicationModal);
    setMinimumSize(380, 260);
    setMaximumSize(minimumSize());
    setModal(true);

    setWindowIcon(icon("system-run"));
    setWindowTitle(tr("Solve problem..."));

    createControls();

    thread = new SolverThread();

    connect(thread, SIGNAL(updateProgress(int)), progressBar, SLOT(setValue(int)));
    connect(thread, SIGNAL(message(QString, bool)), this, SLOT(doShowMessage(QString, bool)));
    connect(thread, SIGNAL(solved()), this, SLOT(doSolved()));
    // connect(thread, SIGNAL(terminated()), this, SLOT(hide()));

    resize(minimumSize());
}

SolverDialog::~SolverDialog()
{
    QFile::remove(tempProblemFileName() + ".mesh");

    while (thread->isRunning())
    {
        thread->quit();
        thread->wait(50);
    }
    delete thread;

    delete btnCancel;
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

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
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

    btnCancel->setEnabled(true);

    QApplication::processEvents();
    thread->start(QThread::LowestPriority);

    return exec();
}

void SolverDialog::doShowMessage(const QString &message, bool isError)
{
    if (isError)
        lstMessage->setTextColor(QColor(Qt::red));
    else
        lstMessage->setTextColor(QColor(Qt::black));

    lstMessage->insertPlainText(message + "\n");
    lstMessage->ensureCursorVisible();
    lblMessage->setText(message);

    // update
    foreach (QWidget *widget, QApplication::allWidgets())
        widget->update();
    QApplication::processEvents();
}

void SolverDialog::doSolved()
{
    setFileNameOrig("");

    thread->cancel();
    emit solved();
}

void SolverDialog::doCancel()
{
    if (!thread->isCanceled())
    {
        setFileNameOrig("");

        btnCancel->setEnabled(false);
        thread->cancel();

        while (thread->isRunning())
        {
            QApplication::processEvents();
            thread->wait(100);
        }
    }
}
