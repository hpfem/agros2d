// This file is part of Hermes2D.
//
// Copyright 2006-2008 Lenka Dubcova <dubcova@gmail.com>
// Copyright 2005-2008 Jakub Cerveny <jakub.cerveny@gmail.com>
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

// $Id: integrals_hcurl.h 1086 2008-10-21 09:05:44Z jakub $

#ifndef __HERMES2D_INTEGRALS_HCURL_H
#define __HERMES2D_INTEGRALS_HCURL_H


//// volume integrals //////////////////////////////////////////////////////////////////////////////

#define hcurl_integrate_jac_expression(exp) \
  {double3* pt = quad->get_points(o); \
  int np = quad->get_num_points(o); \
  double2x2* me, *mf; \
  if (re->is_jacobian_const()) { \
    me = re->get_const_inv_ref_map(); \
    mf = rf->get_const_inv_ref_map(); \
    for (int i = 0; i < np; i++) \
      result += pt[i][2] * (exp); \
    result *= re->get_const_jacobian(); \
  } \
  else { \
    me = re->get_inv_ref_map(o); \
    mf = rf->get_inv_ref_map(o); \
    double* jac = re->get_jacobian(o); \
    for (int i = 0; i < np; i++, me++, mf++) \
      result += pt[i][2] * jac[i] * (exp); \
  }}

 

template<typename T>
inline T int_curl_e_curl_f(Function<T>* fe, Function<T>* ff, RefMap* re, RefMap* rf)
{
  Quad2D* quad = re->get_quad_2d();
  int o = fe->get_fn_order() + ff->get_fn_order() + 2 + re->get_inv_ref_order();  // fixme: jac nepolynom.
  limit_order(o);
  fe->set_quad_order(o, FN_DY_0 | FN_DX_1);
  ff->set_quad_order(o, FN_DY_0 | FN_DX_1);

  T *e0dy = fe->get_dy_values(0), *e1dx = fe->get_dx_values(1);
  T *f0dy = ff->get_dy_values(0), *f1dx = ff->get_dx_values(1);

  T result = 0.0;
  
  double3* pt = quad->get_points(o); 
  int np = quad->get_num_points(o);
  double2x2* me, *mf;
  me = re->get_inv_ref_map(o);
  mf = rf->get_inv_ref_map(o);
  double* jac = re->get_jacobian(o);
  for (int i = 0; i < np; i++, me++, mf++)
  {
    double e_ijac = ((*me)[0][0] * (*me)[1][1] - (*me)[1][0] * (*me)[0][1]);
    double f_ijac = ((*mf)[0][0] * (*mf)[1][1] - (*mf)[1][0] * (*mf)[0][1]);
    result += pt[i][2] * jac[i] * ((e1dx[i] - e0dy[i])*e_ijac * (f1dx[i] - f0dy[i])*f_ijac); 
  }

  return result;
}


template<typename T>
inline T int_e_f(Function<T>* fe, Function<T>* ff, RefMap* re, RefMap* rf)
{
  Quad2D* quad = re->get_quad_2d();
  int o = fe->get_fn_order() + ff->get_fn_order() + 2 + re->get_inv_ref_order();
  limit_order(o);
  fe->set_quad_order(o, FN_VAL);
  ff->set_quad_order(o, FN_VAL);

  T *e0 = fe->get_fn_values(0), *e1 = fe->get_fn_values(1);
  T *f0 = ff->get_fn_values(0), *f1 = ff->get_fn_values(1);

  T result = 0.0;

  double3* pt = quad->get_points(o);
  int np = quad->get_num_points(o);
  double2x2* me, *mf;
  me = re->get_inv_ref_map(o);
  mf = rf->get_inv_ref_map(o);
  double* jac = re->get_jacobian(o);
  for (int i = 0; i < np; i++, me++, mf++)
      result += pt[i][2] * jac[i] * (((*me)[0][0]*e0[i] + (*me)[0][1]*e1[i]) * ((*mf)[0][0]*f0[i] + (*mf)[0][1]*f1[i]) + 
                                     ((*me)[1][0]*e0[i] + (*me)[1][1]*e1[i]) * ((*mf)[1][0]*f0[i] + (*mf)[1][1]*f1[i]));
  
  return result;                               
}


