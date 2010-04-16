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
#include "sceneview.h"

class SceneView;

class SceneViewDialog : public QDialog
{
    Q_OBJECT

public:
    SceneViewDialog(SceneView *sceneView, QWidget *parent);

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
