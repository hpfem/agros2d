#ifndef SCENEVIEWDIALOG_H
#define SCENEVIEWDIALOG_H

#include <QtGui/QDialog>
#include <QtGui/QCheckBox>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>

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
    QCheckBox *chkShowSolutionMesh;
    QCheckBox *chkShowOrder;
    QCheckBox *chkShowContours;
    QCheckBox *chkShowScalarField;
    QCheckBox *chkShowVectors;

    // grid
    QLineEdit *txtGridStep;

    // contours
    QLineEdit *txtContoursCount;

    // scalar field
    QComboBox *cmbScalarFieldVariable;
    QComboBox *cmbScalarFieldVariableComp;
    QCheckBox *chkScalarFieldRangeAuto;
    QLineEdit *txtScalarFieldRangeMin;
    QLineEdit *txtScalarFieldRangeMax;
    QComboBox *cmbPalette;
    QCheckBox *chkPaletteFilter;
    QLineEdit *txtPaletteSteps;
    QCheckBox *chkView3D;

    // vector field
    QComboBox *cmbVectorFieldVariable;

    void load();
    void save();

    void createControls();

private slots:
    void doScalarFieldVariable(int index);
    void doScalarFieldRangeAuto(int state);
    void doPaletteFilter(int state);
};

#endif // SCENEVIEWDIALOG_H
