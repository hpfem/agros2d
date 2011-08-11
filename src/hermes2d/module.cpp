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

#include "module.h"

// #include "hermes_general.h"
#include "hermes_electrostatic.h"
#include "hermes_magnetic.h"
#include "hermes_heat.h"
#include "hermes_current.h"
#include "hermes_elasticity.h"
#include "hermes_rf.h"
#include "hermes_acoustic.h"
// #include "hermes_flow.h"

#include "progressdialog.h"
#include "util.h"
#include "scene.h"

#include "mesh/h2d_reader.h"

#include "newton_solver.h"

#include <dirent.h>

double actualTime;

Hermes::Module::ModuleAgros *moduleFactory(std::string id, ProblemType problem_type, AnalysisType analysis_type)
{
    Hermes::Module::ModuleAgros *module = NULL;

    if (id == "electrostatic")
        module = new ModuleElectrostatic(problem_type, analysis_type);
    if (id == "current")
        module = new ModuleCurrent(problem_type, analysis_type);
    if (id == "magnetic")
        module = new ModuleMagnetic(problem_type, analysis_type);
    if (id == "acoustic")
        module = new ModuleAcoustic(problem_type, analysis_type);
    if (id == "elasticity")
        module = new ModuleElasticity(problem_type, analysis_type);
    if (id == "heat")
        module = new ModuleHeat(problem_type, analysis_type);
    if (id == "rf")
        module = new ModuleRF(problem_type, analysis_type);

    if (module)
        module->read((datadir() + "/modules/" + QString::fromStdString(id) + ".xml").toStdString());
    else
        std::cout << "Module doesn't exists." << std::endl;

    return module;
}

// boundary dialog factory
SceneBoundaryDialog *boundaryDialogFactory(SceneBoundary *scene_boundary, QWidget *parent)
{
    if (Util::scene()->problemInfo()->module()->id == "electrostatic")
        return new SceneBoundaryElectrostaticDialog(scene_boundary, parent);
    if (Util::scene()->problemInfo()->module()->id == "current")
        return new SceneBoundaryCurrentDialog(scene_boundary, parent);
    if (Util::scene()->problemInfo()->module()->id == "magnetic")
        return new SceneBoundaryMagneticDialog(scene_boundary, parent);
    if (Util::scene()->problemInfo()->module()->id == "acoustic")
        return new SceneBoundaryAcousticDialog(scene_boundary, parent);
    if (Util::scene()->problemInfo()->module()->id == "elasticity")
        return new SceneBoundaryElasticityDialog(scene_boundary, parent);
    if (Util::scene()->problemInfo()->module()->id == "heat")
        return new SceneBoundaryHeatDialog(scene_boundary, parent);
    if (Util::scene()->problemInfo()->module()->id == "rf")
        return new SceneBoundaryRFDialog(scene_boundary, parent);
}

// material dialog factory
SceneMaterialDialog *materialDialogFactory(SceneMaterial *scene_material, QWidget *parent)
{
    if (Util::scene()->problemInfo()->module()->id == "electrostatic")
        return new SceneMaterialElectrostaticDialog(scene_material, parent);
    if (Util::scene()->problemInfo()->module()->id == "current")
        return new SceneMaterialCurrentDialog(scene_material, parent);
    if (Util::scene()->problemInfo()->module()->id == "magnetic")
        return new SceneMaterialMagneticDialog(scene_material, parent);
    if (Util::scene()->problemInfo()->module()->id == "acoustic")
        return new SceneMaterialAcousticDialog(scene_material, parent);
    if (Util::scene()->problemInfo()->module()->id == "elasticity")
        return new SceneMaterialElasticityDialog(scene_material, parent);
    if (Util::scene()->problemInfo()->module()->id == "heat")
        return new SceneMaterialHeatDialog(scene_material, parent);
    if (Util::scene()->problemInfo()->module()->id == "rf")
        return new SceneMaterialRFDialog(scene_material, parent);
}

std::map<std::string, std::string> availableModules()
{
    static std::map<std::string, std::string> modules;

    // read modules
    if (modules.size() == 0)
    {
        DIR *dp;
        if ((dp = opendir((datadir() + "/modules").toStdString().c_str())) == NULL)
            error("Modules dir '%s' doesn't exists", (datadir() + "/modules").toStdString().c_str());

        struct dirent *dirp;
        while ((dirp = readdir(dp)) != NULL)
        {
            std::string filename = dirp->d_name;

            // skip current and parent dir
            if (filename == "." || filename == "..")
                continue;

            if (filename.substr(filename.size() - 4, filename.size() - 1) == ".xml")
            {
                // read name
                rapidxml::file<> file_data((datadir().toStdString() + "/modules/" + filename).c_str());

                // parse xml
                rapidxml::xml_document<> doc;
                doc.parse<0>(file_data.data());

                // module name
                modules[filename.substr(0, filename.size() - 4)] =
                        QObject::tr(doc.first_node("module")->first_node("general")->first_attribute("name")->value()).toStdString();
            }
        }
        closedir(dp);
    }

    return modules;
}

template <typename Scalar>
void WeakFormAgros<Scalar>::registerForms()
{
    // boundary conditions
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        SceneBoundary *boundary = Util::scene()->edges[i]->boundary;

        if (boundary && boundary != Util::scene()->boundaries[0])
        {
            Hermes::Module::BoundaryType *boundary_type = Util::scene()->problemInfo()->module()->get_boundary_type(boundary->type);

            for (Hermes::vector<ParserFormMatrix *>::iterator it = boundary_type->weakform_matrix_surface.begin();
                 it < boundary_type->weakform_matrix_surface.end(); ++it)
            {
                ParserFormMatrix *form = ((ParserFormMatrix *) *it);

                add_matrix_form_surf(new CustomParserMatrixFormSurf<Scalar>(form->i - 1, form->j - 1,
                                                                            QString::number(i + 1).toStdString(),
                                                                            form->expression,
                                                                            boundary));
            }

            for (Hermes::vector<ParserFormVector *>::iterator it = boundary_type->weakform_vector_surface.begin();
                 it < boundary_type->weakform_vector_surface.end(); ++it)
            {
                ParserFormVector *form = ((ParserFormVector *) *it);

                add_vector_form_surf(new CustomParserVectorFormSurf<Scalar>(form->i - 1,
                                                                            QString::number(i + 1).toStdString(),
                                                                            form->expression,
                                                                            boundary));
            }
        }
    }

    // materials
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        SceneMaterial *material = Util::scene()->labels[i]->material;

        if (material && material != Util::scene()->materials[0])
        {
            for (Hermes::vector<ParserFormMatrix *>::iterator it = Util::scene()->problemInfo()->module()->weakform_matrix_volume.begin();
                 it < Util::scene()->problemInfo()->module()->weakform_matrix_volume.end(); ++it)
            {
                ParserFormMatrix *form = ((ParserFormMatrix *) *it);

                add_matrix_form(new CustomParserMatrixFormVol<Scalar>(form->i - 1, form->j - 1,
                                                                      QString::number(i).toStdString(),
                                                                      form->sym,
                                                                      form->expression,
                                                                      material));
            }

            for (Hermes::vector<ParserFormVector *>::iterator it = Util::scene()->problemInfo()->module()->weakform_vector_volume.begin();
                 it < Util::scene()->problemInfo()->module()->weakform_vector_volume.end(); ++it)
            {
                ParserFormVector *form = ((ParserFormVector *) *it);

                // previous solution (time dep)
                add_vector_form(new CustomParserVectorFormVol<Scalar>(form->i - 1,
                                                                      QString::number(i).toStdString(),
                                                                      form->expression,
                                                                      material));
            }
        }
    }
}

// ***********************************************************************************************

