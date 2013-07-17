#include "pythonengine.h"

#include "compile.h"
#include "frameobject.h"

#include "../resources_source/python/pythonlab.cpp"

static PythonEngine *pythonEngine = NULL;

// silent mode
static bool m_silentMode = false;

// create custom python engine
void createPythonEngine(PythonEngine *custom)
{
    if (custom)
        pythonEngine = custom;
    else
        pythonEngine = new PythonEngine();

    pythonEngine->init();
}

// current python engine
PythonEngine *currentPythonEngine()
{
    return pythonEngine;
}

// silent mode
void setSilentMode(bool mode)
{
    m_silentMode = mode;
}

bool silentMode()
{
    return m_silentMode;
}

// ****************************************************************************

// print stdout
PyObject* pythonStdout(PyObject* self, PyObject* pArgs)
{
    char *str = NULL;
    if (PyArg_ParseTuple(pArgs, "s", &str))
    {
        emit currentPythonEngine()->pythonShowMessageCommand(QString(str) + "\n");
        Py_RETURN_NONE;
    }
    return NULL;
}

// show image
PyObject* pythonShowFigure(PyObject* self, PyObject* pArgs)
{
    char *str = NULL;
    int width = 0;
    int height = 0;
    if (PyArg_ParseTuple(pArgs, "s|ii", &str, &width, &height))
    {
        emit currentPythonEngine()->pythonShowImageCommand(QString(str), width, height);
        Py_RETURN_NONE;
    }
    return NULL;
}

// print html
PyObject* pythonInsertHtml(PyObject* self, PyObject* pArgs)
{
    char *str = NULL;
    if (PyArg_ParseTuple(pArgs, "s", &str))
    {
        emit currentPythonEngine()->pythonShowHtmlCommand(QString(str));
        Py_RETURN_NONE;
    }
    return NULL;
}

// clear
static PyObject* pythonClear(PyObject* self, PyObject* pArgs)
{
    emit currentPythonEngine()->pythonClearCommand();

    Py_RETURN_NONE;
    return NULL;
}

int scriptQuit(void *)
{
    PyErr_SetString(PyExc_SystemError, "Script interrupted.");
    PyErr_SetInterrupt();

    return 0;
}

static PyObject *pythonTempname(PyObject* self, PyObject* pArgs)
{
    QString tempDir = tempProblemDir() + "/temp/";
    QDir(tempDir).mkdir(tempDir);

    QString tempName = QUuid::createUuid().toString().remove("{").remove("}");

    char *str = "";
    if (PyArg_ParseTuple(pArgs, "|s", &str))
    {
        if (str != "")
            tempName = tempName + "." + str;
    }

    return PyString_FromString(compatibleFilename(tempDir + tempName).toLatin1().data());
}

static PyMethodDef pythonEngineFuntions[] =
{
    {"__stdout__", pythonStdout, METH_VARARGS, "__stdout__(str)"},
    {"image", pythonShowFigure, METH_VARARGS, "image(file)"},
    {"clear", pythonClear, METH_NOARGS, "clear()"},
    {"html", pythonInsertHtml, METH_VARARGS, "html(str)"},
    {"tempname", pythonTempname, METH_VARARGS, "tempname(extension = \"\")"},
    {NULL, NULL, 0, NULL}
};

// ****************************************************************************

// tracing
int traceFunction(PyObject *obj, _frame *frame, int what, PyObject *arg)
{
    PyObject *str = PyObject_Str(frame->f_code->co_filename);
    if (str)
    {
        QString fileName = QString::fromStdString(PyString_AsString(str));
        Py_DECREF(str);

        if (!currentPythonEngine()->profilerFileName().isEmpty() && fileName.contains(currentPythonEngine()->profilerFileName()))
        {
            // catch line change events, print the filename and line number
            if (what == PyTrace_LINE)
            {
                currentPythonEngine()->profilerAddLine(frame->f_lineno);
                // qDebug() << "PyTrace_LINE: " << fileName << ": " << frame->f_lineno;
            }
            // catch line change events, print the filename and line number
            if (what == PyTrace_CALL)
            {
                // qDebug() << "PyTrace_CALL: " << fileName << ": " << frame->f_lineno;
            }
            // catch line change events, print the filename and line number
            if (what == PyTrace_RETURN)
            {
                // qDebug() << "PyTrace_RETURN: " << fileName << ": " << frame->f_lineno;
            }
        }
    }

    return 0;
}

