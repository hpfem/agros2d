// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#include "module.h"
#include "weak_form.h"
#include "field.h"
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

#include "util/constants.h"

#include "../../resources_source/classes/module_xml.h"

double actualTime;

QMap<QString, QString> Module::availableModules()
{
    static QMap<QString, QString> modules;

    // read modules
    if (modules.isEmpty())
    {
        QDir dir(datadir() + MODULEROOT);

        QStringList filter;
        filter << "*.xml";
        QStringList list = dir.entryList(filter);

        foreach (QString filename, list)
        {
            try
            {
                // todo: find a way to validate if required. If validated here, sensible error messages will be obtained
                bool validateAtTheBeginning = false;
                ::xml_schema::flags parsing_flags = xml_schema::flags::dont_validate;
                if(validateAtTheBeginning)
                {
                    parsing_flags = 0;
                    qDebug() << "Warning: Validating all XML files. This is time-consuming and should be switched off in module.cpp for release. Set validateAtTheBeginning = false.";
                }
                std::auto_ptr<XMLModule::module> module_xsd(XMLModule::module_(compatibleFilename(datadir() + MODULEROOT + "/" + filename).toStdString(), parsing_flags));

                XMLModule::module *mod = module_xsd.get();
                assert(mod->field().present());
                XMLModule::field *field = &mod->field().get();

                // module name
                modules[filename.left(filename.size() - 4)] = QString::fromStdString(field->general_field().name());
            }
            catch (const xml_schema::expected_element& e)
            {
                QString str = QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.name()));
                qDebug() << str;
                throw AgrosException(str);
            }
            catch (const xml_schema::expected_attribute& e)
            {
                QString str = QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.name()));
                qDebug() << str;
                throw AgrosException(str);
            }
            catch (const xml_schema::unexpected_element& e)
            {
                QString str = QString("%1: %2 instead of %3").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.encountered_name())).arg(QString::fromStdString(e.expected_name()));
                qDebug() << str;
                throw AgrosException(str);
            }
            catch (const xml_schema::unexpected_enumerator& e)
            {
                QString str = QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.enumerator()));
                qDebug() << str;
                throw AgrosException(str);
            }
            catch (const xml_schema::expected_text_content& e)
            {
                QString str = QString("%1").arg(QString::fromStdString(e.what()));
                qDebug() << str;
                throw AgrosException(str);
            }
            catch (const xml_schema::parsing& e)
            {
                QString str = QString("%1").arg(QString::fromStdString(e.what()));
                qDebug() << str;
                xml_schema::diagnostics diagnostic = e.diagnostics();
                for(int i = 0; i < diagnostic.size(); i++)
                {
                    xml_schema::error err = diagnostic.at(i);
                    qDebug() << QString("%1, position %2:%3, %4").arg(QString::fromStdString(err.id())).arg(err.line()).arg(err.column()).arg(QString::fromStdString(err.message()));
                }
                throw AgrosException(str);
            }
            catch (const xml_schema::exception& e)
            {
                qDebug() << QString("Unknow parser exception: %1").arg(QString::fromStdString(e.what()));
                throw AgrosException(QString::fromStdString(e.what()));
            }
        }
    }

    return modules;
}

