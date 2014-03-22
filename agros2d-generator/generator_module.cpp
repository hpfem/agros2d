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

#include "generator.h"
#include "generator_module.h"
#include "parser.h"
#include "hermes2d/module.h"


#include "util/constants.h"

Agros2DGeneratorModule::Agros2DGeneratorModule(const QString &moduleId) : m_output(nullptr)
{
    QDir root(QApplication::applicationDirPath());
    root.mkpath(QString("%1/%2").arg(GENERATOR_PLUGINROOT).arg(moduleId));

    // read module
    module_xsd = XMLModule::module_(compatibleFilename(datadir() + MODULEROOT + "/" + moduleId + ".xml").toStdString(), xml_schema::flags::dont_validate);
    XMLModule::module *mod = module_xsd.get();
    assert(mod->field().present());
    m_module = &mod->field().get();

    QDir().mkdir(GENERATOR_PLUGINROOT + "/" + moduleId);

    // documentation
    QDir doc_root(QApplication::applicationDirPath());
    doc_root.mkpath(QString("%1/%2").arg(GENERATOR_DOCROOT).arg(moduleId));
    QDir().mkdir(GENERATOR_DOCROOT + "/" + moduleId);

    // variables
    foreach (XMLModule::quantity quantity, m_module->volume().quantity())
    {
        QString shortName = QString::fromStdString(quantity.shortname().get()).replace(" ", "");
        QString iD = QString::fromStdString(quantity.id().c_str()).replace(" ", "");
        m_volumeVariables.insert(iD, shortName);
    }

    foreach (XMLModule::quantity quantity, m_module->surface().quantity())
    {
        QString shortName = QString::fromStdString(quantity.shortname().get()).replace(" ", "");
        QString iD = QString::fromStdString(quantity.id().c_str()).replace(" ", "");
        m_surfaceVariables.insert(iD, shortName);
    }

    // localization
    getNames(moduleId);

    Module::volumeQuantityProperties(m_module, quantityOrdering, quantityIsNonlinear, functionOrdering);

    m_parser = QSharedPointer<FieldParser>(new FieldParser(m_module));
}

Agros2DGeneratorModule::~Agros2DGeneratorModule()
{
}

