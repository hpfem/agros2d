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

class {{CLASS}}ViewScalarFilter : public Hermes::Hermes2D::Filter<double>
{
public:
    {{CLASS}}ViewScalarFilter(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionType,
                     Hermes::vector<Hermes::Hermes2D::MeshFunctionSharedPtr<double> > sln,
                     const QString &variable,
                     PhysicFieldVariableComp physicFieldVariableComp);

    virtual Hermes::Hermes2D::Func<double> *get_pt_value(double x, double y, bool use_MeshHashGrid = false, Hermes::Hermes2D::Element* e = NULL);

    {{CLASS}}ViewScalarFilter* clone() const;

protected:

    void precalculate(int order, int mask);

private:
    const FieldInfo *m_fieldInfo;
    int m_timeStep;
    int m_adaptivityStep;
    SolutionMode m_solutionType;

    QString m_variable;
    uint m_variableHash;
    PhysicFieldVariableComp m_physicFieldVariableComp;
};

#endif // {{ID}}_FILTER_H
