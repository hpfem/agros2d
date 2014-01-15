#include "generator.h"
#include "generator_module.h"
#include "parser.h"

void Agros2DGeneratorModule::generateSpecialFunctionsPostprocessor(ctemplate::TemplateDictionary &output)
{
    foreach(XMLModule::function function, m_module->volume().function())
    {
        if(function.postprocessor_linearity().present())
        {
            assert(function.postprocessor_analysis().present());
            LinearityType linearityType = linearityTypeFromStringKey(QString::fromStdString(function.postprocessor_linearity().get()));
            AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(function.postprocessor_analysis().get()));

            generateSpecialFunction(function, analysisType, linearityType, CoordinateType_Planar, output);
        }
    }
}

void Agros2DGeneratorModule::generatePluginFilterFiles()
{
    Hermes::Mixins::Loggable::Static::info(QString("generating filter file").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", id.toStdString());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

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
            AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(expr.analysistype()));
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                if (coordinateType == CoordinateType_Planar)
                {
                    if (lv.type() == "scalar")
                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisType,
                                               coordinateType,
                                               PhysicFieldVariableComp_Scalar,
                                               QString::fromStdString(expr.planar().get()));
                    if (lv.type() == "vector")
                    {
                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisType,
                                               coordinateType,
                                               PhysicFieldVariableComp_X,
                                               QString::fromStdString(expr.planar_x().get()));

                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisType,
                                               coordinateType,
                                               PhysicFieldVariableComp_Y,
                                               QString::fromStdString(expr.planar_y().get()));

                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisType,
                                               coordinateType,
                                               PhysicFieldVariableComp_Magnitude,
                                               QString("sqrt(pow((double) %1, 2) + pow((double) %2, 2))").arg(QString::fromStdString(expr.planar_x().get())).arg(QString::fromStdString(expr.planar_y().get())));

                    }
                }
                else
                {
                    if (lv.type() == "scalar")
                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisType,
                                               coordinateType,
                                               PhysicFieldVariableComp_Scalar,
                                               QString::fromStdString(expr.axi().get()));
                    if (lv.type() == "vector")
                    {
                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisType,
                                               coordinateType,
                                               PhysicFieldVariableComp_X,
                                               QString::fromStdString(expr.axi_r().get()));

                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisType,
                                               coordinateType,
                                               PhysicFieldVariableComp_Y,
                                               QString::fromStdString(expr.axi_z().get()));

                        createFilterExpression(output, QString::fromStdString(lv.id()),
                                               analysisType,
                                               coordinateType,
                                               PhysicFieldVariableComp_Magnitude,
                                               QString("sqrt(pow((double) %1, 2) + pow((double) %2, 2))").arg(QString::fromStdString(expr.axi_r().get())).arg(QString::fromStdString(expr.axi_z().get())));
                    }
                }

            }
        }
    }

    generateSpecialFunctionsPostprocessor(output);


    // header - expand template
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/filter_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_filter.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

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
    Hermes::Mixins::Loggable::Static::info(QString("generating force file").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());


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

            ParserModuleInfo pmi(*m_module, analysisType, coordinateType, LinearityType_Linear);
            if (coordinateType == CoordinateType_Planar)
            {
                expression->SetValue("EXPRESSION_X", m_parser->parsePostprocessorExpression(pmi, QString::fromStdString(expr.planar_x().get()), true).replace("[i]", "").toStdString());
                expression->SetValue("EXPRESSION_Y", m_parser->parsePostprocessorExpression(pmi, QString::fromStdString(expr.planar_y().get()), true).replace("[i]", "").toStdString());
                expression->SetValue("EXPRESSION_Z", m_parser->parsePostprocessorExpression(pmi, QString::fromStdString(expr.planar_z().get()), true).replace("[i]", "").toStdString());
            }
            else
            {
                {
                    expression->SetValue("EXPRESSION_X", m_parser->parsePostprocessorExpression(pmi, QString::fromStdString(expr.axi_r().get()), true).replace("[i]", "").toStdString());
                    expression->SetValue("EXPRESSION_Y", m_parser->parsePostprocessorExpression(pmi, QString::fromStdString(expr.axi_z().get()), true).replace("[i]", "").toStdString());
                    expression->SetValue("EXPRESSION_Z", m_parser->parsePostprocessorExpression(pmi, QString::fromStdString(expr.axi_phi().get()), true).replace("[i]", "").toStdString());
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
    Hermes::Mixins::Loggable::Static::info(QString("generating local point file").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());

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
            AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(expr.analysistype()));
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                if (coordinateType == CoordinateType_Planar)
                {
                    createLocalValueExpression(output, QString::fromStdString(lv.id()),
                                               analysisType,
                                               coordinateType,
                                               (expr.planar().present() ? QString::fromStdString(expr.planar().get()) : ""),
                                               (expr.planar_x().present() ? QString::fromStdString(expr.planar_x().get()) : ""),
                                               (expr.planar_y().present() ? QString::fromStdString(expr.planar_y().get()) : ""));
                }
                else
                {
                    createLocalValueExpression(output, QString::fromStdString(lv.id()),
                                               analysisType,
                                               coordinateType,
                                               (expr.axi().present() ? QString::fromStdString(expr.axi().get()) : ""),
                                               (expr.axi_r().present() ? QString::fromStdString(expr.axi_r().get()) : ""),
                                               (expr.axi_z().present() ? QString::fromStdString(expr.axi_z().get()) : ""));
                }
            }
        }
    }

    generateSpecialFunctionsPostprocessor(output);

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
    Hermes::Mixins::Loggable::Static::info(QString("generating surface integral file").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());

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

    int counter = 0;
    foreach (XMLModule::surfaceintegral surf, m_module->postprocessor().surfaceintegrals().surfaceintegral())
    {
        foreach (XMLModule::expression expr, surf.expression())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                if (coordinateType == CoordinateType_Planar)
                {
                    createIntegralExpression(output,
                                             "VARIABLE_SOURCE",
                                             QString::fromStdString(surf.id()),
                                             analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                             coordinateType,
                                             (expr.planar().present() ? QString::fromStdString(expr.planar().get()) : ""),
                                             counter);
                }
                else
                {
                    createIntegralExpression(output,
                                             "VARIABLE_SOURCE",
                                             QString::fromStdString(surf.id()),
                                             analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                             coordinateType,
                                             (expr.axi().present() ? QString::fromStdString(expr.axi().get()) : ""),
                                             counter);
                }
            }
        }

        counter++;
    }
    output.SetValue("INTEGRAL_COUNT", QString::number(counter).toStdString());

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
    Hermes::Mixins::Loggable::Static::info(QString("generating volume integral file").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());

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

    generateSpecialFunctionsPostprocessor(output);

    // normal volume integral
    int counter = 0;
    foreach (XMLModule::volumeintegral vol, m_module->postprocessor().volumeintegrals().volumeintegral())
    {
        // normal volume integral
        if (vol.eggshell().present())
            if (vol.eggshell().get() == 1)
                continue;

        foreach (XMLModule::expression expr, vol.expression())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                if (coordinateType == CoordinateType_Planar)
                {
                    createIntegralExpression(output,
                                             "VARIABLE_SOURCE",
                                             QString::fromStdString(vol.id()),
                                             analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                             coordinateType,
                                             (expr.planar().present() ? QString::fromStdString(expr.planar().get()) : ""),
                                             counter);
                }
                else
                {
                    createIntegralExpression(output,
                                             "VARIABLE_SOURCE",
                                             QString::fromStdString(vol.id()),
                                             analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                             coordinateType,
                                             (expr.axi().present() ? QString::fromStdString(expr.axi().get()) : ""),
                                             counter);
                }
            }
        }

        counter++;
    }
    output.SetValue("INTEGRAL_COUNT", QString::number(counter).toStdString());

    // eggshell volume integral
    counter = 0;
    foreach (XMLModule::volumeintegral vol, m_module->postprocessor().volumeintegrals().volumeintegral())
    {
        // normal volume integral
        if (vol.eggshell().present())
        {
            if (vol.eggshell().get() == 1)
            {

                foreach (XMLModule::expression expr, vol.expression())
                {
                    foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
                    {
                        if (coordinateType == CoordinateType_Planar)
                        {
                            createIntegralExpression(output,
                                                     "VARIABLE_SOURCE_EGGSHELL",
                                                     QString::fromStdString(vol.id()),
                                                     analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                                     coordinateType,
                                                     (expr.planar().present() ? QString::fromStdString(expr.planar().get()) : ""),
                                                     counter);
                        }
                        else
                        {
                            createIntegralExpression(output,
                                                     "VARIABLE_SOURCE_EGGSHELL",
                                                     QString::fromStdString(vol.id()),
                                                     analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                                     coordinateType,
                                                     (expr.axi().present() ? QString::fromStdString(expr.axi().get()) : ""),
                                                     counter);
                        }
                    }
                }

                counter++;
            }
        }
    }
    output.SetValue("INTEGRAL_COUNT_EGGSHELL", QString::number(counter).toStdString());

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
        ParserModuleInfo pmi(*m_module, analysisType, coordinateType, LinearityType_Linear);
        expression->SetValue("EXPRESSION", m_parser->parsePostprocessorExpression(pmi, expr, true, true).toStdString());
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

    ParserModuleInfo pmi(*m_module, analysisType, coordinateType, LinearityType_Linear);
    expression->SetValue("VARIABLE", variable.toStdString());
    expression->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisType).toStdString());
    expression->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
    expression->SetValue("EXPRESSION_SCALAR", exprScalar.isEmpty() ? "0" : m_parser->parsePostprocessorExpression(pmi, exprScalar, true).replace("[i]", "").toStdString());
    expression->SetValue("EXPRESSION_VECTORX", exprVectorX.isEmpty() ? "0" : m_parser->parsePostprocessorExpression(pmi, exprVectorX, true).replace("[i]", "").toStdString());
    expression->SetValue("EXPRESSION_VECTORY", exprVectorY.isEmpty() ? "0" : m_parser->parsePostprocessorExpression(pmi, exprVectorY, true).replace("[i]", "").toStdString());
}

void Agros2DGeneratorModule::createIntegralExpression(ctemplate::TemplateDictionary &output,
                                                      const QString &section,
                                                      const QString &variable,
                                                      AnalysisType analysisType,
                                                      CoordinateType coordinateType,
                                                      const QString &expr,
                                                      int pos)
{
    if (!expr.isEmpty())
    {
        ctemplate::TemplateDictionary *expression = output.AddSectionDictionary(section.toStdString());

        ParserModuleInfo pmi(*m_module, analysisType, coordinateType, LinearityType_Linear);
        expression->SetValue("VARIABLE", variable.toStdString());
        expression->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisType).toStdString());
        expression->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
        expression->SetValue("EXPRESSION", m_parser->parsePostprocessorExpression(pmi, expr, true).toStdString());
        expression->SetValue("POSITION", QString::number(pos).toStdString());
    }
}