Hermes::Module::LocalVariable::Expression::Expression(rapidxml::xml_node<> *node, ProblemType problemType)
{
    if (problemType == ProblemType_Planar)
    {
        if (node->first_attribute("planar_scalar"))
            scalar = node->first_attribute("planar_scalar")->value();
        if (node->first_attribute("planar_comp_x"))
            comp_x = node->first_attribute("planar_comp_x")->value();
        if (node->first_attribute("planar_comp_y"))
            comp_y = node->first_attribute("planar_comp_y")->value();
    }
    else
    {
        if (node->first_attribute("planar_scalar"))
            scalar = node->first_attribute("axi_scalar")->value();
        if (node->first_attribute("axi_comp_r"))
            comp_x = node->first_attribute("axi_comp_r")->value();
        if (node->first_attribute("axi_comp_z"))
            comp_y = node->first_attribute("axi_comp_z")->value();
    }
}

// ***********************************************************************************************

Hermes::Module::LocalVariable::LocalVariable(rapidxml::xml_node<> *node, ProblemType problemType, AnalysisType analysisType)
{
    id = node->first_attribute("id")->value();
    name = QObject::tr(node->first_attribute("name")->value()).toStdString();
    shortname = node->first_attribute("shortname")->value();
    unit = node->first_attribute("unit")->value();

    if (node->first_attribute("scalar"))
        is_scalar = atoi(node->first_attribute("scalar")->value());
    else
        is_scalar = false;

    if (node->first_node(analysis_type_tostring(analysisType).c_str()))
        expression = Expression(node->first_node(analysis_type_tostring(analysisType).c_str()), problemType);
}

// ***********************************************************************************************

Hermes::Module::Integral::Expression::Expression(rapidxml::xml_node<> *node, ProblemType problemType)
{
    if (node)
    {
        if (problemType == ProblemType_Planar)
        {
            if (node->first_attribute("planar"))
                scalar = node->first_attribute("planar")->value();
        }
        else
        {
            if (node->first_attribute("axi"))
                scalar = node->first_attribute("axi")->value();
        }
    }
}

// ***********************************************************************************************

Hermes::Module::MaterialTypeVariable::MaterialTypeVariable(rapidxml::xml_node<> *node)
{
    id = node->first_attribute("id")->value();
    name = QObject::tr(node->first_attribute("name")->value()).toStdString();
    shortname = node->first_attribute("shortname")->value();
    unit = node->first_attribute("unit")->value();
    if (node->first_attribute("default"))
        default_value = atoi(node->first_attribute("default")->value());
    else
        default_value = 0.0;
}

Hermes::Module::MaterialTypeVariable::MaterialTypeVariable(std::string id, std::string name,
                                                           std::string shortname, std::string unit,
                                                           double default_value)
{
    this->id = id;
    this->name = name;
    this->shortname = shortname;
    this->unit = unit;
    this->default_value = default_value;
}

// ***********************************************************************************************

Hermes::Module::BoundaryType::BoundaryType(Hermes::vector<BoundaryTypeVariable> boundary_type_variables,
                                           rapidxml::xml_node<> *node,
                                           ProblemType problem_type)
{
    id = node->first_attribute("id")->value();
    name = QObject::tr(node->first_attribute("name")->value()).toStdString();

    // variables
    for (rapidxml::xml_node<> *variable = node->first_node("item");
         variable; variable = variable->next_sibling())
        if (std::string(variable->name()) == "item")
            for (Hermes::vector<Hermes::Module::BoundaryTypeVariable>::iterator it = boundary_type_variables.begin();
                 it < boundary_type_variables.end(); ++it )
            {
                Hermes::Module::BoundaryTypeVariable old = (Hermes::Module::BoundaryTypeVariable) *it;

                if (old.id == variable->first_attribute("id")->value())
                {
                    Hermes::Module::BoundaryTypeVariable *var = new Hermes::Module::BoundaryTypeVariable(
                                old.id, old.name, old.shortname, old.unit, old.default_value);

                    variables.push_back(var);
                }
            }

    // weakform
    for (rapidxml::xml_node<> *node_matrix = node->first_node("matrix");
         node_matrix; node_matrix = node_matrix->next_sibling())
        if (std::string(node_matrix->name()) == "matrix")
            weakform_matrix_surface.push_back(new ParserFormMatrix(node_matrix, problem_type));

    for (rapidxml::xml_node<> *node_vector = node->first_node("vector");
         node_vector; node_vector = node_vector->next_sibling())
        if (std::string(node_vector->name()) == "vector")
            weakform_vector_surface.push_back(new ParserFormVector(node_vector, problem_type));

    // essential
    for (rapidxml::xml_node<> *node_essential = node->first_node("essential");
         node_essential; node_essential = node_essential->next_sibling())
        if (std::string(node_essential->name()) == "essential")
            for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = variables.begin();
                 it < variables.end(); ++it )
            {
                Hermes::Module::BoundaryTypeVariable *var = (Hermes::Module::BoundaryTypeVariable *) *it;

                if (var->id == node_essential->first_attribute("id")->value())
                    essential[atoi(node_essential->first_attribute("i")->value())] = var;
            }
}

Hermes::Module::BoundaryTypeVariable::BoundaryTypeVariable(rapidxml::xml_node<> *node)
{
    id = node->first_attribute("id")->value();
    name = QObject::tr(node->first_attribute("name")->value()).toStdString();
    shortname = node->first_attribute("shortname")->value();
    unit = node->first_attribute("unit")->value();
    if (node->first_attribute("default"))
        default_value = atoi(node->first_attribute("default")->value());
    else
        default_value = 0.0;
}

Hermes::Module::BoundaryTypeVariable::BoundaryTypeVariable(std::string id, std::string name,
                                                           std::string shortname, std::string unit,
                                                           double default_value)
{
    this->id = id;
    this->name = name;
    this->shortname = shortname;
    this->unit = unit;
    this->default_value = default_value;
}

Hermes::Module::BoundaryType::~BoundaryType()
{
    // essential
    essential.clear();

    // variables
    for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = variables.begin(); it < variables.end(); ++it)
        delete *it;
    variables.clear();

    // volume weak form
    for (Hermes::vector<ParserFormMatrix *>::iterator it = weakform_matrix_surface.begin(); it < weakform_matrix_surface.end(); ++it)
        delete *it;
    weakform_matrix_surface.clear();

    for (Hermes::vector<ParserFormVector *>::iterator it = weakform_vector_surface.begin(); it < weakform_vector_surface.end(); ++it)
        delete *it;
    weakform_vector_surface.clear();
}

// ***********************************************************************************************

Hermes::Module::Integral::Integral(rapidxml::xml_node<> *node, ProblemType problemType, AnalysisType analysisType)
{
    id = node->first_attribute("id")->value();
    name = QObject::tr(node->first_attribute("name")->value()).toStdString();
    shortname = node->first_attribute("shortname")->value();
    unit = node->first_attribute("unit")->value();

    expression = Expression(node->first_node(analysis_type_tostring(analysisType).c_str()), problemType);
}

// ***********************************************************************************************

Hermes::Module::Module::Module(ProblemType problemType, AnalysisType analysisType)
{
    m_problemType = problemType;
    m_analysisType = analysisType;

    clear();
}

Hermes::Module::Module::~Module()
{
    clear();
}