/*
template <typename Scalar>
WeakFormAgros<Scalar>::WeakFormAgros(Block* block) :
    WeakForm<Scalar>(block->numSolutions()), m_block(block)
{
    m_bdf2Table = new BDF2ATable;
}

template <typename Scalar>
WeakFormAgros<Scalar>::~WeakFormAgros()
{
    foreach (Form<Scalar> *form, this->forms)
        delete form;

    this->delete_all();

    if(m_bdf2Table)
        delete m_bdf2Table;
    m_bdf2Table = NULL;
}

template <typename Scalar>
Form<Scalar> *WeakFormAgros<Scalar>::factoryForm(WeakFormKind type, const ProblemID problemId,
                                        const QString &area, FormInfo *form, Material* markerSource, Marker *markerTarget)
{
    assert(problemId.targetFieldId != "");
    assert(problemId.analysisTypeTarget != AnalysisType_Undefined);
    assert(markerTarget != nullptr);

    QString fieldId = (problemId.analysisTypeSource == AnalysisType_Undefined) ?
                problemId.targetFieldId : problemId.sourceFieldId + "-" + problemId.targetFieldId;

    PluginInterface *plugin = NULL;
    try
    {
        if (markerSource)
            plugin = Agros2D::problem()->couplingInfo(markerSource->fieldId(), markerTarget->fieldId())->plugin();
        else
            plugin = Agros2D::problem()->fieldInfo(fieldId)->plugin();
    }
    catch(...)
    {
        assert(0);
    }

    assert(plugin);

    Form<Scalar> *weakForm = NULL;

    if (type == WeakForm_MatVol)
    {
        MatrixFormVolAgros<double> *weakFormAgros = plugin->matrixFormVol(problemId, form, this, static_cast<Material *>(markerTarget));
        if (!weakFormAgros)
        {
            return NULL;
        }

        // volume
        // MeshSharedPtr initialMesh = markerTarget->fieldInfo()->initialMesh();
        // double volume = initialMesh->get_marker_area(initialMesh->get_element_markers_conversion().get_internal_marker(area.toStdString()).marker);
        // weakFormAgros->setMarkerVolume(volume);
        weakFormAgros->setMarkerVolume(-1);

        // symmetric flag
        weakFormAgros->setSymFlag(form->sym(problemId.coordinateType));
        // source marker
        weakFormAgros->setMarkerSource(markerSource);
        // target marker
        weakFormAgros->setMarkerTarget(markerTarget);

        weakForm = weakFormAgros;
    }
    else if (type == WeakForm_MatSurf)
    {
        MatrixFormSurfAgros<double> *weakFormAgros = plugin->matrixFormSurf(problemId, form, this, static_cast<Boundary *>(markerTarget));
        if (!weakFormAgros) return NULL;

        // source marker
        weakFormAgros->setMarkerTarget(markerTarget);

        weakForm = weakFormAgros;
    }
    else if (type == WeakForm_VecVol)
    {
        VectorFormVolAgros<double> *weakFormAgros = plugin->vectorFormVol(problemId, form, this, static_cast<Material *>(markerTarget));
        if (!weakFormAgros) return NULL;

        // volume
        // MeshSharedPtr initialMesh = markerTarget->fieldInfo()->initialMesh();
        // double volume = initialMesh->get_marker_area(initialMesh->get_element_markers_conversion().get_internal_marker(area.toStdString()).marker);
        // weakFormAgros->setMarkerVolume(volume);
        weakFormAgros->setMarkerVolume(-1);

        // source marker
        weakFormAgros->setMarkerSource(markerSource);
        // target marker
        weakFormAgros->setMarkerTarget(markerTarget);

        weakForm = weakFormAgros;
    }
    else if (type == WeakForm_VecSurf)
    {
        VectorFormSurfAgros<double> *weakFormAgros = plugin->vectorFormSurf(problemId, form, this, static_cast<Boundary *>(markerTarget));
        if (!weakFormAgros) return NULL;

        // source marker
        weakFormAgros->setMarkerTarget(markerTarget);

        weakForm = weakFormAgros;
    }

    if (!weakForm)
        assert(0);

    // set area
    weakForm->set_area(area.toStdString());

    return weakForm;
}

template <typename Scalar>
void WeakFormAgros<Scalar>::addForm(WeakFormKind type, Form<Scalar> *form)
{
    //    Agros2D::log()->printDebug(tr("Solver"), QString("Add form: type: %1, area: %2").
    //                            arg(weakFormString(type)).
    //                            arg(QString::fromStdString(form->getAreas().at(0))));

    if (type == WeakForm_MatVol)
        this->add_matrix_form((MatrixFormVol<Scalar>*) form);
    else if (type == WeakForm_MatSurf)
        this->add_matrix_form_surf((MatrixFormSurf<Scalar>*) form);
    else if (type == WeakForm_VecVol)
        this->add_vector_form((VectorFormVol<Scalar>*) form);
    else if (type == WeakForm_VecSurf)
        this->add_vector_form_surf((VectorFormSurf<Scalar>*) form);
    else
        assert(0);
}

template <typename Scalar>
void WeakFormAgros<Scalar>::registerForm(WeakFormKind type, FieldBlock *field, QString area, FormInfo form, Marker* marker)
{
    ProblemID problemId;

    problemId.targetFieldId = field->fieldInfo()->fieldId();
    problemId.analysisTypeTarget = field->fieldInfo()->analysisType();
    problemId.coordinateType = Agros2D::problem()->config()->coordinateType();
    problemId.linearityType = field->fieldInfo()->linearityType();

    // compiled form
    Form<Scalar> *custom_form = factoryForm(type, problemId, area, &form, NULL, marker);

    // weakform with zero coefficients
    if (!custom_form) return;

    // set time discretisation table
    if (field->fieldInfo()->analysisType() == AnalysisType_Transient)
    {
        dynamic_cast<FormAgrosInterface<Scalar> *>(custom_form)->setTimeDiscretisationTable(&m_bdf2Table);
    }

    addForm(type, custom_form);
    m_numberOfForms++;
}

template <typename Scalar>
void WeakFormAgros<Scalar>::registerFormCoupling(WeakFormKind type, QString area, FormInfo form,
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

    // qDebug() << "register form coupling " << materialSource->fieldId() << ", " << materialTarget->fieldId();

    // compiled form
    Form<Scalar> *custom_form = factoryForm(type, problemId, area, &form, materialSource, materialTarget);
    // weakform with zero coefficients
    if (!custom_form)
    {
        qDebug() << "No such coupling form found";
        return;
    }

    // TODO at the present moment, it is impossible to have more sources !
    //assert(field->m_couplingSources.size() <= 1);

    assert((type == WeakForm_MatVol) || (type == WeakForm_VecVol));

    addForm(type, custom_form);
    m_numberOfForms++;
}


template <typename Scalar>
void WeakFormAgros<Scalar>::registerForms()
{
    m_numberOfForms = 0;

    foreach(FieldInfo* fieldInfo, m_block->sourceFieldInfosCoupling())
        fieldInfo->createValuePointerTable();

    foreach(FieldBlock* field, m_block->fields())
    {
        FieldInfo* fieldInfo = field->fieldInfo();

        fieldInfo->createValuePointerTable();
        m_positionInfos[fieldInfo->numberId()].formsOffset = m_block->offset(field);

        // boundary conditions
        for (int edgeNum = 0; edgeNum<Agros2D::scene()->edges->count(); edgeNum++)
        {
            SceneBoundary *boundary = Agros2D::scene()->edges->at(edgeNum)->marker(fieldInfo);
            if (boundary && boundary != Agros2D::scene()->boundaries->getNone(fieldInfo))
            {
                Module::BoundaryType boundaryType = fieldInfo->boundaryType(boundary->type());

                foreach (FormInfo expression, boundaryType.wfMatrixSurface())
                {
                    expression.i += m_block->offset(field);
                    expression.j += m_block->offset(field);
                    registerForm(WeakForm_MatSurf, field, QString::number(edgeNum), expression, boundary);
                }

                foreach (FormInfo expression, boundaryType.wfVectorSurface())
                {
                    expression.i += m_block->offset(field);
                    expression.j += m_block->offset(field);
                    registerForm(WeakForm_VecSurf, field, QString::number(edgeNum), expression, boundary);
                }
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
                foreach (FormInfo expression, Module::wfMatrixVolumeSeparated(fieldInfo->plugin()->module(), fieldInfo->analysisType(), fieldInfo->linearityType()))
                {
                    expression.i += m_block->offset(field);
                    expression.j += m_block->offset(field);
                    registerForm(WeakForm_MatVol, field, QString::number(labelNum), expression, material);
                }

                foreach (FormInfo expression, Module::wfVectorVolumeSeparated(fieldInfo->plugin()->module(), fieldInfo->analysisType(), fieldInfo->linearityType()))
                {
                    expression.i += m_block->offset(field);
                    expression.j += m_block->offset(field);
                    registerForm(WeakForm_VecVol, field, QString::number(labelNum), expression, material);
                }

                // weak coupling
                foreach(CouplingInfo* couplingInfo, field->couplingInfos())
                {
                    foreach (FormInfo expression, couplingInfo->wfVectorVolumeSeparated(&couplingInfo->plugin()->coupling()->volume(),
                                                                                        couplingInfo->sourceField()->analysisType(),
                                                                                        couplingInfo->targetField()->analysisType(),
                                                                                        couplingInfo->couplingType(),
                                                                                        couplingInfo->linearityType()))
                    {
                        SceneMaterial* materialSource = Agros2D::scene()->labels->at(labelNum)->marker(couplingInfo->sourceField());
                        assert(materialSource);

                        if (materialSource != Agros2D::scene()->materials->getNone(couplingInfo->sourceField()))
                        {
                            expression.i += m_block->offset(field);
                            expression.j += m_block->offset(field);
                            registerFormCoupling(WeakForm_VecVol, QString::number(labelNum), expression, materialSource, material, couplingInfo);
                        }
                    }

                    // todo remove code repetition
                    foreach (FormInfo expression, couplingInfo->wfMatrixVolumeSeparated(&couplingInfo->plugin()->coupling()->volume(),
                                                                                        couplingInfo->sourceField()->analysisType(),
                                                                                        couplingInfo->targetField()->analysisType(),
                                                                                        couplingInfo->couplingType(),
                                                                                        couplingInfo->linearityType()))
                    {
                        SceneMaterial* materialSource = Agros2D::scene()->labels->at(labelNum)->marker(couplingInfo->sourceField());
                        assert(materialSource);

                        if (materialSource != Agros2D::scene()->materials->getNone(couplingInfo->sourceField()))
                        {
                            expression.i += m_block->offset(field);
                            expression.j += m_block->offset(field);
                            registerFormCoupling(WeakForm_MatVol, QString::number(labelNum), expression, materialSource, material, couplingInfo);
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
            FieldBlock* sourceField = m_block->field(couplingInfo->sourceField());
            FieldBlock* targetField = m_block->field(couplingInfo->targetField());

            for (int labelNum = 0; labelNum<Agros2D::scene()->labels->count(); labelNum++)
            {
                SceneMaterial *sourceMaterial = Agros2D::scene()->labels->at(labelNum)->marker(sourceField->fieldInfo());
                SceneMaterial *targetMaterial = Agros2D::scene()->labels->at(labelNum)->marker(targetField->fieldInfo());

                if (sourceMaterial && (sourceMaterial != Agros2D::scene()->materials->getNone(sourceField->fieldInfo()))
                        && targetMaterial && (targetMaterial != Agros2D::scene()->materials->getNone(targetField->fieldInfo())))
                {

                    qDebug() << "hard coupling form on marker " << labelNum;

                    qDebug() << "stav: offset source " << m_block->offset(sourceField) << ", target " << m_block->offset(targetField) << ", ns" << sourceField->fieldInfo()->numberOfSolutions();
                    foreach (FormInfo expression, couplingInfo->wfMatrixVolumeSeparated(&couplingInfo->plugin()->coupling()->volume(), sourceField->fieldInfo()->analysisType(), targetField->fieldInfo()->analysisType(), couplingInfo->couplingType(), couplingInfo->linearityType()))
                    {
                        expression.i += m_block->offset(targetField) - sourceField->fieldInfo()->numberOfSolutions();
                        expression.j += m_block->offset(sourceField);
                        qDebug() << "(i, j) = " << expression.i << ", " << expression.j;
                        registerFormCoupling(WeakForm_MatVol, QString::number(labelNum), expression, sourceMaterial, targetMaterial, couplingInfo);
                    }

                    foreach (FormInfo expression, couplingInfo->wfVectorVolumeSeparated(&couplingInfo->plugin()->coupling()->volume(), sourceField->fieldInfo()->analysisType(), targetField->fieldInfo()->analysisType(), couplingInfo->couplingType(), couplingInfo->linearityType()))
                    {
                        expression.i += m_block->offset(targetField) - sourceField->fieldInfo()->numberOfSolutions();
                        expression.j += m_block->offset(sourceField);
                        qDebug() << "(i, j) = " << expression.i << ", " << expression.j;
                        registerFormCoupling(WeakForm_VecVol, QString::number(labelNum), expression, sourceMaterial, targetMaterial, couplingInfo);
                    }

                }
            }
        }
    }
    //qDebug() << QString("registered %1 forms").arg(m_numberOfForms);
}

template <typename Scalar>
std::vector<UExtFunctionSharedPtr<Scalar> > WeakFormAgros<Scalar>::quantitiesAndSpecialFunctions(const FieldInfo* fieldInfo, bool linearize) const
{
    std::vector<UExtFunctionSharedPtr<Scalar> > result;

    XMLModule::field* module = fieldInfo->plugin()->module();
    ProblemID problemId;
    problemId.targetFieldId = fieldInfo->fieldId();
    problemId.analysisTypeTarget = fieldInfo->analysisType();
    problemId.coordinateType = Agros2D::problem()->config()->coordinateType();
    problemId.linearityType = fieldInfo->linearityType();

    assert(fieldInfo->numberId() < MAX_FIELDS);

    XMLModule::weakform_volume weakform = module->volume().weakforms_volume().weakform_volume().at(0);
    foreach(XMLModule::weakform_volume weakformTest, module->volume().weakforms_volume().weakform_volume())
    {
        if(analysisTypeFromStringKey(QString::fromStdString(weakformTest.analysistype())) == fieldInfo->analysisType())
        {
            weakform = weakformTest;
            break;
        }
    }

    // first register Values
    QMap<QString, int> quantityOrdering;
    QMap<QString, bool> quantityIsNonlin;
    QMap<QString, int> functionOrdering;
    Module::volumeQuantityProperties(module, quantityOrdering, quantityIsNonlin, functionOrdering);
    QList<int> numbers = quantityOrdering.values();
    qSort(numbers);
    foreach(int index, numbers)
    {
        QString quantityID = quantityOrdering.key(index);

        bool containedInAnalysis = false;
        foreach(XMLModule::quantity quantity, weakform.quantity())
        {
            if(QString::fromStdString(quantity.id()) == quantityID)
            {
                containedInAnalysis = true;
                break;
            }
        }

        UExtFunctionSharedPtr<Scalar> extFunction;
        if(containedInAnalysis)
        {
            bool linearizeThis = quantityIsNonlin[quantityID] && linearize;
            extFunction = UExtFunctionSharedPtr<Scalar>(fieldInfo->plugin()->extFunction(problemId, quantityID, false, linearizeThis, this));
        }
        else
        {
            extFunction = UExtFunctionSharedPtr<Scalar>(new AgrosEmptyExtFunction());
        }

        result.push_back(extFunction);

        // for nonlinear quantities, register derivative as well
        if(quantityIsNonlin[quantityID])
        {
            extFunction = NULL;
            if(containedInAnalysis && !linearize)
                extFunction = UExtFunctionSharedPtr<Scalar>(fieldInfo->plugin()->extFunction(problemId, quantityID, true, false, this));
            else
                // pass an empty functions if the quantity is not contained in the given analysis or should be linearized
                // the reason is that we can use uniform indexing ext[n] in the weak forms (the same n for all analysis)
                // as a result we can generate only one variant of each form (although there are more variants of ext functions because of different dependencies of nonlinear and time dependent terms)
                extFunction = UExtFunctionSharedPtr<Scalar>(new AgrosEmptyExtFunction());

            assert(extFunction);
            result.push_back(extFunction);
        }
    }

    // register special ext functions
    numbers = functionOrdering.values();
    qSort(numbers);

    foreach(int index, numbers)
    {
        QString functionID = functionOrdering.key(index);

        bool containedInAnalysis = false;
        foreach(XMLModule::function_use functionUse, weakform.function_use())
        {
            if(QString::fromStdString(functionUse.id()) == functionID)
            {
                containedInAnalysis = true;
                break;
            }
        }
        UExtFunctionSharedPtr<Scalar> extFunction;
        if(containedInAnalysis)
        {
            UExtFunction<Scalar> *extFunctionPtr = fieldInfo->plugin()->extFunction(problemId, functionID, false, linearize, this);
            assert(extFunctionPtr);
            extFunction = UExtFunctionSharedPtr<Scalar>(extFunctionPtr);
        }
        else
            extFunction = UExtFunctionSharedPtr<Scalar>(new AgrosEmptyExtFunction());

        result.push_back(extFunction);
    }

    return result;
}

template <typename Scalar>
std::vector<MeshFunctionSharedPtr<Scalar> > WeakFormAgros<Scalar>::previousTimeLevelsSolutions(const FieldInfo* fieldInfo) const
{
    std::vector<MeshFunctionSharedPtr<Scalar> > result;

    assert(m_bdf2Table);

    //m_offsetCouplingExt = m_offsetPreviousTimeExt + m_bdf2Table->n() * transientFieldInfo->numberOfSolutions() ;

    int lastTimeStep = Agros2D::problem()->actualTimeStep() - 1; // todo: check

    for(int backLevel = 0; backLevel < m_bdf2Table->n(); backLevel++)
    {
        int timeStep = lastTimeStep - backLevel;
        int adaptivityStep = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo, SolutionMode_Normal, timeStep);
        FieldSolutionID solutionID(fieldInfo, timeStep, adaptivityStep, SolutionMode_Reference);
        if(! Agros2D::solutionStore()->contains(solutionID))
            solutionID.solutionMode = SolutionMode_Normal;
        assert(Agros2D::solutionStore()->contains(solutionID));

        // for (int comp = 0; comp < solutionID.group->numberOfSolutions(); comp++)
        //     result.push_back(Agros2D::solutionStore()->multiArray(solutionID).solutions().at(comp));
    }

    return result;
}

template <typename Scalar>
std::vector<MeshFunctionSharedPtr<Scalar> > WeakFormAgros<Scalar>::sourceCouplingSolutions(const FieldInfo* fieldInfo) const
{
    std::vector<MeshFunctionSharedPtr<Scalar> > result;

    FieldSolutionID solutionID = Agros2D::solutionStore()->lastTimeAndAdaptiveSolution(fieldInfo, SolutionMode_Finer);

    for (int comp = 0; comp < solutionID.group->numberOfSolutions(); comp++)
        result.push_back(Agros2D::solutionStore()->multiArray(solutionID).solutions().at(comp));

    return result;
}

template <typename Scalar>
void WeakFormAgros<Scalar>::updateExtField()
{
    // implicit values. Values have to be renewed each step, since e.g.number of previous time solutions may vary due to changing BDF order
    for(int i = 0; i < MAX_FIELDS; i++)
        m_positionInfos[i] = PositionInfo();

    // register two types of external functions. Quantities and special functions of all fields go to externalUSlns
    std::vector<UExtFunctionSharedPtr<Scalar> > externalUSlns;
    std::vector<UExtFunctionSharedPtr<Scalar> > fieldUExt;

    // first push external functions related to source fields (source fields weakly coupled to some of the field in the block)
    foreach(FieldInfo* fieldInfo, m_block->sourceFieldInfosCoupling())
    {
        // quantities have to be linearized (field that they depend on has allready been solved, now we are solving different
        // field and thus 1. u_ext does not contain the source field and 2. it would be unnecessary anyway)
        fieldUExt = quantitiesAndSpecialFunctions(fieldInfo, true);

        m_positionInfos[fieldInfo->numberId()].numQuantAndSpecFun = fieldUExt.size();
        m_positionInfos[fieldInfo->numberId()].quantAndSpecOffset = externalUSlns.size();
        m_positionInfos[fieldInfo->numberId()].isSource = true;

        externalUSlns.insert(externalUSlns.end(), fieldUExt.begin(), fieldUExt.end());
    }

    // next push external functions related to fields contained in the block (if more, than hard - coupled)
    foreach(FieldBlock* field, m_block->fields())
    {
        FieldInfo* fieldInfo = field->fieldInfo();

        // inside the block use nonlinear quantities
        fieldUExt = quantitiesAndSpecialFunctions(fieldInfo, false);

        m_positionInfos[fieldInfo->numberId()].formsOffset = m_block->offset(field);
        m_positionInfos[fieldInfo->numberId()].numQuantAndSpecFun = fieldUExt.size();
        m_positionInfos[fieldInfo->numberId()].quantAndSpecOffset = externalUSlns.size();
        m_positionInfos[fieldInfo->numberId()].isSource = false;

        externalUSlns.insert(externalUSlns.end(), fieldUExt.begin(), fieldUExt.end());
    }

    // register second type of external functions. Weak coupling sources and previous time solutions go to externalSlns
    // the numbering, however, have to be done together, since in ext field in forms, no difference is made
    // by Hermes convention, USlns go first, than go Slns
    std::vector<MeshFunctionSharedPtr<Scalar> > externalSlns;
    std::vector<MeshFunctionSharedPtr<Scalar> > fieldExt;

    // push external solutions for previous time levels
    // for each field, we add all field components of previous time level, than all components of time level -2, than all
    // components of level -3, etc, depending of order of BDF used.
    foreach(FieldBlock* field, m_block->fields())
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if(fieldInfo->analysisType() == AnalysisType_Transient)
        {
            fieldExt = previousTimeLevelsSolutions(fieldInfo);

            m_positionInfos[fieldInfo->numberId()].numPreviousSolutions = fieldExt.size();
            m_positionInfos[fieldInfo->numberId()].previousSolutionsOffset = externalUSlns.size() + externalSlns.size();

            externalSlns.insert(externalSlns.end(), fieldExt.begin(), fieldExt.end());
        }
    }

    // push external solution for weak couplings
    foreach(FieldInfo* fieldInfo, m_block->sourceFieldInfosCoupling())
    {
        fieldExt = sourceCouplingSolutions(fieldInfo) ;

        m_positionInfos[fieldInfo->numberId()].numPreviousSolutions = fieldExt.size();
        m_positionInfos[fieldInfo->numberId()].previousSolutionsOffset = externalUSlns.size() + externalSlns.size();

        externalSlns.insert(externalSlns.end(), fieldExt.begin(), fieldExt.end());
    }

    // set both types of external functions
    this->set_u_ext_fn(externalUSlns);
    this->set_ext(externalSlns);

    // outputPositionInfos();
    // qDebug() << "total number of u_ext_fn: " << externalUSlns.size() << " and ext_fn: " << externalSlns.size();
}
*/