void PythonEngineProfiler::startProfiler()
{
    // set trace callback
    PyEval_SetTrace(traceFunction, NULL);
    m_profilerTime.restart();
    profilerAddLine(0);
}

void PythonEngineProfiler::finishProfiler()
{
    // add last time
    profilerAddLine(m_profilerLines.last() + 1);

    // remove trace callback
    PyEval_SetTrace(NULL, NULL);

    assert(m_profilerLines.count() == m_profilerTimes.count());
    // clear accumulated lines and times
    m_profilerAccumulatedLines.clear();
    m_profilerAccumulatedTimes.clear();
    // store accumulated time
    m_profilerMaxAccumulatedCall = 0;
    m_profilerMaxAccumulatedTime = 0;
    for (int i = 1; i < m_profilerLines.count(); i++)
    {
        m_profilerAccumulatedLines[m_profilerLines.at(i-1)]++;
        m_profilerAccumulatedTimes[m_profilerLines.at(i-1)] += (m_profilerTimes.at(i) - m_profilerTimes.at(i - 1));

        if (m_profilerAccumulatedTimes[m_profilerLines.at(i-1)] > m_profilerMaxAccumulatedTime)
        {
            m_profilerMaxAccumulatedLine = m_profilerLines.at(i-1);
            m_profilerMaxAccumulatedTime = m_profilerAccumulatedTimes[m_profilerLines.at(i-1)];
        }
        if (m_profilerAccumulatedLines[m_profilerLines.at(i-1)] > m_profilerMaxAccumulatedCall)
        {
            m_profilerMaxAccumulatedCallLine = m_profilerLines.at(i-1);
            m_profilerMaxAccumulatedCall = m_profilerAccumulatedLines[m_profilerLines.at(i-1)];
        }
    }

    // clear temp variables
    m_profilerLines.clear();
    m_profilerTimes.clear();
}


// ****************************************************************************

PythonEngine::~PythonEngine()
{
    // finalize and garbage python
    Py_DECREF(m_dict);
    Py_DECREF(m_dict);

    if (Py_IsInitialized())
        Py_Finalize();
}

void PythonEngine::init()
{
    m_isRunning = false;
    m_stdOut = "";

    // connect stdout
    connect(this, SIGNAL(pythonShowMessage(QString)), this, SLOT(stdOut(QString)));

    // init python
    PyEval_InitThreads();
    Py_Initialize();

    // read pythonlab functions
    addFunctions(readFileContent(datadir() + "/functions_pythonlab.py"));
    addCustomFunctions();

    m_dict = PyDict_New();
    PyDict_SetItemString(m_dict, "__builtins__", PyEval_GetBuiltins());

    // init engine extensions
    Py_InitModule("pythonlab", pythonEngineFuntions);

    addCustomExtensions();

    // custom modules
    PyRun_String(QString("import sys; sys.path.insert(0, \"" + datadir() + "/resources/python" + "\")").toLatin1().data(), Py_file_input, m_dict, m_dict);

    // functions.py
    PyRun_String(m_functions.toLatin1().data(), Py_file_input, m_dict, m_dict);
}

void PythonEngine::abortScript()
{
    Py_AddPendingCall(&scriptQuit, NULL);
}

void PythonEngine::pythonShowMessageCommand(const QString &message)
{
    if (message != "\n\n")
        emit pythonShowMessage(message);
}

void PythonEngine::pythonShowImageCommand(const QString &fileName, int width, int height)
{
    emit pythonShowImage(fileName, width, height);
}

void PythonEngine::pythonShowHtmlCommand(const QString &fileName)
{
    emit pythonShowHtml(fileName);
}

void PythonEngine::pythonClearCommand()
{
    emit pythonClear();
}

void PythonEngine::stdOut(const QString &message)
{
    m_stdOut.append(message);
}

