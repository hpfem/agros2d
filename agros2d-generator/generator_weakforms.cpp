#include "generator.h"
#include "generator_module.h"
#include "parser.h"

#include "hermes2d/weak_form.h"
#include "hermes2d/module.h"
#include "hermes2d/coupling.h"

void Agros2DGeneratorModule::generatePluginWeakFormFiles()
{
    generatePluginWeakFormSourceFiles();
    generatePluginWeakFormHeaderFiles();
}

void Agros2DGeneratorModule::generatePluginWeakFormSourceFiles()
{
    Hermes::Mixins::Loggable::Static::info(QString("generating weakform source file").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());
    std::string text;

    ExpandTemplate(compatibleFilename(QString("%1/%2/weakform_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                   ctemplate::DO_NOT_STRIP, m_output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorModule::generatePluginWeakFormHeaderFiles()
{
    Hermes::Mixins::Loggable::Static::info(QString("generating weakform header file").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());

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
}


void Agros2DGeneratorModule::generateExtFunctions(ctemplate::TemplateDictionary &output)
{
    foreach(XMLModule::weakform_volume weakform, m_module->volume().weakforms_volume().weakform_volume())
    {
        AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype().c_str()));
        foreach(XMLModule::linearity_option linearityOption, weakform.linearity_option())
        {
            LinearityType linearityType = linearityTypeFromStringKey(QString::fromStdString(linearityOption.type().c_str()));
            foreach(XMLModule::quantity quantity, weakform.quantity())
            {
                generateExtFunction(quantity, analysisType, linearityType, false, output);
                if(quantityIsNonlinear[QString::fromStdString(quantity.id())])
                    generateExtFunction(quantity, analysisType, linearityType, true, output);
            }
       }
    }
}

void Agros2DGeneratorModule::generateSpecialFunctions(ctemplate::TemplateDictionary &output)
{
    foreach(XMLModule::weakform_volume weakform, m_module->volume().weakforms_volume().weakform_volume())
    {
        AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype().c_str()));
        foreach(XMLModule::linearity_option linearityOption, weakform.linearity_option())
        {
            LinearityType linearityType = linearityTypeFromStringKey(QString::fromStdString(linearityOption.type().c_str()));
            foreach(XMLModule::function_use functionUse, weakform.function_use())
            {
                foreach(XMLModule::function function, m_module->volume().function())
                {
                    if(function.id() == functionUse.id())
                    {
                        foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
                        {
                            if(function.interpolation_count().present())
                                generateSpecialFunction(function, analysisType, linearityType, coordinateType, output);
                            else
                                generateValueExtFunction(function, analysisType, linearityType, coordinateType, output);
                        }
                    }
                }
            }
        }
    }
}


void Agros2DGeneratorModule::generateWeakForms(ctemplate::TemplateDictionary &output)
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


void Agros2DGeneratorModule::generateExtFunction(XMLModule::quantity quantity, AnalysisType analysisType, LinearityType linearityType, bool derivative, ctemplate::TemplateDictionary &output)
{
    foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
    {
        QString type("value");
        if(derivative)
            type = "derivative";

        QString functionName = QString("ext_function_%1_%2_%3_%4_%5_%6").
                arg(QString::fromStdString(m_module->general_field().id())).
                arg(analysisTypeToStringKey(analysisType)).
                arg(linearityTypeToStringKey(linearityType)).
                arg(coordinateTypeToStringKey(coordinateType)).
                arg(QString::fromStdString(quantity.id())).
                arg(type);

        ctemplate::TemplateDictionary *field;
        field = output.AddSectionDictionary("EXT_FUNCTION");
        field->SetValue("EXT_FUNCTION_NAME", functionName.toStdString());
        QString dependence("0");
        if(linearityType != LinearityType_Linear)
        {
            if((coordinateType == CoordinateType_Planar) && (quantity.nonlinearity_planar().present()))
                dependence = QString::fromStdString(quantity.nonlinearity_planar().get());
            if((coordinateType == CoordinateType_Axisymmetric) && (quantity.nonlinearity_axi().present()))
                dependence = QString::fromStdString(quantity.nonlinearity_axi().get());

            ParserModuleInfo pmi(*m_module, analysisType, coordinateType, linearityType);
            dependence = m_parser->parseWeakFormExpression(pmi, dependence);
        }

        // nonlinear or constant (in which case numberFromTable returns just a constant number)
        QString valueMethod("numberFromTable");
        if(derivative)
            valueMethod = "derivativeFromTable";

        // other dependence
        if(quantity.dependence().present())
        {
            if(quantity.dependence().get() == "time")
            {
//                valueMethod = "numberAtTime";
//                dependence = "Agros2D::problem()->actualTime()";
            }
            else if(quantity.dependence().get() == "")
            {
                // todo: why are for some quantities in XML dependence=""? remove?
            }
            else
            {
                std::cout << "not implemented " << quantity.dependence().get() << std::endl;
                assert(0);
            }
        }

        QString shortname;
        foreach(XMLModule::quantity quantityDefinition, m_module->volume().quantity())
        {
            if(quantityDefinition.id() == quantity.id())
            {
                shortname = QString::fromStdString(quantityDefinition.shortname().get());
                break;
            }
        }

        field->SetValue("DEPENDENCE", dependence.toStdString());
        field->SetValue("VALUE_METHOD", valueMethod.toStdString());
        field->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
        field->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisType).toStdString());
        field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
        field->SetValue("QUANTITY_ID", quantity.id());
        field->SetValue("QUANTITY_SHORTNAME", shortname.toStdString());
        if(derivative)
            field->SetValue("IS_DERIVATIVE", "true");
        else
            field->SetValue("IS_DERIVATIVE", "false");
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
                    arg(QString::fromStdString(m_module->general_field().id())).
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
                        generateSpecialFunction(functionDefinition, analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), linearityType, coordinateType, *field);
                    }
                }
            }

            field->SetValue("FUNCTION_NAME", functionName.toStdString());
            field->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
            field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
            field->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype()))).toStdString());
            field->SetValue("ROW_INDEX", QString::number(formInfo.i).toStdString());
            field->SetValue("MODULE_ID", m_module->general_field().id());
            field->SetValue("WEAKFORM_ID", formInfo.id.toStdString());

            ParserModuleInfo pmi(*m_module, analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), coordinateType, linearityType);

            // expression
            QString exprCpp = m_parser->parseWeakFormExpression(pmi, expression);
            field->SetValue("EXPRESSION", exprCpp.toStdString());

            QString exprCppCheck = m_parser->parseWeakFormExpressionCheck(pmi, formInfo.condition);
            field->SetValue("EXPRESSION_CHECK", exprCppCheck.toStdString());

            // add weakform
            field = output.AddSectionDictionary("SOURCE");
            field->SetValue("FUNCTION_NAME", functionName.toStdString());
        }
    }
}

