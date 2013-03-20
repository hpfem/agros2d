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

#ifndef GUI_HTMLEDIT_H
#define GUI_HTMLEDIT_H

#include "util.h"

class HtmlValueEdit : public QWidget
{
    Q_OBJECT

public:
    HtmlValueEdit(QWidget *parent = NULL, const QString &str = "");

    inline void setText(const QString &str) { txtLineEdit->setText(str); evaluate(); }
    inline QString text() { return txtLineEdit->text(); }

private slots:
    bool evaluate();

private:
    QLineEdit *txtLineEdit;
    QLabel *lblValue;
};

#endif // GUI_HTMLEDIT_H
