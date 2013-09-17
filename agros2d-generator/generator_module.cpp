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

// todo: remove
#include "../agros2d-library/hermes2d/field.h"

#include <QDir>

#include "util/constants.h"
#include "hermes2d/module.h"
#include "hermes2d/coupling.h"
#include "parser/lex.h"

#include "util/constants.h"

Agros2DGeneratorModule::Agros2DGeneratorModule(const QString &moduleId)
{
    QDir root(QApplication::applicationDirPath());
    root.mkpath(QString("%1/%2").arg(GENERATOR_PLUGINROOT).arg(moduleId));

    // read module
    module_xsd = XMLModule::module_(compatibleFilename(datadir() + MODULEROOT + "/" + moduleId + ".xml").toStdString(), xml_schema::flags::dont_validate);
    m_module = module_xsd.get();

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
}

void Agros2DGeneratorModule::generatePluginProjectFile()
{
    qDebug() << tr("%1: generating plugin project file.").arg(QString::fromStdString(m_module->general().id()));

    QString id = QString::fromStdString(m_module->general().id());

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

void Agros2DGeneratorModule::generatePluginInterfaceFiles()
{
    qDebug() << tr("%1: generating plugin interface file.").arg(QString::fromStdString(m_module->general().id()));

    QString id = QString::fromStdString(m_module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", m_module->general().id());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    QString description = QString::fromStdString(m_module->general().description());
    description = description.replace("\n","");
    output.SetValue("DESCRIPTION", description.toStdString());
    if (m_module->cpp().present())
        output.SetValue("CPP", m_module->cpp().get());

    foreach(XMLModule::function function, m_module->volume().function())
    {
        generateSpecialFunction(&function, &output);
    }

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/interface_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // source - expand template
    text.clear();
    generateWeakForms(output, false);

    foreach(QString name, m_names)
    {
        ctemplate::TemplateDictionary *field = output.AddSectionDictionary("NAMES");
        field->SetValue("NAME",name.toStdString());
    }

    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/interface_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorModule::generatePluginWeakFormFiles()
{
    generatePluginWeakFormSourceFiles();
    generatePluginWeakFormHeaderFiles();
}


QString Agros2DGeneratorModule::underline(QString text,  char symbol)
{
    QString underlined = text + "\n";
    for(int i = 0; i < text.length(); i++)
    {
        underlined += symbol;
    }
    underlined += "\n";
    return underlined;
}

QString Agros2DGeneratorModule::capitalize(QString text)
{
    text[0] = text[0].toUpper();
    return text;
}

QString Agros2DGeneratorModule::createTable(QList<QStringList> table)
{
    QString text = "";
    int columnsNumber = table.length();
    int rowNumber = table.at(0).length();

    QList<int> columnWidths;
    for(int i = 0; i < columnsNumber; i++)
    {
        columnWidths.append(0);
        for(int j = 0; j < table.at(i).length(); j++)
        {
            if(columnWidths[i] < table.at(i).at(j).length())
                columnWidths[i] = table.at(i).at(j).length();
        }
        columnWidths[i] += 3;
    }

    for(int k = 0; k < rowNumber; k++ )
    {
        text += "+";
        for(int i = 0; i < columnsNumber; i++)
        {
            for(int j =0; j < columnWidths[i] - 1; j++)
            {
                if (k == 1)
                    text += "=";
                else
                    text += "-";
            }
            text += "+";
        }
        text += "\n";

        for(int i = 0; i < columnsNumber; i++)
        {
            QString item =  "| " + table.at(i).at(k) + " ";
            int rest = columnWidths.at(i) - item.length();
            QString fillRest(rest, ' ');
            text += item + fillRest;
        };
        text += "|\n";
    }
    text += "+";
    for(int i = 0; i < columnsNumber; i++)
    {
        for(int j =0; j < columnWidths[i] - 1; j++)
        {
            text += "-";
        }
        text += "+";
    }
    text += "\n\n";
    return text;
}

void Agros2DGeneratorModule::generatePluginDocumentationFiles()
{    
    QString id = QString::fromStdString(m_module->general().id());
    //    QString name = QString::fromStdString(m_module->general().name());
    QString text = "";
    //   text += underline(name,'=');
    //   text += QString::fromStdString(m_module->general().description()) + "\n\n";

    /* Creates table of constants */

    text += underline("Constants:",'-');
    text += "\n";

    QList<QStringList> table;
    QStringList names;
    QStringList values;

    names.append("Agros variable");
    values.append("Units");
    foreach(XMLModule::constant con, m_module->constants().constant())
    {
        names.append(QString::fromStdString(con.id()));
        values.append(QString::number(con.value()));
    }

    table.append(names);
    table.append(values);
    text += createTable(table);
    text += "\n\n";
    names.clear();
    values.clear();
    table.clear();

    // Creates volume variables table
    text += underline("Volume variables:",'-');
    text += "\n";

    names.append("Volume (material) variable");
    values.append("Agros2D variable");
    foreach(XMLModule::quantity quantity, m_module->volume().quantity())
    {
        values.append(QString::fromStdString(quantity.id().c_str()));
        names.append(QString::fromStdString(quantity.shortname().get()));
    }

    table.append(values);
    table.append(names);
    text += createTable(table);
    text += "\n\n";
    names.clear();
    values.clear();
    table.clear();

    // Creates surface variables table
    text += underline("Surface (boundary) variables:",'-');
    text += "\n";

    names.append("Surface variable");
    values.append("Agros2D variable");
    foreach(XMLModule::quantity quantity, m_module->surface().quantity())
    {
        values.append(QString::fromStdString(quantity.id().c_str()));
        names.append(QString::fromStdString(quantity.shortname().get()));
    }

    table.append(values);
    table.append(names);
    text += createTable(table);
    text += "\n\n";
    names.clear();
    values.clear();
    table.clear();


    /* Creates variable table */

    text += underline("Postprocessor variables:", '-');
    text +=  "\n";


    QStringList latexShortNames;
    QStringList units;
    QStringList descriptions;
    QStringList shortNames;


    latexShortNames.append("Name");
    units.append("Units");
    descriptions.append("Description");
    shortNames.append("Agros2D variable");

    foreach(XMLModule::localvariable var, m_module->postprocessor().localvariables().localvariable())
    {
        shortNames.append(var.shortname().c_str());
        if(var.shortname_latex().present())
            latexShortNames.append(QString::fromStdString(":math:`" + var.shortname_latex().get() + "`"));
        else latexShortNames.append(" ");
        units.append(var.unit().c_str());
        descriptions.append(QString::fromStdString(var.name()));
    }

    table.append(shortNames);
    table.append(latexShortNames);
    table.append(units);
    table.append(descriptions);
    text += createTable(table);
    text += "\n\n";
    table.clear();

    // Creates table of volume integrals
    text += underline("Volume integrals:", '-');
    text +=  "\n";

    latexShortNames.clear();
    units.clear();
    descriptions.clear();
    shortNames.clear();

    latexShortNames.append("Name");
    units.append("Units");
    descriptions.append("Description");
    shortNames.append("Agros2D variable");


    foreach(XMLModule::volumeintegral volume_int, m_module->postprocessor().volumeintegrals().volumeintegral())
    {
        shortNames.append(volume_int.shortname().c_str());
        if(volume_int.shortname_latex().present())
            latexShortNames.append(QString::fromStdString(":math:`" + volume_int.shortname_latex().get() + "`"));
        else latexShortNames.append(" ");
        units.append(volume_int.unit().c_str());
        descriptions.append(QString::fromStdString(volume_int.name()));
    }

    table.append(shortNames);
    table.append(latexShortNames);
    table.append(units);
    table.append(descriptions);
    text += createTable(table);
    text += "\n\n";
    table.clear();


    // Creates table of volume integrals
    text += underline("Surface integrals:", '-');
    text +=  "\n";

    latexShortNames.clear();
    units.clear();
    descriptions.clear();
    shortNames.clear();

    latexShortNames.append("Name");
    units.append("Units");
    descriptions.append("Description");
    shortNames.append("Agros2D variable");


    foreach(XMLModule::surfaceintegral surf_int, m_module->postprocessor().surfaceintegrals().surfaceintegral())
    {
        shortNames.append(surf_int.shortname().c_str());
        if(surf_int.shortname_latex().present())
            latexShortNames.append(QString::fromStdString(":math:`" + surf_int.shortname_latex().get() + "`"));
        else latexShortNames.append(" ");
        units.append(surf_int.unit().c_str());
        descriptions.append(QString::fromStdString(surf_int.name()));
    }

    table.append(shortNames);
    table.append(latexShortNames);
    table.append(units);
    table.append(descriptions);
    text += createTable(table);

    // documentation - save to file
    writeStringContent(QString("%1/%2/%3/%3.gen").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_DOCROOT).
                       arg(id),
                       text);
}

void Agros2DGeneratorModule::generatePluginEquations()
{
    qDebug() << tr("%1: generating plugin equations.").arg(QString::fromStdString(m_module->general().id()));

    QString id = QString::fromStdString(m_module->general().id());
    QString outputDir = QDir().absoluteFilePath(QString("%1/%2").arg(QApplication::applicationDirPath()).arg("resources/images/equations/"));

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", m_module->general().id());
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
    qDebug() << tr("%1: generating plugin error calculator file.").arg(QString::fromStdString(m_module->general().id()));

    QString id = QString::fromStdString(m_module->general().id());


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
                        QString exprCpp;
                        if (coordinateType == CoordinateType_Planar)
                            exprCpp = parseWeakFormExpression(analysisType, coordinateType, linearityTypes[lt], QString::fromStdString(expr.planar().get()), true, true);
                        else
                            exprCpp = parseWeakFormExpression(analysisType, coordinateType, linearityTypes[lt], QString::fromStdString(expr.axi().get()), true, true);

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

void Agros2DGeneratorModule::generatePluginWeakFormSourceFiles()
{
    qDebug() << tr("%1: generating plugin weakform source file.").arg(QString::fromStdString(m_module->general().id()));

    QString id = QString::fromStdString(m_module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", m_module->general().id());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());


    std::string text;
    generateWeakForms(output);

    ExpandTemplate(compatibleFilename(QString("%1/%2/weakform_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                   ctemplate::DO_NOT_STRIP, &output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorModule::generatePluginWeakFormHeaderFiles()
{
    qDebug() << tr("%1: generating plugin weakform header file.").arg(QString::fromStdString(m_module->general().id()));

    QString id = QString::fromStdString(m_module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", m_module->general().id());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    generateWeakForms(output);

    // header - expand template
    std::string text;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/weakform_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorModule::generateWeakForms(ctemplate::TemplateDictionary &output, bool withSpecialFunctions)
{
    this->m_docString = "";
    foreach(XMLModule::weakform_volume weakform, m_module->volume().weakforms_volume().weakform_volume())
    {
        AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype().c_str()));

        foreach(XMLModule::linearity_option option, weakform.linearity_option())
        {
            LinearityType linearityType = linearityTypeFromStringKey(QString::fromStdString(option.type().c_str()));

            // generate individual forms
            QList<FormInfo> matrixForms = WeakFormAgros<double>::wfMatrixVolumeSeparated(m_module, analysisType, linearityType);
            // genrate also complete forms
            //matrixForms.append(WeakFormAgros<double>::wfMatrixVolumeComplete(m_module, analysisType, linearityType));
            foreach(FormInfo formInfo, matrixForms)
            {
                generateForm(formInfo, linearityType, output, weakform, "VOLUME_MATRIX", 0);
            }

            // generate individual forms
            QList<FormInfo> vectorForms = WeakFormAgros<double>::wfVectorVolumeSeparated(m_module, analysisType, linearityType);
            // genrate also complete forms
            //vectorForms.append(WeakFormAgros<double>::wfVectorVolumeComplete(m_module, analysisType, linearityType));
            foreach(FormInfo formInfo, vectorForms)
            {
                generateForm(formInfo, linearityType, output, weakform, "VOLUME_VECTOR", 0);
            }
        }
    }

    foreach(XMLModule::weakform_surface weakform, m_module->surface().weakforms_surface().weakform_surface())
    {
        AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype().c_str()));
        foreach(XMLModule::boundary boundary, weakform.boundary())
        {
            foreach(XMLModule::linearity_option option, boundary.linearity_option())
            {
                LinearityType linearityType = linearityTypeFromStringKey(QString::fromStdString(option.type().c_str()));

                QList<FormInfo> matrixForms = Module::BoundaryType::wfMatrixSurface(&m_module->surface(), &boundary, analysisType, linearityType);
                QList<FormInfo> vectorForms = Module::BoundaryType::wfVectorSurface(&m_module->surface(), &boundary, analysisType, linearityType);
                QList<FormInfo> essentialForms = Module::BoundaryType::essential(&m_module->surface(), &boundary, analysisType, linearityType);
                foreach(FormInfo formInfo, matrixForms)
                {
                    generateForm(formInfo, linearityType, output, weakform, "SURFACE_MATRIX", &boundary);
                }

                foreach(FormInfo formInfo, vectorForms)
                {
                    generateForm(formInfo, linearityType, output, weakform, "SURFACE_VECTOR", &boundary);
                }

                foreach(FormInfo formInfo, essentialForms)
                {
                    generateForm(formInfo, linearityType, output, weakform, "EXACT", &boundary);
                }
            }
        }
    }
}

void Agros2DGeneratorModule::generatePluginFilterFiles()
{
    qDebug() << tr("%1: generating plugin filter file.").arg(QString::fromStdString(m_module->general().id()));

    QString id = QString::fromStdString(m_module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", id.toStdString());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/filter_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    foreach (XMLModule::quantity quantity, m_module->volume().quantity())
    {
        if (quantity.shortname().present())
        {
            ctemplate::TemplateDictionary *variable = output.AddSectionDictionary("VARIABLE_MATERIAL");

            variable->SetValue("MATERIAL_VARIABLE", quantity.id());
        }
    }

    foreach (XMLModule::localvariable lv, m_module->postprocessor().localvariables().localvariable())
    {
        foreach (XMLModule::expression expr, lv.expression())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                if (coordinateType == CoordinateType_Planar)
                {
                    if (lv.type() == "scalar")
                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                               coordinateType,
                                               PhysicFieldVariableComp_Scalar,
                                               QString::fromStdString(expr.planar().get()));
                    if (lv.type() == "vector")
                    {
                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                               coordinateType,
                                               PhysicFieldVariableComp_X,
                                               QString::fromStdString(expr.planar_x().get()));

                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                               coordinateType,
                                               PhysicFieldVariableComp_Y,
                                               QString::fromStdString(expr.planar_y().get()));

                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                               coordinateType,
                                               PhysicFieldVariableComp_Magnitude,
                                               QString("sqrt(pow((double) %1, 2) + pow((double) %2, 2))").arg(QString::fromStdString(expr.planar_x().get())).arg(QString::fromStdString(expr.planar_y().get())));

                    }
                }
                else
                {
                    if (lv.type() == "scalar")
                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                               coordinateType,
                                               PhysicFieldVariableComp_Scalar,
                                               QString::fromStdString(expr.axi().get()));
                    if (lv.type() == "vector")
                    {
                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                               coordinateType,
                                               PhysicFieldVariableComp_X,
                                               QString::fromStdString(expr.axi_r().get()));

                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                               coordinateType,
                                               PhysicFieldVariableComp_Y,
                                               QString::fromStdString(expr.axi_z().get()));

                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                               coordinateType,
                                               PhysicFieldVariableComp_Magnitude,
                                               QString("sqrt(pow((double) %1, 2) + pow((double) %2, 2))").arg(QString::fromStdString(expr.axi_r().get())).arg(QString::fromStdString(expr.axi_z().get())));
                    }
                }
            }
        }
    }

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_filter.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    foreach(XMLModule::function function, m_module->volume().function())
    {
        generateSpecialFunction(&function, &output);
    }

    // source - expand template
    text.clear();
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/filter_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_filter.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorModule::generatePluginForceFiles()
{
    qDebug() << tr("%1: generating plugin force file.").arg(QString::fromStdString(m_module->general().id()));

    QString id = QString::fromStdString(m_module->general().id());


    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", id.toStdString());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/force_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // force
    XMLModule::force force = m_module->postprocessor().force();
    foreach (XMLModule::quantity quantity, m_module->volume().quantity())
    {
        if (quantity.shortname().present())
        {
            ctemplate::TemplateDictionary *variable = output.AddSectionDictionary("VARIABLE_MATERIAL");

            variable->SetValue("MATERIAL_VARIABLE", quantity.id());
        }
    }

    // force
    foreach (XMLModule::expression expr, force.expression())
    {
        AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(expr.analysistype()));

        foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
        {
            ctemplate::TemplateDictionary *expression = output.AddSectionDictionary("VARIABLE_SOURCE");

            expression->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisType).toStdString());
            expression->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());

            if (coordinateType == CoordinateType_Planar)
            {
                expression->SetValue("EXPRESSION_X", parsePostprocessorExpression(analysisType, coordinateType,
                                                                                  QString::fromStdString(expr.planar_x().get())).replace("[i]", "").toStdString());
                expression->SetValue("EXPRESSION_Y", parsePostprocessorExpression(analysisType, coordinateType,
                                                                                  QString::fromStdString(expr.planar_y().get())).replace("[i]", "").toStdString());
                expression->SetValue("EXPRESSION_Z", parsePostprocessorExpression(analysisType, coordinateType,
                                                                                  QString::fromStdString(expr.planar_z().get())).replace("[i]", "").toStdString());
            }
            else
            {
                {
                    expression->SetValue("EXPRESSION_X", parsePostprocessorExpression(analysisType, coordinateType,
                                                                                      QString::fromStdString(expr.axi_r().get())).replace("[i]", "").toStdString());
                    expression->SetValue("EXPRESSION_Y", parsePostprocessorExpression(analysisType, coordinateType,
                                                                                      QString::fromStdString(expr.axi_z().get())).replace("[i]", "").toStdString());
                    expression->SetValue("EXPRESSION_Z", parsePostprocessorExpression(analysisType, coordinateType,
                                                                                      QString::fromStdString(expr.axi_phi().get())).replace("[i]", "").toStdString());
                }
            }
        }
    }


    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_force.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // source - expand template
    text.clear();
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/force_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_force.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorModule::generatePluginLocalPointFiles()
{
    qDebug() << tr("%1: generating plugin local point file.").arg(QString::fromStdString(m_module->general().id()));

    QString id = QString::fromStdString(m_module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", id.toStdString());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/localvalue_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    foreach (XMLModule::quantity quantity, m_module->volume().quantity())
    {
        if (quantity.shortname().present())
        {
            ctemplate::TemplateDictionary *variable = output.AddSectionDictionary("VARIABLE_MATERIAL");

            variable->SetValue("MATERIAL_VARIABLE", quantity.id());
        }
    }

    foreach (XMLModule::localvariable lv, m_module->postprocessor().localvariables().localvariable())
    {
        foreach (XMLModule::expression expr, lv.expression())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                if (coordinateType == CoordinateType_Planar)
                {
                    createLocalValueExpression(output, QString::fromStdString(lv.id()),
                                               analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                               coordinateType,
                                               (expr.planar().present() ? QString::fromStdString(expr.planar().get()) : ""),
                                               (expr.planar_x().present() ? QString::fromStdString(expr.planar_x().get()) : ""),
                                               (expr.planar_y().present() ? QString::fromStdString(expr.planar_y().get()) : ""));
                }
                else
                {
                    createLocalValueExpression(output, QString::fromStdString(lv.id()),
                                               analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                               coordinateType,
                                               (expr.axi().present() ? QString::fromStdString(expr.axi().get()) : ""),
                                               (expr.axi_r().present() ? QString::fromStdString(expr.axi_r().get()) : ""),
                                               (expr.axi_z().present() ? QString::fromStdString(expr.axi_z().get()) : ""));
                }
            }
        }
    }

    foreach(XMLModule::function function, m_module->volume().function())
    {
        generateSpecialFunction(&function, &output);
    }

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_localvalue.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // source - expand template
    text.clear();
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/localvalue_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_localvalue.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorModule::generatePluginSurfaceIntegralFiles()
{
    qDebug() << tr("%1: generating plugin surface integral file.").arg(QString::fromStdString(m_module->general().id()));

    QString id = QString::fromStdString(m_module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", id.toStdString());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/surfaceintegral_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    foreach (XMLModule::quantity quantity, m_module->volume().quantity())
    {
        if (quantity.shortname().present())
        {
            ctemplate::TemplateDictionary *variable = output.AddSectionDictionary("VARIABLE_MATERIAL");

            variable->SetValue("MATERIAL_VARIABLE", quantity.id());
        }
    }

    foreach (XMLModule::surfaceintegral surf, m_module->postprocessor().surfaceintegrals().surfaceintegral())
    {
        foreach (XMLModule::expression expr, surf.expression())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                if (coordinateType == CoordinateType_Planar)
                {
                    createIntegralExpression(output, QString::fromStdString(surf.id()),
                                             analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                             coordinateType,
                                             (expr.planar().present() ? QString::fromStdString(expr.planar().get()) : ""));
                }
                else
                {
                    createIntegralExpression(output, QString::fromStdString(surf.id()),
                                             analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                             coordinateType,
                                             (expr.axi().present() ? QString::fromStdString(expr.axi().get()) : ""));
                }
            }
        }
    }

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_surfaceintegral.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // source - expand template
    text.clear();
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/surfaceintegral_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_surfaceintegral.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorModule::generatePluginVolumeIntegralFiles()
{
    qDebug() << tr("%1: generating plugin volume integral file.").arg(QString::fromStdString(m_module->general().id()));

    QString id = QString::fromStdString(m_module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", id.toStdString());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/volumeintegral_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    foreach (XMLModule::quantity quantity, m_module->volume().quantity())
    {
        if (quantity.shortname().present())
        {
            ctemplate::TemplateDictionary *variable = output.AddSectionDictionary("VARIABLE_MATERIAL");

            variable->SetValue("MATERIAL_VARIABLE", quantity.id());
        }
    }

    foreach(XMLModule::function function, m_module->volume().function())
    {
        generateSpecialFunction(&function, &output);
    }

    foreach (XMLModule::volumeintegral vol, m_module->postprocessor().volumeintegrals().volumeintegral())
    {
        foreach (XMLModule::expression expr, vol.expression())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                if (coordinateType == CoordinateType_Planar)
                {
                    createIntegralExpression(output, QString::fromStdString(vol.id()),
                                             analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                             coordinateType,
                                             (expr.planar().present() ? QString::fromStdString(expr.planar().get()) : ""));
                }
                else
                {
                    createIntegralExpression(output, QString::fromStdString(vol.id()),
                                             analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                             coordinateType,
                                             (expr.axi().present() ? QString::fromStdString(expr.axi().get()) : ""));
                }
            }
        }
    }

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_volumeintegral.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // source - expand template
    text.clear();
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/volumeintegral_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_volumeintegral.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

