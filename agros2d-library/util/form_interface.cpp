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

#include "util/form_interface.h"
#include "util/form_script.h"

#include "util.h"

// read forms
void readCustomForms(QMenu *menu)
{
    QDir dir(datadir() + "/libs/");

    QStringList filter;
    filter << "*agros2d_forms_*";
    QStringList list = dir.entryList(filter);

    foreach (QString filename, list)
    {
        QString fn = QString("%1/%2").arg(dir.absolutePath()).arg(filename);
        if (QFile::exists(fn))
        {
            QPluginLoader *loader = new QPluginLoader(fn);

            if (!loader)
            {
                throw AgrosException(QObject::tr("Could not find '%1'").arg(fn));
            }


            if (!loader->load())
            {
                qDebug() << loader->errorString();
                delete loader;
                throw AgrosException(QObject::tr("Could not load '%1'. %2").arg(fn).arg(loader->errorString()));
            }

            assert(loader->instance());
            FormInterface *form = qobject_cast<FormInterface *>(loader->instance());
            delete loader;

            if (form)
            {
                qDebug() << QString("Form '%1' loaded.").arg(form->formId());
                menu->addAction(form->action());
            }
        }
    }
}

void readCustomScripts(QMenu *menu, PythonScriptingConsoleView *consoleView, QWidget *parent)
{
    QDir dir(datadir() + "/resources/forms");

    QStringList filter;
    filter << "*.ui";
    QStringList list = dir.entryList(filter);

    foreach (QString filename, list)
    {
        QString fn = QString("%1/%2").arg(dir.absolutePath()).arg(filename);
        if (QFile::exists(fn))
        {
            // QSharedPointer<FormScript> form = QSharedPointer<FormScript>(new FormScript(fn));
            // menu->addAction(form.data()->action());
            FormScript *form = new FormScript(fn, consoleView, parent);
            menu->addAction(form->action());
        }
    }
}
