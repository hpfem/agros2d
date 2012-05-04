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
#include "field.h"
#include "block.h"
#include "problem.h"

#include "../weakform/src/weakform_factory.h"

#include "util.h"
#include "scene.h"
#include "scenebasic.h"
#include "scenemarkerdialog.h"
#include "scenelabel.h"
#include "sceneedge.h"
#include "hermes2d/solver.h"
#include "hermes2d/coupling.h"
#include "hermes2d/solutionstore.h"

#include "mesh/mesh_reader_h2d.h"

#include "newton_solver.h"
#include "picard_solver.h"

//#include "solver.h"

#include <dirent.h>

#include "../../resources_source/classes/module_xml.h"

double actualTime;

QMap<QString, QString> availableModules()
{
    static QMap<QString, QString> modules;
    
    // read modules
    if (modules.size() == 0)
    {
        QDir dir(datadir()+ MODULEROOT);

        QStringList filter;
        filter << "*.xml";
        QStringList list = dir.entryList(filter);

        foreach (QString filename, list)
        {
            std::auto_ptr<XMLModule::module> module_xsd = XMLModule::module_((datadir().toStdString() + MODULEROOT.toStdString() + "/" + filename.toStdString()).c_str());
            XMLModule::module *mod = module_xsd.get();

            // module name
            modules[filename.left(filename.size() - 4)] = QString::fromStdString(mod->general().name());
        }
    }
    
    // custom module
    // modules["custom"] = "Custom field";
    
    return modules;
}

QMap<AnalysisType, QString> availableAnalyses(const QString &fieldId)
{
    QMap<AnalysisType, QString> analyses;
    
    std::auto_ptr<XMLModule::module> module_xsd = XMLModule::module_((datadir().toStdString() + MODULEROOT.toStdString() + "/" + fieldId.toStdString() + ".xml").c_str());
    XMLModule::module *mod = module_xsd.get();
    
    for (int i = 0; i < mod->general().analyses().analysis().size(); i++)
    {
        XMLModule::analysis an = mod->general().analyses().analysis().at(i);

        analyses[analysisTypeFromStringKey(QString::fromStdString(an.id()))] = QString::fromStdString(an.name());
    }
    
    return analyses;
}

template <typename Scalar>
WeakFormAgros<Scalar>::WeakFormAgros(Block* block) :
    Hermes::Hermes2D::WeakForm<Scalar>(block->numSolutions()), m_block(block)
{
}

template <typename Scalar>
Hermes::Hermes2D::Form<Scalar> *factoryForm(WFType type, const QString &problemId,
                                            const QString &area, ParserFormExpression *form,
                                            Marker* marker)
{
    if(type == WFType_MatVol)
        return factoryMatrixFormVol<Scalar>(problemId.toStdString(), form->i, form->j, area.toStdString(), form->sym, (SceneMaterial*) marker);
    else if(type == WFType_MatSurf)
        return factoryMatrixFormSurf<Scalar>(problemId.toStdString(), form->i, form->j, area.toStdString(), (SceneBoundary*) marker);
    else if(type == WFType_VecVol)
        return factoryVectorFormVol<Scalar>(problemId.toStdString(), form->i, form->j, area.toStdString(), (SceneMaterial*) marker);
    else if(type == WFType_VecSurf)
        return factoryVectorFormSurf<Scalar>(problemId.toStdString(), form->i, form->j, area.toStdString(), (SceneBoundary*) marker);
    else
        assert(0);
}

template <typename Scalar>
Hermes::Hermes2D::Form<Scalar> *factoryParserForm(WFType type, int i, int j, const QString &area,
                                                  Hermes::Hermes2D::SymFlag sym, string expression, FieldInfo *fieldInfo,
                                                  CouplingInfo *couplingInfo,
                                                  Marker* marker, Material* markerSecond)
{
    //    cout << "factory form (" << i << ", " << j << "), area: " << area << " -> " << expression << ", marker " << marker->getName() <<  endl;
    if(type == WFType_MatVol)
        return new CustomParserMatrixFormVol<Scalar>(i, j,
                                                     area.toStdString(),
                                                     sym,
                                                     expression,
                                                     fieldInfo,
                                                     couplingInfo,
                                                     (SceneMaterial*) marker,
                                                     markerSecond);
    else if(type == WFType_MatSurf)
        return new CustomParserMatrixFormSurf<Scalar>(i, j,
                                                      area.toStdString(),
                                                      expression,
                                                      (SceneBoundary*) marker);
    else if(type == WFType_VecVol)
        return new CustomParserVectorFormVol<Scalar>(i, j,
                                                     area.toStdString(),
                                                     expression,
                                                     fieldInfo,
                                                     couplingInfo,
                                                     (SceneMaterial*) marker,
                                                     markerSecond);
    else if(type == WFType_VecSurf)
        return new CustomParserVectorFormSurf<Scalar>(i, j,
                                                      area.toStdString(),
                                                      expression,
                                                      (SceneBoundary*) marker);
    else
        assert(0);
}