void Hermes::Module::Module::read(std::string filename)
{
    clear();

    // read name
    // file exists
    if (ifstream(filename.c_str()))
    {
        // save current locale
        char *plocale = setlocale (LC_NUMERIC, "");
        setlocale (LC_NUMERIC, "C");

        rapidxml::file<> file_data(filename.c_str());

        // parse document
        rapidxml::xml_document<> doc;
        doc.parse<0>(file_data.data());

        // problems
        rapidxml::xml_node<> *general = doc.first_node("module")->first_node("general");

        id = general->first_attribute("id")->value();
        name = QObject::tr(general->first_attribute("name")->value()).toStdString();
        description = general->first_node("description")->value();

        rapidxml::xml_node<> *properties = general->first_node("properties");
        steady_state_solutions = atoi(properties->first_attribute("steadystate")->value());
        harmonic_solutions = atoi(properties->first_attribute("harmonic")->value());
        transient_solutions = atoi(properties->first_attribute("transient")->value());

        // constants
        for (rapidxml::xml_node<> *node = doc.first_node("module")->first_node("constants")->first_node("item");
             node; node = node->next_sibling())
            constants[node->first_attribute("id")->value()] = atof(node->first_attribute("value")->value());

        // boundary conditions
        Hermes::vector<Hermes::Module::BoundaryTypeVariable> boundary_type_variables_tmp;
        for (rapidxml::xml_node<> *node = doc.first_node("module")->first_node("boundary")->first_node("item");
             node; node = node->next_sibling())
            if (std::string(node->name()) == "item")
                boundary_type_variables_tmp.push_back(Hermes::Module::BoundaryTypeVariable(node));

        if (rapidxml::xml_node<> *node_analysis =
                doc.first_node("module")->first_node("boundary")->first_node(analysis_type_tostring(m_analysisType).c_str()))
        {
            for (rapidxml::xml_node<> *node = node_analysis->first_node("boundary");
                 node; node = node->next_sibling())
                if (std::string(node->name()) == "boundary")
                    boundary_types.push_back(new Hermes::Module::BoundaryType(boundary_type_variables_tmp, node, m_problemType));
        }
        boundary_type_variables_tmp.clear();

        // material properties
        Hermes::vector<Hermes::Module::MaterialTypeVariable> material_type_variables_tmp;
        for (rapidxml::xml_node<> *node = doc.first_node("module")->first_node("material")->first_node("item");
             node; node = node->next_sibling())
            if (std::string(node->name()) == "item")
                material_type_variables_tmp.push_back(Hermes::Module::MaterialTypeVariable(node));

        if (rapidxml::xml_node<> *node_analysis =
                doc.first_node("module")->first_node("material")->first_node(analysis_type_tostring(m_analysisType).c_str()))
        {
            for (rapidxml::xml_node<> *node = node_analysis->first_node("material");
                 node; node = node->next_sibling())
            {
                if (std::string(node->name()) == "material")
                    for(Hermes::vector<Hermes::Module::MaterialTypeVariable>::iterator it = material_type_variables_tmp.begin();
                        it < material_type_variables_tmp.end(); ++it )
                    {
                        Hermes::Module::MaterialTypeVariable old = (Hermes::Module::MaterialTypeVariable) *it;

                        if (old.id == node->first_attribute("id")->value())
                        {
                            Hermes::Module::MaterialTypeVariable *var = new Hermes::Module::MaterialTypeVariable(
                                        old.id, old.name, old.shortname, old.unit, old.default_value);

                            material_type_variables.push_back(var);
                        }
                    }
            }

            // weakform
            for (rapidxml::xml_node<> *node = node_analysis->first_node("matrix");
                 node; node = node->next_sibling())
                if (std::string(node->name()) == "matrix")
                    weakform_matrix_volume.push_back(new ParserFormMatrix(node, m_problemType));

            for (rapidxml::xml_node<> *node = node_analysis->first_node("vector");
                 node; node = node->next_sibling())
                if (std::string(node->name()) == "vector")
                    weakform_vector_volume.push_back(new ParserFormVector(node, m_problemType));
        }
        material_type_variables_tmp.clear();

        // local variables
        for (rapidxml::xml_node<> *node = doc.first_node("module")->first_node("localvariable")->first_node("item");
             node; node = node->next_sibling())
            variables.push_back(new LocalVariable(node, m_problemType, m_analysisType));
        // custom variable
        LocalVariable *variable = new LocalVariable("custom", "Custom", "custom", "-");
        variable->expression.scalar = "value1";
        variables.push_back(variable);

        // view
        rapidxml::xml_node<> *view = doc.first_node("module")->first_node("view");

        // scalar variables
        rapidxml::xml_node<> *view_scalar_vars = view->first_node("scalarvariable");
        if (view_scalar_vars->first_node(analysis_type_tostring(m_analysisType).c_str()))
        {
            if (view_scalar_vars->first_node(analysis_type_tostring(m_analysisType).c_str())->first_attribute("default"))
                view_default_scalar_variable = get_variable(view_scalar_vars->first_node(analysis_type_tostring(m_analysisType).c_str())->first_attribute("default")->value());

            rapidxml::xml_node<> *node_scalar = view_scalar_vars->first_node(analysis_type_tostring(m_analysisType).c_str());
            for (rapidxml::xml_node<> *node = node_scalar->first_node(); node; node = node->next_sibling())
            {
                LocalVariable *variable = get_variable(node->first_attribute("id")->value());
                if (variable)
                    view_scalar_variables.push_back(variable);
            }
        }
        // custom        
        view_scalar_variables.push_back(get_variable("custom"));

        // vector variables
        rapidxml::xml_node<> *view_vector_vars = view->first_node("vectorvariable");
        if (view_vector_vars->first_node(analysis_type_tostring(m_analysisType).c_str()))
        {
            if (view_vector_vars->first_node(analysis_type_tostring(m_analysisType).c_str())->first_attribute("default"))
                view_default_vector_variable = get_variable(view_vector_vars->first_node(analysis_type_tostring(m_analysisType).c_str())->first_attribute("default")->value());

            rapidxml::xml_node<> *node_vector = view_vector_vars->first_node(analysis_type_tostring(m_analysisType).c_str());
            for (rapidxml::xml_node<> *node = node_vector->first_node("item"); node; node = node->next_sibling())
            {
                LocalVariable *variable = get_variable(node->first_attribute("id")->value());
                if (variable)
                    view_vector_variables.push_back(variable);
            }
        }

        // local variables
        rapidxml::xml_node<> *pointvalues = doc.first_node("module")->first_node("pointvalue");

        rapidxml::xml_node<> *node_pointvalue = pointvalues->first_node(analysis_type_tostring(m_analysisType).c_str());
        if (node_pointvalue)
        {
            for (rapidxml::xml_node<> *node = node_pointvalue->first_node("item"); node; node = node->next_sibling())
            {
                LocalVariable *variable = get_variable(node->first_attribute("id")->value());
                if (variable)
                    local_point.push_back(variable);
            }
        }

        // surface integral
        for (rapidxml::xml_node<> *node = doc.first_node("module")->first_node("surfaceintegral")->first_node();
             node; node = node->next_sibling())
            surface_integral.push_back(new Hermes::Module::Integral(node, m_problemType, m_analysisType));

        // volume integral
        for (rapidxml::xml_node<> *node = doc.first_node("module")->first_node("volumeintegral")->first_node();
             node; node = node->next_sibling())
            volume_integral.push_back(new Hermes::Module::Integral(node, m_problemType, m_analysisType));

        // set system locale
        setlocale(LC_NUMERIC, plocale);
    }
}

void Hermes::Module::Module::clear()
{
    // id
    id = "";
    // name
    name = "";
    // description
    description = "";

    steady_state_solutions = false;
    harmonic_solutions = false;
    transient_solutions = false;

    // constants
    constants.clear();

    // boundary types
    for (Hermes::vector<BoundaryType *>::iterator it = boundary_types.begin(); it < boundary_types.end(); ++it)
        delete *it;
    boundary_types.clear();

    // material types
    for (Hermes::vector<MaterialTypeVariable *>::iterator it = material_type_variables.begin(); it < material_type_variables.end(); ++it)
        delete *it;
    material_type_variables.clear();

    // variables
    for (Hermes::vector<LocalVariable *>::iterator it = variables.begin(); it < variables.end(); ++it)
        delete *it;
    variables.clear();

    // scalar and vector variables
    view_scalar_variables.clear();
    view_vector_variables.clear();
    // default variables
    view_default_scalar_variable = NULL;
    view_default_vector_variable = NULL;

    // local variables
    local_point.clear();

    // surface integrals
    surface_integral.clear();

    // volume integrals
    volume_integral.clear();

    // volume weak form
    for (Hermes::vector<ParserFormMatrix *>::iterator it = weakform_matrix_volume.begin(); it < weakform_matrix_volume.end(); ++it)
        delete *it;
    weakform_matrix_volume.clear();

    for (Hermes::vector<ParserFormVector *>::iterator it = weakform_vector_volume.begin(); it < weakform_vector_volume.end(); ++it)
        delete *it;
    weakform_vector_volume.clear();
}

