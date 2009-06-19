#ifndef PROJECTDIALOG_H
#define PROJECTDIALOG_H

#include <QtGui/QDialog>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QSpinBox>

#include "gui.h"
#include "scene.h"

struct ProjectInfo;

class ProjectDialog: public QDialog
{
    Q_OBJECT
public:
    ProjectDialog(ProjectInfo &projectInfo, bool isNewProject, QWidget *parent = 0);
    ~ProjectDialog();

    int showDialog();

private slots:
    void doAccept();
    void doReject();

private:
    bool m_isNewProject;
    ProjectInfo *m_projectInfo;

    QLineEdit *txtName;
    QComboBox *cmbProblemType;
    QComboBox *cmbPhysicField;
    QDateTimeEdit *dtmDate;
    QSpinBox *txtNumberOfRefinements;
    QSpinBox *txtPolynomialOrder;
    QSpinBox *txtAdaptivitySteps;
    SLineEdit *txtAdaptivityTolerance;

    void createControls();
    void fillComboBox();

    void load();
    void save();
};

#endif // PROJECTDIALOG_H
