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
#include "generator_coupling.h"

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

    Hermes::Mixins::Loggable::Static::info(QString("generating project file").toLatin1());

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

    Hermes::Mixins::Loggable::Static::info(QString("generating interface file").toLatin1());

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

//    foreach(XMLModule::weakform_volume weakform, m_coupling->volume().weakforms_volume().weakform_volume())
//    {
//        foreach(XMLModule::matrix_form form, weakform.matrix_form())
//        {
//            generateForm(form, weakform, output, "VOLUME_MATRIX");
//        }

//        foreach(XMLModule::vector_form form, weakform.vector_form())
//        {
//            generateForm(form, weakform, output, "VOLUME_VECTOR");
//        }
//    }

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
    Hermes::Mixins::Loggable::Static::warn(QString("Coupling: %1.").arg(QString::fromStdString(m_coupling->general_coupling().id())).toLatin1());

    generatePluginWeakFormSourceFiles();
    generatePluginWeakFormHeaderFiles();
}

void Agros2DGeneratorCoupling::generatePluginWeakFormHeaderFiles()
{
    QString id = QString::fromStdString(m_coupling->general_coupling().id());

    Hermes::Mixins::Loggable::Static::info(QString("generating weakform header file").toLatin1());

//    foreach(XMLModule::weakform_volume weakform, m_coupling->volume().weakforms_volume().weakform_volume())
//    {
//        foreach(XMLModule::matrix_form form, weakform.matrix_form())
//        {
//            generateForm(form, weakform, output, "VOLUME_MATRIX");
//        }

//        foreach(XMLModule::vector_form form, weakform.vector_form())
//        {
//            generateForm(form, weakform, output, "VOLUME_VECTOR");
//        }
//    }

    // header - expand template
    std::string text;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/coupling_weakform_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &text);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}


void Agros2DGeneratorCoupling::generatePluginWeakFormSourceFiles()
{
    QString id = QString::fromStdString(m_coupling->general_coupling().id());
    QStringList modules = QString::fromStdString(m_coupling->general_coupling().id()).split("-");

    Hermes::Mixins::Loggable::Static::info(QString("generating weakform source file").toLatin1());

//    foreach(XMLModule::weakform_volume weakform, m_coupling->volume().weakforms_volume().weakform_volume())
//    {
//        foreach(XMLModule::matrix_form form, weakform.matrix_form())
//        {
//            generateForm(form, weakform, output, "VOLUME_MATRIX");
//        }

//        foreach(XMLModule::vector_form form, weakform.vector_form())
//        {
//            generateForm(form, weakform, output, "VOLUME_VECTOR");
//        }
//    }

    std::string text;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/coupling_weakform_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}



QString Agros2DGeneratorCoupling::nonlinearExpression(const QString &variable, AnalysisType analysisType, CoordinateType coordinateType)
{
    foreach (XMLModule::weakform_volume wf, m_targetModule->volume().weakforms_volume().weakform_volume())
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

    foreach (XMLModule::weakform_volume wf, m_sourceModule->volume().weakforms_volume().weakform_volume())
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


    return "";
}

