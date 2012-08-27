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

#include <QtPlugin>

#include "weakform_interface.h"
#include "{{ID}}_interface.h"
#include "{{ID}}_weakform.h"
#include "{{ID}}_filter.h"

#include "util.h"
#include "hermes2d.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

#include "hermes2d/localpoint.h"
#include "hermes2d/volumeintegral.h"
#include "hermes2d/surfaceintegral.h"

#include "hermes2d/marker.h"
#include "hermes2d/localpoint.h"


Hermes::Hermes2D::MatrixFormVol<double> *{{CLASS}}Interface::matrixFormVol(const ProblemID problemId, int i, int j,
                                                              const std::string &area, Hermes::Hermes2D::SymFlag sym,
                                                              SceneMaterial *materialSource, Material *materialTarget, int offsetI, int offsetJ)
{
	{{#MATRIX_VOL_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) && (i == {{ROW_INDEX}})
        && (j == {{COLUMN_INDEX}}))
        return new {{FUNCTION_NAME}}<double>(i-1 + offsetI, j-1+ offsetJ , area, sym, materialSource, materialTarget);
	{{/MATRIX_VOL_SOURCE}}

    return NULL;
}

Hermes::Hermes2D::VectorFormVol<double> *{{CLASS}}Interface::vectorFormVol(const ProblemID problemId, int i, int j,
                                                              const std::string &area, SceneMaterial *materialSource, Material *materialTarget, int offsetI, int offsetJ)
{
	{{#VECTOR_VOL_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}} && (problemId.linearityType == {{LINEARITY_TYPE}}) && (i == {{ROW_INDEX}})
        && (j == {{COLUMN_INDEX}})))
    	return new {{FUNCTION_NAME}}<double>(i-1+ offsetI, j-1+ offsetJ, area, materialSource, materialTarget);
	{{/VECTOR_VOL_SOURCE}}

    return NULL;
}

Hermes::Hermes2D::MatrixFormSurf<double> *{{CLASS}}Interface::matrixFormSurf(const ProblemID problemId, int i, int j,
                                                                const std::string &area, SceneBoundary *boundary, int offsetI, int offsetJ)
{
	{{#MATRIX_SURF_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}} && (problemId.linearityType == {{LINEARITY_TYPE}}) && (i == {{ROW_INDEX}})
        && (j == {{COLUMN_INDEX}}) && (boundary->getType() == "{{BOUNDARY_TYPE}}")))
        return new {{FUNCTION_NAME}}<double>(i-1+ offsetI, j-1+ offsetJ, area, boundary);
	{{/MATRIX_SURF_SOURCE}}

    return NULL;
}

Hermes::Hermes2D::VectorFormSurf<double> *{{CLASS}}Interface::vectorFormSurf(const ProblemID problemId, int i, int j,
                                                                const std::string &area, SceneBoundary *boundary, int offsetI, int offsetJ)
{
	{{#VECTOR_SURF_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}} && (problemId.linearityType == {{LINEARITY_TYPE}}) && (i == {{ROW_INDEX}})
        && (j == {{COLUMN_INDEX}}) && (boundary->getType() == "{{BOUNDARY_TYPE}}")))
        return new {{FUNCTION_NAME}}<double>(i-1 + offsetI, j-1 + offsetJ, area, boundary);
	{{/VECTOR_SURF_SOURCE}}

    return NULL;
}

Hermes::Hermes2D::ExactSolutionScalar<double> *{{CLASS}}Interface::exactSolution(const ProblemID problemId, int i,Hermes::Hermes2D::Mesh *mesh, Boundary *boundary)
{
	{{#EXACT_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}} && (problemId.linearityType == {{LINEARITY_TYPE}}) && (i == {{ROW_INDEX}})))
        return new {{FUNCTION_NAME}}<double>(mesh, boundary);
	{{/EXACT_SOURCE}}
	
    return NULL;
}

Hermes::Hermes2D::Filter<double> *{{CLASS}}Interface::filter(FieldInfo *fieldInfo,
                                                     Hermes::vector<Hermes::Hermes2D::MeshFunction<double> *> sln,
                                                     const QString &variable,
                                                     PhysicFieldVariableComp physicFieldVariableComp,
                                                     AnalysisType analysisType,
                                                     CoordinateType coordinateType)
{
    return new {{CLASS}}ViewScalarFilter(fieldInfo, sln, variable, physicFieldVariableComp, analysisType, coordinateType);
}

Q_EXPORT_PLUGIN2({{ID}}, {{CLASS}}Interface)
