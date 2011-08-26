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

class ColorButton;
class SLineEditDouble;
class ValueLineEdit;

class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    ConfigDialog(QWidget *parent);

private slots:
    void doCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void doScalarFieldLog(int state);
    void doClearApplicationLog();
    void doClearCommandHistory();
    void doAdaptivityDefault();
    void doCommandsDefault();
    void doColorsDefault();

    void doAccept();
    void doReject();

private:
    QListWidget *lstView;
    QStackedWidget *pages;
    QWidget *panMain;
    QWidget *panView;
    QWidget *panSolver;
    QWidget *panColors;
    QWidget *panGlobalScriptWidget;

    // main
    QComboBox *cmbGUIStyle;
    QComboBox *cmbLanguage;
    QComboBox *cmbDefaultPhysicField;
    SLineEditDouble *txtCollaborationServerURL;

    // delete files
    QCheckBox *chkDeleteTriangleMeshFiles;
    QCheckBox *chkDeleteHermes2DMeshFile;

    // clear application log
    QPushButton *cmdClearApplicationLog;

    // clear command history
    QPushButton *cmdClearCommandHistory;

    // show result in line edit value widget
    QCheckBox *chkLineEditValueShowResult;

    // save with solution
    QCheckBox *chkSaveWithSolution;

    // check version
    QCheckBox *chkCheckVersion;

    // show convergence chart
    QCheckBox *chkShowConvergenceChart;

    // show nonlinear chart
    QCheckBox *chkShowNonlinearChart;

    // logs
    QCheckBox *chkEnabledApplicationLog;
    QCheckBox *chkEnabledProgressLog;

    // experimental features
    QCheckBox *chkExperimentalFeatures;

    // general view
    QCheckBox *chkZoomToMouse;

    // geometry
    QSpinBox *txtMeshAngleSegmentsCount;
    SLineEditDouble *txtGeometryNodeSize;
    SLineEditDouble *txtGeometryEdgeWidth;
    SLineEditDouble *txtGeometryLabelSize;
    QCheckBox *chkMeshCurvilinearElements;

    // scene font
    QLabel *lblSceneFontExample;
    QPushButton *btnSceneFont;

    // colors
    ColorButton *colorBackground;
    ColorButton *colorGrid;
    ColorButton *colorCross;
    ColorButton *colorNodes;
    ColorButton *colorEdges;
    ColorButton *colorLabels;
    ColorButton *colorContours;
    ColorButton *colorVectors;
    ColorButton *colorInitialMesh;
    ColorButton *colorSolutionMesh;
    ColorButton *colorHighlighted;
    ColorButton *colorSelected;

    // grid
    QLineEdit *txtGridStep;
    // show grid
    QCheckBox *chkShowGrid;
    // snap to grid
    QCheckBox *chkSnapToGrid;
    // rulers
    QCheckBox *chkRulers;

    // axes
    QCheckBox *chkShowAxes;

    // label
    QCheckBox *chkShowLabel;

    // scalar view
    QCheckBox *chkScalarFieldRangeLog;
    QLineEdit *txtScalarFieldRangeBase;
    QSpinBox *txtScalarDecimalPlace;

    // 3d
    QCheckBox *chkView3DLighting;
    QDoubleSpinBox *txtView3DAngle;
    QCheckBox *chkView3DBackground;
    QDoubleSpinBox *txtView3DHeight;

    // deform shape
    QCheckBox *chkDeformScalar;
    QCheckBox *chkDeformContour;
    QCheckBox *chkDeformVector;

    // adaptivity
    QLabel *lblMaxDofs;
    QSpinBox *txtMaxDOFs;
    //QCheckBox *chkIsoOnly;
    //QLabel *lblIsoOnly;
    SLineEditDouble *txtConvExp;
    QLabel *lblConvExp;
    SLineEditDouble *txtThreshold;
    QLabel *lblThreshold;
    QComboBox *cmbStrategy;
    QLabel *lblStrategy;
    QComboBox *cmbMeshRegularity;
    QComboBox *cmbProjNormType;

    QLineEdit *txtArgumentTriangle;
    QLineEdit *txtArgumentFFmpeg;

    // global script
    ScriptEditor *txtGlobalScript;

    void load();
    void save();

    void createControls();
    QWidget *createMainWidget();
    QWidget *createViewWidget();
    QWidget *createSolverWidget();
    QWidget *createColorsWidget();
    QWidget *createGlobalScriptWidget();
};

// *************************************************************************

class ColorButton : public QPushButton
{
    Q_OBJECT

public:
    ColorButton(QWidget *parent = 0);
    ~ColorButton();

    inline QColor color() { return m_color; }
    void setColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void doClicked();

private:
    QColor m_color;
};

#endif // OPTIONSDIALOG_H