template <typename Scalar>
void WeakFormAgros<Scalar>::addForm(WFType type, Hermes::Hermes2D::Form<Scalar> *form)
{
    cout << "addForm : pridavam formu typu " << type << ", i: " << form->i <<  /*", j: " << form->j <<*/ ", areas: ";
    for(int i = 0; i < form->areas.size(); i++)
        cout << form->areas.at(i) << ", ";
    cout << endl;
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
void WeakFormAgros<Scalar>::registerForm(WFType type, Field* field, QString area, ParserFormExpression *form, int offsetI, int offsetJ,
                                         Marker* marker, SceneMaterial* marker_second, CouplingInfo* couplingInfo)
{
    //TODO zatim jen interpretovane formy. Pak se musi nejak rozlisit, jestli je registrovana forma z modulu nebo ze sdruzeni
    string problemId = field->fieldInfo()->fieldId().toStdString() + "_" +
            analysisTypeToStringKey(field->fieldInfo()->module()->analysisType()).toStdString()  + "_" +
            coordinateTypeToStringKey(field->fieldInfo()->module()->coordinateType()).toStdString();
    
    Hermes::Hermes2D::Form<Scalar>* custom_form = NULL;
    
    // compiled form
    if (field->fieldInfo()->weakFormsType() == WeakFormsType_Compiled)
    {
        //assert(0);
        custom_form = factoryForm<Scalar>(type, QString::fromStdString(problemId), area, form, marker);
    }
    
    if ((custom_form == NULL) && field->fieldInfo()->weakFormsType() == WeakFormsType_Compiled)
        qDebug() << "Cannot find compiled VectorFormVol().";
    
    // interpreted form
    if (!custom_form || field->fieldInfo()->weakFormsType() == WeakFormsType_Interpreted)
    {
        FieldInfo* fieldInfo = couplingInfo ? NULL : field->fieldInfo();
        custom_form = factoryParserForm<Scalar>(type, form->i - 1 + offsetI, form->j - 1 + offsetJ, area, form->sym, form->expression,
                                                fieldInfo, couplingInfo, marker, marker_second);
    }
    
    //Decide what solution to push, implicitly none
    FieldSolutionID solutionID(NULL, 0, 0, SolutionType_NonExisting);
    
    // weak coupling, push solutions
    if (marker_second && couplingInfo->isWeak())
    {
        // TODO at the present moment, it is impossible to have more sources !
        assert(field->m_couplingSources.size() <= 1);
        
        solutionID = Util::solutionStore()->lastTimeAndAdaptiveSolution(couplingInfo->sourceField(), SolutionType_Finer);
        assert(solutionID.group->module()->numberOfSolutions() <= maxSourceFieldComponents);
    }
    else
    {
        if (field->fieldInfo()->analysisType() == AnalysisType_Transient)
        {
            solutionID = Util::solutionStore()->lastTimeAndAdaptiveSolution(field->fieldInfo(), SolutionType_Finer);
        }
    }
    
    if (solutionID.solutionType != SolutionType_NonExisting)
    {
        for (int comp = 0; comp < solutionID.group->module()->numberOfSolutions(); comp++)
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
    
    foreach(Field* field, m_block->fields())
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        
        // boundary conditions
        for (int edgeNum = 0; edgeNum<Util::scene()->edges->count(); edgeNum++)
        {
            SceneBoundary *boundary = Util::scene()->edges->at(edgeNum)->getMarker(fieldInfo);
            cout << "registerForms : registering edge " << edgeNum << endl;
            
            if (boundary && boundary != Util::scene()->boundaries->getNone(fieldInfo))
            {
                Module::BoundaryType *boundary_type = fieldInfo->module()->boundaryType(boundary->getType());
                
                foreach (ParserFormExpression *expression, boundary_type->m_wfMatrixSurface)
                    registerForm(WFType_MatSurf, field, QString::number(edgeNum), expression,
                                 m_block->offset(field), m_block->offset(field), boundary);
                
                foreach (ParserFormExpression *expression, boundary_type->m_wfVectorSurface)
                    registerForm(WFType_VecSurf, field, QString::number(edgeNum), expression,
                                 m_block->offset(field), m_block->offset(field), boundary);
            }
        }
        
        // materials
        for (int labelNum = 0; labelNum<Util::scene()->labels->count(); labelNum++)
        {
            SceneMaterial *material = Util::scene()->labels->at(labelNum)->getMarker(fieldInfo);
            
            assert(material);
            if (material != Util::scene()->materials->getNone(fieldInfo))
            {
                foreach (ParserFormExpression *expression, fieldInfo->module()->wfMatrixVolumeExpression())
                    registerForm(WFType_MatVol, field, QString::number(labelNum), expression,
                                 m_block->offset(field), m_block->offset(field), material);

                foreach (ParserFormExpression *expression, fieldInfo->module()->wfVectorVolumeExpression())
                    registerForm(WFType_VecVol, field, QString::number(labelNum), expression,
                                 m_block->offset(field), m_block->offset(field), material);

                // weak coupling
                foreach(CouplingInfo* couplingInfo, field->m_couplingSources)
                {
                    foreach (ParserFormExpression *expression, couplingInfo->coupling()->weakform_vector_volume)
                    {
                        SceneMaterial* material2 = Util::scene()->labels->at(labelNum)->getMarker(couplingInfo->sourceField());
                        assert(material2);

                        if (material2 != Util::scene()->materials->getNone(couplingInfo->sourceField()))
                        {
                            registerForm(WFType_VecVol, field, QString::number(labelNum), expression,
                                         m_block->offset(field), m_block->offset(field), material, material2, couplingInfo);
                        }
                    }
                }
            }
        }
    }
    
    // hard coupling
    foreach (CouplingInfo* couplingInfo, m_block->couplings())
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
                
                cout << "hard coupling form on marker " << labelNum << endl;

                foreach (ParserFormExpression *pars, coupling->weakform_matrix_volume)
                    registerForm(WFType_MatVol, sourceField, QString::number(labelNum), pars,
                                 m_block->offset(targetField) - sourceField->fieldInfo()->module()->numberOfSolutions(), m_block->offset(sourceField),
                                 sourceMaterial, targetMaterial, couplingInfo);
                
                foreach (ParserFormExpression *pars, coupling->weakform_vector_volume)
                    registerForm(WFType_VecVol, sourceField, QString::number(labelNum), pars,
                                 m_block->offset(targetField) - sourceField->fieldInfo()->module()->numberOfSolutions(), m_block->offset(sourceField),
                                 sourceMaterial, targetMaterial, couplingInfo);

            }
        }
    }
}

