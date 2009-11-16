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
    // harmonic magnetic
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