void Agros2DGeneratorModule::generateValueExtFunction(XMLModule::function function, AnalysisType analysisType, LinearityType linearityType, CoordinateType coordinateType, ctemplate::TemplateDictionary &output)
{
    ctemplate::TemplateDictionary *functionTemplate = output.AddSectionDictionary("VALUE_FUNCTION_SOURCE");
    functionTemplate->SetValue("VALUE_FUNCTION_NAME", function.shortname());
    functionTemplate->SetValue("VALUE_FUNCTION_ID", function.id());
    functionTemplate->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
    functionTemplate->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
    functionTemplate->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisType).toStdString());

    QString fullName = QString("%1_ext_function_%2_%3_%4_%5").
            arg(QString::fromStdString(m_module->general_field().id())).
            arg(analysisTypeToStringKey(analysisType)).
            arg(coordinateTypeToStringKey(coordinateType)).
            arg(linearityTypeToStringKey(linearityType)).
            arg(QString::fromStdString(function.shortname()));

    functionTemplate->SetValue("VALUE_FUNCTION_FULL_NAME", fullName.toStdString());

    ParserModuleInfo pmi(*m_module, analysisType, coordinateType, linearityType);
    QString dependence("0");
    if(linearityType != LinearityType_Linear)
        dependence = pmi.specialFunctionNonlinearExpression(QString::fromStdString(function.id()));

    dependence = m_parser->parseWeakFormExpression(pmi, dependence);

    functionTemplate->SetValue("DEPENDENCE", dependence.toStdString());

    foreach(XMLModule::weakform_volume weakForm, m_module->volume().weakforms_volume().weakform_volume())
    {
        if(QString::fromStdString(weakForm.analysistype()) == analysisTypeToStringKey(analysisType))
        {
            foreach(XMLModule::quantity quantity, function.quantity())
            {
                QString section;

                foreach(XMLModule::quantity quantDepend, weakForm.quantity())
                {
                    if(quantDepend.id() == quantity.id())
                    {
                        if(quantDepend.nonlinearity_axi().present() || quantDepend.nonlinearity_planar().present())
                            section = "PARAMETERS_NONLINEAR";
                        else
                            section = "PARAMETERS_LINEAR";
                        break;
                    }
                }

                ctemplate::TemplateDictionary *functionParameters = functionTemplate->AddSectionDictionary(section.toStdString());
                for(int i = 0; i < m_module->volume().quantity().size(); i++)
                {
                    if(m_module->volume().quantity().at(i).id() == quantity.id())
                    {
                        functionParameters->SetValue("PARAMETER_NAME", m_module->volume().quantity().at(i).shortname().get().c_str());
                        functionParameters->SetValue("PARAMETER_ID", m_module->volume().quantity().at(i).id().c_str());
                        functionParameters->SetValue("PARAMETER_FULL_NAME", m_module->volume().quantity().at(i).id().c_str());
                        break;
                    }
                }
            }

            break;
        }
    }


    // todo: get rid of variants
    foreach(XMLModule::function_variant variant, function.function_variant())
    {
        QString expression = QString::fromStdString(variant.expr());

        // todo:
        //expression = parseWeakFormExpression(analysisType, coordinateType, linearityType, expression, false, false);
        functionTemplate->SetValue("EXPR", expression.toStdString());
    }

}

