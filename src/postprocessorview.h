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

class SceneViewPreprocessor;
class SceneViewMesh;
class SceneViewPost2D;
class SceneViewPost3D;
class ChartWidget;
class LineEditDouble;
class CollapsableGroupBoxButton;
class FieldInfo;
class ValueLineEdit;
class ChartControlsWidget;

class PostprocessorWidget : public QWidget
{
    Q_OBJECT

public:
    PostprocessorWidget(SceneViewPreprocessor *sceneGeometry,
                      SceneViewMesh *sceneMesh,
                      SceneViewPost2D *scenePost2D,
                      SceneViewPost3D *scenePost3D,
                      ChartWidget *sceneChart,
                      QWidget *parent);

private slots:
    void doApply();

private:
    SceneViewPreprocessor *m_sceneGeometry;
    SceneViewMesh *m_sceneMesh;
    SceneViewPost2D *m_scenePost2D;
    SceneViewPost3D *m_scenePost3D;
    ChartWidget *m_sceneChart;

    QWidget *basic;
    QWidget *advanced;

    // basic
    QComboBox *cmbFieldInfo;

    // show
    QCheckBox *chkShowInitialMeshView;
    QCheckBox *chkShowSolutionMeshView;
    QCheckBox *chkShowPost2DContourView;
    QCheckBox *chkShowPost2DScalarView;
    QCheckBox *chkShowOrderView;
    QCheckBox *chkShowPost2DVectorView;
    QCheckBox *chkShowPost2DParticleView;

    QButtonGroup *butPost3DGroup;
    QRadioButton *radPost3DNone;
    QRadioButton *radPost3DScalarField3D;
    QRadioButton *radPost3DScalarField3DSolid;
    QRadioButton *radPost3DModel;
    QRadioButton *radPost3DParticleTracing;

    // scalar field
    QComboBox *cmbPostScalarFieldVariable;
    QComboBox *cmbPostScalarFieldVariableComp;
    QCheckBox *chkScalarFieldRangeAuto;
    QLabel *lblScalarFieldRangeMin;
    QLabel *lblScalarFieldRangeMax;
    LineEditDouble *txtScalarFieldRangeMin;
    LineEditDouble *txtScalarFieldRangeMax;
    QLabel *lblScalarFieldRangeMinError;
    QLabel *lblScalarFieldRangeMaxError;

    // vector field
    QComboBox *cmbPost2DVectorFieldVariable;

    // transient
    QGroupBox *grpTransient;
    QComboBox *cmbTimeStep;

    // adaptivity
    QGroupBox *grpAdaptivity;
    QComboBox *cmbAdaptivityStep;
    QComboBox *cmbAdaptivitySolutionType;

    // scalar field
    QCheckBox *chkShowScalarColorBar;
    QComboBox *cmbPalette;
    QCheckBox *chkPaletteFilter;
    QSpinBox *txtPaletteSteps;
    QComboBox *cmbLinearizerQuality;
    QCheckBox *chkScalarFieldRangeLog;
    LineEditDouble *txtScalarFieldRangeBase;
    QSpinBox *txtScalarDecimalPlace;

    // contours
    QComboBox *cmbPost2DContourVariable;
    QSpinBox *txtContoursCount;
    QDoubleSpinBox *txtContourWidth;

    // vector field
    QCheckBox *chkVectorProportional;
    QCheckBox *chkVectorColor;
    QSpinBox *txtVectorCount;
    QDoubleSpinBox *txtVectorScale;
    QComboBox *cmbVectorType;
    QComboBox *cmbVectorCenter;

    // polynomial order
    QCheckBox *chkShowOrderColorbar;
    QComboBox *cmbOrderPaletteOrder;
    QCheckBox *chkOrderLabel;