QString Agros2DGeneratorModule::nonlinearExpression(const QString &variable, AnalysisType analysisType, CoordinateType coordinateType)
{
    // volume
    foreach (XMLModule::weakform_volume wf, m_module->volume().weakforms_volume().weakform_volume())
    {
        if (wf.analysistype() == analysisTypeToStringKey(analysisType).toStdString())
        {
            foreach (XMLModule::quantity quantityAnalysis, wf.quantity())
            {
                if (quantityAnalysis.id() == variable.toStdString())
                {
                    if (coordinateType == CoordinateType_Planar)
                    {
                        if (quantityAnalysis.nonlinearity_planar().present())
                            return QString::fromStdString(quantityAnalysis.nonlinearity_planar().get());
                    }
                    else
                    {
                        if (quantityAnalysis.nonlinearity_axi().present())
                            return QString::fromStdString(quantityAnalysis.nonlinearity_axi().get());
                    }
                }
            }
        }
    }

    // surface
    foreach (XMLModule::weakform_surface wf, m_module->surface().weakforms_surface().weakform_surface())
    {
        if (wf.analysistype() == analysisTypeToStringKey(analysisType).toStdString())
        {
            foreach (XMLModule::boundary boundary, wf.boundary())
            {
                foreach (XMLModule::quantity quantityAnalysis, boundary.quantity())
                {
                    if (quantityAnalysis.id() == variable.toStdString())
                    {
                        if (coordinateType == CoordinateType_Planar)
                        {
                            if (quantityAnalysis.nonlinearity_planar().present())
                                return QString::fromStdString(quantityAnalysis.nonlinearity_planar().get());
                        }
                        else
                        {
                            if (quantityAnalysis.nonlinearity_axi().present())
                                return QString::fromStdString(quantityAnalysis.nonlinearity_axi().get());
                        }
                    }
                }
            }
        }
    }

    return "";
}

