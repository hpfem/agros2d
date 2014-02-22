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

#ifndef PYTHONENGINEAGROS_H
#define PYTHONENGINEAGROS_H

#include "pythonlab/pythonconsole.h"
#include "pythonlab/pythonengine.h"
#include "pythonlab/pythoneditor.h"

#include "util.h"

class AGROS_LIBRARY_API PythonEngineOptilab : public PythonEngine
{
    Q_OBJECT
public:
    PythonEngineOptilab() : PythonEngine() {}

protected:
    virtual void addCustomExtensions();
    virtual void addCustomFunctions();
    virtual void runPythonHeader();

private:
};

class AGROS_LIBRARY_API PythonEditorOptilabDialog : public PythonEditorDialog
{
    Q_OBJECT
public:
    PythonEditorOptilabDialog(PythonEngine *pythonEngine, QStringList args, QWidget *parent);
    ~PythonEditorOptilabDialog();

private:
};

// current python engine agros
AGROS_LIBRARY_API PythonEngineOptilab *currentPythonEngineOptilab();

#endif // PYTHONENGINEAGROS_H
