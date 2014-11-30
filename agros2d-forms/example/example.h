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

#ifndef FORM_EXAMPLE_H
#define FORM_EXAMPLE_H

#include "util/form_interface.h"

#include "util.h"

class AGROS_API FormExample : public FormInterface
{
    Q_OBJECT
    Q_INTERFACES(FormInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.agros.FormExample" FILE "")
#endif

public:
    FormExample(QWidget *parent = 0);
    virtual ~FormExample();

    virtual QString formId() { return "example"; }
    virtual QAction *action();

public slots:
    virtual int show();
    virtual void acceptForm();
    virtual void rejectForm();

protected:
    QAction *actShow;
    QWidget *mainWidget;
};

#endif // FORM_EXAMPLE_H
