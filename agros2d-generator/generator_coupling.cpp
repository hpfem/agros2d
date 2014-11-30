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

#include "generator.h"
#include "generator_coupling.h"
#include "parser.h"

#include <QDir>

#include "util/constants.h"
#include "hermes2d/weak_form.h"
#include "hermes2d/module.h"
#include "hermes2d/coupling.h"

#include "parser/lex.h"

#include "util/constants.h"


Agros2DGeneratorCoupling::Agros2DGeneratorCoupling(const QString &couplingId) : m_output(nullptr)
{
    QString iD = couplingId;
    QDir root(QApplication::applicationDirPath());
    root.mkpath(QString("%1/%2").arg(GENERATOR_PLUGINROOT).arg(iD));

    coupling_xsd = XMLModule::module_(compatibleFilename(datadir() + COUPLINGROOT + "/" + couplingId + ".xml").toStdString(), xml_schema::flags::dont_validate);
    XMLModule::module *mod = coupling_xsd.get();
    assert(mod->coupling().present());
    m_coupling = &mod->coupling().get();

    QString sourceModuleId = QString::fromStdString(m_coupling->general_coupling().modules().source().id().c_str());
    QString targetModuleId = QString::fromStdString(m_coupling->general_coupling().modules().target().id().c_str());

    m_source_module_xsd = XMLModule::module_(compatibleFilename(datadir() + MODULEROOT + "/" + sourceModuleId + ".xml").toStdString(), xml_schema::flags::dont_validate);
    mod = m_source_module_xsd.get();
    assert(mod->field().present());
    m_sourceModule = &mod->field().get();

    m_target_module_xsd = XMLModule::module_(compatibleFilename(datadir() + MODULEROOT + "/" + targetModuleId + ".xml").toStdString(), xml_schema::flags::dont_validate);
    mod = m_target_module_xsd.get();
    assert(mod->field().present());
    m_targetModule = &mod->field().get();

    QDir().mkdir(GENERATOR_PLUGINROOT + "/" + iD);

    // variables
    foreach (XMLModule::quantity quantity, m_sourceModule->volume().quantity())
    {
        QString shortName = QString::fromStdString(quantity.shortname().get()).replace(" ", "");
        QString iD = QString::fromStdString(quantity.id().c_str()).replace(" ", "");
        m_sourceVariables.insert(iD, shortName);
    }

    foreach (XMLModule::quantity quantity, m_targetModule->volume().quantity())
    {
        QString shortName = QString::fromStdString(quantity.shortname().get()).replace(" ", "");
        QString iD = QString::fromStdString(quantity.id().c_str()).replace(" ", "");
        m_targetVariables.insert(iD, shortName);
    }

    Module::volumeQuantityProperties(m_targetModule, quantityOrdering, quantityIsNonlinear, functionOrdering);
    Module::volumeQuantityProperties(m_sourceModule, sourceQuantityOrdering, sourceQuantityIsNonlinear, sourceFunctionOrdering);
}

