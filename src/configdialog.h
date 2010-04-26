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
#include "scene.h"
#include "sceneview.h"

class ColorButton;

struct SceneViewSettings;

class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    ConfigDialog(QWidget *parent);

private slots:
    void doCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void doPaletteFilter(int state);
    void doClearCommandHistory();
    void doAdvancedDefault();
    void doScalarFieldLog(int state);

    void doAccept();
    void doReject();

private:
    QListWidget *lstView;
    QStackedWidget *pages;
    QWidget *panMain;
    QWidget *panView;
    QWidget *panColors;
    QWidget *panAdvanced;

    // main
    QComboBox *cmbGUIStyle;
    QComboBox *cmbLanguage;
    QComboBox *cmbDefaultPhysicField;

    // delete files
    QCheckBox *chkDeleteTriangleMeshFiles;
    QCheckBox *chkDeleteHermes2DMeshFile;

    // clear command history
    QPushButton *cmdClearCommandHistory;

    // show result in line edit value widget
    QCheckBox *chkLineEditValueShowResult;

    // save with solution
#ifdef BETA
    QCheckBox *chkSaveWithSolution;
#endif

    // check version
    QCheckBox *chkCheckVersion;

    // general view
    QCheckBox *chkZoomToMouse;

    // geometry
    QSpinBox *txtGeometryAngleSegmentsCount;
    SLineEditDouble *txtGeometryNodeSize;
    SLineEditDouble *txtGeometryEdgeWidth;
    SLineEditDouble *txtGeometryLabelSize;

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
    // snap to grid
    QCheckBox *chkSnapToGrid;
    // rulers
    QCheckBox *chkRulers;

    // contours
    QSpinBox *txtContoursCount;

    // scalar field
    QComboBox *cmbPalette;
    QCheckBox *chkPaletteFilter;
    QSpinBox *txtPaletteSteps;
    QCheckBox *chkScalarFieldRangeLog;
    QLineEdit *txtScalarFieldRangeBase;

    // vector field
    QCheckBox *chkVectorProportional;
    QCheckBox *chkVectorColor;
    QSpinBox *txtVectorCount;
    SLineEditDouble *txtVectorScale;

    // order view
    QCheckBox *chkOrderLabel;

    // 3d
    QCheckBox *chkView3DLighting;
    SLineEditDouble *txtView3DAngle;
    QCheckBox *chkView3DBackground;

    // advanced
    QCheckBox *chkIsoOnly;
    QLabel *lblIsoOnly;
    SLineEditDouble *txtConvExp;
    QLabel *lblConvExp;
    SLineEditDouble *txtThreshold;
    QLabel *lblThreshold;
    QComboBox *cmbStrategy;
    QLabel *lblStrategy;
    QComboBox *cmbMeshRegularity;

    QLineEdit *txtArgumentTriangle;
    QLineEdit *txtArgumentFFmpeg;

    void load();
    void save();

    void createControls();
    QWidget *createMainWidget();
    QWidget *createViewWidget();
    QWidget *createColorsWidget();
    QWidget *createAdvancedWidget();
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
