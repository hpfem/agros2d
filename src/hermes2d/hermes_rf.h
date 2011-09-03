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

#ifndef RF_H
#define RF_H

#include "util.h"
#include "scenemarker.h"
#include "module.h"

struct ModuleRF : public Hermes::Module::ModuleAgros
{
    Q_OBJECT
public:
    ModuleRF(ProblemType problemType, AnalysisType analysisType) : Hermes::Module::ModuleAgros(problemType, analysisType) {}

    SceneBoundary *newBoundary();
};

// ***********************************************************************************

class SceneBoundaryRFDialog : public SceneBoundaryDialog
{
    Q_OBJECT

public:
    SceneBoundaryRFDialog(SceneBoundary *boundary, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    QLabel *lblValueUnitReal;
    QLabel *lblValueUnitImag;
    QComboBox *cmbType;
    QComboBox *cmbMode;
    ValueLineEdit *txtValueReal;
    ValueLineEdit *txtValueImag;

private slots:
    void doTypeChanged(int index);
};

class SceneMaterialRFDialog : public SceneMaterialDialog
{
    Q_OBJECT

public:
    SceneMaterialRFDialog(SceneMaterial *material, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    ValueLineEdit *txtPermittivity;
    ValueLineEdit *txtPermeability;
    ValueLineEdit *txtConductivity;
    ValueLineEdit *txtCurrentDensityReal;
    ValueLineEdit *txtCurrentDensityImag;

};

#endif // RF_H