template <typename SectionWithTemplates>
QList<FormInfo> wfMatrixTemplates(SectionWithTemplates *section)
{
    // matrix weakforms
    QList<FormInfo> weakForms;
    // weakform
    for (unsigned int i = 0; i < section->matrix_form().size(); i++)
    {
        XMLModule::matrix_form form = section->matrix_form().at(i);
        assert(form.i().present() && form.j().present() && form.planar().present() && form.axi().present());
        SymFlag symPlanar = HERMES_NONSYM;
        SymFlag symAxi = HERMES_NONSYM;
        if(form.symmetric().present())
        {
            symPlanar = (SymFlag)form.symmetric().get();
            symAxi = (SymFlag)form.symmetric().get();
        }
        if(form.symmetric_planar().present())
        {
            symPlanar = (SymFlag)form.symmetric_planar().get();
        }
        if(form.symmetric_axi().present())
        {
            symAxi = (SymFlag)form.symmetric_axi().get();
        }
        FormInfo formInfo(QString::fromStdString(form.id()),
                          form.i().get(),
                          form.j().get(),
                          symPlanar,
                          symAxi);
        formInfo.condition = form.condition().present() ? QString::fromStdString(form.condition().get()) : "";
        formInfo.expr_planar = QString::fromStdString(form.planar().get());
        formInfo.expr_axi = QString::fromStdString(form.axi().get());
        weakForms.append(formInfo);
    }

    return weakForms;
}

