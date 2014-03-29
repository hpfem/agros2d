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

#ifndef PYTHONENGINE_H
#define PYTHONENGINE_H

#include "util.h"

typedef struct _object PyObject;

struct PythonVariable
{
    QString name;
    QString type;
    QVariant value;
};

class AGROS_PYTHONLAB_API PythonEngineProfiler
{
public:
    PythonEngineProfiler() {}

    inline void profilerAddLine(int line)
    {
        m_profilerLines.append(line);;
        m_profilerTimes.append(m_profilerTime.elapsed());
    }
    inline QMap<int, int> profilerAccumulatedLines() const { return m_profilerAccumulatedLines; }
    inline QMap<int, int> profilerAccumulatedTimes() const { return m_profilerAccumulatedTimes; }

    inline int profilerMaxAccumulatedLine() const { return m_profilerMaxAccumulatedLine; }
    inline int profilerMaxAccumulatedTime() const { return m_profilerMaxAccumulatedTime; }
    inline int profilerMaxAccumulatedCallLine() const { return m_profilerMaxAccumulatedCallLine; }
    inline int profilerMaxAccumulatedCall() const { return m_profilerMaxAccumulatedCall; }

    void startProfiler();
    void finishProfiler();

    inline void setProfilerFileName(const QString &fileName) { m_profilerFileName = fileName; }
    inline QString profilerFileName() { return m_profilerFileName; }


private:
    QTime m_profilerTime;
    QList<int> m_profilerTimes;
    QList<int> m_profilerLines;
    QMap<int, int> m_profilerAccumulatedLines;
    QMap<int, int> m_profilerAccumulatedTimes;

    int m_profilerMaxAccumulatedLine;
    int m_profilerMaxAccumulatedTime;
    int m_profilerMaxAccumulatedCallLine;
    int m_profilerMaxAccumulatedCall;

    QString m_profilerFileName;
};

class ErrorResult
{
public:
    ErrorResult() : m_error(""), m_traceback(""), m_line(-1) {}

    ErrorResult(const QString &error, const QString &traceback, int line = -1)
    {
        this->m_error = error;
        this->m_traceback = traceback;
        this->m_line = line;
    }

    inline QString error() { return m_error; }
    inline QString traceback() { return m_traceback; }
    inline int line() { return m_line; }

private:
    QString m_error;
    QString m_traceback;
    int m_line;
};

class AGROS_PYTHONLAB_API PythonEngine : public QObject, public PythonEngineProfiler
{
    Q_OBJECT

signals:
    void pythonClear();
    void pythonShowMessage(const QString &);
    void pythonShowHtml(const QString &);
    void pythonShowImage(const QString &, int, int);

    void executedScript();
    void startedScript();

public:
    PythonEngine() : errorType(NULL), errorValue(NULL), errorTraceback(NULL) {}
    ~PythonEngine();

    void init();

    // python commands
    void pythonClearCommand();
    void pythonShowMessageCommand(const QString &message);
    void pythonShowHtmlCommand(const QString &fileName);
    void pythonShowImageCommand(const QString &fileName, int width = 0, int height = 0);

    bool runScript(const QString &script, const QString &fileName = "", bool useProfiler = false);
    bool runExpression(const QString &expression, double *value = NULL, const QString &command = QString());
    bool runExpressionConsole(const QString &expression);
    ErrorResult parseError();
    inline bool isScriptRunning() { return m_isScriptRunning; }

    void deleteUserModules();
    QStringList codeCompletionScript(const QString& code, int row, int column, const QString& fileName = "");
    QStringList codeCompletionInterpreter(const QString& code);
    QStringList codeCompletion(const QString& command);
    QStringList codePyFlakes(const QString& fileName);
    QList<PythonVariable> variableList();

    inline PyObject *dict() const { return m_dict; }

public slots:
    virtual void abortScript();

protected:
    PyObject *m_dict;
    bool m_isScriptRunning;

    inline void addFunctions(const QString& code) { m_functions += "\n\n" + code; }
    virtual void addCustomExtensions();
    virtual void addCustomFunctions() {}
    virtual void runPythonHeader() {}

private:
    QString m_functions;

    PyObject *errorType;
    PyObject *errorValue;
    PyObject *errorTraceback;
};

// create custom python engine
AGROS_PYTHONLAB_API void createPythonEngine(PythonEngine *custom = NULL);

// current python engine
AGROS_PYTHONLAB_API PythonEngine *currentPythonEngine();

// silent mode
AGROS_PYTHONLAB_API void setSilentMode(bool mode);
AGROS_PYTHONLAB_API bool silentMode();

#endif // PYTHONENGINE_H
