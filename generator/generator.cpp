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

QList<CouplingType> Agros2DGenerator::couplingFormTypeList()
{
    QList<CouplingType> list;
    list << CouplingType_Weak << CouplingType_Hard << CouplingType_None << CouplingType_Undefined;
    return list;
}

QString Agros2DGenerator::couplingTypeStringEnum(CouplingType couplingType)
{
    switch (couplingType)
    {
    case WeakForm_MatVol:
        return("CouplingType_Weak");
        break;
    case WeakForm_MatSurf:
        return("CouplingType_Hard");
        break;
    case WeakForm_VecVol:
        return("WeakForm_VecVol");
        break;
    case WeakForm_VecSurf:
        return("CouplingType_None");
        break;
    case WeakForm_ExactSol:
        return("CouplingType_Undefined");
        break;
    default:
        assert(0);
    }
}

QString Agros2DGenerator::couplingTypeToString(QString couplingType)
{
    if(couplingType == "hard")
        return ("CouplingType_Hard");
    if(couplingType == "weak")
        return ("CouplingType_Weak");
    if(couplingType == "none")
        return ("CouplingType_None");
    if(couplingType == "undefined")
        return ("CouplingType_Undefined");
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
}

void Agros2DGenerator::run()
{
    // create directory
    QDir root(QApplication::applicationDirPath());
    root.mkpath(GENERATOR_PLUGINROOT);

    ctemplate::TemplateDictionary output("project_output");
    QMap<QString, QString> modules = availableModules();
    QMap<QString, QString> couplings = availableCouplings();

    foreach (QString moduleId, modules.keys())
    {
        Agros2DGeneratorModule generator(moduleId);
        generator.generatePluginProjectFile();
        generator.generatePluginInterfaceFiles();
        generator.generatePluginFilterFiles();
        generator.generatePluginLocalPointFiles();
        generator.generatePluginSurfaceIntegralFiles();
        generator.generatePluginVolumeIntegralFiles();
        generator.generatePluginWeakFormFiles();

        ctemplate::TemplateDictionary *field = output.AddSectionDictionary("SOURCE");
        field->SetValue("ID", moduleId.toStdString());
    }

    foreach (QString couplingId, couplings.keys())
    {
        Agros2DGeneratorCoupling generator(couplingId);
        generator.generatePluginProjectFile();
        generator.generatePluginInterfaceFiles();
        generator.generatePluginWeakFormFiles();

        ctemplate::TemplateDictionary *field = output.AddSectionDictionary("SOURCE");
        field->SetValue("ID", couplingId.replace("-", "").toStdString());
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
                                               QString("sqrt(pow(%1, 2) + pow(%2, 2))").arg(QString::fromStdString(expr.planar_x().get())).arg(QString::fromStdString(expr.planar_y().get())));

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

void Agros2DGeneratorModule::generatePluginLocalPointFiles()
{
    qDebug() << tr("%1: generating plugin local point file.").arg(QString::fromStdString(m_module->general().id()));

    QString id = QString::fromStdString(m_module->general().id());

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", id.toStdString());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(QString("%1/%2/localvalue_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

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

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_localvalue.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // source - expand template
    text.clear();
    ctemplate::ExpandTemplate(QString("%1/%2/localvalue_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
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
    ctemplate::ExpandTemplate(QString("%1/%2/surfaceintegral_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

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
    ctemplate::ExpandTemplate(QString("%1/%2/surfaceintegral_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
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
    ctemplate::ExpandTemplate(QString("%1/%2/volumeintegral_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

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
    ctemplate::ExpandTemplate(QString("%1/%2/volumeintegral_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
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

        expression->SetValue("VARIABLE", variable.toStdString());
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

QString Agros2DGeneratorModule::parsePostprocessorExpression(AnalysisType analysisType, CoordinateType coordinateType, const QString &expr)
{
    int numOfSol = Agros2DGenerator::numberOfSolutions(m_module->general().analyses(), analysisType);

    LexicalAnalyser lex;

    // coordinates
    if (coordinateType == CoordinateType_Planar)
    {
        lex.addVariable(QString("x"));
        lex.addVariable(QString("y"));
        lex.addVariable(QString("tanx"));
        lex.addVariable(QString("tany"));
    }
    else
    {
        lex.addVariable(QString("r"));
        lex.addVariable(QString("z"));
        lex.addVariable(QString("tanr"));
        lex.addVariable(QString("tanz"));
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
                if (repl == "tanx") { exprCpp += "tan[i][0]"; isReplaced = true; }
                if (repl == "tany") { exprCpp += "tan[i][1]"; isReplaced = true; }
            }
            else
            {
                if (repl == "r") { exprCpp += "x[i]"; isReplaced = true; }
                if (repl == "z") { exprCpp += "y[i]"; isReplaced = true; }
                if (repl == "tanr") { exprCpp += "tan[i][0]"; isReplaced = true; }
                if (repl == "tanz") { exprCpp += "tan[i][1]"; isReplaced = true; }
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
                {
                    if (repl == QString::fromStdString(quantity.shortname().get()))
                    {
                        QString nonlinearExpr = nonlinearExpression(QString::fromStdString(quantity.id()), analysisType, coordinateType);
                        if (nonlinearExpr.isEmpty())
                            // linear material
                            exprCpp += QString("material->value(\"%1\").number()").arg(QString::fromStdString(quantity.id()));
                        else
                            // nonlinear material
                            exprCpp += QString("material->value(\"%1\").value(%2)").
                                    arg(QString::fromStdString(quantity.id())).
                                    arg(parsePostprocessorExpression(analysisType, coordinateType, nonlinearExpr));

                        isReplaced = true;
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
        qDebug() << e.toString() << "in module: " << QString::fromStdString(m_module->general().id());

        return "";
    }
}
//-----------------------------------------------------------------------------------------
QString Agros2DGeneratorModule::parseWeakFormExpression(AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType, const QString &expr)
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
                    if ((repl == QString::fromStdString(quantity.shortname().get())) || repl == QString::fromStdString("d" + quantity.shortname().get()))
                    {
                        QString nonlinearExpr = nonlinearExpression(QString::fromStdString(quantity.id()), analysisType, coordinateType);
                        if (linearityType == LinearityType_Linear || nonlinearExpr.isEmpty())
                            // linear material
                            exprCpp += QString("%1.number()").arg(QString::fromStdString(quantity.shortname().get()));
                        else
                        {
                            // nonlinear material
                            if (repl == QString::fromStdString(quantity.shortname().get()))
                                exprCpp += QString("%1.value(%2)").
                                        arg(QString::fromStdString(quantity.shortname().get())).
                                        arg(parseWeakFormExpression(analysisType, coordinateType, linearityType, nonlinearExpr));
                            if (repl == QString::fromStdString("d" + quantity.shortname().get()))
                                exprCpp += QString("%1.derivative(%2)").
                                        arg(QString::fromStdString(quantity.shortname().get())).
                                        arg(parseWeakFormExpression(analysisType, coordinateType, linearityType, nonlinearExpr));
                        }

                        isReplaced = true;
                    }
            }

            // FIXME: David, what is the reason of this block?
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

        // TODO: move from lex
        exprCpp = lex.replaceOperatorByFunction(exprCpp);
        return exprCpp;
    }
    catch (ParserException e)
    {
        qDebug() << e.toString() << "in module: " << QString::fromStdString(m_module->general().id());

        return "";
    }
}

template <typename TForm>
QString Agros2DGeneratorModule::weakformExpression(CoordinateType coordinateType, LinearityType linearityType, TForm tForm)
{
    QString expression;

    if ((linearityType == LinearityType_Linear || linearityType == LinearityType_Picard) && (coordinateType == CoordinateType_Planar))
    {
        expression = tForm.planar_linear().c_str();
    }

    if ((linearityType == LinearityType_Newton || linearityType == LinearityType_Picard) && (coordinateType == CoordinateType_Planar))
    {
        expression = tForm.planar_newton().c_str();
    }

    if ((linearityType == LinearityType_Linear || linearityType == LinearityType_Picard) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = tForm.axi_linear().c_str();
    }

    if ((linearityType == LinearityType_Newton || linearityType == LinearityType_Picard) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = tForm.axi_newton().c_str();
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
                    exprCpp = parseWeakFormExpression(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())),
                                                      coordinateType, linearityType, expression);
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
                    exprCpp = parseWeakFormExpression(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())),
                                                      coordinateType, linearityType, expression);
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
                    exprCpp = parseWeakFormExpression(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())),
                                                      coordinateType, linearityType, expression);
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
                    exprCpp = parseWeakFormExpression(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())),
                                                      coordinateType, linearityType, expression);
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
                    exprCpp = parseWeakFormExpression(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())),
                                                      coordinateType, linearityType, expression);
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

/*********************************************************************************************************************************************/
Agros2DGeneratorCoupling::Agros2DGeneratorCoupling(const QString &couplingId)
{
    QString iD = couplingId;
    iD = iD.replace("-", "");
    QDir root(QApplication::applicationDirPath());
    root.mkpath(QString(GENERATOR_PLUGINROOT));
    qDebug() << QString::fromStdString(datadir().toStdString() + COUPLINGROOT.toStdString() + "/" + couplingId.toStdString() + ".xml");
    coupling_xsd = XMLCoupling::coupling_((datadir().toStdString() + COUPLINGROOT.toStdString() + "/" + couplingId.toStdString() + ".xml").c_str());
    m_coupling = coupling_xsd.get();

    QString sourceModuleId = QString::fromStdString(m_coupling->general().modules().source().id().c_str());
    QString targetModuleId = QString::fromStdString(m_coupling->general().modules().target().id().c_str());

    m_source_module_xsd = XMLModule::module_((datadir().toStdString() + MODULEROOT.toStdString() + "/" + sourceModuleId.toStdString() + ".xml").c_str());
    m_sourceModule = m_source_module_xsd.get();

    m_target_module_xsd = XMLModule::module_((datadir().toStdString() + MODULEROOT.toStdString() + "/" + targetModuleId.toStdString() + ".xml").c_str());
    m_targetModule = m_target_module_xsd.get();

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
}

void Agros2DGeneratorCoupling::generatePluginProjectFile()
{    

    QString id = (QString::fromStdString(m_coupling->general().id().c_str())).replace("-","");
    qDebug() << tr("%1: generating plugin project file.").arg(id);

    ctemplate::TemplateDictionary output("output");
    output.SetValue("ID", id.toStdString());

    // expand template
    std::string text;
    ctemplate::ExpandTemplate(QString("%1/%2/coupling_pro.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // save to file
    writeStringContent(QString("%1/%2/%3/%3.pro").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorCoupling::generatePluginInterfaceFiles()
{
    QString id = QString::fromStdString(m_coupling->general().id()).replace("-", "");

    qDebug() << tr("%1: generating plugin interface file.").arg(id);

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", id.toStdString());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(QString("%1/%2/coupling_interface_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // source - expand template
    text.clear();
    foreach(XMLCoupling::weakform_volume weakform, m_coupling->volume().weakforms_volume().weakform_volume())
    {
        generateVolumeMatrixForm(weakform, output);
        generateVolumeVectorForm(weakform, output);
    }

    ctemplate::ExpandTemplate(QString("%1/%2/coupling_interface_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);
    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorCoupling::generatePluginWeakFormFiles()
{
    generatePluginWeakFormSourceFiles();
    generatePluginWeakFormHeaderFiles();
}

void Agros2DGeneratorCoupling::generatePluginWeakFormHeaderFiles()
{
    QString id = (QString::fromStdString(m_coupling->general().id().c_str())).replace("-","");
    qDebug() << tr("%1: generating plugin weakform header file.").arg(id);

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", id.toStdString());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    foreach(XMLCoupling::weakform_volume weakform, m_coupling->volume().weakforms_volume().weakform_volume())
    {
        generateVolumeMatrixForm(weakform, output);
        generateVolumeVectorForm(weakform, output);
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


void Agros2DGeneratorCoupling::generatePluginWeakFormSourceFiles()
{


    QString id = (QString::fromStdString(m_coupling->general().id().c_str())).replace("-","");
    qDebug() << tr("%1: generating plugin weakform source file.").arg(id);
    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", id.toStdString());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    foreach(XMLCoupling::weakform_volume weakform, m_coupling->volume().weakforms_volume().weakform_volume())
    {
        generateVolumeMatrixForm(weakform, output);
        generateVolumeVectorForm(weakform, output);
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
    int numOfSol = Agros2DGenerator::numberOfSolutions(m_sourceModule->general().analyses(), sourceAnalysisType) +
            Agros2DGenerator::numberOfSolutions(m_targetModule->general().analyses(), targetAnalysisType);

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

    if (sourceAnalysisType == AnalysisType_Transient)
    {
        lex.addVariable("deltat");
    }

    if (targetAnalysisType == AnalysisType_Transient)
    {
        lex.addVariable("deltat");
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
    foreach (XMLCoupling::constant cnst, m_coupling->constants().constant())
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
            if (repl == "f") { exprCpp += "Util::problem()->config()->frequency()"; isReplaced = true; }
            foreach (XMLCoupling::constant cnst, m_coupling->constants().constant())
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

            for (int i = 0; i < numOfSol + 1; i++)
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
                if (repl == QString("source%1").arg(i)) { exprCpp += QString("ext->fn[%1]->val[i]").arg(i-1); isReplaced = true; }
                if (repl == QString("source%1dx").arg(i)) { exprCpp += QString("ext->fn[%1]->dx[i]").arg(i-1); isReplaced = true; }
                if (repl == QString("source%1dy").arg(i)) { exprCpp += QString("ext->fn[%1]->dy[i]").arg(i-1); isReplaced = true; }
                if (repl == QString("source%1dr").arg(i)) { exprCpp += QString("ext->fn[%1]->dx[i]").arg(i-1); isReplaced = true; }
                if (repl == QString("source%1dz").arg(i)) { exprCpp += QString("ext->fn[%1]->dy[i]").arg(i-1); isReplaced = true; }
            }

            // variables
            foreach (XMLModule::quantity quantity, m_sourceModule->volume().quantity())
            {
                if (quantity.shortname().present())
                    if ((repl == QString::fromStdString(quantity.shortname().get())) || repl == QString::fromStdString("d" + quantity.shortname().get()))
                    {
                        QString nonlinearExpr = nonlinearExpression(QString::fromStdString(quantity.id()), sourceAnalysisType, coordinateType);
                        if (nonlinearExpr.isEmpty())
                            // linear material
                            exprCpp += QString("%1.number()").arg(QString::fromStdString(quantity.shortname().get()));
                        else
                        {
                            // nonlinear material
                            if (repl == QString::fromStdString(quantity.shortname().get()))
                                exprCpp += QString("%1.value(%2)").
                                        arg(QString::fromStdString(quantity.shortname().get())).
                                        arg(parseWeakFormExpression(sourceAnalysisType, targetAnalysisType, coordinateType, nonlinearExpr));
                            if (repl == QString::fromStdString("d" + quantity.shortname().get()))
                                exprCpp += QString("%1.derivative(%2)").
                                        arg(QString::fromStdString(quantity.shortname().get())).
                                        arg(parseWeakFormExpression(sourceAnalysisType, targetAnalysisType, coordinateType, nonlinearExpr));
                        }

                        isReplaced = true;
                    }
            }

            foreach (XMLModule::quantity quantity, m_targetModule->volume().quantity())
            {
                if (quantity.shortname().present())
                    if ((repl == QString::fromStdString(quantity.shortname().get())) || repl == QString::fromStdString("d" + quantity.shortname().get()))
                    {
                        QString nonlinearExpr = nonlinearExpression(QString::fromStdString(quantity.id()), sourceAnalysisType, coordinateType);
                        if (nonlinearExpr.isEmpty())
                            // linear material
                            exprCpp += QString("%1.number()").arg(QString::fromStdString(quantity.shortname().get()));
                        else
                        {
                            // nonlinear material
                            if (repl == QString::fromStdString(quantity.shortname().get()))
                                exprCpp += QString("%1.value(%2)").
                                        arg(QString::fromStdString(quantity.shortname().get())).
                                        arg(parseWeakFormExpression(sourceAnalysisType, targetAnalysisType, coordinateType, nonlinearExpr));
                            if (repl == QString::fromStdString("d" + quantity.shortname().get()))
                                exprCpp += QString("%1.derivative(%2)").
                                        arg(QString::fromStdString(quantity.shortname().get())).
                                        arg(parseWeakFormExpression(sourceAnalysisType, targetAnalysisType, coordinateType, nonlinearExpr));
                        }

                        isReplaced = true;
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
        qDebug() << e.toString() << "in coupling: " << QString::fromStdString(m_coupling->general().id());

        return "";
    }
}

template <typename TForm>
QString Agros2DGeneratorCoupling::weakformExpression(CoordinateType coordinateType, LinearityType linearityType, TForm tForm)
{
    QString expression;

    if ((linearityType == LinearityType_Linear || linearityType == LinearityType_Picard) && (coordinateType == CoordinateType_Planar))
    {
        expression = tForm.planar_linear().c_str();
    }

    if ((linearityType == LinearityType_Newton || linearityType == LinearityType_Picard) && (coordinateType == CoordinateType_Planar))
    {
        expression = tForm.planar_newton().c_str();
    }

    if ((linearityType == LinearityType_Linear || linearityType == LinearityType_Picard) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = tForm.axi_linear().c_str();
    }

    if ((linearityType == LinearityType_Newton || linearityType == LinearityType_Picard) && (coordinateType == CoordinateType_Axisymmetric))
    {
        expression = tForm.axi_newton().c_str();
    }

    return expression;
}

void Agros2DGeneratorCoupling::generateVolumeMatrixForm(XMLCoupling::weakform_volume weakform, ctemplate::TemplateDictionary &output)
{
    foreach(XMLCoupling::matrix_form matrix_form, weakform.matrix_form())
    {
        foreach(LinearityType linearityType, Agros2DGenerator::linearityTypeList())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                QString expression = weakformExpression(coordinateType, linearityType, matrix_form);

                if(expression != "")
                {
                    QString id = (QString::fromStdString(m_coupling->general().id().c_str())).replace("-","");
                    QString functionName = QString("%1_%2_%3_%4_%5_%6_%7_%8_%9").
                            arg("volume_matrix").
                            arg(id).
                            arg(QString::fromStdString(weakform.sourceanalysis())).
                            arg(QString::fromStdString(weakform.targetanalysis())).
                            arg(coordinateTypeToStringKey(coordinateType)).
                            arg(linearityTypeToStringKey(linearityType)).
                            arg(QString::number(matrix_form.i())).
                            arg(QString::number(matrix_form.j())).
                            arg(QString::fromStdString(weakform.couplingtype()));

                    ctemplate::TemplateDictionary *field = 0;
                    field = output.AddSectionDictionary("MATRIX_VOL_SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                    field->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
                    field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
                    field->SetValue("ANALYSIS_TYPE", weakform.sourceanalysis() + weakform.targetanalysis());
                    field->SetValue("ROW_INDEX", QString::number(matrix_form.i()).toStdString());
                    field->SetValue("COLUMN_INDEX", QString::number(matrix_form.j()).toStdString());
                    field->SetValue("MODULE_ID", id.toStdString());
                    field->SetValue("COUPLING_TYPE", Agros2DGenerator::couplingTypeToString(weakform.couplingtype().c_str()).toStdString());
                    QString exprCpp;
                    exprCpp = parseWeakFormExpression(analysisTypeFromStringKey(QString::fromStdString(weakform.sourceanalysis())),
                                                      analysisTypeFromStringKey(QString::fromStdString(weakform.targetanalysis())),
                                                      coordinateType,
                                                      expression);
                    field->SetValue("EXPRESSION", exprCpp.toStdString());

                    foreach(XMLModule::quantity quantity, m_sourceModule->volume().quantity())
                    {
                        ctemplate::TemplateDictionary *subField = 0;
                        subField = field->AddSectionDictionary("VARIABLE_SOURCE");
                        subField->SetValue("VARIABLE", quantity.id().c_str());
                        subField->SetValue("VARIABLE_SHORT", m_sourceVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());
                    }

                    foreach(XMLModule::quantity quantity, m_targetModule->volume().quantity())
                    {
                        ctemplate::TemplateDictionary *subField = 0;
                        subField = field->AddSectionDictionary("VARIABLE_SOURCE");
                        subField->SetValue("VARIABLE", quantity.id().c_str());
                        subField->SetValue("VARIABLE_SHORT", m_targetVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());
                    }

                    field = output.AddSectionDictionary("SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                }
            }
        }
    }

}

void Agros2DGeneratorCoupling::generateVolumeVectorForm(XMLCoupling::weakform_volume weakform, ctemplate::TemplateDictionary &output)
{
    foreach(XMLCoupling::vector_form vector_form, weakform.vector_form())
    {
        foreach(LinearityType linearityType, Agros2DGenerator::linearityTypeList())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                QString expression = weakformExpression(coordinateType, linearityType, vector_form);
                if(expression != "")
                {
                    QString id = (QString::fromStdString(m_coupling->general().id().c_str())).replace("-", "_");
                    QString functionName = QString("%1_%2_%3_%4_%5_%6_%7_%8_%9").
                            arg("volume_vector").
                            arg(QString::fromStdString(id.toStdString())).
                            arg(QString::fromStdString(weakform.sourceanalysis())).
                            arg(QString::fromStdString(weakform.targetanalysis())).
                            arg(coordinateTypeToStringKey(coordinateType)).
                            arg(linearityTypeToStringKey(linearityType)).
                            arg(QString::number(vector_form.i())).
                            arg(QString::number(vector_form.j())).
                            arg(QString::fromStdString(weakform.couplingtype()));


                    ctemplate::TemplateDictionary *field;
                    field = output.AddSectionDictionary("VECTOR_VOL_SOURCE"); // parameter output
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                    field->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
                    field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
                    field->SetValue("ANALYSIS_TYPE", weakform.sourceanalysis() + weakform.targetanalysis());
                    field->SetValue("ROW_INDEX", QString::number(vector_form.i()).toStdString());
                    field->SetValue("COLUMN_INDEX", QString::number(vector_form.j()).toStdString());
                    field->SetValue("MODULE_ID", id.toStdString());
                    field->SetValue("COUPLING_TYPE", Agros2DGenerator::couplingTypeToString(weakform.couplingtype().c_str()).toStdString());
                    QString exprCpp;
                    exprCpp = parseWeakFormExpression(analysisTypeFromStringKey(QString::fromStdString(weakform.sourceanalysis())),
                                                      analysisTypeFromStringKey(QString::fromStdString(weakform.targetanalysis())),
                                                      coordinateType,
                                                      expression);
                    field->SetValue("EXPRESSION", exprCpp.toStdString());

                    foreach(XMLModule::quantity quantity, m_sourceModule->volume().quantity())
                    {
                        ctemplate::TemplateDictionary *subField = 0;
                        subField = field->AddSectionDictionary("VARIABLE_SOURCE");
                        subField->SetValue("VARIABLE", quantity.id().c_str());
                        subField->SetValue("VARIABLE_SHORT", m_sourceVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());
                    }

                    foreach(XMLModule::quantity quantity, m_targetModule->volume().quantity())
                    {
                        ctemplate::TemplateDictionary *subField = 0;
                        subField = field->AddSectionDictionary("VARIABLE_SOURCE");
                        subField->SetValue("VARIABLE", quantity.id().c_str());
                        subField->SetValue("VARIABLE_SHORT", m_targetVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());
                    }

                    field = output.AddSectionDictionary("SOURCE");
                    field->SetValue("FUNCTION_NAME", functionName.toStdString());
                }
            }
        }
    }
}
