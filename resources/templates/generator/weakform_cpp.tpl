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

#include "{{ID}}_weakform.h"

#include "util.h"
#include "util/global.h"

#include "scene.h"
#include "hermes2d.h"
#include "hermes2d/module.h"

#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"
#include "hermes2d/bdf2.h"
 		
{{#VOLUME_MATRIX_SOURCE}}
template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(unsigned int i, unsigned int j, int offsetI, int offsetJ)
    : MatrixFormVolAgros<Scalar>(i, j, offsetI, offsetJ)
{       
}


template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u,
                                          Hermes::Hermes2D::Func<double> *v, Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::Func<Scalar> **ext) const
{
    Scalar result = 0;
    for (int i = 0; i < n; i++)
    {
        result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}

template <typename Scalar>
Hermes::Ord {{FUNCTION_NAME}}<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u,
                                             Hermes::Hermes2D::Func<Hermes::Ord> *v, Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::Func<Hermes::Ord> **ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * ({{EXPRESSION}});
    }	
    return result;
}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>* {{FUNCTION_NAME}}<Scalar>::clone() const
{
    return new {{FUNCTION_NAME}}(*this);
}

template <typename Scalar>
void {{FUNCTION_NAME}}<Scalar>::setMarkerSource(Marker *marker)
{
    FormAgrosInterface::setMarkerSource(marker);

    {{#VARIABLE_SOURCE}}
    {{VARIABLE_SHORT}} = &this->m_markerSource->value("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}
    {{#SPECIAL_FUNCTION_SOURCE}}
    {{#PARAMETERS}}
    {{SPECIAL_FUNCTION_NAME}}.{{PARAMETER_NAME}} = this->{{PARAMETER_NAME}}->number(); {{/PARAMETERS}}
    {{SPECIAL_FUNCTION_NAME}}.setVariant("{{SELECTED_VARIANT}}");
    {{SPECIAL_FUNCTION_NAME}}.setType(specialFunctionTypeFromStringKey("{{TYPE}}"));
    {{SPECIAL_FUNCTION_NAME}}.setBounds({{FROM}}, {{TO}}, {{EXTRAPOLATE_LOW_PRESENT}}, {{EXTRAPOLATE_HI_PRESENT}});
    {{SPECIAL_FUNCTION_NAME}}.createInterpolation();{{/SPECIAL_FUNCTION_SOURCE}}

}

template <typename Scalar>
void {{FUNCTION_NAME}}<Scalar>::setMarkerTarget(Marker *marker)
{
    FormAgrosInterface::setMarkerTarget(marker);

    {{#VARIABLE_TARGET}}
    {{VARIABLE_SHORT}} = &this->m_markerTarget->value("{{VARIABLE}}");{{/VARIABLE_TARGET}}
}
{{/VOLUME_MATRIX_SOURCE}}

// ***********************************************************************************************************************************

{{#VOLUME_VECTOR_SOURCE}}
template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(unsigned int i, unsigned int j, int offsetI, int offsetJ, int *offsetTimeExt)
    : VectorFormVolAgros<Scalar>(i, offsetI, offsetJ, offsetTimeExt), j(j)
{
}

template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                          Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::Func<Scalar> **ext) const
{
    Scalar result = 0;

    for (int i = 0; i < n; i++)
    {
        result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}

template <typename Scalar>
Hermes::Ord {{FUNCTION_NAME}}<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                             Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::Func<Hermes::Ord> **ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * ({{EXPRESSION}});
    }	
    return result;
}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>* {{FUNCTION_NAME}}<Scalar>::clone() const
{
    return new {{FUNCTION_NAME}}(*this);
}

template <typename Scalar>
void {{FUNCTION_NAME}}<Scalar>::setMarkerSource(Marker *marker)
{
    FormAgrosInterface::setMarkerSource(marker);

    {{#VARIABLE_SOURCE}}
    {{VARIABLE_SHORT}} = &this->m_markerSource->value("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}
    {{#SPECIAL_FUNCTION_SOURCE}}
    {{#PARAMETERS}}
    {{SPECIAL_FUNCTION_NAME}}.{{PARAMETER_NAME}} = this->{{PARAMETER_NAME}}->number(); {{/PARAMETERS}}
    {{SPECIAL_FUNCTION_NAME}}.setVariant("{{SELECTED_VARIANT}}");
    {{SPECIAL_FUNCTION_NAME}}.setType(specialFunctionTypeFromStringKey("{{TYPE}}"));
    {{SPECIAL_FUNCTION_NAME}}.setBounds({{FROM}}, {{TO}}, {{EXTRAPOLATE_LOW_PRESENT}}, {{EXTRAPOLATE_HI_PRESENT}});
    {{SPECIAL_FUNCTION_NAME}}.createInterpolation();{{/SPECIAL_FUNCTION_SOURCE}}
}

template <typename Scalar>
void {{FUNCTION_NAME}}<Scalar>::setMarkerTarget(Marker *marker)
{
    FormAgrosInterface::setMarkerTarget(marker);

    {{#VARIABLE_TARGET}}
    {{VARIABLE_SHORT}} = &this->m_markerTarget->value("{{VARIABLE}}");{{/VARIABLE_TARGET}}
}
{{/VOLUME_VECTOR_SOURCE}}

// ***********************************************************************************************************************************

{{#SURFACE_MATRIX_SOURCE}}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(unsigned int i, unsigned int j, int offsetI, int offsetJ)
    : MatrixFormSurfAgros<Scalar>(i, j, offsetI, offsetJ)
{
}

template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u, Hermes::Hermes2D::Func<double> *v,
                                           Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::Func<Scalar> **ext) const
{
    Scalar result = 0;
    for (int i = 0; i < n; i++)
    {
        result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}

template <typename Scalar>
Hermes::Ord {{FUNCTION_NAME}}<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u, Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                              Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::Func<Hermes::Ord> **ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * ({{EXPRESSION}});
    }	
    return result;

}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>* {{FUNCTION_NAME}}<Scalar>::clone() const
{
    return new {{FUNCTION_NAME}}(*this);
}

template <typename Scalar>
void {{FUNCTION_NAME}}<Scalar>::setMarkerSource(Marker *marker)
{
    FormAgrosInterface::setMarkerSource(marker);

    {{#VARIABLE_SOURCE}}
    {{VARIABLE_SHORT}} = &this->m_markerSource->value("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}
}
{{/SURFACE_MATRIX_SOURCE}}

// ***********************************************************************************************************************************

{{#SURFACE_VECTOR_SOURCE}}
template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(unsigned int i, unsigned int j, int offsetI, int offsetJ)

    : VectorFormSurfAgros<Scalar>(i, offsetI, offsetJ), j(j)
{
}

template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                           Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::Func<Scalar> **ext) const
{
    Scalar result = 0;
    for (int i = 0; i < n; i++)
    {
        result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}

template <typename Scalar>
Hermes::Ord {{FUNCTION_NAME}}<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                              Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::Func<Hermes::Ord> **ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * ({{EXPRESSION}});
    }	
    return result;

}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>* {{FUNCTION_NAME}}<Scalar>::clone() const
{
    return new {{FUNCTION_NAME}}(*this);
}

template <typename Scalar>
void {{FUNCTION_NAME}}<Scalar>::setMarkerSource(Marker *marker)
{
    FormAgrosInterface::setMarkerSource(marker);

    {{#VARIABLE_SOURCE}}
    {{VARIABLE_SHORT}} = &this->m_markerSource->value("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}
}
{{/SURFACE_VECTOR_SOURCE}}

// ***********************************************************************************************************************************

{{#EXACT_SOURCE}}
template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(MeshSharedPtr mesh)
    : ExactSolutionScalarAgros<Scalar>(mesh)
{
}

template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(double x, double y) const
{
    Scalar result = {{EXPRESSION}};
    return result;
}

template <typename Scalar>
void {{FUNCTION_NAME}}<Scalar>::derivatives (double x, double y, Scalar& dx, Scalar& dy) const
{

}

template <typename Scalar>
void {{FUNCTION_NAME}}<Scalar>::setMarkerSource(Marker *marker)
{
    FormAgrosInterface::setMarkerSource(marker);

    {{#VARIABLE_SOURCE}}
    {{VARIABLE_SHORT}} = &this->m_markerSource->value("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}
}
{{/EXACT_SOURCE}}


// ***********************************************************************************************************************************

{{#SOURCE}}template class {{FUNCTION_NAME}}<double>;
{{/SOURCE}}