void PythonEngine::deleteUserModules()
{
    // delete all user modules
    //
    // When working with Python scripts interactively, one must keep in mind that Python
    // import a module from its source code (on disk) only when parsing the first corresponding
    // import statement. During this first import, the byte code is generated (.pyc file)
    // if necessary and the imported module code object is cached in sys.modules. Then, when
    // re-importing the same module, this cached code object will be directly used even
    // if the source code file (.py[w] file) has changed meanwhile.
    //
    // This behavior is sometimes unexpected when working with the Python interpreter in
    // interactive mode, because one must either always restart the interpreter or remove manually the .pyc
    // files to be sure that changes made in imported modules were taken into account.

    QStringList filter_name;
    filter_name << "pythonlab" << "agros2d" << "sys";

    QList<PythonVariable> list = variableList();

    foreach (PythonVariable variable, list)
    {
        if (variable.type == "module")
        {
            if (filter_name.contains(variable.name))
                continue;

            QString exp = QString("del %1; import sys; del sys.modules[\"%1\"]").arg(variable.name);
            // qDebug() << exp;
            PyRun_String(exp.toLatin1().data(), Py_single_input, m_dict, m_dict);
        }
    }

    PyErr_Clear();
}

bool PythonEngine::runScript(const QString &script, const QString &fileName, bool useProfiler)
{
    m_isRunning = true;
    m_stdOut = "";

    PyGILState_STATE gstate = PyGILState_Ensure();

    emit startedScript();

    QSettings settings;
    // enable user module deleter
    if (settings.value("PythonEngine/UserModuleDeleter", true).toBool())
        deleteUserModules();

    runPythonHeader();

    PyObject *output = NULL;
    if (QFile::exists(fileName))
    {
        QString str = QString("from os import chdir; chdir(u'" + QFileInfo(fileName).absolutePath() + "')");
        PyRun_String(str.toLatin1().data(), Py_single_input, m_dict, m_dict);
    }
    // compile
    PyObject *code = Py_CompileString(script.toLatin1().data(), fileName.toLatin1().data(), Py_file_input);
    // run
    if (useProfiler)
    {
        setProfilerFileName(fileName);
        startProfiler();
    }
    if (code) output = PyEval_EvalCode((PyCodeObject *) code, m_dict, m_dict);
    if (useProfiler)
        finishProfiler();

    bool successfulRun = false;
    if (output)
        successfulRun = true;

    Py_XDECREF(output);

    m_isRunning = false;

    // release the thread, no Python API allowed beyond this point
    PyGILState_Release(gstate);

    emit executedScript();

    return successfulRun;
}

ExpressionResult PythonEngine::runExpression(const QString &expression, bool returnValue)
{
    ExpressionResult expressionResult;

    runPythonHeader();

    QString exp;
    if (returnValue)
        exp = QString("result_pythonlab = %1").arg(expression);
    else
        exp = expression;

#pragma omp atomic
    {
        PyObject *output = PyRun_String(exp.toLatin1().data(), Py_single_input, m_dict, m_dict);

        if (output)
        {
            PyObject *type = NULL, *value = NULL, *traceback = NULL, *str = NULL;
            PyErr_Fetch(&type, &value, &traceback);

            if (type != NULL && (str = PyObject_Str(type)) != NULL && (PyString_Check(str)))
            {
                Py_INCREF(str);
                expressionResult.error = PyString_AsString(str);
                Py_XDECREF(type);
                Py_XDECREF(str);
            }
            else
            {
                // parse result
                if (returnValue)
                {
                    PyObject *result = PyDict_GetItemString(m_dict, "result_pythonlab");


                    if (result)
                    {
                        if ((QString(result->ob_type->tp_name) == "bool") ||
                                (QString(result->ob_type->tp_name) == "int") ||
                                (QString(result->ob_type->tp_name) == "float"))
                        {
                            Py_INCREF(result);
                            PyArg_Parse(result, "d", &expressionResult.value);
                            if (fabs(expressionResult.value) < EPS_ZERO)
                                expressionResult.value = 0.0;
                            Py_XDECREF(result);
                        }
                        else
                        {
                            qDebug() << tr("Type '%1' is not supported.").arg(result->ob_type->tp_name).arg(expression);
                            expressionResult.error = tr("Type '%1' is not supported.").arg(result->ob_type->tp_name);
                            expressionResult.value = 0.0;
                        }
                    }

                    PyRun_String("del result_pythonlab", Py_single_input, m_dict, m_dict);
                }
            }
        }
        else
        {
            ScriptResult error = parseError();
            expressionResult.error = error.text;
            expressionResult.traceback = error.traceback;
        }
        Py_XDECREF(output);

    }

    emit executedExpression();

    return expressionResult;
}