Hermes::Module::LocalVariable *Hermes::Module::Module::get_variable(std::string id)
{
    for (Hermes::vector<Hermes::Module::LocalVariable *>::iterator it = variables.begin(); it < variables.end(); ++it)
    {
        if (((Hermes::Module::LocalVariable *) *it)->id == id)
            return *it;
    }
    return NULL;
}

Hermes::Module::BoundaryType *Hermes::Module::Module::get_boundary_type(std::string id)
{
    for(Hermes::vector<Hermes::Module::BoundaryType *>::iterator it = boundary_types.begin(); it < boundary_types.end(); ++it )
    {
        if (((Hermes::Module::BoundaryType *) *it)->id == id)
            return *it;
    }
    return NULL;
}

Hermes::Module::BoundaryTypeVariable *Hermes::Module::Module::get_boundary_type_variable(std::string id)
{
    for(Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type_variables.begin(); it < boundary_type_variables.end(); ++it )
    {
        if (((Hermes::Module::BoundaryTypeVariable *) *it)->id == id)
            return *it;
    }
    return NULL;
}

Hermes::Module::MaterialTypeVariable *Hermes::Module::Module::get_material_type_variable(std::string id)
{
    for(Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = material_type_variables.begin(); it < material_type_variables.end(); ++it )
    {
        if (((Hermes::Module::MaterialTypeVariable *) *it)->id == id)
            return *it;
    }
    return NULL;
}

int Hermes::Module::Module::number_of_solution() const
{
    if (m_analysisType == AnalysisType_SteadyState)
        return steady_state_solutions;
    else if (m_analysisType == AnalysisType_Harmonic)
        return harmonic_solutions;
    else if (m_analysisType == AnalysisType_Transient)
        return transient_solutions;

    return 0;
}

mu::Parser *Hermes::Module::Module::get_parser()
{
    mu::Parser *parser = new mu::Parser();

    // pi
    parser->DefineConst("PI", M_PI);

    // frequency
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Harmonic)
        parser->DefineConst("f", Util::scene()->problemInfo()->frequency);

    // timestep
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
        parser->DefineConst("dt", Util::scene()->problemInfo()->timeStep.number);

    for (std::map<std::string, double>::iterator it = constants.begin(); it != constants.end(); ++it)
        parser->DefineConst(it->first, it->second);

    parser->EnableOptimizer(true);

    return parser;
}

std::string Hermes::Module::Module::get_expression(Hermes::Module::LocalVariable *physicFieldVariable,
                                                   PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariableComp)
    {
    // case PhysicFieldVariableComp_Undefined:
    //    return "";
    case PhysicFieldVariableComp_Scalar:
        return physicFieldVariable->expression.scalar;
    case PhysicFieldVariableComp_X:
        return physicFieldVariable->expression.comp_x;
    case PhysicFieldVariableComp_Y:
        return physicFieldVariable->expression.comp_y;
    case PhysicFieldVariableComp_Magnitude:
        return "sqrt((" + physicFieldVariable->expression.comp_x + ") * (" + physicFieldVariable->expression.comp_x + ") + (" + physicFieldVariable->expression.comp_y + ") * (" + physicFieldVariable->expression.comp_y + "))";
    default:
        error("Unknown type.");
    }
}

ViewScalarFilter<double> *Hermes::Module::Module::view_scalar_filter(Hermes::Module::LocalVariable *physicFieldVariable, //TODO PK <double>
                                                                     PhysicFieldVariableComp physicFieldVariableComp)
{
    Hermes::vector<Hermes::Hermes2D::MeshFunction<double> *> sln; //TODO PK <double>
    for (int k = 0; k < Util::scene()->problemInfo()->module()->number_of_solution(); k++)
        sln.push_back(Util::scene()->sceneSolution()->sln(k + (Util::scene()->sceneSolution()->timeStep() * Util::scene()->problemInfo()->module()->number_of_solution())));

    return new ViewScalarFilter<double>(sln, get_expression(physicFieldVariable, physicFieldVariableComp));//TODO PK <double>
}

bool Hermes::Module::Module::solve_init_variables()
{
    // transient
    if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
    {
        if (!Util::scene()->problemInfo()->timeStep.evaluate()) return false;
        if (!Util::scene()->problemInfo()->timeTotal.evaluate()) return false;
        if (!Util::scene()->problemInfo()->initialCondition.evaluate()) return false;
    }

    // edge markers
    for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    {
        SceneBoundary *boundary = Util::scene()->boundaries[i];

        // evaluate script
        for (std::map<std::string, Value>::iterator it = boundary->values.begin(); it != boundary->values.end(); ++it)
            if (!it->second.evaluate()) return false;
    }

    // label markers
    for (int i = 1; i<Util::scene()->materials.count(); i++)
    {
        SceneMaterial *material = Util::scene()->materials[i];

        // evaluate script
        for (std::map<std::string, Value>::iterator it = material->values.begin(); it != material->values.end(); ++it)
            if (!it->second.evaluate()) return false;
    }


    return true;
}

Hermes::vector<SolutionArray<double> *> Hermes::Module::Module::solve(ProgressItemSolve *progressItemSolve)  //TODO PK <double>
{
    if (!solve_init_variables())
        return Hermes::vector<SolutionArray<double> *>(); //TODO PK <double>

    // essential boundary conditions
    Hermes::vector<Hermes::Hermes2D::EssentialBCs<double> > bcs; //TODO PK <double>
    for (int i = 0; i < Util::scene()->problemInfo()->module()->number_of_solution(); i++)
        bcs.push_back(Hermes::Hermes2D::EssentialBCs<double>());  //TODO PK <double>

    for (int i = 0; i < Util::scene()->edges.count(); i++)
    {
        SceneBoundary *boundary = Util::scene()->edges[i]->boundary;

        if (boundary)
        {
            Hermes::Module::BoundaryType *boundary_type = Util::scene()->problemInfo()->module()->get_boundary_type(boundary->type);

            if (boundary_type)
            {
                for (std::map<int, Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->essential.begin();
                     it != boundary_type->essential.end(); ++it)
                {
                    bcs[it->first - 1].add_boundary_condition(new Hermes::Hermes2D::DefaultEssentialBCConst<double>(QString::number(i+1).toStdString(), //TODO PK <double>
                                                                                                                    boundary->values[it->second->id].number));

                }
            }
        }
    }

    WeakFormAgros<double> wf(number_of_solution()); //TODO PK <double>

    Hermes::vector<SolutionArray<double> *> solutionArrayList = solveSolutioArray<double>(progressItemSolve, bcs, &wf);//TODO PK <double>

    return solutionArrayList;
}

// ***********************************************************************************************

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
        cmbFieldVariable->addItem(QString::fromStdString(material->name),
                                  QString::fromStdString(material->id));
    }
}

SceneMaterial *Hermes::Module::ModuleAgros::newMaterial()
{
    return new SceneMaterial(tr("new material").toStdString());
}

// ***********************************************************************************************

