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

#ifndef FORM_SCRIPT_H
#define FORM_SCRIPT_H

#include "util/form_interface.h"

#include "util.h"

namespace XMLForm
{
  class form;
}

class AGROS_API FormScript : public FormInterface
{
    Q_OBJECT
    Q_INTERFACES(FormInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.hpfem.agros2d.FormScript" FILE "")
#endif

public:
    FormScript(const QString &fileName = "", QWidget *parent = 0);
    virtual ~FormScript();

    virtual QString formId();
    virtual QAction *action();

public slots:
    virtual int show();
    virtual void acceptForm();
    virtual void rejectForm();

protected:
    QAction *actShow;
    QWidget *mainWidget;
    QLabel *errorMessage;
    QString fileName;

private:
    QString valueForWidget(XMLForm::form *doc, const QString &objectName, const QString &defaultValue);

private slots:
    void load();
    void save();
};

#endif // FORM_SCRIPT_H