    // particle tracing
    QCheckBox *chkParticleIncludeRelativisticCorrection;
    QSpinBox *txtParticleNumberOfParticles;
    LineEditDouble *txtParticleStartingRadius;
    LineEditDouble *txtParticleMass;
    LineEditDouble *txtParticleConstant;
    LineEditDouble *txtParticlePointX;
    LineEditDouble *txtParticlePointY;
    QLabel *lblParticlePointX;
    QLabel *lblParticlePointY;
    QLabel *lblParticleVelocityX;
    QLabel *lblParticleVelocityY;
    LineEditDouble *txtParticleVelocityX;
    LineEditDouble *txtParticleVelocityY;
    QCheckBox *chkParticleReflectOnDifferentMaterial;
    QCheckBox *chkParticleReflectOnBoundary;
    LineEditDouble *txtParticleCoefficientOfRestitution;
    QLabel *lblParticleMotionEquations;
    QLabel *lblParticleCustomForceX;
    QLabel *lblParticleCustomForceY;
    QLabel *lblParticleCustomForceZ;
    LineEditDouble *txtParticleCustomForceX;
    LineEditDouble *txtParticleCustomForceY;
    LineEditDouble *txtParticleCustomForceZ;
    QCheckBox *chkParticleColorByVelocity;
    QCheckBox *chkParticleShowPoints;
    LineEditDouble *txtParticleMaximumRelativeError;
    LineEditDouble *txtParticleMinimumStep;
    QSpinBox *txtParticleMaximumNumberOfSteps;
    LineEditDouble *txtParticleDragDensity;
    LineEditDouble *txtParticleDragCoefficient;
    LineEditDouble *txtParticleDragReferenceArea;

    // solid view - materials
    QListWidget *lstSolidMaterials;

    // toolbar
    QToolBox *tbxPostprocessor;
    QPushButton *btnOK;

    // chart
    ChartControlsWidget *m_chartWidget;

    void loadBasic();
    void loadAdvanced();
    void saveBasic();
    void saveAdvanced();

    void createControls();
    QWidget *controlsBasic();
    QWidget *controlsAdvanced();

    QStackedLayout *widgetsLayout;
    QWidget *groupMesh;
    QWidget *groupPost2d;
    QWidget *groupPost3d;
    QWidget *groupChart;

    QWidget *groupMeshOrder;
    CollapsableGroupBoxButton *groupPostScalar;
    QWidget *groupPostScalarAdvanced;
    CollapsableGroupBoxButton *groupPostContour;
    QWidget *groupPostContourAdvanced;
    CollapsableGroupBoxButton *groupPostVector;
    QWidget *groupPostVectorAdvanced;
    CollapsableGroupBoxButton *groupPostParticalTracing;
    QWidget *groupPostParticalTracingAdvanced;
    CollapsableGroupBoxButton *groupPostSolid;
    QWidget *groupPostSolidAdvanced;
    QWidget *groupPostChart;

    QWidget *meshWidget();
    QWidget *post2DWidget();
    QWidget *post3DWidget();
    QWidget *chartWidget();

    QWidget *meshOrderWidget();
    CollapsableGroupBoxButton *postScalarWidget();
    QWidget *postScalarAdvancedWidget();
    CollapsableGroupBoxButton *postContourWidget();
    QWidget *postContourAdvancedWidget();
    CollapsableGroupBoxButton *postVectorWidget();
    QWidget *postVectorAdvancedWidget();
    CollapsableGroupBoxButton *postParticalTracingWidget();
    QWidget *postParticalTracingAdvancedWidget();
    CollapsableGroupBoxButton *postSolidWidget();
    QWidget *postPostSolidAdvancedWidget();
    QWidget *postChartWidget();

    int selectedTimeStep();
    int selectedAdaptivityStep();
    FieldInfo* selectedField();

signals:
    void apply();

public slots:
    void updateControls();
    void refresh();

private slots:
    void doFieldInfo(int index);
    void doTimeStep(int index);
    void doAdaptivityStep(int index);
    void doCalculationFinished();
    void doScalarFieldVariable(int index);
    void doScalarFieldVariableComp(int index);
    void doScalarFieldRangeAuto(int state);
    void doPostprocessorGroupClicked(QAbstractButton *button);
    void doScalarFieldRangeMinChanged();
    void doScalarFieldRangeMaxChanged();
    void doPaletteFilter(int state);
    void doScalarFieldDefault();
    void doContoursVectorsDefault();
    void doParticleDefault();
    void doOrderDefault();
    void doScalarFieldLog(int state);

    void doScalarFieldExpandCollapse(bool collapsed);
    void doContourFieldExpandCollapse(bool collapsed);
    void doVectorFieldExpandCollapse(bool collapsed);
    void doSolidExpandCollapse(bool collapsed);
    void doParticleFieldExpandCollapse(bool collapsed);       
};

#endif // SCENEVIEWDIALOG_H
