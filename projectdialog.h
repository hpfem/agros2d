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
    ProjectDialog(ProjectInfo &projectInfo, bool newProject);
    ~ProjectDialog();

    int showDialog();

private slots:
    void doAccept();
    void doReject();

protected:
    bool m_newProject;
    ProjectInfo *m_projectInfo;

    void createControls();

    void load();
    void save();

private:
    QLineEdit *txtName;
    QComboBox *cmbProblemType;
    QComboBox *cmbPhysicField;
    QDateTimeEdit *dtmDate;
    QSpinBox *txtNumberOfRefinements;
    QSpinBox *txtPolynomialOrder;
    QSpinBox *txtAdaptivitySteps;
    SLineEdit *txtAdaptivityTolerance;

    void fillComboBox();
};

#endif // PROJECTDIALOG_H
