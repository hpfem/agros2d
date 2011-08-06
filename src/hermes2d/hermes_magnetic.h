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

#ifndef MAGNETIC_H
#define MAGNETIC_H

#include "util.h"
#include "module.h"

struct ModuleMagnetic : public Hermes::Module::ModuleAgros
{
    Q_OBJECT
public:
    ModuleMagnetic(ProblemType problemType, AnalysisType analysisType) : Hermes::Module::ModuleAgros(problemType, analysisType) {}

    void update_time_functions(double time);

    // rewrite
    SceneBoundary *newBoundary();
    SceneBoundary *newBoundary(PyObject *self, PyObject *args);
    SceneBoundary *modifyBoundary(PyObject *self, PyObject *args);
    SceneMaterial *newMaterial(PyObject *self, PyObject *args);
    SceneMaterial *modifyMaterial(PyObject *self, PyObject *args);
};

// *******************************************************************************************

class SceneBoundaryMagneticDialog : public SceneBoundaryDialog
{
    Q_OBJECT

public:
    SceneBoundaryMagneticDialog(SceneBoundary *boundary, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    QLabel *lblValueUnit;
    QComboBox *cmbType;
    ValueLineEdit *txtValueReal;
    ValueLineEdit *txtValueImag;

private slots:
    void doTypeChanged(int index);
};

class SceneMaterialMagneticDialog : public SceneMaterialDialog
{
    Q_OBJECT

public:
    SceneMaterialMagneticDialog(SceneMaterial *material, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    ValueLineEdit *txtPermeability;
    ValueLineEdit *txtConductivity;
    ValueLineEdit *txtCurrentDensityReal;
    ValueLineEdit *txtCurrentDensityImag;
    ValueLineEdit *txtRemanence;
    ValueLineEdit *txtRemanenceAngle;
    ValueLineEdit *txtVelocityX;
    ValueLineEdit *txtVelocityY;
    ValueLineEdit *txtVelocityAngular;
};

#endif // MAGNETIC_H