QStringList PythonEngine::codeCompletion(const QString& code, int offset, const QString& fileName)
{
    runPythonHeader();

    QStringList out;

    QString exp;
    if (QFile::exists(fileName))
    {
        exp = QString("result_rope_pythonlab = python_engine_get_completion_file(\"%1\", %2)").
                arg(compatibleFilename(fileName)).
                arg(offset);
    }
    else
    {
        QString str = code;
        // if (str.lastIndexOf("=") != -1)
        //    str = str.right(str.length() - str.lastIndexOf("=") - 1);

        for (int i = 33; i <= 126; i++)
        {
            // skip numbers and alphabet and dot
            if ((i >= 48 && i <= 57) || (i >= 65 && i <= 90) || (i >= 97 && i <= 122) || (i == 46))
                continue;

            QChar c(i);
            // qDebug() << c << ", " << str.lastIndexOf(c) << ", " << str.length();

            if (str.lastIndexOf(c) != -1)
            {
                str = str.right(str.length() - str.lastIndexOf(c) - 1);
                break;
            }
        }

        if (str.contains("."))
        {
            QString search = str.left(str.lastIndexOf("."));
            exp = QString("result_rope_pythonlab = python_engine_get_completion_string_dot(\"%1\")").
                    arg(search);
        }
        else
            exp = QString("result_rope_pythonlab = python_engine_get_completion_string(\"%1\", %2)").
                    arg(str.trimmed()).
                    arg(str.trimmed().length());
    }

    // QTime time;
    // time.start();
    PyObject *output = PyRun_String(exp.toLatin1().data(), Py_single_input, m_dict, m_dict);
    // qDebug() << time.elapsed();

    // parse result
    if (output)
    {
        PyObject *result = PyDict_GetItemString(m_dict, "result_rope_pythonlab");
        if (result)
        {
            Py_INCREF(result);
            PyObject *list;
            if (PyArg_Parse(result, "O", &list))
            {
                int count = PyList_Size(list);
                for (int i = 0; i < count; i++)
                {
                    PyObject *item = PyList_GetItem(list, i);

                    QString str = PyString_AsString(item);

                    // remove builtin methods
                    if (!str.startsWith("__"))
                        out.append(str);
                }
            }
            Py_DECREF(result);
        }

        PyRun_String("del result_rope_pythonlab", Py_single_input, m_dict, m_dict);
    }
    else
    {
        PyErr_Clear();
    }

    return out;
}

QStringList PythonEngine::codePyFlakes(const QString& fileName)
{
    QStringList out;

    if (!isRunning())
    {
        QString exp = QString("result_pyflakes_pythonlab = python_engine_pyflakes_check(\"%1\")").arg(compatibleFilename(fileName));

        PyRun_String(exp.toLatin1().data(), Py_single_input, m_dict, m_dict);

        // parse result
        PyObject *result = PyDict_GetItemString(m_dict, "result_pyflakes_pythonlab");
        if (result)
        {
            Py_INCREF(result);
            PyObject *list;
            if (PyArg_Parse(result, "O", &list))
            {
                int count = PyList_Size(list);
                for (int i = 0; i < count; i++)
                {
                    PyObject *item = PyList_GetItem(list, i);

                    QString str = PyString_AsString(item);
                    out.append(str);
                }
            }
            Py_DECREF(result);
        }

        PyRun_String("del result_pyflakes_pythonlab", Py_single_input, m_dict, m_dict);
    }

    return out;
}

