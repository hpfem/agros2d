// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef GENERATOR_H
#define GENERATOR_H

#include "util.h"
#include "util/enums.h"

#include "ctemplate/template.h"
#include "../../resources_source/classes/module_xml.h"

const QString GENERATOR_TEMPLATEROOT = "resources/templates/generator";
const QString GENERATOR_DOCROOT = "resources_source/doc/source/modules";
const QString GENERATOR_PLUGINROOT = "plugins";

class LexicalAnalyser;

class Agros2DGenerator : public QCoreApplication
{
    Q_OBJECT

public:
    Agros2DGenerator(int &argc, char **argv);

    inline void setModuleName(const QString &module = "") { m_module = module; }

    // static methods
    static QList<WeakFormKind> weakFormTypeList();
    static QString weakFormTypeStringEnum(WeakFormKind weakformType);

    static QList<CouplingType> couplingFormTypeList();
    static QString couplingTypeStringEnum(CouplingType couplingType);
    static QString couplingTypeToString(QString couplingType);
    static CouplingType couplingTypeFromString(QString couplingType);

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

private:
    QString m_module;
};

class Agros2DGeneratorBase : public QObject
{
    Q_OBJECT

public:

private:
};

#endif // GENERATOR_H
