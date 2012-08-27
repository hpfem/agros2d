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


#include <QDir>
#include "generator.h"
#include "hermes2d/module.h"
#include "hermes2d/coupling.h"

#include "parser/lex.h"
#include "parser/tree.h"

const QString GENERATOR_TEMPLATEROOT = "generator/templates";
const QString GENERATOR_PLUGINROOT = "plugins";

// functions

QList<WeakFormKind> Agros2DGenerator::weakFormTypeList()
{
    QList<WeakFormKind> list;
    list << WeakForm_MatVol << WeakForm_MatSurf << WeakForm_VecVol << WeakForm_VecSurf << WeakForm_ExactSol;

    return list;
}

QString Agros2DGenerator::weakFormTypeStringEnum(WeakFormKind weakformType)
{
    switch (weakformType)
    {
    case WeakForm_MatVol:
        return("WeakForm_MatVol");
        break;
    case WeakForm_MatSurf:
        return("WeakForm_MatSurf");
        break;
    case WeakForm_VecVol:
        return("WeakForm_VecVol");
        break;
    case WeakForm_VecSurf:
        return("WeakForm_VecSurf");
        break;
    case WeakForm_ExactSol:
        return("WeakForm_ExactSol");
        break;
    default:
        assert(0);
    }
}

QList<LinearityType> Agros2DGenerator::linearityTypeList()
{
    QList<LinearityType> list;
    list << LinearityType_Linear << LinearityType_Newton << LinearityType_Picard << LinearityType_Undefined;

    return list;
}

QString Agros2DGenerator::linearityTypeStringEnum(LinearityType linearityType)
{
    switch (linearityType)
    {
    case LinearityType_Linear:
        return ("LinearityType_Linear");
        break;
    case LinearityType_Newton:
        return ("LinearityType_Newton");
        break;
    case LinearityType_Picard:
        return ("LinearityType_Picard");
        break;
    case LinearityType_Undefined:
        return ("LinearityType_Undefined");
        break;
    default:
        assert(0);
    }
}

QString Agros2DGenerator::physicFieldVariableCompStringEnum(PhysicFieldVariableComp physicFieldVariableComp)
{
    if (physicFieldVariableComp == PhysicFieldVariableComp_Scalar)
        return "PhysicFieldVariableComp_Scalar";
    else if (physicFieldVariableComp == PhysicFieldVariableComp_Magnitude)
        return "PhysicFieldVariableComp_Magnitude";
    else if (physicFieldVariableComp == PhysicFieldVariableComp_X)
        return "PhysicFieldVariableComp_X";
    else if (physicFieldVariableComp == PhysicFieldVariableComp_Y)
        return "PhysicFieldVariableComp_Y";
    else
        assert(0);
}

QList<CoordinateType> Agros2DGenerator::coordinateTypeList()
{
    QList<CoordinateType> list;
    list << CoordinateType_Planar << CoordinateType_Axisymmetric;

    return list;
}

QString Agros2DGenerator::coordinateTypeStringEnum(CoordinateType coordinateType)
{
    if (coordinateType == CoordinateType_Planar)
        return "CoordinateType_Planar";
    else if (coordinateType == CoordinateType_Axisymmetric)
        return "CoordinateType_Axisymmetric";
    else
        assert(0);
}

QString Agros2DGenerator::analysisTypeStringEnum(AnalysisType analysisType)
{
    if (analysisType == AnalysisType_SteadyState)
        return "AnalysisType_SteadyState";
    else if (analysisType == AnalysisType_Transient)
        return "AnalysisType_Transient";
    else if (analysisType == AnalysisType_Harmonic)
        return "AnalysisType_Harmonic";
    else
        assert(0);
}

QString Agros2DGenerator::boundaryTypeString(const QString boundaryName)
{
    return boundaryName.toLower().replace(" ","_");
}

int Agros2DGenerator::numberOfSolutions(XMLModule::analyses analyses, AnalysisType analysisType)
{
    foreach (XMLModule::analysis analysis, analyses.analysis())
        if (analysis.id() == analysisTypeToStringKey(analysisType).toStdString())
            return analysis.solutions();

    return -1;
}

// *****************************************************************************************************************