QString Agros2DGeneratorModule::dependence(const QString &variable, AnalysisType analysisType)
{
    // volume
    foreach (XMLModule::weakform_volume wf, m_module->volume().weakforms_volume().weakform_volume())
    {
        if (wf.analysistype() == analysisTypeToStringKey(analysisType).toStdString())
        {
            foreach (XMLModule::quantity quantityAnalysis, wf.quantity())
            {
                if (quantityAnalysis.id() == variable.toStdString())
                {
                    if (quantityAnalysis.dependence().present())
                        return QString::fromStdString(quantityAnalysis.dependence().get());
                }
            }
        }
    }

    // surface
    foreach (XMLModule::weakform_surface wf, m_module->surface().weakforms_surface().weakform_surface())
    {
        if (wf.analysistype() == analysisTypeToStringKey(analysisType).toStdString())
        {
            foreach (XMLModule::boundary boundary, wf.boundary())
            {
                foreach (XMLModule::quantity quantityAnalysis, boundary.quantity())
                {
                    if (quantityAnalysis.id() == variable.toStdString())
                    {
                        if (quantityAnalysis.dependence().present())
                            return QString::fromStdString(quantityAnalysis.dependence().get());
                    }
                }
            }
        }
    }

    return "";
}

void Agros2DGeneratorModule::createFilterExpression(ctemplate::TemplateDictionary &output,
                                                    const QString &variable,
                                                    AnalysisType analysisType,
                                                    CoordinateType coordinateType,
                                                    PhysicFieldVariableComp physicFieldVariableComp,
                                                    const QString &expr)
{
    if (!expr.isEmpty())
    {
        ctemplate::TemplateDictionary *expression = output.AddSectionDictionary("VARIABLE_SOURCE");

        expression->SetValue("VARIABLE_HASH", QString::number(qHash(variable)).toStdString());
        expression->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisType).toStdString());
        expression->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
        expression->SetValue("PHYSICFIELDVARIABLECOMP_TYPE", Agros2DGenerator::physicFieldVariableCompStringEnum(physicFieldVariableComp).toStdString());
        expression->SetValue("EXPRESSION", parsePostprocessorExpression(analysisType, coordinateType, expr).toStdString());
    }
}