void readMeshDirtyFix()
{
    // fix precalulating matrices for mapping of curved elements

    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    std::ostringstream os;
    os << "vertices = [" << std::endl <<
          "  [ 0, 0 ]," << std::endl <<
          "  [ 1, 0 ]," << std::endl <<
          "  [ 0, 1 ]" << std::endl <<
          "]" << std::endl << std::endl <<
          "elements = [" << std::endl <<
          "  [ 0, 1, 2, \"element_0\" ]" << std::endl << std::endl <<
          "boundaries = [" << std::endl <<
          "  [ 0, 1, \"0\" ]," << std::endl <<
          "  [ 1, 2, \"0\" ]," << std::endl <<
          "  [ 2, 0, \"0\" ]" << std::endl <<
          "]" << std::endl << std::endl <<
          "curves = [" << std::endl <<
          "  [ 0, 1, 90 ]" << std::endl <<
          "]" << std::endl;

    // qDebug() << QString::fromStdString(os.str());

    Hermes::Hermes2D::Mesh mesh;
    Hermes::Hermes2D::H2DReader meshloader;

    // FIXME - Amuthan - hack!!!
    std::ofstream outputFile((tempProblemDir().toStdString() + "/dummy.mesh").c_str(), fstream::out);
    outputFile << os.str();
    outputFile.close();

    meshloader.load((tempProblemDir().toStdString() + "/dummy.mesh").c_str(), &mesh);

    // FIXME - Amuthan - load_stream doesn't support streams!!!
    // meshloader.load_str(os.str().c_str(), &mesh);

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

Hermes::Hermes2D::Mesh *readMeshFromFile(const QString &fileName)
{
    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    // load the mesh file
    Hermes::Hermes2D::Mesh *mesh = new Hermes::Hermes2D::Mesh();
    Hermes::Hermes2D::H2DReader meshloader;
    meshloader.load(fileName.toStdString().c_str(), mesh);

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return mesh;
}

void writeMeshFromFile(const QString &fileName, Hermes::Hermes2D::Mesh *mesh)
{
    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    Hermes::Hermes2D::H2DReader meshloader;
    meshloader.save(fileName.toStdString().c_str(), mesh);

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

void refineMesh(Hermes::Hermes2D::Mesh *mesh, bool refineGlobal, bool refineTowardsEdge)
{
    // refine mesh - global
    if (refineGlobal)
        for (int i = 0; i < Util::scene()->problemInfo()->numberOfRefinements; i++)
            mesh->refine_all_elements(0);

    // refine mesh - boundary
    if (refineTowardsEdge)
        for (int i = 0; i < Util::scene()->edges.count(); i++)
            if (Util::scene()->edges[i]->refineTowardsEdge > 0)
                mesh->refine_towards_boundary(QString::number(((Util::scene()->edges[i]->boundary->type != "") ? i + 1 : -i)).toStdString(),
                                              Util::scene()->edges[i]->refineTowardsEdge);
}

// return geom type
Hermes::Hermes2D::GeomType convertProblemType(ProblemType problemType)
{
    return (problemType == ProblemType_Planar ? Hermes::Hermes2D::HERMES_PLANAR : Hermes::Hermes2D::HERMES_AXISYM_Y);
}


template <typename Scalar>
Hermes::vector<SolutionArray<Scalar> *> solveSolutioArray(ProgressItemSolve *progressItemSolve,  //TODO PK <double>
                                                          Hermes::vector<Hermes::Hermes2D::EssentialBCs<Scalar> > bcs,
                                                          WeakFormAgros<Scalar> *wf)
{
    SolutionAgros<Scalar> solutionAgros(progressItemSolve, wf);

    Hermes::vector<SolutionArray<double> *> solutionArrayList = solutionAgros.solveSolutionArray(bcs);//TODO PK <double>
    return solutionArrayList;
}

// *********************************************************************************************************************************************

template <typename Scalar>
SolutionAgros<Scalar>::SolutionAgros(ProgressItemSolve *progressItemSolve, WeakFormAgros<Scalar> *wf)
{
    analysisType = Util::scene()->problemInfo()->analysisType;
    polynomialOrder = Util::scene()->problemInfo()->polynomialOrder;
    adaptivityType = Util::scene()->problemInfo()->adaptivityType;
    adaptivitySteps = Util::scene()->problemInfo()->adaptivitySteps;
    adaptivityTolerance = Util::scene()->problemInfo()->adaptivityTolerance;
    adaptivityMaxDOFs = Util::scene()->problemInfo()->adaptivityMaxDOFs;
    numberOfSolution = Util::scene()->problemInfo()->module()->number_of_solution();
    timeTotal = Util::scene()->problemInfo()->timeTotal.number;
    timeStep = Util::scene()->problemInfo()->timeStep.number;
    initialCondition = Util::scene()->problemInfo()->initialCondition.number;

    nonlinearTolerance = Util::scene()->problemInfo()->nonlinearTolerance;
    nonlinearSteps = Util::scene()->problemInfo()->nonlinearSteps;

    matrixSolver = Util::scene()->problemInfo()->matrixSolver;

    m_progressItemSolve = progressItemSolve;
    m_wf = wf;
}

//template <typename Scalar>
//Hermes::vector<SolutionArray<Scalar> *> SolutionAgros<Scalar>::solveSolutioArray2(Hermes::vector<Hermes::Hermes2D::EssentialBCs<Scalar> > bcs)
//{
//    prepare();
//    for(timesteps){
//        if(not adapt)
//            solve(space, solution);
//        else{
//            solve(refspace, refsolution);
//            project....
//        }

//    }
//}

//TODO PK why bcs has to be reference???

template <typename Scalar>
void SolutionAgros<Scalar>::initCalculation(Hermes::vector<Hermes::Hermes2D::EssentialBCs<Scalar> >& bcs)
{
    // load the mesh file
    mesh = readMeshFromFile(tempProblemFileName() + ".mesh");
    refineMesh(mesh, true, true);

    Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *select = NULL;
    switch (adaptivityType)
    {
    case AdaptivityType_H:
        select = new Hermes::Hermes2D::RefinementSelectors::HOnlySelector<Scalar>();
        break;
    case AdaptivityType_P:
        select = new Hermes::Hermes2D::RefinementSelectors::H1ProjBasedSelector<Scalar>(Hermes::Hermes2D::RefinementSelectors::H2D_P_ANISO,
                                                                                        Util::config()->convExp,
                                                                                        H2DRS_DEFAULT_ORDER);
        break;
    case AdaptivityType_HP:
        select = new Hermes::Hermes2D::RefinementSelectors::H1ProjBasedSelector<Scalar>(Hermes::Hermes2D::RefinementSelectors::H2D_HP_ANISO,
                                                                                        Util::config()->convExp,
                                                                                        H2DRS_DEFAULT_ORDER);
        break;
    }

    for (int i = 0; i < numberOfSolution; i++)
    {
        space.push_back(new Hermes::Hermes2D::H1Space<Scalar>(mesh, &bcs[i], polynomialOrder));

        // set order by element
        for (int j = 0; j < Util::scene()->labels.count(); j++)
            if (Util::scene()->labels[j]->material != Util::scene()->materials[0])
                space.at(i)->set_uniform_order(Util::scene()->labels[j]->polynomialOrder > 0 ? Util::scene()->labels[j]->polynomialOrder : polynomialOrder,
                                               QString::number(j).toStdString());

        // solution agros array
        solution.push_back(new Hermes::Hermes2D::Solution<Scalar>());

        if (adaptivityType != AdaptivityType_None)
        {
            // add norm
            projNormType.push_back(Util::config()->projNormType);
            // add refinement selector
            selector.push_back(select);
            // reference solution
            solutionReference.push_back(new Hermes::Hermes2D::Solution<Scalar>());
        }
    }
}

template <typename Scalar>
void SolutionAgros<Scalar>::cleanup()
{

}

template <typename Scalar>
bool SolutionAgros<Scalar>::solveOneProblem(Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaceParam,
                                            Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutionParam)
{
    double actualTime = 0.;
    Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(spaceParam, actualTime);
    Util::scene()->problemInfo()->module()->update_time_functions(actualTime);

    m_wf->set_current_time(actualTime);
    m_wf->delete_all();
    m_wf->registerForms();

    // Initialize the FE problem.
    Hermes::Hermes2D::DiscreteProblem<double> dp(m_wf, spaceParam);

    int ndof = Hermes::Hermes2D::Space<Scalar>::get_num_dofs(spaceParam);
    cout << "ndof " << ndof << endl;

    // Initial coefficient vector for the Newton's method.
    double* coeff_vec = new double[ndof];
    memset(coeff_vec, 0, ndof*sizeof(double));

    // Perform Newton's iteration and translate the resulting coefficient vector into a Solution.
    Hermes::Hermes2D::NewtonSolver<double> newton(&dp, Hermes::SOLVER_UMFPACK);
    if (!newton.solve(coeff_vec, nonlinearTolerance, nonlinearSteps))
    {
        m_progressItemSolve->emitMessage(QObject::tr("Newton's iteration failed"), true);
        return false;
    }
    else
        Hermes::Hermes2D::Solution<double>::vector_to_solutions(newton.get_sln_vector(), spaceParam, solutionParam);

    return true;
}


template <typename Scalar>
Hermes::vector<SolutionArray<Scalar> *> SolutionAgros<Scalar>::solveSolutionArray(Hermes::vector<Hermes::Hermes2D::EssentialBCs<Scalar> > bcs)
{
    QTime time;

    // solution agros array
    Hermes::vector<SolutionArray<Scalar> *> solutionArrayList;

    // error marker
    bool isError = false;

    initCalculation(bcs);

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space) == 0)
    {
        m_progressItemSolve->emitMessage(QObject::tr("DOF is zero"), true);
        cleanup();
        return solutionArrayList;
    }

    for (int i = 0; i < numberOfSolution; i++)
    {
        // transient
        if (analysisType == AnalysisType_Transient)
        {
            // constant initial solution
            solution.at(i)->set_const(mesh, initialCondition);
            solutionArrayList.push_back(solutionArray(solution.at(i)));
        }

        // nonlinear - initial solution
        solution.at(i)->set_const(mesh, 0.0);
    }



    actualTime = 0.0;
    int timesteps = (analysisType == AnalysisType_Transient) ? floor(timeTotal/timeStep) : 1;
    //for (int n = 0; n<timesteps; n++)
    {
        solveOneProblem(space, solution);

    }

    int n, error = 0, actualAdaptivitySteps; //TODO

    // output
    if (!isError)
    {
        for (int i = 0; i < numberOfSolution; i++){
            cout << "solution" << i << endl;
            solutionArrayList.push_back(solutionArray(solution.at(i), space.at(i), error, actualAdaptivitySteps, (n+1)*timeStep));
        }

        if (analysisType == AnalysisType_Transient)
            m_progressItemSolve->emitMessage(QObject::tr("Transient time step (%1/%2): %3 s").
                                             arg(n+1).
                                             arg(timesteps).
                                             arg(actualTime, 0, 'e', 2), false, n+2);
    }

    cleanup();
    return solutionArrayList;
}

