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

#ifndef FORM_INTERFACE_H
#define FORM_INTERFACE_H

#include <QtPlugin>

#include "util.h"
#include "hermes2d.h"

// plugin interface
class AGROS_API FormInterface : public QDialog
{   
public:
    FormInterface(QWidget *parent = 0) : QDialog(parent) {}
    virtual ~FormInterface() {}

    virtual QString formId() = 0;
    virtual QAction *action() = 0;


public slots:
    virtual int show() = 0;
    virtual void acceptForm() = 0;
    virtual void rejectForm() = 0;

protected:
};

// read forms
void readCustomForms(QMenu *menu);
void readCustomScripts(QMenu *menu);

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(FormInterface, "agros2d.FormInterface/1.0")
QT_END_NAMESPACE

#endif // FORM_INTERFACE_H