// ***********************************************************************************************

Module::LocalVariable::LocalVariable(XMLModule::localvariable lv,
                                     CoordinateType coordinateType, AnalysisType analysisType)
{
    id = QString::fromStdString(lv.id());
    name = QString::fromStdString(lv.name());
    shortname = QString::fromStdString(lv.shortname());
    shortname_html = (lv.shortname_html().present()) ? QString::fromStdString(lv.shortname_html().get()) : shortname;
    unit = QString::fromStdString(lv.unit());
    unit_html = (lv.unit_html().present()) ? QString::fromStdString(lv.unit_html().get()) : unit;
    
    isScalar = (lv.type() == "scalar");
    
    for (int i = 0; i < lv.expression().size(); i++)
    {
        XMLModule::expression exp = lv.expression().at(i);
        if (exp.analysistype() == analysisTypeToStringKey(analysisType).toStdString())
        {
            if (coordinateType == CoordinateType_Planar)
                expr = Expression(isScalar ? QString::fromStdString(exp.planar().get()) : "",
                                  isScalar ? "" : QString::fromStdString(exp.planar_x().get()),
                                  isScalar ? "" : QString::fromStdString(exp.planar_y().get()));
            else
                expr = Expression(isScalar ? QString::fromStdString(exp.axi().get()) : "",
                                  isScalar ? "" : QString::fromStdString(exp.axi_r().get()),
                                  isScalar ? "" : QString::fromStdString(exp.axi_z().get()));
        }
    }
}

// ***********************************************************************************************

Module::MaterialTypeVariable::MaterialTypeVariable(XMLModule::quantity quant)
{
    id = QString::fromStdString(quant.id());
    if (quant.shortname().present())
        shortname = QString::fromStdString(quant.shortname().get());
    if (quant.default_().present())
        default_value = quant.default_().get();
    else
        default_value = 0.0;
}

Module::MaterialTypeVariable::MaterialTypeVariable(const QString &id, QString shortname,
                                                   double default_value)
{
    this->id = id;
    this->shortname = shortname;
    this->default_value = default_value;
}

// ***********************************************************************************************

Module::BoundaryType::BoundaryType(QList<BoundaryTypeVariable> boundary_type_variables,
                                   XMLModule::boundary bdy,
                                   CoordinateType problem_type)
{
    id = QString::fromStdString(bdy.id());
    name = QString::fromStdString(bdy.name());
    
    // variables
    for (int i = 0; i < bdy.quantity().size(); i++)
    {
        XMLModule::quantity qty = bdy.quantity().at(i);
        
        foreach (Module::BoundaryTypeVariable old, boundary_type_variables)
        {
            if (old.id.toStdString() == qty.id())
            {
                Module::BoundaryTypeVariable *var = new Module::BoundaryTypeVariable(
                            old.id, old.shortname, old.default_value);
                
                variables.append(var);
            }
        }
    }
    
    // weakform
    for (int i = 0; i < bdy.matrix_form().size(); i++)
    {
        XMLModule::matrix_form form = bdy.matrix_form().at(i);
        m_wfMatrixSurface.append(new ParserFormExpression(form.i(), form.j(),
                                                          form.symmetric() ? Hermes::Hermes2D::HERMES_SYM : Hermes::Hermes2D::HERMES_NONSYM,
                                                          (problem_type == CoordinateType_Planar) ? form.planar() : form.axi()));
    }
    
    for (int i = 0; i < bdy.vector_form().size(); i++)
    {
        XMLModule::vector_form form = bdy.vector_form().at(i);
        m_wfVectorSurface.append(new ParserFormExpression(form.i(), form.j(),
                                                          Hermes::Hermes2D::HERMES_NONSYM,
                                                          (problem_type == CoordinateType_Planar) ? form.planar() : form.axi()));
    }
    
    // essential
    for (int i = 0; i < bdy.essential_form().size(); i++)
    {
        XMLModule::essential_form form = bdy.essential_form().at(i);
        essential.append(new ParserFormEssential(form.i(),
                                                 (problem_type == CoordinateType_Planar) ? form.planar() : form.axi()));
    }
}

