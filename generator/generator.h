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

#ifndef GENERATOR_H
#define GENERATOR_H

#include "util.h"
#include "ctemplate/template.h"
#include "../../resources_source/classes/module_xml.h"
#include "../../resources_source/classes/coupling_xml.h"

class Agros2DGenerator : public QCoreApplication
{
    Q_OBJECT

public:
    Agros2DGenerator(int &argc, char **argv);

    QList<XMLModule::module> m_modules;


    // static methods
    static QList<WeakFormKind> weakFormTypeList();
    static QString weakFormTypeStringEnum(WeakFormKind weakformType);

    static QList<CouplingType> couplingFormTypeList();
    static QString couplingTypeStringEnum(CouplingType couplingType);
    static QString couplingTypeToString(QString couplingType);

    static QList<LinearityType> linearityTypeList();
    static QString linearityTypeStringEnum(LinearityType linearityType);

    static QString physicFieldVariableCompStringEnum(PhysicFieldVariableComp physicFieldVariableComp);

    static QList<CoordinateType> coordinateTypeList();
    static QString coordinateTypeStringEnum(CoordinateType coordinateType);

    static QString analysisTypeStringEnum(AnalysisType analysisType);

    static QString boundaryTypeString(const QString boundaryName);
    static int numberOfSolutions(XMLModule::analyses analyses, AnalysisType analysisType);

public slots:
    void run();
};

class Agros2DGeneratorModule : public QObject
{
    Q_OBJECT

public:
    Agros2DGeneratorModule(const QString &moduleId);

    void generatePluginProjectFile();
    void generatePluginFilterFiles();
    void generatePluginLocalPointFiles();
    void generatePluginInterfaceFiles();
    void generatePluginWeakFormFiles();
    QHash<QString, QString>  volumeVaribales() const {return m_volumeVariables;}
    QHash<QString, QString>  surfaceVaribales() const {return m_volumeVariables;}

private:
    std::auto_ptr<XMLModule::module> module_xsd;
    XMLModule::module *m_module;

    // dictionary for variables used in weakforms
    QHash<QString, QString> m_volumeVariables;
    QHash<QString, QString> m_surfaceVariables;

    void generatePluginWeakFormSourceFiles();
    void generatePluginWeakFormHeaderFiles();

    void generateVolumeMatrixForm(XMLModule::weakform_volume weakform, ctemplate::TemplateDictionary &output);
    void generateVolumeVectorForm(XMLModule::weakform_volume weakform, ctemplate::TemplateDictionary &output);
    void generateSurfaceMatrixForm(XMLModule::boundary boundary, ctemplate::TemplateDictionary &output, XMLModule::weakform_surface weakform);
    void generateSurfaceVectorForm(XMLModule::boundary boundary, ctemplate::TemplateDictionary &output, XMLModule::weakform_surface weakform);
    void generateExactSolution(XMLModule::boundary boundary, ctemplate::TemplateDictionary &output, XMLModule::weakform_surface weakform);

    template <typename TForm>
    QString weakformExpression(CoordinateType coordinateType, LinearityType linearityType, TForm tForm);

    QString nonlinearExpression(const QString &variable, AnalysisType analysisType, CoordinateType coordinateType);

    QString parsePostprocessorExpression(AnalysisType analysisType, CoordinateType coordinateType, const QString &expr);
    void createPostprocessorExpression(ctemplate::TemplateDictionary &output, const QString &variable, AnalysisType analysisType, CoordinateType coordinateType, PhysicFieldVariableComp physicFieldVariableComp, const QString &expr);

    void createLocalValueExpression(ctemplate::TemplateDictionary &output, const QString &variable, AnalysisType analysisType, CoordinateType coordinateType, const QString &exprScalar, const QString &exprVectorX, const QString &exprVectorY);

    QString parseWeakFormExpression(AnalysisType analysisType, CoordinateType coordinateType, const QString &expr);
};

class Agros2DGeneratorCoupling : public QObject
{
    Q_OBJECT

public:
    Agros2DGeneratorCoupling(const QString &couplingId);

    void generatePluginProjectFile();
    void generatePluginFilterFiles();
    void generatePluginLocalPointFiles();
    void generatePluginInterfaceFiles();
    void generatePluginWeakFormFiles();
    QHash<QString, QString>  sourceVaribales() const {return m_sourceVariables;}
    QHash<QString, QString>  targetVaribales() const {return m_targetVariables;}

private:
    std::auto_ptr<XMLCoupling::coupling> coupling_xsd;
    XMLCoupling::coupling *m_coupling;

    std::auto_ptr<XMLModule::module> m_source_module_xsd;
    XMLModule::module *m_sourceModule;

    std::auto_ptr<XMLModule::module> m_target_module_xsd;
    XMLModule::module *m_targetModule;

    // dictionary for variables used in weakforms
    QHash<QString, QString> m_sourceVariables;
    QHash<QString, QString> m_targetVariables;

    void generatePluginWeakFormSourceFiles();
    void generatePluginWeakFormHeaderFiles();

    void generateVolumeMatrixForm(XMLCoupling::weakform_volume weakform, ctemplate::TemplateDictionary &output);
    void generateVolumeVectorForm(XMLCoupling::weakform_volume weakform, ctemplate::TemplateDictionary &output);

    template <typename TForm>
    QString weakformExpression(CoordinateType coordinateType, LinearityType linearityType, TForm tForm);
    QString nonlinearExpression(const QString &variable, AnalysisType analysisType, CoordinateType coordinateType);

    QString parseWeakFormExpression(AnalysisType sourceAnalysisType, AnalysisType targetAnalysisType,CoordinateType coordinateType, const QString &expr);
};

#endif // GENERATOR_H
