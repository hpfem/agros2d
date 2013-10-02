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

class ProblemConfig;
class FieldInfo;
class CouplingInfo;

class LineEditDouble;
class ValueLineEdit;
class LaTeXViewer;

class FieldInfo;

class ProblemWidget;

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
    CouplingsWidget(QWidget *parent);

    void createContent();

signals:
    void changed();

public slots:
    void save();
    void refresh();
    void itemChanged(int index);

private:
    void createComboBoxes();

    QList<QComboBox *> m_comboBoxes;
    QList<QLabel *> m_labels;
};

class FieldWidget : public QWidget
{
    Q_OBJECT
public:
    FieldWidget(FieldInfo *m_fieldInfo, QWidget *parent);

    void createContent();
    QWidget *createSolverWidget();
    QWidget *createAdaptivityWidget();
    QWidget *createTransientAnalysisWidget();
    QWidget *createLinearSolverWidget();

    void load();
    bool save();

    FieldInfo *fieldInfo();
    void refresh();

private:
    FieldInfo *m_fieldInfo;

    // main
    QComboBox *cmbAnalysisType;

    // adaptivity
    QComboBox *cmbAdaptivityType;
    QSpinBox *txtAdaptivitySteps;
    LineEditDouble *txtAdaptivityTolerance;
    LineEditDouble *txtAdaptivityThreshold;
    QComboBox *cmbAdaptivityErrorCalculator;
    QComboBox *cmbAdaptivityStoppingCriterionType;
    QCheckBox *chkAdaptivityUseAniso;
    QCheckBox *chkAdaptivityFinerReference;
    QSpinBox *txtAdaptivityOrderIncrease;
    QCheckBox *chkAdaptivitySpaceRefinement;
    QSpinBox *txtAdaptivityBackSteps;
    QSpinBox *txtAdaptivityRedoneEach;

    QComboBox *cmbLinearityType;
    QComboBox *cmbLinearSolver;

    // mesh
    QSpinBox *txtNumberOfRefinements;
    QSpinBox *txtPolynomialOrder;

    // linearity
    LineEditDouble *txtNonlinearResidual;
    LineEditDouble *txtNonlinearRelativeChangeOfSolutions;
    QComboBox *cmbNonlinearDampingType;
    QCheckBox *chkNewtonReuseJacobian;
    LineEditDouble *txtNonlinearDampingCoeff;
    LineEditDouble *txtNewtonSufficientImprovementFactorForJacobianReuse;
    LineEditDouble *txtNewtonSufficientImprovementFactor;
    QSpinBox *txtNewtonMaximumStepsWithReusedJacobian;
    QSpinBox *txtNonlinearDampingNumberToIncrease;
    QCheckBox *chkPicardAndersonAcceleration;
    LineEditDouble *txtPicardAndersonBeta;
    QSpinBox *txtPicardAndersonNumberOfLastVectors;

    // transient
    LineEditDouble *txtTransientInitialCondition;
    LineEditDouble *txtTransientTimeSkip;

    // linear solver
    QComboBox *cmbIterLinearSolverMethod;
    QComboBox *cmbIterLinearSolverPreconditioner;
    LineEditDouble *txtIterLinearSolverToleranceAbsolute;
    QSpinBox *txtIterLinearSolverIters;

    // equation
    // LaTeXViewer *equationLaTeX;
    QLabel *equationImage;

    void fillComboBox();

private slots:
    void doAnalysisTypeChanged(int index);
    void doAdaptivityChanged(int index);
    void doLinearityTypeChanged(int index);
    void doLinearSolverChanged(int index);

    void doNonlinearDampingChanged(int index);
    void doNewtonReuseJacobian(bool checked);
    void doPicardAndersonChanged(int index);
    void doShowEquation();
};

class FieldDialog : public QDialog
{
    Q_OBJECT
public:
    FieldDialog(FieldInfo *fieldInfo, QWidget *parent = 0);
    ~FieldDialog();

private:
    FieldWidget *fieldWidget;

private slots:
    void doAccept();
    void deleteField();
    void moduleEditor();
};

class FieldsToobar: public QWidget
{
    Q_OBJECT

public:
    FieldsToobar(QWidget *parent = 0);

signals:
    void changed();

public slots:
    void refresh();

private:
    QList<QToolButton *> m_buttons;
    QList<QLabel *> m_labels;

    void createControls();

private slots:
    void fieldDialog(int index);
    void addField();
};

class ProblemWidget: public QWidget
{
    Q_OBJECT
public:
    ProblemWidget(QWidget *parent = 0);

    QAction *actProperties;

signals:
    void changed();

public slots:
    void updateControls();

private:
    QDialogButtonBox *buttonBox;

    FieldsToobar *fieldsToolbar;
    CouplingsWidget *couplingsWidget;

    QComboBox *cmbCoordinateType;
    QComboBox *cmbMeshType;

    // harmonic
    QGroupBox *grpHarmonicAnalysis;
    LineEditDouble *txtFrequency;

    // transient
    QGroupBox *grpTransientAnalysis;
    LineEditDouble *txtTransientTimeTotal;
    QLabel* lblTransientSteps;
    QSpinBox *txtTransientSteps;
    LineEditDouble *txtTransientTolerance;
    QLabel *lblTransientTimeTotal;
    QSpinBox *txtTransientOrder;
    QComboBox *cmbTransientMethod;
    QLabel *lblTransientTimeStep;

    // couplings
    QGroupBox *grpCouplings;

    void createActions();
    void createControls();

    void fillComboBox();

private slots:
    void transientChanged();

    void changedWithClear();
};

#endif // PROBLEMDIALOG_H
