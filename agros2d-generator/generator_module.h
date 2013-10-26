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

#ifndef GENERATOR_MODULE_H
#define GENERATOR_MODULE_H

#include "generator.h"
struct FormInfo;

class Agros2DGeneratorModule : public Agros2DGeneratorBase
{
public:
    Agros2DGeneratorModule(const QString &moduleId);
    ~Agros2DGeneratorModule();

    void prepareWeakFormsOutput();
    void deleteWeakFormOutput();
    void generatePluginProjectFile();
    void generatePluginFilterFiles();
    void generatePluginForceFiles();
    void generatePluginLocalPointFiles();
    void generatePluginSurfaceIntegralFiles();
    void generatePluginVolumeIntegralFiles();
    void generatePluginInterfaceFiles();
    void generatePluginWeakFormFiles();
    void generatePluginDocumentationFiles();
    void generatePluginEquations();
    void generatePluginErrorCalculator();


private:
    std::auto_ptr<XMLModule::module> module_xsd;
    XMLModule::module *m_module;
    QString m_docString;
    QStringList m_names;
    // dictionary for variables used in weakforms
    QHash<QString, QString> m_volumeVariables;
    QHash<QString, QString> m_surfaceVariables;

    void getNames(const QString &moduleId);
    void generatePluginWeakFormSourceFiles();
    void generatePluginWeakFormHeaderFiles();

    void generateWeakForms(ctemplate::TemplateDictionary &output);
    void generateExtFunctions(ctemplate::TemplateDictionary &output);

    //ToDo: make up better names
    template <typename WeakForm>
    void generateForm(FormInfo form, LinearityType linearityType, ctemplate::TemplateDictionary &output, WeakForm weakform, QString weakFormType, XMLModule::boundary *boundary = 0);
    void generateExtFunction(XMLModule::quantity quantity, AnalysisType analysisType, LinearityType linearityType, bool derivative, ctemplate::TemplateDictionary &output);
    void generateSpecialFunction(XMLModule::function* function, ctemplate::TemplateDictionary *output);

    QString nonlinearExpression(const QString &variable, AnalysisType analysisType, CoordinateType coordinateType);
    QString dependence(const QString &variable, AnalysisType analysisType);
    LexicalAnalyser *postprocessorLexicalAnalyser(AnalysisType analysisType, CoordinateType coordinateType);
    QString parsePostprocessorExpression(AnalysisType analysisType, CoordinateType coordinateType, const QString &expr, bool includeVariables, bool forFilter = false);

    void createFilterExpression(ctemplate::TemplateDictionary &output, const QString &variable, AnalysisType analysisType, CoordinateType coordinateType, PhysicFieldVariableComp physicFieldVariableComp, const QString &expr);
    void createLocalValueExpression(ctemplate::TemplateDictionary &output, const QString &variable, AnalysisType analysisType, CoordinateType coordinateType, const QString &exprScalar, const QString &exprVectorX, const QString &exprVectorY);
    void createIntegralExpression(ctemplate::TemplateDictionary &output, const QString &section, const QString &variable, AnalysisType analysisType, CoordinateType coordinateType, const QString &expr, int pos);

    LexicalAnalyser *weakFormLexicalAnalyser(AnalysisType analysisType, CoordinateType coordinateType);
    QString parseWeakFormExpression(AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType, const QString &expr, bool includeVariables = true, bool errorCalculation = false);
    QString parseWeakFormExpressionCheck(AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType, const QString &expr);
    QString generateDocWeakFormExpression(AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType, const QString &expr, bool includeVariables = true);
    QString underline(QString text, char symbol);
    QString capitalize(QString text);
    QString createTable(QList<QStringList>);

    QMap<QString, int> quantityOrdering;
    QMap<QString, bool> quantityIsNonlinear;
    QMap<QString, int> functionOrdering;

    ctemplate::TemplateDictionary* m_output;
};

#endif // GENERATOR_MODULE_H
