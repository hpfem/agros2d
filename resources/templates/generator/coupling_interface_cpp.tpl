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

#include "hermes2d/plugin_interface.h"

#include "{{ID}}_interface.h"
#include "{{ID}}_weakform.h"

#include "util.h"
#include "hermes2d.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

#include "hermes2d/marker.h"

Hermes::Hermes2D::MatrixFormVol<double> *{{CLASS}}Interface::matrixFormVol(const ProblemID problemId, int i, int j,                                                              
                                                              Material *materialSource,
                                                              Material *materialTarget, int offsetI, int offsetJ)
{
	{{#VOLUME_MATRIX_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) && (i == {{ROW_INDEX}})
        && (j == {{COLUMN_INDEX}}) && (problemId.couplingType == {{COUPLING_TYPE}}))
        return new {{FUNCTION_NAME}}<double>(i-1 + offsetI, j-1 + offsetJ, materialSource, materialTarget);
	{{/VOLUME_MATRIX_SOURCE}}

    return NULL;
}

Hermes::Hermes2D::VectorFormVol<double> *{{CLASS}}Interface::vectorFormVol(const ProblemID problemId, int i, int j,
                                                                           Material *materialSource,
                                                                           Material *materialTarget, int offsetI, int offsetJ)
{
	{{#VOLUME_VECTOR_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) && (i == {{ROW_INDEX}})
        && (j == {{COLUMN_INDEX}}) && (problemId.couplingType == {{COUPLING_TYPE}}))
        return new {{FUNCTION_NAME}}<double>(i-1 + offsetI, j-1 + offsetJ, materialSource, materialTarget);
	{{/VOLUME_VECTOR_SOURCE}}

    return NULL;
}

Q_EXPORT_PLUGIN2({{ID}}, {{CLASS}}Interface)
