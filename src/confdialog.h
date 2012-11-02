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

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include "util.h"

struct SceneViewSettings;
class ScriptEditor;

class LineEditDouble;
class ValueLineEdit;

class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    ConfigDialog(QWidget *parent);

private slots:
    void doCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void doAdaptivityDefault();
    void doCommandsDefault();

    void moduleClicked(QTreeWidgetItem *item, int role);
    void moduleDoubleClicked(QTreeWidgetItem *item, int role);
    void couplingClicked(QTreeWidgetItem *item, int role);
    void couplingDoubleClicked(QTreeWidgetItem *item, int role);
    void buildModule();
    void buildCoupling();

    void doAccept();
    void doReject();

private:
    QListWidget *lstView;
    QStackedWidget *pages;
    QWidget *panMain;
    QWidget *panSolver;
    QWidget *panPlugin;
    QWidget *panGlobalScriptWidget;

    // main
    QComboBox *cmbGUIStyle;
    QComboBox *cmbLanguage;
    QComboBox *cmbDefaultPhysicField;
    LineEditDouble *txtCollaborationServerURL;

    // delete files
    QCheckBox *chkDeleteTriangleMeshFiles;
    QCheckBox *chkDeleteHermes2DMeshFile;

    // show result in line edit value widget
    QCheckBox *chkLineEditValueShowResult;

    // save with solution
    QCheckBox *chkSaveWithSolution;

    // check version
    QCheckBox *chkCheckVersion;

    // log std out
    QCheckBox *chkLogStdOut;

    // geometry
    QSpinBox *txtMeshAngleSegmentsCount;
    QCheckBox *chkMeshCurvilinearElements;

    // discrete
    QLineEdit *txtDiscreteDirectory;
    QCheckBox *chkDiscreteSaveMatrixRHS;

    // scene font
    QLabel *lblSceneFontExample;
    QPushButton *btnSceneFont;

    // adaptivity
    QLabel *lblMaxDofs;
    QSpinBox *txtMaxDOFs;
    //QCheckBox *chkIsoOnly;
    //QLabel *lblIsoOnly;
    LineEditDouble *txtConvExp;
    QLabel *lblConvExp;
    LineEditDouble *txtThreshold;
    QLabel *lblThreshold;
    QComboBox *cmbStrategy;
    QLabel *lblStrategy;
    QComboBox *cmbMeshRegularity;
    QComboBox *cmbProjNormType;

    // threads
    QSpinBox *txtNumOfThreads;

    QLineEdit *txtArgumentTriangle;
    QLineEdit *txtArgumentGmsh;

    // global script
    ScriptEditor *txtGlobalScript;

    // modules and couplings
    QTreeWidget *treeModules;
    QTreeWidget *treeCouplings;
    QPushButton *btnBuildModule;
    QPushButton *btnBuildCoupling;

    void load();
    void save();

    void createControls();
    QWidget *createMainWidget();
    QWidget *createSolverWidget();
    QWidget *createPluginWidget();
    QWidget *createGlobalScriptWidget();

    void buildModuleOrCoupling(const QString &id);
    void readModulesAndCouplings();

    void fillComboBoxPhysicField(QComboBox *cmbPhysicField);
};


#endif // OPTIONSDIALOG_H
