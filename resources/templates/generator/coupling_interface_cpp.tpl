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

#include "{{ID}}_interface.h"
#include "{{ID}}_weakform.h"

#include "util.h"
#include "util/global.h"
#include "util/constants.h"

{{CLASS}}Interface::{{CLASS}}Interface() : PluginInterface()
{
    // xml coupling description
    std::auto_ptr<XMLCoupling::coupling> coupling_xsd = XMLCoupling::coupling_((datadir() + COUPLINGROOT + QDir::separator() + "{{ID}}.xml").toStdString());
    m_coupling = coupling_xsd.release();
}

{{CLASS}}Interface::~{{CLASS}}Interface()
{
    delete m_coupling;
}

MatrixFormVolAgros<double> *{{CLASS}}Interface::matrixFormVol(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Material *material)
{
    {{#VOLUME_MATRIX_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}}) && (problemId.analysisTypeSource == {{SOURCE_ANALYSIS_TYPE}}) &&
            (problemId.analysisTypeTarget == {{TARGET_ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) &&
            (form->id == "{{WEAKFORM_ID}}") && (form->i == {{ROW_INDEX}}) && (form->j == {{COLUMN_INDEX}}) && (problemId.couplingType == {{COUPLING_TYPE}}))
        return new {{FUNCTION_NAME}}<double>(form->i - 1 + offsetI, form->j - 1 + offsetJ, offsetI, offsetJ);
    {{/VOLUME_MATRIX_SOURCE}}

    return NULL;
}

VectorFormVolAgros<double> *{{CLASS}}Interface::vectorFormVol(const ProblemID problemId, FormInfo *form, int offsetI, int offsetJ, Material *material, int offsetTimeExt)
{
    {{#VOLUME_VECTOR_SOURCE}}
    if ((problemId.coordinateType == {{COORDINATE_TYPE}}) && (problemId.analysisTypeSource == {{SOURCE_ANALYSIS_TYPE}}) &&
            (problemId.analysisTypeTarget == {{TARGET_ANALYSIS_TYPE}}) && (problemId.linearityType == {{LINEARITY_TYPE}}) &&
            (form->id == "{{WEAKFORM_ID}}") && (form->i == {{ROW_INDEX}}) && (form->j == {{COLUMN_INDEX}}) && (problemId.couplingType == {{COUPLING_TYPE}}))
        return new {{FUNCTION_NAME}}<double>(form->i - 1 + offsetI, form->j - 1 + offsetJ, offsetI, offsetJ, offsetTimeExt);
    {{/VOLUME_VECTOR_SOURCE}}

    return NULL;
}

Q_EXPORT_PLUGIN2(agros2d_plugin_{{ID}}, {{CLASS}}Interface)
