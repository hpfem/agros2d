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

#include "electrostatic_steadystate_axi.h"

#include "util.h"
#include "scene.h"
#include "hermes2d.h"
#include "module.h"
#include "problem.h"
#include "value.h"

using namespace electrostaticsteadystateaxi;


template <typename Scalar>
CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>::CustomMatrixFormVol__1_1_steadystate_linear_(unsigned int i, unsigned int j,
                                                 std::string area,
                                                 Hermes::Hermes2D::SymFlag sym,
                                                 Material *materialSource,
						 Material *materialTarget
						)
    : Hermes::Hermes2D::MatrixFormVol<Scalar>(i, j, area, sym), m_materialSource(materialSource), m_materialTarget(materialTarget), m_sym(sym)
{
    assert(0);

 el_epsr = m_materialSource->value("electrostatic_permittivity");

 qDebug() << el_epsr.text();
 el_rho = m_materialSource->value("electrostatic_charge_density");
	
}



 

template <typename Scalar>
Scalar CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u,
                                          Hermes::Hermes2D::Func<double> *v, Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * (el_epsr.number()*EPS0*e->x[i]*(u->dx[i]*v->dx[i]+u->dy[i]*v->dy[i]));
    }
    return result;
}



template <typename Scalar>
Hermes::Ord CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u,
                                             Hermes::Hermes2D::Func<Hermes::Ord> *v, Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * (1*EPS0*e->x[i]*(u->dx[i]*v->dx[i]+u->dy[i]*v->dy[i]));
    }	
    return result;
}

template <typename Scalar>
CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>* CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>::clone()
{
    return new CustomMatrixFormVol__1_1_steadystate_linear_(this->i, this->j, this->areas[0], this->m_sym, 
                                         this->m_materialSource, this->m_materialTarget);
}




template <typename Scalar>
CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>::CustomMatrixFormVol__1_1_steadystate_newton_(unsigned int i, unsigned int j,
                                                 std::string area,
                                                 Hermes::Hermes2D::SymFlag sym,
                                                 Material *materialSource,
						 Material *materialTarget
						)
    : Hermes::Hermes2D::MatrixFormVol<Scalar>(i, j, area, sym), m_materialSource(materialSource), m_materialTarget(materialTarget), m_sym(sym)
{
 el_epsr = m_materialSource->value("electrostatic_permittivity");
 el_rho = m_materialSource->value("electrostatic_charge_density");
	
}



 

template <typename Scalar>
Scalar CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u,
                                          Hermes::Hermes2D::Func<double> *v, Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * (el_epsr.number()*EPS0*e->x[i]*(u->dx[i]*v->dx[i]+u->dy[i]*v->dy[i]));
    }
    return result;
}



template <typename Scalar>
Hermes::Ord CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u,
                                             Hermes::Hermes2D::Func<Hermes::Ord> *v, Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * (1*EPS0*e->x[i]*(u->dx[i]*v->dx[i]+u->dy[i]*v->dy[i]));
    }	
    return result;
}

template <typename Scalar>
CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>* CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>::clone()
{
    return new CustomMatrixFormVol__1_1_steadystate_newton_(this->i, this->j, this->areas[0], this->m_sym, 
                                         this->m_materialSource, this->m_materialTarget);
}




template <typename Scalar>
CustomVectorFormVol__1_1_steadystate_linear_<Scalar>::CustomVectorFormVol__1_1_steadystate_linear_(unsigned int i, unsigned int j,
                                                 std::string area, 
                                                 Material* materialSource, Material* materialTarget)
    : Hermes::Hermes2D::VectorFormVol<Scalar>(i, area), m_materialSource(materialSource), m_materialTarget(materialTarget), j(j)
{
 el_epsr = m_materialSource->value("electrostatic_permittivity");
 el_rho = m_materialSource->value("electrostatic_charge_density");
	
}





template <typename Scalar>
Scalar CustomVectorFormVol__1_1_steadystate_linear_<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                          Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;

    for (int i = 0; i < n; i++)
    {
        result += wt[i] * (el_rho.number()*e->x[i]*v->val[i]);
    }
    return result;
}





template <typename Scalar>
Hermes::Ord CustomVectorFormVol__1_1_steadystate_linear_<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                             Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
{
    //variable_definition
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * (1*e->x[i]*v->val[i]);
    }	
    return result;
}



template <typename Scalar>
CustomVectorFormVol__1_1_steadystate_linear_<Scalar>* CustomVectorFormVol__1_1_steadystate_linear_<Scalar>::clone()
{
    return new CustomVectorFormVol__1_1_steadystate_linear_(this->i, this->j, this->areas[0],
                                         this->m_materialSource, this->m_materialTarget);
}




template <typename Scalar>
CustomVectorFormVol__1_1_steadystate_newton_<Scalar>::CustomVectorFormVol__1_1_steadystate_newton_(unsigned int i, unsigned int j,
                                                 std::string area, 
                                                 Material* materialSource, Material* materialTarget)
    : Hermes::Hermes2D::VectorFormVol<Scalar>(i, area), m_materialSource(materialSource), m_materialTarget(materialTarget), j(j)
{
 el_epsr = m_materialSource->value("electrostatic_permittivity");
 el_rho = m_materialSource->value("electrostatic_charge_density");
	
}