#ifdef COMPLEX
inline complex int_F_f(void (*F)(double, double, complex2&), RealFunction* ff, RefMap* re, bool adapt = false, double EPS = 1e-5)
{
  Quad2D* quad = re->get_quad_2d();
  int o = quad->get_safe_max_order(); // triangles have bad points in rule 20 (outside ref. domain)
  ff->set_quad_order(o, FN_VAL);

  double *f0 = ff->get_fn_values(0), *f1 = ff->get_fn_values(1);
  double *x  = re->get_phys_x(o);
  double *y  = re->get_phys_y(o);

  complex2 fn;
  complex result(0.0, 0.0);
  
  double3* pt = quad->get_points(o);
  int np = quad->get_num_points(o);
  double2x2* me;
  me = re->get_inv_ref_map(o);
  double* jac = re->get_jacobian(o);
  for (int i = 0; i < np; i++, me++)
  {
    F(x[i],y[i], fn);
    result += pt[i][2] * jac[i] * (fn[0] * ((*me)[0][0]*f0[i] + (*me)[0][1]*f1[i]) +
                                  fn[1] * ((*me)[1][0]*f0[i] + (*me)[1][1]*f1[i]));
  }
  
  if (adapt)
  {
    scalar sum = 0.0;
    scalar sub[4];
    for (int i = 0; i < 4; i++)
    {
      ff->push_transform(i);
      re->push_transform(i);
      sum += sub[i] = int_F_f(F, ff, re);
      ff->pop_transform();
      re->pop_transform();
    } 
    
    if (std::norm(sum - result) / std::norm(result) < EPS) return sum;
    
    //if (ff->get_ctm()->m[0] < 1.0 / 265.0) info("Adaptive quadrature: depth > 8!!!");
    
    if (ff->get_ctm()->m[0] < 1.0 / 256.0)
    {
      warn("Adaptive quadrature: could not reach required accuracy.");
      return sum;
    }
    
    sum = 0.0;
    for (int i = 0; i < 4; i++)
    {
      ff->push_transform(i);
      re->push_transform(i);
      sum += int_F_f(F, ff, re, true, EPS);
      ff->pop_transform();
      re->pop_transform();
    }
    
    return sum;
  }
  
  return result;                                  
}
#endif


//// errors ///////////////////////////////////////////////////////////////////////////////////

#ifdef COMPLEX

inline double int_hcurl_error(ScalarFunction* fe, ScalarFunction* ff, RefMap* re, RefMap* rf, double kap = 1.0)
{
  Quad2D* quad = re->get_quad_2d();
  int o = 2 * std::max(fe->get_fn_order(), ff->get_fn_order()) + 2 + re->get_inv_ref_order();  // fixme: jac nepolynom.
  limit_order(o);
  fe->set_quad_order(o);
  ff->set_quad_order(o);

  scalar *e0 = fe->get_fn_values(0), *e1 = fe->get_fn_values(1);
  scalar *f0 = ff->get_fn_values(0), *f1 = ff->get_fn_values(1);
  scalar *e0dy = fe->get_dy_values(0), *e1dx = fe->get_dx_values(1);
  scalar *f0dy = ff->get_dy_values(0), *f1dx = ff->get_dx_values(1);

  double result = 0.0;
  
  double3* pt = quad->get_points(o); 
  int np = quad->get_num_points(o);
  double* jac = re->get_jacobian(o);
  for (int i = 0; i < np; i++)
  {
    result += pt[i][2] * jac[i] * ( kap * (std::norm(e0[i] - f0[i]) + std::norm(e1[i] - f1[i])) + 
                                    std::norm((e1dx[i] - e0dy[i]) - (f1dx[i] - f0dy[i])) ); 
  }
  return result;
}

inline double int_hcurl_norm(ScalarFunction* fe, RefMap* re, double kap = 1.0)
{
  Quad2D* quad = re->get_quad_2d();
  int o = 2 * fe->get_fn_order() + 2 + re->get_inv_ref_order();  // fixme: jac nepolynom.
  limit_order(o);
  fe->set_quad_order(o);

  scalar *e0 = fe->get_fn_values(0), *e1 = fe->get_fn_values(1);
  scalar *e0dy = fe->get_dy_values(0), *e1dx = fe->get_dx_values(1);

  double result = 0.0;
  
  double3* pt = quad->get_points(o); 
  int np = quad->get_num_points(o);
  double* jac = re->get_jacobian(o);
  for (int i = 0; i < np; i++)
  {
    result += pt[i][2] * jac[i] * ( kap * (std::norm(e0[i]) + std::norm(e1[i]))
                                         + std::norm(e1dx[i] - e0dy[i]) ); 
  }
  return result;
}

#endif


//// surface integrals /////////////////////////////////////////////////////////////////////////////

inline double surf_int_e_tau_f_tau(RealFunction* fe, RealFunction* ff, RefMap* re, RefMap* rf, EdgePos* ep)
{
  Quad2D* quad2d = re->get_quad_2d();
  int eo = quad2d->get_edge_points(ep->edge);
  
  fe->set_quad_order(eo, FN_VAL);
  ff->set_quad_order(eo, FN_VAL);
  double *e0 = fe->get_fn_values(0), *e1 = fe->get_fn_values(1);
  double *f0 = ff->get_fn_values(0), *f1 = ff->get_fn_values(1);
  double3* tan = re->get_tangent(ep->edge);
    
  double3* pt = quad2d->get_points(eo);
  double2x2* me = re->get_inv_ref_map(eo);
  double2x2* mf = rf->get_inv_ref_map(eo);
  double result = 0.0;
  for (int i = 0; i < quad2d->get_num_points(eo); i++, me++, mf++)
    result += pt[i][2] * tan[i][2] * (((*me)[0][0]*e0[i] + (*me)[0][1]*e1[i])*tan[i][0] + ((*me)[1][0]*e0[i] + (*me)[1][1]*e1[i])*tan[i][1]) 
                                   * (((*mf)[0][0]*f0[i] + (*mf)[0][1]*f1[i])*tan[i][0] + ((*mf)[1][0]*f0[i] + (*mf)[1][1]*f1[i])*tan[i][1]);

  return result * 0.5;
}
  