void Agros2DGeneratorModule::createLocalValueExpression(ctemplate::TemplateDictionary &output,
                                                        const QString &variable,
                                                        AnalysisType analysisType,
                                                        CoordinateType coordinateType,
                                                        const QString &exprScalar,
                                                        const QString &exprVectorX,
                                                        const QString &exprVectorY)
{
    ctemplate::TemplateDictionary *expression = output.AddSectionDictionary("VARIABLE_SOURCE");

    expression->SetValue("VARIABLE", variable.toStdString());
    expression->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisType).toStdString());
    expression->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
    expression->SetValue("EXPRESSION_SCALAR", exprScalar.isEmpty() ? "0" : parsePostprocessorExpression(analysisType, coordinateType, exprScalar).replace("[i]", "").toStdString());
    expression->SetValue("EXPRESSION_VECTORX", exprVectorX.isEmpty() ? "0" : parsePostprocessorExpression(analysisType, coordinateType, exprVectorX).replace("[i]", "").toStdString());
    expression->SetValue("EXPRESSION_VECTORY", exprVectorY.isEmpty() ? "0" : parsePostprocessorExpression(analysisType, coordinateType, exprVectorY).replace("[i]", "").toStdString());
}

void Agros2DGeneratorModule::createIntegralExpression(ctemplate::TemplateDictionary &output,
                                                      const QString &variable,
                                                      AnalysisType analysisType,
                                                      CoordinateType coordinateType,
                                                      const QString &expr)
{
    if (!expr.isEmpty())
    {
        ctemplate::TemplateDictionary *expression = output.AddSectionDictionary("VARIABLE_SOURCE");

        expression->SetValue("VARIABLE", variable.toStdString());
        expression->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisType).toStdString());
        expression->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
        expression->SetValue("EXPRESSION", parsePostprocessorExpression(analysisType, coordinateType, expr).toStdString());
    }
}