QString Agros2DGeneratorCoupling::parseWeakFormExpression(AnalysisType sourceAnalysisType, AnalysisType targetAnalysisType,CoordinateType coordinateType, const QString &expr)
{
    int numOfSol = Agros2DGenerator::numberOfSolutions(m_sourceModule->general_field().analyses(), sourceAnalysisType) +
            Agros2DGenerator::numberOfSolutions(m_targetModule->general_field().analyses(), targetAnalysisType);

    LexicalAnalyser lex;

    lex.addVariable("uval");
    lex.addVariable("upval");
    lex.addVariable("uptval");
    lex.addVariable("vval");

    // coordinates
    if (coordinateType == CoordinateType_Planar)
    {
        lex.addVariable("udx");
        lex.addVariable("vdx");
        lex.addVariable("udy");
        lex.addVariable("vdy");
        lex.addVariable("updx");
        lex.addVariable("updy");
        lex.addVariable("x");
        lex.addVariable("y");
    }
    else
    {
        lex.addVariable("udr");
        lex.addVariable("vdr");
        lex.addVariable("udz");
        lex.addVariable("vdz");
        lex.addVariable("updr");
        lex.addVariable("updz");
        lex.addVariable("r");
        lex.addVariable("z");
    }

    // functions
    for (int i = 0; i < numOfSol + 1; i++)
    {
        lex.addVariable(QString("value%1").arg(i));
        if (coordinateType == CoordinateType_Planar)
        {
            lex.addVariable(QString("dx%1").arg(i));
            lex.addVariable(QString("dy%1").arg(i));
        }
        else
        {
            lex.addVariable(QString("dr%1").arg(i));
            lex.addVariable(QString("dz%1").arg(i));
        }
        lex.addVariable(QString("source%1").arg(i));
        lex.addVariable(QString("source%1dx").arg(i));
        lex.addVariable(QString("source%1dy").arg(i));
        lex.addVariable(QString("source%1dr").arg(i));
        lex.addVariable(QString("source%1dz").arg(i));
    }

    // constants
    lex.addVariable("PI");
    lex.addVariable("f");
    foreach (XMLModule::constant cnst, m_coupling->constants().constant())
        lex.addVariable(QString::fromStdString(cnst.id()));

    // variables
    foreach (XMLModule::quantity quantity, m_sourceModule->volume().quantity())
    {
        if (quantity.shortname().present())
        {
            lex.addVariable(QString::fromStdString(quantity.shortname().get()));
            lex.addVariable(QString::fromStdString("d" + quantity.shortname().get()));
        }
    }

    foreach (XMLModule::quantity quantity, m_targetModule->volume().quantity())
    {
        if (quantity.shortname().present())
        {
            lex.addVariable(QString::fromStdString(quantity.shortname().get()));
            lex.addVariable(QString::fromStdString("d" + quantity.shortname().get()));
        }
    }

    try
    {
        lex.setExpression(expr);

        // replace tokens
        QString exprCpp;
        foreach (Token token, lex.tokens())
        {
            QString repl = token.toString();

            bool isReplaced = false;

            // coordinates
            if (coordinateType == CoordinateType_Planar)
            {
                if (repl == "x") { exprCpp += "e->x[i]"; isReplaced = true; }
                if (repl == "y") { exprCpp += "e->y[i]"; isReplaced = true; }
            }
            else
            {
                if (repl == "r") { exprCpp += "e->x[i]"; isReplaced = true; }
                if (repl == "z") { exprCpp += "e->y[i]"; isReplaced = true; }
            }

            // constants
            if (repl == "PI") { exprCpp += "M_PI"; isReplaced = true; }
            if (repl == "f") { exprCpp += "this->m_markerSource->fieldInfo()->frequency()"; isReplaced = true; }
            foreach (XMLModule::constant cnst, m_coupling->constants().constant())
                if (repl == QString::fromStdString(cnst.id())) { exprCpp += QString::number(cnst.value()); isReplaced = true; }

            // functions

            if (repl == QString("uval")) { exprCpp += QString("u->val[i]"); isReplaced = true; }
            if (repl == QString("vval")) { exprCpp += QString("v->val[i]"); isReplaced = true; }
            if (repl == QString("upval")) { exprCpp += QString("u_ext[this->j]->val[i]"); isReplaced = true; }
            if (repl == QString("uptval")) { exprCpp += QString("ext[dfsfdsthis->i]->val[i]"); isReplaced = true; }

            if (coordinateType == CoordinateType_Planar)
            {
                if (repl == QString("udx")) { exprCpp += QString("u->dx[i]"); isReplaced = true; }
                if (repl == QString("vdx")) { exprCpp += QString("v->dx[i]"); isReplaced = true; }
                if (repl == QString("udy")) { exprCpp += QString("u->dy[i]"); isReplaced = true; }
                if (repl == QString("vdy")) { exprCpp += QString("v->dy[i]"); isReplaced = true; }
                if (repl == QString("updx")) { exprCpp += QString("u_ext[this->j]->dx[i]"); isReplaced = true; }
                if (repl == QString("updy")) { exprCpp += QString("u_ext[this->j]->dy[i]"); isReplaced = true; }

            }
            else
            {
                if (repl == QString("udr")) { exprCpp += QString("u->dx[i]"); isReplaced = true; }
                if (repl == QString("vdr")) { exprCpp += QString("v->dx[i]"); isReplaced = true; }
                if (repl == QString("udz")) { exprCpp += QString("u->dy[i]"); isReplaced = true; }
                if (repl == QString("vdz")) { exprCpp += QString("v->dy[i]"); isReplaced = true; }
                if (repl == QString("updr")) { exprCpp += QString("u_ext[this->j]->dx[i]"); isReplaced = true; }
                if (repl == QString("updz")) { exprCpp += QString("u_ext[this->j]->dy[i]"); isReplaced = true; }
            }

            for (int i = 1; i < numOfSol + 1; i++)
            {
                if (repl == QString("value%1").arg(i)) { exprCpp += QString("u_ext[%1 + offset.forms /*todo: jinak, I i J*/]->val[i]").arg(i-1); isReplaced = true; }
                if (coordinateType == CoordinateType_Planar)
                {
                    if (repl == QString("dx%1").arg(i)) { exprCpp += QString("u_ext[%1 + offset.forms /*todo: jinak, I i J*/]->dx[i]").arg(i-1); isReplaced = true; }
                    if (repl == QString("dy%1").arg(i)) { exprCpp += QString("u_ext[%1 + offset.forms /*todo: jinak, I i J*/]->dy[i]").arg(i-1); isReplaced = true; }
                }
                else
                {
                    if (repl == QString("dr%1").arg(i)) { exprCpp += QString("u_ext[%1 + offset.forms /*todo: jinak, I i J*/]->dx[i]").arg(i-1); isReplaced = true; }
                    if (repl == QString("dz%1").arg(i)) { exprCpp += QString("u_ext[%1 + offset.forms /*todo: jinak, I i J*/]->dy[i]").arg(i-1); isReplaced = true; }
                }
                if (repl == QString("source%1").arg(i)) { exprCpp += QString("ext[%1 + offset.sourcePrevSol]->val[i]").arg(i-1); isReplaced = true; }
                if (repl == QString("source%1dx").arg(i)) { exprCpp += QString("ext[%1 + offset.sourcePrevSol]->dx[i]").arg(i-1); isReplaced = true; }
                if (repl == QString("source%1dy").arg(i)) { exprCpp += QString("ext[%1 + offset.sourcePrevSol]->dy[i]").arg(i-1); isReplaced = true; }
                if (repl == QString("source%1dr").arg(i)) { exprCpp += QString("ext[%1 + offset.sourcePrevSol]->dx[i]").arg(i-1); isReplaced = true; }
                if (repl == QString("source%1dz").arg(i)) { exprCpp += QString("ext[%1 + offset.sourcePrevSol]->dy[i]").arg(i-1); isReplaced = true; }
            }

//            foreach (XMLModule::quantity quantity, m_sourceModule->volume().quantity())
//            {
//                if (quantity.shortname().present())
//                    if ((repl == QString::fromStdString(quantity.shortname().get())) || repl == QString::fromStdString("d" + quantity.shortname().get()))
//                    {
//                        QString nonlinearExpr = nonlinearExpression(QString::fromStdString(quantity.id()), sourceAnalysisType, coordinateType);
//                        if (nonlinearExpr.isEmpty())
//                            // linear material
//                            exprCpp += QString("%1->number()").arg(QString::fromStdString(quantity.shortname().get()));
//                        else
//                        {
//                            // nonlinear material
//                            if (repl == QString::fromStdString(quantity.shortname().get()))
//                                exprCpp += QString("%1->value(%2)").
//                                        arg(QString::fromStdString(quantity.shortname().get())).
//                                        arg(parseWeakFormExpression(sourceAnalysisType, targetAnalysisType, coordinateType, nonlinearExpr));
//                            if (repl == QString::fromStdString("d" + quantity.shortname().get()))
//                                exprCpp += QString("%1->derivative(%2)").
//                                        arg(QString::fromStdString(quantity.shortname().get())).
//                                        arg(parseWeakFormExpression(sourceAnalysisType, targetAnalysisType, coordinateType, nonlinearExpr));
//                        }

//                        isReplaced = true;
//                    }
//            }

            foreach (XMLModule::quantity quantity, m_sourceModule->volume().quantity())
            {
                if (quantity.shortname().present())
                {
                    if (repl == QString::fromStdString(quantity.shortname().get()))
                    {
                        QString nonlinearExpr = nonlinearExpression(QString::fromStdString(quantity.id()), sourceAnalysisType, coordinateType);
                        if (nonlinearExpr.isEmpty())
                        {
                            // linear material
                            exprCpp += QString("ext[%1 + offset.sourceQuant]->val[i]").arg(sourceQuantityOrdering[QString::fromStdString(quantity.id())]);
                        }
                        else
                        {
                            // todo: not implemented
                            assert(0);
                        }

                        isReplaced = true;
                    }
                    if (repl == QString::fromStdString("d" + quantity.shortname().get()))
                    {
                        // todo: not implemented
                        assert(0);
                    }
                }
            }

            foreach (XMLModule::quantity quantity, m_targetModule->volume().quantity())
            {
                if (quantity.shortname().present())
                {
                    if (repl == QString::fromStdString(quantity.shortname().get()))
                    {
                        QString nonlinearExpr = nonlinearExpression(QString::fromStdString(quantity.id()), targetAnalysisType, coordinateType);
                        if (nonlinearExpr.isEmpty())
                        {
                            // linear material
                            exprCpp += QString("ext[%1 + offset.quant]->val[i]").arg(quantityOrdering[QString::fromStdString(quantity.id())]);
                        }
                        else
                        {
                            // todo: not implemented
                            assert(0);
                        }

                        isReplaced = true;
                    }
                    if (repl == QString::fromStdString("d" + quantity.shortname().get()))
                    {
                        // todo: not implemented
                        assert(0);
                    }
                }
            }

            // operators and other
            if (!isReplaced)
                exprCpp += repl;
        }

        // TODO: move from lex
        exprCpp = lex.replaceOperatorByFunction(exprCpp);
        return exprCpp;
    }
    catch (ParserException e)
    {
        Hermes::Mixins::Loggable::Static::error(QString("%1 in coupling %2").arg(e.toString()).arg(QString::fromStdString(m_coupling->general_coupling().id())).toLatin1());

        return "";
    }
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
            QString functionName = QString("%1_%2_%3_%4_%5_%6_%7_%8_%9").
                    arg(weakFormType.toLower()).
                    arg(id).
                    arg(QString::fromStdString(weakform.sourceanalysis().get())).
                    arg(QString::fromStdString(weakform.analysistype())).
                    arg(coordinateTypeToStringKey(coordinateType)).
                    arg(linearityTypeToStringKey(linearityType)).
                    arg(formInfo.id).
                    arg(QString::fromStdString(weakform.couplingtype().get())).
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

//            if (boundary != 0)
//            {
//                assert(0);
//            }
//            else
            {
                foreach(XMLModule::quantity quantity, m_sourceModule->volume().quantity())
                {
                    ctemplate::TemplateDictionary *subField = field->AddSectionDictionary("VARIABLE_SOURCE");
                    subField->SetValue("VARIABLE", quantity.id().c_str());
                    subField->SetValue("VARIABLE_SHORT", m_sourceVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());
                }

                foreach(XMLModule::quantity quantity, m_targetModule->volume().quantity())
                {
                    ctemplate::TemplateDictionary *subField = field->AddSectionDictionary("VARIABLE_TARGET");
                    subField->SetValue("VARIABLE", quantity.id().c_str());
                    subField->SetValue("VARIABLE_SHORT", m_targetVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());
                }
            }

//            foreach(XMLModule::function_use functionUse, weakform.function_use())
//            {
//                foreach(XMLModule::function functionDefinition, m_module->volume().function())
//                {
//                    if(functionUse.id() == functionDefinition.id())
//                    {
//                        generateSpecialFunction(functionDefinition, analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), linearityType, coordinateType, *field);
//                    }
//                }
//            }

            field->SetValue("FUNCTION_NAME", functionName.toStdString());
            field->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
            field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
            field->SetValue("SOURCE_ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisTypeFromStringKey(QString::fromStdString(weakform.sourceanalysis().get()))).toStdString());
            field->SetValue("TARGET_ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype()))).toStdString());
            field->SetValue("ROW_INDEX", QString::number(formInfo.i).toStdString());
            field->SetValue("MODULE_ID", id.toStdString());
            field->SetValue("WEAKFORM_ID", formInfo.id.toStdString());
            field->SetValue("COUPLING_TYPE", Agros2DGenerator::couplingTypeToString(weakform.couplingtype().get().c_str()).toStdString());

            // expression
            QString exprCpp = parseWeakFormExpression(analysisTypeFromStringKey(QString::fromStdString(weakform.sourceanalysis().get())),
                                                      analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())),
                                                      coordinateType,
                                                      expression);
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
            // genrate also complete forms
            //matrixForms.append(WeakFormAgros<double>::wfMatrixVolumeComplete(m_module, analysisType, linearityType));
            foreach(FormInfo formInfo, matrixForms)
            {
                generateForm(formInfo, linearityType, output, weakform, "VOLUME_MATRIX");
            }

            // generate individual forms
            QList<FormInfo> vectorForms = CouplingInfo::wfVectorVolumeSeparated(&m_coupling->volume(), sourceAnalysis, targetAnalysis, couplingType, linearityType);
            // genrate also complete forms
            //vectorForms.append(WeakFormAgros<double>::wfVectorVolumeComplete(m_module, analysisType, linearityType));
            foreach(FormInfo formInfo, vectorForms)
            {
                generateForm(formInfo, linearityType, output, weakform, "VOLUME_VECTOR");
            }
        }
    }