template <typename Scalar>
Scalar CustomVectorFormVol__1_1_steadystate_newton_<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                          Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;

    for (int i = 0; i < n; i++)
    {
        result += wt[i] * (el_epsr.number()*EPS0*e->x[i]*(u_ext[this->j]->dx[i]*v->dx[i]+u_ext[this->j]->dy[i]*v->dy[i])-e->x[i]*el_rho.number()*v->val[i]);
    }
    return result;
}





template <typename Scalar>
Hermes::Ord CustomVectorFormVol__1_1_steadystate_newton_<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                             Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
{
    //variable_definition
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * (1*EPS0*e->x[i]*(u_ext[this->j]->dx[i]*v->dx[i]+u_ext[this->j]->dy[i]*v->dy[i])-e->x[i]*1*v->val[i]);
    }	
    return result;
}



template <typename Scalar>
CustomVectorFormVol__1_1_steadystate_newton_<Scalar>* CustomVectorFormVol__1_1_steadystate_newton_<Scalar>::clone()
{
    return new CustomVectorFormVol__1_1_steadystate_newton_(this->i, this->j, this->areas[0],
                                         this->m_materialSource, this->m_materialTarget);
}




template <typename Scalar>
CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_linear_<Scalar>::CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_linear_(Hermes::Hermes2D::Mesh *mesh, Boundary *boundary)
    : Hermes::Hermes2D::ExactSolutionScalar<Scalar>(mesh), m_boundarySource(boundary)
{

    qDebug() << "m_boundarySource" << m_boundarySource;
 V = m_boundarySource->value("electrostatic_potential");
 qDebug() << V.text();
 sigma = m_boundarySource->value("electrostatic_surface_charge_density");

}





template <typename Scalar>
Scalar CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_linear_<Scalar>::value(double x, double y) const
{
    qDebug() << "value";
    double result = (V.number());
    return result;
}





template <typename Scalar>
void CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_linear_<Scalar>::derivatives (double x, double y, Scalar& dx, Scalar& dy) const
{

}





template <typename Scalar>
CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_newton_<Scalar>::CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_newton_(Hermes::Hermes2D::Mesh *mesh, Boundary *boundary)
    : Hermes::Hermes2D::ExactSolutionScalar<Scalar>(mesh), m_boundarySource(boundary)
{
 V = m_boundarySource->value("electrostatic_potential");
 sigma = m_boundarySource->value("electrostatic_surface_charge_density");

}





template <typename Scalar>
Scalar CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_newton_<Scalar>::value(double x, double y) const
{
    double result = (V.number());
    return result;
}





template <typename Scalar>
void CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_newton_<Scalar>::derivatives (double x, double y, Scalar& dx, Scalar& dy) const
{

}





template <typename Scalar>
CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_linear_<Scalar>::CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_linear_(unsigned int i, unsigned int j,
                                                   std::string area, 
                                                   Boundary *boundary)

    : Hermes::Hermes2D::VectorFormSurf<Scalar>(i, area), m_boundarySource(boundary), j(j)
{
 V = m_boundarySource->value("electrostatic_potential");
 sigma = m_boundarySource->value("electrostatic_surface_charge_density");

}





template <typename Scalar>
Scalar CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_linear_<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                           Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;    
    for (int i = 0; i < n; i++)
    {
        result += wt[i] * (sigma.number()*e->x[i]*v->val[i]);
    }
    return result;
}





template <typename Scalar>
Hermes::Ord CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_linear_<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                              Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * (1*e->x[i]*v->val[i]);
    }	
    return result;

}



template <typename Scalar>
CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_linear_<Scalar>* CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_linear_<Scalar>::clone()
{
    return new CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_linear_(this->i, this->j, this->areas[0],  
                                         this->m_boundarySource);
}




template <typename Scalar>
CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_newton_<Scalar>::CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_newton_(unsigned int i, unsigned int j,
                                                   std::string area, 
                                                   Boundary *boundary)

    : Hermes::Hermes2D::VectorFormSurf<Scalar>(i, area), m_boundarySource(boundary), j(j)
{
 V = m_boundarySource->value("electrostatic_potential");
 sigma = m_boundarySource->value("electrostatic_surface_charge_density");

}





template <typename Scalar>
Scalar CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_newton_<Scalar>::value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                                           Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const
{
    double result = 0;    
    for (int i = 0; i < n; i++)
    {
        result += wt[i] * (sigma.number()*e->x[i]*v->val[i]);
    }
    return result;
}





template <typename Scalar>
Hermes::Ord CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_newton_<Scalar>::ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                                              Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const
{
    Hermes::Ord result(0);    
    for (int i = 0; i < n; i++)
    {
       result += wt[i] * (1*e->x[i]*v->val[i]);
    }	
    return result;

}



template <typename Scalar>
CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_newton_<Scalar>* CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_newton_<Scalar>::clone()
{
    return new CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_newton_(this->i, this->j, this->areas[0],  
                                         this->m_boundarySource);
}



template class CustomVectorFormVol__1_1_steadystate_newton_<double>;
template class CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_linear_<double>;
template class CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_newton_<double>;
template class CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_newton_<double>;
template class CustomMatrixFormVol__1_1_steadystate_newton_<double>;
template class CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_linear_<double>;
template class CustomMatrixFormVol__1_1_steadystate_linear_<double>;
template class CustomVectorFormVol__1_1_steadystate_linear_<double>;
