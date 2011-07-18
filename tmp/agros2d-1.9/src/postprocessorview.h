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

#ifndef SCENEVIEWDIALOG_H
#define SCENEVIEWDIALOG_H

#include "util.h"

class SceneView;
class SLineEditDouble;

class PostprocessorView : public QDockWidget
{
    Q_OBJECT

public:
    PostprocessorView(SceneView *sceneView, QWidget *parent);

private slots:
    void doApply();
    void doDefault();

private:
    SceneView *m_sceneView;

    QWidget *basic;
    QWidget *advanced;

    // basic
    // show
    QCheckBox *chkShowGeometry;
    QCheckBox *chkShowInitialMesh;

    QButtonGroup *butPostprocessorGroup;
    QRadioButton *radPostprocessorNone;
    QRadioButton *radPostprocessorScalarField;
    QRadioButton *radPostprocessorScalarField3D;
    QRadioButton *radPostprocessorScalarField3DSolid;
    QRadioButton *radPostprocessorModel;
    QRadioButton *radPostprocessorOrder;

    QCheckBox *chkShowContours;
    QCheckBox *chkShowVectors;
    QCheckBox *chkShowSolutionMesh;

    // scalar field
    QComboBox *cmbScalarFieldVariable;
    QComboBox *cmbScalarFieldVariableComp;
    QCheckBox *chkScalarFieldRangeAuto;
    QLabel *lblScalarFieldRangeMin;
    QLabel *lblScalarFieldRangeMax;
    SLineEditDouble *txtScalarFieldRangeMin;
    SLineEditDouble *txtScalarFieldRangeMax;
    QLabel *lblScalarFieldRangeMinError;
    QLabel *lblScalarFieldRangeMaxError;

    // vector field
    QComboBox *cmbVectorFieldVariable;

    // transient
    QComboBox *cmbTimeStep;

    // advanced
    // contours
    QSpinBox *txtContoursCount;

    // scalar field
    QComboBox *cmbPalette;
    QCheckBox *chkPaletteFilter;
    QSpinBox *txtPaletteSteps;
    QComboBox *cmbLinearizerQuality;

    // vector field
    QCheckBox *chkVectorProportional;
    QCheckBox *chkVectorColor;
    QSpinBox *txtVectorCount;
    QDoubleSpinBox *txtVectorScale;

    // order view
    QCheckBox *chkOrderLabel;
    QComboBox *cmbOrderPaletteOrder;

    QPushButton *btnOK;

    void loadBasic();
    void loadAdvanced();
    void saveBasic();
    void saveAdvanced();

    void createControls();
    QWidget *controlsBasic();
    QWidget *controlsAdvanced();

signals:
    void apply();

public slots:
    void updateControls();
    void setControls();

private slots:
    void doScalarFieldVariable(int index);
    void doScalarFieldRangeAuto(int state);
    void doPostprocessorGroupClicked(QAbstractButton *button);
    void doScalarFieldRangeMinChanged();
    void doScalarFieldRangeMaxChanged();
    void doPaletteFilter(int state);
};

#endif // SCENEVIEWDIALOG_H
