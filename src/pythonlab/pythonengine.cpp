#include "pythonengine.h"

#include "compile.h"
#include "frameobject.h"

static PythonEngine *pythonEngine = NULL;

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
    // assert(pythonEngine);
    return pythonEngine;
}

// ****************************************************************************

static PyMethodDef pythonEngineFuntions[] =
{
    {"capturestdout", pythonCaptureStdout, METH_VARARGS, "stdout"},
    {NULL, NULL, 0, NULL}
};

// print stdout
PyObject* pythonCaptureStdout(PyObject* self, PyObject* pArgs)
{
    char *str = NULL;
    if (PyArg_ParseTuple(pArgs, "s", &str))
    {
        emit currentPythonEngine()->showMessage(QString(str) + "\n");
        Py_RETURN_NONE;
    }
    return NULL;
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
    connect(this, SIGNAL(printStdOut(QString)), this, SLOT(stdOut(QString)));

    // init python
    Py_Initialize();

    // read functions
    m_functions = readFileContent(datadir() + "/functions.py");

    m_dict = PyDict_New();
    PyDict_SetItemString(m_dict, "__builtins__", PyEval_GetBuiltins());

    // init engine extensions
    Py_InitModule("python_engine", pythonEngineFuntions);

    addCustomExtensions();

    // stdout
    PyRun_String(QString("agrosstdout = \"" + tempProblemDir() + "/stdout.txt" + "\"").toStdString().c_str(), Py_file_input, m_dict, m_dict);

    // functions.py
    PyRun_String(m_functions.toStdString().c_str(), Py_file_input, m_dict, m_dict);
}

void PythonEngine::showMessage(const QString &message)
{
    if (message != "\n\n")
        emit printStdOut(message);
}

void PythonEngine::stdOut(const QString &message)
{
    m_stdOut.append(message);
}

ScriptResult PythonEngine::runPythonScript(const QString &script, const QString &fileName)
{
    m_isRunning = true;
    m_stdOut = "";

    runPythonHeader();

    PyObject *output = NULL;
    if (QFile::exists(fileName))
    {
        // compile
        PyObject *code = Py_CompileString(QString("from os import chdir \nchdir(u'" + QFileInfo(fileName).absolutePath() + "')").toStdString().c_str(), "", Py_file_input);
        // run
        if (code) output = PyEval_EvalCode((PyCodeObject *) code, m_dict, m_dict);
    }
    // compile
    PyObject *code = Py_CompileString(script.toStdString().c_str(), fileName.toStdString().c_str(), Py_file_input);
    // run
    if (code) output = PyEval_EvalCode((PyCodeObject *) code, m_dict, m_dict);

    ScriptResult scriptResult;
    if (output)
    {        
        scriptResult.isError = false;
        scriptResult.text = m_stdOut.trimmed();
    }
    else
    {
        scriptResult = parseError();
    }
    Py_DECREF(Py_None);

    m_isRunning = false;
    //TODO Util::scene()->refresh();
    //TODO sceneView()->doInvalidated();

    emit executed();

    return scriptResult;
}

ExpressionResult PythonEngine::runPythonExpression(const QString &expression, bool returnValue)
{
    runPythonHeader();

    QString exp;
    if (returnValue)
        exp = QString("result = %1").arg(expression);
    else
        exp = expression;

    PyObject *output = PyRun_String(exp.toLatin1().data(), Py_single_input, m_dict, m_dict);

    ExpressionResult expressionResult;
    if (output)
    {
        PyObject *type = NULL, *value = NULL, *traceback = NULL, *str = NULL;
        PyErr_Fetch(&type, &value, &traceback);

        if (type != NULL && (str = PyObject_Str(type)) != NULL && (PyString_Check(str)))
        {
            Py_INCREF(type);

            expressionResult.error = PyString_AsString(str);
            if (type) Py_DECREF(type);
            if (str) Py_DECREF(str);
        }
        else
        {
            // parse result
            if (returnValue)
            {
                PyObject *result = PyDict_GetItemString(m_dict, "result"); //FIX maybe m_dict_globals???
                if (result)
                {
                    Py_INCREF(result);
                    PyArg_Parse(result, "d", &expressionResult.value);
                    if (fabs(expressionResult.value) < EPS_ZERO)
                        expressionResult.value = 0.0;
                    Py_DECREF(result);
                }
            }
        }
    }
    else
    {
        expressionResult.error = parseError().text;
    }
    Py_DECREF(Py_None);

    emit executed();

    return expressionResult;
}

