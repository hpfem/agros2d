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
class CouplingInfo;
class ScriptEditor;

class SLineEditDouble;
class ValueLineEdit;

class FieldInfo;

class ProblemDialog;

class FieldSelectDialog : public QDialog
{
    Q_OBJECT
public:
    FieldSelectDialog(QList<QString> fields, QWidget *parent);

    int showDialog();
    inline QString selectedFieldId() { return m_selectedFieldId; }

private slots:
    void doAccept();
    void doReject();

    void doItemSelected(QListWidgetItem *item);
    void doItemDoubleClicked(QListWidgetItem *item);

private:
    QListWidget *lstFields;
    QString m_selectedFieldId;

    QDialogButtonBox *buttonBox;
};

class CouplingsWidget : public QWidget
{
    Q_OBJECT
public:
    CouplingsWidget(bool isNewProblem, QWidget *parent);

    void createContent();

    void load();
    void save();
    inline int count() { return m_couplingInfos.count(); }

public slots:
    void refresh();

private:
    void createComboBoxes();
    void fillComboBox();

    QGridLayout *layoutTable;

    QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* > m_couplingInfos;
    QMap<CouplingInfo*, QComboBox*> m_comboBoxes;

    bool m_isNewProblem;
};

class FieldWidget : public QWidget
{
    Q_OBJECT
public:
    FieldWidget(const ProblemInfo *problemInfo, FieldInfo *m_fieldInfo, QWidget *parent);

    // equation
    QLabel *equationImage;

    void createContent();

    void load();
    bool save();

    FieldInfo *fieldInfo();
    void refresh();

private:
    // problem info
    const ProblemInfo *problemInfo;
    FieldInfo *m_fieldInfo;

    // main
    QComboBox *cmbAnalysisType;

    QComboBox *cmbAdaptivityType;
    QSpinBox *txtAdaptivitySteps;
    SLineEditDouble *txtAdaptivityTolerance;

    QComboBox *cmbLinearityType;

    // mesh
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

class FieldDialog : public QDialog
{
    Q_OBJECT
public:
    FieldDialog(FieldInfo * fieldInfo, QWidget *parent = 0);
    ~FieldDialog();

private:
    FieldWidget *fieldWidget;

private slots:
    void doAccept();
    void deleteField();
};

class FieldsToobar: public QWidget
{
    Q_OBJECT
public:
    FieldsToobar(QWidget *parent = 0, Qt::Orientation oriantation = Qt::Vertical);

public slots:
    void refresh();

private:
    QActionGroup *actFieldsGroup;
    QToolBar *tlbFields;

    void createControls(Qt::Orientation oriantation);

private slots:
    void fieldDialog(QAction *action);
    void addField();
};

class ProblemDialog: public QDialog
{
    Q_OBJECT
public:
    ProblemDialog(ProblemInfo *problemInfo, bool isNewProblem, QWidget *parent = 0);

    int showDialog();
    ProblemInfo* problemInfo() {return m_problemInfo;}

private slots:
    void doTransientChanged();
    void doAccept();
    void doReject();
    void doOpenXML();

private:
    bool m_isNewProblem;
    ProblemInfo *m_problemInfo;

    QDialogButtonBox *buttonBox;

    CouplingsWidget *couplingsWidget;

    QLineEdit *txtName;
    QComboBox *cmbCoordinateType;
    QComboBox *cmbMeshType;
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

    // couplings
    QGroupBox *grpCouplings;

    void createControls();
    QWidget *createControlsGeneral();
    QWidget *createControlsStartupScript();    
    QWidget *createControlsDescription();

    void fillComboBox();

    void load();
    bool save();
};

#endif // PROBLEMDIALOG_H
