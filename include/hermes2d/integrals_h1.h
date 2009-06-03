// This file is part of Hermes2D.
//
// Copyright 2005-2008 Jakub Cerveny <jakub.cerveny@gmail.com>
// Copyright 2005-2008 Lenka Dubcova <dubcova@gmail.com>
// Copyright 2005-2008 Pavel Solin <solin@unr.edu>
//
// Hermes2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Hermes2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hermes2D.  If not, see <http://www.gnu.org/licenses/>.

// $Id: integrals_h1.h 1086 2008-10-21 09:05:44Z jakub $

#ifndef __HERMES2D_INTEGRALS_H1_H
#define __HERMES2D_INTEGRALS_H1_H


//// volume integrals //////////////////////////////////////////////////////////////////////////////

// the inner integration loops for both constant and non-constant jacobian elements
// for expression without partial derivatives - the variables e, quad, o must be already
// defined and initialized
#define h1_integrate_expression(exp) \
  {double3* pt = quad->get_points(o); \
  int np = quad->get_num_points(o); \
  if (ru->is_jacobian_const()){ \
    for (int i = 0; i < np; i++) \
      result += pt[i][2] * (exp); \
    result *= ru->get_const_jacobian(); \
  } \
  else { \
    double* jac = ru->get_jacobian(o); \
    for (int i = 0; i < np; i++) \
      result += pt[i][2] * jac[i] * (exp); \
  }}


// the inner integration loops for both constant and non-constant jacobian elements
// for expressions containing partial derivatives (ie. their transformations)
// - the variables e, quad, o must be already defined and initialized
#define h1_integrate_dd_expression(exp) \
  {double3* pt = quad->get_points(o); \
  int np = quad->get_num_points(o); \
  double2x2 *mv, *mu; \
  if (ru->is_jacobian_const()) { \
    mu = ru->get_const_inv_ref_map(); \
    mv = rv->get_const_inv_ref_map(); \
    for (int i = 0; i < np; i++) \
      result += pt[i][2] * (exp); \
    result *= ru->get_const_jacobian(); \
  } \
  else { \
    mu = ru->get_inv_ref_map(o); \
    mv = rv->get_inv_ref_map(o); \
    double* jac = ru->get_jacobian(o); \
    for (int i = 0; i < np; i++, mu++, mv++) \
      result += pt[i][2] * jac[i] * (exp); \
  }}


// transformations of partial derivatives
#define t_dudx (dudx[i]*(*mu)[0][0] + dudy[i]*(*mu)[0][1])
#define t_dudy (dudx[i]*(*mu)[1][0] + dudy[i]*(*mu)[1][1])
#define t_dvdx (dvdx[i]*(*mv)[0][0] + dvdy[i]*(*mv)[0][1])
#define t_dvdy (dvdx[i]*(*mv)[1][0] + dvdy[i]*(*mv)[1][1])




inline double int_u(RealFunction* fu, RefMap* ru)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o, FN_VAL);

  double* uval = fu->get_fn_values();
  
  double result = 0.0;
  h1_integrate_expression(uval[i]);
  return result;
}

#define int_v(fv, rv) int_u(fv, rv)


inline double int_u_v(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o, FN_VAL);
  fv->set_quad_order(o, FN_VAL);

  double* uval = fu->get_fn_values();
  double* vval = fv->get_fn_values();

  double result = 0.0;
  h1_integrate_expression(uval[i] * vval[i]);
  return result;
}


inline scalar int_w_v(ScalarFunction* w, RealFunction* fu, RefMap* ru)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + w->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o, FN_VAL);
  w->set_quad_order(o, FN_VAL);

  double* uval = fu->get_fn_values();
  scalar* wval = w->get_fn_values();

  scalar result = 0.0;
  double3* pt = quad->get_points(o);
  int np = quad->get_num_points(o);
  if (ru->is_jacobian_const()){ 
    for (int i = 0; i < np; i++)
      result += pt[i][2] * (uval[i] * wval[i]); 
    result *= ru->get_const_jacobian(); 
  } 
  else { 
    double* jac = ru->get_jacobian(o); 
    for (int i = 0; i < np; i++) 
      result += pt[i][2] * jac[i] * (uval[i] * wval[i]); 
  }
  return result;
}