void Agros2DGeneratorModule::generatePluginProjectFile()
{
    Hermes::Mixins::Loggable::Static::info(QString("generating project file").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());

    ctemplate::TemplateDictionary output("output");
    output.SetValue("ID", id.toStdString());

    // expand template
    std::string text;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/module_CMakeLists_txt.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // save to file
    writeStringContent(QString("%1/%2/%3/CMakeLists.txt").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorModule::prepareWeakFormsOutput()
{
    Hermes::Mixins::Loggable::Static::info(QString("parsing weak forms").toLatin1());
    assert(! m_output);
    m_output = new ctemplate::TemplateDictionary("output");

    QString id = QString::fromStdString(m_module->general_field().id());

    m_output->SetValue("ID", m_module->general_field().id());
    m_output->SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    //comment on beginning of weakform.cpp, may be removed
    ctemplate::TemplateDictionary *field;
    foreach(QString quantID, this->quantityOrdering.keys())
    {
        field = m_output->AddSectionDictionary("QUANTITY_INFO");
        field->SetValue("QUANT_ID", quantID.toStdString());
        field->SetValue("INDEX", QString("%1").arg(quantityOrdering[quantID]).toStdString());
        if(quantityIsNonlinear[quantID])
        {
            field = m_output->AddSectionDictionary("QUANTITY_INFO");
            field->SetValue("QUANT_ID", QString("derivative %1").arg(quantID).toStdString());
            field->SetValue("INDEX", QString("%1").arg(quantityOrdering[quantID] + 1).toStdString());
        }
    }
    foreach(QString funcID, this->functionOrdering.keys())
    {
        field = m_output->AddSectionDictionary("QUANTITY_INFO");
        field->SetValue("QUANT_ID", funcID.toStdString());
        field->SetValue("INDEX", QString("%1").arg(functionOrdering[funcID]).toStdString());
    }

    QString description = QString::fromStdString(m_module->general_field().description());
    description = description.replace("\n","");
    m_output->SetValue("DESCRIPTION", description.toStdString());
    if (m_module->cpp().present())
        m_output->SetValue("CPP", m_module->cpp().get());

    generateSpecialFunctions(*m_output);
    generateExtFunctions(*m_output);
    generateWeakForms(*m_output);

    foreach(QString name, m_names)
    {
        ctemplate::TemplateDictionary *field = m_output->AddSectionDictionary("NAMES");
        field->SetValue("NAME",name.toStdString());
    }

}

void Agros2DGeneratorModule::deleteWeakFormOutput()
{
    delete m_output;
    m_output = nullptr;
}

void Agros2DGeneratorModule::generatePluginInterfaceFiles()
{
    Hermes::Mixins::Loggable::Static::info(QString("generating interface file").toLatin1());
    QString id = QString::fromStdString(m_module->general_field().id());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/interface_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &text);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // source - expand template
    text.clear();

    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/interface_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}



void Agros2DGeneratorModule::generatePluginEquations()
{
    Hermes::Mixins::Loggable::Static::info(QString("generating equations").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());
    QString outputDir = QDir().absoluteFilePath(QString("%1/%2").arg(QApplication::applicationDirPath()).arg("resources/images/equations/"));

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", m_module->general_field().id());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    output.SetValue("LATEX_TEMPLATE", compatibleFilename(QString("%1/%2/equations.tex").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString());

    foreach(XMLModule::weakform_volume weakform, m_module->volume().weakforms_volume().weakform_volume())
    {
        ctemplate::TemplateDictionary *equation = output.AddSectionDictionary("EQUATION_SECTION");

        equation->SetValue("EQUATION", weakform.equation());
        equation->SetValue("OUTPUT_DIRECTORY", outputDir.toStdString());

        equation->SetValue("NAME", QString("%1").arg(QString::fromStdString(weakform.analysistype())).toStdString());
    }

    foreach(XMLModule::weakform_surface weakform, m_module->surface().weakforms_surface().weakform_surface())
    {
        foreach(XMLModule::boundary boundary, weakform.boundary())
        {
            ctemplate::TemplateDictionary *equation = output.AddSectionDictionary("EQUATION_SECTION");

            equation->SetValue("EQUATION", boundary.equation());
            equation->SetValue("OUTPUT_DIRECTORY", outputDir.toStdString());

            equation->SetValue("NAME", QString("%1_%2").arg(QString::fromStdString(weakform.analysistype())).arg(QString::fromStdString(boundary.id())).toStdString());
        }
    }

    ExpandTemplate(compatibleFilename(QString("%1/%2/equations.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                   ctemplate::DO_NOT_STRIP, &output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_equations.py").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorModule::generatePluginErrorCalculator()
{
    Hermes::Mixins::Loggable::Static::info(QString("generating error calculator file").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());


    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", id.toStdString());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/errorcalculator_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    for (unsigned int i = 0; i < m_module->error_calculator().calculator().size(); i++)
    {
        XMLModule::calculator calc = m_module->error_calculator().calculator().at(i);

        for (unsigned int i = 0; i < calc.expression().size(); i++)
        {
            XMLModule::expression expr = calc.expression().at(i);

            AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(expr.analysistype()));

            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                // TODO: better !!!
                LinearityType linearityTypes[3] = {LinearityType_Linear, LinearityType_Newton, LinearityType_Picard};
                for (int lt = 0; lt < 3; lt++)
                {
                    ctemplate::TemplateDictionary *expression = output.AddSectionDictionary("CALCULATOR_SOURCE");

                    expression->SetValue("ID_CALCULATOR", calc.id());
                    expression->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisType).toStdString());
                    expression->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityTypes[lt]).toStdString());
                    expression->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());

                    if (expr.analysistype() == analysisTypeToStringKey(analysisType).toStdString())
                    {
                        QString exprOriginal, exprCpp;
                        if (coordinateType == CoordinateType_Planar)
                            exprOriginal = QString::fromStdString(expr.planar().get());
                        else
                            exprOriginal = QString::fromStdString(expr.axi().get());

                        ParserModuleInfo pmi(*m_module, analysisType, coordinateType, linearityTypes[lt]);
                        exprCpp = m_parser->parseErrorExpression(pmi, exprOriginal);

                        expression->SetValue("EXPRESSION", exprCpp.toStdString());

                        foreach(QString key, m_volumeVariables.keys())
                        {
                            ctemplate::TemplateDictionary *subField = 0;
                            subField = expression->AddSectionDictionary("VARIABLE_SOURCE");
                            subField->SetValue("VARIABLE", key.toStdString());
                            subField->SetValue("VARIABLE_SHORT", m_volumeVariables.value(key).toStdString());
                        }
                    }
                }
            }
        }
    }


    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_errorcalculator.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // source - expand template
    text.clear();
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/errorcalculator_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_errorcalculator.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorModule::getNames(const QString &moduleId)
{
    QFile * file = new QFile((datadir().toStdString() + MODULEROOT.toStdString() + "/" + moduleId.toStdString() + ".xml").c_str());
    file->open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xml(file);
    QStringList names;
    while(!xml.atEnd())
    {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = xml.readNext();
        if(token == QXmlStreamReader::EndDocument)
            break;
        if(xml.attributes().hasAttribute("name"))
        {
            if(!m_names.contains(xml.attributes().value("name").toString()))
                m_names.append(xml.attributes().value("name").toString());
        }
        if(xml.attributes().hasAttribute("name"))
        {
            if(!m_names.contains(xml.attributes().value("analysistype").toString()))
                m_names.append(xml.attributes().value("analysistype").toString());
        }
    }
}
