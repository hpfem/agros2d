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
#include "mainwindow.h"

#include "util.h"

QActionTool::QActionTool(const QString &text, const QString &fn, QObject *parent) : QAction(text, parent)
{
    setData(fn);
    connect(this, SIGNAL(triggered()), this, SLOT(openCustomTool()));
}

void QActionTool::openCustomTool()
{
    if (QFile::exists(data().toString()))
    {
        QPluginLoader *loader = new QPluginLoader(data().toString());

        if (!loader)
        {
            throw AgrosException(QObject::tr("Could not find '%1'").arg(data().toString()));
        }

        if (!loader->load())
        {
            qInfo() << loader->errorString();
            delete loader;
            throw AgrosException(QObject::tr("Could not load '%1'. %2").arg(data().toString()).arg(loader->errorString()));
        }

        assert(loader->instance());
        ToolInterface *form = qobject_cast<ToolInterface *>(loader->instance());
        delete loader;

        if (form)
        {
            // show form
            form->show();
            delete form;
        }
    }
}

// read forms
void readCustomTools(QMenu *menu)
{
#ifdef Q_WS_X11
    QDir dir(datadir() + "/libs/");
#endif
#ifdef Q_WS_WIN
    QDir dir(datadir() + "/");
#endif

    QStringList filter;
    filter << "*tool_*";
    QStringList list = dir.entryList(filter);

    if (list.isEmpty())
        menu->setVisible(false);

    foreach (QString filename, list)
    {
        QString fn = QString("%1/%2").arg(dir.absolutePath()).arg(filename);

        QPluginLoader *loader = new QPluginLoader(fn);

        if (!loader)
        {
            throw AgrosException(QObject::tr("Could not find '%1'").arg(fn));
        }

        if (!loader->load())
        {
            qInfo() << loader->errorString();
            delete loader;
            throw AgrosException(QObject::tr("Could not load '%1'. %2").arg(fn).arg(loader->errorString()));
        }

        assert(loader->instance());
        ToolInterface *form = qobject_cast<ToolInterface *>(loader->instance());
        delete loader;

        if (form)
        {
            menu->addAction(new QActionTool(form->formName(), fn));
            delete form;
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
            FormScript *form = new FormScript(fn, consoleView, parent);
            menu->addAction(form->action());
        }
    }
}