ScriptResult PythonEngine::parseError()
{
    // error
    ScriptResult error;
    error.isError = true;

    PyObject *error_type = NULL;
    PyObject *error_value = NULL;
    PyObject *error_traceback = NULL;
    PyObject *error_string = NULL;
    PyErr_Fetch(&error_type, &error_value, &error_traceback);
    PyErr_NormalizeException(&error_type, &error_value, &error_traceback);

    if (error_traceback)
    {
        PyTracebackObject *traceback = (PyTracebackObject *) error_traceback;
        error.line = traceback->tb_lineno;
        error.text.append(QString("Line %1: ").arg(traceback->tb_lineno));

        while (traceback)
        {
            PyFrameObject *frame = traceback->tb_frame;

            if (frame && frame->f_code) {
                PyCodeObject* codeObject = frame->f_code;
                if (PyString_Check(codeObject->co_filename))
                    error.traceback.append(QString("File '%1'").arg(PyString_AsString(codeObject->co_filename)));

                int errorLine = PyCode_Addr2Line(codeObject, frame->f_lasti);
                error.traceback.append(QString(", line %1").arg(errorLine));

                if (PyString_Check(codeObject->co_name))
                    error.traceback.append(QString(", in %1").arg(PyString_AsString(codeObject->co_name)));
            }
            error.traceback.append(QString("\n"));

            traceback = traceback->tb_next;
        }
    }
    error.traceback = error.traceback.trimmed();

    if (error_type != NULL && (error_string = PyObject_Str(error_type)) != NULL && (PyString_Check(error_string)))
    {
        Py_INCREF(error_string);
        error.text.append(PyString_AsString(error_string));
        Py_XDECREF(error_string);
    }
    else
    {
        error.text.append("\n<unknown exception type>");
    }

    if (error_value != NULL && (error_string = PyObject_Str(error_value)) != NULL && (PyString_Check(error_string)))
    {
        Py_INCREF(error_string);
        error.text.append("\n");
        error.text.append(PyString_AsString(error_string));
        Py_XDECREF(error_string);
    }
    else
    {
        error.text.append("\n<unknown exception data>");
    }

    Py_XDECREF(error_type);
    Py_XDECREF(error_value);
    Py_XDECREF(error_traceback);

    PyErr_Clear();

    return error;
}

void PythonEngine::addCustomExtensions()
{
    // init pythonlab cython extensions
    initpythonlab();
}

QList<PythonVariable> PythonEngine::variableList()
{
    QStringList filter_name;
    filter_name << "__builtins__" << "StdoutCatcher" << "python_engine_stdout" << "chdir"
                << "python_engine_get_completion_file" << "python_engine_get_completion_string"
                << "python_engine_get_completion_string_dot" << "PythonLabRopeProject"
                << "pythonlab_rope_project"
                << "python_engine_pyflakes_check"
                << "CatchOutErr"
                << "agros2d_material_eval"
                << "__a2d_help__";

    QStringList filter_type;
    filter_type << "builtin_function_or_method";

    QList<PythonVariable> list;

    PyObject *keys = PyDict_Keys(m_dict);
    for (int i = 0; i < PyList_Size(keys); ++i)
    {
        PyObject *key = PyList_GetItem(keys, i);
        PyObject *value = PyDict_GetItem(m_dict, key);

        // variable
        PythonVariable var;

        // variable name
        var.name = PyString_AsString(key);

        // variable type
        var.type = value->ob_type->tp_name;

        // variable value
        if (var.type == "bool")
        {
            var.value = PyInt_AsLong(value) ? "True" : "False";
        }
        else if (var.type == "int")
        {
            var.value = (int) PyInt_AsLong(value);
        }
        else if (var.type == "float")
        {
            var.value = PyFloat_AsDouble(value);
        }
        else if (var.type == "str")
        {
            var.value = PyString_AsString(value);
        }
        else if (var.type == "list")
        {
            var.value = QString("%1 items").arg(PyList_Size(value));
        }
        else if (var.type == "tuple")
        {
            var.value = QString("%1 items").arg(PyTuple_Size(value));
        }
        else if (var.type == "dict")
        {
            var.value = QString("%1 items").arg(PyDict_Size(value));
        }
        else if (var.type == "numpy.ndarray")
        {
            var.value = ""; //TODO count
        }
        else if (var.type == "module")
        {
            var.value = PyString_AsString(PyObject_GetAttrString(value, "__name__"));
        }
        else if (var.type == "function"
                 || var.type == "instance"
                 || var.type == "classobj")
        {
            // qDebug() << value->ob_type->tp_name;
        }

        // append
        if (!filter_name.contains(var.name) && !filter_type.contains(var.type))
        {
            list.append(var);
        }
    }
    Py_DECREF(keys);

    return list;
}