template <typename SectionWithTemplates>
QList<FormInfo> wfVectorTemplates(SectionWithTemplates *section)
{
    // vector weakforms
    QList<FormInfo> weakForms;
    for (unsigned int i = 0; i < section->vector_form().size(); i++)
    {
        XMLModule::vector_form form = section->vector_form().at(i);
        assert(form.i().present() && form.j().present() && form.planar().present() && form.axi().present());
        FormInfo formInfo(QString::fromStdString(form.id()),
                          form.i().get(),
                          form.j().get());
        formInfo.condition = form.condition().present() ? QString::fromStdString(form.condition().get()) : "";
        formInfo.expr_planar = QString::fromStdString(form.planar().get());
        formInfo.expr_axi = QString::fromStdString(form.axi().get());
        weakForms.append(formInfo);
    }

    return weakForms;
}

QList<FormInfo> wfEssentialTemplates(XMLModule::surface *surface)
{
    // vector weakforms
    QList<FormInfo> weakForms;
    for (unsigned int i = 0; i < surface->essential_form().size(); i++)
    {
        XMLModule::essential_form form = surface->essential_form().at(i);
        assert(form.i().present() && form.planar().present() && form.axi().present());
        FormInfo formInfo(QString::fromStdString(form.id()),
                          form.i().get());
        formInfo.condition = form.condition().present() ? QString::fromStdString(form.condition().get()) : "";
        formInfo.expr_planar = QString::fromStdString(form.planar().get());
        formInfo.expr_axi = QString::fromStdString(form.axi().get());
        weakForms.append(formInfo);
    }

    return weakForms;
}

