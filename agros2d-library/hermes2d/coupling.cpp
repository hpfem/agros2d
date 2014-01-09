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

#include "coupling.h"

#include "util/constants.h"
#include "util/global.h"

#include "logview.h"
#include "scene.h"
#include "util.h"
#include "module.h"
#include "weak_form.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/plugin_interface.h"

#include "../../resources_source/classes/module_xml.h"

static CouplingList *m_couplingList = NULL;
CouplingList *couplingList()
{
    if (!m_couplingList)
        m_couplingList = new CouplingList();

    return m_couplingList;
}

CouplingList::CouplingList()
{
    // read couplings
    QDir dir(datadir() + COUPLINGROOT);

    QStringList filter;
    filter << "*.xml";
    QStringList list = dir.entryList(filter);

    foreach (QString filename, list)
    {
        try
        {
            // todo: this was copied from module. Find a way to do all catching at one place

            // todo: find a way to validate if required. If validated here, sensible error messages will be obtained
            bool validateAtTheBeginning = true;
            ::xml_schema::flags parsing_flags = xml_schema::flags::dont_validate;
            if(validateAtTheBeginning)
            {
                parsing_flags = 0;
                qDebug() << "Warning: Validating all XML coupling files. This is time-consuming and should be switched off in coupling.cpp for release. Set validateAtTheBeginning = false.";
            }

            std::auto_ptr<XMLModule::module> module_xsd = XMLModule::module_(compatibleFilename(datadir() + COUPLINGROOT + "/" + filename).toStdString(),
                                                                                   xml_schema::flags::dont_validate & xml_schema::flags::dont_initialize);
            XMLModule::module *mod = module_xsd.get();
            assert(mod->coupling().present());
            XMLModule::coupling *coup = &mod->coupling().get();

            // check whether coupling is available for values of source and target fields such as analysis type
            for (int i = 0; i < coup->volume().weakforms_volume().weakform_volume().size(); i++)
            {
                XMLModule::weakform_volume wf = coup->volume().weakforms_volume().weakform_volume().at(i);

                CouplingList::Item item;

                item.sourceField = QString::fromStdString(coup->general_coupling().modules().source().id());
                item.sourceAnalysisType = analysisTypeFromStringKey(QString::fromStdString(wf.sourceanalysis().get()));
                item.targetField = QString::fromStdString(coup->general_coupling().modules().target().id());
                item.targetAnalysisType = analysisTypeFromStringKey(QString::fromStdString(wf.analysistype()));
                item.couplingType = couplingTypeFromStringKey(QString::fromStdString(wf.couplingtype().get()));

                m_couplings.append(item);
            }
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

QList<QString> CouplingList::availableCouplings()
{
    QList<QString> couplings;

    foreach (Item item, m_couplings)
    {
        QString couplingId = QString("%1-%2").arg(item.sourceField).arg(item.targetField);
        if (!couplings.contains(couplingId))
            couplings.append(couplingId);
    }

    return couplings;
}

bool CouplingList::isCouplingAvailable(FieldInfo *sourceField, FieldInfo *targetField)
{    
    foreach (Item item, m_couplings)
    {
        if (item.sourceAnalysisType == sourceField->analysisType() && item.targetAnalysisType == targetField->analysisType()
                && item.sourceField == sourceField->fieldId() && item.targetField == targetField->fieldId())
            return true;
    }

    return false;
}

CouplingInfo::CouplingInfo(FieldInfo *sourceField,
                           FieldInfo *targetField,
                           CouplingType couplingType) :
    m_plugin(NULL),
    m_sourceField(sourceField), m_targetField(targetField),
    m_couplingType(couplingType)
{
    reload();
}

CouplingInfo::~CouplingInfo()
{
    delete m_plugin;
}

void CouplingInfo::setCouplingType(CouplingType couplingType)
{
    m_couplingType = couplingType;
    reload();
}

CouplingType CouplingInfo::couplingType() const
{
//    if(m_couplingType == CouplingType_Hard)
//    {
//        Agros2D::log()->printDebug(QObject::tr("Solver"), QObject::tr("Hard coupling is not available yet, switching to weak coupling"));
//        return CouplingType_Weak;
//    }
    return m_couplingType;
}

void CouplingInfo::reload()
{
    // coupling id
    m_couplingId = m_sourceField->fieldId() + "-" + m_targetField->fieldId();

    // read plugin
    if (m_plugin)
        delete m_plugin;

    try{
        m_plugin = Agros2D::loadPlugin(m_couplingId);
    }
    catch (AgrosPluginException &e)
    {
        Agros2D::log()->printError("Solver", "Cannot load plugin");
        throw;
    }

    assert(m_plugin);
}

// name
QString CouplingInfo::name() const
{
    return QString::fromStdString(m_plugin->coupling()->general_coupling().name());
}

// description
QString CouplingInfo::description() const
{
    return QString::fromStdString(m_plugin->coupling()->general_coupling().description());
}

// constants
QMap<QString, double> CouplingInfo::constants()
{
    QMap<QString, double> constants;
    // constants
    foreach (XMLModule::constant cnst, m_plugin->coupling()->constants().constant())
        constants[QString::fromStdString(cnst.id())] = cnst.value();

    return constants;
}



// todo: copied!
// todo: copied!
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
        Hermes::Hermes2D::SymFlag symPlanar = Hermes::Hermes2D::HERMES_NONSYM;
        Hermes::Hermes2D::SymFlag symAxi = Hermes::Hermes2D::HERMES_NONSYM;
        if(form.symmetric().present())
        {
            symPlanar = (Hermes::Hermes2D::SymFlag)form.symmetric().get();
            symAxi = (Hermes::Hermes2D::SymFlag)form.symmetric().get();
        }
        if(form.symmetric_planar().present())
        {
            symPlanar = (Hermes::Hermes2D::SymFlag)form.symmetric_planar().get();
        }
        if(form.symmetric_axi().present())
        {
            symAxi = (Hermes::Hermes2D::SymFlag)form.symmetric_axi().get();
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



// todo: copied!
// todo: copied!
XMLModule::linearity_option findLinearityOption(XMLModule::volume *volume, AnalysisType analysisTypeSource, AnalysisType analysisTypeTarget, CouplingType couplingType, LinearityType linearityType)
{
    for (unsigned int i = 0; i < volume->weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume wf = volume->weakforms_volume().weakform_volume().at(i);

        if ((wf.sourceanalysis().get() == analysisTypeToStringKey(analysisTypeSource).toStdString()) &&
                (wf.analysistype() == analysisTypeToStringKey(analysisTypeTarget).toStdString()) &&
                (wf.couplingtype().get() == couplingTypeToStringKey(couplingType).toStdString()))
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

// todo: copied!
// todo: copied!
template <typename SectionWithElements>
QList<FormInfo> wfMatrixElements(SectionWithElements *section, AnalysisType analysisTypeSource, AnalysisType analysisTypeTarget, CouplingType couplingType, LinearityType linearityType)
{
    // matrix weakforms
    QList<FormInfo> weakForms;
    XMLModule::linearity_option lo = findLinearityOption(section, analysisTypeSource, analysisTypeTarget, couplingType, linearityType);

    for (unsigned int i = 0; i < lo.matrix_form().size(); i++)
    {
        XMLModule::matrix_form form = lo.matrix_form().at(i);
        FormInfo formInfo(QString::fromStdString(form.id()));
        weakForms.append(formInfo);
    }

    return weakForms;
}

// todo: copied!
// todo: copied!
template <typename SectionWithElements>
QList<FormInfo> wfVectorElements(SectionWithElements *section, AnalysisType analysisTypeSource, AnalysisType analysisTypeTarget, CouplingType couplingType, LinearityType linearityType)
{
    // vector weakforms
    QList<FormInfo> weakForms;
    XMLModule::linearity_option lo = findLinearityOption(section, analysisTypeSource, analysisTypeTarget, couplingType, linearityType);

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


// weak forms
QList<FormInfo> CouplingInfo::wfMatrixVolumeSeparated(XMLModule::volume* volume, AnalysisType sourceAnalysis, AnalysisType targetAnalysis, CouplingType couplingType, LinearityType linearityType)
{
    QList<FormInfo> templates = wfMatrixTemplates(volume);
    QList<FormInfo> elements = wfMatrixElements(volume, sourceAnalysis, targetAnalysis, couplingType, linearityType);

    return generateSeparated(elements, templates);
}

QList<FormInfo> CouplingInfo::wfVectorVolumeSeparated(XMLModule::volume* volume, AnalysisType sourceAnalysis, AnalysisType targetAnalysis, CouplingType couplingType, LinearityType linearityType)
{
    QList<FormInfo> templatesVector = wfVectorTemplates(volume);
    QList<FormInfo> templatesMatrix = wfMatrixTemplates(volume);
    QList<FormInfo> elements = wfVectorElements(volume, sourceAnalysis, targetAnalysis, couplingType, linearityType);

    return generateSeparated(elements, templatesVector, templatesMatrix);
}

LinearityType CouplingInfo::linearityType()
{
    // TODO: FIX - warning
    if (couplingType() == CouplingType_Hard)
        assert(sourceField()->linearityType() == targetField()->linearityType());

    return targetField()->linearityType();
}
