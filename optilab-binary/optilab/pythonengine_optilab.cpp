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

#ifdef _MSC_VER
# ifdef _DEBUG
#  undef _DEBUG
#  include <Python.h>
#  define _DEBUG
# else
#  include <Python.h>
# endif
#else
#  include <Python.h>
#endif

#include "pythonengine_optilab.h"
#include "util/memory_monitor.h"

// current python engine agros
AGROS_LIBRARY_API PythonEngineOptilab *currentPythonEngineOptilab()
{
    return static_cast<PythonEngineOptilab *>(currentPythonEngine());
}

void PythonEngineOptilab::addCustomExtensions()
{
    PythonEngine::addCustomExtensions();

    // init agros cython extensions
    // initagros2d();
}

void PythonEngineOptilab::addCustomFunctions()
{
    // addFunctions(readFileContent(datadir() + "/resources/python/functions_optilab.py"));
}

void PythonEngineOptilab::runPythonHeader()
{

}

PythonEditorOptilabDialog::PythonEditorOptilabDialog(PythonEngine *pythonEngine, QStringList args, QWidget *parent)
    : PythonEditorDialog(pythonEngine, args, parent)
{
    QSettings settings;

    // mnuTools->addSeparator();
    // mnuTools->addAction(actCreateFromModel);

    // tlbTools->addSeparator();
    // tlbTools->addAction(actCreateFromModel);

    // console output
    // actConsoleOutput = new QAction(tr("Console output"), this);
    // actConsoleOutput->setCheckable(true);
    // actConsoleOutput->setChecked(settings.value("PythonEditorDialog/ConsoleOutput", true).toBool());

    // mnuOptions->addAction(actConsoleOutput);
}

PythonEditorOptilabDialog::~PythonEditorOptilabDialog()
{
    QSettings settings;
    // settings.setValue("PythonEditorDialog/ConsoleOutput", actConsoleOutput->isChecked());
}