LexicalAnalyser *Agros2DGeneratorModule::postprocessorLexicalAnalyser(AnalysisType analysisType, CoordinateType coordinateType)
{
    int numOfSol = Agros2DGenerator::numberOfSolutions(m_module->general().analyses(), analysisType);

    LexicalAnalyser *lex = new LexicalAnalyser();

    // coordinates
    if (coordinateType == CoordinateType_Planar)
    {
        lex->addVariable("tanx");
        lex->addVariable("tany");
        lex->addVariable("velx");
        lex->addVariable("vely");
        lex->addVariable("velz");
        lex->addVariable("x");
        lex->addVariable("y");
        lex->addVariable("tx");
        lex->addVariable("ty");
        lex->addVariable("nx");
        lex->addVariable("ny");
    }
    else
    {
        lex->addVariable("tanr");
        lex->addVariable("tanz");
        lex->addVariable("velr");
        lex->addVariable("velz");
        lex->addVariable("velphi");
        lex->addVariable("r");
        lex->addVariable("z");
        lex->addVariable("tr");
        lex->addVariable("tz");
        lex->addVariable("nr");
        lex->addVariable("nz");
    }

    // functions
    for (int i = 1; i < numOfSol + 1; i++)
    {
        lex->addVariable(QString("value%1").arg(i));
        if (coordinateType == CoordinateType_Planar)
        {
            lex->addVariable(QString("dx%1").arg(i));
            lex->addVariable(QString("dy%1").arg(i));
        }
        else
        {
            lex->addVariable(QString("dr%1").arg(i));
            lex->addVariable(QString("dz%1").arg(i));
        }
    }

    // marker area
    lex->addVariable("area");

    // TODO: duplicate
    // constants
    lex->addVariable("PI");
    lex->addVariable("f");
    foreach (XMLModule::constant cnst, m_module->constants().constant())
        lex->addVariable(QString::fromStdString(cnst.id()));

    // variables
    foreach (XMLModule::quantity quantity, m_module->volume().quantity())
    {
        if (quantity.shortname().present())
        {
            lex->addVariable(QString::fromStdString(quantity.shortname().get()));
            lex->addVariable(QString::fromStdString("d" + quantity.shortname().get()));
        }
    }

    foreach (XMLModule::quantity quantity, m_module->surface().quantity())
    {
        if (quantity.shortname().present())
        {
            lex->addVariable(QString::fromStdString(quantity.shortname().get()));
            lex->addVariable(QString::fromStdString("d" + quantity.shortname().get()));
        }
    }

    return lex;
}

QString Agros2DGeneratorModule::parsePostprocessorExpression(AnalysisType analysisType, CoordinateType coordinateType, const QString &expr, bool includeVariables)
{
    try
    {
        int numOfSol = Agros2DGenerator::numberOfSolutions(m_module->general().analyses(), analysisType);

        QMap<QString, QString> dict;

        // coordinates
        if (coordinateType == CoordinateType_Planar)
        {
            dict["x"] = "x[i]";
            dict["y"] = "y[i]";
            // surface integral
            dict["tanx"] = "tan[i][0]";
            dict["tany"] = "tan[i][1]";
            // velocity (force calculation)
            dict["velx"] = "velocity.x";
            dict["vely"] = "velocity.y";
            dict["velz"] = "velocity.z";
        }
        else
        {
            dict["r"] = "x[i]";
            dict["z"] = "y[i]";
            // surface integral
            dict["tanr"] = "tan[i][0]";
            dict["tanz"] = "tan[i][1]";
            // velocity (force calculation)
            dict["velr"] = "velocity.x";
            dict["velz"] = "velocity.y";
            dict["velphi"] = "velocity.z";
        }

        // constants
        dict["PI"] = "M_PI";
        dict["f"] = "Agros2D::problem()->config()->value(ProblemConfig::Frequency).toDouble()";
        foreach (XMLModule::constant cnst, m_module->constants().constant())
            dict[QString::fromStdString(cnst.id())] = QString::number(cnst.value());

        // functions
        for (int i = 1; i < numOfSol + 1; i++)
        {
            dict[QString("value%1").arg(i)] = QString("value[%1][i]").arg(i-1);
            if (coordinateType == CoordinateType_Planar)
            {
                dict[QString("dx%1").arg(i)] = QString("dudx[%1][i]").arg(i-1);
                dict[QString("dy%1").arg(i)] = QString("dudy[%1][i]").arg(i-1);
            }
            else
            {
                dict[QString("dr%1").arg(i)] = QString("dudx[%1][i]").arg(i-1);
                dict[QString("dz%1").arg(i)] = QString("dudy[%1][i]").arg(i-1);
            }
        }

        // variables
        if (includeVariables)
        {
            foreach (XMLModule::quantity quantity, m_module->volume().quantity())
            {
                if (quantity.shortname().present())
                {
                    QString nonlinearExpr = nonlinearExpression(QString::fromStdString(quantity.id()), analysisType, coordinateType);

                    if (nonlinearExpr.isEmpty())
                        // linear material
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("material_%1->number()").arg(QString::fromStdString(quantity.id()));
                    else
                        // nonlinear material
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("material_%1->numberFromTable(%2)").
                                arg(QString::fromStdString(quantity.id())).
                                arg(parsePostprocessorExpression(analysisType, coordinateType, nonlinearExpr, false));
                }
            }
        }

        LexicalAnalyser *lex = postprocessorLexicalAnalyser(analysisType, coordinateType);
        lex->setExpression(expr);
        QString exprCpp = lex->replaceVariables(dict);

        // TODO: move from lex
        exprCpp = lex->replaceOperatorByFunction(exprCpp);

        delete lex;

        return exprCpp;
    }
    catch (ParserException e)
    {
        qDebug() << e.toString() << "in module: " << QString::fromStdString(m_module->general().id());

        return "";
    }
}

//-----------------------------------------------------------------------------------------

LexicalAnalyser *Agros2DGeneratorModule::weakFormLexicalAnalyser(AnalysisType analysisType, CoordinateType coordinateType)
{
    int numOfSol = Agros2DGenerator::numberOfSolutions(m_module->general().analyses(), analysisType);

    LexicalAnalyser *lex = new LexicalAnalyser();

    // scalar field
    lex->addVariable("uval");
    lex->addVariable("upval");
    lex->addVariable("uptval");
    lex->addVariable("vval");

    // vector field
    lex->addVariable("val0");
    lex->addVariable("val1");
    lex->addVariable("ucurl");
    lex->addVariable("vcurl");
    lex->addVariable("upcurl");

    // coordinates
    if (coordinateType == CoordinateType_Planar)
    {
        // scalar field
        lex->addVariable("udx");
        lex->addVariable("vdx");
        lex->addVariable("udy");
        lex->addVariable("vdy");
        lex->addVariable("updx");
        lex->addVariable("updy");
        lex->addVariable("uptdx");
        lex->addVariable("uptdy");

        // vector field
        lex->addVariable("dx0");
        lex->addVariable("dx1");
        lex->addVariable("dy0");
        lex->addVariable("dy1");

        lex->addVariable(QString("x"));
        lex->addVariable(QString("y"));
    }
    else
    {
        // scalar field
        lex->addVariable("udr");
        lex->addVariable("vdr");
        lex->addVariable("udz");
        lex->addVariable("vdz");
        lex->addVariable("updr");
        lex->addVariable("updz");
        lex->addVariable("uptdr");
        lex->addVariable("uptdz");

        // vector field
        lex->addVariable("dr0");
        lex->addVariable("dr1");
        lex->addVariable("dz0");
        lex->addVariable("dz1");

        lex->addVariable(QString("r"));
        lex->addVariable(QString("z"));
    }

    if (analysisType == AnalysisType_Transient)
    {
        lex->addVariable("deltat");
        lex->addVariable("timedermat");
        lex->addVariable("timedervec");
    }

    // functions
    for (int i = 1; i < numOfSol + 1; i++)
    {
        lex->addVariable(QString("value%1").arg(i));
        if (coordinateType == CoordinateType_Planar)
        {
            lex->addVariable(QString("dx%1").arg(i));
            lex->addVariable(QString("dy%1").arg(i));
        }
        else
        {
            lex->addVariable(QString("dr%1").arg(i));
            lex->addVariable(QString("dz%1").arg(i));
        }
    }

    // TODO: duplicate
    // constants
    lex->addVariable("PI");
    lex->addVariable("f");
    foreach (XMLModule::constant cnst, m_module->constants().constant())
        lex->addVariable(QString::fromStdString(cnst.id()));

    // variables
    foreach (XMLModule::quantity quantity, m_module->volume().quantity())
    {
        if (quantity.shortname().present())
        {
            lex->addVariable(QString::fromStdString(quantity.shortname().get()));
            lex->addVariable(QString::fromStdString("d" + quantity.shortname().get()));
        }
    }

    foreach (XMLModule::quantity quantity, m_module->surface().quantity())
    {
        if (quantity.shortname().present())
        {
            lex->addVariable(QString::fromStdString(quantity.shortname().get()));
            lex->addVariable(QString::fromStdString("d" + quantity.shortname().get()));
        }
    }

    return lex;
}

