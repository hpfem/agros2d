// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef AGROS_SOLVER_H
#define AGROS_SOLVER_H

#include "util.h"
#include "util/global.h"

class LogStdOut;

class AgrosSolver : public AgrosApplication
{
    Q_OBJECT

public:
    AgrosSolver(int& argc, char ** argv);
    ~AgrosSolver();

    // reimplemented from QApplication so we can throw exceptions in slots
    virtual bool notify(QObject *receiver, QEvent *event);

    inline void setCommand(const QString &command) { m_command = command; }
    inline void setFileName(const QString &fileName) { m_fileName = fileName; }
    inline void setEnableLog(bool enableLog = true) { m_enableLog = enableLog; }
    inline void setScriptSuite(const QString &name) { m_suiteName = name; }

public slots:
    void solveProblem();
    void runCommand();
    void runScript();
    void runSuite();
    void printTestSuites();

private slots:
    void stdOut(const QString &str);
    void stdHtml(const QString &str);

private:
    QString m_fileName;
    QString m_command;
    QString m_suiteName;
    bool m_enableLog;
    LogStdOut *m_log;
};

#endif // AGROS_SOLVER_H
