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

#include <QApplication>

#include <QtGui>
#include <QtCore>

#include "qtsingleapplication.h"

#include "util.h"
#include "pythonlab/pythonengine.h"
#include "pythonlab/pythoneditor.h"
#include "pythonlab.h"

int main(int argc, char *argv[])
{    
    PythonLabApplication a(argc, argv);
    if (a.isRunning())
    {
        QString msg;
        msg = QCoreApplication::arguments().join("#!#");
        msg += "#!#";
        a.sendMessage(msg);

        return 0;
    }

    createPythonEngine();
    PythonEngine *pythonEngine = currentPythonEngine();

    PythonEditorDialog w(pythonEngine, QApplication::arguments());

    QObject::connect(&a, SIGNAL(messageReceived(const QString &)),
                     &w, SLOT(onOtherInstanceMessage(const QString &)));
    w.show();

    return a.exec();
}