QString Agros2DGeneratorModule::parseWeakFormExpression(AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType,
                                                        const QString &expr, bool includeVariables, bool errorCalculation)
{
    try
    {
        int numOfSol = Agros2DGenerator::numberOfSolutions(m_module->general().analyses(), analysisType);

        QMap<QString, QString> dict;

        // coordinates
        if (coordinateType == CoordinateType_Planar)
        {
            dict["x"] = "e->x[i]";
            dict["y"] = "e->y[i]";
            dict["tx"] = "e->tx[i]";
            dict["ty"] = "e->ty[i]";
            dict["nx"] = "e->nx[i]";
            dict["ny"] = "e->ny[i]";
        }
        else
        {
            dict["r"] = "e->x[i]";
            dict["z"] = "e->y[i]";
            dict["tr"] = "e->tx[i]";
            dict["tz"] = "e->ty[i]";
            dict["nr"] = "e->nx[i]";
            dict["nz"] = "e->ny[i]";
        }

        // constants
        dict["PI"] = "M_PI";
        dict["f"] = "Agros2D::problem()->config()->value(ProblemConfig::Frequency).toDouble()";
        foreach (XMLModule::constant cnst, m_module->constants().constant())
            dict[QString::fromStdString(cnst.id())] = QString::number(cnst.value());

        // area of a label
        // assumes, that this->getAreas has allways only one component (it is true at the moment, since in Agros we create one form for each label)
        dict["area"] = "this->markerVolume()";

        // functions
        // scalar field
        dict["uval"] = "u->val[i]";
        dict["vval"] = "v->val[i]";
        dict["upval"] = "u_ext[this->j]->val[i]";
        dict["uptval"] = "ext[this->j - this->m_offsetJ]->val[i]";
        dict["deltat"] = "Agros2D::problem()->actualTimeStepLength()";

        // vector field
        dict["uval0"] = "u->val0[i]";
        dict["uval1"] = "u->val1[i]";
        dict["ucurl"] = "u->curl[i]";
        dict["vval0"] = "v->val0[i]";
        dict["vval1"] = "v->val1[i]";
        dict["vcurl"] = "v->curl[i]";
        dict["upcurl"] = "u_ext[this->j]->curl[i]";

        dict["timedermat"] = "(*this->m_table)->matrixFormCoefficient()";
        dict["timedervec"] = "(*this->m_table)->vectorFormCoefficient(ext, this->j, this->m_markerSource->fieldInfo()->numberOfSolutions(), i)";

        if (coordinateType == CoordinateType_Planar)
        {
            // scalar field
            dict["udx"] = "u->dx[i]";
            dict["vdx"] = "v->dx[i]";
            dict["udy"] = "u->dy[i]";
            dict["vdy"] = "v->dy[i]";
            dict["updx"] = "u_ext[this->j]->dx[i]";
            dict["updy"] = "u_ext[this->j]->dy[i]";
            dict["uptdx"] = "ext[this->j]->dx[i]";
            dict["uptdy"] = "ext[this->j]->dy[i]";
        }
        else
        {
            // scalar field
            dict["udr"] = "u->dx[i]";
            dict["vdr"] = "v->dx[i]";
            dict["udz"] = "u->dy[i]";
            dict["vdz"] = "v->dy[i]";
            dict["updr"] = "u_ext[this->j]->dx[i]";
            dict["updz"] = "u_ext[this->j]->dy[i]";
            dict["uptdr"] = "ext[this->j]->dx[i]";
            dict["uptdz"] = "ext[this->j]->dy[i]";
        }

        for (int i = 1; i < numOfSol + 1; i++)
        {
            if (!errorCalculation)
            {
                dict[QString("value%1").arg(i)] = QString("u_ext[%1 + this->m_offsetI]->val[i]").arg(i-1);

                if (coordinateType == CoordinateType_Planar)
                {
                    dict[QString("dx%1").arg(i)] = QString("u_ext[%1 + this->m_offsetI]->dx[i]").arg(i-1);
                    dict[QString("dy%1").arg(i)] = QString("u_ext[%1 + this->m_offsetI]->dy[i]").arg(i-1);
                }
                else
                {
                    dict[QString("dr%1").arg(i)] = QString("u_ext[%1 + this->m_offsetI]->dx[i]").arg(i-1);
                    dict[QString("dz%1").arg(i)] = QString("u_ext[%1 + this->m_offsetI]->dy[i]").arg(i-1);
                }
            }
            else
            {
                // TODO: better !!!
                dict[QString("value%1").arg(i)] = QString("u->val[i]");

                if (coordinateType == CoordinateType_Planar)
                {
                    dict[QString("dx%1").arg(i)] = QString("u->dx[i]");
                    dict[QString("dy%1").arg(i)] = QString("u->dy[i]");
                }
                else
                {
                    dict[QString("dr%1").arg(i)] = QString("u->dx[i]");
                    dict[QString("dz%1").arg(i)] = QString("u->dy[i]");
                }
            }
        }

        // variables
        if (includeVariables)
        {
            foreach (XMLModule::quantity quantity, m_module->volume().quantity())
            {
                if (quantity.shortname().present())
                {
                    QString dep = dependence(QString::fromStdString(quantity.id()), analysisType);
                    QString nonlinearExpr = nonlinearExpression(QString::fromStdString(quantity.id()), analysisType, coordinateType);

                    if (linearityType == LinearityType_Linear || nonlinearExpr.isEmpty())
                    {
                        if (dep.isEmpty())
                        {
                            // linear material
                            dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->number()").
                                    arg(QString::fromStdString(quantity.shortname().get()));
                        }
                        else if (dep == "time")
                        {
                            // linear boundary condition
                            // ERROR: Python expression evaluation doesn't work from weakform ("false" should be removed)
                            dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtTime(Agros2D::problem()->actualTime(), false)").
                                    arg(QString::fromStdString(quantity.shortname().get()));
                        }
                        else if (dep == "space")
                        {
                            // spacedep boundary condition
                            // ERROR: Python expression evaluation doesn't work from weakform - ERROR
                            dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtPoint(Point(x, y))").
                                    arg(QString::fromStdString(quantity.shortname().get()));
                        }
                        else if (dep == "time-space")
                        {
                            // spacedep boundary condition
                            // ERROR: Python expression evaluation doesn't work from weakform - ERROR
                            dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtTimeAndPoint(Agros2D::problem()->actualTime(), Point(x, y))").
                                    arg(QString::fromStdString(quantity.shortname().get()));
                        }
                    }
                    else
                    {
                        // nonlinear material
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberFromTable(%2)").
                                arg(QString::fromStdString(quantity.shortname().get())).
                                arg(parseWeakFormExpression(analysisType, coordinateType, linearityType, nonlinearExpr, false, errorCalculation));

                        if (linearityType == LinearityType_Newton)
                            dict["d" + QString::fromStdString(quantity.shortname().get())] = QString("%1->derivativeFromTable(%2)").
                                    arg(QString::fromStdString(quantity.shortname().get())).
                                    arg(parseWeakFormExpression(analysisType, coordinateType, linearityType, nonlinearExpr, false, errorCalculation));
                    }
                }
            }

            foreach (XMLModule::quantity quantity, m_module->surface().quantity())
            {
                if (quantity.shortname().present())
                {
                    QString dep = dependence(QString::fromStdString(quantity.id()), analysisType);

                    if (dep.isEmpty())
                    {
                        // linear boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->number()").
                                arg(QString::fromStdString(quantity.shortname().get()));
                    }
                    else if (dep == "time")
                    {
                        // linear boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtTime(Agros2D::problem()->actualTime(), false)").
                                arg(QString::fromStdString(quantity.shortname().get()));
                    }
                    else if (dep == "space")
                    {
                        // spacedep boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtPoint(Point(x, y))").
                                arg(QString::fromStdString(quantity.shortname().get()));
                    }
                    else if (dep == "time-space")
                    {
                        // spacedep boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtTimeAndPoint(Agros2D::problem()->actualTime(), Point(x, y))").
                                arg(QString::fromStdString(quantity.shortname().get()));
                    }
                }
            }
        }

        LexicalAnalyser *lex = weakFormLexicalAnalyser(analysisType, coordinateType);
        lex->setExpression(expr);
        QString exprCpp = lex->replaceVariables(dict);

        // TODO: move from lex
        exprCpp = lex->replaceOperatorByFunction(exprCpp);

        delete lex;

        return exprCpp;
    }
    catch (ParserException e)
    {
        qDebug() << e.toString() << "in module: " << QString::fromStdString(m_module->general().id());

        return "";
    }
}

