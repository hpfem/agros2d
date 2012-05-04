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

#include "module_agros.h"

#include "util.h"
#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "field.h"
#include "problem.h"

void Hermes::Module::ModuleAgros::fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
{
    fillComboBox(cmbFieldVariable, view_scalar_variables);
}

void Hermes::Module::ModuleAgros::fillComboBoxContourVariable(QComboBox *cmbFieldVariable)
{
    foreach (Hermes::Module::LocalVariable *variable, view_scalar_variables)
        cmbFieldVariable->addItem(variable->name,
                                  variable->id);
}

void Hermes::Module::ModuleAgros::fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
{
    fillComboBox(cmbFieldVariable, view_vector_variables);
}

void Hermes::Module::ModuleAgros::fillComboBox(QComboBox *cmbFieldVariable, QList<Hermes::Module::LocalVariable *> list)
{
    foreach (Hermes::Module::LocalVariable *variable, list)
        cmbFieldVariable->addItem(variable->name,
                                  variable->id);
}

void Hermes::Module::ModuleAgros::fillComboBoxBoundaryCondition(QComboBox *cmbFieldVariable)
{
    foreach (Hermes::Module::BoundaryType *boundary, boundary_types)
        cmbFieldVariable->addItem(boundary->name,
                                  boundary->id);    
}

void Hermes::Module::ModuleAgros::fillComboBoxMaterialProperties(QComboBox *cmbFieldVariable)
{
    foreach (Hermes::Module::MaterialTypeVariable *material, material_type_variables)
        cmbFieldVariable->addItem(material->id,
                                  material->id);
}

SceneBoundary *Hermes::Module::ModuleAgros::newBoundary()
{
    //TODO - add dialog
    FieldInfo *field = Util::problem()->fieldInfo(this->fieldid);

    return new SceneBoundary(field, tr("new boundary"),
                             field->module()->boundary_type_default->id);
}

SceneMaterial *Hermes::Module::ModuleAgros::newMaterial()
{
    //TODO - add dialog
    FieldInfo *field = Util::problem()->fieldInfo(this->fieldid);

    return new SceneMaterial(field, tr("new material"));
}

//SceneBoundaryContainer Hermes::Module::ModuleAgros::boundaries()
//{
//    //TODO - how to cast???
//    MarkerContainer<SceneBoundary> boundaries = Util::scene()->boundaries->filter(QString::fromStdString(fieldid));

//    SceneBoundaryContainer boundaryContainer;
//    foreach (SceneBoundary *boundary, boundaries.items())
//        boundaryContainer.add(boundary);

//    return boundaryContainer;
//}

//SceneMaterialContainer Hermes::Module::ModuleAgros::materials()
//{
//    //TODO - how to cast???
//    MarkerContainer<SceneMaterial> materials = Util::scene()->materials->filter(QString::fromStdString(fieldid));

//    SceneMaterialContainer materialContainer;
//    foreach (SceneMaterial *material, materials.items())
//        materialContainer.add(material);

//    return materialContainer;
//}

template <class T>
void deformShapeTemplate(T linVert, int count)
{
    MultiSolutionArray<double> msa = Util::scene()->activeMultiSolutionArray();

    double min =  numeric_limits<double>::max();
    double max = -numeric_limits<double>::max();
    for (int i = 0; i < count; i++)
    {
        double x = linVert[i][0];
        double y = linVert[i][1];

        double dx = msa.component(0).sln.get()->get_pt_value(x, y);
        double dy = msa.component(1).sln.get()->get_pt_value(x, y);

        double dm = sqrt(Hermes::sqr(dx) + Hermes::sqr(dy));

        if (dm < min) min = dm;
        if (dm > max) max = dm;
    }

    RectPoint rect = Util::scene()->boundingBox();
    double k = qMax(rect.width(), rect.height()) / qMax(min, max) / 15.0;

    for (int i = 0; i < count; i++)
    {
        double x = linVert[i][0];
        double y = linVert[i][1];

        double dx = msa.component(0).sln.get()->get_pt_value(x, y);
        double dy = msa.component(1).sln.get()->get_pt_value(x, y);

        linVert[i][0] += k*dx;
        linVert[i][1] += k*dy;
    }
}

void Hermes::Module::ModuleAgros::deform_shape(double3* linVert, int count)
{
    if (Util::problem()->fieldInfo(fieldid)->module()->deformed_shape)
        deformShapeTemplate<double3 *>(linVert, count);
}

void Hermes::Module::ModuleAgros::deform_shape(double4* linVert, int count)
{
    if (Util::problem()->fieldInfo(fieldid)->module()->deformed_shape)
        deformShapeTemplate<double4 *>(linVert, count);
}

// ***************************************************************************************

void ModuleMagnetic::update_time_functions(double time)
{
    // update materials
    foreach (SceneMaterial *material, Util::scene()->materials->items())
        material->evaluate("magnetic_current_density_external_real", time);
}

void ModuleHeat::update_time_functions(double time)
{
    // update materials
    foreach (SceneMaterial *material, Util::scene()->materials->items())
        material->evaluate("heat_volume_heat", time);
}

// ****************************************************************************************************

Hermes::Module::ModuleAgros *moduleFactory(std::string id, CoordinateType coordinate_type, AnalysisType analysis_type,
                                           std::string filename_custom)
{
    // std::cout << filename_custom << std::endl;

    Hermes::Module::ModuleAgros *module = NULL;
    if (id == "heat")
        module = new ModuleHeat(coordinate_type, analysis_type);
    else if (id == "magnetic")
        module = new ModuleMagnetic(coordinate_type, analysis_type);
    else
        module = new Hermes::Module::ModuleAgros(coordinate_type, analysis_type);

    // try to open custom module
    if (id == "custom")
    {
        ifstream ifile_custom(filename_custom.c_str());
        if (!ifile_custom)
            module->read(datadir() + "/resources/custom.xml");
        else
            module->read(QString::fromStdString(filename_custom));

        return module;
    }

    // open default module
    std::string filename_default = (datadir() + MODULEROOT + "/" + QString::fromStdString(id) + ".xml").toStdString();
    ifstream ifile_default(filename_default.c_str());
    if (ifile_default)
    {
        module->read(QString::fromStdString(filename_default));
        return module;
    }

    std::cout << "Module doesn't exists." << std::endl;
    return NULL;
}
