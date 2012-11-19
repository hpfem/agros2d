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

#include "scenenode.h"
#include "logview.h"
#include "pythonlab_agros.h"

#include "hermes2d.h"

ArgosSolver::ArgosSolver(int &argc, char **argv)
    : QApplication(argc, argv) {}

// reimplemented from QApplication so we can throw exceptions in slots
bool ArgosSolver::notify(QObject *receiver, QEvent *event)
{
    try
    {
        return QCoreApplication::notify(receiver, event);
    }
    catch (std::exception& e)
    {
        qCritical() << "Exception thrown: " << e.what();
        throw;
    }
    catch (Hermes::Exceptions::Exception& e)
    {
        qCritical() << "Hermes exception thrown: " << QString("%1").arg(e.what());
        throw;
    }
    catch (AgrosException e)
    {
        qCritical() << "Exception thrown: " << e.what();
        throw;
    }
    catch (...)
    {
        qCritical() << "Unknown exception thrown";
        throw;
    }

    return false;
}

void ArgosSolver::solveProblem()
{  
    Util::createSingleton();

    // log stdout
    new LogStdOut();

    createPythonEngine(new PythonEngineAgros());

    QTime time;
    time.start();

    ErrorResult result = Util::scene()->readFromFile(m_fileName);
    if (result.isError())
    {
        Util::log()->printMessage(tr("Solver"), tr("%1\n.").arg(result.message()));

        QApplication::exit(-1);
    }
    else
    {
        Util::log()->printMessage(tr("Solver"), tr("Problem '%1' successfuly loaded.").arg(m_fileName));
        Util::problem()->solve();
        Util::log()->printMessage(tr("Solver"), tr("Problem was solved in %1.").arg(milisecondsToTime(time.elapsed()).toString("mm:ss.zzz")));

        QApplication::exit(0);
    }
}

void ArgosSolver::runScript()
{
    Util::createSingleton();

    // log stdout
    new LogStdOut();

    if (!QFile::exists(m_fileName))
    {
        Util::log()->printMessage(tr("Solver"), tr("Python script '%1' not found.").arg(m_fileName));
        QApplication::exit(-1);
    }

    createPythonEngine(new PythonEngineAgros());

    QTime time;
    time.start();

    // silent mode
    setSilentMode(true);

    connect(currentPythonEngineAgros(), SIGNAL(pythonShowMessage(QString)), this, SLOT(stdOut(QString)));
    connect(currentPythonEngineAgros(), SIGNAL(pythonShowHtml(QString)), this, SLOT(stdHtml(QString)));

    ScriptResult result = currentPythonEngineAgros()->runScript(readFileContent(m_fileName));
    if (result.isError)
    {
        Util::log()->printMessage(tr("Solver"), tr("%1\nLine: %2\nStacktrace:\n%3\n.").
                                  arg(result.text).
                                  arg(result.line).
                                  arg(result.traceback));

        QApplication::exit(-1);
    }
    else
    {
        Util::log()->printMessage(tr("Solver"), tr("Problem was solved in %1.").arg(milisecondsToTime(time.elapsed()).toString("mm:ss.zzz")));

        QApplication::exit(0);
    }
}

void ArgosSolver::stdOut(const QString &str)
{
    std::cout << str.toStdString() << std::endl;
}

void ArgosSolver::stdHtml(const QString &str)
{
    std::cout << str.toStdString() << std::endl;
}