XMLModule::linearity_option findLinearityOption(XMLModule::volume *volume, AnalysisType analysisType, LinearityType linearityType)
{
    for (unsigned int i = 0; i < volume->weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume wf = volume->weakforms_volume().weakform_volume().at(i);

        if (wf.analysistype() == analysisTypeToStringKey(analysisType).toStdString())
        {
            for(unsigned int i = 0; i < wf.linearity_option().size(); i++)
            {
                XMLModule::linearity_option lo = wf.linearity_option().at(i);
                if(lo.type() == linearityTypeToStringKey(linearityType).toStdString())
                {
                    return lo;
                }
            }
        }
    }
}

XMLModule::linearity_option findLinearityOption(XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType)
{
    for(unsigned int i = 0; i < boundary->linearity_option().size(); i++)
    {
        XMLModule::linearity_option lo = boundary->linearity_option().at(i);
        if(lo.type() == linearityTypeToStringKey(linearityType).toStdString())
        {
            return lo;
        }
    }
}

template <typename SectionWithElements>
QList<FormInfo> wfMatrixElements(SectionWithElements *section, AnalysisType analysisType, LinearityType linearityType)
{
    // matrix weakforms
    QList<FormInfo> weakForms;
    XMLModule::linearity_option lo = findLinearityOption(section, analysisType, linearityType);

    for (unsigned int i = 0; i < lo.matrix_form().size(); i++)
    {
        XMLModule::matrix_form form = lo.matrix_form().at(i);
        FormInfo formInfo(QString::fromStdString(form.id()));
        weakForms.append(formInfo);
    }

    return weakForms;
}

