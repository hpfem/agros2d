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

class Agros2DGenerator : public QCoreApplication
{
    Q_OBJECT

public:
    Agros2DGenerator(int &argc, char **argv);

public slots:
    void run();

private:
    void generatePluginProjectFile(XMLModule::module *module);
    void generatePluginInterfaceFiles(XMLModule::module *module);
    void generatePluginFilterFiles(XMLModule::module *module);
    void generatePluginWeakFormFiles(XMLModule::module *module);
    void generateVolumeMatrixForm(XMLModule::weakform_volume weakform, ctemplate::TemplateDictionary &output, XMLModule::module *module);
    void generateVolumeVectorForm(XMLModule::weakform_volume weakform, ctemplate::TemplateDictionary &output, XMLModule::module *module);
    void generateSurfaceMatrixForm(XMLModule::boundary boundary, ctemplate::TemplateDictionary &output, XMLModule::module *module, XMLModule::weakform_surface weakform);
    void generateSurfaceVectorForm(XMLModule::boundary boundary, ctemplate::TemplateDictionary &output, XMLModule::module *module, XMLModule::weakform_surface weakform);
    void generateExactSolution(XMLModule::boundary boundary, ctemplate::TemplateDictionary &output, XMLModule::module *module, XMLModule::weakform_surface weakform);
    QString getExpression(CoordinateType coordinateType, LinearityType linearityType, XMLModule::matrix_form matrix_form);
    QString getExpression(CoordinateType coordinateType, LinearityType linearityType, XMLModule::vector_form vector_form);
    QString getExpression(CoordinateType coordinateType, LinearityType linearityType, XMLModule::essential_form essential_form);

    QString parsePostprocessorExpression(XMLModule::module *module, AnalysisType analysisType, CoordinateType coordinateType, const QString &expr);
    int numberOfSolutions(XMLModule::analyses analyses, AnalysisType analysisType);

    inline QList<WeakFormKind> weakFormTypeList() { QList<WeakFormKind> list; list << WeakForm_MatVol << WeakForm_MatSurf << WeakForm_VecVol << WeakForm_VecSurf << WeakForm_ExactSol; return list; }
    QString weakFormTypeStringEnum(WeakFormKind weakformType)
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

    inline QList<LinearityType> linearityTypeList() { QList<LinearityType> list; list << LinearityType_Linear << LinearityType_Newton << LinearityType_Picard << LinearityType_Undefined; return list; }
    QString linearityTypeStringEnum(LinearityType linearityType)
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

    inline QList<CoordinateType> coordinateTypeList() { QList<CoordinateType> list; list << CoordinateType_Planar << CoordinateType_Axisymmetric; return list; }
    QString coordinateTypeStringEnum(CoordinateType coordinateType)
    {
        if (coordinateType == CoordinateType_Planar)
            return "CoordinateType_Planar";
        else if (coordinateType == CoordinateType_Axisymmetric)
            return "CoordinateType_Axisymmetric";
        else
            assert(0);
    }

    QString analysisTypeStringEnum(AnalysisType analysisType)
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

    QString boundaryTypeString(const QString boundaryName)
    {
        return boundaryName.toLower().replace(" ","_");
    }
};

#endif // GENERATOR_H
