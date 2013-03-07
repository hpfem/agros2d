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

#ifndef GUI_COMMON_H
#define GUI_COMMON_H

#include "util.h"

class FieldInfo;

void readPixmap(QLabel *lblEquation, const QString &name);
QLabel *createLabel(const QString &label, const QString &toolTip = "");
void fillComboBoxFieldInfo(QComboBox *cmbFieldInfo);
void fillComboBoxContourVariable(FieldInfo *fieldInfo, QComboBox *cmbFieldVariable);
void fillComboBoxScalarVariable(FieldInfo *fieldInfo, QComboBox *cmbFieldVariable);
void fillComboBoxVectorVariable(FieldInfo *fieldInfo, QComboBox *cmbFieldVariable);
void fillComboBoxTimeStep(const FieldInfo *fieldInfo, QComboBox *cmbFieldVariable);
void fillComboBoxAdaptivityStep(FieldInfo* fieldInfo, int timeStep, QComboBox *cmbFieldVariable);
void fillComboBoxSolutionType(FieldInfo* fieldInfo, int timeStep, int adaptivityStep, QComboBox *cmbFieldVariable);
void addTreeWidgetItemValue(QTreeWidgetItem *parent, const QString &name, const QString &text, const QString &unit);
void fillComboBoxFonts(QComboBox *cmbFonts);
void fillComboBoxFontSizes(QComboBox *cmbFontSizes);

inline QString htmlFontFamily() { return QApplication::font().family(); }
#ifdef Q_WS_WIN
    inline int htmlFontSize() { return QApplication::font().pointSize() + 4; }
#endif
#ifdef Q_WS_X11
    inline int htmlFontSize() { return QApplication::font().pointSize() + 1; }
#endif
#ifdef Q_WS_MAC
    inline int htmlFontSize() { return QApplication::font().pointSize() + 1; }
#endif

QNetworkAccessManager *networkAccessManager();

int columnMinimumWidth();

#endif // GUI_COMMON_H
