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

#include <QApplication>

#include <QtGui>
#include <QtCore>

#include "qtsingleapplication.h"

#include "util.h"
#include "util/system_utils.h"
#include "pythonlab/pythonengine.h"
#include "pythonlab/pythoneditor.h"
#include "pythonlab.h"

int main(int argc, char *argv[])
{    
    CleanExit cleanExit;
    PythonLabApplication a(argc, argv);
    if (a.isRunning())
    {
        QString msg;
        msg = QCoreApplication::arguments().join("#!#");
        msg += "#!#";
        a.sendMessage(msg);

        return 0;
    }

    createPythonEngine(argc, argv);
    PythonEngine *pythonEngine = currentPythonEngine();

    PythonEditorDialog w(pythonEngine, QApplication::arguments());

    QObject::connect(&a, SIGNAL(messageReceived(const QString &)),
                     &w, SLOT(onOtherInstanceMessage(const QString &)));
    w.show();

    return a.exec();
}