void Agros2DGeneratorCoupling::generatePluginProjectFile()
{
    QString id = (QString::fromStdString(m_coupling->general_coupling().id().c_str()));

    qDebug() << (QString("generating project file").toLatin1());

    ctemplate::TemplateDictionary output("output");
    output.SetValue("ID", id.toStdString());

    // expand template
    std::string text;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/coupling_CMakeLists_txt.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // save to file
    writeStringContent(QString("%1/%2/%3/CMakeLists.txt").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorCoupling::generatePluginInterfaceFiles()
{
    QString id = QString::fromStdString(m_coupling->general_coupling().id());

    qDebug() << (QString("generating interface file").toLatin1());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/coupling_interface_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &text);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // source - expand template
    text.clear();


    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/coupling_interface_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &text);
    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorCoupling::generatePluginWeakFormFiles()
{
    qDebug() << (QString("Coupling: %1.").arg(QString::fromStdString(m_coupling->general_coupling().id())).toLatin1());

    generatePluginWeakFormSourceFiles();
    generatePluginWeakFormHeaderFiles();
}

void Agros2DGeneratorCoupling::generatePluginWeakFormHeaderFiles()
{
    QString id = QString::fromStdString(m_coupling->general_coupling().id());

    qDebug() << (QString("generating weakform header file").toLatin1());

    // header - expand template
    std::string text;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/weakform_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &text);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // generate empty extfunction header file
    writeStringContent(QString("%1/%2/%3/%3_extfunction.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(""));
}


void Agros2DGeneratorCoupling::generatePluginWeakFormSourceFiles()
{
    QString id = QString::fromStdString(m_coupling->general_coupling().id());
    QStringList modules = QString::fromStdString(m_coupling->general_coupling().id()).split("-");

    qDebug() << (QString("generating weakform source file").toLatin1());

    std::string text;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/weakform_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}


template <typename WeakForm>
void Agros2DGeneratorCoupling::generateForm(FormInfo formInfo, LinearityType linearityType, ctemplate::TemplateDictionary &output, WeakForm weakform, QString weakFormType)
{
    foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
    {
        QString expression = (coordinateType == CoordinateType_Planar ? formInfo.expr_planar : formInfo.expr_axi);
        if(expression != "")
        {
            ctemplate::TemplateDictionary *field;
            field = output.AddSectionDictionary(weakFormType.toStdString() + "_SOURCE");

            QString id = (QString::fromStdString(m_coupling->general_coupling().id().c_str())).replace("-", "_");

            // source files
            QString functionName = QString("%1_%2_%3_%4_%5_%6_%7_%8_%9_%10").
                    arg(weakFormType.toLower()).
                    arg(id).
                    arg(QString::fromStdString(weakform.sourceanalysis().get())).
                    arg(QString::fromStdString(weakform.analysistype())).
                    arg(coordinateTypeToStringKey(coordinateType)).
                    arg(linearityTypeToStringKey(linearityType)).
                    arg(formInfo.id).
                    arg(QString::fromStdString(weakform.couplingtype().get())).
                    arg(QString::number(formInfo.i)).
                    arg(QString::number(formInfo.j));

            CouplingType couplingType = Agros2DGenerator::couplingTypeFromString(QString::fromStdString(weakform.couplingtype().get()));

            field->SetValue("COLUMN_INDEX", QString::number(formInfo.j).toStdString());
            field->SetValue("FUNCTION_NAME", functionName.toStdString());
            field->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
            field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
            field->SetValue("SOURCE_ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisTypeFromStringKey(QString::fromStdString(weakform.sourceanalysis().get()))).toStdString());
            field->SetValue("TARGET_ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype()))).toStdString());
            field->SetValue("ROW_INDEX", QString::number(formInfo.i).toStdString());
            field->SetValue("MODULE_ID", id.toStdString());
            field->SetValue("WEAKFORM_ID", formInfo.id.toStdString());
            field->SetValue("COUPLING_TYPE", Agros2DGenerator::couplingTypeToString(weakform.couplingtype().get().c_str()).toStdString());

            ParserModuleInfo pmiSource(*m_sourceModule,
                                       analysisTypeFromStringKey(QString::fromStdString(weakform.sourceanalysis().get())),
                                       coordinateType, linearityType);

            ParserModuleInfo pmi(*m_targetModule,
                                       analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())),
                                       coordinateType, linearityType);

            QString exprCpp = Parser::parseCouplingWeakFormExpression(pmiSource, pmi, expression);
            field->SetValue("EXPRESSION", exprCpp.toStdString());

            // add weakform
            field = output.AddSectionDictionary("SOURCE");
            field->SetValue("FUNCTION_NAME", functionName.toStdString());
        }
    }
}

void Agros2DGeneratorCoupling::generateWeakForms(ctemplate::TemplateDictionary &output)
{
    //this->m_docString = "";
    foreach(XMLModule::weakform_volume weakform, m_coupling->volume().weakforms_volume().weakform_volume())
    {
        AnalysisType sourceAnalysis = analysisTypeFromStringKey(QString::fromStdString(weakform.sourceanalysis().get().c_str()));
        AnalysisType targetAnalysis = analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype().c_str()));
        CouplingType couplingType = couplingTypeFromStringKey(QString::fromStdString(weakform.couplingtype().get().c_str()));

        foreach(XMLModule::linearity_option option, weakform.linearity_option())
        {
            LinearityType linearityType = linearityTypeFromStringKey(QString::fromStdString(option.type().c_str()));

            // generate individual forms
            QList<FormInfo> matrixForms = CouplingInfo::wfMatrixVolumeSeparated(&m_coupling->volume(), sourceAnalysis, targetAnalysis, couplingType, linearityType);
            foreach(FormInfo formInfo, matrixForms)
            {
                generateForm(formInfo, linearityType, output, weakform, "VOLUME_MATRIX");
            }

            // generate individual forms
            QList<FormInfo> vectorForms = CouplingInfo::wfVectorVolumeSeparated(&m_coupling->volume(), sourceAnalysis, targetAnalysis, couplingType, linearityType);
            foreach(FormInfo formInfo, vectorForms)
            {
                generateForm(formInfo, linearityType, output, weakform, "VOLUME_VECTOR");
            }
        }
    }

}


