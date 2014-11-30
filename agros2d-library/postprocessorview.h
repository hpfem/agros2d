// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef SCENEVIEWDIALOG_H
#define SCENEVIEWDIALOG_H

#include "util.h"

class PostDeal;
class SceneViewPreprocessor;
class SceneViewMesh;
class SceneViewPost2D;
class SceneViewPost3D;
class ChartView;
class LineEditDouble;
class CollapsableGroupBoxButton;
class FieldInfo;
class ValueLineEdit;
class PhysicalFieldWidget;

class PostprocessorWidget : public QWidget
{
    Q_OBJECT

public:
    PostprocessorWidget(PostDeal *postDeal,
                        SceneViewPreprocessor *sceneGeometry,
                        SceneViewMesh *sceneMesh,
                        SceneViewPost2D *scenePost2D,
                        SceneViewPost3D *scenePost3D,
                        QWidget *parent);

private slots:
    void doApply();

private:
    PostDeal *m_postDeal;
    SceneViewPreprocessor *m_sceneGeometry;
    SceneViewMesh *m_sceneMesh;
    SceneViewPost2D *m_scenePost2D;
    SceneViewPost3D *m_scenePost3D;

    QWidget *basic;
    QWidget *advanced;

    // basic
    PhysicalFieldWidget *fieldWidget;

    // show
    QCheckBox *chkShowInitialMeshView;
    QCheckBox *chkShowSolutionMeshView;
    QCheckBox *chkShowPost2DContourView;
    QCheckBox *chkShowPost2DScalarView;
    QCheckBox *chkShowOrderView;
    QCheckBox *chkShowPost2DVectorView;

    QButtonGroup *butPost3DGroup;
    QRadioButton *radPost3DNone;
    QRadioButton *radPost3DScalarField3D;
    QRadioButton *radPost3DScalarField3DSolid;
    QRadioButton *radPost3DModel;
    QCheckBox *chkView3DLighting;
    QDoubleSpinBox *txtView3DAngle;
    QCheckBox *chkView3DBackground;
    QDoubleSpinBox *txtView3DHeight;
    QCheckBox *chkView3DBoundingBox;
    QCheckBox *chkView3DSolidGeometry;

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
    QCheckBox *chkScalarDeform;

    // vector field
    QComboBox *cmbPost2DVectorFieldVariable;

    // scalar field
    QCheckBox *chkShowScalarColorBar;
    QComboBox *cmbPalette;
    QCheckBox *chkPaletteFilter;
    QSpinBox *txtPaletteSteps;
    QCheckBox *chkScalarFieldRangeLog;
    LineEditDouble *txtScalarFieldRangeBase;
    QSpinBox *txtScalarDecimalPlace;

    // contours
    QComboBox *cmbPost2DContourVariable;
    QSpinBox *txtContoursCount;
    QDoubleSpinBox *txtContourWidth;
    QCheckBox *chkContourDeform;

    // vector field
    QCheckBox *chkVectorProportional;
    QCheckBox *chkVectorColor;
    QSpinBox *txtVectorCount;
    QDoubleSpinBox *txtVectorScale;
    QComboBox *cmbVectorType;
    QComboBox *cmbVectorCenter;
    QCheckBox *chkVectorDeform;

    // polynomial order
    QCheckBox *chkShowOrderColorbar;
    QComboBox *cmbOrderPaletteOrder;
    QCheckBox *chkOrderLabel;
    QSpinBox *txtOrderComponent;

    // solid view - materials
    QListWidget *lstSolidMaterials;

    // toolbar
    QToolBox *tbxPostprocessor;
    QPushButton *btnOK;

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

    QWidget *groupMeshOrder;
    CollapsableGroupBoxButton *groupPostScalar;
    QWidget *groupPostScalarAdvanced;
    CollapsableGroupBoxButton *groupPostContour;
    QWidget *groupPostContourAdvanced;
    CollapsableGroupBoxButton *groupPostVector;
    QWidget *groupPostVectorAdvanced;
    CollapsableGroupBoxButton *groupPostSolid;
    QWidget *groupPostSolidAdvanced;
    CollapsableGroupBoxButton *groupPost3D;
    QWidget *groupPost3DAdvanced;

    QWidget *meshWidget();
    QWidget *post2DWidget();
    QWidget *post3DWidget();

    QWidget *meshOrderWidget();
    CollapsableGroupBoxButton *postScalarWidget();
    QWidget *postScalarAdvancedWidget();
    CollapsableGroupBoxButton *postContourWidget();
    QWidget *postContourAdvancedWidget();
    CollapsableGroupBoxButton *postVectorWidget();
    QWidget *postVectorAdvancedWidget();
    CollapsableGroupBoxButton *postSolidWidget();
    QWidget *postPostSolidAdvancedWidget();
    CollapsableGroupBoxButton *postPost3DWidget();
    QWidget *postPost3DAdvancedWidget();

signals:
    void apply();

public slots:
    void updateControls();
    void refresh();

private slots:
    void doField();
    void doCalculationFinished();
    void doScalarFieldVariable(int index);
    void doScalarFieldRangeAuto(int state);
    void doPostprocessorGroupClicked(QAbstractButton *button);
    void doScalarFieldRangeMinChanged();
    void doScalarFieldRangeMaxChanged();
    void doPaletteFilter(int state);
    void doScalarFieldDefault();
    void doContoursVectorsDefault();
    void doOrderDefault();
    void doScalarFieldLog(int state);

    void doScalarFieldExpandCollapse(bool collapsed);
    void doContourFieldExpandCollapse(bool collapsed);
    void doVectorFieldExpandCollapse(bool collapsed);
    void doSolidExpandCollapse(bool collapsed);
    void doPost3DExpandCollapse(bool collapsed);
};

#endif // SCENEVIEWDIALOG_H