class ValueGenerator
{
public:
    ValueGenerator(int initValue) : m_nextValue(initValue) {}
    QString value()
    {
        m_nextValue++;
        return QString::number(m_nextValue) + ".0"; // MSVC fix pow(int, double) doesn't work
    }

private:
    int m_nextValue;
};

QString Agros2DGeneratorModule::parseWeakFormExpressionCheck(AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType,
                                                             const QString &expr, int generatorStartValue)
{
    try
    {
        int numOfSol = Agros2DGenerator::numberOfSolutions(m_module->general().analyses(), analysisType);

        QMap<QString, QString> dict;
        ValueGenerator generator(generatorStartValue);

        // coordinates
        if (coordinateType == CoordinateType_Planar)
        {
            dict["x"] = generator.value();
            dict["y"] = generator.value();
            dict["tx"] = generator.value();
            dict["ty"] = generator.value();
            dict["nx"] = generator.value();
            dict["ny"] = generator.value();
        }
        else
        {
            dict["r"] = generator.value();
            dict["z"] = generator.value();
            dict["tr"] = generator.value();
            dict["tz"] = generator.value();
            dict["nr"] = generator.value();
            dict["nz"] = generator.value();
        }

        // constants
        dict["PI"] = "M_PI";
        dict["f"] = "Agros2D::problem()->config()->value(ProblemConfig::Frequency).toDouble()";
        foreach (XMLModule::constant cnst, m_module->constants().constant())
            dict[QString::fromStdString(cnst.id())] = QString::number(cnst.value());

        dict["area"] = "1";

        // functions
        // scalar field
        dict["uval"] = generator.value();
        dict["vval"] = generator.value();
        dict["upval"] = generator.value();
        dict["uptval"] = generator.value();

        // vector field
        dict["uval0"] = generator.value();
        dict["uval1"] = generator.value();
        dict["vval0"] = generator.value();
        dict["vval1"] = generator.value();
        dict["ucurl"] = generator.value();
        dict["vcurl"] = generator.value();
        dict["upcurl"] = generator.value();

        dict["deltat"] = "Agros2D::problem()->actualTimeStepLength()";
        dict["timedermat"] = generator.value();
        dict["timedervec"] = generator.value();

        if (coordinateType == CoordinateType_Planar)
        {
            // scalar field
            dict["udx"] = generator.value();
            dict["vdx"] = generator.value();
            dict["udy"] = generator.value();
            dict["vdy"] = generator.value();
            dict["updx"] = generator.value();
            dict["updy"] = generator.value();
            dict["uptdx"] = generator.value();
            dict["uptdy"] = generator.value();
        }
        else
        {
            // scalar field
            dict["udr"] = generator.value();
            dict["vdr"] = generator.value();
            dict["udz"] = generator.value();
            dict["vdz"] = generator.value();
            dict["updr"] = generator.value();
            dict["updz"] = generator.value();
            dict["uptdr"] = generator.value();
            dict["uptdz"] = generator.value();
        }

        for (int i = 1; i < numOfSol + 1; i++)
        {
            dict[QString("value%1").arg(i)] = generator.value();
            if (coordinateType == CoordinateType_Planar)
            {
                dict[QString("dx%1").arg(i)] = generator.value();
                dict[QString("dy%1").arg(i)] = generator.value();
            }
            else
            {
                dict[QString("dr%1").arg(i)] = generator.value();
                dict[QString("dz%1").arg(i)] = generator.value();
            }
        }

        // variables
        foreach (XMLModule::quantity quantity, m_module->volume().quantity())
        {
            if (quantity.shortname().present())
            {
                QString dep = dependence(QString::fromStdString(quantity.id()), analysisType);
                QString nonlinearExpr = nonlinearExpression(QString::fromStdString(quantity.id()), analysisType, coordinateType);

                if (linearityType == LinearityType_Linear || nonlinearExpr.isEmpty())
                {
                    if (dep.isEmpty())
                    {
                        // linear material
                        dict[QString::fromStdString(quantity.shortname().get())] = QString("material->value(\"%1\").number()").
                                arg(QString::fromStdString(quantity.id()));
                    }
                    else if (dep == "time")
                    {
                        // timedep boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                    }
                    else if (dep == "space")
                    {
                        // spacedep boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                    }
                    else if (dep == "time-space")
                    {
                        // spacedep boundary condition
                        dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                    }
                }
                else
                {
                    // nonlinear material
                    dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                    dict["d" + QString::fromStdString(quantity.shortname().get())] = generator.value();
                }
            }
        }

        foreach (XMLModule::quantity quantity, m_module->surface().quantity())
        {
            if (quantity.shortname().present())
            {
                QString dep = dependence(QString::fromStdString(quantity.id()), analysisType);

                if (dep.isEmpty())
                {
                    // linear boundary condition
                    dict[QString::fromStdString(quantity.shortname().get())] = QString("boundary->value(\"%1\").number()").
                            arg(QString::fromStdString(quantity.id()));
                }
                else if (dep == "time")
                {
                    // timedep boundary condition
                    dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                }
                else if (dep == "space")
                {
                    // spacedep boundary condition
                    dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                }
                else if (dep == "time-space")
                {
                    // spacedep boundary condition
                    dict[QString::fromStdString(quantity.shortname().get())] = generator.value();
                }
            }
        }

        LexicalAnalyser *lex = weakFormLexicalAnalyser(analysisType, coordinateType);
        lex->setExpression(expr);
        QString exprCpp = lex->replaceVariables(dict);

        // TODO: move from lex
        exprCpp = lex->replaceOperatorByFunction(exprCpp);

        delete lex;

        return exprCpp;
    }
    catch (ParserException e)
    {
        qDebug() << e.toString() << "in module: " << QString::fromStdString(m_module->general().id());

        return "";
    }
}


template <typename Form, typename WeakForm>
void Agros2DGeneratorModule::generateFormOld(Form form, ctemplate::TemplateDictionary &output, WeakForm weakform, QString weakFormType, int i, XMLModule::boundary *boundary, int j)
{
    // old version of generate form
    // still used for surface forms
    FormInfo formInfo(form.id().c_str(), i, j);
    // LinearityType linearityTypes[2] = {LinearityType_Linear, LinearityType_Newton};
    LinearityType linearityTypes[3] = {LinearityType_Linear, LinearityType_Newton, LinearityType_Picard};
    for(int lt = 0; lt < 3; lt++)
    {
        //assert(0);
        formInfo.expr_planar = this->weakformExpression(CoordinateType_Planar, linearityTypes[lt], form);
        formInfo.expr_axi = this->weakformExpression(CoordinateType_Axisymmetric, linearityTypes[lt], form);
        generateForm(formInfo, linearityTypes[lt], output, weakform, weakFormType, boundary);
    }
}

template <typename WeakForm>
void Agros2DGeneratorModule::generateForm(FormInfo formInfo, LinearityType linearityType, ctemplate::TemplateDictionary &output, WeakForm weakform, QString weakFormType, XMLModule::boundary *boundary)
{
    foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
    {
        QString expression = (coordinateType == CoordinateType_Planar ? formInfo.expr_planar : formInfo.expr_axi);
        if(expression != "")
        {
            ctemplate::TemplateDictionary *field;
            field = output.AddSectionDictionary(weakFormType.toStdString() + "_SOURCE");

            // assert(form.i().present());
            // source files
            QString functionName = QString("%1_%2_%3_%4_%5_%6_%7").
                    arg(weakFormType.toLower()).
                    arg(QString::fromStdString(m_module->general().id())).
                    arg(QString::fromStdString(weakform.analysistype())).
                    arg(coordinateTypeToStringKey(coordinateType)).
                    arg(linearityTypeToStringKey(linearityType)).
                    arg(formInfo.id).
                    arg(QString::number(formInfo.i));

            if (formInfo.j != 0)
            {
                field->SetValue("COLUMN_INDEX", QString::number(formInfo.j).toStdString());
                functionName += "_" + QString::number(formInfo.j);
            }
            else
            {
                field->SetValue("COLUMN_INDEX", "0");
                functionName += "_0";
            }

            if (boundary != 0)
            {
                field->SetValue("BOUNDARY_TYPE", boundary->id().c_str());
                functionName += "_" + QString::fromStdString((boundary->id().c_str()));
                foreach(XMLModule::quantity quantity, boundary->quantity())
                {
                    ctemplate::TemplateDictionary *subField = 0;
                    subField = field->AddSectionDictionary("VARIABLE_SOURCE");
                    subField->SetValue("VARIABLE", quantity.id().c_str());
                    subField->SetValue("VARIABLE_SHORT", m_surfaceVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());
                }
            }
            else
            {
                foreach(XMLModule::quantity quantity, weakform.quantity())
                {
                    ctemplate::TemplateDictionary *subField = 0;
                    subField = field->AddSectionDictionary("VARIABLE_SOURCE");
                    subField->SetValue("VARIABLE", quantity.id().c_str());
                    subField->SetValue("VARIABLE_SHORT", m_volumeVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());
                }
            }

            foreach(XMLModule::function_use functionUse, weakform.function_use())
            {
                foreach(XMLModule::function functionDefinition, m_module->volume().function())
                {
                    if(functionUse.id() == functionDefinition.id())
                    {
                        generateSpecialFunction(&functionDefinition, field);
                    }
                }
            }
            
            field->SetValue("FUNCTION_NAME", functionName.toStdString());
            field->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
            field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
            field->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype()))).toStdString());
            field->SetValue("ROW_INDEX", QString::number(formInfo.i).toStdString());
            field->SetValue("MODULE_ID", m_module->general().id());
            field->SetValue("WEAKFORM_ID", formInfo.id.toStdString());

            // expression
            QString exprCpp = parseWeakFormExpression(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())),
                                                      coordinateType, linearityType, expression);
            field->SetValue("EXPRESSION", exprCpp.toStdString());

            QString exprCppCheck1 = "1";
            // QString exprCppCheck1 = parseWeakFormExpressionCheck(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())),
            //                                                      coordinateType, linearityType, expression, 1);
            field->SetValue("EXPRESSION_CHECK_1", exprCppCheck1.toStdString());
            QString exprCppCheck2 = "1";
            // QString exprCppCheck2 = parseWeakFormExpressionCheck(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())),
            //                                                      coordinateType, linearityType, expression, 17);
            field->SetValue("EXPRESSION_CHECK_2", exprCppCheck2.toStdString());

            // add weakform
            field = output.AddSectionDictionary("SOURCE");
            field->SetValue("FUNCTION_NAME", functionName.toStdString());
        }
    }
}

