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

#ifndef {{ID}}_FILTER_H
#define {{ID}}_FILTER_H

#include <QObject>

#include "util.h"
#include "util/enums.h"
#include "hermes2d/field.h"
#include "hermes2d.h"

#include "{{ID}}_interface.h"

class SceneLabelContainer;

class {{CLASS}}ViewScalarFilter : public dealii::DataPostprocessorScalar<2>
{
public:
    {{CLASS}}ViewScalarFilter(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                     MultiArray *ma,
                     const QString &variable,
                     PhysicFieldVariableComp physicFieldVariableComp);
    virtual ~{{CLASS}}ViewScalarFilter();    

    virtual void compute_derived_quantities_scalar (const std::vector<double> &uh,
                                                    const std::vector<dealii::Tensor<1,2> > &duh,
                                                    const std::vector<dealii::Tensor<2,2> > &dduh,
                                                    const std::vector<dealii::Point<2> > &normals,
                                                    const std::vector<dealii::Point<2> > &evaluation_points,
                                                    std::vector<dealii::Vector<double> > &computed_quantities) const;
protected:

private:
    const FieldInfo *m_fieldInfo;
    int m_timeStep;
    int m_adaptivityStep;
    SolutionMode m_solutionType;
    MultiArray *ma;

    SceneLabelContainer *m_labels;

    QString m_variable;
    uint m_variableHash;
    PhysicFieldVariableComp m_physicFieldVariableComp;
    CoordinateType m_coordinateType;

    {{#SPECIAL_FUNCTION_SOURCE}}
    QSharedPointer<{{SPECIAL_EXT_FUNCTION_FULL_NAME}}> {{SPECIAL_FUNCTION_NAME}};{{/SPECIAL_FUNCTION_SOURCE}}   
};

#endif // {{ID}}_FILTER_H