inline scalar surf_int_w_u_v(ScalarFunction* w, RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = w->get_fn_order() + fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o, FN_VAL);
  fv->set_quad_order(o, FN_VAL);
  w->set_quad_order(o, FN_VAL);

  double* uval = fu->get_fn_values();
  double* vval = fv->get_fn_values();
  scalar* wval = w->get_fn_values();
  
  scalar result = 0.0;
  h1_integrate_expression(wval[i] * uval[i] * vval[i]);
  return result;
}


inline double int_F_u(double (*F)(double x, double y), RealFunction* fu, RefMap* ru)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = quad->get_safe_max_order(); // triangles have bad points in rule 20 (outside ref. domain)
  limit_order(o);
  fu->set_quad_order(o, FN_VAL);

  double* uval = fu->get_fn_values();
  double* x = ru->get_phys_x(o);
  double* y = ru->get_phys_y(o);

  double result = 0.0;
  h1_integrate_expression(uval[i] * F(x[i], y[i]));
  return result;
}

#define int_F_v(F, fv, rv) int_F_u(F, fv, rv)


inline double int_x_u(RealFunction* fu, RefMap* ru)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + ru->get_inv_ref_order() + 1;
  limit_order(o);
  fu->set_quad_order(o, FN_VAL);

  double* uval = fu->get_fn_values();
  double* x = ru->get_phys_x(o);

  double result = 0.0;
  h1_integrate_expression(x[i] * uval[i]);
  return result;
}

#define int_x_v(fv, rv) int_x_u(fv, rv)


inline double int_x_u_v(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order() + 1;
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);
  
  double* uval = fu->get_fn_values();
  double* vval = fv->get_fn_values();
  double* x = ru->get_phys_x(o);

  double result = 0.0;
  h1_integrate_expression(x[i] * uval[i] * vval[i]);
  return result;
}


inline double int_u_dvdx(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *uval = fu->get_fn_values();
  double *dvdx = fv->get_dx_values();
  double *dvdy = fv->get_dy_values();

  double result = 0.0;
  h1_integrate_dd_expression(uval[i] * t_dvdx);
  return result;
}


inline double int_u_dvdy(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *uval = fu->get_fn_values();
  double *dvdx = fv->get_dx_values();
  double *dvdy = fv->get_dy_values();

  double result = 0.0;
  h1_integrate_dd_expression(uval[i] * t_dvdy);
  return result;
}


#define int_dudx_v(fu, fv, ru, rv) int_u_dvdx(fv, fu, rv, ru)
#define int_dudy_v(fu, fv, ru, rv) int_u_dvdy(fv, fu, rv, ru)


inline double int_dudx_dvdx(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);

  double result = 0.0;
  h1_integrate_dd_expression(t_dudx * t_dvdx);
  return result;
}


inline double int_dudy_dvdy(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);

  double result = 0.0;
  h1_integrate_dd_expression(t_dudy * t_dvdy);
  return result;
}


inline double int_dudx_dvdy(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);

  double result = 0.0;
  h1_integrate_dd_expression(t_dudx * t_dvdy);
  return result;
}


#define int_dudy_dvdx(fu, fv, ru, rv) int_dudx_dvdy(fv, fu, rv, ru)


inline double int_grad_u_grad_v(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);

  double result = 0.0;
  h1_integrate_dd_expression(t_dudx * t_dvdx + t_dudy * t_dvdy);
  return result;
}


inline scalar int_grad_w_grad_v(ScalarFunction* w, RealFunction* fu, RefMap* ru)
{
  Quad2D* quad = fu->get_quad_2d();
  RefMap* rv = ru;
  
  int o = fu->get_fn_order() + w->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);
  w->set_quad_order(o);

  double *dudx, *dudy;
  scalar *dwdx, *dwdy;
  fu->get_dx_dy_values(dudx, dudy);
  w->get_dx_dy_values(dwdx, dwdy);

  scalar result = 0.0;
  double3* pt = quad->get_points(o);
  int np = quad->get_num_points(o); 
  double2x2 *mu;
  if (ru->is_jacobian_const()) { 
    mu = ru->get_const_inv_ref_map(); 
    for (int i = 0; i < np; i++) 
      result += pt[i][2] * (t_dudx * dwdx[i] + t_dudy * dwdy[i]);
    result *= ru->get_const_jacobian();
  } 
  else { 
    mu = ru->get_inv_ref_map(o); 
    double* jac = ru->get_jacobian(o); 
    for (int i = 0; i < np; i++, mu++) 
      result += pt[i][2] * jac[i] * (t_dudx * dwdx[i] + t_dudy * dwdy[i]); 
  }
  return result;
}


