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

#ifndef GENERATOR_COUPLING_H
#define GENERATOR_COUPLING_H

#include "generator.h"

class Agros2DGeneratorCoupling : public Agros2DGeneratorBase
{

public:
    Agros2DGeneratorCoupling(const QString &couplingId);

    void generatePluginProjectFile();
    void generatePluginFilterFiles();
    void generatePluginLocalPointFiles();
    void generatePluginInterfaceFiles();
    void generatePluginWeakFormFiles();
    QMap<QString, QString>  sourceVaribales() const {return m_sourceVariables;}
    QMap<QString, QString>  targetVaribales() const {return m_targetVariables;}


private:
    std::auto_ptr<XMLCoupling::coupling> coupling_xsd;
    XMLCoupling::coupling *m_coupling;

    std::auto_ptr<XMLModule::module> m_source_module_xsd;
    XMLModule::module *m_sourceModule;

    std::auto_ptr<XMLModule::module> m_target_module_xsd;
    XMLModule::module *m_targetModule;

    // dictionary for variables used in weakforms
    QMap<QString, QString> m_sourceVariables;
    QMap<QString, QString> m_targetVariables;


    void generatePluginWeakFormSourceFiles();
    void generatePluginWeakFormHeaderFiles();

    template <typename Form>
    void generateForm(Form form, XMLCoupling::weakform_volume weakform, ctemplate::TemplateDictionary &output, QString weakFormType);

    //template <typename Form>
    //QString weakformExpression(CoordinateType coordinateType, LinearityType linearityType, Form form);
    QString nonlinearExpression(const QString &variable, AnalysisType analysisType, CoordinateType coordinateType);

    QString parseWeakFormExpression(AnalysisType sourceAnalysisType, AnalysisType targetAnalysisType,CoordinateType coordinateType, const QString &expr);    
    QString generateDocWeakFormExpression(QString symbol);
};

#endif // GENERATOR_COUPLING_H