template <typename Scalar>
Hermes::vector<SolutionArray<Scalar> *> SolutionAgros<Scalar>::solveSolutionArrayOld(Hermes::vector<Hermes::Hermes2D::EssentialBCs<Scalar> > bcs)
{
    QTime time;

    // solution agros array
    Hermes::vector<SolutionArray<Scalar> *> solutionArrayList;

    // load the mesh file
    mesh = readMeshFromFile(tempProblemFileName() + ".mesh");
    refineMesh(mesh, true, true);

    // create an H1 space
    Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> space;
    // create hermes solution array
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solution;
    // create reference solution
    Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solutionReference;

    // projection norms
    Hermes::vector<Hermes::Hermes2D::ProjNormType> projNormType;

    // prepare selector
    Hermes::vector<Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *> selector;

    // error marker
    bool isError = false;

    Hermes::Hermes2D::RefinementSelectors::Selector<Scalar> *select = NULL;
    switch (adaptivityType)
    {
    case AdaptivityType_H:
        select = new Hermes::Hermes2D::RefinementSelectors::HOnlySelector<Scalar>();
        break;
    case AdaptivityType_P:
        select = new Hermes::Hermes2D::RefinementSelectors::H1ProjBasedSelector<Scalar>(Hermes::Hermes2D::RefinementSelectors::H2D_P_ANISO,
                                                                                        Util::config()->convExp,
                                                                                        H2DRS_DEFAULT_ORDER);
        break;
    case AdaptivityType_HP:
        select = new Hermes::Hermes2D::RefinementSelectors::H1ProjBasedSelector<Scalar>(Hermes::Hermes2D::RefinementSelectors::H2D_HP_ANISO,
                                                                                        Util::config()->convExp,
                                                                                        H2DRS_DEFAULT_ORDER);
        break;
    }

    for (int i = 0; i < numberOfSolution; i++)
    {
        space.push_back(new Hermes::Hermes2D::H1Space<Scalar>(mesh, &bcs[i], polynomialOrder));

        // set order by element
        for (int j = 0; j < Util::scene()->labels.count(); j++)
            if (Util::scene()->labels[j]->material != Util::scene()->materials[0])
                space.at(i)->set_uniform_order(Util::scene()->labels[j]->polynomialOrder > 0 ? Util::scene()->labels[j]->polynomialOrder : polynomialOrder,
                                               QString::number(j).toStdString());

        // solution agros array
        solution.push_back(new Hermes::Hermes2D::Solution<Scalar>());

        if (adaptivityType != AdaptivityType_None)
        {
            // add norm
            projNormType.push_back(Util::config()->projNormType);
            // add refinement selector
            selector.push_back(select);
            // reference solution
            solutionReference.push_back(new Hermes::Hermes2D::Solution<Scalar>());
        }
    }

    // check for DOFs
    if (Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space) == 0)
    {
        m_progressItemSolve->emitMessage(QObject::tr("DOF is zero"), true);
    }
    else
    {
        for (int i = 0; i < numberOfSolution; i++)
        {
            // transient
            if (analysisType == AnalysisType_Transient)
            {
                // constant initial solution
                solution.at(i)->set_const(mesh, initialCondition);
                solutionArrayList.push_back(solutionArray(solution.at(i)));
            }

            // nonlinear - initial solution
            solution.at(i)->set_const(mesh, 0.0);
        }

        actualTime = 0.0;

        // update time function
        /*
Util::scene()->problemInfo()->module()->update_time_functions(actualTime);

m_wf->set_current_time(actualTime);
m_wf->solution.clear();
// FIXME
if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
for (int i = 0; i < solution.size(); i++)
m_wf->solution.push_back((Hermes::Hermes2D::MeshFunction<Scalar> *) solution[i]);
m_wf->delete_all();
m_wf->registerForms();
*/

        // emit message
        if (adaptivityType != AdaptivityType_None)
            m_progressItemSolve->emitMessage(QObject::tr("Adaptivity type: %1").arg(adaptivityTypeString(adaptivityType)), false);

        double error = 0.0;

        // solution
        /*
int maxAdaptivitySteps = (adaptivityType == AdaptivityType_None) ? 1 : adaptivitySteps;
int actualAdaptivitySteps = -1;
for (int i = 0; i<maxAdaptivitySteps; i++)
{
// set up the solver, matrix, and rhs according to the solver selection.
SparseMatrix<Scalar> *matrix = Hermes::Algebra::create_matrix<Scalar>(matrixSolver);
Vector<Scalar> *rhs = create_vector<Scalar>(matrixSolver);
Hermes::Solvers::LinearSolver<Scalar> *solver = create_linear_solver<Scalar>(matrixSolver, matrix, rhs); //TODO PK LinearSolver ??

if (adaptivityType == AdaptivityType_None)
{
if (analysisType != AnalysisType_Transient)
solve(space, solution, solver, matrix, rhs); //TODO PK solver...
}
else
{
// construct globally refined reference mesh and setup reference space.
Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> spaceReference = *Hermes::Hermes2D::Space<Scalar>::construct_refined_spaces(space);

// assemble reference problem.
//solve(spaceReference, solutionReference, solver, matrix, rhs); //TODO PK solver...

if (!isError)
{
// project the fine mesh solution onto the coarse mesh.
Hermes::Hermes2D::OGProjection<Scalar>::project_global(space, solutionReference, solution, matrixSolver);

// Calculate element errors and total error estimate.
Hermes::Hermes2D::Adapt<Scalar> adaptivity(space, projNormType);

// Calculate error estimate for each solution component and the total error estimate.
error = adaptivity.calc_err_est(solution,
solutionReference) * 100;

// emit signal
m_progressItemSolve->emitMessage(QObject::tr("Adaptivity rel. error (step: %2/%3, DOFs: %4/%5): %1%").
arg(error, 0, 'f', 3).
arg(i + 1).
arg(maxAdaptivitySteps).
arg(Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space)).
arg(Hermes::Hermes2D::Space<Scalar>::get_num_dofs(spaceReference)), false, 1);
// add error to the list
m_progressItemSolve->addAdaptivityError(error, Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space));

if (error < adaptivityTolerance || Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space) >= adaptivityMaxDOFs)
{
break;
}
if (i != maxAdaptivitySteps-1) adaptivity.adapt(selector,
Util::config()->threshold,
Util::config()->strategy,
Util::config()->meshRegularity);
actualAdaptivitySteps = i+1;
}

if (m_progressItemSolve->isCanceled())
{
isError = true;
break;
}

// delete reference space
for (int i = 0; i < spaceReference.size(); i++)
{
delete spaceReference.at(i)->get_mesh();
delete spaceReference.at(i);
}
spaceReference.clear();
}

// clean up.
//delete solver; //TODO PK solver...
delete matrix;
delete rhs;
}

// delete reference solution
for (int i = 0; i < solutionReference.size(); i++)
delete solutionReference.at(i);
solutionReference.clear();

// delete selector
if (select) delete select;
selector.clear();
*/

        // timesteps
        // if (!isError)
        {
            // set up the solver, matrix, and rhs according to the solver selection.
            SparseMatrix<Scalar> *matrix = create_matrix<Scalar>(matrixSolver);
            Vector<Scalar> *rhs = create_vector<Scalar>(matrixSolver);
            LinearSolver<Scalar> *solver = create_linear_solver<Scalar>(matrixSolver, matrix, rhs);

            // allocate dp for transient solution
            /*
Hermes::Hermes2D::DiscreteProblem<Scalar> *dpTran = NULL;
if (analysisType == AnalysisType_Transient)
{
dpTran = new Hermes::Hermes2D::DiscreteProblem<Scalar>(m_wf, space);
}
*/

            int maxAdaptivitySteps = (adaptivityType == AdaptivityType_None) ? 1 : adaptivitySteps;
            int actualAdaptivitySteps = -1;

            int timesteps = (analysisType == AnalysisType_Transient) ? floor(timeTotal/timeStep) : 1;
            for (int n = 0; n<timesteps; n++)
            {
                // set actual time
                actualTime = (n+1)*timeStep;

                // update essential bc values
                Hermes::Hermes2D::Space<Scalar>::update_essential_bc_values(space, actualTime);
                // update timedep values
                Util::scene()->problemInfo()->module()->update_time_functions(actualTime);

                m_wf->set_current_time(actualTime);
                if (Util::scene()->problemInfo()->analysisType == AnalysisType_Transient)
                    for (int i = 0; i < solution.size(); i++)
                        m_wf->solution.push_back((Hermes::Hermes2D::MeshFunction<Scalar> *) solution[i]);
                m_wf->delete_all();
                m_wf->registerForms();

                // Initialize the FE problem.
                Hermes::Hermes2D::DiscreteProblem<double> dp(m_wf, space);

                int ndof = Hermes::Hermes2D::Space<Scalar>::get_num_dofs(space);

                // Initial coefficient vector for the Newton's method.
                double* coeff_vec = new double[ndof];
                memset(coeff_vec, 0, ndof*sizeof(double));

                // Perform Newton's iteration and translate the resulting coefficient vector into a Solution.
                Hermes::Hermes2D::NewtonSolver<double> newton(&dp, Hermes::SOLVER_UMFPACK);
                if (!newton.solve(coeff_vec, nonlinearTolerance, nonlinearSteps))
                {
                    m_progressItemSolve->emitMessage(QObject::tr("Newton's iteration failed"), true);
                    isError = true;
                }
                else
                    Hermes::Hermes2D::Solution<double>::vector_to_solutions(newton.get_sln_vector(), space, solution);

                // output
                if (!isError)
                {
                    for (int i = 0; i < numberOfSolution; i++)
                        solutionArrayList.push_back(solutionArray(solution.at(i), space.at(i), error, actualAdaptivitySteps, (n+1)*timeStep));

                    if (analysisType == AnalysisType_Transient)
                        m_progressItemSolve->emitMessage(QObject::tr("Transient time step (%1/%2): %3 s").
                                                         arg(n+1).
                                                         arg(timesteps).
                                                         arg(actualTime, 0, 'e', 2), false, n+2);
                }
                else
                {
                    break;
                }

                if (m_progressItemSolve->isCanceled())
                {
                    isError = true;
                    break;
                }
            }

            // clean up
            if (solver) delete solver;
            if (matrix) delete matrix;
            if (rhs) delete rhs;

            // if (dpTran) delete dpTran;
        }
    }
    // delete mesh
    delete mesh;

    // delete space
    for (unsigned int i = 0; i < space.size(); i++)
    {
        // delete space.at(i)->get_mesh();
        delete space.at(i);
    }
    space.clear();

    // delete last solution
    for (unsigned int i = 0; i < solution.size(); i++)
        delete solution.at(i);
    solution.clear();

    if (isError)
    {
        for (int i = 0; i < solutionArrayList.size(); i++)
            delete solutionArrayList.at(i);
        solutionArrayList.clear();
    }
    return solutionArrayList;
}

