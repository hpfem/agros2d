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

class AGROS_API PythonEngine : public QObject
{
    Q_OBJECT

signals:
    void pythonClear();
    void pythonShowMessage(const QString &);
    void pythonShowHtml(const QString &);
    void pythonShowImage(const QString &);

    void executedExpression();
    void executedScript();

public:
    PythonEngine() {}
    ~PythonEngine();

    void init();

    // python commands
    void pythonClearCommand();
    void pythonShowMessageCommand(const QString &message);
    void pythonShowHtmlCommand(const QString &fileName);
    void pythonShowImageCommand(const QString &fileName);

    ScriptResult runScript(const QString &script, const QString &fileName = "");
    ExpressionResult runExpression(const QString &expression, bool returnValue);
    ScriptResult parseError();
    inline bool isRunning() { return m_isRunning; }

    void deleteUserModules();
    QStringList codeCompletion(const QString& code, int offset, const QString& fileName = "");
    QStringList codePyFlakes(const QString& fileName);
    QList<PythonVariable> variableList();

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
};

// create custom python engine
AGROS_API void createPythonEngine(PythonEngine *custom = NULL);

// current python engine
AGROS_API PythonEngine *currentPythonEngine();

// silent mode
AGROS_API void setSilentMode(bool mode);
AGROS_API bool silentMode();

#endif // PYTHONENGINE_H
