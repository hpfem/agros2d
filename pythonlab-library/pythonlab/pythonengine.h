#ifndef PYTHONENGINE_H
#define PYTHONENGINE_H

#include "util.h"
#include <Python.h>

struct PythonVariable
{
    QString name;
    QString type;
    QVariant value;
};

class AGROS_API PythonEngineProfiler
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

class AGROS_API PythonEngine : public QObject, public PythonEngineProfiler
{
    Q_OBJECT

signals:
    void pythonClear();
    void pythonShowMessage(const QString &);
    void pythonShowHtml(const QString &);
    void pythonShowImage(const QString &, int, int);

    void executedExpression();
    void executedScript();
    void startedScript();

public:
    PythonEngine() {}
    ~PythonEngine();

    void init();

    // python commands
    void pythonClearCommand();
    void pythonShowMessageCommand(const QString &message);
    void pythonShowHtmlCommand(const QString &fileName);
    void pythonShowImageCommand(const QString &fileName, int width = 0, int height = 0);

    bool runScript(const QString &script, const QString &fileName = "", bool useProfiler = false);
    ExpressionResult runExpression(const QString &expression, bool returnValue);
    ScriptResult parseError();
    inline bool isRunning() { return m_isRunning; }

    void deleteUserModules();
    QStringList codeCompletion(const QString& code, int offset, const QString& fileName = "");
    QStringList codePyFlakes(const QString& fileName);
    QList<PythonVariable> variableList();

public slots:
    void stopScript();

protected:
    PyObject *m_dict;
    bool m_isRunning;

    inline void addFunctions(const QString& code) { m_functions += "\n\n" + code; }
    virtual void addCustomExtensions();
    virtual void addCustomFunctions() {}
    virtual void runPythonHeader() {}

private slots:
    void stdOut(const QString &message);    

private:
    QString m_stdOut;
    QString m_functions;
    QMutex m_mutex;
};

// create custom python engine
AGROS_API void createPythonEngine(PythonEngine *custom = NULL);

// current python engine
AGROS_API PythonEngine *currentPythonEngine();

// silent mode
AGROS_API void setSilentMode(bool mode);
AGROS_API bool silentMode();

#endif // PYTHONENGINE_H