Module::BoundaryTypeVariable::BoundaryTypeVariable(XMLModule::quantity quant)
{
    id = QString::fromStdString(quant.id());
    if (quant.shortname().present())
        shortname = QString::fromStdString(quant.shortname().get());
    if (quant.default_().present())
        default_value = quant.default_().get();
    else
        default_value = 0.0;
}

Module::BoundaryTypeVariable::BoundaryTypeVariable(const QString &id, QString shortname,
                                                   double default_value)
{
    this->id = id;
    this->shortname = shortname;
    this->default_value = default_value;
}

Module::BoundaryType::~BoundaryType()
{
    // essential
    essential.clear();
    
    // variables
    foreach (Module::BoundaryTypeVariable *variable, variables)
        delete variable;
    variables.clear();
    
    // volume weak form
    foreach (ParserFormExpression *expression, m_wfMatrixSurface)
        delete expression;
    m_wfMatrixSurface.clear();
    
    foreach (ParserFormExpression *expression, m_wfVectorSurface)
        delete expression;
    m_wfVectorSurface.clear();
    
    foreach (ParserFormExpression *expression, m_wfVectorSurface)
        delete expression;
    m_wfVectorSurface.clear();
}

// ***********************************************************************************************

// dialog row UI
Module::DialogUI::DialogRow::DialogRow(XMLModule::quantity qty)
{
    id = QString::fromStdString(qty.id());
    name = (qty.name().present()) ? QString::fromStdString(qty.name().get()) : "";

    nonlin = (qty.nonlin().present()) ? qty.nonlin().get() : false;
    timedep = (qty.timedep().present()) ? qty.timedep().get() : false;
    
    shortname = (qty.shortname().present()) ? QString::fromStdString(qty.shortname().get()) : "";
    shortname_html = (qty.shortname_html().present()) ? QString::fromStdString(qty.shortname_html().get()) : "";

    unit = (qty.unit().present()) ? QString::fromStdString(qty.unit().get()) : "";
    unit_html = (qty.unit_html().present()) ? QString::fromStdString(qty.unit_html().get()) : "";
    unit_latex = (qty.unit_latex().present()) ? QString::fromStdString(qty.unit_latex().get()) : "";
    
    default_value = (qty.default_().present()) ? qty.default_().get() : 0.0;
    condition = (qty.condition().present()) ? QString::fromStdString(qty.condition().get()) : "0.0";
}

// dialog UI
Module::DialogUI::DialogUI(XMLModule::gui ui)
{
    for (int i = 0; i < ui.group().size(); i++)
    {
        XMLModule::group grp = ui.group().at(i);

        // group name
        QString name = (grp.name().present()) ? QString::fromStdString(grp.name().get()) : "";
        
        QList<DialogUI::DialogRow> materials;
        for (int i = 0; i < grp.quantity().size(); i++)
        {
            XMLModule::quantity quant = grp.quantity().at(i);

            // append material
            materials.append(DialogUI::DialogRow(quant));
        }
        
        groups[name] = materials;
    }
}

void Module::DialogUI::clear()
{
    groups.clear();
}

// ***********************************************************************************************

Module::BasicModule::BasicModule(const QString &fieldId, CoordinateType problemType, AnalysisType analysisType) :
    m_coordinateType(problemType), m_analysisType(analysisType),
    m_materialUI(NULL), m_boundaryUI(NULL),
    m_defaultViewScalarVariable(NULL), m_defaultViewVectorVariable(NULL), m_boundaryTypeDefault(NULL)
{
    clear();

    // open xml description
    read(datadir() + MODULEROOT + "/" + fieldId + ".xml");
}

Module::BasicModule::~BasicModule()
{
    clear();
}