template <typename Scalar>
bool SolutionAgros<Scalar>::solveLinear(Hermes::Hermes2D::DiscreteProblem<Scalar> *dp,
                                        Hermes::vector<Hermes::Hermes2D::Space<Scalar> *> space,
                                        Hermes::vector<Hermes::Hermes2D::Solution<Scalar> *> solution,
                                        Hermes::Solvers::LinearSolver<Scalar> *solver, Hermes::Algebra::SparseMatrix<Scalar> *matrix, Hermes::Algebra::Vector<Scalar> *rhs)
{
    // QTime time;
    // time.start();
    dp->assemble(matrix, rhs);
    // qDebug() << "assemble: " << time.elapsed();

    if(solver->solve())
    {
        //Hermes::Hermes2D::Solution<Scalar>::vector_to_solutions(solver->get_solution(), space, solution); //TODO PK
        return true;
    }
    else
    {
        m_progressItemSolve->emitMessage(QObject::tr("Matrix solver failed."), true, 1);
        return false;
    }
}

template <typename Scalar>
SolutionArray<Scalar> *SolutionAgros<Scalar>::solutionArray(Hermes::Hermes2D::Solution<Scalar> *sln, Hermes::Hermes2D::Space<Scalar> *space, double adaptiveError, double adaptiveSteps, double time)
{
    SolutionArray<Scalar> *solution = new SolutionArray<Scalar>();
    solution->order = new Hermes::Hermes2D::Views::Orderizer();
    if (space) solution->order->process_space(space);
    solution->sln = new Hermes::Hermes2D::Solution<Scalar>();
    if (sln) solution->sln->copy(sln);
    solution->adaptiveError = adaptiveError;
    solution->adaptiveSteps = adaptiveSteps;
    solution->time = time;

    return solution;
}

