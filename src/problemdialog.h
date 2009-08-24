#ifndef PROBLEMDIALOG_H
#define PROBLEMDIALOG_H

#include "gui.h"
#include "scene.h"

struct ProblemInfo;

class ProblemDialog: public QDialog
{
    Q_OBJECT
public:
    ProblemDialog(ProblemInfo &problemInfo, bool isNewProblem, QWidget *parent = 0);
    ~ProblemDialog();

    int showDialog();

private slots:
    void doPhysicFieldChanged(int index);
    void doAdaptivityChanged(int index);
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
    SLineEdit *txtAdaptivityTolerance;
    SLineEdit *txtFrequency;

    void createControls();
    void fillComboBox();

    void load();
    void save();
};

#endif // PROBLEMDIALOG_H
