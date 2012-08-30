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


#include "util.h"
#include "scene.h"
#include "hermes2d.h"
#include "module.h"
#include "problem.h"
#include "{{ID}}_weakform.h"
 		
{{#VOLUME_MATRIX_SOURCE}}
template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(unsigned int i, unsigned int j,
                                                 std::string area,
                                                 Hermes::Hermes2D::SymFlag sym,
                                                 Material *materialSource,
						 Material *materialTarget
						)
    : Hermes::Hermes2D::MatrixFormVol<Scalar>(i, j, area, sym), m_materialSource(materialSource), m_materialTarget(materialTarget), m_sym(sym)
{
	{{#VARIABLE_SOURCE}}
    {{VARIABLE_SHORT}} = m_materialSource->value("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}
}


template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u,
                                          Hermes::Hermes2D::Func<double> *v, Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;    
    for (int i = 0; i < n; i++)
    {
        result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}

template <typename Scalar>
Hermes::Ord {{FUNCTION_NAME}}<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u,
                                             Hermes::Hermes2D::Func<Hermes::Ord> *v, Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * ({{EXPRESSION}});
    }	
    return result;
}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>* {{FUNCTION_NAME}}<Scalar>::clone()
{
    return new {{FUNCTION_NAME}}(this->i, this->j, this->areas[0], this->m_sym, 
                                         this->m_materialSource, this->m_materialTarget);
}
{{/VOLUME_MATRIX_SOURCE}}


{{#VOLUME_VECTOR_SOURCE}}
template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(unsigned int i, unsigned int j,
                                                 std::string area, 
                                                 Material* materialSource, Material* materialTarget)
    : Hermes::Hermes2D::VectorFormVol<Scalar>(i, area), m_materialSource(materialSource), m_materialTarget(materialTarget), j(j)
{
	{{#VARIABLE_SOURCE}}
    {{VARIABLE_SHORT}} = m_materialSource->value("{{VARIABLE}}");{{/VARIABLE_SOURCE}}
}

template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                          Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;

    for (int i = 0; i < n; i++)
    {
        result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}


template <typename Scalar>
Hermes::Ord {{FUNCTION_NAME}}<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                             Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * ({{EXPRESSION}});
    }	
    return result;
}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>* {{FUNCTION_NAME}}<Scalar>::clone()
{
    return new {{FUNCTION_NAME}}(this->i, this->j, this->areas[0],
                                         this->m_materialSource, this->m_materialTarget);
}
{{/VOLUME_VECTOR_SOURCE}}

{{#SURFACE_MATRIX_SOURCE}}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(unsigned int i, unsigned int j,
                                                   std::string area, 
                                                   Boundary *boundary)
    : Hermes::Hermes2D::MatrixFormSurf<Scalar>(i, j, area), m_boundarySource(boundary)
{
    {{#VARIABLE_SOURCE}}
    {{VARIABLE_SHORT}} = m_boundarySource->value("{{VARIABLE}}");{{/VARIABLE_SOURCE}}
}

template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u, Hermes::Hermes2D::Func<double> *v,
                                           Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;    
    for (int i = 0; i < n; i++)
    {
        result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}

template <typename Scalar>
Hermes::Ord {{FUNCTION_NAME}}<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u, Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                              Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * ({{EXPRESSION}});
    }	
    return result;

}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>* {{FUNCTION_NAME}}<Scalar>::clone()
{
    return new {{FUNCTION_NAME}}(this->i, this->j, this->areas[0],
                                         this->m_boundarySource);
}
{{/SURFACE_MATRIX_SOURCE}}

{{#SURFACE_VECTOR_SOURCE}}
template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(unsigned int i, unsigned int j,
                                                   std::string area, 
                                                   Boundary *boundary)

    : Hermes::Hermes2D::VectorFormSurf<Scalar>(i, area), m_boundarySource(boundary), j(j)
{
	{{#VARIABLE_SOURCE}}
    {{VARIABLE_SHORT}} = m_boundarySource->value("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}
}

template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                           Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;    
    for (int i = 0; i < n; i++)
    {
        result += wt[i] * ({{EXPRESSION}});
    }
    return result;
}

template <typename Scalar>
Hermes::Ord {{FUNCTION_NAME}}<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                              Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * ({{EXPRESSION}});
    }	
    return result;

}

template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>* {{FUNCTION_NAME}}<Scalar>::clone()
{
    return new {{FUNCTION_NAME}}(this->i, this->j, this->areas[0],  
                                         this->m_boundarySource);
}
{{/SURFACE_VECTOR_SOURCE}}

{{#EXACT_SOURCE}}
template <typename Scalar>
{{FUNCTION_NAME}}<Scalar>::{{FUNCTION_NAME}}(Hermes::Hermes2D::Mesh *mesh, Boundary *boundary)
    : Hermes::Hermes2D::ExactSolutionScalar<Scalar>(mesh), m_boundarySource(boundary)
{
	{{#VARIABLE_SOURCE}}
		{{VARIABLE_SHORT}} = m_boundarySource->value("{{VARIABLE}}"); {{/VARIABLE_SOURCE}}
}


template <typename Scalar>
Scalar {{FUNCTION_NAME}}<Scalar>::value(double x, double y) const
{
    double result = {{EXPRESSION}};
    return result;
}

template <typename Scalar>
void {{FUNCTION_NAME}}<Scalar>::derivatives (double x, double y, Scalar& dx, Scalar& dy) const
{

}
{{/EXACT_SOURCE}}


{{#SOURCE}}template class {{FUNCTION_NAME}}<double>;
{{/SOURCE}}


