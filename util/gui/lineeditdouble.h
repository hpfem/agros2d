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

#ifndef GUI_LINEEDIT_H
#define GUI_LINEEDIT_H

#include "../util.h"

#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets>
#endif

class AGROS_API LineEditDouble : public QLineEdit
{
    Q_OBJECT
public:
    LineEditDouble(double val = 0, bool validator = false, QWidget *parent = 0);
    ~LineEditDouble();

    inline void setBottom(double value) { if (m_validator) m_validator->setBottom(value); }
    inline void setTop(double value) { if (m_validator) m_validator->setTop(value); }
    inline double value() { return text().toDouble(); }
    inline void setValue(double value) { setText(QString::number(value)); }

private:
    QDoubleValidator *m_validator;
};

#endif // GUI_LINEEDIT_H
