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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "util.h"

class ColorButton;
class SceneViewPreprocessor;
class SceneViewMesh;
class SceneViewPost2D;
class SceneViewPost3D;
class LineEditDouble;
class ScriptEditor;

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    SettingsWidget(QWidget *parent);

    QAction *actSettings;

private slots:
    void doApply();

private:
    // grid
    QLineEdit *txtGridStep;
    QCheckBox *chkShowGrid;
    QCheckBox *chkSnapToGrid;

    // general view
    QCheckBox *chkZoomToMouse;
    QSpinBox *txtGeometryNodeSize;
    QSpinBox *txtGeometryEdgeWidth;
    QSpinBox *txtGeometryLabelSize;

    // scene font
    QComboBox *cmbRulersFont;
    QComboBox *cmbPostFont;

    // workspace other
    QCheckBox *chkShowAxes;
    QCheckBox *chkShowRulers;

    // advanced
    QCheckBox *chkView3DLighting;
    QDoubleSpinBox *txtView3DAngle;
    QCheckBox *chkView3DBackground;
    QDoubleSpinBox *txtView3DHeight;
    QCheckBox *chkDeformScalar;
    QCheckBox *chkDeformContour;
    QCheckBox *chkDeformVector;
    QCheckBox *chkView3DBoundingBox;
    QCheckBox *chkView3DSolidGeometry;

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

    // mesh and solver
    QSpinBox *txtMeshAngleSegmentsCount;
    QCheckBox *chkMeshCurvilinearElements;

    // adaptivity
    QLabel *lblMaxDofs;
    QSpinBox *txtMaxDOFs;
    LineEditDouble *txtConvExp;
    QLabel *lblConvExp;
    LineEditDouble *txtThreshold;
    QLabel *lblThreshold;
    QComboBox *cmbStrategy;
    QLabel *lblStrategy;
    QComboBox *cmbMeshRegularity;
    QComboBox *cmbProjNormType;
    QCheckBox *chkUseAnIso;
    QCheckBox *chkFinerReference;

    // commands
    QLineEdit *txtArgumentTriangle;
    QLineEdit *txtArgumentGmsh;

    // script and description
    // startup script
    ScriptEditor *txtStartupScript;
    QLabel *lblStartupScriptError;
    // description
    QTextEdit *txtDescription;

    QPushButton *btnOK;

    void createActions();

    void createControls();
    QWidget *controlsControlsScriptAndDescription();
    QWidget *controlsWorkspace();
    QWidget *controlsMeshAndSolver();
    QWidget *controlsColors();

    void load();
    void save();

signals:
    void apply();

public slots:
    void updateControls();
    void setControls();

private slots:
    void doWorkspaceDefault();
    void doMeshAndSolverDefault();
    void doColorsDefault();
    void doShowGridChanged();
    void doStartupScriptChanged();
    void doStrategyChanged(int index);
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

#endif // SETTINGS
