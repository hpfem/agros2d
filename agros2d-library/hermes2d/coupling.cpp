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

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/plugin_interface.h"

#include "../../resources_source/classes/coupling_xml.h"

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
                qDebug() << "Warning: Validating all XML files. This is time-consuming and should be switched off in coupling.cpp for release. Set validateAtTheBeginning = false.";
            }

            std::auto_ptr<XMLCoupling::coupling> coupling_xsd = XMLCoupling::coupling_(compatibleFilename(datadir() + COUPLINGROOT + "/" + filename).toStdString(),
                                                                                   xml_schema::flags::dont_validate & xml_schema::flags::dont_initialize);
            XMLCoupling::coupling *coup = coupling_xsd.get();

            // check whether coupling is available for values of source and target fields such as analysis type
            for (int i = 0; i < coup->volume().weakforms_volume().weakform_volume().size(); i++)
            {
                XMLCoupling::weakform_volume wf = coup->volume().weakforms_volume().weakform_volume().at(i);

                CouplingList::Item item;

                item.sourceField = QString::fromStdString(coup->general().modules().source().id());
                item.sourceAnalysisType = analysisTypeFromStringKey(QString::fromStdString(wf.sourceanalysis()));
                item.targetField = QString::fromStdString(coup->general().modules().target().id());
                item.targetAnalysisType = analysisTypeFromStringKey(QString::fromStdString(wf.targetanalysis()));
                item.couplingType = couplingTypeFromStringKey(QString::fromStdString(wf.couplingtype()));

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
    if(m_couplingType == CouplingType_Hard)
    {
        Agros2D::log()->printDebug(QObject::tr("Solver"), QObject::tr("Hard coupling is not available yet, switching to weak coupling"));
        return CouplingType_Weak;
    }
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
    return QString::fromStdString(m_plugin->coupling()->general().name());
}

// description
QString CouplingInfo::description() const
{
    return QString::fromStdString(m_plugin->coupling()->general().description());
}

// constants
QMap<QString, double> CouplingInfo::constants()
{
    QMap<QString, double> constants;
    // constants
    foreach (XMLCoupling::constant cnst, m_plugin->coupling()->constants().constant())
        constants[QString::fromStdString(cnst.id())] = cnst.value();

    return constants;
}

// weak forms
QList<FormInfo> CouplingInfo::wfMatrixVolume() const
{
    // matrix weakforms
    QList<FormInfo> wfMatrixVolume;
    for (int i = 0; i < m_plugin->coupling()->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLCoupling::weakform_volume wf = m_plugin->coupling()->volume().weakforms_volume().weakform_volume().at(i);

        if ((wf.couplingtype() == couplingTypeToStringKey(this->couplingType()).toStdString()) &&
                (wf.sourceanalysis() == analysisTypeToStringKey(m_sourceField->analysisType()).toStdString()) &&
                (wf.targetanalysis() == analysisTypeToStringKey(m_targetField->analysisType()).toStdString()))
        {
            for (int i = 0; i < wf.matrix_form().size(); i++)
            {
                XMLCoupling::matrix_form form = wf.matrix_form().at(i);
                wfMatrixVolume.push_back(FormInfo(QString::fromStdString(form.id()), form.i(), form.j()));
            }
        }
    }

    return wfMatrixVolume;
}

QList<FormInfo> CouplingInfo::wfVectorVolume() const
{
    // vector weakforms
    QList<FormInfo> wfVectorVolume;
    for (int i = 0; i < m_plugin->coupling()->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLCoupling::weakform_volume wf = m_plugin->coupling()->volume().weakforms_volume().weakform_volume().at(i);

        if ((wf.couplingtype() == couplingTypeToStringKey(this->couplingType()).toStdString()) &&
                (wf.sourceanalysis() == analysisTypeToStringKey(m_sourceField->analysisType()).toStdString()) &&
                (wf.targetanalysis() == analysisTypeToStringKey(m_targetField->analysisType()).toStdString()))
        {
            for (int i = 0; i < wf.vector_form().size(); i++)
            {
                XMLCoupling::vector_form form = wf.vector_form().at(i);
                wfVectorVolume.push_back(FormInfo(QString::fromStdString(form.id()), form.i(), form.j()));
            }
        }
    }

    return wfVectorVolume;
}

LinearityType CouplingInfo::linearityType()
{
    // TODO: FIX - warning
    if (couplingType() == CouplingType_Hard)
        assert(sourceField()->linearityType() == targetField()->linearityType());

    return targetField()->linearityType();
}
