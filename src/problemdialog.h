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

#ifndef PROBLEMDIALOG_H
#define PROBLEMDIALOG_H

#include "gui.h"
#include "scene.h"
#include "scripteditordialog.h"

struct ProblemInfo;
class ScriptEditor;

class ProblemDialog: public QDialog
{
    Q_OBJECT
public:
    ProblemDialog(ProblemInfo *problemInfo, bool isNewProblem, QWidget *parent = 0);
    ~ProblemDialog();

    int showDialog();

private slots:
    void doPhysicFieldChanged(int index);
    void doAdaptivityChanged(int index);
    void doAnalysisTypeChanged(int index);
    void doTransientChanged();
    void doAccept();
    void doReject();

private:
    bool m_isNewProblem;
    ProblemInfo *m_problemInfo;

    QLineEdit *txtName;
    QComboBox *cmbProblemType;
    QComboBox *cmbPhysicField;
    QDateTimeEdit *dtmDate;
    QSpinBox *txtNumberOfRefinements;
    QSpinBox *txtPolynomialOrder;
    QComboBox *cmbAdaptivityType;
    QSpinBox *txtAdaptivitySteps;
    SLineEditDouble *txtAdaptivityTolerance;

    // harmonic
    SLineEditDouble *txtFrequency;

    // transient
    QComboBox *cmbAnalysisType;
    SLineEditDouble *txtTransientTimeStep;
    SLineEditDouble *txtTransientTimeTotal;
    SLineEditDouble *txtTransientInitialCondition;
    QLabel *lblTransientSteps;

    // startup script
    ScriptEditor *txtStartupScript;

    // description
    QTextEdit *txtDescription;

    QTabWidget *tabType;

    void createControls();
    QWidget *createControlsGeneral();
    QWidget *createControlsStartupScript();
    QWidget *createControlsDescription();

    void fillComboBox();

    void load();
    bool save();
};

#endif // PROBLEMDIALOG_H
