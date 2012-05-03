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

namespace Hermes
{
namespace Module
{

class LocalVariable;

struct ModuleAgros : public QObject, public ModuleDeprecated
{
    Q_OBJECT
public:
    ModuleAgros(CoordinateType problemType, AnalysisType analysisType) : ModuleDeprecated(problemType, analysisType) {}

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable);
    void fillComboBoxContourVariable(QComboBox *cmbFieldVariable);
    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable);
    void fillComboBoxBoundaryCondition(QComboBox *cmbFieldVariable);
    void fillComboBoxMaterialProperties(QComboBox *cmbFieldVariable);

    SceneBoundary *newBoundary();
    SceneMaterial *newMaterial();

//    SceneBoundaryContainer boundaries();
//    SceneMaterialContainer materials();

    void deform_shape(double3* linVert, int count);
    void deform_shape(double4* linVert, int count);

private:
    void fillComboBox(QComboBox *cmbFieldVariable, Hermes::vector<Hermes::Module::LocalVariable *> list);
};

}
}

// module factory
Hermes::Module::ModuleAgros *moduleFactory(std::string id, CoordinateType problem_type, AnalysisType analysis_type,
                                           std::string filename_custom = "");


struct ModuleMagnetic : public Hermes::Module::ModuleAgros
{
    Q_OBJECT
public:
    ModuleMagnetic(CoordinateType problemType, AnalysisType analysisType) : Hermes::Module::ModuleAgros(problemType, analysisType) {}

    void update_time_functions(double time);
};

struct ModuleHeat : public Hermes::Module::ModuleAgros
{
    Q_OBJECT
public:
    ModuleHeat(CoordinateType problemType, AnalysisType analysisType) : Hermes::Module::ModuleAgros(problemType, analysisType) {}

    void update_time_functions(double time);
};

#endif // MODULE_AGROS_H
