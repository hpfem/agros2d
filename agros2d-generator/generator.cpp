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
#include "generator_module.h"
#include "generator_coupling.h"

#include "util/constants.h"
#include "hermes2d/module.h"
#include "hermes2d/coupling.h"

#include "parser/lex.h"

#include "util/constants.h"

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
    // generate structure
    createStructure();

    QStringList args = QCoreApplication::arguments();
    if (args.count() == 2)
    {
        // generate one module or coupling
        QMap<QString, QString> modules = Module::availableModules();
        QMap<QString, QString> couplings = availableCouplings();

        if (modules.keys().contains(args[1]))
            generateModule(args[1]);
        else if (couplings.keys().contains(args[1]))
            generateCoupling(args[1]);
    }
    else
    {
        // generate all sources
        generateSources();
    }
}

void Agros2DGenerator::createStructure()
{
    // create directory
    QDir root(QApplication::applicationDirPath());
    root.mkpath(GENERATOR_PLUGINROOT);

    // documentation
    QDir doc_root(QApplication::applicationDirPath());
    doc_root.mkpath(GENERATOR_DOCROOT);

    ctemplate::TemplateDictionary output("project_output");
    QMap<QString, QString> modules = Module::availableModules();
    QMap<QString, QString> couplings = availableCouplings();

    foreach (QString moduleId, modules.keys())
    {
        ctemplate::TemplateDictionary *field = output.AddSectionDictionary("SOURCE");
        field->SetValue("ID", moduleId.toStdString());
    }

    // generate documentation

        std::string doc_text;
        ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/doc_index.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                           ctemplate::DO_NOT_STRIP, &output, &doc_text);

    // save to file
    writeStringContent(QString("%1/%2/index.rst").
                      arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_DOCROOT),
                      QString::fromStdString(doc_text));


    foreach (QString couplingId, couplings.keys())
    {
        ctemplate::TemplateDictionary *field = output.AddSectionDictionary("SOURCE");
        field->SetValue("ID", couplingId.toStdString());
    }

    // generate plugins project file
    // expand template
    std::string text;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/plugins_pro.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);
    // save to file
    writeStringContent(QString("%1/%2/plugins.pro").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT),
                       QString::fromStdString(text));

    exit(0);
}

void Agros2DGenerator::generateSources()
{
    QMap<QString, QString> modules = Module::availableModules();
    QMap<QString, QString> couplings = availableCouplings();

    foreach (QString moduleId, modules.keys())
    {
        generateModule(moduleId);
        generateDocumentation(moduleId);
    }

    foreach (QString couplingId, couplings.keys())
    {
        generateCoupling(couplingId);
    }
}

void Agros2DGenerator::generateModule(const QString &moduleId)
{
    Agros2DGeneratorModule generator(moduleId);

    generator.generatePluginProjectFile();
    generator.generatePluginInterfaceFiles();
    generator.generatePluginFilterFiles();
    generator.generatePluginForceFiles();
    generator.generatePluginLocalPointFiles();
    generator.generatePluginSurfaceIntegralFiles();
    generator.generatePluginVolumeIntegralFiles();
    generator.generatePluginWeakFormFiles();

    // generates documentation
    generator.generatePluginDocumentationFiles();

    // generates equations
    generator.generatePluginEquations();
}

void Agros2DGenerator::generateDocumentation(const QString &moduleId)
{
    Agros2DGeneratorModule generator(moduleId);

}

void Agros2DGenerator::generateCoupling(const QString &couplingId)
{
    Agros2DGeneratorCoupling generator(couplingId);

    generator.generatePluginProjectFile();
    generator.generatePluginInterfaceFiles();
    generator.generatePluginWeakFormFiles();
}