inline scalar int_x_grad_w_grad_v(ScalarFunction* w, RealFunction* fu, RefMap* ru)
{
  Quad2D* quad = fu->get_quad_2d();
  RefMap* rv = ru;

  int o = fu->get_fn_order() + w->get_fn_order() + ru->get_inv_ref_order() +1;
  limit_order(o);
  fu->set_quad_order(o);
  w->set_quad_order(o);

  double *dudx, *dudy;
  scalar *dwdx, *dwdy;
  fu->get_dx_dy_values(dudx, dudy);
  w->get_dx_dy_values(dwdx, dwdy);
  double* x = ru->get_phys_x(o);

  scalar result = 0.0;
  double3* pt = quad->get_points(o);
  int np = quad->get_num_points(o);
  double2x2 *mu;
  if (ru->is_jacobian_const()) {
    mu = ru->get_const_inv_ref_map();
    for (int i = 0; i < np; i++)
      result += pt[i][2] * x[i] * (t_dudx * dwdx[i] + t_dudy * dwdy[i]);
    result *= ru->get_const_jacobian();
  }
  else {
    mu = ru->get_inv_ref_map(o);
    double* jac = ru->get_jacobian(o);
    for (int i = 0; i < np; i++, mu++)
      result += pt[i][2] * jac[i] * x[i] * (t_dudx * dwdx[i] + t_dudy * dwdy[i]);
  }
  return result;
}


inline double int_x_grad_u_grad_v(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + 1 + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);
  double* x = ru->get_phys_x(o);

  double result = 0.0;
  h1_integrate_dd_expression(x[i] * (t_dudx * t_dvdx + t_dudy * t_dvdy));
  return result;
}


inline double int_w_nabla_u_v(RealFunction* w1, RealFunction* w2,
                              RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() +
          w1->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);

  w1->set_quad_order(o, FN_VAL);
  w2->set_quad_order(o, FN_VAL);
  fu->set_quad_order(o);
  fv->set_quad_order(o, FN_VAL);

  double *dudx, *dudy;
  fu->get_dx_dy_values(dudx, dudy);
  double* vval = fv->get_fn_values();
  double* w1val = w1->get_fn_values();
  double* w2val = w2->get_fn_values();

  double result = 0.0;
  h1_integrate_dd_expression((w1val[i] * t_dudx + w2val[i] * t_dudy) * vval[i]);
  return result;
}


inline double int_a_dudx_dvdx_b_dudy_dvdy(double a, RealFunction* fu, double b, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);

  double result = 0.0;
  h1_integrate_dd_expression(a * t_dudx * t_dvdx + b * t_dudy * t_dvdy);
  return result;
}


inline double int_a_dudx_dvdy_b_dudy_dvdx(double a, RealFunction* fu, double b, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);

  double result = 0.0;
  h1_integrate_dd_expression(a * t_dudx * t_dvdy + b * t_dudy * t_dvdx);
  return result;
}


inline double int_u_v_over_x(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order() + 4;
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);
  double *uval, *vval;
  uval = fu->get_fn_values();
  vval = fv->get_fn_values();
  double* x = ru->get_phys_x(o);

  double result = 0.0;
  h1_integrate_expression(uval[i] * vval[i] / x[i]);
  return result;
}


inline double int_u_v_over_y(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order() + 10;
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);
  double *uval, *vval;
  uval = fu->get_fn_values();
  vval = fv->get_fn_values();
  double* y = ru->get_phys_y(o);

  double result = 0.0;
  h1_integrate_expression(uval[i] * vval[i] / y[i]);
  return result;
}


inline double int_F_u_v(double (*F)(double x, double y),
        RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order() +4;
  limit_order(o);
  fu->set_quad_order(o, FN_VAL);
  fv->set_quad_order(o, FN_VAL);

  double* uval = fu->get_fn_values();
  double* vval = fv->get_fn_values();
  double* x = ru->get_phys_x(o);
  double* y = ru->get_phys_y(o);

  double result = 0.0;
  h1_integrate_expression(uval[i] * vval[i] * F(x[i], y[i]));
  return result;
}


inline double int_x_a_dudx_dvdx_b_dudy_dvdy(double a, RealFunction* fu, double b, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order() + 1;
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);
  double* x = ru->get_phys_x(o);

  double result = 0.0;
  h1_integrate_dd_expression((x[i] * (a * t_dudx * t_dvdx + b * t_dudy * t_dvdy)));
  return result;
}

