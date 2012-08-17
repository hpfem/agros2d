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
#ifndef electrostatic_steadystate_axi
#define electrostatic_steadystate_axi

#include "util.h"
#include <weakform/weakform.h>
#include "hermes2d/marker.h"
namespace electrostaticsteadystateaxi
{template<typename Scalar>
class CustomMatrixFormVol__1_1_steadystate_linear_ : public Hermes::Hermes2D::MatrixFormVol<Scalar>
{
public:
    CustomMatrixFormVol__1_1_steadystate_linear_(unsigned int i, unsigned int j,
                              std::string area,
                              Hermes::Hermes2D::SymFlag sym,
                              Material *materialSource,
			      Material *materialTarget);

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u,
                         Hermes::Hermes2D::Func<double> *v, Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u,
                            Hermes::Hermes2D::Func<Hermes::Ord> *v, Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const;   	    
    CustomMatrixFormVol__1_1_steadystate_linear_<Scalar>* clone();
private:
    Material *m_materialSource;
    Material *m_materialTarget;	
    Hermes::Hermes2D::SymFlag m_sym;
     mutable Value el_epsr;
 mutable Value el_rho;
	    	
};template<typename Scalar>
class CustomMatrixFormVol__1_1_steadystate_newton_ : public Hermes::Hermes2D::MatrixFormVol<Scalar>
{
public:
    CustomMatrixFormVol__1_1_steadystate_newton_(unsigned int i, unsigned int j,
                              std::string area,
                              Hermes::Hermes2D::SymFlag sym,
                              Material *materialSource,
			      Material *materialTarget);

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *u,
                         Hermes::Hermes2D::Func<double> *v, Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *u,
                            Hermes::Hermes2D::Func<Hermes::Ord> *v, Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const;   	    
    CustomMatrixFormVol__1_1_steadystate_newton_<Scalar>* clone();
private:
    Material *m_materialSource;
    Material *m_materialTarget;	
    Hermes::Hermes2D::SymFlag m_sym;
     mutable Value el_epsr;
 mutable Value el_rho;
	    	
};template<typename Scalar>
class CustomVectorFormVol__1_1_steadystate_linear_ : public Hermes::Hermes2D::VectorFormVol<Scalar>
{
public:
    CustomVectorFormVol__1_1_steadystate_linear_(unsigned int i, unsigned int j,
                              std::string area, 
                              Material *materialSource,
			      Material *materialTarget	
				
    );

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                         Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                            Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const;
    CustomVectorFormVol__1_1_steadystate_linear_<Scalar>* clone();	
private:		
    Material *m_materialSource;
    Material *m_materialTarget;	
     mutable Value el_epsr;
 mutable Value el_rho;

    unsigned int j;
};template<typename Scalar>
class CustomVectorFormVol__1_1_steadystate_newton_ : public Hermes::Hermes2D::VectorFormVol<Scalar>
{
public:
    CustomVectorFormVol__1_1_steadystate_newton_(unsigned int i, unsigned int j,
                              std::string area, 
                              Material *materialSource,
			      Material *materialTarget	
				
    );

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                         Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                            Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const;
    CustomVectorFormVol__1_1_steadystate_newton_<Scalar>* clone();	
private:		
    Material *m_materialSource;
    Material *m_materialTarget;	
     mutable Value el_epsr;
 mutable Value el_rho;

    unsigned int j;
};

 template<typename Scalar>
class CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_linear_ : public Hermes::Hermes2D::ExactSolutionScalar<Scalar>
{
public:
    CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_linear_(Hermes::Hermes2D::Mesh *mesh, Boundary *boundary);

    inline Scalar value(double x, double y) const;
    void derivatives (double x, double y, Scalar& dx, Scalar& dy) const;

    Hermes::Ord ord (Hermes::Ord x, Hermes::Ord y) const
    {
        return Hermes::Ord(Hermes::Ord::get_max_order());
    }
private:
    Boundary *m_boundarySource;  
     mutable Value V;
 mutable Value sigma;
	
};template<typename Scalar>
class CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_newton_ : public Hermes::Hermes2D::ExactSolutionScalar<Scalar>
{
public:
    CustomEssentialFormSurf_electrostatic_potential_1_0_steadystate_newton_(Hermes::Hermes2D::Mesh *mesh, Boundary *boundary);

    Scalar value(double x, double y) const;
    void derivatives (double x, double y, Scalar& dx, Scalar& dy) const;

    Hermes::Ord ord (Hermes::Ord x, Hermes::Ord y) const
    {
        return Hermes::Ord(Hermes::Ord::get_max_order());
    }
private:
    Boundary *m_boundarySource;  
     mutable Value V;
 mutable Value sigma;
	
};template<typename Scalar>
class CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_linear_ : public Hermes::Hermes2D::VectorFormSurf<Scalar>
{
public:
    CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_linear_(unsigned int i, unsigned int j,
                               std::string area, 
                               Boundary *boundary);

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                         Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                            Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const;
    CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_linear_<Scalar>* clone();
private:
    Boundary *m_boundarySource;
     mutable Value V;
 mutable Value sigma;

    unsigned int j;
};template<typename Scalar>
class CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_newton_ : public Hermes::Hermes2D::VectorFormSurf<Scalar>
{
public:
    CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_newton_(unsigned int i, unsigned int j,
                               std::string area, 
                               Boundary *boundary);

    virtual Scalar value(int n, double *wt, Hermes::Hermes2D::Func<Scalar> *u_ext[], Hermes::Hermes2D::Func<double> *v,
                         Hermes::Hermes2D::Geom<double> *e, Hermes::Hermes2D::ExtData<Scalar> *ext) const;
    virtual Hermes::Ord ord(int n, double *wt, Hermes::Hermes2D::Func<Hermes::Ord> *u_ext[], Hermes::Hermes2D::Func<Hermes::Ord> *v,
                            Hermes::Hermes2D::Geom<Hermes::Ord> *e, Hermes::Hermes2D::ExtData<Hermes::Ord> *ext) const;
    CustomVectorFormSurf_electrostatic_surface_charge_density_1_1_steadystate_newton_<Scalar>* clone();
private:
    Boundary *m_boundarySource;
     mutable Value V;
 mutable Value sigma;

    unsigned int j;
}; }
#endif 
