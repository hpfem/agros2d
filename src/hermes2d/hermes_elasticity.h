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

#ifndef HERMES_ELASTICITY_H
#define HERMES_ELASTICITY_H

#include "util.h"
#include "module.h"

struct ModuleElasticity : public Hermes::Module::ModuleAgros
{
    Q_OBJECT
public:
    ModuleElasticity(ProblemType problemType, AnalysisType analysisType) : Hermes::Module::ModuleAgros(problemType, analysisType) {}

    void deform_shape(double3* linVert, int count);
    void deform_shape(double4* linVert, int count);

    // rewrite
    SceneBoundary *newBoundary();
    SceneBoundary *newBoundary(PyObject *self, PyObject *args);
    SceneBoundary *modifyBoundary(PyObject *self, PyObject *args);
    SceneMaterial *newMaterial(PyObject *self, PyObject *args);
    SceneMaterial *modifyMaterial(PyObject *self, PyObject *args);
};

// *******************************************************************************************

class SceneBoundaryElasticityDialog : public SceneBoundaryDialog
{
    Q_OBJECT

public:
    SceneBoundaryElasticityDialog(SceneBoundary *boundary, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    QLabel *lblEquationX;
    QLabel *lblEquationImageX;
    QLabel *lblEquationY;
    QLabel *lblEquationImageY;
    QComboBox *cmbType;
    ValueLineEdit *txtForceX;
    ValueLineEdit *txtForceY;
    ValueLineEdit *txtDisplacementX;
    ValueLineEdit *txtDisplacementY;

private slots:
    void doTypeChanged(int index);
};

class SceneMaterialElasticityDialog : public SceneMaterialDialog
{
    Q_OBJECT

public:
    SceneMaterialElasticityDialog(SceneMaterial *material, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    ValueLineEdit *txtYoungModulus;
    ValueLineEdit *txtPoissonNumber;
    ValueLineEdit *txtForceX;
    ValueLineEdit *txtForceY;
    ValueLineEdit *txtAlpha;
    ValueLineEdit *txtTemp;
    ValueLineEdit *txtTempRef;
};

#endif // HERMES_ELASTICITY_H
