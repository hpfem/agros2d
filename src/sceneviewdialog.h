#ifndef SCENEVIEWDIALOG_H
#define SCENEVIEWDIALOG_H

#include "util.h"
#include "sceneview.h"

class SceneView;

class SceneViewDialog : public QDialog
{
    Q_OBJECT

public:
    SceneViewDialog(SceneView *sceneView, QWidget *parent);
    ~SceneViewDialog();

    int showDialog();

private slots:
    void doAccept();
    void doReject();

private:
    SceneView *m_sceneView;

    // show
    QCheckBox *chkShowGrid;
    QCheckBox *chkShowGeometry;
    QCheckBox *chkShowInitialMesh;

    QButtonGroup *butPostprocessorGroup;
    QRadioButton *radPostprocessorNone;
    QRadioButton *radPostprocessorScalarField;
    QRadioButton *radPostprocessorScalarField3D;
    QRadioButton *radPostprocessorScalarField3DSolid;
    QRadioButton *radPostprocessorOrder;

    QCheckBox *chkShowContours;
    QCheckBox *chkShowVectors;
    QCheckBox *chkShowSolutionMesh;

    // scalar field
    QComboBox *cmbScalarFieldVariable;
    QComboBox *cmbScalarFieldVariableComp;
    QCheckBox *chkScalarFieldRangeAuto;
    QLineEdit *txtScalarFieldRangeMin;
    QLineEdit *txtScalarFieldRangeMax;

    // vector field
    QComboBox *cmbVectorFieldVariable;

    // transient
    QComboBox *cmbTimeStep;

    void load();
    void save();

    void createControls();

private slots:
    void doScalarFieldVariable(int index);
    void doScalarFieldRangeAuto(int state);
};

#endif // SCENEVIEWDIALOG_H