ScriptResult PythonEngine::parseError()
{    
    // error
    ScriptResult error;
    error.isError = true;

    PyObject *type = NULL, *value = NULL, *traceback = NULL, *str = NULL;
    PyErr_Fetch(&type, &value, &traceback);

    if (traceback)
    {
        PyTracebackObject *object = (PyTracebackObject *) traceback;
        error.text.append(QString("Line %1: ").arg(object->tb_lineno));
        error.line = object->tb_lineno;
        Py_DECREF(traceback);
    }

    if (type != NULL && (str = PyObject_Str(type)) != NULL && (PyString_Check(str)))
    {
        Py_INCREF(type);
        error.text.append("\n");
        error.text.append(PyString_AsString(str));
        if (type) Py_DECREF(type);
        if (str) Py_DECREF(str);
    }
    else
    {
        error.text.append("\n");
        error.text.append("<unknown exception type> ");
    }

    if (value != NULL && (str = PyObject_Str(value)) != NULL && (PyString_Check(str)))
    {
        Py_INCREF(value);
        error.text.append("\n");
        error.text.append(PyString_AsString(value));
        if (value) Py_DECREF(value);
        if (str) Py_DECREF(str);
    }
    else
    {
        error.text.append("\n");
        error.text.append("<unknown exception date> ");
    }

    PyErr_Clear();

    return error;
}

QList<Variables> PythonEngine::variableList()
{
    QStringList filter;
    filter << "__builtins__" << "StdoutCatcher" << "agrosstdout" << "capturestdout" << "chdir";

    QList<Variables> list;

    PyObject *keys = PyDict_Keys(m_dict);
    for (int i = 0; i < PyList_Size(keys); ++i)
    {
        PyObject *key = PyList_GetItem(keys, i);
        PyObject *value = PyDict_GetItem(m_dict, key);

        bool append = false;

        // variable
        Variables var;

        // variable name
        var.name = PyString_AsString(key);

        // variable type
        var.type = value->ob_type->tp_name;

        // variable value
        if (var.type == "bool")
        {
            var.value = PyInt_AsLong(value) ? "True" : "False";
            append = true;
        }
        if (var.type == "int")
        {
            var.value = (int) PyInt_AsLong(value);
            append = true;
        }
        if (var.type == "float")
        {
            var.value = PyFloat_AsDouble(value);
            append = true;
        }
        if (var.type == "str")
        {
            var.value = PyString_AsString(value);
            append = true;
        }
        if (var.type == "list")
        {
            var.value = QString("%1 items").arg(PyList_Size(value));
            append = true;
        }
        if (var.type == "tuple")
        {
            var.value = QString("%1 items").arg(PyTuple_Size(value));
            append = true;
        }
        if (var.type == "dict")
        {
            var.value = QString("%1 items").arg(PyDict_Size(value));
            append = true;
        }
        if (var.type == "numpy.ndarray")
        {
            var.value = ""; //TODO count
            append = true;
        }
        if (var.type == "function"
                || var.type == "module"
                || var.type == "instance"
                || var.type == "classobj")
        {
            append = true;
        }

        // qDebug() << var.type;

        if (append && !filter.contains(var.name))
            list.append(var);
    }
    Py_DECREF(keys);

    return list;
}
