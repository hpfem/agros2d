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

#ifndef GUI_PHYSICALFIELD_H
#define GUI_PHYSICALFIELD_H

#include "util.h"
#include "hermes2d/plugin_interface.h"

class PhysicalFieldWidget : public QWidget
{
    Q_OBJECT

signals:
    void fieldChanged();

public slots:
    void updateControls();

public:
    PhysicalFieldWidget(QWidget *parent = 0);
    ~PhysicalFieldWidget();

    FieldInfo* selectedField();
    int selectedTimeStep();
    int selectedAdaptivityStep();
    SolutionMode selectedAdaptivitySolutionType();

private:
    QComboBox *cmbFieldInfo;

    // transient
    QGroupBox *grpTime;
    QLabel *lblTimeStep;
    QComboBox *cmbTimeStep;

    // adaptivity
    QGroupBox *grpAdaptivity;
    QLabel *lblAdaptivityStep;
    QComboBox *cmbAdaptivityStep;
    QLabel *lblAdaptivitySolutionType;
    QComboBox *cmbAdaptivitySolutionType;

    QString m_currentFieldName;

private slots:
    void doFieldInfo(int index);
    void doTimeStep(int index);
    void doAdaptivityStep(int index);
};

#endif // GUI_PHYSICALFIELD_H
