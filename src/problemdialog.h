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

class FieldInfo;

class FieldSelectDialog : public QDialog
{
    Q_OBJECT
public:
    FieldSelectDialog(QWidget *parent);

    int showDialog();
    inline QString fieldId() { return m_fieldId; }

private slots:
    void doAccept();
    void doReject();

    void doItemSelected(QListWidgetItem *item);
    void doItemDoubleClicked(QListWidgetItem *item);

private:
    QListWidget *lstFields;
    QString m_fieldId;

    QDialogButtonBox *buttonBox;
};


class FieldWidget : public QWidget
{
    Q_OBJECT
public:
    FieldWidget(const ProblemInfo *problemInfo, FieldInfo *fieldInfo, QWidget *parent);

    // equation
    QLabel *equationImage;

    void createContent();

    void load();
    bool save();

    void refresh();

private:
    QString fieldId;

    // problem info
    const ProblemInfo *problemInfo;
    FieldInfo *fieldInfo;

    // main
    QComboBox *cmbAnalysisType;

    QComboBox *cmbAdaptivityType;
    QSpinBox *txtAdaptivitySteps;
    SLineEditDouble *txtAdaptivityTolerance;

    QComboBox *cmbLinearityType;

    // mesh
    QComboBox *cmbMeshType;
    QSpinBox *txtNumberOfRefinements;
    QSpinBox *txtPolynomialOrder;

    // linearity
    QSpinBox *txtNonlinearSteps;
    SLineEditDouble *txtNonlinearTolerance;

    // transient
    ValueLineEdit *txtTransientInitialCondition;

    // weak forms
    QComboBox *cmbWeakForms;

    // equation
    QLabel *lblEquationPixmap;

    void fillComboBox();

private slots:
    void doAnalysisTypeChanged(int index);
    void doAdaptivityChanged(int index);
    void doLinearityTypeChanged(int index);
    void doShowEquation();
};

class ProblemDialog: public QDialog
{
    Q_OBJECT
public:
    ProblemDialog(ProblemInfo *problemInfo,
                  QMap<QString, FieldInfo *> fieldInfos,
                  bool isNewProblem, QWidget *parent = 0);

    int showDialog();

private slots:
    void doPhysicFieldChanged(int index);
    void doTransientChanged();
    void doAccept();
    void doReject();
    void doOpenXML();
    void doAddField();

private:
    bool m_isNewProblem;
    ProblemInfo *m_problemInfo;
    QMap<QString, FieldInfo *>  m_fieldInfos;

    QTabWidget *tabFields;

    QLineEdit *txtName;
    QComboBox *cmbCoordinateType;
    QDateTimeEdit *dtmDate;
    QComboBox *cmbMatrixSolver;

    // harmonic
    SLineEditDouble *txtFrequency;

    // transient
    ValueLineEdit *txtTransientTimeStep;
    ValueLineEdit *txtTransientTimeTotal;
    QLabel *lblTransientSteps;

    // startup script
    ScriptEditor *txtStartupScript;

    // description
    QTextEdit *txtDescription;

    void createControls();
    QWidget *createControlsGeneral();
    QWidget *createControlsStartupScript();    
    QWidget *createControlsDescription();

    void fillComboBox();
    void refresh();

    void load();
    bool save();
};

#endif // PROBLEMDIALOG_H