void Module::BasicModule::read(const QString &filename)
{
    assert(QFile::exists(filename));

    qDebug() << "reading module: " << filename;

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    module_xsd = XMLModule::module_(filename.toStdString().c_str());
    XMLModule::module *mod = module_xsd.get();

    // problem
    m_fieldid = QString::fromStdString(mod->general().id());
    m_name = QString::fromStdString(mod->general().name());
    m_hasDeformableShape = mod->general().deformed_shape();
    n_description = QString::fromStdString(mod->general().description());

    // analyses
    m_numberOfSolutions = 0;

    for (int i = 0; i < mod->general().analyses().analysis().size(); i++)
    {
        XMLModule::analysis an = mod->general().analyses().analysis().at(i);

        // TODO: not good
        m_analyses[analysisTypeFromStringKey(QString::fromStdString(an.id()))] = QString::fromStdString(an.type());

        if (an.type() == QString("steadystate").toStdString())
            m_numberOfSolutions = an.solutions();
        else if (an.type() == QString("harmonic").toStdString())
            m_numberOfSolutions = an.solutions();
        else if (an.type() == QString("transient").toStdString())
            m_numberOfSolutions = an.solutions();
    }

    // constants
    for (int i = 0; i < mod->constants().constant().size(); i++)
    {
        XMLModule::constant cnst = mod->constants().constant().at(i);
        m_constants[QString::fromStdString(cnst.id())] = cnst.value();
    }

    // macros
    /*
        for (rapidxml::xml_node<> *macro = doc.first_node("module")->first_node("macros")->first_node("macro");
             macro; macro = macro->next_sibling())
            macros[macro->first_attribute("id")->value()] = macro->first_attribute("expression")->value();
        */

    // surface weakforms
    QList<Module::BoundaryTypeVariable> boundary_type_variables_tmp;
    for (int i = 0; i < mod->surface().quantity().size(); i++)
    {
        XMLModule::quantity quant = mod->surface().quantity().at(i);
        boundary_type_variables_tmp.append(Module::BoundaryTypeVariable(quant));
    }

    for (int i = 0; i < mod->surface().weakforms_surface().weakform_surface().size(); i++)
    {
        XMLModule::weakform_surface wf = mod->surface().weakforms_surface().weakform_surface().at(i);

        if (wf.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
            for (int i = 0; i < wf.boundary().size(); i++)
            {
                XMLModule::boundary bdy = wf.boundary().at(i);
                m_boundaryTypes.append(new Module::BoundaryType(boundary_type_variables_tmp, bdy, m_coordinateType));
            }

        // default
        m_boundaryTypeDefault = boundaryType(QString::fromStdString(wf.default_().get()));
    }
    boundary_type_variables_tmp.clear();

    // volumetric weakforms
    QList<Module::MaterialTypeVariable> material_type_variables_tmp;
    for (int i = 0; i < mod->volume().quantity().size(); i++)
    {
        XMLModule::quantity quant = mod->volume().quantity().at(i);
        material_type_variables_tmp.append(Module::MaterialTypeVariable(quant));
    }

    for (int i = 0; i < mod->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume wf = mod->volume().weakforms_volume().weakform_volume().at(i);

        if (wf.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
        {
            for (int i = 0; i < wf.quantity().size(); i++)
            {
                XMLModule::quantity qty = wf.quantity().at(i);

                for (QList<Module::MaterialTypeVariable>::iterator it = material_type_variables_tmp.begin();
                     it < material_type_variables_tmp.end(); ++it )
                {
                    Module::MaterialTypeVariable old = (Module::MaterialTypeVariable) *it;
                    if (old.id.toStdString() == qty.id())
                    {
                        Module::MaterialTypeVariable *var = new Module::MaterialTypeVariable(
                                    old.id, old.shortname, old.default_value);
                        m_materialTypeVariables.append(var);
                    }
                }
            }
            material_type_variables_tmp.clear();

            // weakform
            for (int i = 0; i < wf.matrix_form().size(); i++)
            {
                XMLModule::matrix_form form = wf.matrix_form().at(i);
                m_wfMatrixVolumeExpression.append(new ParserFormExpression(form.i(), form.j(),
                                                                           form.symmetric() ? Hermes::Hermes2D::HERMES_SYM : Hermes::Hermes2D::HERMES_NONSYM,
                                                                           (m_coordinateType == CoordinateType_Planar) ? form.planar() : form.axi()));
            }

            for (int i = 0; i < wf.vector_form().size(); i++)
            {
                XMLModule::vector_form form = wf.vector_form().at(i);
                m_wfVectorVolumeExpression.append(new ParserFormExpression(form.i(), form.j(),
                                                                           Hermes::Hermes2D::HERMES_NONSYM,
                                                                           (m_coordinateType == CoordinateType_Planar) ? form.planar() : form.axi()));
            }
        }
    }

    // local variables
    for (int i = 0; i < mod->postprocessor().localvariables().localvariable().size(); i++)
    {
        XMLModule::localvariable lv = mod->postprocessor().localvariables().localvariable().at(i);

        // HACK
        for (int i = 0; i < lv.expression().size(); i++)
        {
            XMLModule::expression expr = lv.expression().at(i);
            if (expr.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
            {
                Module::LocalVariable *var = new Module::LocalVariable(lv,
                                                                       m_coordinateType,
                                                                       m_analysisType);
                variables.append(var);

                // HACK - local point
                m_localPointVariables.append(var);
                // HACK - scalar view
                m_viewScalarVariables.append(var);
                // HACK - vector view
                if (!var->isScalar)
                    m_viewVectorVariables.append(var);
            }
        }
    }

    // custom local variable
    //TODO
    //Module::LocalVariable *customLocalVariable = new Module::LocalVariable("custom", "Custom", "custom", "-");
    //customLocalVariable->expression.scalar = "value1";
    //view_scalar_variables.append(customLocalVariable);

    // scalar variables default
    for (int i = 0; i < mod->postprocessor().view().scalar_view().default_().size(); i++)
    {
        XMLModule::default_ def = mod->postprocessor().view().scalar_view().default_().at(i);
        if (def.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
            m_defaultViewScalarVariable = localVariable(QString::fromStdString(def.id()));
    }

    // vector variables default
    for (int i = 0; i < mod->postprocessor().view().vector_view().default_().size(); i++)
    {
        XMLModule::default_ def = mod->postprocessor().view().vector_view().default_().at(i);
        if (def.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
            m_defaultViewVectorVariable = localVariable(QString::fromStdString(def.id()));
    }

    // volume integral
    for (int i = 0; i < mod->postprocessor().volumeintegrals().volumeintegral().size(); i++)
    {
        XMLModule::volumeintegral vol = mod->postprocessor().volumeintegrals().volumeintegral().at(i);

        Module::Integral *volint = new Module::Integral();

        volint->id = QString::fromStdString(vol.id());
        volint->name = QString::fromStdString(vol.name());
        volint->shortname = QString::fromStdString(vol.shortname());
        volint->shortname_html = (vol.shortname_html().present()) ? QString::fromStdString(vol.shortname_html().get()) : volint->shortname;
        volint->unit = QString::fromStdString(vol.unit());
        volint->unit_html = (vol.unit_html().present()) ? QString::fromStdString(vol.unit_html().get()) : volint->unit;

        for (int i = 0; i < vol.expression().size(); i++)
        {
            XMLModule::expression exp = vol.expression().at(i);
            if (exp.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
            {
                if (m_coordinateType == CoordinateType_Planar)
                    volint->expr.scalar = QString::fromStdString(exp.planar().get());
                else
                    volint->expr.scalar = QString::fromStdString(exp.axi().get());
            }
        }

        m_volumeIntegrals.append(volint);
    }

    // surface integral
    for (int i = 0; i < mod->postprocessor().surfaceintegrals().surfaceintegral().size(); i++)
    {
        XMLModule::surfaceintegral vol = mod->postprocessor().surfaceintegrals().surfaceintegral().at(i);

        Module::Integral *volint = new Module::Integral();

        volint->id = QString::fromStdString(vol.id());
        volint->name = QString::fromStdString(vol.name());
        volint->shortname = QString::fromStdString(vol.shortname());
        volint->shortname_html = (vol.shortname_html().present()) ? QString::fromStdString(vol.shortname_html().get()) : volint->shortname;
        volint->unit = QString::fromStdString(vol.unit());
        volint->unit_html = (vol.unit_html().present()) ? QString::fromStdString(vol.unit_html().get()) : volint->unit;

        for (int i = 0; i < vol.expression().size(); i++)
        {
            XMLModule::expression exp = vol.expression().at(i);
            if (exp.analysistype() == analysisTypeToStringKey(m_analysisType).toStdString())
            {
                if (m_coordinateType == CoordinateType_Planar)
                    volint->expr.scalar = QString::fromStdString(exp.planar().get());
                else
                    volint->expr.scalar = QString::fromStdString(exp.axi().get());
            }
        }

        m_surfaceIntegrals.append(volint);
    }

    // preprocessor
    for (int i = 0; i < mod->preprocessor().gui().size(); i++)
    {
        XMLModule::gui ui = mod->preprocessor().gui().at(i);
        if (ui.type() == "volume")
            m_materialUI = new Module::DialogUI(ui);
        else if (ui.type() == "surface")
            m_boundaryUI = new Module::DialogUI(ui);
    }

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

void Module::BasicModule::clear()
{
    // general information
    m_fieldid = "";
    m_name = "";
    n_description = "";

    // analyses
    m_analyses.clear();

    // constants
    m_constants.clear();

    // macros
    m_macros.clear();

    // boundary types
    for (QList<BoundaryType *>::iterator it = m_boundaryTypes.begin(); it < m_boundaryTypes.end(); ++it)
        delete *it;
    m_boundaryTypes.clear();
    m_boundaryTypeDefault = NULL;

    // material types
    for (QList<MaterialTypeVariable *>::iterator it = m_materialTypeVariables.begin(); it < m_materialTypeVariables.end(); ++it)
        delete *it;
    m_materialTypeVariables.clear();

    // variables
    foreach (Module::LocalVariable *variable, variables)
        delete variable;
    variables.clear();

    // scalar and vector variables
    m_viewScalarVariables.clear();
    m_viewVectorVariables.clear();

    // default variables
    m_defaultViewScalarVariable = NULL;
    m_defaultViewVectorVariable = NULL;

    // local variables
    m_localPointVariables.clear();

    // surface integrals
    m_surfaceIntegrals.clear();

    // volume integrals
    m_volumeIntegrals.clear();

    // volume weak form
    for (QList<ParserFormExpression *>::iterator it = m_wfMatrixVolumeExpression.begin(); it < m_wfMatrixVolumeExpression.end(); ++it)
        delete *it;
    m_wfMatrixVolumeExpression.clear();

    for (QList<ParserFormExpression *>::iterator it = m_wfVectorVolumeExpression.begin(); it < m_wfVectorVolumeExpression.end(); ++it)
        delete *it;
    m_wfVectorVolumeExpression.clear();

    if (m_materialUI)
    {
        m_materialUI->clear();
        delete m_materialUI;
    }
    m_materialUI = NULL;

    if (m_boundaryUI)
    {
        m_boundaryUI->clear();
        delete m_boundaryUI;
    }
    m_boundaryUI = NULL;
}

Module::LocalVariable *Module::BasicModule::localVariable(const QString &id)
{
    foreach (Module::LocalVariable *variable, variables)
        if (variable->id == id)
            return variable;

    return NULL;
}

Module::BoundaryType *Module::BasicModule::boundaryType(const QString &id)
{
    foreach (Module::BoundaryType *boundaryType, m_boundaryTypes)
        if (boundaryType->id == id)
            return boundaryType;

    return NULL;
}

Module::BoundaryTypeVariable *Module::BasicModule::boundaryTypeVariable(const QString &id)
{
    foreach (Module::BoundaryTypeVariable *variable, m_boundaryTypeVariables)
        if (variable->id == id)
            return variable;

    return NULL;
}

Module::MaterialTypeVariable *Module::BasicModule::materialTypeVariable(const QString &id)
{
    foreach (Module::MaterialTypeVariable *variable, m_materialTypeVariables)
        if (variable->id == id)
            return variable;

    return NULL;
}

mu::Parser *Module::BasicModule::expressionParser(const QString &expr)
{
    mu::Parser *parser = new mu::Parser();

    // pi
    parser->DefineConst("PI", M_PI);

    // frequency
    parser->DefineConst("f", Util::problem()->config()->frequency());

    // timestep
    parser->DefineConst("dt", Util::problem()->config()->timeStep().number());

    if (!expr.isEmpty())
        parser->SetExpr(expr.toStdString());

    QMapIterator<QString, double> iConstant(m_constants);
    while (iConstant.hasNext())
    {
        iConstant.next();

        parser->DefineConst(iConstant.key().toStdString(), iConstant.value());
    }

    parser->EnableOptimizer(true);

    return parser;
}

QString Module::BasicModule::expression(Module::LocalVariable *physicFieldVariable,
                                   PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariableComp)
    {
    case PhysicFieldVariableComp_Undefined:
        return "";
    case PhysicFieldVariableComp_Scalar:
        return physicFieldVariable->expr.scalar;
    case PhysicFieldVariableComp_X:
        return physicFieldVariable->expr.comp_x;
    case PhysicFieldVariableComp_Y:
        return physicFieldVariable->expr.comp_y;
    case PhysicFieldVariableComp_Magnitude:
        return "sqrt((" + physicFieldVariable->expr.comp_x + ") * (" + physicFieldVariable->expr.comp_x + ") + (" + physicFieldVariable->expr.comp_y + ") * (" + physicFieldVariable->expr.comp_y + "))";
    default:
        error("Unknown type.");
    }
}

ViewScalarFilter<double> *Module::BasicModule::viewScalarFilter(Module::LocalVariable *physicFieldVariable,
                                                           PhysicFieldVariableComp physicFieldVariableComp)
{
    // update time functions
    /*
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
        m_fieldInfo->module()->update_time_functions(Util::problem()->time());
    */

    Hermes::vector<Hermes::Hermes2D::MeshFunction<double> *> sln;
    for (int k = 0; k < numberOfSolutions(); k++)
    {
        FieldSolutionID fsid(Util::scene()->activeViewField(), Util::scene()->activeTimeStep(), Util::scene()->activeAdaptivityStep(), Util::scene()->activeSolutionType());
        sln.push_back(Util::solutionStore()->multiSolution(fsid).component(k).sln.get());
    }
    return new ViewScalarFilter<double>(Util::scene()->activeViewField(),
                                        sln,
                                        expression(physicFieldVariable, physicFieldVariableComp));
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
    foreach(FieldInfo* fieldInfo, Util::problem()->fieldInfos())
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
        for (int i = 0; i < fieldInfo->numberOfRefinements(); i++)
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

Parser::Parser(FieldInfo *fieldInfo) : m_fieldInfo(fieldInfo), m_couplingInfo(NULL)
{

}

Parser::Parser(CouplingInfo *couplingInfo) : m_fieldInfo(NULL), m_couplingInfo(couplingInfo)
{

}

Parser::~Parser()
{
    // delete parser
    for (QList<mu::Parser *>::iterator it = parser.begin(); it < parser.end(); ++it)
        delete *it;

    parser.clear();
}

void Parser::setParserVariables(Material* material, Boundary *boundary, double value, double dx, double dy)
{
    assert(m_couplingInfo == NULL);
    QList<Material *> materials;
    if(material)
        materials.append(material);
    setParserVariables(materials, boundary, value, dx, dy);
}

void Parser::setParserVariables(QList<Material *> materialMarkers, Boundary *boundary, double value, double dx, double dy)
{
    // todo: ??? je to zmatecne, jak je to nekdy volano s fieldInfo, nekddy couplingInfo...
    //    if(materialMarkers.size() > 1)
    //        assert(m_couplingInfo);

    //TODO zkontrolovat volani value, proc u boundary neni derivace, ...
    if (materialMarkers.size())
    {
        foreach (Material *material, materialMarkers)
        {
            QList<Module::MaterialTypeVariable *> materials = material->getFieldInfo()->module()->materialTypeVariables();
            foreach (Module::MaterialTypeVariable *variable, materials)
            {
                parser_variables[variable->shortname] = material->getValue(variable->id).value(value);
                parser_variables["d" + variable->shortname] = material->getValue(variable->id).derivative(value);
            }
        }
    }

    if (boundary)
    {
        Module::BoundaryType *boundaryType = m_fieldInfo->module()->boundaryType(boundary->getType());
        foreach (Module::BoundaryTypeVariable *variable, boundaryType->variables)
        {
            parser_variables[variable->shortname] = boundary->getValue(variable->id).value(0.0);
            //cout << "setParserVariables: shortname: " << variable->shortname << ", id: " << variable->id << ", value: " << boundary->getValue(variable->id).value(0.0) << endl;
        }
    }

}

void Parser::initParserMaterialVariables()
{
    QList<Module::MaterialTypeVariable *> materials = m_fieldInfo->module()->materialTypeVariables();
    for (QList<Module::MaterialTypeVariable *>::iterator it = materials.begin(); it < materials.end(); ++it)
    {
        Module::MaterialTypeVariable *variable = ((Module::MaterialTypeVariable *) *it);
        parser_variables[variable->shortname] = 0.0;
    }

    // set material variables
    for (std::map<QString, double>::iterator itv = parser_variables.begin(); itv != parser_variables.end(); ++itv)
        for (QList<mu::Parser *>::iterator it = parser.begin(); it < parser.end(); ++it)
            ((mu::Parser *) *it)->DefineVar(itv->first.toStdString(), &itv->second);
}

//void Parser::initParserBoundaryVariables(Boundary *boundary)
//{
//    Module::BoundaryType *boundary_type = fieldInfo->module()->get_boundary_type(boundary->getType());
//    for (QList<Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin(); it < boundary_type->variables.end(); ++it)
//    {
//        Module::BoundaryTypeVariable *variable = ((Module::BoundaryTypeVariable *) *it);
//        parser_variables[variable->shortname] = 0.0;
//    }

//    // set material variables
//    for (std::map<QString, double>::iterator itv = parser_variables.begin(); itv != parser_variables.end(); ++itv)
//        for (QList<mu::Parser *>::iterator it = parser.begin(); it < parser.end(); ++it)
//            ((mu::Parser *) *it)->DefineVar(itv->first, &itv->second);
//}

// *********************************************************************************************************************************************

template <typename Scalar>
ViewScalarFilter<Scalar>::ViewScalarFilter(FieldInfo *fieldInfo,
                                           Hermes::vector<Hermes::Hermes2D::MeshFunction<Scalar> *> sln,
                                           QString expression)
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
void ViewScalarFilter<Scalar>::initParser(const QString &expression)
{
    mu::Parser *pars = m_fieldInfo->module()->expressionParser();

    pars->SetExpr(expression.toStdString());

    pars->DefineVar(Util::problem()->config()->labelX().toLower().toStdString(), &px);
    pars->DefineVar(Util::problem()->config()->labelY().toLower().toStdString(), &py);

    pvalue = new double[Hermes::Hermes2D::Filter<Scalar>::num];
    pdx = new double[Hermes::Hermes2D::Filter<Scalar>::num];
    pdy = new double[Hermes::Hermes2D::Filter<Scalar>::num];

    for (int k = 0; k < Hermes::Hermes2D::Filter<Scalar>::num; k++)
    {
        std::stringstream number;
        number << (k+1);

        pars->DefineVar("value" + number.str(), &pvalue[k]);
        pars->DefineVar("d" + Util::problem()->config()->labelX().toLower().toStdString() + number.str(), &pdx[k]);
        pars->DefineVar("d" + Util::problem()->config()->labelY().toLower().toStdString() + number.str(), &pdy[k]);
    }

    parser->parser.append(pars);
}

template <typename Scalar>
double ViewScalarFilter<Scalar>::get_pt_value(double x, double y, int item)
{
    return 0.0;
}

template <typename Scalar>
void ViewScalarFilter<Scalar>::precalculate(int order, int mask)
{
    bool isLinear = (m_fieldInfo->linearityType() == LinearityType_Linear);

    Hermes::Hermes2D::Quad2D* quad = Hermes::Hermes2D::Filter<Scalar>::quads[Hermes::Hermes2D::Function<Scalar>::cur_quad];
    int np = quad->get_num_points(order, Hermes::Hermes2D::HERMES_MODE_TRIANGLE) + quad->get_num_points(order, Hermes::Hermes2D::HERMES_MODE_QUAD);
    node = Hermes::Hermes2D::Function<Scalar>::new_node(Hermes::Hermes2D::H2D_FN_DEFAULT, np);

    double **value = new double*[m_fieldInfo->module()->numberOfSolutions()];
    double **dudx = new double*[m_fieldInfo->module()->numberOfSolutions()];
    double **dudy = new double*[m_fieldInfo->module()->numberOfSolutions()];

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

    SceneMaterial *material = Util::scene()->labels->at(atoi(Hermes::Hermes2D::MeshFunction<Scalar>::mesh->get_element_markers_conversion().get_user_marker(e->marker).marker.c_str()))->getMarker(m_fieldInfo);
    if (isLinear)
        parser->setParserVariables(material, NULL);

    for (int i = 0; i < np; i++)
    {
        px = x[i];
        py = y[i];

        for (int k = 0; k < m_fieldInfo->module()->numberOfSolutions(); k++)
        {
            pvalue[k] = value[k][i];
            pdx[k] = dudx[k][i];
            pdy[k] = dudy[k][i];
        }


        if (!isLinear)
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

    return new ViewScalarFilter(m_fieldInfo, slns, QString::fromStdString(parser->parser[0]->GetExpr()));
}

template class WeakFormAgros<double>;
