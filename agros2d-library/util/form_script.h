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

#ifndef FORM_SCRIPT_H
#define FORM_SCRIPT_H

#include "util/form_interface.h"

#include "util.h"

namespace XMLForm
{
  class form;
}

class PythonScriptingConsoleView;

class AGROS_LIBRARY_API FormScript : public FormInterface
{
    Q_OBJECT
    Q_INTERFACES(FormInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.agros.FormScript" FILE "")
#endif

public:
    FormScript(const QString &m_fileName = "", PythonScriptingConsoleView *consoleView = NULL, QWidget *parent = 0);
    virtual ~FormScript();

    virtual QString formId();
    virtual QAction *action();
    inline QString fileName() { return m_fileName; }

public slots:
    virtual int show();
    int showForm(const QString &m_fileName = "");
    virtual void acceptForm();
    virtual void rejectForm();

    void loadFromFile(const QString &m_fileName = "");
    void saveToFile(const QString &m_fileName = "");

protected:
    QAction *actShow;
    QWidget *mainWidget;
    QMenu *menu;
    QPushButton *btnMore;
    QLabel *errorMessage;
    QString m_fileName;

    QProcess *process;

    PythonScriptingConsoleView *consoleView;

private:
    QString valueForWidget(XMLForm::form *doc, const QString &objectName, const QString &defaultValue);

    void loadWidget(const QString &m_fileName);

private slots:
    void reloadWidget();
    void showWidget();
    void designer();
    void designerFinished(int status);
};

#endif // FORM_SCRIPT_H