//    foreach(XMLModule::weakform_surface weakform, m_module->surface().weakforms_surface().weakform_surface())
//    {
//        AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype().c_str()));
//        foreach(XMLModule::boundary boundary, weakform.boundary())
//        {
//            foreach(XMLModule::linearity_option option, boundary.linearity_option())
//            {
//                LinearityType linearityType = linearityTypeFromStringKey(QString::fromStdString(option.type().c_str()));

//                QList<FormInfo> matrixForms = Module::BoundaryType::wfMatrixSurface(&m_module->surface(), &boundary, analysisType, linearityType);
//                QList<FormInfo> vectorForms = Module::BoundaryType::wfVectorSurface(&m_module->surface(), &boundary, analysisType, linearityType);
//                QList<FormInfo> essentialForms = Module::BoundaryType::essential(&m_module->surface(), &boundary, analysisType, linearityType);
//                foreach(FormInfo formInfo, matrixForms)
//                {
//                    generateForm(formInfo, linearityType, output, weakform, "SURFACE_MATRIX", &boundary);
//                }

//                foreach(FormInfo formInfo, vectorForms)
//                {
//                    generateForm(formInfo, linearityType, output, weakform, "SURFACE_VECTOR", &boundary);
//                }

//                foreach(FormInfo formInfo, essentialForms)
//                {
//                    generateForm(formInfo, linearityType, output, weakform, "EXACT", &boundary);
//                }
//            }
//        }
//    }
}


void Agros2DGeneratorCoupling::prepareWeakFormsOutput()
{
    Hermes::Mixins::Loggable::Static::info(QString("parsing weak forms").toLatin1());
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

//    QString description = QString::fromStdString(m_module->general().description());
//    description = description.replace("\n","");
//    m_output->SetValue("DESCRIPTION", description.toStdString());
//    if (m_module->cpp().present())
//        m_output->SetValue("CPP", m_module->cpp().get());

//    generateSpecialFunctions(*m_output);
//    generateExtFunctions(*m_output);
    generateWeakForms(*m_output);

//    foreach(QString name, m_names)
//    {
//        ctemplate::TemplateDictionary *field = m_output->AddSectionDictionary("NAMES");
//        field->SetValue("NAME",name.toStdString());
//    }

}

void Agros2DGeneratorCoupling::deleteWeakFormOutput()
{
    delete m_output;
    m_output = nullptr;
}

