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
#include "util/enums.h"

#include "ctemplate/template.h"
#include "../../resources_source/classes/module_xml.h"
#include "../../resources_source/classes/coupling_xml.h"

const QString GENERATOR_TEMPLATEROOT = "resources/templates/generator";
const QString GENERATOR_DOCROOT = "resources_source/doc/source/modules";
const QString GENERATOR_PLUGINROOT = "plugins";

class LexicalAnalyser;

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
    void createStructure();
    void generateSources();
    void generateModule(const QString &moduleId);
    void generateCoupling(const QString &couplingId);
    void generateDocumentation(const QString &couplingId);
};

class Agros2DGeneratorBase : public QObject
{
    Q_OBJECT

public:

protected:
    template <typename Form>
    QString weakformExpression(CoordinateType coordinateType, LinearityType linearityType, Form form)
    {
        QString expression;

        if ((linearityType == LinearityType_Linear || linearityType == LinearityType_Picard) && (coordinateType == CoordinateType_Planar))
        {
            expression = form.planar_linear().c_str();
        }

        if ((linearityType == LinearityType_Newton) && (coordinateType == CoordinateType_Planar))
        {
            expression = form.planar_newton().c_str();
        }

        if ((linearityType == LinearityType_Linear || linearityType == LinearityType_Picard) && (coordinateType == CoordinateType_Axisymmetric))
        {
            expression = form.axi_linear().c_str();
        }

        if ((linearityType == LinearityType_Newton) && (coordinateType == CoordinateType_Axisymmetric))
        {
            expression = form.axi_newton().c_str();
        }

        return expression;
    }

private:
};

#endif // GENERATOR_H