template <typename SectionWithElements>
QList<FormInfo> wfVectorElements(SectionWithElements *section, AnalysisType analysisType, LinearityType linearityType)
{
    // vector weakforms
    QList<FormInfo> weakForms;
    XMLModule::linearity_option lo = findLinearityOption(section, analysisType, linearityType);

    for (unsigned int i = 0; i < lo.vector_form().size(); i++)
    {
        XMLModule::vector_form form = lo.vector_form().at(i);
        FormInfo formInfo(QString::fromStdString(form.id()));
        if(form.variant().present())
            formInfo.variant = weakFormVariantFromStringKey(QString::fromStdString(form.variant().get()));
        if(form.coefficient().present())
            formInfo.coefficient = QString::fromStdString(form.coefficient().get()).toDouble();
        weakForms.append(formInfo);
    }

    return weakForms;
}

QList<FormInfo> wfEssentialElements(XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType)
{
    // essential
    QList<FormInfo> weakForms;
    XMLModule::linearity_option lo = findLinearityOption(boundary, analysisType, linearityType);

    for (unsigned int i = 0; i < lo.essential_form().size(); i++)
    {
        XMLModule::essential_form form = lo.essential_form().at(i);
        FormInfo formInfo(QString::fromStdString(form.id()));
        weakForms.append(formInfo);
    }

    return weakForms;
}