#define int_x_a_dudy_dvdy_b_dudx_dvdx(a, fu, b, fv, ru, rv) int_x_a_dudx_dvdx_b_dudy_dvdy(b, fu, a, fv, ru, rv)


inline double int_x_dudx_dvdy(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order() + 1;
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);
  double* x = ru->get_phys_x(o);

  double result = 0.0;
  h1_integrate_dd_expression(x[i] * t_dudx * t_dvdy);
  return result;
}

#define int_x_dudy_dvdx(fu, fv, ru, rv) int_x_dudx_dvdy(fv, fu, rv, ru)



inline double int_x2_dudx_dvdx(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order() + 2;
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);
  double* x = ru->get_phys_x(o);

  double result = 0.0;
  h1_integrate_dd_expression(sqr(x[i]) * t_dudx * t_dvdx);
  return result;
}


inline double int_xy_dudx_dvdy(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order() + 2;
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);
  double* x = ru->get_phys_x(o);
  double* y = ru->get_phys_y(o);

  double result = 0.0;
  h1_integrate_dd_expression(x[i] * y[i] * t_dudx * t_dvdy);
  return result;
}


inline double int_yx_dudy_dvdx(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order() + 2;
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);
  double* x = ru->get_phys_x(o);
  double* y = ru->get_phys_y(o);

  double result = 0.0;
  h1_integrate_dd_expression(y[i] * x[i] * t_dudy * t_dvdx);
  return result;
}


inline double int_y2_dudy_dvdy(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order() + 2;
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);
  double* y = ru->get_phys_y(o);

  double result = 0.0;
  h1_integrate_dd_expression(sqr(y[i]) * t_dudy * t_dvdy);
  return result;
}


inline double int_x_dudx_v(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order() + 1;
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *dudx, *dudy;
  fu->get_dx_dy_values(dudx, dudy);
  double* vval = fv->get_fn_values();
  double* x = ru->get_phys_x(o);

  double result = 0.0;
  h1_integrate_dd_expression(x[i] * t_dudx * vval[i]);
  return result;
}


inline double int_y_dudy_v(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = fu->get_fn_order() + fv->get_fn_order() + ru->get_inv_ref_order() + 1;
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  double *dudx, *dudy;
  fu->get_dx_dy_values(dudx, dudy);
  double* vval = fv->get_fn_values();
  double* y = ru->get_phys_y(o);

  double result = 0.0;
  h1_integrate_dd_expression(y[i] * t_dudy * vval[i]);
  return result;
}


//// error & norm integrals ////////////////////////////////////////////////////////////////////////

template<typename T>
inline double int_h1_error(Function<T>* fu, Function<T>* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();
  assert(quad == fv->get_quad_2d());

  int o = std::max(2*fu->get_fn_order(), 2*fv->get_fn_order()) + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  scalar* fnu = fu->get_fn_values();
  scalar* fnv = fv->get_fn_values();

  scalar *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);

  double result = 0.0;
  h1_integrate_expression(sqr(fnu[i] - fnv[i]) + sqr(dudx[i] - dvdx[i]) + sqr(dudy[i] - dvdy[i]));
  return result;
}


template<typename T>
inline double int_h1_semi_error(Function<T>* fu, Function<T>* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();
  assert(quad == fv->get_quad_2d());
  
  int o = std::max(2*fu->get_fn_order(), 2*fv->get_fn_order()) + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  scalar* fnu = fu->get_fn_values();
  scalar* fnv = fv->get_fn_values();
  
  scalar *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);
  
  double result = 0.0;
  h1_integrate_expression(sqr(dudx[i] - dvdx[i]) + sqr(dudy[i] - dvdy[i]));
  return result;
}


template<typename T>
inline double int_l2_error(Function<T>* fu, Function<T>* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();
  assert(quad == fv->get_quad_2d());

  int o = std::max(2*fu->get_fn_order(), 2*fv->get_fn_order()) + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o, FN_VAL);
  fv->set_quad_order(o, FN_VAL);

  scalar* fnu = fu->get_fn_values();
  scalar* fnv = fv->get_fn_values();

  double result = 0.0;
  h1_integrate_expression(sqr(fnu[i] - fnv[i]));
  return result;
}