void Agros2DGeneratorModule::generateSpecialFunction(XMLModule::function* function, ctemplate::TemplateDictionary *output)
{
    ctemplate::TemplateDictionary *functionTemplate = output->AddSectionDictionary("SPECIAL_FUNCTION_SOURCE");
    functionTemplate->SetValue("SPECIAL_FUNCTION_NAME", function->shortname());
    functionTemplate->SetValue("SPECIAL_FUNCTION_FULL_NAME", m_module->general().id() + "_function_" + function->shortname());
    functionTemplate->SetValue("FROM", function->bound_low().present() ? function->bound_low().get() : "-1");
    functionTemplate->SetValue("TO", function->bound_hi().present() ? function->bound_hi().get() : "1");
    functionTemplate->SetValue("TYPE", function->type());
    if(function->extrapolate_low().present())
    {
        functionTemplate->SetValue("EXTRAPOLATE_LOW_PRESENT", "true");
        functionTemplate->SetValue("EXTRAPOLATE_LOW", function->extrapolate_low().get());
    }
    else
    {
        functionTemplate->SetValue("EXTRAPOLATE_LOW_PRESENT", "false");
        functionTemplate->SetValue("EXTRAPOLATE_LOW", "-123456");
    }
    if(function->extrapolate_hi().present())
    {
        functionTemplate->SetValue("EXTRAPOLATE_HI_PRESENT", "true");
        functionTemplate->SetValue("EXTRAPOLATE_HI", function->extrapolate_hi().get());
    }
    else
    {
        functionTemplate->SetValue("EXTRAPOLATE_HI_PRESENT", "false");
        functionTemplate->SetValue("EXTRAPOLATE_HI", "-123456");
    }
    QString selectedVariant("no_variant");
    if(function->switch_combo().present())
    {
        foreach(XMLModule::gui gui, m_module->preprocessor().gui())
        {
            if(gui.type() == "volume")
            {
                foreach(XMLModule::group group, gui.group())
                {
                    foreach(XMLModule::switch_combo switch_combo, group.switch_combo())
                    {
                        if(switch_combo.id() == function->switch_combo().get())
                        {
                            selectedVariant = QString::fromStdString(switch_combo.implicit_option());
                        }
                    }
                }
            }
        }
    }
    functionTemplate->SetValue("SELECTED_VARIANT", selectedVariant.toStdString().c_str());

    foreach(XMLModule::quantity quantity, function->quantity())
    {
        ctemplate::TemplateDictionary *functionParameters = functionTemplate->AddSectionDictionary("PARAMETERS");
        for(int i = 0; i < m_module->volume().quantity().size(); i++)
        {
            if(m_module->volume().quantity().at(i).id() == quantity.id())
            {
                functionParameters->SetValue("PARAMETER_NAME", m_module->volume().quantity().at(i).shortname().get().c_str());
                functionParameters->SetValue("PARAMETER_FULL_NAME", m_module->volume().quantity().at(i).id().c_str());
                break;
            }
        }
    }
    foreach(XMLModule::function_variant variant, function->function_variant())
    {
        ctemplate::TemplateDictionary *functionVariant = functionTemplate->AddSectionDictionary("VARIANT");
        functionVariant->SetValue("ID", variant.switch_value().present() ? variant.switch_value().get().c_str() : "no_variant");
        functionVariant->SetValue("EXPR", variant.expr());
    }
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