QList<FormInfo> Module::wfMatrixVolumeSeparated(XMLModule::field* module, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> templates = wfMatrixTemplates(&module->volume());
    QList<FormInfo> elements = wfMatrixElements(&module->volume(), analysisType, linearityType);

    return generateSeparated(elements, templates);
}

QList<FormInfo> Module::wfVectorVolumeSeparated(XMLModule::field* module, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> templatesVector = wfVectorTemplates(&module->volume());
    QList<FormInfo> templatesMatrix = wfMatrixTemplates(&module->volume());
    QList<FormInfo> elements = wfVectorElements(&module->volume(), analysisType, linearityType);

    return generateSeparated(elements, templatesVector, templatesMatrix);
}

QList<FormInfo> Module::wfMatrixSurface(XMLModule::surface *surface, XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> matrixTemplates = wfMatrixTemplates(surface);
    QList<FormInfo> matrixElements = wfMatrixElements<XMLModule::boundary>(boundary, analysisType, linearityType);
    return generateSeparated(matrixElements, matrixTemplates);
}

QList<FormInfo> Module::wfVectorSurface(XMLModule::surface *surface, XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> vectorTemplates = wfVectorTemplates(surface);
    QList<FormInfo> matrixTemplates = wfMatrixTemplates(surface);
    QList<FormInfo> vectorElements = wfVectorElements<XMLModule::boundary>(boundary, analysisType, linearityType);
    return generateSeparated(vectorElements, vectorTemplates, matrixTemplates);
}

