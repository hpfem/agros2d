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

#include "util.h"

class ProblemInfo;
class FieldInfo;
class ScriptEditor;

class SLineEditDouble;
class ValueLineEdit;

class ProblemDialog: public QDialog
{
    Q_OBJECT
public:
    ProblemDialog(ProblemInfo *problemInfo, bool isNewProblem, QWidget *parent = 0);

    int showDialog();

private slots:
    void doPhysicFieldChanged(int index);
    void doAdaptivityChanged(int index);
    void doAnalysisTypeChanged(int index);
    void doLinearityTypeChanged(int index);
    void doTransientChanged();
    void doShowEquation();
    void doAccept();
    void doReject();
    void doOpenXML();

private:
    bool m_isNewProblem;
    ProblemInfo *m_problemInfo;

    QLineEdit *txtName;
    QComboBox *cmbProblemType;
    QComboBox *cmbPhysicField;
    QDateTimeEdit *dtmDate;
    QComboBox *cmbAdaptivityType;
    QSpinBox *txtAdaptivitySteps;
    SLineEditDouble *txtAdaptivityTolerance;
    QComboBox *cmbLinearityType;
    QComboBox *cmbMatrixSolver;

    // mesh
    QComboBox *cmbMeshType;
    QSpinBox *txtNumberOfRefinements;
    QSpinBox *txtPolynomialOrder;

    // linearity
    QSpinBox *txtNonlinearSteps;
    SLineEditDouble *txtNonlinearTolerance;

    // harmonic
    SLineEditDouble *txtFrequency;

    // transient
    QComboBox *cmbAnalysisType;
    ValueLineEdit *txtTransientTimeStep;
    ValueLineEdit *txtTransientTimeTotal;
    ValueLineEdit *txtTransientInitialCondition;
    QLabel *lblTransientSteps;

    // startup script
    ScriptEditor *txtStartupScript;

    // description
    QTextEdit *txtDescription;

    // equation
    QLabel *lblEquationPixmap;

    // weak forms
    QComboBox *cmbWeakForms;

    QTabWidget *tabType;

    void createControls();
    QWidget *createControlsGeneral();
    QWidget *createControlsStartupScript();
    // QWidget *createControlsCustomForms();
    QWidget *createControlsDescription();

    void fillComboBox();

    void load();
    bool save();
};

#endif // PROBLEMDIALOG_H