#ifdef COMPLEX
inline complex surf_int_G_tau_f_tau(RealFunction* ff, RefMap* rf, EdgePos* ep)
{
  const Quad1D* quad1d = rf->get_quad_1d();
  const int o = 24;
  int eo = rf->get_quad_2d()->get_edge_points(ep->edge);
  
  ff->set_quad_order(eo, FN_VAL);
  double *f0 = ff->get_fn_values(0), *f1 = ff->get_fn_values(1);
  double3* tan = rf->get_tangent(ep->edge);
  
  double2* pt = quad1d->get_points(o);
  double2x2* mf = rf->get_inv_ref_map(eo);
  complex result = 0.0;
  ep->space = ep->space_v;
  for (int i = 0; i < quad1d->get_num_points(o); i++, mf++)
  {
    ep->t = ep->lo + (pt[i][0] + 1.0) * 0.5 * (ep->hi - ep->lo);
    result += pt[i][1] * tan[i][2] * ep->space->bc_value_callback_by_edge(ep)
                                   * (((*mf)[0][0]*f0[i] + (*mf)[0][1]*f1[i])*tan[i][0] + ((*mf)[1][0]*f0[i] + (*mf)[1][1]*f1[i])*tan[i][1]);
  }
  
  return result * 0.5;
}


inline complex surf_int_J_f(scalar J1, scalar J2, RealFunction* ff, RefMap* rf, EdgePos* ep)
{
  Quad2D* quad2d = rf->get_quad_2d();
  int eo = quad2d->get_edge_points(ep->edge);
  
  ff->set_quad_order(eo, FN_VAL);
  double *f0 = ff->get_fn_values(0), *f1 = ff->get_fn_values(1);
  double3* tan = rf->get_tangent(ep->edge);
    
  double3* pt = quad2d->get_points(eo);
  double2x2* m = rf->get_inv_ref_map(eo);
  scalar result = 0.0;
  for (int i = 0; i < quad2d->get_num_points(eo); i++, m++)
    result += pt[i][2] * tan[i][2] * (J1*((*m)[0][0]*f0[i] + (*m)[0][1]*f1[i]) + J2*((*m)[1][0]*f0[i] + (*m)[1][1]*f1[i]));
  
  return result * 0.5;
}


inline complex surf_int_F_f(void (*Fn)(double x, double y, scalar& F0, scalar& F1), RealFunction* ff, RefMap* rf, EdgePos* ep)
{
  Quad2D* quad2d = rf->get_quad_2d();
  int eo = quad2d->get_edge_points(ep->edge);
  
  ff->set_quad_order(eo);
  
  double *f0 = ff->get_fn_values(0), *f1 = ff->get_fn_values(1);
  double* x = rf->get_phys_x(eo);
  double* y = rf->get_phys_y(eo);
  
  double3* tan = rf->get_tangent(ep->edge);    
  double3* pt = quad2d->get_points(eo);
  double2x2* m = rf->get_inv_ref_map(eo);
  scalar result = 0.0;
  scalar F0, F1;
  for (int i = 0; i < quad2d->get_num_points(eo); i++, m++)
  {
    Fn(x[i], y[i], F0, F1);
    result += pt[i][2] * tan[i][2] * (F0*((*m)[0][0]*f0[i] + (*m)[0][1]*f1[i]) + F1*((*m)[1][0]*f0[i] + (*m)[1][1]*f1[i]));
  }
  
  return result * 0.5;
}

#endif




//  adaptivni kvadratura:
// double adapt_quad_2(double (*integral)(RealFunction*, RealFunction*, RefMap*), RealFunction* fn1, RealFunction* fn2, RefMap* refmap,
//                     double val = 0.0, bool top_level = true)
// {
//   int i;
//   double EPS = 1e-12;
//   double sub[4], sum = 0.0;
//   
//   if (top_level) val = integral(fn1, fn2, refmap);
//     
//   for (i = 0; i < 4; i++)
//   {
//     fn1->push_transform(i);
//     fn2->push_transform(i);
//     refmap->push_transform(i);
//     sum += sub[i] = integral(fn1, fn2, refmap);
//     fn1->pop_transform();
//     fn2->pop_transform();
//     refmap->pop_transform();
//   } 
//   
//   if (fabs(sum - val) / val < EPS) return sum;
//     
//   sum = 0.0;
//   for (i = 0; i < 4; i++)
//   {
//     fn1->push_transform(i);
//     fn2->push_transform(i);
//     refmap->push_transform(i);
//     sum += adapt_quad_2(integral, fn1, fn2, refmap, sub[i], false);
//     fn1->pop_transform();
//     fn2->pop_transform();
//     refmap->pop_transform();
//   }
//   return sum;
// }


/*return int_grad_u_grad_v(fnu, fnv, refmap);
return adapt_quad_2(int_grad_u_grad_v, fnu, fnv, refmap);*/



#endif
