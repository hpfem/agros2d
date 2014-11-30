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

#include "pythonlab_functions.h"

char *pyVersion()
{
    return const_cast<char*>(QApplication::applicationVersion().toStdString().c_str());
}

void pyQuit()
{
    // doesn't work without main event loop (run from script)
    // QApplication::exit(0);

    exit(0);
}

char *pyInput(std::string str)
{
    QString text = QInputDialog::getText(QApplication::activeWindow(), QObject::tr("Script input"), QString::fromStdString(str));
    return const_cast<char*>(text.toStdString().c_str());
}

void pyMessage(std::string str)
{
    QMessageBox::information(QApplication::activeWindow(), QObject::tr("Script message"), QString::fromStdString(str));
}

std::string pyDatadir(std::string str)
{
    QString path = QFileInfo(datadir() + "/" + QString::fromStdString(str)).absoluteFilePath();
    return compatibleFilename(path).toStdString();
}