template<typename T>
inline double int_dx_error(Function<T>* fu, Function<T>* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();
  assert(quad == fv->get_quad_2d());

  int o = std::max(2*fu->get_fn_order(), 2*fv->get_fn_order()) + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  scalar *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);

  double result = 0.0;
  h1_integrate_expression(sqr(dudx[i] - dvdx[i]));
  return result;
}


template<typename T>
inline double int_dy_error(Function<T>* fu, Function<T>* fv, RefMap* ru, RefMap* rv)
{
  Quad2D* quad = fu->get_quad_2d();
  assert(quad == fv->get_quad_2d());

  int o = std::max(2*fu->get_fn_order(), 2*fv->get_fn_order()) + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);
  fv->set_quad_order(o);

  scalar *dudx, *dudy, *dvdx, *dvdy;
  fu->get_dx_dy_values(dudx, dudy);
  fv->get_dx_dy_values(dvdx, dvdy);

  double result = 0.0;
  h1_integrate_expression(sqr(dudy[i] - dvdy[i]));
  return result;
}


template<typename T>
inline double int_h1_norm(Function<T>* fu, RefMap* ru)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = 2*fu->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);

  scalar* fnu = fu->get_fn_values();
  scalar *dudx, *dudy;
  fu->get_dx_dy_values(dudx, dudy);
  
  double result = 0.0;
  h1_integrate_expression(sqr(fnu[i]) + sqr(dudx[i]) + sqr(dudy[i]));
  return result;
}


template<typename T>
inline double int_h1_seminorm(Function<T>* fu, RefMap* ru)
{
  Quad2D* quad = fu->get_quad_2d();
  
  int o = 2*fu->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o);
  
  scalar* fnu = fu->get_fn_values();
  scalar *dudx, *dudy;
  fu->get_dx_dy_values(dudx, dudy);
  
  double result = 0.0;
  h1_integrate_expression(sqr(dudx[i]) + sqr(dudy[i]));
  return result;
}


template<typename T>
inline double int_l2_norm(Function<T>* fu, RefMap* ru)
{
  Quad2D* quad = fu->get_quad_2d();

  int o = 2*fu->get_fn_order() + ru->get_inv_ref_order();
  limit_order(o);
  fu->set_quad_order(o, FN_VAL);

  scalar* fnu = fu->get_fn_values();
  
  double result = 0.0;
  h1_integrate_expression(sqr(fnu[i]));
  return result;
}


//// surface integrals /////////////////////////////////////////////////////////////////////////////

inline double surf_int_v(RealFunction* fv, RefMap* rv, EdgePos* ep)
{
  Quad2D* quad2d = rv->get_quad_2d();
  int eo = quad2d->get_edge_points(ep->edge);

  fv->set_quad_order(eo, FN_VAL);
  double* vval = fv->get_fn_values();
  double3* tan = rv->get_tangent(ep->edge);

  double3* pt = quad2d->get_points(eo);
  double result = 0.0;
  for (int i = 0; i < quad2d->get_num_points(eo); i++)
    result += pt[i][2] * vval[i] * tan[i][2];

  return result * 0.5;
}


inline double surf_int_x_v(RealFunction* fv, RefMap* rv, EdgePos* ep)
{
  Quad2D* quad2d = rv->get_quad_2d();
  int eo = quad2d->get_edge_points(ep->edge);

  fv->set_quad_order(eo, FN_VAL);
  double* vval = fv->get_fn_values();
  double3* tan = rv->get_tangent(ep->edge);
  double* x = rv->get_phys_x(eo);
  
  double3* pt = quad2d->get_points(eo);
  double result = 0.0;
  for (int i = 0; i < quad2d->get_num_points(eo); i++)
    result += pt[i][2] * x[i] * vval[i] * tan[i][2];

  return result * 0.5;
}


inline scalar surf_int_G_v(RealFunction* fv, RefMap* rv, EdgePos* ep)
{
  const Quad1D* quad1d = rv->get_quad_1d();
  int eo = rv->get_quad_2d()->get_edge_points(ep->edge);
  const int o = 24;

  fv->set_quad_order(eo, FN_VAL);
  double* vval = fv->get_fn_values();
  double3* tan = rv->get_tangent(ep->edge);

  double2* pt = quad1d->get_points(o);
  int np = quad1d->get_num_points(o);
  scalar result = 0.0;
  double tmp = 0.5 * (ep->hi - ep->lo);
  ep->space = ep->space_v;
  for (int i = 0; i < np; i++)
  {
    ep->t = ep->lo + (pt[i][0] + 1.0) * tmp;
    result += pt[i][1] * vval[i] * tan[i][2] * ep->space->bc_value_callback_by_edge(ep);
  }

  return result * 0.5;
}


