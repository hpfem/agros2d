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
    return pythonEngine;
}

// ****************************************************************************

// print stdout
PyObject* pythonCaptureStdout(PyObject* self, PyObject* pArgs)
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
    if (PyArg_ParseTuple(pArgs, "s", &str))
    {
        emit currentPythonEngine()->pythonShowImageCommand(QString(str));
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

static PyMethodDef pythonEngineFuntions[] =
{
    {"stdout", pythonCaptureStdout, METH_VARARGS, "stdout"},
    {"image", pythonShowFigure, METH_VARARGS, "image(file)"},
    {"clear", pythonClear, METH_NOARGS, "clear()"},
    {"html", pythonInsertHtml, METH_VARARGS, "html(str)"},
    {NULL, NULL, 0, NULL}
};


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
    Py_Initialize();

    // read functions
    m_functions = readFileContent(datadir() + "/functions.py");

    m_dict = PyDict_New();
    PyDict_SetItemString(m_dict, "__builtins__", PyEval_GetBuiltins());

    // init engine extensions
    Py_InitModule("pythonlab", pythonEngineFuntions);

    addCustomExtensions();

    // stdout
    // PyRun_String(QString("agrosstdout = \"" + tempProblemDir() + "/stdout.txt" + "\"").toStdString().c_str(), Py_file_input, m_dict, m_dict);

    // custom modules
    PyRun_String(QString("import sys; sys.path.insert(0, \"" + datadir() + "/resources/python" + "\")").toStdString().c_str(), Py_file_input, m_dict, m_dict);

    // functions.py
    PyRun_String(m_functions.toStdString().c_str(), Py_file_input, m_dict, m_dict);
}

void PythonEngine::pythonShowMessageCommand(const QString &message)
{
    if (message != "\n\n")
        emit pythonShowMessage(message);
}

void PythonEngine::pythonShowImageCommand(const QString &fileName)
{
    emit pythonShowImage(fileName);
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
    Py_XDECREF(output);
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
        exp = QString("result_pythonlab = %1").arg(expression);
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
                    Py_INCREF(result);
                    PyArg_Parse(result, "d", &expressionResult.value);
                    if (fabs(expressionResult.value) < EPS_ZERO)
                        expressionResult.value = 0.0;
                    Py_DECREF(result);
                }
            }
        }

        if (returnValue)
            PyRun_String("del result_pythonlab", Py_single_input, m_dict, m_dict);
    }
    else
    {
        expressionResult.error = parseError().text;
    }
    Py_XDECREF(output);
    Py_DECREF(Py_None);

    emit executed();

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
                arg(fileName).
                arg(offset);
    }
    else
    {
        exp = QString("result_rope_pythonlab = python_engine_get_completion_string(\"%1\", %2)").
                arg(code).
                arg(offset);
    }

    // QTime time;
    // time.start();
    PyRun_String(exp.toLatin1().data(), Py_single_input, m_dict, m_dict);
    // qDebug() << time.elapsed();

    // parse result
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
                out.append(str);
            }
        }
        Py_DECREF(result);
    }

    PyRun_String("del result_rope_pythonlab", Py_single_input, m_dict, m_dict);

    Py_DECREF(Py_None);

    return out;
}

QStringList PythonEngine::codePyFlakes(const QString& fileName)
{
    QStringList out;

    QString exp = QString("result_pyflakes_pythonlab = python_engine_pyflakes_check(\"%1\")").arg(fileName);

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

    Py_DECREF(Py_None);

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

QList<PythonVariables> PythonEngine::variableList()
{
    QStringList filter;
    filter << "__builtins__" << "StdoutCatcher" << "agrosstdout" << "capturestdout" << "chdir"
           << "python_engine_get_completion_file" << "python_engine_get_completion_string"
           << "python_engine_pyflakes_check";

    QList<PythonVariables> list;

    PyObject *keys = PyDict_Keys(m_dict);
    for (int i = 0; i < PyList_Size(keys); ++i)
    {
        PyObject *key = PyList_GetItem(keys, i);
        PyObject *value = PyDict_GetItem(m_dict, key);

        bool append = false;

        // variable
        PythonVariables var;

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
        if (var.type == "module")
        {
            var.value = PyString_AsString(PyObject_GetAttrString(value, "__name__"));
            append = true;
        }
        if (var.type == "function"
                || var.type == "instance"
                || var.type == "classobj")
        {
            append = true;
        }

        // qDebug() << var.name << " : " << var.type;

        if (append && !filter.contains(var.name))
            list.append(var);
    }
    Py_DECREF(keys);

    return list;
}