void Agros2DGeneratorModule::generateSpecialFunction(XMLModule::function function, AnalysisType analysisType, LinearityType linearityType, CoordinateType coordinateType, ctemplate::TemplateDictionary &output)
{
    ctemplate::TemplateDictionary *functionTemplate = output.AddSectionDictionary("SPECIAL_FUNCTION_SOURCE");
    functionTemplate->SetValue("SPECIAL_FUNCTION_NAME", function.shortname());
    functionTemplate->SetValue("SPECIAL_FUNCTION_ID", function.id());
    functionTemplate->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
    functionTemplate->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
    functionTemplate->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisType).toStdString());

    QString fullName = QString("%1_ext_function_%2_%3_%4_%5").
            arg(QString::fromStdString(m_module->general_field().id())).
            arg(analysisTypeToStringKey(analysisType)).
            arg(coordinateTypeToStringKey(coordinateType)).
            arg(linearityTypeToStringKey(linearityType)).
            arg(QString::fromStdString(function.shortname()));

    functionTemplate->SetValue("SPECIAL_EXT_FUNCTION_FULL_NAME", fullName.toStdString());
    functionTemplate->SetValue("FROM", function.bound_low().present() ? function.bound_low().get() : "-1");
    functionTemplate->SetValue("TO", function.bound_hi().present() ? function.bound_hi().get() : "1");
    functionTemplate->SetValue("TYPE", function.type());

    ParserModuleInfo pmi(*m_module, analysisType, coordinateType, linearityType);
    QString dependence("0");
    if(linearityType != LinearityType_Linear)
        dependence = pmi.specialFunctionNonlinearExpression(QString::fromStdString(function.id()));

    dependence = m_parser->parseWeakFormExpression(pmi, dependence);

    functionTemplate->SetValue("DEPENDENCE", dependence.toStdString());
    functionTemplate->SetValue("INTERPOLATION_COUNT", function.interpolation_count().present() ? function.interpolation_count().get() : "0");
    if(function.extrapolate_low().present())
    {
        functionTemplate->SetValue("EXTRAPOLATE_LOW_PRESENT", "true");
        functionTemplate->SetValue("EXTRAPOLATE_LOW", function.extrapolate_low().get());
    }
    else
    {
        functionTemplate->SetValue("EXTRAPOLATE_LOW_PRESENT", "false");
        functionTemplate->SetValue("EXTRAPOLATE_LOW", "-123456");
    }
    if(function.extrapolate_hi().present())
    {
        functionTemplate->SetValue("EXTRAPOLATE_HI_PRESENT", "true");
        functionTemplate->SetValue("EXTRAPOLATE_HI", function.extrapolate_hi().get());
    }
    else
    {
        functionTemplate->SetValue("EXTRAPOLATE_HI_PRESENT", "false");
        functionTemplate->SetValue("EXTRAPOLATE_HI", "-123456");
    }
    QString selectedVariant("no_variant");
    if(function.switch_combo().present())
    {
        foreach(XMLModule::gui gui, m_module->preprocessor().gui())
        {
            if(gui.type() == "volume")
            {
                foreach(XMLModule::group group, gui.group())
                {
                    foreach(XMLModule::switch_combo switch_combo, group.switch_combo())
                    {
                        if(switch_combo.id() == function.switch_combo().get())
                        {
                            selectedVariant = QString::fromStdString(switch_combo.implicit_option());
                        }
                    }
                }
            }
        }
    }
    functionTemplate->SetValue("SELECTED_VARIANT", selectedVariant.toStdString().c_str());

    foreach(XMLModule::quantity quantity, function.quantity())
    {
        ctemplate::TemplateDictionary *functionParameters = functionTemplate->AddSectionDictionary("PARAMETERS");
        for(int i = 0; i < m_module->volume().quantity().size(); i++)
        {
            if(m_module->volume().quantity().at(i).id() == quantity.id())
            {
                functionParameters->SetValue("PARAMETER_NAME", m_module->volume().quantity().at(i).shortname().get().c_str());
                functionParameters->SetValue("PARAMETER_ID", m_module->volume().quantity().at(i).id().c_str());
                functionParameters->SetValue("PARAMETER_FULL_NAME", m_module->volume().quantity().at(i).id().c_str());
                break;
            }
        }
    }
    foreach(XMLModule::function_variant variant, function.function_variant())
    {
        ctemplate::TemplateDictionary *functionVariant = functionTemplate->AddSectionDictionary("VARIANT");
        functionVariant->SetValue("ID", variant.switch_value().present() ? variant.switch_value().get().c_str() : "no_variant");
        QString expression = QString::fromStdString(variant.expr());

        // todo:
        //expression = parseWeakFormExpression(analysisType, coordinateType, linearityType, expression, false, false);
        functionVariant->SetValue("EXPR", expression.toStdString());
    }
}