// *********************************************************************************************************************************************

Parser::Parser()
{

}

Parser::~Parser()
{
    // delete parser
    for (Hermes::vector<mu::Parser *>::iterator it = parser.begin(); it < parser.end(); ++it)
        delete *it;

    parser.clear();
}

void Parser::setParserVariables(Material *material, Boundary *boundary)
{
    if (material)
    {
        Hermes::vector<Hermes::Module::MaterialTypeVariable *> materials = Util::scene()->problemInfo()->module()->material_type_variables;
        for (Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = materials.begin(); it < materials.end(); ++it)
        {
            Hermes::Module::MaterialTypeVariable *variable = ((Hermes::Module::MaterialTypeVariable *) *it);
            parser_variables[variable->shortname] = material->values[variable->id].number;
        }
    }

    if (boundary)
    {
        Hermes::Module::BoundaryType *boundary_type = Util::scene()->problemInfo()->module()->get_boundary_type(boundary->type);
        for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin(); it < boundary_type->variables.end(); ++it)
        {
            Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);
            parser_variables[variable->shortname] = boundary->values[variable->id].number;
        }
    }
}

void Parser::initParserMaterialVariables()
{
    Hermes::vector<Hermes::Module::MaterialTypeVariable *> materials = Util::scene()->problemInfo()->module()->material_type_variables;
    for (Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = materials.begin(); it < materials.end(); ++it)
    {
        Hermes::Module::MaterialTypeVariable *variable = ((Hermes::Module::MaterialTypeVariable *) *it);
        parser_variables[variable->shortname] = 0.0;
    }

    // set material variables
    for (std::map<std::string, double>::iterator itv = parser_variables.begin(); itv != parser_variables.end(); ++itv)
        for (Hermes::vector<mu::Parser *>::iterator it = parser.begin(); it < parser.end(); ++it)
            ((mu::Parser *) *it)->DefineVar(itv->first, &itv->second);
}

void Parser::initParserBoundaryVariables(Boundary *boundary)
{
    Hermes::Module::BoundaryType *boundary_type = Util::scene()->problemInfo()->module()->get_boundary_type(boundary->type);
    for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin(); it < boundary_type->variables.end(); ++it)
    {
        Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);
        parser_variables[variable->shortname] = 0.0;
    }

    // set material variables
    for (std::map<std::string, double>::iterator itv = parser_variables.begin(); itv != parser_variables.end(); ++itv)
        for (Hermes::vector<mu::Parser *>::iterator it = parser.begin(); it < parser.end(); ++it)
            ((mu::Parser *) *it)->DefineVar(itv->first, &itv->second);
}

// *********************************************************************************************************************************************

template <typename Scalar>
ViewScalarFilter<Scalar>::ViewScalarFilter(Hermes::vector<Hermes::Hermes2D::MeshFunction<Scalar> *> sln,
                                           std::string expression)
    : Hermes::Hermes2D::Filter<Scalar>(sln)
{
    parser = new Parser();
    initParser(expression);

    // init material variables
    parser->initParserMaterialVariables();
}

template <typename Scalar>
ViewScalarFilter<Scalar>::~ViewScalarFilter()
{
    delete parser;

    delete [] pvalue;
    delete [] pdx;
    delete [] pdy;
}

template <typename Scalar>
void ViewScalarFilter<Scalar>::initParser(std::string expression)
{
    mu::Parser *pars = Util::scene()->problemInfo()->module()->get_parser();

    pars->SetExpr(expression);

    pars->DefineVar("x", &px);
    pars->DefineVar("y", &py);

    pvalue = new double[Hermes::Hermes2D::Filter<Scalar>::num];
    pdx = new double[Hermes::Hermes2D::Filter<Scalar>::num];
    pdy = new double[Hermes::Hermes2D::Filter<Scalar>::num];

    for (int k = 0; k < Hermes::Hermes2D::Filter<Scalar>::num; k++)
    {
        std::stringstream number;
        number << (k+1);

        pars->DefineVar("value" + number.str(), &pvalue[k]);
        pars->DefineVar("dx" + number.str(), &pdx[k]);
        pars->DefineVar("dy" + number.str(), &pdy[k]);
    }

    parser->parser.push_back(pars);
}

template <typename Scalar>
double ViewScalarFilter<Scalar>::get_pt_value(double x, double y, int item)
{
    return 0.0;
}

template <typename Scalar>
void ViewScalarFilter<Scalar>::precalculate(int order, int mask)
{
    Hermes::Hermes2D::Quad2D* quad = Hermes::Hermes2D::Filter<Scalar>::quads[Hermes::Hermes2D::Function<Scalar>::cur_quad];
    int np = quad->get_num_points(order);
    node = Hermes::Hermes2D::Function<Scalar>::new_node(Hermes::Hermes2D::H2D_FN_DEFAULT, np);

    double **value = new double*[Util::scene()->problemInfo()->module()->number_of_solution()];
    double **dudx = new double*[Util::scene()->problemInfo()->module()->number_of_solution()];
    double **dudy = new double*[Util::scene()->problemInfo()->module()->number_of_solution()];

    for (int k = 0; k < Hermes::Hermes2D::Filter<Scalar>::num; k++)
    {
        Hermes::Hermes2D::Filter<Scalar>::sln[k]->set_quad_order(order, Hermes::Hermes2D::H2D_FN_VAL | Hermes::Hermes2D::H2D_FN_DX | Hermes::Hermes2D::H2D_FN_DY);
        Hermes::Hermes2D::Filter<Scalar>::sln[k]->get_dx_dy_values(dudx[k], dudy[k]);
        value[k] = Hermes::Hermes2D::Filter<Scalar>::sln[k]->get_fn_values();
    }

    Hermes::Hermes2D::Filter<Scalar>::update_refmap();

    double *x = Hermes::Hermes2D::MeshFunction<Scalar>::refmap->get_phys_x(order);
    double *y = Hermes::Hermes2D::MeshFunction<Scalar>::refmap->get_phys_y(order);
    Hermes::Hermes2D::Element *e = Hermes::Hermes2D::MeshFunction<Scalar>::refmap->get_active_element();

    SceneMaterial *material = Util::scene()->labels[atoi(Hermes::Hermes2D::MeshFunction<Scalar>::mesh->get_element_markers_conversion().get_user_marker(e->marker).c_str())]->material;
    parser->setParserVariables(material, NULL);

    for (int i = 0; i < np; i++)
    {
        px = x[i];
        py = y[i];

        for (int k = 0; k < Hermes::Hermes2D::Filter<Scalar>::num; k++)   //TODO PK proc se vsude musi davat ten kvalifikator u zdedenych polozek?
        {
            pvalue[k] = value[k][i];
            pdx[k] = dudx[k][i];
            pdy[k] = dudy[k][i];
        }

        // parse expression
        try
        {
            node->values[0][0][i] = parser->parser[0]->Eval();
        }
        catch (mu::Parser::exception_type &e)
        {
            std::cout << e.GetMsg() << endl;
        }
    }

    delete [] value;
    delete [] dudx;
    delete [] dudy;

    if (Hermes::Hermes2D::Function<Scalar>::nodes->present(order))
    {
        assert(Hermes::Hermes2D::Function<Scalar>::nodes->get(order) == Hermes::Hermes2D::Function<Scalar>::cur_node);
        ::free(Hermes::Hermes2D::Function<Scalar>::nodes->get(order));
    }
    Hermes::Hermes2D::Function<Scalar>::nodes->add(node, order);
    Hermes::Hermes2D::Function<Scalar>::cur_node = node;
}

template Hermes::vector<SolutionArray<double> *> solveSolutioArray(ProgressItemSolve *progressItemSolve,
                                                                   Hermes::vector<Hermes::Hermes2D::EssentialBCs<double> > bcs,
                                                                   WeakFormAgros<double> *wf);

template class SolutionAgros<double>;

