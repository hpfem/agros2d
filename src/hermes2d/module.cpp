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
#include "field.h"
#include "block.h"
#include "problem.h"
#include "logview.h"

#include "util.h"
#include "util/global.h"
#include "scene.h"
#include "scenebasic.h"
#include "scenemarkerdialog.h"
#include "scenelabel.h"
#include "sceneedge.h"
#include "hermes2d/solver.h"
#include "hermes2d/coupling.h"
#include "hermes2d/solutionstore.h"
#include "hermes2d/plugin_interface.h"
#include "hermes2d/bdf2.h"
#include "hermes2d/problem_config.h"

#include "mesh/mesh_reader_h2d.h"

#include "newton_solver.h"
#include "picard_solver.h"

#include "util/constants.h"

#include "../../resources_source/classes/module_xml.h"

double actualTime;

QMap<QString, QString> Module::availableModules()
{
    static QMap<QString, QString> modules;

    // read modules
    if (modules.size() == 0)
    {
        QDir dir(datadir() + MODULEROOT);

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

    return modules;
}

template <typename Scalar>
WeakFormAgros<Scalar>::WeakFormAgros(Block* block) :
    Hermes::Hermes2D::WeakForm<Scalar>(block->numSolutions()), m_block(block)
{
}

template <typename Scalar>
WeakFormAgros<Scalar>::~WeakFormAgros()
{
    foreach (Hermes::Hermes2D::Form<Scalar> *form, this->forms)
        delete form;

    delete_all();
}

template <typename Scalar>
Hermes::Hermes2D::Form<Scalar> *factoryForm(WeakFormKind type, const ProblemID problemId,
                                            const QString &area, FormInfo *form,
                                            Marker* markerSource, Material *markerTarget,
                                            int offsetI, int offsetJ)
{
    QString fieldId = (problemId.analysisTypeTarget == AnalysisType_Undefined) ?
                problemId.sourceFieldId : problemId.sourceFieldId + "-" + problemId.targetFieldId;

    PluginInterface *plugin = NULL;
    if (markerTarget)
        plugin = Agros2D::problem()->couplingInfo(markerSource->fieldId(), markerTarget->fieldId())->plugin();
    else
        plugin = Agros2D::problem()->fieldInfo(fieldId)->plugin();

    assert(plugin);

    Hermes::Hermes2D::Form<Scalar> *weakForm = NULL;

    if (type == WeakForm_MatVol)
    {
        MatrixFormVolAgros<double> *weakFormAgros = plugin->matrixFormVol(problemId, form, offsetI, offsetJ,
                                                                          static_cast<Material *>(markerSource));
        if (!weakFormAgros) return NULL;

        // symmetric flag
        weakFormAgros->setSymFlag(form->sym);
        // source marker
        weakFormAgros->setMarkerSource(markerSource);
        // target marker
        weakFormAgros->setMarkerTarget(markerTarget);

        weakForm = weakFormAgros;
    }
    else if (type == WeakForm_MatSurf)
    {
        MatrixFormSurfAgros<double> *weakFormAgros = plugin->matrixFormSurf(problemId, form, offsetI, offsetJ,
                                                                            static_cast<Boundary *>(markerSource));
        if (!weakFormAgros) return NULL;

        // source marker
        weakFormAgros->setMarkerSource(markerSource);

        weakForm = weakFormAgros;
    }
    else if (type == WeakForm_VecVol)
    {
        VectorFormVolAgros<double> *weakFormAgros = plugin->vectorFormVol(problemId, form, offsetI, offsetJ,
                                                                          static_cast<Material *>(markerSource));
        if (!weakFormAgros) return NULL;

        // source marker
        weakFormAgros->setMarkerSource(markerSource);
        // target marker
        weakFormAgros->setMarkerTarget(markerTarget);

        weakForm = weakFormAgros;
    }
    else if (type == WeakForm_VecSurf)
    {
        VectorFormSurfAgros<double> *weakFormAgros = plugin->vectorFormSurf(problemId, form, offsetI, offsetJ,
                                                                            static_cast<Boundary *>(markerSource));
        if (!weakFormAgros) return NULL;

        // source marker
        weakFormAgros->setMarkerSource(markerSource);

        weakForm = weakFormAgros;
    }

    if (!weakForm)
        assert(0);

    // set area
    weakForm->set_area(area.toStdString());

    return weakForm;
}

template <typename Scalar>
void WeakFormAgros<Scalar>::addForm(WeakFormKind type, Hermes::Hermes2D::Form<Scalar> *form)
{
    //    Agros2D::log()->printDebug("WeakFormAgros", QString("add form: type: %1, area: %2").
    //                            arg(weakFormString(type)).
    //                            arg(QString::fromStdString(form->getAreas().at(0))));

    if (type == WeakForm_MatVol)
        this->add_matrix_form((Hermes::Hermes2D::MatrixFormVol<Scalar>*) form);
    else if (type == WeakForm_MatSurf)
        this->add_matrix_form_surf((Hermes::Hermes2D::MatrixFormSurf<Scalar>*) form);
    else if (type == WeakForm_VecVol)
        this->add_vector_form((Hermes::Hermes2D::VectorFormVol<Scalar>*) form);
    else if (type == WeakForm_VecSurf)
        this->add_vector_form_surf((Hermes::Hermes2D::VectorFormSurf<Scalar>*) form);
    else
        assert(0);
}

template <typename Scalar>
void WeakFormAgros<Scalar>::registerForm(WeakFormKind type, Field *field, QString area, FormInfo form, int offsetI, int offsetJ, Marker* marker, BDF2Table* bdf2Table)
{
    ProblemID problemId;

    problemId.sourceFieldId = field->fieldInfo()->fieldId();
    problemId.analysisTypeSource = field->fieldInfo()->analysisType();
    problemId.coordinateType = Agros2D::problem()->config()->coordinateType();
    problemId.linearityType = field->fieldInfo()->linearityType();

    // compiled form
    Hermes::Hermes2D::Form<Scalar> *custom_form = factoryForm<Scalar>(type, problemId, area, &form, marker, NULL, offsetI, offsetJ);

    // weakform with zero coefficients
    if (!custom_form) return;

    // set time discretisation table
    if ((field->fieldInfo()->analysisType() == AnalysisType_Transient) && bdf2Table)
    {
        dynamic_cast<FormAgrosInterface *>(custom_form)->setTimeDiscretisationTable(bdf2Table);

        if((type == WeakForm_MatVol) || (type == WeakForm_VecVol))
        {
            Hermes::vector<Hermes::Hermes2D::MeshFunction<Scalar>* > previousSlns;

            int lastTimeStep = Agros2D::problem()->actualTimeStep() - 1; // todo: check

            for(int backLevel = 0; backLevel < bdf2Table->n(); backLevel++)
            {
                int timeStep = lastTimeStep - backLevel;
                int adaptivityStep = Agros2D::solutionStore()->lastAdaptiveStep(field->fieldInfo(), SolutionMode_Normal, timeStep);
                FieldSolutionID solutionID(field->fieldInfo(), timeStep, adaptivityStep, SolutionMode_Reference);
                if(! Agros2D::solutionStore()->contains(solutionID))
                    solutionID.solutionMode = SolutionMode_Normal;
                assert(Agros2D::solutionStore()->contains(solutionID));

                for (int comp = 0; comp < solutionID.group->numberOfSolutions(); comp++)
                    previousSlns.push_back(Agros2D::solutionStore()->multiArray(solutionID).solutions().at(comp));
            }
            custom_form->set_ext(previousSlns);
        }
    }

    addForm(type, custom_form);
}

template <typename Scalar>
void WeakFormAgros<Scalar>::registerFormCoupling(WeakFormKind type, QString area, FormInfo form, int offsetI, int offsetJ,
                                                 SceneMaterial* materialSource, SceneMaterial* materialTarget, CouplingInfo *couplingInfo)
{
    ProblemID problemId;

    problemId.sourceFieldId = materialSource->fieldInfo()->fieldId();
    problemId.targetFieldId = materialTarget->fieldInfo()->fieldId();
    problemId.analysisTypeSource = materialSource->fieldInfo()->analysisType();
    problemId.analysisTypeTarget = materialTarget->fieldInfo()->analysisType();
    problemId.coordinateType = Agros2D::problem()->config()->coordinateType();
    problemId.linearityType = materialTarget->fieldInfo()->linearityType();
    problemId.couplingType = couplingInfo->couplingType();

    // compiled form
    Hermes::Hermes2D::Form<Scalar> *custom_form = factoryForm<Scalar>(type, problemId,
                                                                      area, &form, materialSource, materialTarget, offsetI, offsetJ);
    // weakform with zero coefficients
    if (!custom_form) return;

    // TODO at the present moment, it is impossible to have more sources !
    //assert(field->m_couplingSources.size() <= 1);

    assert((type == WeakForm_MatVol) || (type == WeakForm_VecVol));

    // push external solution for weak coupling
    if (couplingInfo->isWeak())
    {
        Hermes::vector<Hermes::Hermes2D::MeshFunction<Scalar>* > couplingSlns;

        FieldSolutionID solutionID = Agros2D::solutionStore()->lastTimeAndAdaptiveSolution(couplingInfo->sourceField(), SolutionMode_Finer);
        assert(solutionID.group->numberOfSolutions() <= maxSourceFieldComponents);

        for (int comp = 0; comp < solutionID.group->numberOfSolutions(); comp++)
            couplingSlns.push_back(Agros2D::solutionStore()->multiArray(solutionID).solutions().at(comp));

        custom_form->set_ext(couplingSlns);
    }

    addForm(type, custom_form);
}


template <typename Scalar>
void WeakFormAgros<Scalar>::registerForms(BDF2Table* bdf2Table)
{
    foreach(Field* field, m_block->fields())
    {
        FieldInfo* fieldInfo = field->fieldInfo();

        // boundary conditions
        for (int edgeNum = 0; edgeNum<Agros2D::scene()->edges->count(); edgeNum++)
        {
            SceneBoundary *boundary = Agros2D::scene()->edges->at(edgeNum)->marker(fieldInfo);
            if (boundary && boundary != Agros2D::scene()->boundaries->getNone(fieldInfo))
            {
                Module::BoundaryType boundaryType = fieldInfo->boundaryType(boundary->type());

                foreach (FormInfo expression, boundaryType.wfMatrixSurface())
                    registerForm(WeakForm_MatSurf, field, QString::number(edgeNum), expression,
                                 m_block->offset(field), m_block->offset(field), boundary);

                foreach (FormInfo expression, boundaryType.wfVectorSurface())
                    registerForm(WeakForm_VecSurf, field, QString::number(edgeNum), expression,
                                 m_block->offset(field), m_block->offset(field), boundary);
            }
        }

        // materials
        for (int labelNum = 0; labelNum<Agros2D::scene()->labels->count(); labelNum++)
        {
            //cout << "material " << labelNum << endl;
            SceneMaterial *material = Agros2D::scene()->labels->at(labelNum)->marker(fieldInfo);

            assert(material);
            if (material != Agros2D::scene()->materials->getNone(fieldInfo))
            {
                foreach (FormInfo expression, fieldInfo->wfMatrixVolume())
                    registerForm(WeakForm_MatVol, field, QString::number(labelNum), expression,
                                 m_block->offset(field), m_block->offset(field), material, bdf2Table);


                foreach (FormInfo expression, fieldInfo->wfVectorVolume())
                    registerForm(WeakForm_VecVol, field, QString::number(labelNum), expression,
                                 m_block->offset(field), m_block->offset(field), material, bdf2Table);

                // weak coupling
                foreach(CouplingInfo* couplingInfo, field->couplingInfos())
                {
                    foreach (FormInfo expression, couplingInfo->wfVectorVolume())
                    {
                        SceneMaterial* materialSource = Agros2D::scene()->labels->at(labelNum)->marker(couplingInfo->sourceField());
                        assert(materialSource);

                        if (materialSource != Agros2D::scene()->materials->getNone(couplingInfo->sourceField()))
                        {
                            registerFormCoupling(WeakForm_VecVol, QString::number(labelNum), expression,
                                                 m_block->offset(field), m_block->offset(field), materialSource, material, couplingInfo);
                        }
                    }
                }
            }
        }
    }

    // hard coupling
    foreach (CouplingInfo* couplingInfo, m_block->couplings())
    {
        if (couplingInfo->isHard())
        {
            Field* sourceField = m_block->field(couplingInfo->sourceField());
            Field* targetField = m_block->field(couplingInfo->targetField());

            for (int labelNum = 0; labelNum<Agros2D::scene()->labels->count(); labelNum++)
            {
                SceneMaterial *sourceMaterial = Agros2D::scene()->labels->at(labelNum)->marker(sourceField->fieldInfo());
                SceneMaterial *targetMaterial = Agros2D::scene()->labels->at(labelNum)->marker(targetField->fieldInfo());

                if (sourceMaterial && (sourceMaterial != Agros2D::scene()->materials->getNone(sourceField->fieldInfo()))
                        && targetMaterial && (targetMaterial != Agros2D::scene()->materials->getNone(targetField->fieldInfo())))
                {

                    qDebug() << "hard coupling form on marker " << labelNum;

                    foreach (FormInfo pars, couplingInfo->wfMatrixVolume())
                        registerFormCoupling(WeakForm_MatVol, QString::number(labelNum), pars,
                                             m_block->offset(targetField) - sourceField->fieldInfo()->numberOfSolutions(), m_block->offset(sourceField),
                                             sourceMaterial, targetMaterial, couplingInfo);

                    foreach (FormInfo pars, couplingInfo->wfVectorVolume())
                        registerFormCoupling(WeakForm_VecVol, QString::number(labelNum), pars,
                                             m_block->offset(targetField) - sourceField->fieldInfo()->numberOfSolutions(), m_block->offset(sourceField),
                                             sourceMaterial, targetMaterial, couplingInfo);

                }
            }
        }
    }
}

// ***********************************************************************************************

Module::LocalVariable::LocalVariable(const FieldInfo *fieldInfo, XMLModule::localvariable lv,
                                     CoordinateType coordinateType, AnalysisType analysisType)
{    
    m_id = QString::fromStdString(lv.id());
    m_name = fieldInfo->plugin()->localeName(QString::fromStdString(lv.name()));
    m_shortname = QString::fromStdString(lv.shortname());
    m_shortnameHtml = (lv.shortname_html().present()) ? QString::fromStdString(lv.shortname_html().get()) : m_shortname;
    m_unit = QString::fromStdString(lv.unit());
    m_unitHtml = (lv.unit_html().present()) ? QString::fromStdString(lv.unit_html().get()) : m_unit;

    m_isScalar = (lv.type() == "scalar");

    for (unsigned int i = 0; i < lv.expression().size(); i++)
    {
        XMLModule::expression exp = lv.expression().at(i);
        if (exp.analysistype() == analysisTypeToStringKey(analysisType).toStdString())
        {
            if (coordinateType == CoordinateType_Planar)
                m_expression = Expression(m_isScalar ? QString::fromStdString(exp.planar().get()) : "",
                                          m_isScalar ? "" : QString::fromStdString(exp.planar_x().get()),
                                          m_isScalar ? "" : QString::fromStdString(exp.planar_y().get()));
            else
                m_expression = Expression(m_isScalar ? QString::fromStdString(exp.axi().get()) : "",
                                          m_isScalar ? "" : QString::fromStdString(exp.axi_r().get()),
                                          m_isScalar ? "" : QString::fromStdString(exp.axi_z().get()));
        }
    }
}

// ***********************************************************************************************

Module::MaterialTypeVariable::MaterialTypeVariable(XMLModule::quantity quant)
    : m_defaultValue(0), m_expressionNonlinear(""), m_isTimeDep(false)
{
    m_id = QString::fromStdString(quant.id());

    if (quant.shortname().present())
        m_shortname = QString::fromStdString(quant.shortname().get());

    if (quant.default_().present())
        m_defaultValue = quant.default_().get();
}

// ***********************************************************************************************

Module::BoundaryType::BoundaryType(const FieldInfo *fieldInfo,
                                   QList<BoundaryTypeVariable> boundary_type_variables,
                                   XMLModule::boundary bdy)
{
    m_id = QString::fromStdString(bdy.id());
    m_name = fieldInfo->plugin()->localeName(QString::fromStdString(bdy.name()));
    m_equation = QString::fromStdString(bdy.equation());

    // variables
    for (unsigned int i = 0; i < bdy.quantity().size(); i++)
    {
        XMLModule::quantity qty = bdy.quantity().at(i);

        foreach (Module::BoundaryTypeVariable old, boundary_type_variables)
        {
            if (old.id().toStdString() == qty.id())
            {
                bool isTimeDep = false;
                bool isSpaceDep = false;

                if (qty.dependence().present())
                {
                    if (QString::fromStdString(qty.dependence().get()) == "time")
                    {
                        isTimeDep = true;
                    }
                    else if (QString::fromStdString(qty.dependence().get()) == "space")
                    {
                        isSpaceDep = true;
                    }
                    else if (QString::fromStdString(qty.dependence().get()) == "time-space")
                    {
                        isTimeDep = true;
                        isSpaceDep = true;
                    }
                }

                m_variables.append(Module::BoundaryTypeVariable(old.id(), old.shortname(), isTimeDep, isSpaceDep));
            }
        }
    }

    // weakform
    for (unsigned int i = 0; i < bdy.matrix_form().size(); i++)
    {
        XMLModule::matrix_form form = bdy.matrix_form().at(i);
        m_wfMatrixSurface.append(FormInfo(QString::fromStdString(form.id()), form.i(), form.j(), form.symmetric() ? Hermes::Hermes2D::HERMES_SYM : Hermes::Hermes2D::HERMES_NONSYM));
    }

    for (unsigned int i = 0; i < bdy.vector_form().size(); i++)
    {
        XMLModule::vector_form form = bdy.vector_form().at(i);
        m_wfVectorSurface.append(FormInfo(QString::fromStdString(form.id()), form.i(), form.j()));
    }

    // essential
    for (unsigned int i = 0; i < bdy.essential_form().size(); i++)
    {
        XMLModule::essential_form form = bdy.essential_form().at(i);
        m_essential.append(FormInfo(QString::fromStdString(form.id()), form.i()));
    }
}

Module::BoundaryTypeVariable::BoundaryTypeVariable(XMLModule::quantity quant)
{
    m_id = QString::fromStdString(quant.id());
    if (quant.shortname().present())
        m_shortname = QString::fromStdString(quant.shortname().get());
    if (quant.default_().present())
        m_defaultValue = quant.default_().get();
    else
        m_defaultValue = 0.0;
}

Module::BoundaryType::~BoundaryType()
{
    m_essential.clear();
    m_variables.clear();
    m_wfMatrixSurface.clear();
    m_wfVectorSurface.clear();
    m_wfVectorSurface.clear();
}

// ***********************************************************************************************

// dialog row UI
Module::DialogRow::DialogRow(const FieldInfo *fieldInfo, XMLModule::quantity qty)
{
    m_id = QString::fromStdString(qty.id());
    m_name = (qty.name().present()) ? fieldInfo->plugin()->localeName(QString::fromStdString(qty.name().get())) : "";

    m_shortname = (qty.shortname().present()) ? QString::fromStdString(qty.shortname().get()) : "";
    m_shortnameHtml = (qty.shortname_html().present()) ? QString::fromStdString(qty.shortname_html().get()) : "";
    m_shortnameDependence = (qty.shortname_dependence().present()) ? QString::fromStdString(qty.shortname_dependence().get()) : "";
    m_shortnameDependenceHtml = (qty.shortname_dependence_html().present()) ? QString::fromStdString(qty.shortname_dependence_html().get()) : "";

    m_unit = (qty.unit().present()) ? QString::fromStdString(qty.unit().get()) : "";
    m_unitHtml = (qty.unit_html().present()) ? QString::fromStdString(qty.unit_html().get()) : "";
    m_unitLatex = (qty.unit_latex().present()) ? QString::fromStdString(qty.unit_latex().get()) : "";

    m_defaultValue = (qty.default_().present()) ? qty.default_().get() : 0.0;
    m_condition = (qty.condition().present()) ? QString::fromStdString(qty.condition().get()) : "";
}

// dialog UI
Module::DialogUI::DialogUI(const FieldInfo *fieldInfo, XMLModule::gui ui)
{
    for (unsigned int i = 0; i < ui.group().size(); i++)
    {
        XMLModule::group grp = ui.group().at(i);

        // group name
        QString name = (grp.name().present()) ? fieldInfo->plugin()->localeName(QString::fromStdString(grp.name().get())) : "";

        QList<Module::DialogRow> materials;
        for (unsigned int i = 0; i < grp.quantity().size(); i++)
        {
            XMLModule::quantity quant = grp.quantity().at(i);

            // append material
            materials.append(Module::DialogRow(fieldInfo, quant));
        }

        m_groups[name] = materials;
    }
}

Module::DialogRow Module::DialogUI::dialogRow(const QString &id)
{
    foreach (QList<Module::DialogRow> rows, m_groups)
        foreach (Module::DialogRow row, rows)
            if (row.id() == id)
                return row;
}

void Module::DialogUI::clear()
{
    m_groups.clear();
}

// ***********************************************************************************************

AGROS_API void Module::updateTimeFunctions(double time)
{
    // update materials
    foreach (SceneMaterial *material, Agros2D::scene()->materials->items())
        if (material->fieldInfo())
            foreach (Module::MaterialTypeVariable variable, material->fieldInfo()->materialTypeVariables())
                if (variable.isTimeDep() && material->fieldInfo()->analysisType() == AnalysisType_Transient)
                    material->evaluate(variable.id(), time);

    // update boundaries
    foreach (SceneBoundary *boundary, Agros2D::scene()->boundaries->items())
        if (boundary->fieldInfo())
            foreach (Module::BoundaryType boundaryType, boundary->fieldInfo()->boundaryTypes())
                foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
                    if (variable.isTimeDep() && boundary->fieldInfo()->analysisType() == AnalysisType_Transient)
                        boundary->evaluate(variable.id(), time);
}

void Module::readMeshDirtyFix()
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

Hermes::vector<Hermes::Hermes2D::Mesh *> Module::readMeshFromFile(const QString &fileName)
{
    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    Hermes::vector<Hermes::Hermes2D::Mesh* > meshes;
    int numMeshes = Agros2D::problem()->fieldInfos().count();
    for(int i = 0; i < numMeshes; i++)
    {
        Hermes::Hermes2D::Mesh *mesh = new Hermes::Hermes2D::Mesh();
        meshes.push_back(mesh);
    }

    Hermes::Hermes2D::MeshReaderH2DXML meshloader;
    meshloader.set_validation(false);
    try
    {
        meshloader.load(fileName.toStdString().c_str(), meshes);
    }
    catch (Hermes::Exceptions::MeshLoadFailureException& e)
    {
        throw Hermes::Exceptions::MeshLoadFailureException(e.what());
    }

    // set system locale
    setlocale(LC_NUMERIC, plocale);

    return meshes;
}

void Module::writeMeshToFile(const QString &fileName, Hermes::vector<Hermes::Hermes2D::Mesh *> meshes)
{
    // save locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    // QTime time;
    // time.start();

    Hermes::Hermes2D::MeshReaderH2DXML meshloader;
    meshloader.set_validation(false);
    meshloader.save(fileName.toStdString().c_str(), meshes);

    // qDebug() << milisecondsToTime(time.elapsed()).toString("hh:mm:ss.zzz");

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}

template class WeakFormAgros<double>;
