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
#include "scenesolution.h"
#include "scenebasic.h"
#include "scenemarker.h"

void Hermes::Module::ModuleAgros::fillComboBoxScalarVariable(QComboBox *cmbFieldVariable)
{
    fillComboBox(cmbFieldVariable, view_scalar_variables);
}

void Hermes::Module::ModuleAgros::fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
{
    fillComboBox(cmbFieldVariable, view_vector_variables);
}

void Hermes::Module::ModuleAgros::fillComboBox(QComboBox *cmbFieldVariable, Hermes::vector<Hermes::Module::LocalVariable *> list)
{
    for(Hermes::vector<LocalVariable *>::iterator it = list.begin(); it < list.end(); ++it )
    {
        Hermes::Module::LocalVariable *variable = ((Hermes::Module::LocalVariable *) *it);
        cmbFieldVariable->addItem(QString::fromStdString(variable->name),
                                  QString::fromStdString(variable->id));
    }
}

void Hermes::Module::ModuleAgros::fillComboBoxBoundaryCondition(QComboBox *cmbFieldVariable)
{
    for(Hermes::vector<Hermes::Module::BoundaryType *>::iterator it = boundary_types.begin(); it < boundary_types.end(); ++it )
    {
        Hermes::Module::BoundaryType *boundary = ((Hermes::Module::BoundaryType *) *it);
        cmbFieldVariable->addItem(QString::fromStdString(boundary->name),
                                  QString::fromStdString(boundary->id));
    }
}

void Hermes::Module::ModuleAgros::fillComboBoxMaterialProperties(QComboBox *cmbFieldVariable)
{
    for(Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = material_type_variables.begin(); it < material_type_variables.end(); ++it )
    {
        Hermes::Module::MaterialTypeVariable *material = ((Hermes::Module::MaterialTypeVariable *) *it);
        //        cmbFieldVariable->addItem(QString::fromStdString(material->name),
        //                                  QString::fromStdString(material->id));
        cmbFieldVariable->addItem(QString::fromStdString(material->id),
                                  QString::fromStdString(material->id));
    }
}

SceneBoundary *Hermes::Module::ModuleAgros::newBoundary()
{
    assert(0); //TODO
//    return new SceneBoundary(tr("new boundary").toStdString(),
//                             Util::scene()->problemInfo()->module()->boundary_type_default->id);
}

SceneMaterial *Hermes::Module::ModuleAgros::newMaterial()
{
    return new SceneMaterial(tr("new material").toStdString());
}

template <class T>
void deformShapeTemplate(T linVert, int count)
{
    double min =  numeric_limits<double>::max();
    double max = -numeric_limits<double>::max();
    for (int i = 0; i < count; i++)
    {
        double x = linVert[i][0];
        double y = linVert[i][1];

        double dx = Util::scene()->sceneSolution()->sln(0)->get_pt_value(x, y);
        double dy = Util::scene()->sceneSolution()->sln(1)->get_pt_value(x, y);

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

        double dx = Util::scene()->sceneSolution()->sln(0)->get_pt_value(x, y);
        double dy = Util::scene()->sceneSolution()->sln(1)->get_pt_value(x, y);

        linVert[i][0] += k*dx;
        linVert[i][1] += k*dy;
    }
}

void Hermes::Module::ModuleAgros::deform_shape(double3* linVert, int count)
{
    if (Util::scene()->fieldInfo("TODO")->module()->deformed_shape)
        deformShapeTemplate<double3 *>(linVert, count);
}

void Hermes::Module::ModuleAgros::deform_shape(double4* linVert, int count)
{
    if (Util::scene()->fieldInfo("TODO")->module()->deformed_shape)
        deformShapeTemplate<double4 *>(linVert, count);
}

// ***************************************************************************************

void ModuleMagnetic::update_time_functions(double time)
{
    // update materials
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterial *material = Util::scene()->materials[i];
        material->evaluate("magnetic_current_density_external_real", time);
    }
}

void ModuleHeat::update_time_functions(double time)
{
    // update materials
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterial *material = dynamic_cast<SceneMaterial *>(Util::scene()->materials[i]);
        material->evaluate("heat_volume_heat", time);
    }
}

// ****************************************************************************************************

Hermes::Module::ModuleAgros *moduleFactory(std::string id, CoordinateType problem_type, AnalysisType analysis_type,
                                           std::string filename_custom)
{
    // std::cout << filename_custom << std::endl;

    Hermes::Module::ModuleAgros *module = NULL;
    if (id == "heat")
        module = new ModuleHeat(problem_type, analysis_type);
    else if (id == "magnetic")
        module = new ModuleMagnetic(problem_type, analysis_type);
    else
        module = new Hermes::Module::ModuleAgros(problem_type, analysis_type);

    // try to open custom module
    if (id == "custom")
    {
        ifstream ifile_custom(filename_custom.c_str());
        if (!ifile_custom)
            module->read(datadir().toStdString() + "/resources/custom.xml");
        else
            module->read(filename_custom);

        return module;
    }

    // open default module
    std::string filename_default = (datadir() + "/modules/" + QString::fromStdString(id) + ".xml").toStdString();
    ifstream ifile_default(filename_default.c_str());
    if (ifile_default)
    {
        module->read(filename_default);
        return module;
    }

    std::cout << "Module doesn't exists." << std::endl;
    return NULL;
}
