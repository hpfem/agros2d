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

// TODO: in each module should be implicit value
CouplingInfo::CouplingInfo(FieldInfo *sourceField, FieldInfo *targetField,
                           CouplingType couplingType) :
    m_sourceField(sourceField), m_targetField(targetField), m_couplingType(couplingType), m_coupling(NULL)
{
    reload();
}

CouplingInfo::~CouplingInfo()
{
    if (m_coupling)
        delete m_coupling;
}

void CouplingInfo::setCouplingType(CouplingType couplingType)
{
    m_couplingType = couplingType;
    reload();
}

void CouplingInfo::reload()
{
    if (m_coupling)
        delete m_coupling;

    m_coupling = couplingFactory(m_sourceField, m_targetField, m_couplingType);
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
            couplings_xsd = XMLCoupling::coupling_((datadir() + COUPLINGROOT + QDir::separator() + filename).toStdString().c_str());
        }
        catch(xsd::cxx::tree::exception<char>& e)
        {
            cout << e;
            std::stringstream str;
            str << e;
            Agros2D::log()->printError("solver",QObject::tr("Unable to read coupling file %1: %2").arg(filename).arg(QString::fromStdString(str.str())));
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

Coupling::Coupling(const QString &couplingId, CoordinateType coordinateType, CouplingType couplingType, AnalysisType sourceFieldAnalysis, AnalysisType targetFieldAnalysis)
{
    m_coordinateType = coordinateType;
    m_couplingType = couplingType;
    m_sourceFieldAnalysis = sourceFieldAnalysis;
    m_targetFieldAnalysis = targetFieldAnalysis;

    clear();

    // read coupling description
    QString filename = (datadir() + COUPLINGROOT + QDir::separator() + couplingId + ".xml");
    assert(QFile::exists(filename));

    read(filename);
}

Coupling::~Coupling()
{
    clear();
}

void Coupling::clear()
{
    m_constants.clear();

    m_wfMatrixVolumeExpression.clear();
    m_wfVectorVolumeExpression.clear();
}

void Coupling::read(const QString &filename)
{
    assert(QFile::exists(filename));

    // qDebug() << "reading coupling: " << filename;

    clear();

    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    std::auto_ptr<XMLCoupling::coupling> couplings_xsd = XMLCoupling::coupling_(filename.toStdString().c_str());
    XMLCoupling::coupling *coup = couplings_xsd.get();

    // problem
    m_couplingId = QString::fromStdString(coup->general().id());
    m_name = QString::fromStdString(coup->general().name());
    m_description = QString::fromStdString(coup->general().description());

    // constants
    for (int i = 0; i < coup->constants().constant().size(); i++)
    {
        XMLCoupling::constant cnst = coup->constants().constant().at(i);
        m_constants[QString::fromStdString(cnst.id())] = cnst.value();
    }

    // volume weakforms
    for (int i = 0; i < coup->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLCoupling::weakform_volume wf = coup->volume().weakforms_volume().weakform_volume().at(i);

        if ((wf.couplingtype() == couplingTypeToStringKey(m_couplingType).toStdString()) &&
                (wf.sourceanalysis() == analysisTypeToStringKey(m_sourceFieldAnalysis).toStdString()) &&
                (wf.targetanalysis() == analysisTypeToStringKey(m_targetFieldAnalysis).toStdString()))
        {
            // matrix form
            for (int i = 0; i < wf.matrix_form().size(); i++)
            {
                XMLCoupling::matrix_form form = wf.matrix_form().at(i);
                m_wfMatrixVolumeExpression.push_back(new FormInfo(QString::fromStdString(form.id()), form.i(), form.j()));
            }

            // vector form
            for (int i = 0; i < wf.vector_form().size(); i++)
            {
                XMLCoupling::vector_form form = wf.vector_form().at(i);
                m_wfVectorVolumeExpression.push_back(new FormInfo(QString::fromStdString(form.id()), form.i(), form.j()));
            }
        }
    }

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}



// ****************************************************************************************************

Coupling *couplingFactory(FieldInfo* sourceField, FieldInfo* targetField, CouplingType couplingType)
{
    // open coupling
    QString couplingId = sourceField->fieldId() + "-" + targetField->fieldId();
    QString filename = (datadir() + COUPLINGROOT + QDir::separator() + couplingId + ".xml");

    if (QFile::exists(filename))
    {
        CoordinateType coordinateType = Agros2D::problem()->config()->coordinateType();
        Coupling *coupling = new Coupling(couplingId,
                                          coordinateType,
                                          couplingType,
                                          sourceField->analysisType(),
                                          targetField->analysisType());

        return coupling;
    }
    else
    {
        qDebug() << "Coupling doesn't exists.";
        return NULL;
    }


}