inline double surf_int_u_v(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv, EdgePos* ep)
{
  Quad2D* quad2d = ru->get_quad_2d();
  int eo = quad2d->get_edge_points(ep->edge);

  fu->set_quad_order(eo, FN_VAL);
  fv->set_quad_order(eo, FN_VAL);
  double* uval = fu->get_fn_values();
  double* vval = fv->get_fn_values();
  double3* tan = ru->get_tangent(ep->edge);

  double3* pt = quad2d->get_points(eo);
  double result = 0.0;
  for (int i = 0; i < quad2d->get_num_points(eo); i++)
    result += pt[i][2] * uval[i] * vval[i] * tan[i][2];

  return result * 0.5;
}

inline scalar surf_int_w_u_v(ScalarFunction* w, RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv, EdgePos* ep)
{
  Quad2D* quad2d = ru->get_quad_2d();
  int eo = quad2d->get_edge_points(ep->edge);

  fu->set_quad_order(eo, FN_VAL);
  fv->set_quad_order(eo, FN_VAL);
  w->set_quad_order(eo, FN_VAL);
  double* uval = fu->get_fn_values();
  double* vval = fv->get_fn_values();
  scalar* wval = w->get_fn_values();
  double3* tan = ru->get_tangent(ep->edge);

  double3* pt = quad2d->get_points(eo);
  scalar result = 0.0;
  for (int i = 0; i < quad2d->get_num_points(eo); i++)
    result += pt[i][2] * wval[i] * uval[i] * vval[i] * tan[i][2];

  return result * 0.5;
}


inline double surf_int_x_u_v(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv, EdgePos* ep)
{
  Quad2D* quad2d = ru->get_quad_2d();
  int eo = quad2d->get_edge_points(ep->edge);

  fu->set_quad_order(eo, FN_VAL);
  fv->set_quad_order(eo, FN_VAL);
  double* uval = fu->get_fn_values();
  double* vval = fv->get_fn_values();
  double3* tan = ru->get_tangent(ep->edge);
  double* x = rv->get_phys_x(eo);

  double3* pt = quad2d->get_points(eo);
  double result = 0.0;
  for (int i = 0; i < quad2d->get_num_points(eo); i++)
    result += pt[i][2] * x[i] * uval[i] * vval[i] * tan[i][2];

  return result * 0.5;
}


inline double surf_int_dudx_v(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv, EdgePos* ep)
{
  Quad2D* quad2d = ru->get_quad_2d();
  int eo = quad2d->get_edge_points(ep->edge);

  fu->set_quad_order(eo);
  fv->set_quad_order(eo, FN_VAL);
  double* dudx = fu->get_dx_values();
  double* dudy = fu->get_dy_values();
  double* vval = fv->get_fn_values();
  double3* tan = ru->get_tangent(ep->edge);

  double3* pt = quad2d->get_points(eo);
  double2x2* mu = ru->get_inv_ref_map(eo);
  double result = 0.0;
  for (int i = 0; i < quad2d->get_num_points(eo); i++, mu++)
    result += pt[i][2] * t_dudx * vval[i] * tan[i][2];

  return result * 0.5;
}


inline double surf_int_dudy_v(RealFunction* fu, RealFunction* fv, RefMap* ru, RefMap* rv, EdgePos* ep)
{
  Quad2D* quad2d = ru->get_quad_2d();
  int eo = quad2d->get_edge_points(ep->edge);

  fu->set_quad_order(eo);
  fv->set_quad_order(eo, FN_VAL);
  double* dudx = fu->get_dx_values();
  double* dudy = fu->get_dy_values();
  double* vval = fv->get_fn_values();
  double3* tan = ru->get_tangent(ep->edge);

  double3* pt = quad2d->get_points(eo);
  double2x2* mu = ru->get_inv_ref_map(eo);
  double result = 0.0;
  for (int i = 0; i < quad2d->get_num_points(eo); i++, mu++)
    result += pt[i][2] * t_dudy * vval[i] * tan[i][2];

  return result * 0.5;
}


#define surf_int_u_dvdx(fu, fv, ru, rv, ep)  surf_int_dudx_v(fv, fu, rv, ru, ep)
#define surf_int_u_dvdy(fu, fv, ru, rv, ep)  surf_int_dudy_v(fv, fu, rv, ru, ep)


#endif
