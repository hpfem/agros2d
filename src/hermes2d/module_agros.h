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

#ifndef MODULE_AGROS_H
#define MODULE_AGROS_H

#include "util.h"
#include "hermes2d.h"
#include "module.h"

class SceneBoundary;
class SceneMaterial;
//class SceneBoundaryContainer;
//class SceneMaterialContainer;

namespace Module
{
class LocalVariable;

struct ModuleAgros : public QObject, public BasicModule
{
    Q_OBJECT
public:
    ModuleAgros(const QString &fieldId, CoordinateType problemType, AnalysisType analysisType) : BasicModule(fieldId, problemType, analysisType) {}

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable);
    void fillComboBoxContourVariable(QComboBox *cmbFieldVariable);
    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable);
    void fillComboBoxBoundaryCondition(QComboBox *cmbFieldVariable);
    void fillComboBoxMaterialProperties(QComboBox *cmbFieldVariable);

    SceneBoundary *newBoundary();
    SceneMaterial *newMaterial();

//    SceneBoundaryContainer boundaries();
//    SceneMaterialContainer materials();

    void deformShape(double3* linVert, int count);
    void deformShape(double4* linVert, int count);

private:
    void fillComboBox(QComboBox *cmbFieldVariable, QList<LocalVariable *> list);
};

}

// module factory
Module::ModuleAgros *moduleFactory(const QString &fieldId, CoordinateType problem_type, AnalysisType analysis_type);

struct ModuleMagnetic : public Module::ModuleAgros
{
    Q_OBJECT
public:
    ModuleMagnetic(const QString &fieldId, CoordinateType problemType, AnalysisType analysisType)
        : Module::ModuleAgros(fieldId, problemType, analysisType) {}

    void updateTimeFunctions(double time);
};

struct ModuleHeat : public Module::ModuleAgros
{
    Q_OBJECT
public:
    ModuleHeat(const QString &fieldId, CoordinateType problemType, AnalysisType analysisType)
        : Module::ModuleAgros(fieldId, problemType, analysisType) {}

    void updateTimeFunctions(double time);
};

#endif // MODULE_AGROS_H
