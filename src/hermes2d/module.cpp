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
#include "module_agros.h"
#include "problem.h"

#include "../weakform/src/weakform_factory.h"

#include "util.h"
#include "scene.h"
#include "scenebasic.h"
#include "scenemarkerdialog.h"
#include "scenelabel.h"
#include "scenesolution.h"
#include "sceneedge.h"
#include "hermes2d/solver.h"
#include "hermes2d/coupling.h"

#include "mesh/mesh_reader_h2d.h"

#include "newton_solver.h"
#include "picard_solver.h"

//#include "solver.h"

#include <dirent.h>

double actualTime;

std::map<std::string, std::string> availableModules()
{
    static std::map<std::string, std::string> modules;

    // read modules
    if (modules.size() == 0)
    {
        DIR *dp;
        if ((dp = opendir((datadir()+ MODULEROOT).toStdString().c_str())) == NULL)
            error("Modules dir '%s' doesn't exists", (datadir() + MODULEROOT).toStdString().c_str());

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
                rapidxml::file<> file_data((datadir().toStdString() + MODULEROOT.toStdString() + "/" + filename).c_str());

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

    // custom module
    // modules["custom"] = "Custom field";

    return modules;
}

std::map<std::string, std::string> availableAnalyses(std::string fieldId)
{
    std::map<std::string, std::string> analyses;

    // read module
    rapidxml::file<> file_data((datadir().toStdString() + MODULEROOT.toStdString() + "/" + fieldId + ".xml").c_str());

    // parse xml
    rapidxml::xml_document<> doc;
    doc.parse<0>(file_data.data());

    for (rapidxml::xml_node<> *analysis = doc.first_node("module")->first_node("general")->first_node("analyses")->first_node("analysis");
         analysis; analysis = analysis->next_sibling())
    {
        analyses[analysis->first_attribute("id")->value()] = analysis->first_attribute("name")->value();
    }

    return analyses;
}

template <typename Scalar>
WeakFormAgros<Scalar>::WeakFormAgros(Block* block) :
        Hermes::Hermes2D::WeakForm<Scalar>(block->numSolutions()), m_block(block)
{
}


template <typename Scalar>
Hermes::Hermes2D::Form<Scalar> *factoryForm(WFType type, const std::string &problemId,
                                                              const std::string &area, ParserFormExpression *form,
                                                              Marker* marker)
{
    if(type == WFType_MatVol)
        return factoryMatrixFormVol<Scalar>(problemId, form->i - 1, form->j - 1, area, form->sym, (SceneMaterial*) marker);
    else if(type == WFType_MatSurf)
        return factoryMatrixFormSurf<Scalar>(problemId, form->i - 1, form->j - 1, area, (SceneBoundary*) marker);
    else if(type == WFType_VecVol)
        return factoryVectorFormVol<Scalar>(problemId, form->i - 1, form->j - 1, area, (SceneMaterial*) marker);
    else if(type == WFType_VecSurf)
        return factoryVectorFormSurf<Scalar>(problemId, form->i - 1, form->j - 1, area, (SceneBoundary*) marker);
    else
        assert(0);
}

template <typename Scalar>
Hermes::Hermes2D::Form<Scalar> *factoryParserForm(WFType type, int i, int j, const std::string &area,
                                                  Hermes::Hermes2D::SymFlag sym, string expression, Marker* marker, Material* markerSecond)
{
//    cout << "factory form (" << i << ", " << j << "), area: " << area << " -> " << expression << ", marker " << marker->getName() <<  endl;
    if(type == WFType_MatVol)
        return new CustomParserMatrixFormVol<Scalar>(i, j,
                                                     area,
                                                     sym,
                                                     expression,
                                                     (SceneMaterial*) marker,
                                                     markerSecond);
    else if(type == WFType_MatSurf)
        return new CustomParserMatrixFormSurf<Scalar>(i, j,
                                                     area,
                                                     expression,
                                                     (SceneBoundary*) marker);
    else if(type == WFType_VecVol)
        return new CustomParserVectorFormVol<Scalar>(i, j,
                                                     area,
                                                     expression,
                                                     (SceneMaterial*) marker,
                                                     markerSecond);
    else if(type == WFType_VecSurf)
        return new CustomParserVectorFormSurf<Scalar>(i, j,
                                                     area,
                                                     expression,
                                                     (SceneBoundary*) marker);
    else
        assert(0);
}

template <typename Scalar>
void WeakFormAgros<Scalar>::addForm(WFType type, Hermes::Hermes2D::Form<Scalar> *form)
{
//    cout << "pridavam vektorovou formu, i: " << form->i << ", areas: "; /* ", j: " << form->j <<*/
//    for(int i = 0; i < form->areas.size(); i++)
//        cout << form->areas.at(i) << ", ";
//    cout << endl;
    if(type == WFType_MatVol)
        add_matrix_form((Hermes::Hermes2D::MatrixFormVol<Scalar>*) form);
    else if(type == WFType_MatSurf)
        add_matrix_form_surf((Hermes::Hermes2D::MatrixFormSurf<Scalar>*) form);
    else if(type == WFType_VecVol)
        add_vector_form((Hermes::Hermes2D::VectorFormVol<Scalar>*) form);
    else if(type == WFType_VecSurf)
        add_vector_form_surf((Hermes::Hermes2D::VectorFormSurf<Scalar>*) form);
    else
        assert(0);
}

template <typename Scalar>
void WeakFormAgros<Scalar>::registerForm(WFType type, Field* field, string area, ParserFormExpression *form, int offsetI, int offsetJ,
                                         Marker* marker, SceneMaterial* marker_second, CouplingInfo* couplingInfo)
{
    //TODO zatim jen interpretovane formy. Pak se musi nejak rozlisit, jestli je registrovana forma z modulu nebo ze zdruzeni
//    string problemId = fieldInfo->fieldId().toStdString() + "_" +
//            analysisTypeToStringKey(fieldInfo->module()->get_analysis_type()).toStdString()  + "_" +
//            coordinateTypeToStringKey(fieldInfo->module()->get_coordinate_type()).toStdString();

    Hermes::Hermes2D::Form<Scalar>* custom_form = NULL;

    // compiled form
//    if (fieldInfo->weakFormsType == WeakFormsType_Compiled)
//    {
//        assert(0);
//        //custom_form = factoryForm<Scalar>(type, problemId, area, form, marker);
//    }

//    if (!custom_form && fieldInfo->weakFormsType == WeakFormsType_Compiled)
//        qDebug() << "Cannot find compiled VectorFormVol().";

    // interpreted form
//    if (!custom_form || fieldInfo->weakFormsType == WeakFormsType_Interpreted)
//    {
        custom_form = factoryParserForm<Scalar>(type, form->i - 1 + offsetI, form->j - 1 + offsetJ, area, form->sym, form->expression, marker, marker_second);
//    }

    //Decide what solution to push, implicitly none
    FieldSolutionID solutionID(NULL, 0, 0, SolutionType_NonExisting);

    // weak coupling, push solutions
    if(marker_second && couplingInfo)
    {
        // TODO at the present moment, it is impossible to have more sources !
        assert(field->m_couplingSources.size() <= 1);        

        solutionID = Util::solutionStore()->lastTimeAndAdaptiveSolution(couplingInfo->sourceField(), SolutionType_Finer);
        assert(solutionID.group->module()->number_of_solution() <= maxSourceFieldComponents);
    }
    else{
        if (field->fieldInfo()->analysisType() == AnalysisType_Transient)
        {
            solutionID = Util::solutionStore()->lastTimeAndAdaptiveSolution(field->fieldInfo(), SolutionType_Finer);
        }
    }

    if(solutionID.solutionType != SolutionType_NonExisting)
    {
        for(int comp = 0; comp < solutionID.group->module()->number_of_solution(); comp++)
        {
            custom_form->ext.push_back(Util::solutionStore()->solution(solutionID, comp).sln.get());
        }
    }

    if (custom_form)
    {
        addForm(type, custom_form);
    }

}


template <typename Scalar>
void WeakFormAgros<Scalar>::registerForms()
{
    qDebug() << "registerForms";

    foreach(Field* field, m_block->m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();

        // boundary conditions
        for (int edgeNum = 0; edgeNum<Util::scene()->edges->count(); edgeNum++)
        {
            SceneBoundary *boundary = Util::scene()->edges->at(edgeNum)->getMarker(fieldInfo);

            if (boundary && boundary != Util::scene()->boundaries->getNone(fieldInfo))
            {
                Hermes::Module::BoundaryType *boundary_type = fieldInfo->module()->get_boundary_type(boundary->getType());

                for (Hermes::vector<ParserFormExpression *>::iterator it = boundary_type->weakform_matrix_surface.begin();
                     it < boundary_type->weakform_matrix_surface.end(); ++it)
                {
                    registerForm(WFType_MatSurf, field, QString::number(edgeNum + 1).toStdString(), (ParserFormExpression *) *it,
                                 m_block->offset(field), m_block->offset(field), boundary);
                }

                for (Hermes::vector<ParserFormExpression *>::iterator it = boundary_type->weakform_vector_surface.begin();
                     it < boundary_type->weakform_vector_surface.end(); ++it)
                {
                    registerForm(WFType_VecSurf, field, QString::number(edgeNum + 1).toStdString(), (ParserFormExpression *) *it,
                                 m_block->offset(field), m_block->offset(field), boundary);
                }
            }
        }

        // materials
        for (int labelNum = 0; labelNum<Util::scene()->labels->count(); labelNum++)
        {
            SceneMaterial *material = Util::scene()->labels->at(labelNum)->getMarker(fieldInfo);
            cout << "registering label " << labelNum << ", material " << material << ", name " << material->getName() << ", marker for hermes " << labelNum + 1 << endl;

            if (material && material != Util::scene()->materials->getNone(fieldInfo))
            {
                for (Hermes::vector<ParserFormExpression *>::iterator it = fieldInfo->module()->weakform_matrix_volume.begin();
                     it < fieldInfo->module()->weakform_matrix_volume.end(); ++it)
                {
                    registerForm(WFType_MatVol, field, QString::number(labelNum + 1).toStdString(), (ParserFormExpression *) *it,
                                 m_block->offset(field), m_block->offset(field), material);

                }

                for (Hermes::vector<ParserFormExpression *>::iterator it = fieldInfo->module()->weakform_vector_volume.begin();
                     it < fieldInfo->module()->weakform_vector_volume.end(); ++it)
                {
                    registerForm(WFType_VecVol, field, QString::number(labelNum + 1).toStdString(), (ParserFormExpression *) *it,
                                 m_block->offset(field), m_block->offset(field), material);
                }


                foreach(CouplingInfo* couplingInfo, field->m_couplingSources)
                {
                     for (Hermes::vector<ParserFormExpression *>::iterator it = couplingInfo->coupling()->weakform_vector_volume.begin();
                          it < couplingInfo->coupling()->weakform_vector_volume.end(); ++it)
                    {
                         registerForm(WFType_VecVol, field, QString::number(labelNum + 1).toStdString(), (ParserFormExpression *) *it,
                                      m_block->offset(field), m_block->offset(field), material,
                                      Util::scene()->labels->at(labelNum)->getMarker(couplingInfo->sourceField()), couplingInfo);
                    }
                }
            }
        }
    }


    // hard coupling
    foreach(CouplingInfo* couplingInfo, m_block->m_couplings)
    {
        assert(couplingInfo->isHard());
        Coupling* coupling = couplingInfo->coupling();
        Field* sourceField = m_block->field(couplingInfo->sourceField());
        Field* targetField = m_block->field(couplingInfo->targetField());


        for (int labelNum = 0; labelNum<Util::scene()->labels->count(); labelNum++)
        {
            SceneMaterial *sourceMaterial = Util::scene()->labels->at(labelNum)->getMarker(sourceField->fieldInfo());
            SceneMaterial *targetMaterial = Util::scene()->labels->at(labelNum)->getMarker(targetField->fieldInfo());

            if (sourceMaterial && (sourceMaterial != Util::scene()->materials->getNone(sourceField->fieldInfo()))
                    && targetMaterial && (targetMaterial != Util::scene()->materials->getNone(targetField->fieldInfo())))
            {

                for (Hermes::vector<ParserFormExpression *>::iterator it = coupling->weakform_matrix_volume.begin();
                     it < coupling->weakform_matrix_volume.end(); ++it)
                {
                    registerForm(WFType_MatVol, sourceField, QString::number(labelNum).toStdString(), (ParserFormExpression *) *it,
                                 m_block->offset(targetField) - sourceField->fieldInfo()->module()->number_of_solution(), m_block->offset(sourceField),
                                 sourceMaterial, targetMaterial);
                }

                for (Hermes::vector<ParserFormExpression *>::iterator it = coupling->weakform_vector_volume.begin();
                     it < coupling->weakform_vector_volume.end(); ++it)
                {
                    registerForm(WFType_VecVol, sourceField, QString::number(labelNum).toStdString(), (ParserFormExpression *) *it,
                                 m_block->offset(targetField) - sourceField->fieldInfo()->module()->number_of_solution(), m_block->offset(sourceField),
                                 sourceMaterial, targetMaterial);

                }
            }
        }
    }
}

// ***********************************************************************************************

Hermes::Module::LocalVariable::Expression::Expression(rapidxml::xml_node<> *node, CoordinateType problemType)
{
    if (problemType == CoordinateType_Planar)
    {
        if (node->first_attribute("planar"))
            scalar = node->first_attribute("planar")->value();
        if (node->first_attribute("planar_x"))
            comp_x = node->first_attribute("planar_x")->value();
        if (node->first_attribute("planar_y"))
            comp_y = node->first_attribute("planar_y")->value();
    }
    else
    {
        if (node->first_attribute("axi"))
            scalar = node->first_attribute("axi")->value();
        if (node->first_attribute("axi_r"))
            comp_x = node->first_attribute("axi_r")->value();
        if (node->first_attribute("axi_z"))
            comp_y = node->first_attribute("axi_z")->value();
    }
}

// ***********************************************************************************************

Hermes::Module::LocalVariable::LocalVariable(rapidxml::xml_node<> *node,
                                             CoordinateType problemType, AnalysisType analysisType)
{
    id = node->first_attribute("id")->value();
    name = node->first_attribute("name")->value();
    shortname = node->first_attribute("shortname")->value();
    shortname_html = (node->first_attribute("shortname_html")) ? node->first_attribute("shortname_html")->value() : shortname;
    unit = node->first_attribute("unit")->value();
    unit_html = (node->first_attribute("unit_html")) ? node->first_attribute("unit_html")->value() : unit;

    is_scalar = (std::string(node->first_attribute("type")->value()) == "scalar");

    for (rapidxml::xml_node<> *expr = node->first_node("expression");
         expr; expr = expr->next_sibling())
        if (expr->first_attribute("analysistype")->value() == Hermes::analysis_type_tostring(analysisType))
            expression = Expression(expr, problemType);
}

// ***********************************************************************************************

Hermes::Module::Integral::Expression::Expression(rapidxml::xml_node<> *node, CoordinateType problemType)
{
    if (node)
    {
        if (problemType == CoordinateType_Planar)
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
    shortname = node->first_attribute("shortname")->value();
    if (node->first_attribute("default"))
        default_value = atoi(node->first_attribute("default")->value());
    else
        default_value = 0.0;
}

Hermes::Module::MaterialTypeVariable::MaterialTypeVariable(std::string id, std::string shortname,
                                                           double default_value)
{
    this->id = id;
    this->shortname = shortname;
    this->default_value = default_value;
}

// ***********************************************************************************************

Hermes::Module::BoundaryType::BoundaryType(Hermes::vector<BoundaryTypeVariable> boundary_type_variables,
                                           rapidxml::xml_node<> *node,
                                           CoordinateType problem_type)
{
    id = node->first_attribute("id")->value();
    name = QObject::tr(node->first_attribute("name")->value()).toStdString();

    // variables
    for (rapidxml::xml_node<> *variable = node->first_node("quantity");
         variable; variable = variable->next_sibling())
        if (std::string(variable->name()) == "quantity")
            for (Hermes::vector<Hermes::Module::BoundaryTypeVariable>::iterator it = boundary_type_variables.begin();
                 it < boundary_type_variables.end(); ++it )
            {
                Hermes::Module::BoundaryTypeVariable old = (Hermes::Module::BoundaryTypeVariable) *it;

                if (old.id == variable->first_attribute("id")->value())
                {
                    Hermes::Module::BoundaryTypeVariable *var = new Hermes::Module::BoundaryTypeVariable(
                                old.id, old.shortname, old.default_value);

                    variables.push_back(var);
                }
            }

    // weakform
    for (rapidxml::xml_node<> *node_matrix = node->first_node("matrix");
         node_matrix; node_matrix = node_matrix->next_sibling())
        if (std::string(node_matrix->name()) == "matrix")
            weakform_matrix_surface.push_back(new ParserFormExpression(node_matrix, problem_type));

    for (rapidxml::xml_node<> *node_vector = node->first_node("vector");
         node_vector; node_vector = node_vector->next_sibling())
        if (std::string(node_vector->name()) == "vector")
            weakform_vector_surface.push_back(new ParserFormExpression(node_vector, problem_type));

    // essential
    for (rapidxml::xml_node<> *node_essential = node->first_node("essential");
         node_essential; node_essential = node_essential->next_sibling())
        if (std::string(node_essential->name()) == "essential")
            essential.push_back(new ParserFormEssential(node_essential, problem_type));
}

Hermes::Module::BoundaryTypeVariable::BoundaryTypeVariable(rapidxml::xml_node<> *node)
{
    id = node->first_attribute("id")->value();
    shortname = node->first_attribute("shortname")->value();
    if (node->first_attribute("default"))
        default_value = atoi(node->first_attribute("default")->value());
    else
        default_value = 0.0;
}

Hermes::Module::BoundaryTypeVariable::BoundaryTypeVariable(std::string id, std::string shortname,
                                                           double default_value)
{
    this->id = id;
    this->shortname = shortname;
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
    for (Hermes::vector<ParserFormExpression *>::iterator it = weakform_matrix_surface.begin(); it < weakform_matrix_surface.end(); ++it)
        delete *it;
    weakform_matrix_surface.clear();

    for (Hermes::vector<ParserFormExpression *>::iterator it = weakform_vector_surface.begin(); it < weakform_vector_surface.end(); ++it)
        delete *it;
    weakform_vector_surface.clear();

    for (Hermes::vector<ParserFormExpression *>::iterator it = weakform_vector_surface.begin(); it < weakform_vector_surface.end(); ++it)
        delete *it;
    weakform_vector_surface.clear();
}

// ***********************************************************************************************

Hermes::Module::Integral::Integral(rapidxml::xml_node<> *node, CoordinateType coordinateType, AnalysisType analysisType)
{
    id = node->first_attribute("id")->value();
    name = QObject::tr(node->first_attribute("name")->value()).toStdString();
    shortname = node->first_attribute("shortname")->value();
    shortname_html = (node->first_attribute("shortname_html")) ? node->first_attribute("shortname_html")->value() : shortname;
    unit = node->first_attribute("unit")->value();
    unit_html = (node->first_attribute("unit_html")) ? node->first_attribute("unit_html")->value() : unit;

    for (rapidxml::xml_node<> *expr = node->first_node("expression");
         expr; expr = expr->next_sibling())
        if (expr->first_attribute("analysistype")->value() == Hermes::analysis_type_tostring(analysisType))
            expression = Expression(expr, coordinateType);
}

// ***********************************************************************************************

// dialog row UI
Hermes::Module::DialogUI::Row::Row(rapidxml::xml_node<> *quantity)
{
    id = quantity->first_attribute("id")->value();

    nonlin = (quantity->first_attribute("nonlin")) ? atoi(quantity->first_attribute("nonlin")->value()) : false;
    timedep = (quantity->first_attribute("timedep")) ? atoi(quantity->first_attribute("timedep")->value()) : false;

    shortname = quantity->first_attribute("shortname")->value();
    shortname_html = (quantity->first_attribute("shortname_html")) ? quantity->first_attribute("shortname_html")->value() : "";
    name = quantity->first_attribute("name")->value();

    unit = quantity->first_attribute("unit")->value();
    unit_html = (quantity->first_attribute("unit_html")) ? quantity->first_attribute("unit_html")->value() : "";
    unit_latex = (quantity->first_attribute("unit_latex")) ? quantity->first_attribute("unit_latex")->value() : "";

    default_value = (quantity->first_attribute("default")) ? atof(quantity->first_attribute("default")->value()) : 0.0;
    condition = (quantity->first_attribute("condition")) ? quantity->first_attribute("condition")->value() : "";
}

// dialog UI
Hermes::Module::DialogUI::DialogUI(rapidxml::xml_node<> *node)
{
    for (rapidxml::xml_node<> *group = node->first_node("group");
         group; group = group->next_sibling())
    {
        // group name
        std::string name = (group->first_attribute("name")) ? group->first_attribute("name")->value() : "";

        Hermes::vector<DialogUI::Row> materials;
        for (rapidxml::xml_node<> *quantity = group->first_node("quantity");
             quantity; quantity = quantity->next_sibling())
            // append material
            materials.push_back(DialogUI::Row(quantity));

        groups[name] = materials;
    }
}

void Hermes::Module::DialogUI::clear()
{
    groups.clear();
}

// ***********************************************************************************************

Hermes::Module::Module::Module(CoordinateType problemType, AnalysisType analysisType)
{
    m_coordinateType = problemType;
    m_analysisType = analysisType;

    clear();
}

Hermes::Module::Module::~Module()
{
    clear();
}

void Hermes::Module::Module::read(std::string filename)
{
    std::cout << "reading module: " << filename << std::endl << std::flush;

    clear();

    if (ifstream(filename.c_str()))
    {
        // save current locale
        char *plocale = setlocale (LC_NUMERIC, "");
        setlocale (LC_NUMERIC, "C");

        rapidxml::file<> file_data(filename.c_str());

        // parse document
        rapidxml::xml_document<> doc;
        doc.parse<0>(file_data.data());

        // problem
        rapidxml::xml_node<> *general = doc.first_node("module")->first_node("general");
        fieldid = general->first_attribute("id")->value();
        name = QObject::tr(general->first_attribute("name")->value()).toStdString(); // FIXME - Qt dependence
        deformed_shape = general->first_attribute("deformed_shape") ? atoi(general->first_attribute("deformed_shape")->value()) : 0;
        description = general->first_node("description")->value();

        // analyses
        steady_state_solutions = 0;
        harmonic_solutions = 0;
        transient_solutions = 0;
        for (rapidxml::xml_node<> *analysis = doc.first_node("module")->first_node("general")->first_node("analyses")->first_node("analysis");
             analysis; analysis = analysis->next_sibling())
        {
            // FIXME
            analyses[analysis->first_attribute("id")->value()] = analysis->first_attribute("type")->value();

            if (std::string(analysis->first_attribute("type")->value()) == QString("steadystate").toStdString())
                steady_state_solutions = atoi(analysis->first_attribute("solutions")->value());

            if (std::string(analysis->first_attribute("type")->value()) == QString("harmonic").toStdString())
                harmonic_solutions = atoi(analysis->first_attribute("solutions")->value());

            if (std::string(analysis->first_attribute("type")->value()) == QString("transient").toStdString())
                transient_solutions = atoi(analysis->first_attribute("solutions")->value());
        }

        // constants
        for (rapidxml::xml_node<> *constant = doc.first_node("module")->first_node("constants")->first_node("constant");
             constant; constant = constant->next_sibling())
            constants[constant->first_attribute("id")->value()] = atof(constant->first_attribute("value")->value());

        // macros
        for (rapidxml::xml_node<> *macro = doc.first_node("module")->first_node("macros")->first_node("macro");
             macro; macro = macro->next_sibling())
            macros[macro->first_attribute("id")->value()] = macro->first_attribute("expression")->value();

        // surface weakforms
        Hermes::vector<Hermes::Module::BoundaryTypeVariable> boundary_type_variables_tmp;
        for (rapidxml::xml_node<> *quantity = doc.first_node("module")->first_node("surface")->first_node("quantity");
             quantity; quantity = quantity->next_sibling())
            if (std::string(quantity->name()) == "quantity")
                boundary_type_variables_tmp.push_back(Hermes::Module::BoundaryTypeVariable(quantity));

        for (rapidxml::xml_node<> *weakform = doc.first_node("module")->first_node("surface")->first_node("weakforms")->first_node("weakform");
             weakform; weakform = weakform->next_sibling())
        {
            if (std::string(weakform->name()) == "weakform" && weakform->first_attribute("analysistype")->value() == analysis_type_tostring(m_analysisType))
                for (rapidxml::xml_node<> *boundary = weakform->first_node("boundary");
                     boundary; boundary = boundary->next_sibling())
                    boundary_types.push_back(new Hermes::Module::BoundaryType(boundary_type_variables_tmp, boundary, m_coordinateType));

            // default
            boundary_type_default = get_boundary_type(weakform->first_attribute("default")->value());
        }
        boundary_type_variables_tmp.clear();

        // volumetric weakforms
        Hermes::vector<Hermes::Module::MaterialTypeVariable> material_type_variables_tmp;
        for (rapidxml::xml_node<> *quantity = doc.first_node("module")->first_node("volume")->first_node("quantity");
             quantity; quantity = quantity->next_sibling())
            if (std::string(quantity->name()) == "quantity")
                material_type_variables_tmp.push_back(Hermes::Module::MaterialTypeVariable(quantity));

        for (rapidxml::xml_node<> *weakform = doc.first_node("module")->first_node("volume")->first_node("weakforms")->first_node("weakform");
             weakform; weakform = weakform->next_sibling())
        {
            if (weakform->first_attribute("analysistype")->value() == analysis_type_tostring(m_analysisType))
            {
                // quantities
                for (rapidxml::xml_node<> *quantity = weakform->first_node("quantity");
                     quantity; quantity = quantity->next_sibling())
                {
                    if (std::string(quantity->name()) == "quantity")
                        for (Hermes::vector<Hermes::Module::MaterialTypeVariable>::iterator it = material_type_variables_tmp.begin();
                             it < material_type_variables_tmp.end(); ++it )
                        {
                            Hermes::Module::MaterialTypeVariable old = (Hermes::Module::MaterialTypeVariable) *it;
                            if (old.id == quantity->first_attribute("id")->value())
                            {
                                Hermes::Module::MaterialTypeVariable *var = new Hermes::Module::MaterialTypeVariable(
                                            old.id, old.shortname, old.default_value);
                                material_type_variables.push_back(var);
                            }
                        }
                }
                material_type_variables_tmp.clear();

                // weakforms
                for (rapidxml::xml_node<> *matrix = weakform->first_node("matrix");
                     matrix; matrix = matrix->next_sibling())
                {
                    if (std::string(matrix->name()) == "matrix")
                        weakform_matrix_volume.push_back(new ParserFormExpression(matrix, m_coordinateType));
                }

                for (rapidxml::xml_node<> *vector = weakform->first_node("vector");
                     vector; vector = vector->next_sibling())
                    if (std::string(vector->name()) == "vector")
                        weakform_vector_volume.push_back(new ParserFormExpression(vector, m_coordinateType));
            }
        }

        // local variables
        for (rapidxml::xml_node<> *localvariable = doc.first_node("module")->first_node("postprocessor")->first_node("localvariables")->first_node("localvariable");
             localvariable; localvariable = localvariable->next_sibling())
        {
            // HACK
            for (rapidxml::xml_node<> *expr = localvariable->first_node("expression");
                 expr; expr = expr->next_sibling())
                if (expr->first_attribute("analysistype")->value() == Hermes::analysis_type_tostring(m_analysisType))
                {
                    Hermes::Module::LocalVariable *var = new Hermes::Module::LocalVariable(localvariable, m_coordinateType, m_analysisType);
                    variables.push_back(var);

                    // HACK - local point
                    local_point.push_back(var);
                    // HACK - scalar view
                    view_scalar_variables.push_back(var);
                    // HACK - vector view
                    if (!var->is_scalar)
                        view_vector_variables.push_back(var);
                }
        }

        // custom local variable
        //TODO
        //Hermes::Module::LocalVariable *customLocalVariable = new Hermes::Module::LocalVariable("custom", "Custom", "custom", "-");
        //customLocalVariable->expression.scalar = "value1";
        //view_scalar_variables.push_back(customLocalVariable);

        // scalar variables
        rapidxml::xml_node<> *view = doc.first_node("module")->first_node("postprocessor")->first_node("view");

        // scalar variables default
        rapidxml::xml_node<> *view_scalar = view->first_node("scalar");
        for (rapidxml::xml_node<> *view_scalar_default = view_scalar->first_node("default");
             view_scalar_default; view_scalar_default = view_scalar_default->next_sibling())
            if (view_scalar_default->first_attribute("analysistype")->value() == analysis_type_tostring(m_analysisType))
                view_default_scalar_variable = get_variable(view_scalar_default->first_attribute("id")->value());

        // vector variables default
        rapidxml::xml_node<> *view_vector = view->first_node("vector");
        for (rapidxml::xml_node<> *view_vector_default = view_vector->first_node("default");
             view_vector_default; view_vector_default = view_vector_default->next_sibling())
            if (view_vector_default->first_attribute("analysistype")->value() == analysis_type_tostring(m_analysisType))
                view_default_vector_variable = get_variable(view_vector_default->first_attribute("id")->value());

        // volume integral
        for (rapidxml::xml_node<> *volumeintegral = doc.first_node("module")->first_node("postprocessor")->first_node("volumeintegrals")->first_node("volumeintegral");
             volumeintegral; volumeintegral = volumeintegral->next_sibling())
            volume_integral.push_back(new Hermes::Module::Integral(volumeintegral, m_coordinateType, m_analysisType));

        // surface integral
        for (rapidxml::xml_node<> *surfaceintegral = doc.first_node("module")->first_node("postprocessor")->first_node("surfaceintegrals")->first_node("surfaceintegral");
             surfaceintegral; surfaceintegral = surfaceintegral->next_sibling())
            surface_integral.push_back(new Hermes::Module::Integral(surfaceintegral, m_coordinateType, m_analysisType));

        // preprocessor
        rapidxml::xml_node<> *materialui = doc.first_node("module")->first_node("preprocessor")->first_node("volume");
        material_ui = Hermes::Module::DialogUI(materialui);
        rapidxml::xml_node<> *boundaryui = doc.first_node("module")->first_node("preprocessor")->first_node("surface");
        boundary_ui = Hermes::Module::DialogUI(boundaryui);

        // set system locale
        setlocale(LC_NUMERIC, plocale);
    }
}

void Hermes::Module::Module::clear()
{
    // general information
    fieldid = "";
    name = "";
    description = "";

    // analyses
    analyses.clear();

    // constants
    constants.clear();

    // macros
    macros.clear();

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
    for (Hermes::vector<ParserFormExpression *>::iterator it = weakform_matrix_volume.begin(); it < weakform_matrix_volume.end(); ++it)
        delete *it;
    weakform_matrix_volume.clear();

    for (Hermes::vector<ParserFormExpression *>::iterator it = weakform_vector_volume.begin(); it < weakform_vector_volume.end(); ++it)
        delete *it;
    weakform_vector_volume.clear();

    material_ui.clear();
    boundary_ui.clear();
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

mu::Parser *Hermes::Module::Module::get_parser(FieldInfo* fieldInfo)
{
    mu::Parser *parser = new mu::Parser();

    // pi
    parser->DefineConst("PI", M_PI);

    // frequency
    parser->DefineConst("f", fieldInfo->frequency());

    // timestep
    parser->DefineConst("dt", fieldInfo->timeStep().number());

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

ViewScalarFilter<double> *Hermes::Module::Module::view_scalar_filter(Hermes::Module::LocalVariable *physicFieldVariable,
                                                                     PhysicFieldVariableComp physicFieldVariableComp)
{
    Hermes::vector<Hermes::Hermes2D::MeshFunction<double> *> sln;
    for (int k = 0; k < number_of_solution(); k++)
    {
        sln.push_back(Util::scene()->activeSceneSolution()->sln(k));
    }
    return new ViewScalarFilter<double>(Util::scene()->activeSceneSolution()->fieldInfo(),
                                        sln,
                                        get_expression(physicFieldVariable, physicFieldVariableComp));
}

bool Hermes::Module::Module::solve_init_variables()
{
    //TODO moved to problem, remove this method
    assert(0); //TODO
    //    // transient
    //    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
    //    {
    //        if (!m_fieldInfo->timeStep.evaluate()) return false;
    //        if (!m_fieldInfo->timeTotal.evaluate()) return false;
    //        if (!m_fieldInfo->initialCondition.evaluate()) return false;
    //    }

    //    // edge markers
    //    for (int i = 1; i<Util::scene()->boundaries.count(); i++)
    //    {
    //        SceneBoundary *boundary = Util::scene()->boundaries[i];

    //        // evaluate script
    //        for (std::map<std::string, Value>::iterator it = boundary->getBoundary("TODO")->getValues().begin(); it != boundary->getBoundary("TODO")->getValues().end(); ++it)
    //            if (!it->second.evaluate()) return false;
    //    }

    //    // label markers
    //    for (int i = 1; i<Util::scene()->materials.count(); i++)
    //    {
    //        SceneMaterial *material = Util::scene()->materials[i];

    //        // evaluate script
    //        for (std::map<std::string, Value>::iterator it = material->values.begin(); it != material->values.end(); ++it)
    //            if (!it->second.evaluate()) return false;
    //    }


    //    return true;
}

Hermes::vector<SolutionArray<double> *> Hermes::Module::Module::solve(ProgressItemSolve *progressItemSolve)  //TODO PK <double>
{
    assert(0);
//    if (!solve_init_variables())
//        return Hermes::vector<SolutionArray<double> *>(); //TODO PK <double>

//    WeakFormAgros<double> wf(number_of_solution()); //TODO PK <double>

//    SolverAgros<double> solutionAgros(progressItemSolve, &wf);

//    Hermes::vector<SolutionArray<double> *> solutionArrayList = solutionAgros.solve(); //TODO PK <double>
//    return solutionArrayList;
}

Hermes::vector<SolutionArray<double> *> Hermes::Module::Module::solveAdaptiveStep(ProgressItemSolve *progressItemSolve)  //TODO PK <double>
{
    assert(0); //TODO
    //    if (!solve_init_variables())
    //        return Hermes::vector<SolutionArray<double> *>(); //TODO PK <double>

    //    WeakFormAgros<double> wf(number_of_solution()); //TODO PK <double>

    //    SolverAgros<double> solutionAgros(progressItemSolve, &wf);

    //    Hermes::vector<const Hermes::Hermes2D::Space<double> *> space;
    //    Hermes::vector<Hermes::Hermes2D::Solution<double> *> solution;

    //    for (int i = 0; i < Util::scene()->fiedInfo(id)->module()->number_of_solution(); i++)
    //    {
    //        int n = i + (Util::scene()->sceneSolution()->timeStepCount() - 1) * m_fieldInfo->module()->number_of_solution();
    //        // space
    //        space.push_back(Util::scene()->sceneSolution()->sln(n)->get_space());
    //        // solution
    //        solution.push_back(Util::scene()->sceneSolution()->sln(n));
    //    }

    //    Hermes::vector<SolutionArray<double> *> solutionArrayList = solutionAgros.solve(space, solution);
    //    return solutionArrayList;
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

    Hermes::Hermes2D::Mesh mesh;
    Hermes::Hermes2D::MeshReaderH2D meshloader;

    std::ofstream outputFile((tempProblemDir().toStdString() + "/dummy.mesh").c_str(), fstream::out);
    outputFile << os.str();
    outputFile.close();

    meshloader.load((tempProblemDir().toStdString() + "/dummy.mesh").c_str(), &mesh);

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> readMeshesFromFile(const QString &fileName)
{
    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    // load the mesh file
    Hermes::Hermes2D::MeshReaderH2DXML meshloader;
    Hermes::vector<Hermes::Hermes2D::Mesh*> meshes;
    QMap<FieldInfo*, Hermes::Hermes2D::Mesh*> meshesMap;
    foreach(FieldInfo* fieldInfo, Util::scene()->fieldInfos())
    {
        Hermes::Hermes2D::Mesh *mesh = new Hermes::Hermes2D::Mesh();
        meshes.push_back(mesh);
        meshesMap[fieldInfo] = mesh;
    }

    meshloader.load(fileName.toStdString().c_str(), meshes);

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return meshesMap;
}

void writeMeshFromFile(const QString &fileName, Hermes::Hermes2D::Mesh *mesh)
{
    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    Hermes::Hermes2D::MeshReaderH2D meshloader;
    meshloader.save(fileName.toStdString().c_str(), mesh);

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

void refineMesh(FieldInfo *fieldInfo, Hermes::Hermes2D::Mesh *mesh, bool refineGlobal, bool refineTowardsEdge)
{
    // refine mesh - global
    if (refineGlobal)
        for (int i = 0; i < fieldInfo->numberOfRefinements; i++)
            mesh->refine_all_elements(0);

    // refine mesh - boundary
    int i = 0;
    if (refineTowardsEdge)
        foreach (SceneEdge *edge, Util::scene()->edges->items())
        {
            if (edge->refineTowardsEdge > 0)
                mesh->refine_towards_boundary(QString::number(((edge->getMarker(fieldInfo)
                                                                != SceneBoundaryContainer::getNone(fieldInfo)) ? i + 1 : -i)).toStdString(),
                                              edge->refineTowardsEdge);

            i++;
        }
}

// return geom type
Hermes::Hermes2D::GeomType convertProblemType(CoordinateType problemType)
{
    return (problemType == CoordinateType_Planar ? Hermes::Hermes2D::HERMES_PLANAR : Hermes::Hermes2D::HERMES_AXISYM_Y);
}

// *********************************************************************************************************************************************

Parser::Parser(FieldInfo *fieldInfo) : m_fieldInfo(fieldInfo)
{

}

Parser::~Parser()
{
    // delete parser
    for (Hermes::vector<mu::Parser *>::iterator it = parser.begin(); it < parser.end(); ++it)
        delete *it;

    parser.clear();
}

void Parser::setParserVariables(Material* material, Boundary *boundary, double value, double dx, double dy)
{
    Hermes::vector<Material *> materials;
    if(material)
        materials.push_back(material);
    setParserVariables(materials, boundary, value, dx, dy);
}

void Parser::setParserVariables(Hermes::vector<Material *> materialMarkers, Boundary *boundary, double value, double dx, double dy)
{    
    //TODO zkontrolovat volani value, proc u boundary neni derivace, ...
    if (materialMarkers.size())
    {
        for(Hermes::vector<Material *>::iterator markerIter = materialMarkers.begin(); markerIter != materialMarkers.end(); ++markerIter)
        {
            Hermes::vector<Hermes::Module::MaterialTypeVariable *> materials = (*markerIter)->getFieldInfo()->module()->material_type_variables;
            for (Hermes::vector<Hermes::Module::MaterialTypeVariable *>::iterator it = materials.begin(); it < materials.end(); ++it)
            {
                Hermes::Module::MaterialTypeVariable *variable = ((Hermes::Module::MaterialTypeVariable *) *it);
                parser_variables[variable->shortname] = (*markerIter)->getValue(variable->id).value(value);
                parser_variables["d" + variable->shortname] = (*markerIter)->getValue(variable->id).derivative(value);
            }
        }
    }

    if (boundary)
    {
        Hermes::Module::BoundaryType *boundary_type = m_fieldInfo->module()->get_boundary_type(boundary->getType());
        for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin(); it < boundary_type->variables.end(); ++it)
        {
            Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);
            parser_variables[variable->shortname] = boundary->getValue(variable->id).value(0.0);
            //cout << "setParserVariables: shortname: " << variable->shortname << ", id: " << variable->id << ", value: " << boundary->getValue(variable->id).value(0.0) << endl;
        }
    }

}

void Parser::initParserMaterialVariables()
{
    Hermes::vector<Hermes::Module::MaterialTypeVariable *> materials = m_fieldInfo->module()->material_type_variables;
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

//void Parser::initParserBoundaryVariables(Boundary *boundary)
//{
//    Hermes::Module::BoundaryType *boundary_type = fieldInfo->module()->get_boundary_type(boundary->getType());
//    for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin(); it < boundary_type->variables.end(); ++it)
//    {
//        Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);
//        parser_variables[variable->shortname] = 0.0;
//    }

//    // set material variables
//    for (std::map<std::string, double>::iterator itv = parser_variables.begin(); itv != parser_variables.end(); ++itv)
//        for (Hermes::vector<mu::Parser *>::iterator it = parser.begin(); it < parser.end(); ++it)
//            ((mu::Parser *) *it)->DefineVar(itv->first, &itv->second);
//}

// *********************************************************************************************************************************************

template <typename Scalar>
ViewScalarFilter<Scalar>::ViewScalarFilter(FieldInfo *fieldInfo,
                                           Hermes::vector<Hermes::Hermes2D::MeshFunction<Scalar> *> sln,
                                           std::string expression)
    : Hermes::Hermes2D::Filter<Scalar>(sln), m_fieldInfo(fieldInfo)
{
    parser = new Parser(fieldInfo);
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
    mu::Parser *pars = m_fieldInfo->module()->get_parser(m_fieldInfo);

    pars->SetExpr(expression);

    pars->DefineVar(m_fieldInfo->labelX().toLower().toStdString(), &px);
    pars->DefineVar(m_fieldInfo->labelY().toLower().toStdString(), &py);

    pvalue = new double[Hermes::Hermes2D::Filter<Scalar>::num];
    pdx = new double[Hermes::Hermes2D::Filter<Scalar>::num];
    pdy = new double[Hermes::Hermes2D::Filter<Scalar>::num];

    for (int k = 0; k < Hermes::Hermes2D::Filter<Scalar>::num; k++)
    {
        std::stringstream number;
        number << (k+1);

        pars->DefineVar("value" + number.str(), &pvalue[k]);
        pars->DefineVar("d" + m_fieldInfo->labelX().toLower().toStdString() + number.str(), &pdx[k]);
        pars->DefineVar("d" + m_fieldInfo->labelY().toLower().toStdString() + number.str(), &pdy[k]);
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
    bool isLinear = (m_fieldInfo->linearityType == LinearityType_Linear);

    Hermes::Hermes2D::Quad2D* quad = Hermes::Hermes2D::Filter<Scalar>::quads[Hermes::Hermes2D::Function<Scalar>::cur_quad];
    int np = quad->get_num_points(order, Hermes::Hermes2D::HERMES_MODE_TRIANGLE) + quad->get_num_points(order, Hermes::Hermes2D::HERMES_MODE_QUAD);
    node = Hermes::Hermes2D::Function<Scalar>::new_node(Hermes::Hermes2D::H2D_FN_DEFAULT, np);

    double **value = new double*[m_fieldInfo->module()->number_of_solution()];
    double **dudx = new double*[m_fieldInfo->module()->number_of_solution()];
    double **dudy = new double*[m_fieldInfo->module()->number_of_solution()];

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

    SceneMaterial *material = Util::scene()->labels->at(atoi(Hermes::Hermes2D::MeshFunction<Scalar>::mesh->get_element_markers_conversion().get_user_marker(e->marker).marker.c_str()) - 1)->getMarker(m_fieldInfo);
    // warning: check this, lienearity...
//    if (isLinear)
//        parser->setParserVariables(material, NULL);

    for (int i = 0; i < np; i++)
    {
        px = x[i];
        py = y[i];

        for (int k = 0; k < Hermes::Hermes2D::Filter<Scalar>::num; k++)
        {
            pvalue[k] = value[k][i];
            pdx[k] = dudx[k][i];
            pdy[k] = dudy[k][i];
        }


        // FIXME
        // warning: check this, lienearity...
//        if (!isLinear)
            parser->setParserVariables(material, NULL,
                                       pvalue[0], pdx[0], pdy[0]);

        // parse expression
        try
        {
            node->values[0][0][i] = parser->parser[0]->Eval();
        }
        catch (mu::Parser::exception_type &e)
        {
            std::cout << "Scalar view: " << e.GetMsg() << std::endl;
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

template <typename Scalar>
ViewScalarFilter<Scalar>* ViewScalarFilter<Scalar>::clone()
{
    Hermes::vector<Hermes::Hermes2D::MeshFunction<Scalar> *> slns;

    for (int i = 0; i < this->num; i++)
        slns.push_back(this->sln[i]);

    return new ViewScalarFilter(m_fieldInfo, slns, parser->parser[0]->GetExpr());
}

template class WeakFormAgros<double>;
