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

class Agros2DGeneratorModule : public Agros2DGeneratorBase
{
public:
    Agros2DGeneratorModule(const QString &moduleId);

    void generatePluginProjectFile();
    void generatePluginFilterFiles();
    void generatePluginForceFiles();
    void generatePluginLocalPointFiles();
    void generatePluginSurfaceIntegralFiles();
    void generatePluginVolumeIntegralFiles();
    void generatePluginInterfaceFiles();
    void generatePluginWeakFormFiles();
    void generatePluginDocuentationFiles();

private:
    std::auto_ptr<XMLModule::module> module_xsd;
    XMLModule::module *m_module;

    // dictionary for variables used in weakforms
    QHash<QString, QString> m_volumeVariables;
    QHash<QString, QString> m_surfaceVariables;

    void generatePluginWeakFormSourceFiles();
    void generatePluginWeakFormHeaderFiles();

    void generateWeakForms(ctemplate::TemplateDictionary &output);

    //ToDo: make up better names
    template <typename Form, typename WeakForm>
    void generateForm(Form form, ctemplate::TemplateDictionary &output, WeakForm weakform, QString weakFormType, XMLModule::boundary *boundary, int j);

    QString nonlinearExpression(const QString &variable, AnalysisType analysisType, CoordinateType coordinateType);
    QString dependence(const QString &variable, AnalysisType analysisType);
    LexicalAnalyser *postprocessorLexicalAnalyser(AnalysisType analysisType, CoordinateType coordinateType);
    QString parsePostprocessorExpression(AnalysisType analysisType, CoordinateType coordinateType, const QString &expr, bool includeVariables = true);

    void createFilterExpression(ctemplate::TemplateDictionary &output, const QString &variable, AnalysisType analysisType, CoordinateType coordinateType, PhysicFieldVariableComp physicFieldVariableComp, const QString &expr);
    void createLocalValueExpression(ctemplate::TemplateDictionary &output, const QString &variable, AnalysisType analysisType, CoordinateType coordinateType, const QString &exprScalar, const QString &exprVectorX, const QString &exprVectorY);
    void createIntegralExpression(ctemplate::TemplateDictionary &output, const QString &variable, AnalysisType analysisType, CoordinateType coordinateType, const QString &expr);

    LexicalAnalyser *weakFormLexicalAnalyser(AnalysisType analysisType, CoordinateType coordinateType);
    QString parseWeakFormExpression(AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType, const QString &expr, bool includeVariables = true);
    QString parseWeakFormExpressionCheck(AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType, const QString &expr);
};

#endif // GENERATOR_MODULE_H
