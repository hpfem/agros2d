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
#include "hermes2d/module.h"

#include "ctemplate/template.h"

#include "parser/lex.h"
#include "parser/tree.h"

const QString GENERATOR_TEMPLATEROOT = "generator/templates";
const QString GENERATOR_PLUGINROOT = "weakform_new/plugins/";

Agros2DGenerator::Agros2DGenerator(int &argc, char **argv) : QCoreApplication(argc, argv)
{
    /*
    QString expr = "0.5 * el_epsr * EPS0 * (dx1 * dx1 + dy1 * dy1)";

    LexicalAnalyser lex(expr);
    SyntacticAnalyser synt;

    synt.parse(lex.tokens());
    synt.printTree();
    */
}

void Agros2DGenerator::run()
{
    // create directory
    QDir root(QApplication::applicationDirPath());
    root.mkpath(GENERATOR_PLUGINROOT);

    QMap<QString, QString> modules = availableModules();

    foreach (QString moduleId, modules.keys())
    {
        // read module
        std::auto_ptr<XMLModule::module> module_xsd = XMLModule::module_((datadir().toStdString() + MODULEROOT.toStdString() + "/" + moduleId.toStdString() + ".xml").c_str());
        XMLModule::module *module = module_xsd.get();

        // generate project file
        root.mkpath(GENERATOR_PLUGINROOT + "/" + moduleId);

        generatePluginProjectFile(module);
        generatePluginInterfaceFiles(module);
        generatePluginFilterFiles(module);
    }

    exit(0);
}

void Agros2DGenerator::generatePluginProjectFile(XMLModule::module *module)
{
    QString id = QString::fromStdString(module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", id.toStdString());

    for (int i = 0; i < module->general().analyses().analysis().size(); i++)
    {
        XMLModule::analysis analysis = module->general().analyses().analysis().at(i);

        foreach (CoordinateType coordinateType, coordinateTypeList())
        {
            ctemplate::TemplateDictionary *field = output.AddSectionDictionary("SOURCE");

            field->SetValue("ID", id.toStdString());
            field->SetValue("ANALYSIS_TYPE", analysis.id());
            field->SetValue("COORDINATE_TYPE", coordinateTypeToStringKey(coordinateType).toStdString());
        }
    }

    // expand template
    std::string text;
    ctemplate::ExpandTemplate(QString("%1/%2/module_pro.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // save to file
    writeStringContent(QString("%1/%2/%3/%3.pro").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGenerator::generatePluginInterfaceFiles(XMLModule::module *module)
{
    QString id = QString::fromStdString(module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", module->general().id());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(QString("%1/%2/interface_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // source - expand template
    text.clear();
    foreach(XMLModule::weakform_volume weakform, module->volume().weakforms_volume().weakform_volume())
    {
        foreach(XMLModule::matrix_form matrix_form, weakform.matrix_form())
        {
            foreach(WeakFormKind weakFormType, weakFormTypeList())
            {
                foreach(LinearityType linearityType, linearityTypeList())
                {
                    foreach (CoordinateType coordinateType, coordinateTypeList())
                    {
                        ctemplate::TemplateDictionary *field;
                        switch (weakFormType)
                        {
                        case WeakForm_MatVol:
                            field = output.AddSectionDictionary("MATRIX_VOL_SOURCE");
                            break;
                        case WeakForm_MatSurf:
                            field = output.AddSectionDictionary("MATRIX_SURF_SOURCE");
                            break;
                        case WeakForm_VecVol:
                            field = output.AddSectionDictionary("VECTOR_VOL_SOURCE");
                            break;
                        case WeakForm_VecSurf:
                            field = output.AddSectionDictionary("VECTOR_SURF_SOURCE");
                            break;
                        case WeakForm_ExactSol:
                            field = output.AddSectionDictionary("EXACT_SOURCE");
                            break;
                        default:
                            assert(0);
                        }
                        QString strLinearityTypeCap = linearityTypeStringEnum(linearityType).replace("LinearityType_","");
                        QString strLinearityType = strLinearityTypeCap.toLower();
                        field->SetValue("COORDINATE_TYPE_CAP",  coordinateTypeStringEnum(coordinateType).toStdString());
                        field->SetValue("COORDINATE_TYPE",  coordinateTypeStringEnum(coordinateType).toLower().toStdString());
                        field->SetValue("LINEARITY_TYPE", strLinearityType.toStdString());
                        field->SetValue("LINEARITY_TYPE_CAP", strLinearityTypeCap.toStdString());
                        field->SetValue("ANALYSIS_TYPE", weakform.analysistype());
                        field->SetValue("ROW_INDEX", QString::number(matrix_form.j()).toStdString());
                        field->SetValue("COLUMN_INDEX", QString::number(matrix_form.i()).toStdString());
                        field->SetValue("MODULE_ID", module->general().id());

                    }
                }
            }
        }
    }
    ctemplate::ExpandTemplate(QString("%1/%2/interface_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);
    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}





void Agros2DGenerator::generatePluginFilterFiles(XMLModule::module *module)
{
    QString id = QString::fromStdString(module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", id.toStdString());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(QString("%1/%2/filter_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    for (int i = 0; i < module->postprocessor().localvariables().localvariable().size(); i++)
    {
        XMLModule::localvariable lv = module->postprocessor().localvariables().localvariable().at(i);

        for (int j = 0; j < lv.expression().size(); j++)
        {
            XMLModule::expression expr = lv.expression().at(j);

            foreach (CoordinateType coordinateType, coordinateTypeList())
            {
                std::string exprVar;
                if (coordinateType == CoordinateType_Planar)
                {
                    if (expr.planar().present())
                        exprVar = expr.planar().get();
                }
                else
                {
                    if (expr.axi().present())
                        exprVar = expr.axi().get();
                }

                if (!exprVar.empty())
                {
                    ctemplate::TemplateDictionary *expression = output.AddSectionDictionary("VARIABLE_SOURCE");

                    expression->SetValue("VARIABLE", lv.id());
                    expression->SetValue("ANALYSIS_TYPE", analysisTypeStringEnum(analysisTypeFromStringKey(QString::fromStdString(expr.analysistype()))).toStdString());
                    expression->SetValue("COORDINATE_TYPE", coordinateTypeStringEnum(coordinateType).toStdString());
                    expression->SetValue("EXPRESSION", exprVar);
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

    // source - expand template
    text.clear();
    ctemplate::ExpandTemplate(QString("%1/%2/filter_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_filter.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}



