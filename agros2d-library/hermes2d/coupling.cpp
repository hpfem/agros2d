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

QMap<QString, QString> availableCouplings()
{
    static QMap<QString, QString> couplings;

    // read modules
    if (couplings.size() == 0)
    {
        QDir dir(datadir() + COUPLINGROOT);

        QStringList filter;
        filter << "*.xml";
        QStringList list = dir.entryList(filter);

        foreach (QString filename, list)
        {
            std::auto_ptr<XMLCoupling::coupling> coupling_xsd = XMLCoupling::coupling_((datadir().toStdString() + COUPLINGROOT.toStdString() + "/" + filename.toStdString()).c_str());
            XMLCoupling::coupling *mod = coupling_xsd.get();

            // module name
            couplings[filename.left(filename.size() - 4)] = QString::fromStdString(mod->general().name());
        }
    }

    return couplings;
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

bool isCouplingAvailable(FieldInfo* sourceField, FieldInfo* targetField)
{
    QDir dir(datadir() + COUPLINGROOT);
    if (!dir.exists())
        qDebug() << QObject::tr("Couplings dir '%1' doesn't exists").arg(datadir() + COUPLINGROOT);

    QStringList filter;
    filter << "*.xml";
    QStringList list = dir.entryList(filter);

    foreach (QString filename, list)
    {
        // read name
        std::auto_ptr<XMLCoupling::coupling> couplings_xsd;
        try{
            couplings_xsd = XMLCoupling::coupling_((datadir() + COUPLINGROOT + QDir::separator() + filename).toLatin1().data());
        }
        catch(xsd::cxx::tree::exception<char>& e)
        {
            cout << e;
            std::stringstream str;
            str << e;
            Agros2D::log()->printError(QObject::tr("Solver"),QObject::tr("Unable to read coupling file %1: %2").arg(filename).arg(QString::fromStdString(str.str())));
            return false;
        }

        XMLCoupling::coupling *coup = couplings_xsd.get();

        // module name
        QString sourceFieldStr(QString::fromStdString(coup->general().modules().source().id()));
        QString targetFieldStr(QString::fromStdString(coup->general().modules().target().id()));

        if ((sourceFieldStr == sourceField->fieldId()) && (targetFieldStr == targetField->fieldId()))
        {
            // check whether coupling is available for values of source and target fields such as analysis type
            for (int i = 0; i < coup->volume().weakforms_volume().weakform_volume().size(); i++)
            {
                XMLCoupling::weakform_volume wf = coup->volume().weakforms_volume().weakform_volume().at(i);

                if ((wf.sourceanalysis() == analysisTypeToStringKey(sourceField->analysisType()).toStdString()) &&
                        (wf.targetanalysis() == analysisTypeToStringKey(targetField->analysisType()).toStdString()))
                {
                    return true;
                }
            }
        }
    }

    return false;
}