QList<FormInfo> Module::essential(XMLModule::surface *surface, XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> essentialTemplates = wfEssentialTemplates(surface);
    QList<FormInfo> essentialElements = wfEssentialElements(boundary, analysisType, linearityType);
    return generateSeparated(essentialElements, essentialTemplates);
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

    if (quant.is_bool().present())
        m_isBool = (quant.is_bool().get() != 0);
    else
        m_isBool = false;

    if (quant.only_if().present())
        m_onlyIf = QString::fromStdString(quant.only_if().get());

    if (quant.only_if_not().present())
        m_onlyIfNot = QString::fromStdString(quant.only_if_not().get());

    if (quant.is_source().present())
        m_isSource = (quant.is_source().get() != 0);
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

    m_wfMatrix = Module::wfMatrixSurface(&fieldInfo->plugin()->module()->surface(), &bdy, fieldInfo->analysisType(), fieldInfo->linearityType());
    m_wfVector = Module::wfVectorSurface(&fieldInfo->plugin()->module()->surface(), &bdy, fieldInfo->analysisType(), fieldInfo->linearityType());
    m_essential = Module::essential(&fieldInfo->plugin()->module()->surface(), &bdy, fieldInfo->analysisType(), fieldInfo->linearityType());
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
    m_wfMatrix.clear();
    m_wfVector.clear();
    m_essential.clear();
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

AGROS_LIBRARY_API void Module::updateTimeFunctions(double time)
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

void findVolumeLinearityOption(XMLModule::linearity_option& option, XMLModule::field *module, AnalysisType analysisType, LinearityType linearityType)

{
    for (unsigned int i = 0; i < module->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume wf = module->volume().weakforms_volume().weakform_volume().at(i);

        if (wf.analysistype() == analysisTypeToStringKey(analysisType).toStdString())
        {
            for(unsigned int i = 0; i < wf.linearity_option().size(); i++)
            {
                XMLModule::linearity_option lo = wf.linearity_option().at(i);
                if(lo.type() == linearityTypeToStringKey(linearityType).toStdString())
                {
                    option = lo;
                }
            }
        }
    }
}

void Module::volumeQuantityProperties(XMLModule::field *module, QMap<QString, int> &quantityOrder, QMap<QString, bool> &quantityIsNonlin, QMap<QString, int> &functionOrder)
{
    int nextIndex = 0;
    foreach(XMLModule::quantity quantity, module->volume().quantity())
    {
        QString quantityId = QString::fromStdString(quantity.id());
        quantityOrder[quantityId] = nextIndex;
        nextIndex++;
        quantityIsNonlin[quantityId] = false;
        foreach(XMLModule::weakform_volume weakform, module->volume().weakforms_volume().weakform_volume())
        {
            foreach(XMLModule::quantity quantityInAnalysis, weakform.quantity())
            {
                if(quantity.id() == quantityInAnalysis.id())
                {
                    if(quantityInAnalysis.nonlinearity_axi().present() || quantityInAnalysis.nonlinearity_planar().present())
                    {
                        quantityIsNonlin[quantityId] = true;
                    }
                }
            }
        }

        // if the quantity is nonlinear, we have to reserve space for its derivative as well
        if(quantityIsNonlin[quantityId])
            nextIndex++;
    }

    foreach(XMLModule::function function, module->volume().function())
    {
        QString functionID = QString::fromStdString(function.id());
        functionOrder[functionID] = nextIndex;
        nextIndex++;
    }
}
