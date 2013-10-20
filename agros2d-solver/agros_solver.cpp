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

#include "agros_solver.h"

#include "util/global.h"

#include "scenenode.h"
#include "logview.h"
#include "pythonlab/pythonengine_agros.h"

#include "hermes2d.h"

AgrosSolver::AgrosSolver(int &argc, char **argv)
    : AgrosApplication(argc, argv), m_log(NULL), m_enableLog(false)
{    
}

AgrosSolver::~AgrosSolver()
{
    if (m_log)
        delete m_log;
}

// reimplemented from QApplication so we can throw exceptions in slots
bool AgrosSolver::notify(QObject *receiver, QEvent *event)
{
    try
    {
        return QCoreApplication::notify(receiver, event);
    }
    catch (Hermes::Exceptions::Exception& e)
    {
        qCritical() << "Hermes exception thrown: " << QString("%1").arg(e.what());
        QApplication::exit(1);
    }
    catch (std::exception& e)
    {
        qCritical() << "Exception thrown: " << e.what();
        QApplication::exit(1);
    }
    catch (AgrosException e)
    {
        qCritical() << "Exception thrown: " << e.what();
        QApplication::exit(1);
    }
    catch (...)
    {
        qCritical() << "Unknown exception thrown";
        QApplication::exit(1);
    }

    return false;
}

void AgrosSolver::solveProblem()
{
    // log stdout
    if (m_enableLog)
        m_log = new LogStdOut();

    createPythonEngine(new PythonEngineAgros());

    QTime time;
    time.start();

    try
    {
        Agros2D::scene()->readFromFile(m_fileName);

        Agros2D::log()->printMessage(tr("Problem"), tr("Problem '%1' successfuly loaded").arg(m_fileName));

        // solve
        Agros2D::problem()->solve(false, true);
        // save solution
        Agros2D::scene()->writeSolutionToFile(m_fileName);

        Agros2D::log()->printMessage(tr("Solver"), tr("Problem was solved in %1").arg(milisecondsToTime(time.elapsed()).toString("mm:ss.zzz")));

        // clear all
        Agros2D::problem()->clearFieldsAndConfig();

        QApplication::exit(0);
    }
    catch (AgrosException &e)
    {
        Agros2D::log()->printError(tr("Problem"), e.toString());
        QApplication::exit(-1);
    }
}

void AgrosSolver::runScript()
{
    // log stdout
    if (m_enableLog)
        new LogStdOut();

    if (!QFile::exists(m_fileName))
    {
        Agros2D::log()->printMessage(tr("Scripting Engine"), tr("Python script '%1' not found").arg(m_fileName));
        QApplication::exit(-1);
    }

    createPythonEngine(new PythonEngineAgros());

    QTime time;
    time.start();

    // silent mode
    setSilentMode(true);

    connect(currentPythonEngineAgros(), SIGNAL(pythonShowMessage(QString)), this, SLOT(stdOut(QString)));
    connect(currentPythonEngineAgros(), SIGNAL(pythonShowHtml(QString)), this, SLOT(stdHtml(QString)));

    bool successfulRun= currentPythonEngineAgros()->runScript(readFileContent(m_fileName), m_fileName);

    if (successfulRun)
    {
        Agros2D::log()->printMessage(tr("Solver"), tr("Problem was solved in %1").arg(milisecondsToTime(time.elapsed()).toString("mm:ss.zzz")));

        Agros2D::scene()->clear();
        Agros2D::clear();
        QApplication::exit(0);
    }
    else
    {
        ErrorResult result = currentPythonEngineAgros()->parseError();
        Agros2D::log()->printMessage(tr("Scripting Engine"), tr("%1\nLine: %2\nStacktrace:\n%3\n").
                                  arg(result.error()).
                                  arg(result.line()).
                                  arg(result.traceback()));

        QApplication::exit(-1);
    }
}

void AgrosSolver::stdOut(const QString &str)
{
    std::cout << str.toStdString();
}

void AgrosSolver::stdHtml(const QString &str)
{
    std::cout << str.toStdString();
}