void Agros2DGeneratorCoupling::prepareWeakFormsOutput()
{
    qDebug() << (QString("parsing weak forms").toLatin1());
    assert(! m_output);
    m_output = new ctemplate::TemplateDictionary("output");

    QString id = QString::fromStdString(m_coupling->general_coupling().id());
    QStringList modules = QString::fromStdString(m_coupling->general_coupling().id()).split("-");
    m_output->SetValue("ID", id.toStdString());
    m_output->SetValue("CLASS", (modules[0].left(1).toUpper() + modules[0].right(modules[0].length() - 1) +
                              modules[1].left(1).toUpper() + modules[1].right(modules[1].length() - 1)).toStdString());

    //comment on beginning of weakform.cpp, may be removed
    ctemplate::TemplateDictionary *field;
    foreach(QString quantID, this->quantityOrdering.keys())
    {
        field = m_output->AddSectionDictionary("QUANTITY_INFO");
        field->SetValue("QUANT_ID", quantID.toStdString());
        field->SetValue("INDEX", QString("%1").arg(quantityOrdering[quantID]).toStdString());
        field->SetValue("OFFSET", "offset.quant");
        if(quantityIsNonlinear[quantID])
        {
            field = m_output->AddSectionDictionary("QUANTITY_INFO");
            field->SetValue("QUANT_ID", QString("derivative %1").arg(quantID).toStdString());
            field->SetValue("INDEX", QString("%1").arg(quantityOrdering[quantID] + 1).toStdString());
            field->SetValue("OFFSET", "offset.quant");
        }
    }
    foreach(QString funcID, this->functionOrdering.keys())
    {
        field = m_output->AddSectionDictionary("QUANTITY_INFO");
        field->SetValue("QUANT_ID", funcID.toStdString());
        field->SetValue("INDEX", QString("%1").arg(functionOrdering[funcID]).toStdString());
        field->SetValue("OFFSET", "offset.quant");
    }

    foreach(QString quantID, this->sourceQuantityOrdering.keys())
    {
        field = m_output->AddSectionDictionary("QUANTITY_INFO");
        field->SetValue("QUANT_ID", quantID.toStdString());
        field->SetValue("INDEX", QString("%1").arg(sourceQuantityOrdering[quantID]).toStdString());
        field->SetValue("OFFSET", "offset.sourceQuant");
        if(sourceQuantityIsNonlinear[quantID])
        {
            field = m_output->AddSectionDictionary("QUANTITY_INFO");
            field->SetValue("QUANT_ID", QString("derivative %1").arg(quantID).toStdString());
            field->SetValue("INDEX", QString("%1").arg(sourceQuantityOrdering[quantID] + 1).toStdString());
            field->SetValue("OFFSET", "offset.sourceQuant");
        }
    }
    foreach(QString funcID, this->sourceFunctionOrdering.keys())
    {
        field = m_output->AddSectionDictionary("QUANTITY_INFO");
        field->SetValue("QUANT_ID", funcID.toStdString());
        field->SetValue("INDEX", QString("%1").arg(sourceFunctionOrdering[funcID]).toStdString());
        field->SetValue("OFFSET", "offset.sourceQuant");
    }

    generateWeakForms(*m_output);
}

void Agros2DGeneratorCoupling::deleteWeakFormOutput()
{
    delete m_output;
    m_output = nullptr;
}