Agros2DGenerator::Agros2DGenerator(int &argc, char **argv) : QCoreApplication(argc, argv)
{
    /*
    QString expr = "0.5  * (dx1 * dx1 + dy1 * dy1)   ";

    LexicalAnalyser lex;
    lex.addVariable("dx1");
    lex.addVariable("dy1");
    lex.addVariable("epsr");
    lex.addVariable("EPS0");

    lex.setExpression(expr);
    lex.printTokens();
    */
    /*
    double dx1 = 2;
    double dy1 = 3;
    double el_epsr = 5;
    double eps0 = 4;

    SyntacticAnalyser synt;
    synt.m_variableMap.insert("dx1", &dx1);
    synt.m_variableMap.insert("dy1", &dy1);
    synt.m_variableMap.insert("epsr", &el_epsr);
    synt.m_variableMap.insert("EPS0", &eps0);
    synt.parse(lex.tokens());
    std::cout << synt.evalTree();
    */
}

void Agros2DGenerator::run()
{
    // create directory
    QDir root(QApplication::applicationDirPath());
    root.mkpath(GENERATOR_PLUGINROOT);

    QMap<QString, QString> modules = availableModules();
    QMap<QString, QString> couplings = availableCouplings();

    ctemplate::TemplateDictionary output("project_output");

    foreach (QString moduleId, modules.keys())
    {
        Agros2DGeneratorModule generator(moduleId);

        generator.generatePluginProjectFile();
        generator.generatePluginInterfaceFiles();
        generator.generatePluginFilterFiles();
        generator.generatePluginWeakFormFiles();

        ctemplate::TemplateDictionary *field = output.AddSectionDictionary("SOURCE");
        field->SetValue("ID", moduleId.toStdString());
    }

    // generate plugins project file
    // expand template
    std::string text;
    ctemplate::ExpandTemplate(QString("%1/%2/plugins_pro.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // save to file
    writeStringContent(QString("%1/%2/plugins.pro").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT),
                       QString::fromStdString(text));

    exit(0);
}

// ********************************************************************************************************************

Agros2DGeneratorModule::Agros2DGeneratorModule(const QString &moduleId)
{
    QDir root(QApplication::applicationDirPath());
    root.mkpath(QString("%1/%2").arg(GENERATOR_PLUGINROOT).arg(moduleId));

    // read module
    module_xsd = XMLModule::module_((datadir().toStdString() + MODULEROOT.toStdString() + "/" + moduleId.toStdString() + ".xml").c_str());
    m_module = module_xsd.get();

    QDir().mkdir(GENERATOR_PLUGINROOT + "/" + moduleId);

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
}

void Agros2DGeneratorModule::generatePluginProjectFile()
{
    qDebug() << tr("%1: generating plugin project file.").arg(QString::fromStdString(m_module->general().id()));

    QString id = QString::fromStdString(m_module->general().id());
    QString header = id.toLower();

    ctemplate::TemplateDictionary output("output");
    output.SetValue("ID", id.toStdString());

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

void Agros2DGeneratorModule::generatePluginInterfaceFiles()
{
    qDebug() << tr("%1: generating plugin interface file.").arg(QString::fromStdString(m_module->general().id()));

    QString id = QString::fromStdString(m_module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", m_module->general().id());
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
    foreach(XMLModule::weakform_volume weakform, m_module->volume().weakforms_volume().weakform_volume())
    {
        generateVolumeMatrixForm(weakform, output);
        generateVolumeVectorForm(weakform, output);
    }

    foreach(XMLModule::weakform_surface weakform, m_module->surface().weakforms_surface().weakform_surface())
    {
        foreach(XMLModule::boundary boundary, weakform.boundary())
        {
            generateSurfaceMatrixForm(boundary, output, weakform);
            generateSurfaceVectorForm(boundary, output, weakform);
            generateExactSolution(boundary, output, weakform);
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

void Agros2DGeneratorModule::generatePluginWeakFormFiles()
{
    generatePluginWeakFormSourceFiles();
    generatePluginWeakFormHeaderFiles();
}

void Agros2DGeneratorModule::generatePluginWeakFormSourceFiles()
{
    qDebug() << tr("%1: generating plugin weakform source file.").arg(QString::fromStdString(m_module->general().id()));

    QString id = QString::fromStdString(m_module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", m_module->general().id());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    foreach(XMLModule::weakform_volume weakform, m_module->volume().weakforms_volume().weakform_volume())
    {
        generateVolumeMatrixForm(weakform, output);
        generateVolumeVectorForm(weakform, output);
    }

    foreach(XMLModule::weakform_surface weakform, m_module->surface().weakforms_surface().weakform_surface())
    {
        foreach(XMLModule::boundary boundary, weakform.boundary())
        {
            generateSurfaceMatrixForm(boundary, output, weakform);
            generateSurfaceVectorForm(boundary, output, weakform);
            generateExactSolution(boundary, output, weakform);
        }
    }

    std::string text;
    ctemplate::ExpandTemplate(QString("%1/%2/weakform_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
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

    foreach(XMLModule::weakform_volume weakform, m_module->volume().weakforms_volume().weakform_volume())
    {
        generateVolumeMatrixForm(weakform, output);
        generateVolumeVectorForm(weakform, output);
    }

    foreach(XMLModule::weakform_surface weakform, m_module->surface().weakforms_surface().weakform_surface())
    {
        foreach(XMLModule::boundary boundary, weakform.boundary())
        {
            generateSurfaceMatrixForm(boundary, output, weakform);
            generateSurfaceVectorForm(boundary, output, weakform);
            generateExactSolution(boundary, output, weakform);
        }
    }

    // header - expand template
    std::string text;
    ctemplate::ExpandTemplate(QString("%1/%2/weakform_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
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
    ctemplate::ExpandTemplate(QString("%1/%2/filter_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    foreach (XMLModule::localvariable lv, m_module->postprocessor().localvariables().localvariable())
    {
        foreach (XMLModule::expression expr, lv.expression())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                if (coordinateType == CoordinateType_Planar)
                {
                    if (lv.type() == "scalar")
                        createPostprocessorExpression(output, QString::fromStdString(lv.id()),
                                                      analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                                      coordinateType,
                                                      PhysicFieldVariableComp_Scalar,
                                                      QString::fromStdString(expr.planar().get()));
                    if (lv.type() == "vector")
                    {
                        createPostprocessorExpression(output, QString::fromStdString(lv.id()),
                                                      analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                                      coordinateType,
                                                      PhysicFieldVariableComp_X,
                                                      QString::fromStdString(expr.planar_x().get()));

                        createPostprocessorExpression(output, QString::fromStdString(lv.id()),
                                                      analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                                      coordinateType,
                                                      PhysicFieldVariableComp_Y,
                                                      QString::fromStdString(expr.planar_y().get()));

                        createPostprocessorExpression(output, QString::fromStdString(lv.id()),
                                                      analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                                      coordinateType,
                                                      PhysicFieldVariableComp_Magnitude,
                                                      QString("sqrt(pow(%1, 2) + pow(%2, 2))").arg(QString::fromStdString(expr.planar_x().get())).arg(QString::fromStdString(expr.planar_y().get())));

                    }
                }
                else
                {
                    if (lv.type() == "scalar")
                        createPostprocessorExpression(output, QString::fromStdString(lv.id()),
                                                      analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                                      coordinateType,
                                                      PhysicFieldVariableComp_Scalar,
                                                      QString::fromStdString(expr.axi().get()));
                    if (lv.type() == "vector")
                    {
                        createPostprocessorExpression(output, QString::fromStdString(lv.id()),
                                                      analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                                      coordinateType,
                                                      PhysicFieldVariableComp_X,
                                                      QString::fromStdString(expr.axi_r().get()));

                        createPostprocessorExpression(output, QString::fromStdString(lv.id()),
                                                      analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                                      coordinateType,
                                                      PhysicFieldVariableComp_Y,
                                                      QString::fromStdString(expr.axi_z().get()));

                        createPostprocessorExpression(output, QString::fromStdString(lv.id()),
                                                      analysisTypeFromStringKey(QString::fromStdString(expr.analysistype())),
                                                      coordinateType,
                                                      PhysicFieldVariableComp_Magnitude,
                                                      QString("sqrt(pow(%1, 2) + pow(%2, 2))").arg(QString::fromStdString(expr.axi_r().get())).arg(QString::fromStdString(expr.axi_z().get())));
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

void Agros2DGeneratorModule::createPostprocessorExpression(ctemplate::TemplateDictionary &output,
                                                           const QString &variable,
                                                           AnalysisType analysisType,
                                                           CoordinateType coordinateType,
                                                           PhysicFieldVariableComp physicFieldVariableComp,
                                                           const QString &expr)
{
    if (!expr.isEmpty())
    {
        ctemplate::TemplateDictionary *expression = output.AddSectionDictionary("VARIABLE_SOURCE");

        expression->SetValue("VARIABLE", variable.toStdString());
        expression->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisType).toStdString());
        expression->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
        expression->SetValue("PHYSICFIELDVARIABLECOMP_TYPE", Agros2DGenerator::physicFieldVariableCompStringEnum(physicFieldVariableComp).toStdString());
        expression->SetValue("EXPRESSION", parsePostprocessorExpression(analysisType, coordinateType, expr).toStdString());
    }
}

QString Agros2DGeneratorModule::parsePostprocessorExpression(AnalysisType analysisType, CoordinateType coordinateType, const QString &expr)
{
    int numOfSol = Agros2DGenerator::numberOfSolutions(m_module->general().analyses(), analysisType);

    LexicalAnalyser lex;

    // coordinates
    if (coordinateType == CoordinateType_Planar)
    {
        lex.addVariable(QString("x"));
        lex.addVariable(QString("y"));
    }
    else
    {
        lex.addVariable(QString("r"));
        lex.addVariable(QString("z"));
    }


    // functions
    for (int i = 1; i < numOfSol + 1; i++)
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
    }

    // constants
    lex.addVariable("PI");
    lex.addVariable("f");
    foreach (XMLModule::constant cnst, m_module->constants().constant())
        lex.addVariable(QString::fromStdString(cnst.id()));

    // variables
    foreach(XMLModule::quantity quantity, m_module->volume().quantity())
    {
        if (quantity.shortname().present())
        {
            lex.addVariable(QString::fromStdString(quantity.shortname().get()));
            lex.addVariable(QString::fromStdString("d" + quantity.shortname().get()));
        }
    }

    foreach(XMLModule::quantity quantity, m_module->surface().quantity())
    {
        if (quantity.shortname().present())
        {
            lex.addVariable(QString::fromStdString(quantity.shortname().get()));
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
                if (repl == "x") { exprCpp += "x[i]"; isReplaced = true; }
                if (repl == "y") { exprCpp += "y[i]"; isReplaced = true; }
            }
            else
            {
                if (repl == "r") { exprCpp += "x[i]"; isReplaced = true; }
                if (repl == "z") { exprCpp += "y[i]"; isReplaced = true; }
            }

            // constants
            if (repl == "PI") { exprCpp += "M_PI"; isReplaced = true; }
            if (repl == "f") { exprCpp += "Util::problem()->config()->frequency()"; isReplaced = true; }
            foreach (XMLModule::constant cnst, m_module->constants().constant())
                if (repl == QString::fromStdString(cnst.id())) { exprCpp += QString::number(cnst.value()); isReplaced = true; }

            // functions
            for (int i = 1; i < numOfSol + 1; i++)
            {
                if (repl == QString("value%1").arg(i)) { exprCpp += QString("value[%1][i]").arg(i-1); isReplaced = true; }
                if (coordinateType == CoordinateType_Planar)
                {
                    if (repl == QString("dx%1").arg(i)) { exprCpp += QString("dudx[%1][i]").arg(i-1); isReplaced = true; }
                    if (repl == QString("dy%1").arg(i)) { exprCpp += QString("dudy[%1][i]").arg(i-1); isReplaced = true; }
                }
                else
                {
                    if (repl == QString("dr%1").arg(i)) { exprCpp += QString("dudx[%1][i]").arg(i-1); isReplaced = true; }
                    if (repl == QString("dz%1").arg(i)) { exprCpp += QString("dudy[%1][i]").arg(i-1); isReplaced = true; }
                }
            }

            // variables
            foreach (XMLModule::quantity quantity, m_module->volume().quantity())
            {
                if (quantity.shortname().present())
                    if (repl == QString::fromStdString(quantity.shortname().get()))
                    {
                        if (!quantity.dependence().present())
                            // linear material
                            exprCpp += QString("material->value(\"%1\").number()").arg(QString::fromStdString(quantity.id()));
                        else
                            // nonlinear material
                            exprCpp += QString("material->value(%1).value(%2)").
                                    arg(QString::fromStdString(quantity.id())).
                                    arg(parsePostprocessorExpression(analysisType, coordinateType, QString::fromStdString(quantity.dependence().get())));

                        isReplaced = true;
                    }
            }

            // operators and other
            if (!isReplaced)
                exprCpp += repl;
        }

        return exprCpp;
    }
    catch (ParserException e)
    {
        qDebug() << e.toString() << "in module: " << QString::fromStdString(m_module->general().id());

        return "";
    }
}
//-----------------------------------------------------------------------------------------
QString Agros2DGeneratorModule::parseWeakFormExpression(AnalysisType analysisType, CoordinateType coordinateType, const QString &expr)
{
    int numOfSol = Agros2DGenerator::numberOfSolutions(m_module->general().analyses(), analysisType);

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
        lex.addVariable(QString("x"));
        lex.addVariable(QString("y"));
    }
    else
    {
        lex.addVariable("udr");
        lex.addVariable("vdr");
        lex.addVariable("udz");
        lex.addVariable("vdz");
        lex.addVariable("updr");
        lex.addVariable("updz");
        lex.addVariable(QString("r"));
        lex.addVariable(QString("z"));
    }

    if (analysisType == AnalysisType_Transient)
    {
        lex.addVariable("deltat");
    }

    // functions
    for (int i = 1; i < numOfSol + 1; i++)
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
    }

    // constants
    lex.addVariable("PI");
    lex.addVariable("f");
    foreach (XMLModule::constant cnst, m_module->constants().constant())
        lex.addVariable(QString::fromStdString(cnst.id()));

    // variables
    foreach (XMLModule::quantity quantity, m_module->volume().quantity())
    {
        if (quantity.shortname().present())
        {
            lex.addVariable(QString::fromStdString(quantity.shortname().get()));
            lex.addVariable(QString::fromStdString("d" + quantity.shortname().get()));
        }
    }

    foreach (XMLModule::quantity quantity, m_module->surface().quantity())
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
            if (repl == "f") { exprCpp += "Util::problem()->config()->frequency()"; isReplaced = true; }
            foreach (XMLModule::constant cnst, m_module->constants().constant())
                if (repl == QString::fromStdString(cnst.id())) { exprCpp += QString::number(cnst.value()); isReplaced = true; }

            // functions

            if (repl == QString("uval")) { exprCpp += QString("u->val[i]"); isReplaced = true; }
            if (repl == QString("vval")) { exprCpp += QString("v->val[i]"); isReplaced = true; }
            if (repl == QString("upval")) { exprCpp += QString("u_ext[this->j]->val[i]"); isReplaced = true; }
            if (repl == QString("uptval")) { exprCpp += QString("ext->fn[this->i]->val[i]"); isReplaced = true; }
            if (repl == QString("deltat")) { exprCpp += QString("Util::problem()->config()->timeStep().number()"); isReplaced = true; }

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
                if (repl == QString("value%1").arg(i)) { exprCpp += QString("u_ext[%1]->val[i]").arg(i-1); isReplaced = true; }
                if (coordinateType == CoordinateType_Planar)
                {
                    if (repl == QString("dx%1").arg(i)) { exprCpp += QString("u_ext[%1]->dx[i]").arg(i-1); isReplaced = true; }
                    if (repl == QString("dy%1").arg(i)) { exprCpp += QString("u_ext[%1]->dy[i]").arg(i-1); isReplaced = true; }
                }
                else
                {
                    if (repl == QString("dr%1").arg(i)) { exprCpp += QString("u_ext[%1]->dx[i]").arg(i-1); isReplaced = true; }
                    if (repl == QString("dz%1").arg(i)) { exprCpp += QString("u_ext[%1]->dx[i]").arg(i-1); isReplaced = true; }
                }
            }

            // variables
            foreach (XMLModule::quantity quantity, m_module->volume().quantity())
            {
                if (quantity.shortname().present())
                    if ((repl == QString::fromStdString(quantity.shortname().get())) || repl == QString::fromStdString("d"+quantity.shortname().get()))
                    {
                        if (!quantity.dependence().present())
                            // linear material
                            exprCpp += QString("%1.number()").arg(QString::fromStdString(quantity.shortname().get()));
                        else
                            // nonlinear material
                            exprCpp += QString("%1.value()").
                                    arg(QString::fromStdString(quantity.shortname().get()));
                        isReplaced = true;
                    }
            }

            if (!isReplaced)
            {
                foreach (XMLModule::quantity quantity, m_module->surface().quantity())
                {
                    if (quantity.shortname().present())
                        if (repl == QString::fromStdString(quantity.shortname().get()))
                        {
                            if (!quantity.dependence().present())
                                // linear material
                                exprCpp += QString("%1.number()").arg(QString::fromStdString(quantity.shortname().get()));
                            else
                                // nonlinear material
                                exprCpp += QString("%1.value()").
                                        arg(QString::fromStdString(quantity.shortname().get()));
                            isReplaced = true;
                        }
                }
            }

            // operators and other
            if (!isReplaced)
                exprCpp += repl;
        }
        exprCpp = lex.replaceOperatorByFunction(exprCpp);
        return exprCpp;
    }
    catch (ParserException e)
    {
        qDebug() << e.toString() << "in module: " << QString::fromStdString(m_module->general().id());

        return "";
    }
}

QString Agros2DGeneratorModule::weakformExpression(CoordinateType coordinateType, LinearityType linearityType, XMLModule::matrix_form matrix_form)
{
    QString expression;

    if ((linearityType == LinearityType_Linear) && (coordinateType == CoordinateType_Planar))
    {
        expression = matrix_form.planar_linear().c_str();
    }

    if ((linearityType == LinearityType_Newton) && (coordinateType == CoordinateType_Planar))
    {
        expression = matrix_form.planar_newton().c_str();
    }

    if ((linearityType == LinearityType_Linear) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = matrix_form.axi_linear().c_str();
    }
    if ((linearityType == LinearityType_Newton) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = matrix_form.axi_newton().c_str();
    }

    return expression;
}

QString Agros2DGeneratorModule::weakformExpression(CoordinateType coordinateType, LinearityType linearityType, XMLModule::vector_form vector_form)
{
    QString expression;

    if ((linearityType == LinearityType_Linear) && (coordinateType == CoordinateType_Planar))
    {
        expression = vector_form.planar_linear().c_str();
    }

    if ((linearityType == LinearityType_Newton) && (coordinateType == CoordinateType_Planar))
    {
        expression = vector_form.planar_newton().c_str();
    }

    if ((linearityType == LinearityType_Linear) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = vector_form.axi_linear().c_str();
    }
    if ((linearityType == LinearityType_Newton) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = vector_form.axi_newton().c_str();
    }

    return expression;
}

QString Agros2DGeneratorModule::weakformExpression(CoordinateType coordinateType, LinearityType linearityType, XMLModule::essential_form essential_form)
{
    QString expression;

    if ((linearityType == LinearityType_Linear) && (coordinateType == CoordinateType_Planar))
    {
        expression = essential_form.planar_linear().c_str();
    }

    if ((linearityType == LinearityType_Newton) && (coordinateType == CoordinateType_Planar))
    {
        expression = essential_form.planar_newton().c_str();
    }

    if ((linearityType == LinearityType_Linear) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = essential_form.axi_linear().c_str();
    }
    if ((linearityType == LinearityType_Newton) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = essential_form.axi_newton().c_str();
    }

    return expression;
}

void Agros2DGeneratorModule::generateVolumeMatrixForm(XMLModule::weakform_volume weakform, ctemplate::TemplateDictionary &output)
{
    foreach(XMLModule::matrix_form matrix_form, weakform.matrix_form())
    {
        foreach(LinearityType linearityType, Agros2DGenerator::linearityTypeList())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                QString expression = weakformExpression(coordinateType, linearityType, matrix_form);

                if(expression != "")
                {
                    QString functionName = QString("%1_%2_%3_%4_%5_%6_%7").
                            arg("volume_matrix").
                            arg(QString::fromStdString(m_module->general().id())).
                            arg(QString::fromStdString(weakform.analysistype())).
                            arg(coordinateTypeToStringKey(coordinateType)).
                            arg(linearityTypeToStringKey(linearityType)).
                            arg(QString::number(matrix_form.i())).
                            arg(QString::number(matrix_form.j()));

                    ctemplate::TemplateDictionary *field = 0;
                    field = output.AddSectionDictionary("MATRIX_VOL_SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                    field->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
                    field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
                    field->SetValue("ANALYSIS_TYPE", weakform.analysistype());
                    field->SetValue("ROW_INDEX", QString::number(matrix_form.i()).toStdString());
                    field->SetValue("COLUMN_INDEX", QString::number(matrix_form.j()).toStdString());
                    field->SetValue("MODULE_ID", m_module->general().id());
                    QString exprCpp;
                    exprCpp = parseWeakFormExpression(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), coordinateType, expression);
                    field->SetValue("EXPRESSION", exprCpp.toStdString());

                    foreach(XMLModule::quantity quantity, weakform.quantity())
                    {
                        ctemplate::TemplateDictionary *subField = 0;
                        subField = field->AddSectionDictionary("VARIABLE_SOURCE");
                        subField->SetValue("VARIABLE", quantity.id().c_str());
                        subField->SetValue("VARIABLE_SHORT", m_volumeVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());
                    }

                    field = output.AddSectionDictionary("SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                }
            }
        }
    }

}

void Agros2DGeneratorModule::generateVolumeVectorForm(XMLModule::weakform_volume weakform, ctemplate::TemplateDictionary &output)
{
    foreach(XMLModule::vector_form vector_form, weakform.vector_form())
    {
        foreach(LinearityType linearityType, Agros2DGenerator::linearityTypeList())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                QString expression = weakformExpression(coordinateType, linearityType, vector_form);
                if(expression != "")
                {
                    QString functionName = QString("%1_%2_%3_%4_%5_%6_%7").
                            arg("volume_vector").
                            arg(QString::fromStdString(m_module->general().id())).
                            arg(QString::fromStdString(weakform.analysistype())).
                            arg(coordinateTypeToStringKey(coordinateType)).
                            arg(linearityTypeToStringKey(linearityType)).
                            arg(QString::number(vector_form.i())).
                            arg(QString::number(vector_form.j()));


                    ctemplate::TemplateDictionary *field;
                    field = output.AddSectionDictionary("VECTOR_VOL_SOURCE"); // parameter output
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                    field->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
                    field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
                    field->SetValue("ANALYSIS_TYPE", weakform.analysistype());
                    field->SetValue("ROW_INDEX", QString::number(vector_form.i()).toStdString());
                    field->SetValue("COLUMN_INDEX", QString::number(vector_form.j()).toStdString());
                    field->SetValue("MODULE_ID", m_module->general().id());
                    QString exprCpp;
                    exprCpp = parseWeakFormExpression(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), coordinateType, expression);
                    field->SetValue("EXPRESSION", exprCpp.toStdString());

                    foreach(XMLModule::quantity quantity, weakform.quantity())
                    {
                        ctemplate::TemplateDictionary *subField = 0;
                        subField = field->AddSectionDictionary("VARIABLE_SOURCE");
                        subField->SetValue("VARIABLE", quantity.id().c_str());
                        subField->SetValue("VARIABLE_SHORT", m_volumeVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());
                    }

                    field = output.AddSectionDictionary("SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                }
            }
        }
    }
}

void Agros2DGeneratorModule::generateSurfaceMatrixForm(XMLModule::boundary boundary, ctemplate::TemplateDictionary &output, XMLModule::weakform_surface weakform)
{
    foreach(XMLModule::matrix_form matrix_form, boundary.matrix_form())
    {
        foreach(LinearityType linearityType, Agros2DGenerator::linearityTypeList())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                QString expression = weakformExpression(coordinateType, linearityType, matrix_form);
                if(expression != "")
                {
                    QString functionName = QString("%1_%2_%3_%4_%5_%6_%7_%8").
                            arg("surface_matrix").
                            arg(QString::fromStdString(m_module->general().id())).
                            arg(QString::fromStdString(weakform.analysistype())).
                            arg(coordinateTypeToStringKey(coordinateType)).
                            arg(linearityTypeToStringKey(linearityType)).
                            arg( QString::fromStdString(boundary.id())).
                            arg(QString::number(matrix_form.i())).
                            arg(QString::number(matrix_form.j()));

                    ctemplate::TemplateDictionary *field;
                    field = output.AddSectionDictionary("MATRIX_SURF_SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                    field->SetValue("BOUNDARY_TYPE", boundary.id());
                    field->SetValue("COORDINATE_TYPE",  Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
                    field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
                    field->SetValue("ANALYSIS_TYPE", weakform.analysistype());
                    field->SetValue("ROW_INDEX", QString::number(matrix_form.i()).toStdString());
                    field->SetValue("COLUMN_INDEX", QString::number(matrix_form.j()).toStdString());
                    field->SetValue("MODULE_ID", m_module->general().id());
                    QString exprCpp;
                    exprCpp = parseWeakFormExpression(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), coordinateType, expression);
                    field->SetValue("EXPRESSION", exprCpp.toStdString());

                    foreach(XMLModule::quantity quantity, boundary.quantity())
                    {
                        ctemplate::TemplateDictionary *subField = 0;
                        subField = field->AddSectionDictionary("VARIABLE_SOURCE");
                        subField->SetValue("VARIABLE", quantity.id().c_str());
                        subField->SetValue("VARIABLE_SHORT", m_surfaceVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());
                    }

                    field = output.AddSectionDictionary("SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                }
            }
        }
    }
}

void Agros2DGeneratorModule::generateSurfaceVectorForm(XMLModule::boundary boundary, ctemplate::TemplateDictionary &output, XMLModule::weakform_surface weakform)
{
    foreach(XMLModule::vector_form vector_form, boundary.vector_form())
    {
        foreach(LinearityType linearityType, Agros2DGenerator::linearityTypeList())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                QString expression = weakformExpression(coordinateType, linearityType, vector_form);
                if(expression != "")
                {
                    QString functionName = QString("%1_%2_%3_%4_%5_%6_%7_%8").
                            arg("surface_vector").
                            arg(QString::fromStdString(m_module->general().id())).
                            arg(QString::fromStdString(weakform.analysistype())).
                            arg(coordinateTypeToStringKey(coordinateType)).
                            arg(linearityTypeToStringKey(linearityType)).
                            arg( QString::fromStdString(boundary.id())).
                            arg(QString::number(vector_form.i())).
                            arg(QString::number(vector_form.j()));

                    ctemplate::TemplateDictionary *field;
                    field = output.AddSectionDictionary("VECTOR_SURF_SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                    field->SetValue("BOUNDARY_TYPE", boundary.id());
                    field->SetValue("COORDINATE_TYPE",  Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
                    field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
                    field->SetValue("ANALYSIS_TYPE", weakform.analysistype());
                    field->SetValue("ROW_INDEX", QString::number(vector_form.i()).toStdString());
                    field->SetValue("COLUMN_INDEX", QString::number(vector_form.j()).toStdString());
                    field->SetValue("MODULE_ID", m_module->general().id());
                    QString exprCpp;
                    exprCpp = parseWeakFormExpression(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), coordinateType, expression);
                    field->SetValue("EXPRESSION", exprCpp.toStdString());

                    foreach(XMLModule::quantity quantity, boundary.quantity())
                    {
                        ctemplate::TemplateDictionary *subField = 0;
                        subField = field->AddSectionDictionary("VARIABLE_SOURCE");
                        subField->SetValue("VARIABLE", quantity.id().c_str());
                        subField->SetValue("VARIABLE_SHORT", m_surfaceVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());
                    }

                    field = output.AddSectionDictionary("SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                }
            }
        }
    }
}

void Agros2DGeneratorModule::generateExactSolution(XMLModule::boundary boundary, ctemplate::TemplateDictionary &output, XMLModule::weakform_surface weakform)
{
    foreach(XMLModule::essential_form essential_form, boundary.essential_form())
    {
        foreach(LinearityType linearityType, Agros2DGenerator::linearityTypeList())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                QString expression = weakformExpression(coordinateType, linearityType, essential_form);
                if(expression != "")
                {
                    QString functionName = QString("%1_%2_%3_%4_%5_%6_%7").
                            arg("exact_source").
                            arg(QString::fromStdString(m_module->general().id())).
                            arg(QString::fromStdString(weakform.analysistype())).
                            arg(coordinateTypeToStringKey(coordinateType)).
                            arg(linearityTypeToStringKey(linearityType)).
                            arg( QString::fromStdString(boundary.id())).
                            arg(QString::number(essential_form.i()));

                    ctemplate::TemplateDictionary *field;
                    field = output.AddSectionDictionary("EXACT_SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                    field->SetValue("BOUNDARY_TYPE", boundary.id());
                    field->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
                    field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
                    field->SetValue("ANALYSIS_TYPE", weakform.analysistype());
                    field->SetValue("ROW_INDEX", QString::number(essential_form.i()).toStdString());
                    field->SetValue("COLUMN_INDEX", "0");
                    field->SetValue("MODULE_ID", m_module->general().id());

                    QString exprCpp;
                    exprCpp = parseWeakFormExpression(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), coordinateType, expression);
                    field->SetValue("EXPRESSION", exprCpp.toStdString());

                    foreach(XMLModule::quantity quantity, boundary.quantity())
                    {
                        ctemplate::TemplateDictionary *subField = 0;
                        subField = field->AddSectionDictionary("VARIABLE_SOURCE");
                        subField->SetValue("VARIABLE", quantity.id().c_str());
                        subField->SetValue("VARIABLE_SHORT", m_surfaceVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());
                    }

                    field = output.AddSectionDictionary("SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                }
            }
        }
    }
}
